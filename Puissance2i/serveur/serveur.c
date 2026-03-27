/* =========================================================
 * serveur.c -- Serveur Puissance 2i
 *
 * Architecture : single-process, select() multiplexing (POSIX).
 * Machine à états par client : ETAT_MENU, ETAT_MATCHMAKING, ETAT_EN_JEU.
 * Conformément au CDC §7.2, §8, §9.1.
 * ========================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include "../reseau/session.h"
#include "../reseau/data.h"
#include "../commun/protocol.h"
#include "../commun/structures.h"
#include "jeu.h"
#include "elo.h"
#include "matchmaking.h"
#include "amis.h"

#define MAX_CLIENTS 50
#define MAX_PARTIES 25
#define SRV_PORT    5000

/* ── Données globales du serveur ── */
static ClientInfo clients[MAX_CLIENTS];
static PartieInfo parties[MAX_PARTIES];
static int next_client_id = 1;
static int next_partie_id = 1;

/* ── Utilitaires ── */

static int get_client_by_id(int id) {
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (clients[i].socket != 0 && clients[i].id == id) return i;
    return -1;
}

static int get_client_by_pseudo(const char *pseudo) {
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (clients[i].socket != 0 && strcmp(clients[i].pseudo, pseudo) == 0) return i;
    return -1;
}

static int get_partie_by_id(int id) {
    for (int i = 0; i < MAX_PARTIES; i++)
        if (parties[i].active && parties[i].id_partie == id) return i;
    return -1;
}

/* ── Gestion déconnexion / forfait ── */

