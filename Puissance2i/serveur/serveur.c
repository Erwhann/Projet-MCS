/* =========================================================
 * serveur.c -- Serveur Puissance 2i
 * select() mono-processus, machine a etats, CDC ss7.2 / ss8 / ss9.1
 * Aucun accent pour portabilite ASCII.
 * ========================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include "../reseau/session.h"
#include "../reseau/data.h"
#include "../commun/protocol.h"
#include "../commun/structures.h"
#include "../commun/profil.h"
#include <sys/stat.h>
#include <dirent.h>
#include "jeu.h"
#include "elo.h"
#include "matchmaking.h"
#include "amis.h"

#define MAX_CLIENTS 50
#define MAX_PARTIES 25
#define SRV_PORT    5000

/* ── Donnees globales ── */
static ClientInfo clients[MAX_CLIENTS];
static PartieInfo parties[MAX_PARTIES];
static int next_client_id = 1;
static int next_partie_id = 1;

/* ── Utilitaires ── */

static int get_by_id(int id) {
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (clients[i].socket != 0 && clients[i].id == id) return i;
    return -1;
}

static int get_by_pseudo(const char *pseudo) {
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (clients[i].socket != 0 && strcmp(clients[i].pseudo, pseudo) == 0) return i;
    return -1;
}

static int get_partie(int id) {
    for (int i = 0; i < MAX_PARTIES; i++)
        if (parties[i].active && parties[i].id_partie == id) return i;
    return -1;
}

/* ── Deconnexion / forfait ── */

static void nettoyer_client(int i) {
    if (clients[i].socket == 0) return;
    printf("[SRV] '%s' (ID=%d) deconnecte.\n", clients[i].pseudo, clients[i].id);
    fermer_socket(clients[i].socket);

    if (clients[i].pseudo[0] != '\0') {
        char path[512];
        snprintf(path, sizeof(path), "profils/%s.dat", clients[i].pseudo);
        ProfilSauvegarde ps;
        memset(&ps, 0, sizeof(ps));
        charger_profil(path, &ps);
        
        strncpy(ps.pseudo, clients[i].pseudo, 31);
        ps.pseudo[31] = '\0';
        ps.elo = clients[i].elo;
        ps.score = clients[i].score;
        ps.nb_victoires = clients[i].nb_victoires;
        ps.nb_defaites = clients[i].nb_defaites;
        ps.nb_nuls = clients[i].nb_nuls;
        
        ps.nb_amis = clients[i].nb_amis;
        for(int a=0; a<clients[i].nb_amis; a++){
            ps.amis[a] = clients[i].amis[a];
        }
        sauvegarder_profil(path, &ps);
    }

    if (clients[i].id_partie_actuelle != 0) {
        int p = get_partie(clients[i].id_partie_actuelle);
        if (p != -1) {
            int adv_id = (parties[p].id_joueur1 == clients[i].id)
                         ? parties[p].id_joueur2 : parties[p].id_joueur1;
            int a = get_by_id(adv_id);
            if (a != -1) {
                PayloadEndGame peg;
                peg.id_vainqueur  = adv_id;
                peg.points_gagnes = 3;
                peg.nouvel_elo    = clients[a].elo + 20;
                clients[a].elo    = peg.nouvel_elo;
                clients[a].score += 3;
                clients[a].nb_victoires++;
                clients[a].etat = ETAT_MENU;
                clients[a].id_partie_actuelle = 0;
                envoyer_message(clients[a].socket, PUSH_ENDGAME, &peg, sizeof(peg));
            }
            parties[p].active = 0;
        }
    }

    clients[i].socket = 0;
    clients[i].etat   = ETAT_MENU;
    clients[i].id_partie_actuelle = 0;
}

/* ── Lancement d'une partie ── */

