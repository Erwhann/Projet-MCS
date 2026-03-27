#ifndef STRUCTURES_H
#define STRUCTURES_H

/* =========================================================
 * structures.h -- Structures de donnees partagees MCS
 * Payloads reseau et modeles internes (CDC ss8)
 * AUCUN accent dans les commentaires pour portabilite ASCII.
 * ========================================================= */

/* --- Statut social du joueur --- */
typedef enum {
    SOCIAL_EN_LIGNE = 0,
    SOCIAL_OCCUPE   = 1,
    SOCIAL_ABSENT   = 2
} EtatSocial;

/* --- Payloads reseau --- */

typedef struct {
    char pseudo[32];
} PayloadLogin;

typedef struct {
    int id_joueur;
    int elo;
    int score;
} PayloadLoginOK;

typedef struct {
    int code_erreur;
} PayloadError;

typedef struct {
    int id_partie;
    int id_adversaire;
    char pseudo[32];
    int elo;
    int est_challenge; /* 1 = partie de defi direct, 0 = matchmaking */
} PayloadMatchFound;

typedef struct {
    int colonne; /* 0-indexed, 0-6 */
} PayloadMove;

typedef struct {
    int grille[6][7]; /* 0=vide, id_joueur=pion */
    int tour_de_jeu;
} PayloadGrid;

typedef struct {
    int id_vainqueur; /* 0 = match nul */
    int points_gagnes;
    int nouvel_elo;
} PayloadEndGame;

/* --- Amis --- */
typedef struct {
    char pseudo_ami[32];
} PayloadAddFriend;

typedef struct {
    int nb_amis;
    int ids[50];
    char pseudos[50][32];
    int en_ligne[50];      /* 1=connecte, 0=deconnecte */
    int elo[50];
    int statut[50];        /* EtatSocial : 0=en_ligne 1=occupe 2=absent */
} PayloadFriendList;

typedef struct {
    int id_ami_cible;
} PayloadChallenge;

typedef struct {
    int id_challengeur;
    char pseudo_challengeur[32];
    int elo_challengeur;
} PayloadChallengeReceived;

typedef struct {
    int accepte;
    int id_challengeur;
} PayloadChallengeResponse;

/* --- Statut social --- */
typedef struct {
    int etat_social; /* EtatSocial */
} PayloadChangeState;

/* --- Mode ELO (partie amicale ou classee) --- */
typedef struct {
    int elo_impact; /* 1=partie classee, 0=partie amicale (sans ELO) */
} PayloadSetEloMode;

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

/* --- Machine a etats reseau client --- */
typedef enum {
    ETAT_MENU = 0,
    ETAT_MATCHMAKING = 1,
    ETAT_EN_JEU = 2,
    ETAT_TOURNOI = 3
} EtatClient;

/* --- Structures internes serveur --- */
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
    EtatClient  etat;
    EtatSocial  etat_social;
    int id_partie_actuelle;
    int amis[MAX_AMIS];
    int nb_amis;
} ClientInfo;

typedef struct {
    int id_partie;
    int id_joueur1;
    int id_joueur2;
    int tour_joueur_id;
    int grille[6][7];
    int active;
    int elo_impact;          /* 1=classee, 0=amicale */
    int elo_mode_pending;    /* 1=attente reponse du challengeur */
    int id_challengeur;      /* ID du joueur qui choisit le mode ELO */
} PartieInfo;

#endif /* STRUCTURES_H */
