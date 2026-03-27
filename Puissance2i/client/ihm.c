/* =========================================================
 * ihm.c -- Interface NCURSES coloriée pour le client Puissance 2i
 *
 * Chaque dessiner_*() efface l'écran (clear/refresh) et repose sur
 * des color_pair définis dans init_ihm(). La saisie clavier est
 * non-bloquante (timeout 50 ms) pour permettre la réception async
 * des messages serveur (CDC §6).
 * ========================================================= */

#include "ihm.h"
#include "../reseau/data.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* ================================================================
 * Helpers internes
 * ================================================================ */

/* Imprime le titre centre dans une boite de 47 chars */
static void afficher_bandeau(const char *titre) {
    attron(COLOR_PAIR(COL_TITRE) | A_BOLD);
    mvprintw(1, 2, "+=============================================+");
    mvprintw(2, 2, "|  %-43s|", titre);
    mvprintw(3, 2, "+=============================================+");
    attroff(COLOR_PAIR(COL_TITRE) | A_BOLD);
}

static void afficher_ligne(int y) {
    attron(COLOR_PAIR(COL_NORMAL));
    mvprintw(y, 2, "---------------------------------------------");
    attroff(COLOR_PAIR(COL_NORMAL));
}

/* Imprime un label + valeur avec accent */
static void kv(int y, int x, const char *label, const char *val) {
    attron(COLOR_PAIR(COL_NORMAL));
    mvprintw(y, x, "%s", label);
    attroff(COLOR_PAIR(COL_NORMAL));
    attron(COLOR_PAIR(COL_ACCENT) | A_BOLD);
    printw("%s", val);
    attroff(COLOR_PAIR(COL_ACCENT) | A_BOLD);
}

/* ================================================================
 * Initialisation / arrêt
 * ================================================================ */

void init_ihm(void) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    timeout(50);
    curs_set(0);

    if (has_colors()) {
        start_color();
        use_default_colors();
        /* Paires : (fg, bg) */
        init_pair(COL_TITRE,   COLOR_CYAN,    COLOR_BLACK);
        init_pair(COL_NORMAL,  COLOR_WHITE,   COLOR_BLACK);
        init_pair(COL_ACCENT,  COLOR_YELLOW,  COLOR_BLACK);
        init_pair(COL_VAINC,   COLOR_GREEN,   COLOR_BLACK);
        init_pair(COL_DEFAITE, COLOR_RED,     COLOR_BLACK);
        init_pair(COL_GRILLE,  COLOR_WHITE,   COLOR_BLUE);
        init_pair(COL_MOI,     COLOR_GREEN,   COLOR_BLUE);
        init_pair(COL_ADV,     COLOR_RED,     COLOR_BLUE);
        init_pair(COL_NOTIF,   COLOR_BLACK,   COLOR_YELLOW);
    }
}

void fin_ihm(void) {
    endwin();
}

/* ================================================================
 * Saisie du pseudo (bloquante, avant la boucle async)
 * ================================================================ */

void demander_pseudo_ncurses(char *pseudo, const char *pseudo_suggestion) {
    timeout(-1);
    echo();
    curs_set(1);
    clear();
    afficher_bandeau("         PUISSANCE 2i  –  ONLINE         ");
    attron(COLOR_PAIR(COL_NORMAL));
    mvprintw(6, 4, "Bienvenue ! Entrez votre pseudonyme :");
    attroff(COLOR_PAIR(COL_NORMAL));

    if (pseudo_suggestion && pseudo_suggestion[0] != '\0') {
        attron(COLOR_PAIR(COL_ACCENT));
        mvprintw(7, 4, "(Profil trouvé : %s — ENTRÉE pour confirmer)", pseudo_suggestion);
        attroff(COLOR_PAIR(COL_ACCENT));
    }

    attron(COLOR_PAIR(COL_ACCENT) | A_BOLD);
    mvprintw(9, 4, "> ");
    attroff(COLOR_PAIR(COL_ACCENT) | A_BOLD);
    refresh();
    getnstr(pseudo, 31);
    /* Si l'utilisateur a juste appuyé sur ENTRÉE et qu'un profil existe, conserver le pseudo */
    if (pseudo[0] == '\0' && pseudo_suggestion && pseudo_suggestion[0] != '\0') {
        strncpy(pseudo, pseudo_suggestion, 31);
    }
    curs_set(0);
    noecho();
    timeout(50);
}

