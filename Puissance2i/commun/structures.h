/*
Nom du fichier : structures.h
Auteurs : Erwhann Deiss, Dorian Pazdziej, Matteo Delattre
Description : definition de toutes les structures de donnees partagees
             entre le client et le serveur du jeu puissance 2i
*/

#ifndef STRUCTURES_H
#define STRUCTURES_H

// enumeration des etats sociaux d'un joueur
typedef enum {
    SOCIAL_EN_LIGNE = 0,    // joueur connecte et disponible
    SOCIAL_OCCUPE   = 1,    // joueur en partie ou occupe
    SOCIAL_ABSENT   = 2     // joueur absent
} EtatSocial;

// payload de la requete de login : pseudo du joueur
typedef struct {
    char pseudo[32];    // pseudonyme du joueur (max 31 caracteres + '\0')
} PayloadLogin;

// payload de la reponse de login reussi
typedef struct {
    int id_joueur;      // identifiant unique du joueur
    int elo;            // score ELO du joueur
    int score;          // score total du joueur
    int nb_victoires;   // nombre de victoires
    int nb_defaites;    // nombre de defaites
    int nb_nuls;        // nombre de matchs nuls
} PayloadLoginOK;

// payload d'une erreur generique
typedef struct {
    int code_erreur;    // code d'erreur specifique
} PayloadError;

// payload de la notification de debut de partie
typedef struct {
    int id_partie;      // identifiant de la partie
    int id_adversaire;  // identifiant de l'adversaire
    char pseudo[32];    // pseudo de l'adversaire
    int elo;            // ELO de l'adversaire
    int est_challenge;  // 1 si c'est un defi entre amis, 0 si matchmaking
} PayloadMatchFound;

// payload d'un coup joue par le client
typedef struct {
    int colonne;    // colonne jouee (0 a 6)
} PayloadMove;

// payload de la grille de jeu mise a jour
typedef struct {
    int grille[6][7];   // grille du jeu (6 lignes, 7 colonnes)
    int tour_de_jeu;    // identifiant du joueur dont c'est le tour
} PayloadGrid;

// payload de fin de partie
typedef struct {
    int id_vainqueur;   // identifiant du vainqueur (0 si match nul)
    int points_gagnes;  // points accordes
    int nouvel_elo;     // nouvel ELO du joueur
} PayloadEndGame;

// payload (ancien) pour ajouter un ami par pseudo
typedef struct {
    char pseudo_ami[32]; // pseudo de l'ami a ajouter
} PayloadAddFriend;

// payload d'une demande d'ami
typedef struct {
    char pseudo_cible[32];   // pseudo du joueur cible
} PayloadFriendRequest;

// payload de la reponse a une demande d'ami
typedef struct {
    int id_demandeur;       // identifiant du demandeur
    int accepte;            // 1 si accepte, 0 si refuse
} PayloadFriendResponse;

// payload d'une demande d'ami recue (push serveur -> client)
typedef struct {
    int  id_demandeur;                  // identifiant du demandeur
    char pseudo_demandeur[32];          // pseudo du demandeur
    int  elo_demandeur;                 // ELO du demandeur
} PayloadFriendRequestReceived;

// payload pour supprimer un ami
typedef struct {
    int id_ami;              // identifiant de l'ami a supprimer
} PayloadRemoveFriend;

// payload de la liste d'amis complete
typedef struct {
    int nb_amis;            // nombre d'amis dans la liste
    int ids[50];            // identifiants des amis
    char pseudos[50][32];   // pseudos des amis
    int en_ligne[50];       // 1 si l'ami est en ligne, 0 sinon
    int elo[50];            // ELO de chaque ami
    int statut[50];         // statut social de chaque ami (EtatSocial)
} PayloadFriendList;

// payload pour envoyer un defi a un ami
typedef struct {
    int id_ami_cible;   // identifiant de l'ami a defier
} PayloadChallenge;

// payload d'un defi recu (push serveur -> client)
typedef struct {
    int id_challengeur;                 // identifiant du challengeur
    char pseudo_challengeur[32];        // pseudo du challengeur
    int elo_challengeur;                // ELO du challengeur
} PayloadChallengeReceived;

