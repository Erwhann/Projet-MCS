/* =========================================================
 * ihm.h -- Interface NCURSES (couleurs) pour le client Puissance 2i
 * ========================================================= */
#ifndef IHM_H
#define IHM_H

#include "../commun/protocol.h"
#include "../commun/structures.h"
#include <ncurses.h>

/* --- Initialisation / arrêt --- */
void init_ihm(void);
void fin_ihm(void);

/* --- Saisie du pseudo (bloquante) --- */
void demander_pseudo_ncurses(char *pseudo, const char *pseudo_suggestion);

/* --- Écrans --- */
void dessiner_menu(const ClientInfo *moi);
void dessiner_matchmaking(const ClientInfo *moi);
void dessiner_partie(const ClientInfo *moi, const PartieInfo *partie, const char *msg);
void dessiner_fin_partie(const ClientInfo *moi, int id_vainqueur, int points, int nv_elo);
void dessiner_amis(const PayloadFriendList *liste);
void dessiner_profil(const ClientInfo *moi);
void dessiner_notification(const char *msg);

/* --- Saisie d'une chaîne en overlay (non-bloquant) --- */
int saisir_chaine_overlay(const char *invite, char *buf, int maxlen);

/* --- Handlers asynchrones --- */
void traiter_message_serveur(Header *h, void *payload, ClientInfo *moi,
                             PartieInfo *partie, PayloadFriendList *amis,
                             int *etat_ihm, int *challenge_en_attente,
                             int *challenger_id);

void traiter_saisie(int ch, ClientInfo *moi, PartieInfo *partie,
                    PayloadFriendList *amis, int *etat_ihm,
                    int *challenge_en_attente, int challenger_id, int sock);

/* --- États IHM --- */
#define IHM_MENU        0
#define IHM_MATCHMAKING 1
#define IHM_EN_JEU      2
#define IHM_FIN_PARTIE  3
#define IHM_AMIS        4
#define IHM_PROFIL      5

/* --- Paires de couleurs --- */
#define COL_TITRE    1
#define COL_NORMAL   2
#define COL_ACCENT   3
#define COL_VAINC    4
#define COL_DEFAITE  5
#define COL_GRILLE   6
#define COL_MOI      7
#define COL_ADV      8
#define COL_NOTIF    9

#endif /* IHM_H */