/* ================================================================
 * Saisir une chaîne en overlay (ecran existant conservé)
 * ================================================================ */

int saisir_chaine_overlay(const char *invite, char *buf, int maxlen) {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    (void)cols;

    timeout(-1);
    echo();
    curs_set(1);

    attron(COLOR_PAIR(COL_NOTIF) | A_BOLD);
    mvprintw(rows - 3, 2, "%-60s", invite);
    mvprintw(rows - 2, 2, "> ");
    attroff(COLOR_PAIR(COL_NOTIF) | A_BOLD);
    refresh();

    char tmp[64] = {0};
    getnstr(tmp, maxlen - 1);
    strncpy(buf, tmp, maxlen - 1);
    buf[maxlen - 1] = '\0';

    curs_set(0);
    noecho();
    timeout(50);

    return (buf[0] != '\0') ? 1 : 0;
}

/* ================================================================
 * 1. Menu principal
 * ================================================================ */

void dessiner_menu(const ClientInfo *moi) {
    clear();
    afficher_bandeau("             MENU PRINCIPAL             ");
    char statut[80];
    snprintf(statut, sizeof(statut), "%s", moi->pseudo);
    kv(5, 4, "Joueur : ", statut);
    char elo_s[16]; snprintf(elo_s, sizeof(elo_s), "%d", moi->elo);
    kv(5, 20, "  ELO : ", elo_s);
    char sc_s[16]; snprintf(sc_s, sizeof(sc_s), "%d", moi->score);
    kv(5, 34, "  Score : ", sc_s);

    afficher_ligne(7);
    attron(COLOR_PAIR(COL_NORMAL));
    mvprintw(9,  6, " [1]  Chercher une partie  (Matchmaking ELO)");
    mvprintw(10, 6, " [2]  Voir la liste d'amis & Defis");
    mvprintw(11, 6, " [4]  Consulter mon profil complet");
    mvprintw(12, 6, " [0]  Quitter");
    attroff(COLOR_PAIR(COL_NORMAL));
    afficher_ligne(14);

    attron(COLOR_PAIR(COL_ACCENT));
    mvprintw(15, 4, "Votre choix :");
    attroff(COLOR_PAIR(COL_ACCENT));
    refresh();
}

/* ================================================================
 * 2. Matchmaking
 * ================================================================ */

void dessiner_matchmaking(const ClientInfo *moi) {
    (void)moi;
    clear();
    afficher_bandeau("             RECHERCHE DE PARTIE        ");

    attron(COLOR_PAIR(COL_ACCENT) | A_BOLD);
    mvprintw(6, 4, ">> Recherche d'un adversaire de niveau proche...");
    mvprintw(7, 4, "   Merci de patienter.");
    attroff(COLOR_PAIR(COL_ACCENT) | A_BOLD);

    attron(COLOR_PAIR(COL_NORMAL));
    mvprintw(10, 4, "(Appuyez sur [c] pour annuler la recherche)");
    attroff(COLOR_PAIR(COL_NORMAL));
    refresh();
}

/* ================================================================
 * 3. Partie en cours (grille colorée)
 * ================================================================ */