static void nettoyer_client(int i) {
    if (clients[i].socket == 0) return;

    printf("[SRV] Client '%s' (ID=%d) déconnecté.\n", clients[i].pseudo, clients[i].id);
    fermer_socket(clients[i].socket);

    /* Forfait si en jeu */
    if (clients[i].id_partie_actuelle != 0) {
        int p = get_partie_by_id(clients[i].id_partie_actuelle);
        if (p != -1) {
            int adv_id = (parties[p].id_joueur1 == clients[i].id)
                         ? parties[p].id_joueur2 : parties[p].id_joueur1;
            int a = get_client_by_id(adv_id);
            if (a != -1) {
                PayloadEndGame peg;
                peg.id_vainqueur = adv_id;
                peg.points_gagnes = 3;
                /* ELO : gain simple de 20 points en cas de forfait */
                peg.nouvel_elo = clients[a].elo + 20;
                clients[a].elo = peg.nouvel_elo;
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

static void lancer_partie(int c1, int c2) {
    /* Trouver un slot libre */
    int p = -1;
    for (int i = 0; i < MAX_PARTIES; i++) {
        if (!parties[i].active) { p = i; break; }
    }
    if (p == -1) {
        fprintf(stderr, "[SRV] Plus de slots de parties disponibles.\n");
        return;
    }

    parties[p].active         = 1;
    parties[p].id_partie      = next_partie_id++;
    parties[p].id_joueur1     = clients[c1].id;
    parties[p].id_joueur2     = clients[c2].id;
    parties[p].tour_joueur_id = clients[c1].id; /* Joueur 1 commence */
    init_grille(parties[p].grille);

    clients[c1].etat              = ETAT_EN_JEU;
    clients[c2].etat              = ETAT_EN_JEU;
    clients[c1].id_partie_actuelle = parties[p].id_partie;
    clients[c2].id_partie_actuelle = parties[p].id_partie;

    /* PUSH_MATCH_FOUND */
    PayloadMatchFound pmf;
    pmf.id_partie     = parties[p].id_partie;
    pmf.id_adversaire = clients[c2].id;
    pmf.elo           = clients[c2].elo;
    strncpy(pmf.pseudo, clients[c2].pseudo, 31);
    pmf.pseudo[31] = '\0';
    envoyer_message(clients[c1].socket, PUSH_MATCH_FOUND, &pmf, sizeof(pmf));

    pmf.id_adversaire = clients[c1].id;
    pmf.elo           = clients[c1].elo;
    strncpy(pmf.pseudo, clients[c1].pseudo, 31);
    pmf.pseudo[31] = '\0';
    envoyer_message(clients[c2].socket, PUSH_MATCH_FOUND, &pmf, sizeof(pmf));

    /* PUSH_GRID (grille vide) */
    PayloadGrid pg;
    memcpy(pg.grille, parties[p].grille, sizeof(parties[p].grille));
    pg.tour_de_jeu = parties[p].tour_joueur_id;
    envoyer_message(clients[c1].socket, PUSH_GRID, &pg, sizeof(pg));
    envoyer_message(clients[c2].socket, PUSH_GRID, &pg, sizeof(pg));

    /* PUSH_TURN */
    envoyer_message(clients[c1].socket, PUSH_TURN, NULL, 0);
    envoyer_message(clients[c2].socket, PUSH_TURN, NULL, 0);

    printf("[SRV] Partie #%d lancée : '%s' vs '%s'\n",
           parties[p].id_partie, clients[c1].pseudo, clients[c2].pseudo);
}

/* ── Traitement des messages selon l'état du client ── */

static void traiter_message(int ci, Header *h, void *payload) {
    int sd = clients[ci].socket;

    /* === ETAT_MENU === */
    if (clients[ci].etat == ETAT_MENU) {

        if (h->type == REQ_LOGIN && h->payload_size == (int)sizeof(PayloadLogin)) {
            PayloadLogin *pl = (PayloadLogin *)payload;
            pl->pseudo[31] = '\0';

            /* Vérifier doublon de pseudo */
            if (get_client_by_pseudo(pl->pseudo) != -1) {
                PayloadError pe = {2};
                envoyer_message(sd, RES_LOGIN_ERROR, &pe, sizeof(pe));
                return;
            }
            strncpy(clients[ci].pseudo, pl->pseudo, 31);
            PayloadLoginOK pok = {clients[ci].id, clients[ci].elo, clients[ci].score};
            envoyer_message(sd, RES_LOGIN_OK, &pok, sizeof(pok));
            printf("[SRV] '%s' (ID=%d) connecté.\n", clients[ci].pseudo, clients[ci].id);
        }

        else if (h->type == REQ_MATCHMAKING) {
            clients[ci].etat = ETAT_MATCHMAKING;
            envoyer_message(sd, RES_WAITING, NULL, 0);
            printf("[SRV] '%s' entre en matchmaking.\n", clients[ci].pseudo);
        }

        else if (h->type == REQ_ADD_FRIEND && h->payload_size == (int)sizeof(PayloadAddFriend)) {
            PayloadAddFriend *paf = (PayloadAddFriend *)payload;
            paf->pseudo_ami[31] = '\0';
            int ok = ajouter_ami(&clients[ci], clients, MAX_CLIENTS, paf->pseudo_ami);
            if (ok) {
                /* Réciprocité */
                int a = get_client_by_pseudo(paf->pseudo_ami);
                if (a != -1) ajouter_ami(&clients[a], clients, MAX_CLIENTS, clients[ci].pseudo);
                envoyer_message(sd, RES_FRIEND_ADDED, NULL, 0);
                printf("[SRV] '%s' a ajouté '%s' en ami.\n", clients[ci].pseudo, paf->pseudo_ami);
            } else {
                PayloadError pe = {1};
                envoyer_message(sd, RES_ERROR_STATE, &pe, sizeof(pe));
            }
        }

        else if (h->type == REQ_FRIEND_LIST) {
            PayloadFriendList pfl;
            construire_liste_amis(&clients[ci], clients, MAX_CLIENTS, &pfl);
            envoyer_message(sd, PUSH_FRIEND_LIST, &pfl, sizeof(pfl));
        }

        else if (h->type == REQ_CHALLENGE && h->payload_size == (int)sizeof(PayloadChallenge)) {
            PayloadChallenge *pc = (PayloadChallenge *)payload;
            int a = get_client_by_id(pc->id_ami_cible);
            if (a != -1 && clients[a].etat == ETAT_MENU) {
                /* Notifier l'ami */
                PayloadChallengeReceived pcr;
                pcr.id_challengeur = clients[ci].id;
                pcr.elo_challengeur = clients[ci].elo;
                strncpy(pcr.pseudo_challengeur, clients[ci].pseudo, 31);
                pcr.pseudo_challengeur[31] = '\0';
                envoyer_message(clients[a].socket, PUSH_CHALLENGE, &pcr, sizeof(pcr));
            } else {
                PayloadError pe = {1}; /* Ami indisponible */
                envoyer_message(sd, RES_ERROR_STATE, &pe, sizeof(pe));
            }
        }

        else if (h->type == RES_CHALLENGE_ACCEPT && h->payload_size == (int)sizeof(PayloadChallengeResponse)) {
            PayloadChallengeResponse *pcr = (PayloadChallengeResponse *)payload;
            int a = get_client_by_id(pcr->id_challengeur);
            if (a != -1) {
                if (pcr->accepte) {
                    /* Démarrer la partie directement */
                    lancer_partie(a, ci);
                } else {
                    /* Notifier le challengeur du refus */
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
            envoyer_message(sd, RES_WAITING, NULL, 0); /* Réutilisé comme ACK annulation */
            printf("[SRV] '%s' annule le matchmaking.\n", clients[ci].pseudo);
        }
        /* Pendant l'attente, accepter aussi les défis entrants */
        else if (h->type == RES_CHALLENGE_ACCEPT && h->payload_size == (int)sizeof(PayloadChallengeResponse)) {
            PayloadChallengeResponse *pcr = (PayloadChallengeResponse *)payload;
            int a = get_client_by_id(pcr->id_challengeur);
            if (a != -1) {
                if (pcr->accepte) {
                    /* Retirer du matchmaking et lancer la partie */
                    clients[ci].etat = ETAT_MENU;
                    if (clients[a].etat == ETAT_MATCHMAKING) clients[a].etat = ETAT_MENU;
                    lancer_partie(a, ci);
                } else {
                    PayloadError pe = {3};
                    envoyer_message(clients[a].socket, RES_ERROR_STATE, &pe, sizeof(pe));
                }
            }
        }
    }

    /* === ETAT_EN_JEU === */
    else if (clients[ci].etat == ETAT_EN_JEU) {
        if (h->type != REQ_MOVE || h->payload_size != (int)sizeof(PayloadMove)) {
            /* Toute autre requête en jeu est ignorée (CDC §9 : filtrage croisé) */
            return;
        }

        PayloadMove *pm = (PayloadMove *)payload;
        int p = get_partie_by_id(clients[ci].id_partie_actuelle);
        if (p == -1) return;

        PartieInfo *partie = &parties[p];

        /* Vérification de l'autorité (CDC §9 : "c'est bien son tour") */
        if (partie->tour_joueur_id != clients[ci].id) {
            PayloadError pe = {2}; /* Pas ton tour */
            envoyer_message(sd, RES_MOVE_ERROR, &pe, sizeof(pe));
            return;
        }

        if (!jouer_coup(partie->grille, pm->colonne, clients[ci].id)) {
            PayloadError pe = {1}; /* Colonne pleine / invalide */
            envoyer_message(sd, RES_MOVE_ERROR, &pe, sizeof(pe));
            return;
        }

        /* Coup valide – diffuser la grille */
        int adv_id = (partie->id_joueur1 == clients[ci].id) ? partie->id_joueur2 : partie->id_joueur1;
        int a      = get_client_by_id(adv_id);

        PayloadGrid pg;
        memcpy(pg.grille, partie->grille, sizeof(partie->grille));

        if (verifier_victoire(partie->grille, clients[ci].id)) {
            pg.tour_de_jeu = 0;
            envoyer_message(sd, PUSH_GRID, &pg, sizeof(pg));
            if (a != -1) envoyer_message(clients[a].socket, PUSH_GRID, &pg, sizeof(pg));

            /* Calcul ELO */
            int nv_v, nv_p;
            int elo_p = (a != -1) ? clients[a].elo : 1200;
            calculer_elo(clients[ci].elo, elo_p, &nv_v, &nv_p, 0);
            clients[ci].elo = nv_v;
            clients[ci].score += 3;
            clients[ci].nb_victoires++;
            if (a != -1) { clients[a].elo = nv_p; clients[a].nb_defaites++; }

            /* Envoyer PUSH_ENDGAME */
            PayloadEndGame peg;
            peg.id_vainqueur = clients[ci].id;
            peg.points_gagnes = 3;
            peg.nouvel_elo = nv_v;
            envoyer_message(sd, PUSH_ENDGAME, &peg, sizeof(peg));

            if (a != -1) {
                peg.points_gagnes = 0;
                peg.nouvel_elo = nv_p;
                envoyer_message(clients[a].socket, PUSH_ENDGAME, &peg, sizeof(peg));
                clients[a].etat = ETAT_MENU;
                clients[a].id_partie_actuelle = 0;
            }
            clients[ci].etat = ETAT_MENU;
            clients[ci].id_partie_actuelle = 0;
            partie->active = 0;

        } else if (verifier_match_nul(partie->grille)) {
            pg.tour_de_jeu = 0;
            envoyer_message(sd, PUSH_GRID, &pg, sizeof(pg));
            if (a != -1) envoyer_message(clients[a].socket, PUSH_GRID, &pg, sizeof(pg));

            int nv_v, nv_p;
            int elo_p = (a != -1) ? clients[a].elo : 1200;
            calculer_elo(clients[ci].elo, elo_p, &nv_v, &nv_p, 1);
            clients[ci].elo = nv_v;
            clients[ci].score += 1;
            clients[ci].nb_nuls++;
            if (a != -1) { clients[a].elo = nv_p; clients[a].score += 1; clients[a].nb_nuls++; }

            PayloadEndGame peg = {0, 1, nv_v};
            envoyer_message(sd, PUSH_ENDGAME, &peg, sizeof(peg));

            if (a != -1) {
                peg.nouvel_elo = nv_p;
                envoyer_message(clients[a].socket, PUSH_ENDGAME, &peg, sizeof(peg));
                clients[a].etat = ETAT_MENU;
                clients[a].id_partie_actuelle = 0;
            }
            clients[ci].etat = ETAT_MENU;
            clients[ci].id_partie_actuelle = 0;
            partie->active = 0;

        } else {
            /* La partie continue : inverser le tour */
            partie->tour_joueur_id = adv_id;
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
    printf("[SRV] Serveur Puissance 2i démarré sur le port %d\n", SRV_PORT);

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

        struct timeval tv = {0, 500000}; /* 500ms – permet le matchmaking périodique */
        int activity = select(max_sd + 1, &readfds, NULL, NULL, &tv);
        if (activity < 0) { perror("select"); continue; }

        /* Nouvelle connexion */
        if (FD_ISSET(server_fd, &readfds)) {
            struct sockaddr_in addr_clt;
            int new_sd = accepter_client(server_fd, &addr_clt);
            if (new_sd >= 0) {
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i].socket == 0) {
                        memset(&clients[i], 0, sizeof(ClientInfo));
                        clients[i].socket = new_sd;
                        clients[i].id     = next_client_id++;
                        clients[i].etat   = ETAT_MENU;
                        clients[i].elo    = 1200;
                        printf("[SRV] Nouvelle connexion acceptée (slot %d).\n", i);
                        break;
                    }
                }
            }
        }

        /* Traitement des messages existants */
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket > 0 && FD_ISSET(clients[i].socket, &readfds)) {
                Header h;
                void  *payload = NULL;
                if (recevoir_message(clients[i].socket, &h, &payload) < 0) {
                    nettoyer_client(i);
                } else {
                    traiter_message(i, &h, payload);
                    if (payload) free(payload);
                }
            }
        }

        /* Matchmaking automatique */
        int id1, id2;
        if (matchmake(clients, MAX_CLIENTS, &id1, &id2)) {
            int c1 = get_client_by_id(id1);
            int c2 = get_client_by_id(id2);
            if (c1 != -1 && c2 != -1) {
                /* Retirer de la file d'attente */
                clients[c1].etat = ETAT_MENU;
                clients[c2].etat = ETAT_MENU;
                lancer_partie(c1, c2);
            }
        }
    }
    return 0;
}