static void lancer_partie(int c1, int c2, int est_challenge) {
    int p = -1;
    for (int i = 0; i < MAX_PARTIES; i++) {
        if (!parties[i].active) { p = i; break; }
    }
    if (p == -1) { fprintf(stderr, "[SRV] Plus de slots de parties.\n"); return; }

    parties[p].active         = 1;
    parties[p].id_partie      = next_partie_id++;
    parties[p].id_joueur1     = clients[c1].id;
    parties[p].id_joueur2     = clients[c2].id;
    parties[p].tour_joueur_id = clients[c1].id;
    parties[p].elo_impact     = 1;          /* par defaut : partie classee */
    parties[p].elo_mode_pending = est_challenge; /* en attente si challenge */
    parties[p].id_challengeur   = est_challenge ? clients[c1].id : 0;
    init_grille(parties[p].grille);

    clients[c1].etat = ETAT_EN_JEU;
    clients[c2].etat = ETAT_EN_JEU;
    clients[c1].id_partie_actuelle = parties[p].id_partie;
    clients[c2].id_partie_actuelle = parties[p].id_partie;

    /* PUSH_MATCH_FOUND */
    PayloadMatchFound pmf;
    pmf.id_partie     = parties[p].id_partie;
    pmf.est_challenge = est_challenge;

    pmf.id_adversaire = clients[c2].id;
    pmf.elo           = clients[c2].elo;
    strncpy(pmf.pseudo, clients[c2].pseudo, 31); pmf.pseudo[31] = '\0';
    envoyer_message(clients[c1].socket, PUSH_MATCH_FOUND, &pmf, sizeof(pmf));

    pmf.id_adversaire = clients[c1].id;
    pmf.elo           = clients[c1].elo;
    strncpy(pmf.pseudo, clients[c1].pseudo, 31); pmf.pseudo[31] = '\0';
    envoyer_message(clients[c2].socket, PUSH_MATCH_FOUND, &pmf, sizeof(pmf));

    if (est_challenge) {
        /* Demander au challengeur (c1) de choisir le mode ELO avant le premier coup */
        envoyer_message(clients[c1].socket, PUSH_CHOOSE_ELO, NULL, 0);
    } else {
        /* Matchmaking : envoyer directement la grille */
        PayloadGrid pg;
        memcpy(pg.grille, parties[p].grille, sizeof(parties[p].grille));
        pg.tour_de_jeu = parties[p].tour_joueur_id;
        envoyer_message(clients[c1].socket, PUSH_GRID, &pg, sizeof(pg));
        envoyer_message(clients[c2].socket, PUSH_GRID, &pg, sizeof(pg));
        envoyer_message(clients[c1].socket, PUSH_TURN, NULL, 0);
        envoyer_message(clients[c2].socket, PUSH_TURN, NULL, 0);
    }

    printf("[SRV] Partie #%d | '%s' vs '%s' | challenge=%d\n",
           parties[p].id_partie, clients[c1].pseudo, clients[c2].pseudo, est_challenge);
}

/* ── Helpers fin de partie ── */