void dessiner_partie(const ClientInfo *moi, const PartieInfo *partie, const char *msg) {
    clear();
    afficher_bandeau("              PARTIE EN COURS           ");

    /* Numeros de colonnes */
    attron(COLOR_PAIR(COL_NORMAL) | A_BOLD);
    mvprintw(5, 4, "  1   2   3   4   5   6   7");
    mvprintw(6, 4, "+---+---+---+---+---+---+---+");
    attroff(COLOR_PAIR(COL_NORMAL) | A_BOLD);

    for (int i = 0; i < 6; i++) {
        int y = 7 + i;
        attron(COLOR_PAIR(COL_GRILLE) | A_BOLD);
        mvprintw(y, 4, "|");
        for (int j = 0; j < 7; j++) {
            int v = partie->grille[i][j];
            if (v == moi->id) {
                attroff(COLOR_PAIR(COL_GRILLE) | A_BOLD);
                attron(COLOR_PAIR(COL_MOI) | A_BOLD);
                printw(" O ");
                attroff(COLOR_PAIR(COL_MOI) | A_BOLD);
                attron(COLOR_PAIR(COL_GRILLE) | A_BOLD);
                printw("|");
            } else if (v != 0) {
                attroff(COLOR_PAIR(COL_GRILLE) | A_BOLD);
                attron(COLOR_PAIR(COL_ADV) | A_BOLD);
                printw(" X ");
                attroff(COLOR_PAIR(COL_ADV) | A_BOLD);
                attron(COLOR_PAIR(COL_GRILLE) | A_BOLD);
                printw("|");
            } else {
                printw("   |");
            }
        }
        attroff(COLOR_PAIR(COL_GRILLE) | A_BOLD);
    }

    attron(COLOR_PAIR(COL_NORMAL) | A_BOLD);
    mvprintw(13, 4, "+---+---+---+---+---+---+---+");
    attroff(COLOR_PAIR(COL_NORMAL) | A_BOLD);

    attron(COLOR_PAIR(COL_MOI));
    mvprintw(14, 4, " O = Vous");
    attroff(COLOR_PAIR(COL_MOI));
    attron(COLOR_PAIR(COL_ADV));
    printw("    X = Adversaire");
    attroff(COLOR_PAIR(COL_ADV));

    afficher_ligne(16);
    attron(COLOR_PAIR(COL_ACCENT) | A_BOLD);
    mvprintw(17, 4, "%s", msg);
    attroff(COLOR_PAIR(COL_ACCENT) | A_BOLD);
    refresh();
}

/* ================================================================
 * 4. Fin de partie
 * ================================================================ */

void dessiner_fin_partie(const ClientInfo *moi, int id_vainqueur, int points, int nv_elo) {
    clear();
    afficher_bandeau("               FIN DE PARTIE           ");

    if (id_vainqueur == moi->id) {
        attron(COLOR_PAIR(COL_VAINC) | A_BOLD);
        mvprintw(6, 10, "*** VICTOIRE ! ***");
        mvprintw(7,  4, "Vous avez aligne 4 jetons !");
        attroff(COLOR_PAIR(COL_VAINC) | A_BOLD);
    } else if (id_vainqueur == 0) {
        attron(COLOR_PAIR(COL_ACCENT) | A_BOLD);
        mvprintw(6, 10, "--- MATCH NUL ---");
        attroff(COLOR_PAIR(COL_ACCENT) | A_BOLD);
    } else {
        attron(COLOR_PAIR(COL_DEFAITE) | A_BOLD);
        mvprintw(6, 10, "    defaite...  ");
        attroff(COLOR_PAIR(COL_DEFAITE) | A_BOLD);
    }

    char buf[32];
    snprintf(buf, sizeof(buf), "%+d pt(s)", points);
    kv(9,  4, "Points gagnés  : ", buf);
    snprintf(buf, sizeof(buf), "%d (%+d)", nv_elo, nv_elo - moi->elo);
    kv(10, 4, "Nouvel ELO     : ", buf);

    afficher_ligne(12);
    attron(COLOR_PAIR(COL_NORMAL));
    mvprintw(13, 4, "Appuyez sur [ENTRÉE] ou [ESPACE] pour retourner au Menu.");
    attroff(COLOR_PAIR(COL_NORMAL));
    refresh();
}

/* ================================================================
 * 5. Liste d'amis
 * ================================================================ */

