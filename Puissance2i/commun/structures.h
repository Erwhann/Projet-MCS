#ifndef STRUCTURES_H
#define STRUCTURES_H

typedef enum {
    SOCIAL_EN_LIGNE = 0,
    SOCIAL_OCCUPE   = 1,
    SOCIAL_ABSENT   = 2
} EtatSocial;


typedef struct {
    char pseudo[32];
} PayloadLogin;

typedef struct {
    int id_joueur;
    int elo;
    int score;
    int nb_victoires;
    int nb_defaites;
    int nb_nuls;
} PayloadLoginOK;

typedef struct {
    int code_erreur;
} PayloadError;

typedef struct {
    int id_partie;
    int id_adversaire;
    char pseudo[32];
    int elo;
    int est_challenge;
} PayloadMatchFound;

typedef struct {
    int colonne; 
} PayloadMove;

typedef struct {
    int grille[6][7]; 
    int tour_de_jeu;
} PayloadGrid;

typedef struct {
    int id_vainqueur; 
    int points_gagnes;
    int nouvel_elo;
} PayloadEndGame;

typedef struct {
    char pseudo_ami[32];
} PayloadAddFriend;

typedef struct {
    char pseudo_cible[32];   
} PayloadFriendRequest;

typedef struct {
    int id_demandeur;       
    int accepte;           
} PayloadFriendResponse;

typedef struct {
    int  id_demandeur;
    char pseudo_demandeur[32];
    int  elo_demandeur;
} PayloadFriendRequestReceived;

typedef struct {
    int id_ami;              
} PayloadRemoveFriend;

typedef struct {
    int nb_amis;
    int ids[50];
    char pseudos[50][32];
    int en_ligne[50];      
    int elo[50];
    int statut[50];       
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

typedef struct {
    int etat_social; 
} PayloadChangeState;

typedef struct {
    int elo_impact; 
} PayloadSetEloMode;

#define MAX_CLASSEMENT 20
typedef struct {
    int  nb;
    char pseudo[MAX_CLASSEMENT][32];
    int  elo[MAX_CLASSEMENT];
    int  nb_victoires[MAX_CLASSEMENT];
    int  nb_defaites[MAX_CLASSEMENT];
    int  nb_nuls[MAX_CLASSEMENT];
} PayloadLeaderboard;

typedef struct {
    int id;
    char pseudo[32];
    int score;
    int elo;
    int nb_victoires;
    int nb_defaites;
    int nb_nuls;
} PayloadProfil;

typedef enum {
    ETAT_MENU = 0,
    ETAT_MATCHMAKING = 1,
    ETAT_EN_JEU = 2,
    ETAT_TOURNOI = 3
} EtatClient;

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
    int elo_impact;
    int elo_mode_pending;
    int id_challengeur;
} PartieInfo;

#endif 
