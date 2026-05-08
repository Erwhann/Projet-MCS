
#ifndef IHM_H
#define IHM_H

#include "../commun/protocol.h"
#include "../commun/structures.h"
#include <ncurses.h>

void init_ihm(void);
void fin_ihm(void);

void demander_pseudo_ncurses(char *pseudo, const char *suggestion);

void dessiner_menu(const ClientInfo *moi);
void dessiner_matchmaking(void);
void dessiner_partie(const ClientInfo *moi, const PartieInfo *partie, const char *msg);
void dessiner_fin_partie(const ClientInfo *moi, int id_vainqueur, int points, int nv_elo, int ancien_elo);
void dessiner_amis(const PayloadFriendList *liste);
void dessiner_profil(const ClientInfo *moi);
void dessiner_statut(int statut_actuel);
void dessiner_choisir_elo(void);
void dessiner_classement(const PayloadLeaderboard *lb);
void dessiner_tournoi(const PayloadTournamentState *t);
void dessiner_notification(const char *msg);
int saisir_chaine_overlay(const char *invite, char *buf, int maxlen);
void traiter_message_serveur(Header *h, void *payload, ClientInfo *moi,
                             PartieInfo *partie, PayloadFriendList *amis,
                             int *etat_ihm, int *challenge_en_attente,
                             int *challenger_id);

void traiter_saisie(int ch, ClientInfo *moi, PartieInfo *partie,
                    PayloadFriendList *amis, int *etat_ihm,
                    int *challenge_en_attente, int challenger_id, int sock);

#define IHM_MENU          0
#define IHM_MATCHMAKING   1
#define IHM_EN_JEU        2
#define IHM_FIN_PARTIE    3
#define IHM_AMIS          4
#define IHM_PROFIL        5
#define IHM_STATUT        6   
#define IHM_CHOISIR_ELO   7   
#define IHM_FRIEND_REQUEST 8 
#define IHM_CLASSEMENT    9   
#define IHM_TOURNOI       10  


#define COL_TITRE    1
#define COL_NORMAL   2
#define COL_ACCENT   3
#define COL_VAINC    4
#define COL_DEFAITE  5
#define COL_GRILLE   6
#define COL_MOI      7
#define COL_ADV      8
#define COL_NOTIF    9

#endif 