static void terminer_partie(int p, int id_vainqueur, int match_nul) {
    int c1 = get_by_id(parties[p].id_joueur1);
    int c2 = get_by_id(parties[p].id_joueur2);

    int nv_elo1 = (c1 != -1) ? clients[c1].elo : 1200;
    int nv_elo2 = (c2 != -1) ? clients[c2].elo : 1200;
    int delta1 = 0, delta2 = 0;

    if (parties[p].elo_impact) {
        int nv1, nv2;
        int elo1 = (c1 != -1) ? clients[c1].elo : 1200;
        int elo2 = (c2 != -1) ? clients[c2].elo : 1200;

        if (match_nul) {
            calculer_elo(elo1, elo2, &nv1, &nv2, 1);
        } else if (id_vainqueur == parties[p].id_joueur1) {
            calculer_elo(elo1, elo2, &nv1, &nv2, 0);
        } else {
            calculer_elo(elo2, elo1, &nv2, &nv1, 0);
        }
        delta1 = nv1 - elo1;
        delta2 = nv2 - elo2;
        nv_elo1 = nv1;
        nv_elo2 = nv2;
        if (c1 != -1) clients[c1].elo = nv_elo1;
        if (c2 != -1) clients[c2].elo = nv_elo2;
    }

    /* Points et statistiques */
    int pts1 = 0, pts2 = 0;
    if (match_nul) { pts1 = pts2 = 1; }
    else if (id_vainqueur == parties[p].id_joueur1) { pts1 = 3; }
    else { pts2 = 3; }

    if (c1 != -1) {
        clients[c1].score += pts1;
        if (match_nul)                              clients[c1].nb_nuls++;
        else if (pts1 == 3)                         clients[c1].nb_victoires++;
        else                                        clients[c1].nb_defaites++;
        clients[c1].etat = ETAT_MENU;
        clients[c1].id_partie_actuelle = 0;

        PayloadEndGame peg = {id_vainqueur, pts1, nv_elo1};
        (void)delta1;
        envoyer_message(clients[c1].socket, PUSH_ENDGAME, &peg, sizeof(peg));

        char path[512];
        snprintf(path, sizeof(path), "profils/%s.dat", clients[c1].pseudo);
        ProfilSauvegarde ps;
        memset(&ps, 0, sizeof(ps));
        charger_profil(path, &ps);
        strncpy(ps.pseudo, clients[c1].pseudo, 31);
        ps.pseudo[31] = '\0';
        ps.elo = clients[c1].elo;
        ps.score = clients[c1].score;
        ps.nb_victoires = clients[c1].nb_victoires;
        ps.nb_defaites = clients[c1].nb_defaites;
        ps.nb_nuls = clients[c1].nb_nuls;
        ps.nb_amis = clients[c1].nb_amis;
        for(int a=0; a<clients[c1].nb_amis; a++) ps.amis[a] = clients[c1].amis[a];
        sauvegarder_profil(path, &ps);
    }
    if (c2 != -1) {
        clients[c2].score += pts2;
        if (match_nul)                              clients[c2].nb_nuls++;
        else if (pts2 == 3)                         clients[c2].nb_victoires++;
        else                                        clients[c2].nb_defaites++;
        clients[c2].etat = ETAT_MENU;
        clients[c2].id_partie_actuelle = 0;

        PayloadEndGame peg = {id_vainqueur, pts2, nv_elo2};
        (void)delta2;
        envoyer_message(clients[c2].socket, PUSH_ENDGAME, &peg, sizeof(peg));

        char path[512];
        snprintf(path, sizeof(path), "profils/%s.dat", clients[c2].pseudo);
        ProfilSauvegarde ps;
        memset(&ps, 0, sizeof(ps));
        charger_profil(path, &ps);
        strncpy(ps.pseudo, clients[c2].pseudo, 31);
        ps.pseudo[31] = '\0';
        ps.elo = clients[c2].elo;
        ps.score = clients[c2].score;
        ps.nb_victoires = clients[c2].nb_victoires;
        ps.nb_defaites = clients[c2].nb_defaites;
        ps.nb_nuls = clients[c2].nb_nuls;
        ps.nb_amis = clients[c2].nb_amis;
        for(int a=0; a<clients[c2].nb_amis; a++) ps.amis[a] = clients[c2].amis[a];
        sauvegarder_profil(path, &ps);
    }
    parties[p].active = 0;
}

/* ── Traitement des messages ── */

