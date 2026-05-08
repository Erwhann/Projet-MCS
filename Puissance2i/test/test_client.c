/*
Nom du programme : test_client.c
Auteurs : Erwhann Deiss, Dorian Pazdziej, Matteo Delattre
Description : client de test automatique pour le jeu puissance 2i

Usage : ./test_client <IP_SERVEUR> [PORT] [MODE] [PSEUDO]
  MODE :
    win        - joue toujours colonne 4 => victoire verticale garantie en 4 coups
    lose       - alterne col 1 et col 7 (jetons disperses, ne gagne jamais)
    fill       - remplit la grille col par col jusqu'au match nul
    overflow   - joue toujours col 4 meme quand elle est pleine (teste RES_MOVE_ERROR)
    random     - coups aleatoires (defaut)
    idle       - se connecte sans jouer (teste timeout / deconnexion)
    tournament - rejoint le tournoi et joue en mode win
  Par defaut : random
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <time.h>

#include "../reseau/session.h"
#include "../reseau/data.h"
#include "../commun/protocol.h"
#include "../commun/structures.h"

#define SRV_PORT_DEF 5000
#define DELAI_MS     300   // delai en ms entre deux coups (pour voir ce qui se passe)

// ── modes de jeu du bot ──────────────────────────────────────────────────────
typedef enum {
    MODE_WIN,        // joue toujours col 4 => victoire verticale en 4 coups
    MODE_LOSE,       // alterne col 1 et col 7 => jetons disperses, ne gagne jamais
    MODE_FILL,       // remplit la grille col par col => force le match nul
    MODE_OVERFLOW,   // joue toujours col 4 meme si pleine => teste RES_MOVE_ERROR
    MODE_RANDOM,     // coups aleatoires
    MODE_IDLE,       // ne joue pas => teste deconnexion / timeout
    MODE_TOURNAMENT, // rejoint le tournoi et joue en mode win
} ModeBot;

// ── etat interne du bot ──────────────────────────────────────────────────────
static int      g_mon_id    = 0;
static int      g_id_partie = 0;
static int      g_tour_id   = 0;
static int      g_grille[6][7];
static ModeBot  g_mode      = MODE_RANDOM;
static int      g_fill_col  = 0;   // colonne courante pour MODE_FILL
static int      g_lose_alt  = 0;   // alternance pour MODE_LOSE (0=col0, 1=col6)

// ── affichage de la grille dans le terminal ──────────────────────────────────
static void afficher_grille(void) {
    printf("\n  1   2   3   4   5   6   7\n");
    printf("+---+---+---+---+---+---+---+\n");
    for (int i = 0; i < 6; i++) {
        printf("|");
        for (int j = 0; j < 7; j++) {
            int v = g_grille[i][j];
            if (v == g_mon_id)       printf(" O |");
            else if (v != 0)         printf(" X |");
            else                     printf("   |");
        }
        printf("\n");
    }
    printf("+---+---+---+---+---+---+---+\n");
}

// ── compte les jetons dans une colonne ───────────────────────────────────────
static int hauteur_colonne(int col) {
    int h = 0;
    for (int i = 0; i < 6; i++)
        if (g_grille[i][col] != 0) h++;
    return h;
}

// ── choisit le prochain coup selon le mode ───────────────────────────────────
static int choisir_coup(void) {
    switch (g_mode) {

    case MODE_WIN:
        // toujours col 4 (index 3) pour viser la victoire verticale
        if (hauteur_colonne(3) < 6) return 3;
        // si col 4 pleine : repli sur la premiere colonne libre
        for (int j = 0; j < 7; j++)
            if (hauteur_colonne(j) < 6) return j;
        return 0;

    case MODE_LOSE:
        // alterne entre col 1 (index 0) et col 7 (index 6) pour disperser les jetons
        // ne peut jamais aligner 4 avec cette strategie
        g_lose_alt = !g_lose_alt;
        if (hauteur_colonne(g_lose_alt ? 6 : 0) < 6)
            return g_lose_alt ? 6 : 0;
        // repli si la colonne est pleine
        for (int j = 0; j < 7; j++)
            if (hauteur_colonne(j) < 6) return j;
        return 0;

    case MODE_FILL:
        // remplit la grille colonne par colonne de gauche a droite (force match nul)
        while (g_fill_col < 7 && hauteur_colonne(g_fill_col) >= 6)
            g_fill_col++;
        if (g_fill_col >= 7) {
            // toutes les colonnes pleines : ne peut plus jouer
            printf("[BOT] Grille pleine, impossible de jouer !\n");
            for (int j = 0; j < 7; j++)
                if (hauteur_colonne(j) < 6) return j;
        }
        return g_fill_col;

    case MODE_OVERFLOW:
        // joue TOUJOURS col 4 (index 3), meme quand elle est pleine
        // teste que le serveur rejette le coup avec RES_MOVE_ERROR
        printf("[BOT-OVERFLOW] Tentative dans col 4 (hauteur=%d/6)\n",
               hauteur_colonne(3));
        return 3;

    case MODE_RANDOM:
    default: {
        // coups aleatoires parmi les colonnes non pleines
        int jouables[7], nb = 0;
        for (int j = 0; j < 7; j++)
            if (hauteur_colonne(j) < 6) jouables[nb++] = j;
        if (nb == 0) return 0;
        return jouables[rand() % nb];
    }

    case MODE_IDLE:
        return -1;  // ne joue pas
    }
}

// ── envoie un coup apres un petit delai ──────────────────────────────────────
static void jouer_coup(int sock) {
    if (g_mode == MODE_IDLE) {
        printf("[BOT] Mode idle : je n'envoie pas de coup.\n");
        return;
    }
    usleep(DELAI_MS * 1000);
    int col = choisir_coup();
    printf("[BOT] Je joue colonne %d\n", col + 1);
    PayloadMove pm;
    pm.colonne = col;
    envoyer_message(sock, REQ_MOVE, &pm, sizeof(pm));
}

// ── traitement des messages du serveur ───────────────────────────────────────
static int traiter(int sock, Header *h, void *payload) {
    switch (h->type) {

    case RES_LOGIN_OK: {
        PayloadLoginOK *p = (PayloadLoginOK *)payload;
        g_mon_id = p->id_joueur;
        printf("[BOT] Connecte ! ID=%d  ELO=%d\n", g_mon_id, p->elo);
        if (g_mode == MODE_TOURNAMENT) {
            // mode tournoi : on rejoint le tournoi au lieu du matchmaking
            printf("[BOT] Demande d'inscription au tournoi...\n");
            envoyer_message(sock, REQ_JOIN_TOURNAMENT, NULL, 0);
        } else {
            printf("[BOT] Demande de matchmaking...\n");
            envoyer_message(sock, REQ_MATCHMAKING, NULL, 0);
        }
        break;
    }

    case RES_LOGIN_ERROR:
        fprintf(stderr, "[BOT] Login refuse (pseudo deja pris ?).\n");
        return -1;

    case RES_WAITING:
        printf("[BOT] En attente d'un adversaire...\n");
        break;

    case PUSH_MATCH_FOUND: {
        PayloadMatchFound *p = (PayloadMatchFound *)payload;
        g_id_partie = p->id_partie;
        printf("[BOT] Partie #%d trouvee ! Adversaire : %s (ELO %d)\n",
               p->id_partie, p->pseudo, p->elo);
        // si c'est un challenge, on accepte automatiquement en mode non-ELO
        break;
    }

    case PUSH_CHOOSE_ELO: {
        // en mode test : on choisit toujours une partie sans impact ELO
        printf("[BOT] Choix ELO : partie amicale (sans ELO)\n");
        PayloadSetEloMode psm;
        psm.elo_impact = 0;
        envoyer_message(sock, REQ_SET_ELO_MODE, &psm, sizeof(psm));
        break;
    }

    case PUSH_GRID: {
        PayloadGrid *pg = (PayloadGrid *)payload;
        memcpy(g_grille, pg->grille, sizeof(g_grille));
        g_tour_id = pg->tour_de_jeu;
        afficher_grille();
        // si c'est notre tour, on joue
        if (g_tour_id == g_mon_id)
            jouer_coup(sock);
        break;
    }

    case PUSH_TURN:
        // serveur nous signale que c'est notre tour
        if (g_tour_id == g_mon_id)
            jouer_coup(sock);
        break;

    case RES_MOVE_ERROR:
        // coup invalide (colonne pleine ou hors limites)
        if (g_mode == MODE_OVERFLOW) {
            // mode overflow : on insiste dans la meme colonne pleine expres
            printf("[BOT-OVERFLOW] Serveur a rejete le coup (colonne pleine) -- on reessaie quand meme !\n");
            usleep(DELAI_MS * 1000);
            PayloadMove pm; pm.colonne = 3;
            envoyer_message(sock, REQ_MOVE, &pm, sizeof(pm));
        } else {
            printf("[BOT] Coup invalide, je reessaie dans une autre colonne...\n");
            jouer_coup(sock);
        }
        break;

    case PUSH_TOURNAMENT_STATE: {
        // affichage de l'etat du tournoi dans le terminal
        PayloadTournamentState *pt = (PayloadTournamentState *)payload;
        printf("[BOT-TOURNOI] Etat : %s | Joueurs : %d/%d\n",
               pt->msg, pt->nb_joueurs, MAX_JOUEURS_TOURNOI);
        break;
    }

    case RES_TOURNAMENT_JOINED:
        printf("[BOT-TOURNOI] Inscription confirmee ! En attente des autres joueurs...\n");
        break;

    case RES_TOURNAMENT_ERROR:
        printf("[BOT-TOURNOI] Erreur d'inscription (deja inscrit, tournoi plein ou en cours).\n");
        break;

    case PUSH_ENDGAME: {
        PayloadEndGame *pe = (PayloadEndGame *)payload;
        if (pe->id_vainqueur == g_mon_id)
            printf("[BOT] J'ai gagne ! Points: %+d  Nouvel ELO: %d\n",
                   pe->points_gagnes, pe->nouvel_elo);
        else if (pe->id_vainqueur == 0)
            printf("[BOT] Match nul. Points: %+d  Nouvel ELO: %d\n",
                   pe->points_gagnes, pe->nouvel_elo);
        else
            printf("[BOT] Perdu. Points: %+d  Nouvel ELO: %d\n",
                   pe->points_gagnes, pe->nouvel_elo);

        // une seule partie : on quitte proprement apres le resultat
        g_fill_col = 0;
        g_lose_alt = 0;
        if (g_mode == MODE_TOURNAMENT) {
            // en tournoi : on reste connecte pour les prochains matchs du bracket
            printf("[BOT-TOURNOI] Partie terminee, en attente du prochain match...\n");
        } else {
            printf("[BOT] Fin de la partie. Deconnexion.\n");
            return -1;  // signal de sortie de la boucle principale
        }
        break;
    }

    case PUSH_CHALLENGE: {
        // on accepte automatiquement tous les defis
        PayloadChallengeReceived *pcr = (PayloadChallengeReceived *)payload;
        printf("[BOT] Defi recu de %s, j'accepte !\n", pcr->pseudo_challengeur);
        PayloadChallengeResponse resp;
        resp.accepte = 1;
        resp.id_challengeur = pcr->id_challengeur;
        envoyer_message(sock, RES_CHALLENGE_ACCEPT, &resp, sizeof(resp));
        break;
    }

    case PUSH_FRIEND_REQUEST: {
        // on accepte automatiquement toutes les demandes d'amis
        PayloadFriendRequestReceived *prr = (PayloadFriendRequestReceived *)payload;
        printf("[BOT] Demande d'ami de %s, j'accepte !\n", prr->pseudo_demandeur);
        PayloadFriendResponse pfr;
        pfr.accepte = 1;
        pfr.id_demandeur = prr->id_demandeur;
        envoyer_message(sock, RES_FRIEND_REQUEST, &pfr, sizeof(pfr));
        break;
    }

    case RES_ERROR_STATE:
        printf("[BOT] Erreur recue du serveur.\n");
        break;

    default:
        break;
    }
    return 0;
}

// ── boucle principale ─────────────────────────────────────────────────────────
int main(int argc, char **argv) {
    srand((unsigned)time(NULL));

    if (argc < 2) {
        fprintf(stderr,
            "Usage : %s <IP_SERVEUR> [PORT] [MODE] [PSEUDO]\n"
            "  MODE : fill | random | win | idle\n", argv[0]);
        return 1;
    }

    const char *ip     = argv[1];
    int         port   = (argc >= 3) ? atoi(argv[2]) : SRV_PORT_DEF;
    const char *mode_s = (argc >= 4) ? argv[3] : "random";
    const char *pseudo = (argc >= 5) ? argv[4] : "TestBot";

    // selection du mode
    if      (strcmp(mode_s, "win")        == 0) g_mode = MODE_WIN;
    else if (strcmp(mode_s, "lose")       == 0) g_mode = MODE_LOSE;
    else if (strcmp(mode_s, "fill")       == 0) g_mode = MODE_FILL;
    else if (strcmp(mode_s, "overflow")   == 0) g_mode = MODE_OVERFLOW;
    else if (strcmp(mode_s, "idle")       == 0) g_mode = MODE_IDLE;
    else if (strcmp(mode_s, "tournament") == 0) g_mode = MODE_TOURNAMENT;
    else                                         g_mode = MODE_RANDOM;

    printf("[BOT] Mode : %s  |  Pseudo : %s  |  Serveur : %s:%d\n",
           mode_s, pseudo, ip, port);

    // connexion au serveur
    int sock = connecter_serveur(ip, port);
    if (sock < 0) {
        fprintf(stderr, "[BOT] Impossible de se connecter a %s:%d\n", ip, port);
        return 1;
    }
    printf("[BOT] Connecte au serveur.\n");

    // envoi du login
    PayloadLogin pl;
    memset(&pl, 0, sizeof(pl));
    strncpy(pl.pseudo, pseudo, 31);
    envoyer_message(sock, REQ_LOGIN, &pl, sizeof(pl));

    // boucle de reception
    while (1) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(sock, &rfds);
        struct timeval tv = {1, 0};

        if (select(sock + 1, &rfds, NULL, NULL, &tv) > 0) {
            Header h;
            void *payload = NULL;
            if (recevoir_message(sock, &h, &payload) < 0) {
                printf("[BOT] Connexion perdue.\n");
                break;
            }
            int ret = traiter(sock, &h, payload);
            if (payload) free(payload);
            if (ret < 0) break;
        }
    }

    fermer_socket(sock);
    return 0;
}