void dessiner_amis(const PayloadFriendList *liste) {
    clear();
    afficher_bandeau("         LISTE D'AMIS & DÉFIS          ");

    if (liste->nb_amis == 0) {
        attron(COLOR_PAIR(COL_NORMAL));
        mvprintw(6, 4, "Vous n'avez pas encore d'amis.");
        attroff(COLOR_PAIR(COL_NORMAL));
    } else {
        attron(COLOR_PAIR(COL_NORMAL) | A_BOLD);
        mvprintw(5, 4, "  #  Pseudo                 ELO    Statut");
        attroff(COLOR_PAIR(COL_NORMAL) | A_BOLD);
        afficher_ligne(6);

        for (int i = 0; i < liste->nb_amis && i < 14; i++) {
            int en_ligne = liste->en_ligne[i];
            if (en_ligne)
                attron(COLOR_PAIR(COL_VAINC));
            else
                attron(COLOR_PAIR(COL_NORMAL));
            mvprintw(7 + i, 4, " [%d] %-22s %4d   %s",
                     i + 1, liste->pseudos[i], liste->elo[i],
                     en_ligne ? "En ligne " : "Hors ligne");
            if (en_ligne)
                attroff(COLOR_PAIR(COL_VAINC));
            else
                attroff(COLOR_PAIR(COL_NORMAL));
        }
    }

    afficher_ligne(22);
    attron(COLOR_PAIR(COL_ACCENT));
    mvprintw(23, 4, " [a] Ajouter un ami  |  [d]+chiffre Defier  |  [q] Retour");
    attroff(COLOR_PAIR(COL_ACCENT));
    refresh();
}

/* ================================================================
 * 6. Profil complet
 * ================================================================ */

void dessiner_profil(const ClientInfo *moi) {
    clear();
    afficher_bandeau("           MON PROFIL COMPLET          ");

    char buf[32];
    kv(5,  4, "Pseudo    : ", moi->pseudo);
    snprintf(buf, sizeof(buf), "%d", moi->id);
    kv(6,  4, "ID        : ", buf);
    afficher_ligne(8);

    snprintf(buf, sizeof(buf), "%d", moi->elo);
    kv(9,  4, "ELO       : ", buf);
    snprintf(buf, sizeof(buf), "%d", moi->score);
    kv(10, 4, "Score     : ", buf);
    afficher_ligne(12);

    attron(COLOR_PAIR(COL_VAINC));
    snprintf(buf, sizeof(buf), "%d", moi->nb_victoires);
    mvprintw(13, 4, "Victoires : %s", buf);
    attroff(COLOR_PAIR(COL_VAINC));

    attron(COLOR_PAIR(COL_DEFAITE));
    snprintf(buf, sizeof(buf), "%d", moi->nb_defaites);
    mvprintw(14, 4, "Défaites  : %s", buf);
    attroff(COLOR_PAIR(COL_DEFAITE));

    attron(COLOR_PAIR(COL_ACCENT));
    snprintf(buf, sizeof(buf), "%d", moi->nb_nuls);
    mvprintw(15, 4, "Nuls      : %s", buf);
    attroff(COLOR_PAIR(COL_ACCENT));

    afficher_ligne(17);
    attron(COLOR_PAIR(COL_NORMAL));
    mvprintw(18, 4, "Appuyez sur [q] ou [ENTRÉE] pour retourner.");
    attroff(COLOR_PAIR(COL_NORMAL));
    refresh();
}

/* ================================================================
 * 7. Notification en bande inférieure
 * ================================================================ */

void dessiner_notification(const char *msg) {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    (void)cols;

    attron(COLOR_PAIR(COL_NOTIF) | A_BOLD);
    mvprintw(rows - 2, 2, " [!] %-60s ", msg);
    attroff(COLOR_PAIR(COL_NOTIF) | A_BOLD);
    refresh();
}

/* ================================================================
 * traiter_message_serveur
 * ================================================================ */

