
#include "ihm.h"
#include "../reseau/data.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static void bandeau(const char *titre) {
    attron(COLOR_PAIR(COL_TITRE) | A_BOLD);
    mvprintw(1, 2, "+=============================================+");
    mvprintw(2, 2, "|  %-43s|", titre);
    mvprintw(3, 2, "+=============================================+");
    attroff(COLOR_PAIR(COL_TITRE) | A_BOLD);
}

static void ligne(int y) {
    attron(COLOR_PAIR(COL_NORMAL));
    mvprintw(y, 2, "---------------------------------------------");
    attroff(COLOR_PAIR(COL_NORMAL));
}

static void ecran_effacer(void) {
    bkgd(COLOR_PAIR(COL_NORMAL));
    clear();
}

static void kv(int y, int x, const char *label, const char *val) {
    attron(COLOR_PAIR(COL_NORMAL));
    mvprintw(y, x, "%s", label);
    attroff(COLOR_PAIR(COL_NORMAL));
    attron(COLOR_PAIR(COL_ACCENT) | A_BOLD);
    printw("%s", val);
    attroff(COLOR_PAIR(COL_ACCENT) | A_BOLD);
}

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
        init_pair(COL_TITRE,   COLOR_CYAN,   COLOR_BLACK);
        init_pair(COL_NORMAL,  COLOR_WHITE,  COLOR_BLACK);
        init_pair(COL_ACCENT,  COLOR_YELLOW, COLOR_BLACK);
        init_pair(COL_VAINC,   COLOR_GREEN,  COLOR_BLACK);
        init_pair(COL_DEFAITE, COLOR_RED,    COLOR_BLACK);
        init_pair(COL_GRILLE,  COLOR_WHITE,  COLOR_BLUE);
        init_pair(COL_MOI,     COLOR_GREEN,  COLOR_BLUE);
        init_pair(COL_ADV,     COLOR_RED,    COLOR_BLUE);
        init_pair(COL_NOTIF,   COLOR_BLACK,  COLOR_YELLOW);
        bkgd(COLOR_PAIR(COL_NORMAL));
    }
}

void fin_ihm(void) {
    endwin();
}

void demander_pseudo_ncurses(char *pseudo, const char *suggestion) {
    timeout(-1);
    echo();
    curs_set(1);
    clear();
    bandeau("         PUISSANCE 2i  -  ONLINE        ");
    attron(COLOR_PAIR(COL_NORMAL));
    mvprintw(6, 4, "Bienvenue ! Entrez votre pseudo :");
    attroff(COLOR_PAIR(COL_NORMAL));

    if (suggestion && suggestion[0] != '\0') {
        attron(COLOR_PAIR(COL_ACCENT));
        mvprintw(7, 4, "(Profil : %s  -- ENTREE pour confirmer)", suggestion);
        attroff(COLOR_PAIR(COL_ACCENT));
    }

    attron(COLOR_PAIR(COL_ACCENT) | A_BOLD);
    mvprintw(9, 4, "> ");
    attroff(COLOR_PAIR(COL_ACCENT) | A_BOLD);
    refresh();
    getnstr(pseudo, 31);
    if (pseudo[0] == '\0' && suggestion && suggestion[0] != '\0')
        strncpy(pseudo, suggestion, 31);

    curs_set(0);
    noecho();
    timeout(50);
}

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

void dessiner_menu(const ClientInfo *moi) {
    const char *statuts[] = {"En ligne", "Occupe", "Absent"};
    int s = (int)moi->etat_social;
    if (s < 0 || s > 2) s = 0;

    ecran_effacer();
    bandeau("             MENU PRINCIPAL             ");
    char buf[32];
    kv(5, 4, "Joueur : ", moi->pseudo);
    snprintf(buf, sizeof(buf), "%d", moi->elo);
    kv(5, 20, "  ELO : ", buf);
    snprintf(buf, sizeof(buf), "%d", moi->score);
    kv(5, 32, "  Score : ", buf);

    attron(COLOR_PAIR(COL_ACCENT));
    mvprintw(6, 4, "Statut : %s", statuts[s]);
    attroff(COLOR_PAIR(COL_ACCENT));

    ligne(8);
    attron(COLOR_PAIR(COL_NORMAL));
    mvprintw(10, 6, " [1]  Chercher une partie  (Matchmaking ELO)");
    mvprintw(11, 6, " [2]  Liste d'amis & Defis");
    mvprintw(12, 6, " [3]  Changer mon statut");
    mvprintw(13, 6, " [4]  Consulter mon profil");
    mvprintw(14, 6, " [5]  Classement ELO");
    mvprintw(15, 6, " [0]  Quitter");
    attroff(COLOR_PAIR(COL_NORMAL));
    ligne(17);
    attron(COLOR_PAIR(COL_ACCENT));
    mvprintw(18, 4, "Votre choix :");
    attroff(COLOR_PAIR(COL_ACCENT));
    refresh();
}