// payload de la reponse a un defi
typedef struct {
    int accepte;        // 1 si le defi est accepte, 0 sinon
    int id_challengeur; // identifiant du challengeur
} PayloadChallengeResponse;

// payload pour changer le statut social du joueur
typedef struct {
    int etat_social;    // nouvel etat social (valeur de EtatSocial)
} PayloadChangeState;

// payload pour definir le mode ELO d'une partie
typedef struct {
    int elo_impact;     // 1 si la partie est classee, 0 si amicale
} PayloadSetEloMode;

// nombre maximum d'entrees dans le classement ELO
#define MAX_CLASSEMENT 20
// payload du classement ELO
typedef struct {
    int  nb;                                // nombre de joueurs dans le classement
    char pseudo[MAX_CLASSEMENT][32];        // pseudos des joueurs
    int  elo[MAX_CLASSEMENT];              // ELO de chaque joueur
    int  nb_victoires[MAX_CLASSEMENT];     // victoires de chaque joueur
    int  nb_defaites[MAX_CLASSEMENT];      // defaites de chaque joueur
    int  nb_nuls[MAX_CLASSEMENT];          // matchs nuls de chaque joueur
} PayloadLeaderboard;

// payload du profil complet d'un joueur
typedef struct {
    int id;             // identifiant du joueur
    char pseudo[32];    // pseudo du joueur
    int score;          // score total
    int elo;            // score ELO
    int nb_victoires;   // nombre de victoires
    int nb_defaites;    // nombre de defaites
    int nb_nuls;        // nombre de matchs nuls
} PayloadProfil;

// enumeration des etats d'un client (cote serveur)
typedef enum {
    ETAT_MENU = 0,      // client dans le menu principal
    ETAT_MATCHMAKING = 1, // client en attente de matchmaking
    ETAT_EN_JEU = 2,    // client en partie
    ETAT_TOURNOI = 3    // client inscrit dans un tournoi
} EtatClient;

// nombre maximum d'amis par joueur
#define MAX_AMIS 50

// structure complete d'un client (utilisee cote serveur et cote client)
typedef struct {
    int id;                     // identifiant unique du joueur
    int socket;                 // descripteur de la socket (cote serveur)
    char pseudo[32];            // pseudonyme du joueur
    int elo;                    // score ELO
    int score;                  // score total
    int nb_victoires;           // nombre de victoires
    int nb_defaites;            // nombre de defaites
    int nb_nuls;                // nombre de matchs nuls
    EtatClient  etat;           // etat courant du client
    EtatSocial  etat_social;    // statut social du client
    int id_partie_actuelle;     // identifiant de la partie en cours (0 si aucune)
    int amis[MAX_AMIS];         // tableau des identifiants des amis
    int nb_amis;                // nombre d'amis
} ClientInfo;

// structure d'une partie en cours (cote serveur)
typedef struct {
    int id_partie;          // identifiant unique de la partie
    int id_joueur1;         // identifiant du joueur 1
    int id_joueur2;         // identifiant du joueur 2
    int tour_joueur_id;     // identifiant du joueur dont c'est le tour
    int grille[6][7];       // grille de jeu
    int active;             // 1 si la partie est en cours, 0 sinon
    int elo_impact;         // 1 si la partie impacte l'ELO
    int elo_mode_pending;   // 1 si le mode ELO n'a pas encore ete choisi
    int id_challengeur;     // identifiant du joueur qui a lance le defi
} PartieInfo;

// nombre maximum de joueurs dans un tournoi
#define MAX_JOUEURS_TOURNOI 4

// payload de l'etat courant d'un tournoi
typedef struct {
    int  etat;          // etat du tournoi (EtatTournoi)
    int  nb_joueurs;    // nombre de joueurs inscrits
    char pseudos[MAX_JOUEURS_TOURNOI][32];  // pseudos des participants
    int  elos[MAX_JOUEURS_TOURNOI];         // ELO des participants
    int  vainqueur_sf1;     // identifiant du vainqueur de la demi-finale 1
    int  vainqueur_sf2;     // identifiant du vainqueur de la demi-finale 2
    int  vainqueur_final;   // identifiant du vainqueur du tournoi
    char msg[64];           // message d'etat du tournoi
} PayloadTournamentState;

#endif
