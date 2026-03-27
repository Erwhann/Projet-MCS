/* =========================================================
 * client.c -- Programme principal du client Puissance 2i
 *
 * Usage : ./client <IP> [PORT] [profil.dat]
 *   IP        : adresse IP du serveur (obligatoire)
 *   PORT      : port TCP (défaut 5000)
 *   profil.dat: fichier de persistance du profil (optionnel)
 *
 * Boucle asynchrone : select() réseau + getch() non-bloquant ncurses.
 * Le profil est sauvegardé à la fermeture propre de l'application.
 * ========================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>

#include "../reseau/session.h"
#include "../reseau/data.h"
#include "../commun/protocol.h"
#include "../commun/structures.h"
#include "../commun/profil.h"
#include "ihm.h"

#define SRV_PORT_DEF 5000

/* Chemin du fichier de profil (global pour sauvegarde à la sortie) */
static char g_chemin_profil[256] = {0};
static ClientInfo g_moi;

/* Convertit ClientInfo -> ProfilSauvegarde et écrit sur disque */
static void sauvegarder(void) {
    if (g_chemin_profil[0] == '\0') return;
    ProfilSauvegarde ps;
    memset(&ps, 0, sizeof(ps));
    strncpy(ps.pseudo, g_moi.pseudo, 31);
    ps.elo         = g_moi.elo;
    ps.score       = g_moi.score;
    ps.nb_victoires = g_moi.nb_victoires;
    ps.nb_defaites  = g_moi.nb_defaites;
    ps.nb_nuls      = g_moi.nb_nuls;
    ps.nb_amis      = g_moi.nb_amis;
    for (int i = 0; i < g_moi.nb_amis && i < MAX_AMIS; i++)
        ps.amis[i] = g_moi.amis[i];
    sauvegarder_profil(g_chemin_profil, &ps);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage : %s <IP_SERVEUR> [PORT] [profil.dat]\n", argv[0]);
        return 1;
    }
    const char *ip   = argv[1];
    int         port = (argc >= 3) ? atoi(argv[2]) : SRV_PORT_DEF;
    const char *fichier_profil = (argc >= 4) ? argv[3] : NULL;

    /* ── 1. Charger le profil sauvegardé (si fourni) ── */
    ProfilSauvegarde ps_sauve;
    memset(&ps_sauve, 0, sizeof(ps_sauve));
    int profil_charge = 0;
    if (fichier_profil) {
        strncpy(g_chemin_profil, fichier_profil, 255);
        profil_charge = charger_profil(fichier_profil, &ps_sauve);
    }

    /* ── 2. Connexion au serveur ── */
    int sock = connecter_serveur(ip, port);
    if (sock < 0) {
        fprintf(stderr, "Erreur : Serveur inaccessible (%s:%d). Fermeture.\n", ip, port);
        return 1;
    }

    /* ── 3. Init NCURSES ── */
    init_ihm();

    /* ── 4. Saisie du pseudo (en suggérant le pseudo sauvegardé) ── */
    char pseudo[32] = {0};
    demander_pseudo_ncurses(pseudo, profil_charge ? ps_sauve.pseudo : "");
    if (pseudo[0] == '\0') {
        if (profil_charge) strncpy(pseudo, ps_sauve.pseudo, 31);
        else { fin_ihm(); fprintf(stderr, "Pseudo vide.\n"); return 1; }
    }

    /* ── 5. Envoi REQ_LOGIN ── */
    PayloadLogin pl;
    strncpy(pl.pseudo, pseudo, 31); pl.pseudo[31] = '\0';
    envoyer_message(sock, REQ_LOGIN, &pl, sizeof(pl));

    /* ── 6. Attente RES_LOGIN_OK (bloquant) ── */
    Header h; void *payload = NULL;
    if (recevoir_message(sock, &h, &payload) < 0 || h.type != RES_LOGIN_OK) {
        fin_ihm();
        fprintf(stderr, "Connexion refusée (pseudo déjà utilisé ?). Fermeture.\n");
        if (payload) free(payload);
        fermer_socket(sock);
        return 1;
    }

    /* ── 7. Initialiser le ClientInfo local ── */
    memset(&g_moi, 0, sizeof(g_moi));
    strncpy(g_moi.pseudo, pseudo, 31);
    g_moi.etat = ETAT_MENU;

    if (h.type == RES_LOGIN_OK) {
        PayloadLoginOK *pok = (PayloadLoginOK *)payload;
        g_moi.id    = pok->id_joueur;
        /* Priorité au profil sauvegardé si disponible */
        g_moi.elo   = profil_charge ? ps_sauve.elo   : pok->elo;
        g_moi.score = profil_charge ? ps_sauve.score  : pok->score;
        if (profil_charge) {
            g_moi.nb_victoires = ps_sauve.nb_victoires;
            g_moi.nb_defaites  = ps_sauve.nb_defaites;
            g_moi.nb_nuls      = ps_sauve.nb_nuls;
            g_moi.nb_amis      = ps_sauve.nb_amis;
            for (int i = 0; i < ps_sauve.nb_amis && i < MAX_AMIS; i++)
                g_moi.amis[i] = ps_sauve.amis[i];
        }
    }
    if (payload) { free(payload); payload = NULL; }

    /* ── 8. Affichage menu + boucle ── */
    PartieInfo       partie_courante     = {0};
    PayloadFriendList amis               = {0};
    int etat_ihm              = IHM_MENU;
    int challenge_en_attente  = 0;
    int challenger_id         = 0;

    dessiner_menu(&g_moi);

    while (1) {
        /* 8a. Réseau non-bloquant (select tv=0) */
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(sock, &rfds);
        struct timeval tv = {0, 0};
        if (select(sock + 1, &rfds, NULL, NULL, &tv) > 0) {
            Header nh; void *np = NULL;
            if (recevoir_message(sock, &nh, &np) < 0) {
                dessiner_notification("Connexion perdue avec le serveur.");
                break;
            }
            traiter_message_serveur(&nh, np, &g_moi, &partie_courante, &amis,
                                    &etat_ihm, &challenge_en_attente, &challenger_id);
            if (np) free(np);
        }

        /* 8b. Clavier non-bloquant (timeout 50 ms) */
        int ch = getch();
        if (ch != ERR) {
            traiter_saisie(ch, &g_moi, &partie_courante, &amis,
                           &etat_ihm, &challenge_en_attente, challenger_id, sock);
        }
    }

    /* ── 9. Sauvegarde propre ── */
    sauvegarder();

    fin_ihm();
    fermer_socket(sock);
    return 0;
}