void dessiner_matchmaking(void) {
    ecran_effacer();
    bandeau("             RECHERCHE DE PARTIE        ");
    attron(COLOR_PAIR(COL_ACCENT) | A_BOLD);
    mvprintw(6, 4, ">> Recherche d'un adversaire en cours...");
    mvprintw(7, 4, "   Merci de patienter.");
    attroff(COLOR_PAIR(COL_ACCENT) | A_BOLD);
    attron(COLOR_PAIR(COL_NORMAL));
    mvprintw(10, 4, "(Appuyez sur [c] pour annuler)");
    attroff(COLOR_PAIR(COL_NORMAL));
    refresh();
}

void dessiner_partie(const ClientInfo *moi, const PartieInfo *partie, const char *msg) {
    ecran_effacer();
    bandeau("              PARTIE EN COURS           ");

    attron(COLOR_PAIR(COL_NORMAL) | A_BOLD);
    mvprintw(5, 4, "  1   2   3   4   5   6   7");
    mvprintw(6, 4, "+---+---+---+---+---+---+---+");
    attroff(COLOR_PAIR(COL_NORMAL) | A_BOLD);

    for (int i = 0; i < 6; i++) {
        attron(COLOR_PAIR(COL_GRILLE) | A_BOLD);
        mvprintw(7 + i, 4, "|");
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
    attron(COLOR_PAIR(COL_MOI));  mvprintw(14, 4, " O = Vous");  attroff(COLOR_PAIR(COL_MOI));
    attron(COLOR_PAIR(COL_ADV));  printw("   X = Adversaire");   attroff(COLOR_PAIR(COL_ADV));

    if (!partie->elo_impact) {
        attron(COLOR_PAIR(COL_ACCENT));
        mvprintw(14, 35, "[Partie amicale - sans ELO]");
        attroff(COLOR_PAIR(COL_ACCENT));
    }

    ligne(16);
    attron(COLOR_PAIR(COL_ACCENT) | A_BOLD);
    mvprintw(17, 4, "%s", msg);
    attroff(COLOR_PAIR(COL_ACCENT) | A_BOLD);
    refresh();
}

void dessiner_fin_partie(const ClientInfo *moi, int id_vainqueur, int points, int nv_elo, int ancien_elo) {
    ecran_effacer();
    bandeau("               FIN DE PARTIE           ");

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
    kv(9,  4, "Points gagnes  : ", buf);
    snprintf(buf, sizeof(buf), "%d (%+d)", nv_elo, nv_elo - ancien_elo);
    kv(10, 4, "Nouvel ELO     : ", buf);

    ligne(12);
    attron(COLOR_PAIR(COL_NORMAL));
    mvprintw(13, 4, "Appuyez sur [ENTREE] ou [ESPACE] pour retourner.");
    attroff(COLOR_PAIR(COL_NORMAL));
    refresh();
}

void dessiner_amis(const PayloadFriendList *liste) {
    ecran_effacer();
    bandeau("         LISTE D'AMIS & DEFIS          ");
    const char *stat_labels[] = {"En ligne", "Occupe", "Absent"};

    if (liste->nb_amis == 0) {
        attron(COLOR_PAIR(COL_NORMAL));
        mvprintw(6, 4, "Vous n'avez pas encore d'amis.");
        attroff(COLOR_PAIR(COL_NORMAL));
    } else {
        attron(COLOR_PAIR(COL_NORMAL) | A_BOLD);
        mvprintw(5, 4, "  #  Pseudo              ELO    Statut");
        attroff(COLOR_PAIR(COL_NORMAL) | A_BOLD);
        ligne(6);
        for (int i = 0; i < liste->nb_amis && i < 14; i++) {
            int en_ligne = liste->en_ligne[i];
            int statut   = liste->statut[i];
            const char *slabel = stat_labels[(statut >= 0 && statut <= 2) ? statut : 0];

            if (en_ligne)
                attron(COLOR_PAIR(COL_VAINC));
            else
                attron(COLOR_PAIR(COL_NORMAL));
            mvprintw(7 + i, 4, " [%d] %-20s %4d   %s",
                     i + 1, liste->pseudos[i], liste->elo[i],
                     en_ligne ? slabel : "Hors ligne");
            if (en_ligne)
                attroff(COLOR_PAIR(COL_VAINC));
            else
                attroff(COLOR_PAIR(COL_NORMAL));
        }
    }

    ligne(22);
    attron(COLOR_PAIR(COL_ACCENT));
    mvprintw(23, 4, " [a] Ajouter  |  [s]+# Suppr  |  [r] Refresh  |  [d]+# Defier  |  [q] Retour");
    attroff(COLOR_PAIR(COL_ACCENT));
    refresh();
}

void dessiner_profil(const ClientInfo *moi) {
    ecran_effacer();
    bandeau("           MON PROFIL COMPLET          ");
    char buf[32];
    kv(5, 4, "Pseudo    : ", moi->pseudo);
    snprintf(buf, sizeof(buf), "%d", moi->id);
    kv(6, 4, "ID        : ", buf);
    ligne(8);
    snprintf(buf, sizeof(buf), "%d", moi->elo);
    kv(9,  4, "ELO       : ", buf);
    snprintf(buf, sizeof(buf), "%d", moi->score);
    kv(10, 4, "Score     : ", buf);
    ligne(12);
    attron(COLOR_PAIR(COL_VAINC));
    snprintf(buf, sizeof(buf), "%d", moi->nb_victoires);
    mvprintw(13, 4, "Victoires : %s", buf);
    attroff(COLOR_PAIR(COL_VAINC));
    attron(COLOR_PAIR(COL_DEFAITE));
    snprintf(buf, sizeof(buf), "%d", moi->nb_defaites);
    mvprintw(14, 4, "Defaites  : %s", buf);
    attroff(COLOR_PAIR(COL_DEFAITE));
    attron(COLOR_PAIR(COL_ACCENT));
    snprintf(buf, sizeof(buf), "%d", moi->nb_nuls);
    mvprintw(15, 4, "Nuls      : %s", buf);
    attroff(COLOR_PAIR(COL_ACCENT));
    ligne(17);
    attron(COLOR_PAIR(COL_NORMAL));
    mvprintw(18, 4, "Appuyez sur [q] ou [ENTREE] pour retourner.");
    attroff(COLOR_PAIR(COL_NORMAL));
    refresh();
}

void dessiner_statut(int statut_actuel) {
    const char *labels[] = {"En ligne", "Occupe", "Absent"};
    ecran_effacer();
    bandeau("           CHANGER MON STATUT          ");
    attron(COLOR_PAIR(COL_NORMAL));
    mvprintw(5, 4, "Statut actuel : ");
    attroff(COLOR_PAIR(COL_NORMAL));
    attron(COLOR_PAIR(COL_ACCENT) | A_BOLD);
    printw("%s", labels[(statut_actuel >= 0 && statut_actuel <= 2) ? statut_actuel : 0]);
    attroff(COLOR_PAIR(COL_ACCENT) | A_BOLD);
    ligne(7);
    attron(COLOR_PAIR(COL_NORMAL));
    mvprintw(9,  6, " [1]  En ligne");
    mvprintw(10, 6, " [2]  Occupe");
    mvprintw(11, 6, " [3]  Absent");
    mvprintw(12, 6, " [q]  Retour au menu");
    attroff(COLOR_PAIR(COL_NORMAL));
    refresh();
}

void dessiner_choisir_elo(void) {
    ecran_effacer();
    bandeau("           MODE DE LA PARTIE           ");
    attron(COLOR_PAIR(COL_ACCENT) | A_BOLD);
    mvprintw(6, 4, "Vous avez accepte un defi.");
    mvprintw(7, 4, "Voulez-vous que cette partie impacte le classement ELO ?");
    attroff(COLOR_PAIR(COL_ACCENT) | A_BOLD);
    ligne(9);
    attron(COLOR_PAIR(COL_NORMAL));
    mvprintw(11, 6, " [o]  Oui - Partie classee  (ELO modifie)");
    mvprintw(12, 6, " [n]  Non - Partie amicale  (ELO conserve)");
    attroff(COLOR_PAIR(COL_NORMAL));
    refresh();
}

void dessiner_classement(const PayloadLeaderboard *lb) {
    ecran_effacer();
    bandeau("           CLASSEMENT ELO SERVEUR      ");

    attron(COLOR_PAIR(COL_NORMAL) | A_BOLD);
    mvprintw(5, 4, "  #  Pseudo              ELO    V   D   N");
    attroff(COLOR_PAIR(COL_NORMAL) | A_BOLD);
    ligne(6);

    for (int i = 0; i < lb->nb && i < 14; i++) {
        if (i == 0)      attron(COLOR_PAIR(COL_VAINC)  | A_BOLD);
        else if (i == 1) attron(COLOR_PAIR(COL_ACCENT) | A_BOLD);
        else if (i == 2) attron(COLOR_PAIR(COL_DEFAITE)| A_BOLD);
        else             attron(COLOR_PAIR(COL_NORMAL));

        mvprintw(7 + i, 4, " %2d. %-20s %4d  %3d %3d %3d",
                 i + 1,
                 lb->pseudo[i],
                 lb->elo[i],
                 lb->nb_victoires[i],
                 lb->nb_defaites[i],
                 lb->nb_nuls[i]);

        if (i == 0)      attroff(COLOR_PAIR(COL_VAINC)  | A_BOLD);
        else if (i == 1) attroff(COLOR_PAIR(COL_ACCENT) | A_BOLD);
        else if (i == 2) attroff(COLOR_PAIR(COL_DEFAITE)| A_BOLD);
        else             attroff(COLOR_PAIR(COL_NORMAL));
    }

    ligne(22);
    attron(COLOR_PAIR(COL_ACCENT));
    mvprintw(23, 4, " [q] ou [ENTREE] pour retourner au menu");
    attroff(COLOR_PAIR(COL_ACCENT));
    refresh();
}

void dessiner_notification(const char *msg) {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    (void)cols;
    attron(COLOR_PAIR(COL_NOTIF) | A_BOLD);
    mvprintw(rows - 2, 2, " [!] %-60s ", msg);
    attroff(COLOR_PAIR(COL_NOTIF) | A_BOLD);
    refresh();
}

void traiter_message_serveur(Header *h, void *payload, ClientInfo *moi,
                             PartieInfo *partie, PayloadFriendList *amis,
                             int *etat_ihm, int *challenge_en_attente,
                             int *challenger_id)
{
    switch (h->type) {

    case RES_LOGIN_OK: {
        PayloadLoginOK *p = (PayloadLoginOK *)payload;
        moi->id           = p->id_joueur;
        moi->elo          = p->elo;
        moi->score        = p->score;
        moi->nb_victoires = p->nb_victoires;
        moi->nb_defaites  = p->nb_defaites;
        moi->nb_nuls      = p->nb_nuls;
        *etat_ihm  = IHM_MENU;
        dessiner_menu(moi);
        break;
    }

    case RES_WAITING:
        if (*etat_ihm == IHM_MATCHMAKING || *etat_ihm == IHM_STATUT) {
            *etat_ihm = IHM_MENU;
            dessiner_menu(moi);
        } else {
            *etat_ihm = IHM_MATCHMAKING;
            dessiner_matchmaking();
        }
        break;

    case PUSH_MATCH_FOUND: {
        PayloadMatchFound *p = (PayloadMatchFound *)payload;
        partie->id_partie   = p->id_partie;
        partie->elo_impact  = 1;
        moi->etat = ETAT_EN_JEU;
        char notif[80];
        snprintf(notif, sizeof(notif), "Adversaire : %s (ELO %d) !", p->pseudo, p->elo);
        dessiner_notification(notif);
        break;
    }

    case PUSH_CHOOSE_ELO:
        *etat_ihm = IHM_CHOISIR_ELO;
        dessiner_choisir_elo();
        break;

    case PUSH_GRID: {
        PayloadGrid *pg = (PayloadGrid *)payload;
        memcpy(partie->grille, pg->grille, sizeof(partie->grille));
        partie->tour_joueur_id = pg->tour_de_jeu;
        moi->etat = ETAT_EN_JEU;
        *etat_ihm = IHM_EN_JEU;
        char msg[80];
        if (partie->tour_joueur_id == moi->id)
            snprintf(msg, sizeof(msg), "C'est a VOUS de jouer !  Colonne (1-7) :");
        else
            snprintf(msg, sizeof(msg), "Tour de l'adversaire... En attente.");
        dessiner_partie(moi, partie, msg);
        break;
    }

    case PUSH_TURN: {
        char msg[80];
        if (partie->tour_joueur_id == moi->id)
            snprintf(msg, sizeof(msg), "C'est a VOUS de jouer !  Colonne (1-7) :");
        else
            snprintf(msg, sizeof(msg), "Tour de l'adversaire... En attente.");
        dessiner_partie(moi, partie, msg);
        break;
    }

    case RES_MOVE_ERROR:
        dessiner_partie(moi, partie, "Erreur : colonne pleine ou invalide.  Rejouez (1-7) :");
        break;

    case PUSH_ENDGAME: {
        PayloadEndGame *pe = (PayloadEndGame *)payload;
        int ancien_elo = moi->elo;
        moi->elo    = pe->nouvel_elo;
        moi->score += pe->points_gagnes;
        if (pe->id_vainqueur == moi->id) moi->nb_victoires++;
        else if (pe->id_vainqueur == 0)  moi->nb_nuls++;
        else                             moi->nb_defaites++;
        moi->etat = ETAT_MENU;
        *etat_ihm = IHM_FIN_PARTIE;
        dessiner_fin_partie(moi, pe->id_vainqueur, pe->points_gagnes, pe->nouvel_elo, ancien_elo);
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
        dessiner_notification("Ami ajoute avec succes !");
        break;

    case RES_FRIEND_REMOVED:
        dessiner_notification("Ami supprime. Rafraichissez avec [r].");
        break;

    case PUSH_LEADERBOARD: {
        PayloadLeaderboard *lb = (PayloadLeaderboard *)payload;
        *etat_ihm = IHM_CLASSEMENT;
        dessiner_classement(lb);
        break;
    }

    case PUSH_FRIEND_REQUEST: {
        PayloadFriendRequestReceived *prr = (PayloadFriendRequestReceived *)payload;
        *challenge_en_attente = 2;          /* 2 = demande d'ami (vs 1 = defi) */
        *challenger_id = prr->id_demandeur;
        char notif[96];
        snprintf(notif, sizeof(notif),
                 "[Ami] %s (ELO %d) veut vous ajouter -- Accepter ? [o/n] :",
                 prr->pseudo_demandeur, prr->elo_demandeur);
        dessiner_notification(notif);
        break;
    }

    case PUSH_CHALLENGE: {
        PayloadChallengeReceived *pcr = (PayloadChallengeReceived *)payload;
        *challenge_en_attente = 1;
        *challenger_id = pcr->id_challengeur;
        char notif[80];
        snprintf(notif, sizeof(notif),
                 "Defi de %s (ELO %d) -- Accepter ? [o/n] :",
                 pcr->pseudo_challengeur, pcr->elo_challengeur);
        dessiner_notification(notif);
        break;
    }

    case RES_ERROR_STATE:
        if (payload) {
            PayloadError *pe = (PayloadError *)payload;
            if (pe->code_erreur == 3)
                dessiner_notification("Votre defi a ete refuse.");
            else if (pe->code_erreur == 4)
                dessiner_notification("Erreur : joueur hors ligne ou introuvable.");
            else if (pe->code_erreur == 5)
                dessiner_notification("Erreur : deja ami ou meme joueur.");
            else if (pe->code_erreur == 6)
                dessiner_notification("Demande d'ami refusee.");
            else
                dessiner_notification("Erreur : joueur introuvable ou indisponible.");
        }
        break;

    default:
        break;
    }
}

void traiter_saisie(int ch, ClientInfo *moi, PartieInfo *partie,
                    PayloadFriendList *amis, int *etat_ihm,
                    int *challenge_en_attente, int challenger_id, int sock)
{
    (void)partie;

    if (*challenge_en_attente && (ch == 'o' || ch == 'O' || ch == 'n' || ch == 'N')) {
        if (*challenge_en_attente == 1) {
            PayloadChallengeResponse pcr;
            pcr.accepte        = (ch == 'o' || ch == 'O') ? 1 : 0;
            pcr.id_challengeur = challenger_id;
            envoyer_message(sock, RES_CHALLENGE_ACCEPT, &pcr, sizeof(pcr));
        } else if (*challenge_en_attente == 2) {
            PayloadFriendResponse pfr;
            pfr.accepte      = (ch == 'o' || ch == 'O') ? 1 : 0;
            pfr.id_demandeur = challenger_id;
            envoyer_message(sock, RES_FRIEND_REQUEST, &pfr, sizeof(pfr));
        }
        *challenge_en_attente = 0;
        return;
    }

    switch (*etat_ihm) {

    case IHM_MENU:
        switch (ch) {
        case '1':
            moi->etat = ETAT_MATCHMAKING;
            envoyer_message(sock, REQ_MATCHMAKING, NULL, 0);
            break;
        case '2':
            envoyer_message(sock, REQ_FRIEND_LIST, NULL, 0);
            break;
        case '3':
            *etat_ihm = IHM_STATUT;
            dessiner_statut((int)moi->etat_social);
            break;
        case '4':
            *etat_ihm = IHM_PROFIL;
            dessiner_profil(moi);
            break;
        case '5':
            envoyer_message(sock, REQ_LEADERBOARD, NULL, 0);
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
            char pseudo_ami[32] = {0};
            if (saisir_chaine_overlay("Pseudo de l'ami a ajouter :", pseudo_ami, 32)) {
                PayloadFriendRequest pfr;
                strncpy(pfr.pseudo_cible, pseudo_ami, 31);
                pfr.pseudo_cible[31] = '\0';
                envoyer_message(sock, REQ_FRIEND_REQUEST, &pfr, sizeof(pfr));
            }
            dessiner_amis(amis);
        }
        else if (ch == 'r' || ch == 'R') {
            envoyer_message(sock, REQ_FRIEND_LIST, NULL, 0);
        }
        else if ((ch == 's' || ch == 'S') && amis->nb_amis > 0) {
            dessiner_notification("Numero de l'ami a supprimer (1-N) :");
            timeout(-1);
            int c2 = getch();
            timeout(50);
            if (c2 >= '1' && c2 <= '9') {
                int idx = c2 - '1';
                if (idx < amis->nb_amis) {
                    PayloadRemoveFriend prf;
                    prf.id_ami = amis->ids[idx];
                    envoyer_message(sock, REQ_REMOVE_FRIEND, &prf, sizeof(prf));
                }
            }
        }
        else if ((ch == 'd' || ch == 'D') && amis->nb_amis > 0) {
            dessiner_notification("Numero de l'ami a defier (1-N) :");
            timeout(-1);
            int c2 = getch();
            timeout(50); 
            if (c2 >= '1' && c2 <= '9') {
                int idx = c2 - '1';
                if (idx < amis->nb_amis && amis->en_ligne[idx]) {
                    PayloadChallenge pc;
                    pc.id_ami_cible = amis->ids[idx];
                    envoyer_message(sock, REQ_CHALLENGE, &pc, sizeof(pc));
                    char notif[60];
                    snprintf(notif, sizeof(notif), "Defi envoye a %s !", amis->pseudos[idx]);
                    dessiner_notification(notif);
                } else {
                    dessiner_notification("Cet ami est hors ligne.");
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

    case IHM_STATUT:
        if (ch == '1' || ch == '2' || ch == '3') {
            int nouveau = ch - '1'; 
            moi->etat_social = (EtatSocial)nouveau;
            PayloadChangeState pcs;
            pcs.etat_social = nouveau;
            envoyer_message(sock, REQ_CHANGE_STATE, &pcs, sizeof(pcs));
        } else if (ch == 'q' || ch == 'Q') {
            *etat_ihm = IHM_MENU;
            dessiner_menu(moi);
        }
        break;

    case IHM_CHOISIR_ELO:
        if (ch == 'o' || ch == 'O') {
            PayloadSetEloMode psm; psm.elo_impact = 1;
            partie->elo_impact = 1;
            envoyer_message(sock, REQ_SET_ELO_MODE, &psm, sizeof(psm));
        } else if (ch == 'n' || ch == 'N') {
            PayloadSetEloMode psm; psm.elo_impact = 0;
            partie->elo_impact = 0;
            envoyer_message(sock, REQ_SET_ELO_MODE, &psm, sizeof(psm));
        }
        break;
    case IHM_CLASSEMENT:
        if (ch == 'q' || ch == 'Q' || ch == '\n' || ch == KEY_ENTER) {
            *etat_ihm = IHM_MENU;
            dessiner_menu(moi);
        }
        break;


    default:
        break;
    }
}