void traiter_message_serveur(Header *h, void *payload, ClientInfo *moi,
                             PartieInfo *partie, PayloadFriendList *amis,
                             int *etat_ihm, int *challenge_en_attente,
                             int *challenger_id)
{
    switch (h->type) {

    case RES_LOGIN_OK: {
        PayloadLoginOK *p = (PayloadLoginOK *)payload;
        moi->id    = p->id_joueur;
        moi->elo   = p->elo;
        moi->score = p->score;
        *etat_ihm  = IHM_MENU;
        dessiner_menu(moi);
        break;
    }

    case RES_WAITING:
        if (*etat_ihm == IHM_MATCHMAKING) {
            *etat_ihm = IHM_MENU;
            dessiner_menu(moi);
        } else {
            *etat_ihm = IHM_MATCHMAKING;
            dessiner_matchmaking(moi);
        }
        break;

    case PUSH_MATCH_FOUND: {
        PayloadMatchFound *p = (PayloadMatchFound *)payload;
        partie->id_partie = p->id_partie;
        moi->etat = ETAT_EN_JEU;
        char notif[80];
        snprintf(notif, sizeof(notif), "Adversaire trouvé : %s (ELO %d) !", p->pseudo, p->elo);
        dessiner_notification(notif);
        break;
    }

    case PUSH_GRID: {
        PayloadGrid *pg = (PayloadGrid *)payload;
        memcpy(partie->grille, pg->grille, sizeof(partie->grille));
        partie->tour_joueur_id = pg->tour_de_jeu;
        moi->etat = ETAT_EN_JEU;
        *etat_ihm = IHM_EN_JEU;
        char msg[80];
        if (partie->tour_joueur_id == moi->id)
            snprintf(msg, sizeof(msg), "C'est à VOUS de jouer !  Choisissez une colonne (1-7) :");
        else
            snprintf(msg, sizeof(msg), "C'est au tour de l'adversaire...  En attente.");
        dessiner_partie(moi, partie, msg);
        break;
    }

    case PUSH_TURN: {
        char msg[80];
        if (partie->tour_joueur_id == moi->id)
            snprintf(msg, sizeof(msg), "C'est à VOUS de jouer !  Choisissez une colonne (1-7) :");
        else
            snprintf(msg, sizeof(msg), "C'est au tour de l'adversaire...  En attente.");
        dessiner_partie(moi, partie, msg);
        break;
    }

    case RES_MOVE_ERROR:
        dessiner_partie(moi, partie, "Erreur : colonne pleine ou saisie incorrecte.  Rejouez (1-7) :");
        break;

    case PUSH_ENDGAME: {
        PayloadEndGame *pe = (PayloadEndGame *)payload;
        moi->elo    = pe->nouvel_elo;
        moi->score += pe->points_gagnes;
        if (pe->id_vainqueur == moi->id) moi->nb_victoires++;
        else if (pe->id_vainqueur == 0)  moi->nb_nuls++;
        else                             moi->nb_defaites++;
        moi->etat = ETAT_MENU;
        *etat_ihm = IHM_FIN_PARTIE;
        dessiner_fin_partie(moi, pe->id_vainqueur, pe->points_gagnes, pe->nouvel_elo);
        break;
    }

    case PUSH_FRIEND_LIST: {
        PayloadFriendList *pfl = (PayloadFriendList *)payload;
        memcpy(amis, pfl, sizeof(PayloadFriendList));
        *etat_ihm = IHM_AMIS;
        dessiner_amis(amis);
        break;
    }

    case RES_FRIEND_ADDED:
        dessiner_notification("Ami ajouté avec succès !");
        break;

    case PUSH_CHALLENGE: {
        PayloadChallengeReceived *pcr = (PayloadChallengeReceived *)payload;
        *challenge_en_attente = 1;
        *challenger_id = pcr->id_challengeur;
        char notif[80];
        snprintf(notif, sizeof(notif),
                 "Défi de %s (ELO %d) — Accepter ? [o/n] :",
                 pcr->pseudo_challengeur, pcr->elo_challengeur);
        dessiner_notification(notif);
        break;
    }

    case RES_ERROR_STATE: {
        if (payload) {
            PayloadError *pe = (PayloadError *)payload;
            if (pe->code_erreur == 3)
                dessiner_notification("Votre défi a été refusé.");
            else
                dessiner_notification("Erreur serveur (joueur introuvable ou indisponible).");
        }
        break;
    }

    default:
        break;
    }
}

/* ================================================================
 * traiter_saisie
 * ================================================================ */