static void traiter_message(int ci, Header *h, void *payload) {
    int sd = clients[ci].socket;

    /* === ETAT_MENU === */
    if (clients[ci].etat == ETAT_MENU) {

        if (h->type == REQ_LOGIN && h->payload_size == (int)sizeof(PayloadLogin)) {
            PayloadLogin *pl = (PayloadLogin *)payload;
            pl->pseudo[31] = '\0';
            if (get_by_pseudo(pl->pseudo) != -1) {
                PayloadError pe = {2};
                envoyer_message(sd, RES_LOGIN_ERROR, &pe, sizeof(pe));
                return;
            }
            strncpy(clients[ci].pseudo, pl->pseudo, 31);

            char path[512];
            snprintf(path, sizeof(path), "profils/%s.dat", clients[ci].pseudo);
            ProfilSauvegarde ps;
            memset(&ps, 0, sizeof(ps));
            if (charger_profil(path, &ps)) {
                clients[ci].elo = ps.elo;
                clients[ci].score = ps.score;
                clients[ci].nb_victoires = ps.nb_victoires;
                clients[ci].nb_defaites = ps.nb_defaites;
                clients[ci].nb_nuls = ps.nb_nuls;
                clients[ci].nb_amis = ps.nb_amis;
                for(int a=0; a<ps.nb_amis; a++){
                    clients[ci].amis[a] = ps.amis[a];
                }
            } else {
                clients[ci].elo = 1200;
                clients[ci].score = 0;
                clients[ci].nb_victoires = 0;
                clients[ci].nb_defaites = 0;
                clients[ci].nb_nuls = 0;
                clients[ci].nb_amis = 0;
            }

            PayloadLoginOK pok;
            pok.id_joueur    = clients[ci].id;
            pok.elo          = clients[ci].elo;
            pok.score        = clients[ci].score;
            pok.nb_victoires = clients[ci].nb_victoires;
            pok.nb_defaites  = clients[ci].nb_defaites;
            pok.nb_nuls      = clients[ci].nb_nuls;
            envoyer_message(sd, RES_LOGIN_OK, &pok, sizeof(pok));
            printf("[SRV] '%s' (ID=%d) connecte.\n", clients[ci].pseudo, clients[ci].id);
        }

        else if (h->type == REQ_MATCHMAKING) {
            clients[ci].etat = ETAT_MATCHMAKING;
            envoyer_message(sd, RES_WAITING, NULL, 0);
        }

        else if (h->type == REQ_CHANGE_STATE && h->payload_size == (int)sizeof(PayloadChangeState)) {
            PayloadChangeState *pcs = (PayloadChangeState *)payload;
            if (pcs->etat_social >= 0 && pcs->etat_social <= 2) {
                clients[ci].etat_social = (EtatSocial)pcs->etat_social;
                envoyer_message(sd, RES_WAITING, NULL, 0); /* ACK */
            }
        }

        else if (h->type == REQ_ADD_FRIEND) {
            /* Desactive : utiliser REQ_FRIEND_REQUEST a la place */
            PayloadError pe = {7};
            envoyer_message(sd, RES_ERROR_STATE, &pe, sizeof(pe));
        }

        else if (h->type == REQ_FRIEND_REQUEST && h->payload_size == (int)sizeof(PayloadFriendRequest)) {
            PayloadFriendRequest *pfr = (PayloadFriendRequest *)payload;
            pfr->pseudo_cible[31] = '\0';
            int a = get_by_pseudo(pfr->pseudo_cible);
            if (a == -1) {
                /* Joueur hors ligne ou inexistant */
                PayloadError pe = {4};
                envoyer_message(sd, RES_ERROR_STATE, &pe, sizeof(pe));
            } else if (a == ci) {
                PayloadError pe = {5};
                envoyer_message(sd, RES_ERROR_STATE, &pe, sizeof(pe));
            } else {
                /* Verifier si deja ami */
                int deja = 0;
                for (int k = 0; k < clients[ci].nb_amis; k++) {
                    if (clients[ci].amis[k] == clients[a].id) { deja = 1; break; }
                }
                if (deja) {
                    PayloadError pe = {5};
                    envoyer_message(sd, RES_ERROR_STATE, &pe, sizeof(pe));
                } else {
                    PayloadFriendRequestReceived prr;
                    prr.id_demandeur = clients[ci].id;
                    prr.elo_demandeur = clients[ci].elo;
                    strncpy(prr.pseudo_demandeur, clients[ci].pseudo, 31);
                    prr.pseudo_demandeur[31] = '\0';
                    envoyer_message(clients[a].socket, PUSH_FRIEND_REQUEST, &prr, sizeof(prr));
                }
            }
        }

        else if (h->type == RES_FRIEND_REQUEST && h->payload_size == (int)sizeof(PayloadFriendResponse)) {
            PayloadFriendResponse *pfr = (PayloadFriendResponse *)payload;
            int a = get_by_id(pfr->id_demandeur);
            if (pfr->accepte) {
                /* Ajouter mutuellement */
                ajouter_ami(&clients[ci], clients, MAX_CLIENTS, (a != -1) ? clients[a].pseudo : "");
                if (a != -1) ajouter_ami(&clients[a], clients, MAX_CLIENTS, clients[ci].pseudo);
                envoyer_message(sd, RES_FRIEND_ADDED, NULL, 0);
                if (a != -1) envoyer_message(clients[a].socket, RES_FRIEND_ADDED, NULL, 0);
            } else {
                /* Notifier le demandeur du refus */
                if (a != -1) {
                    PayloadError pe = {6};
                    envoyer_message(clients[a].socket, RES_ERROR_STATE, &pe, sizeof(pe));
                }
            }
        }

        else if (h->type == REQ_FRIEND_LIST) {
            PayloadFriendList pfl;
            construire_liste_amis(&clients[ci], clients, MAX_CLIENTS, &pfl);
            envoyer_message(sd, PUSH_FRIEND_LIST, &pfl, sizeof(pfl));
        }

        else if (h->type == REQ_REMOVE_FRIEND && h->payload_size == (int)sizeof(PayloadRemoveFriend)) {
            PayloadRemoveFriend *prf = (PayloadRemoveFriend *)payload;
            int ok = supprimer_ami(&clients[ci], prf->id_ami);
            if (ok) {
                // Supprimer aussi l'autre sens si l'ami est connecte
                int a = get_by_id(prf->id_ami);
                if (a != -1) supprimer_ami(&clients[a], clients[ci].id);
                envoyer_message(sd, RES_FRIEND_REMOVED, NULL, 0);
                // Mettre a jour les profils
                char path[512];
                snprintf(path, sizeof(path), "profils/%s.dat", clients[ci].pseudo);
                ProfilSauvegarde ps; memset(&ps, 0, sizeof(ps));
                charger_profil(path, &ps);
                strncpy(ps.pseudo, clients[ci].pseudo, 31);
                ps.elo = clients[ci].elo; ps.score = clients[ci].score;
                ps.nb_victoires = clients[ci].nb_victoires;
                ps.nb_defaites = clients[ci].nb_defaites;
                ps.nb_nuls = clients[ci].nb_nuls;
                ps.nb_amis = clients[ci].nb_amis;
                for (int k = 0; k < clients[ci].nb_amis; k++) ps.amis[k] = clients[ci].amis[k];
                sauvegarder_profil(path, &ps);
            } else {
                PayloadError pe = {1};
                envoyer_message(sd, RES_ERROR_STATE, &pe, sizeof(pe));
            }
        }

        else if (h->type == REQ_LEADERBOARD) {
            // Scanner tous les profils .dat et construire le classement ELO
            PayloadLeaderboard lb;
            memset(&lb, 0, sizeof(lb));

            /* Tableau temporaire pour le tri (max 200 profils) */
            #define MAX_PROFILS_SCAN 200
            ProfilSauvegarde tmp[MAX_PROFILS_SCAN];
            int nb_tmp = 0;

            DIR *d = opendir("profils");
            if (d) {
                struct dirent *ent;
                while ((ent = readdir(d)) != NULL && nb_tmp < MAX_PROFILS_SCAN) {
                    int len = (int)strlen(ent->d_name);
                    if (len > 4 && strcmp(ent->d_name + len - 4, ".dat") == 0) {
                        char path[512];
                        snprintf(path, sizeof(path), "profils/%s", ent->d_name);
                        ProfilSauvegarde ps;
                        memset(&ps, 0, sizeof(ps));
                        if (charger_profil(path, &ps)) {
                            tmp[nb_tmp++] = ps;
                        }
                    }
                }
                closedir(d);
            }

            /* Tri par ELO decroissant (tri bulle simple) */
            for (int x = 0; x < nb_tmp - 1; x++) {
                for (int y = 0; y < nb_tmp - 1 - x; y++) {
                    if (tmp[y].elo < tmp[y+1].elo) {
                        ProfilSauvegarde t = tmp[y];
                        tmp[y] = tmp[y+1];
                        tmp[y+1] = t;
                    }
                }
            }

            /* Remplir le payload (top MAX_CLASSEMENT) */
            lb.nb = (nb_tmp < MAX_CLASSEMENT) ? nb_tmp : MAX_CLASSEMENT;
            for (int x = 0; x < lb.nb; x++) {
                strncpy(lb.pseudo[x], tmp[x].pseudo, 31);
                lb.pseudo[x][31] = '\0';
                lb.elo[x]          = tmp[x].elo;
                lb.nb_victoires[x] = tmp[x].nb_victoires;
                lb.nb_defaites[x]  = tmp[x].nb_defaites;
                lb.nb_nuls[x]      = tmp[x].nb_nuls;
            }
            envoyer_message(sd, PUSH_LEADERBOARD, &lb, sizeof(lb));
        }

        else if (h->type == REQ_CHALLENGE && h->payload_size == (int)sizeof(PayloadChallenge)) {
            PayloadChallenge *pc = (PayloadChallenge *)payload;
            int a = get_by_id(pc->id_ami_cible);
            if (a != -1 && (clients[a].etat == ETAT_MENU || clients[a].etat == ETAT_MATCHMAKING)) {
                PayloadChallengeReceived pcr;
                pcr.id_challengeur  = clients[ci].id;
                pcr.elo_challengeur = clients[ci].elo;
                strncpy(pcr.pseudo_challengeur, clients[ci].pseudo, 31);
                pcr.pseudo_challengeur[31] = '\0';
                envoyer_message(clients[a].socket, PUSH_CHALLENGE, &pcr, sizeof(pcr));
            } else {
                PayloadError pe = {1};
                envoyer_message(sd, RES_ERROR_STATE, &pe, sizeof(pe));
            }
        }

        else if (h->type == RES_CHALLENGE_ACCEPT && h->payload_size == (int)sizeof(PayloadChallengeResponse)) {
            PayloadChallengeResponse *pcr = (PayloadChallengeResponse *)payload;
            int a = get_by_id(pcr->id_challengeur);
            if (a != -1) {
                if (pcr->accepte) {
                    /* c1 = challengeur, c2 = accepteur */
                    lancer_partie(a, ci, 1);
                } else {
                    PayloadError pe = {3};
                    envoyer_message(clients[a].socket, RES_ERROR_STATE, &pe, sizeof(pe));
                }
            }
        }
    }

    /* === ETAT_MATCHMAKING === */
    else if (clients[ci].etat == ETAT_MATCHMAKING) {
        if (h->type == REQ_CANCEL_MATCH) {
            clients[ci].etat = ETAT_MENU;
            envoyer_message(sd, RES_WAITING, NULL, 0);
        }
        else if (h->type == RES_CHALLENGE_ACCEPT && h->payload_size == (int)sizeof(PayloadChallengeResponse)) {
            PayloadChallengeResponse *pcr = (PayloadChallengeResponse *)payload;
            int a = get_by_id(pcr->id_challengeur);
            if (a != -1) {
                if (pcr->accepte) {
                    clients[ci].etat = ETAT_MENU;
                    if (clients[a].etat == ETAT_MATCHMAKING) clients[a].etat = ETAT_MENU;
                    lancer_partie(a, ci, 1);
                } else {
                    PayloadError pe = {3};
                    envoyer_message(clients[a].socket, RES_ERROR_STATE, &pe, sizeof(pe));
                }
            }
        }
    }

    /* === ETAT_EN_JEU === */
    else if (clients[ci].etat == ETAT_EN_JEU) {

        /* Choix du mode ELO (challenge uniquement, avant le premier coup) */
        if (h->type == REQ_SET_ELO_MODE && h->payload_size == (int)sizeof(PayloadSetEloMode)) {
            PayloadSetEloMode *psm = (PayloadSetEloMode *)payload;
            int p = get_partie(clients[ci].id_partie_actuelle);
            if (p != -1 && parties[p].elo_mode_pending && parties[p].id_challengeur == clients[ci].id) {
                parties[p].elo_impact      = psm->elo_impact;
                parties[p].elo_mode_pending = 0;

                /* Maintenant on peut envoyer la grille et le PUSH_TURN */
                int c1 = get_by_id(parties[p].id_joueur1);
                int c2 = get_by_id(parties[p].id_joueur2);
                PayloadGrid pg;
                memcpy(pg.grille, parties[p].grille, sizeof(parties[p].grille));
                pg.tour_de_jeu = parties[p].tour_joueur_id;
                if (c1 != -1) envoyer_message(clients[c1].socket, PUSH_GRID, &pg, sizeof(pg));
                if (c2 != -1) envoyer_message(clients[c2].socket, PUSH_GRID, &pg, sizeof(pg));
                if (c1 != -1) envoyer_message(clients[c1].socket, PUSH_TURN, NULL, 0);
                if (c2 != -1) envoyer_message(clients[c2].socket, PUSH_TURN, NULL, 0);
            }
            return;
        }

        if (h->type != REQ_MOVE || h->payload_size != (int)sizeof(PayloadMove)) return;

        PayloadMove *pm = (PayloadMove *)payload;
        int p = get_partie(clients[ci].id_partie_actuelle);
        if (p == -1) return;

        /* Filtrage : attente choix ELO avant premier coup */
        if (parties[p].elo_mode_pending) {
            PayloadError pe = {5};
            envoyer_message(sd, RES_MOVE_ERROR, &pe, sizeof(pe));
            return;
        }

        if (parties[p].tour_joueur_id != clients[ci].id) {
            PayloadError pe = {2};
            envoyer_message(sd, RES_MOVE_ERROR, &pe, sizeof(pe));
            return;
        }

        if (!jouer_coup(parties[p].grille, pm->colonne, clients[ci].id)) {
            PayloadError pe = {1};
            envoyer_message(sd, RES_MOVE_ERROR, &pe, sizeof(pe));
            return;
        }

        int adv_id = (parties[p].id_joueur1 == clients[ci].id)
                     ? parties[p].id_joueur2 : parties[p].id_joueur1;
        int a = get_by_id(adv_id);

        PayloadGrid pg;
        memcpy(pg.grille, parties[p].grille, sizeof(parties[p].grille));

        if (verifier_victoire(parties[p].grille, clients[ci].id)) {
            pg.tour_de_jeu = 0;
            envoyer_message(sd, PUSH_GRID, &pg, sizeof(pg));
            if (a != -1) envoyer_message(clients[a].socket, PUSH_GRID, &pg, sizeof(pg));
            terminer_partie(p, clients[ci].id, 0);
        } else if (verifier_match_nul(parties[p].grille)) {
            pg.tour_de_jeu = 0;
            envoyer_message(sd, PUSH_GRID, &pg, sizeof(pg));
            if (a != -1) envoyer_message(clients[a].socket, PUSH_GRID, &pg, sizeof(pg));
            terminer_partie(p, 0, 1);
        } else {
            parties[p].tour_joueur_id = adv_id;
            pg.tour_de_jeu = adv_id;
            envoyer_message(sd, PUSH_GRID, &pg, sizeof(pg));
            if (a != -1) envoyer_message(clients[a].socket, PUSH_GRID, &pg, sizeof(pg));
            envoyer_message(sd, PUSH_TURN, NULL, 0);
            if (a != -1) envoyer_message(clients[a].socket, PUSH_TURN, NULL, 0);
        }
    }
}

