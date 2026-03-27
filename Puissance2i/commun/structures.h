#ifndef STRUCTURES_H
#define STRUCTURES_H

/* =========================================================
 * structures.h -- Structures de données partagées MCS
 * Payloads réseau et modèles internes (Cahier des Charges §8)
 * ========================================================= */

/* --- Payloads réseau (échangés en binaire via data.c) --- */

typedef struct {
    char pseudo[32];
} PayloadLogin;

typedef struct {
    int id_joueur;
    int elo;
    int score;
} PayloadLoginOK;

typedef struct {
    int code_erreur; /* 1=pseudo invalide, 2=déjà connecté */
} PayloadError;

typedef struct {
    int id_partie;
    int id_adversaire;
    char pseudo[32];
    int elo;
} PayloadMatchFound;

typedef struct {
    int colonne; /* 0-indexed, 0-6 */
} PayloadMove;

typedef struct {
    int grille[6][7]; /* 0=vide, id_joueur=pion */
    int tour_de_jeu;  /* ID du joueur dont c'est le tour */
} PayloadGrid;

typedef struct {
    int id_vainqueur; /* 0 = match nul */
    int points_gagnes;
    int nouvel_elo;
} PayloadEndGame;

/* --- Amis --- */
typedef struct {
    char pseudo_ami[32]; /* Pseudo du joueur à ajouter (plus ergonomique que l'ID) */
} PayloadAddFriend;

typedef struct {
    int nb_amis;
    int ids[50];
    char pseudos[50][32];
    int en_ligne[50];    /* 1=en ligne, 0=offline */
    int elo[50];
} PayloadFriendList;

typedef struct {
    int id_ami_cible; /* ID de l'ami à défier */
} PayloadChallenge;

typedef struct {
    int id_challengeur;
    char pseudo_challengeur[32];
    int elo_challengeur;
} PayloadChallengeReceived;

typedef struct {
    int accepte; /* 1=accepté, 0=refusé */
    int id_challengeur;
} PayloadChallengeResponse;

/* --- Profil --- */
typedef struct {
    int id;
    char pseudo[32];
    int score;
    int elo;
    int nb_victoires;
    int nb_defaites;
    int nb_nuls;
} PayloadProfil;

/* --- Machines à état --- */
typedef enum {
    ETAT_MENU = 0,
    ETAT_MATCHMAKING = 1,
    ETAT_EN_JEU = 2,
    ETAT_TOURNOI = 3
} EtatClient;

/* --- Structures internes du serveur --- */
#define MAX_AMIS 50

typedef struct {
    int id;
    int socket;
    char pseudo[32];
    int elo;
    int score;
    int nb_victoires;
    int nb_defaites;
    int nb_nuls;
    EtatClient etat;
    int id_partie_actuelle;
    int amis[MAX_AMIS]; /* IDs des amis */
    int nb_amis;
} ClientInfo;

typedef struct {
    int id_partie;
    int id_joueur1;
    int id_joueur2;
    int tour_joueur_id;
    int grille[6][7];
    int active;
} PartieInfo;

#endif /* STRUCTURES_H */