void traiter_saisie(int ch, ClientInfo *moi, PartieInfo *partie,
                    PayloadFriendList *amis, int *etat_ihm,
                    int *challenge_en_attente, int challenger_id, int sock)
{
    (void)partie; /* la PartieInfo est mise à jour via les messages serveur */

    /* Défi en attente : priorité absolue */
    if (*challenge_en_attente && (ch == 'o' || ch == 'O' || ch == 'n' || ch == 'N')) {
        PayloadChallengeResponse pcr;
        pcr.accepte = (ch == 'o' || ch == 'O') ? 1 : 0;
        pcr.id_challengeur = challenger_id;
        envoyer_message(sock, RES_CHALLENGE_ACCEPT, &pcr, sizeof(pcr));
        *challenge_en_attente = 0;
        return;
    }

    switch (*etat_ihm) {

    case IHM_MENU:
        switch (ch) {
        case '1':
            moi->etat = ETAT_MATCHMAKING;
            envoyer_message(sock, REQ_MATCHMAKING, NULL, 0);
            /* L'écran sera dessiné à RES_WAITING */
            break;
        case '2':
            envoyer_message(sock, REQ_FRIEND_LIST, NULL, 0);
            break;
        case '4':
            *etat_ihm = IHM_PROFIL;
            dessiner_profil(moi);
            break;
        case '0':
            fin_ihm();
            exit(0);
        default: break;
        }
        break;

    case IHM_MATCHMAKING:
        if (ch == 'c' || ch == 'C') {
            moi->etat = ETAT_MENU;
            envoyer_message(sock, REQ_CANCEL_MATCH, NULL, 0);
        }
        break;

    case IHM_EN_JEU:
        if (ch >= '1' && ch <= '7') {
            PayloadMove pm;
            pm.colonne = ch - '1';
            envoyer_message(sock, REQ_MOVE, &pm, sizeof(pm));
        }
        break;

    case IHM_FIN_PARTIE:
        if (ch == '\n' || ch == ' ' || ch == KEY_ENTER) {
            moi->id_partie_actuelle = 0;
            *etat_ihm = IHM_MENU;
            dessiner_menu(moi);
        }
        break;

    case IHM_AMIS:
        if (ch == 'q' || ch == 'Q') {
            *etat_ihm = IHM_MENU;
            dessiner_menu(moi);
        }
        else if (ch == 'a' || ch == 'A') {
            /* Saisir le pseudo de l'ami à ajouter */
            char pseudo_ami[32] = {0};
            if (saisir_chaine_overlay("Pseudo de l'ami à ajouter :", pseudo_ami, 32)) {
                PayloadAddFriend paf;
                strncpy(paf.pseudo_ami, pseudo_ami, 31);
                paf.pseudo_ami[31] = '\0';
                envoyer_message(sock, REQ_ADD_FRIEND, &paf, sizeof(paf));
            }
            /* Redessiner l'écran amis pour effacer le bandeau de saisie */
            dessiner_amis(amis);
        }
        else if ((ch == 'd' || ch == 'D') && amis->nb_amis > 0) {
            /* Défier un ami : attendre un chiffre */
            dessiner_notification("Numéro de l'ami à défier (1-N) :");
            int c2 = getch();
            if (c2 >= '1' && c2 <= '9') {
                int idx = c2 - '1';
                if (idx < amis->nb_amis && amis->en_ligne[idx]) {
                    PayloadChallenge pc;
                    pc.id_ami_cible = amis->ids[idx];
                    envoyer_message(sock, REQ_CHALLENGE, &pc, sizeof(pc));
                    char notif[60];
                    snprintf(notif, sizeof(notif), "Défi envoyé à %s !", amis->pseudos[idx]);
                    dessiner_notification(notif);
                } else {
                    dessiner_notification("Cet ami est hors ligne ou introuvable.");
                }
            }
        }
        break;

    case IHM_PROFIL:
        if (ch == 'q' || ch == 'Q' || ch == '\n' || ch == KEY_ENTER) {
            *etat_ihm = IHM_MENU;
            dessiner_menu(moi);
        }
        break;

    default:
        break;
    }
}