/* ── Boucle principale ── */

int main(void) {
    memset(clients, 0, sizeof(clients));
    memset(parties, 0, sizeof(parties));

    int server_fd = creer_serveur(SRV_PORT);
    printf("[SRV] Serveur Puissance 2i demarre sur le port %d\n", SRV_PORT);

    mkdir("profils", 0755);

    fd_set readfds;
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        int max_sd = server_fd;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket > 0) {
                FD_SET(clients[i].socket, &readfds);
                if (clients[i].socket > max_sd) max_sd = clients[i].socket;
            }
        }

        struct timeval tv = {0, 500000};
        if (select(max_sd + 1, &readfds, NULL, NULL, &tv) < 0) { perror("select"); continue; }

        if (FD_ISSET(server_fd, &readfds)) {
            struct sockaddr_in addr;
            int ns = accepter_client(server_fd, &addr);
            if (ns >= 0) {
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i].socket == 0) {
                        memset(&clients[i], 0, sizeof(ClientInfo));
                        clients[i].socket = ns;
                        clients[i].id     = next_client_id++;
                        clients[i].etat   = ETAT_MENU;
                        clients[i].elo    = 1200;
                        break;
                    }
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket > 0 && FD_ISSET(clients[i].socket, &readfds)) {
                Header h; void *p = NULL;
                if (recevoir_message(clients[i].socket, &h, &p) < 0) {
                    nettoyer_client(i);
                } else {
                    traiter_message(i, &h, p);
                    if (p) free(p);
                }
            }
        }

        /* Matchmaking automatique */
        int id1, id2;
        if (matchmake(clients, MAX_CLIENTS, &id1, &id2)) {
            int c1 = get_by_id(id1), c2 = get_by_id(id2);
            if (c1 != -1 && c2 != -1) {
                clients[c1].etat = ETAT_MENU;
                clients[c2].etat = ETAT_MENU;
                lancer_partie(c1, c2, 0);
            }
        }
    }
    return 0;
}
