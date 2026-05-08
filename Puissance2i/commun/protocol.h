/*
Nom du fichier : protocol.h
Auteurs : Erwhann Deiss, Dorian Pazdziej, Matteo Delattre
Description : definition du protocole reseau du jeu puissance 2i
             (types de messages echanges entre client et serveur)
*/

#ifndef PROTOCOL_H
#define PROTOCOL_H

// enumeration des types de messages du protocole reseau
typedef enum {
    // messages de connexion / login
    REQ_LOGIN = 10,         // requete de connexion (client -> serveur)
    RES_LOGIN_OK = 11,      // reponse connexion acceptee (serveur -> client)
    RES_LOGIN_ERROR = 12,   // reponse connexion refusee (serveur -> client)
    // messages de matchmaking
    REQ_MATCHMAKING = 20,   // requete de matchmaking (client -> serveur)
    RES_WAITING = 21,       // reponse en attente de matchmaking (serveur -> client)
    REQ_CANCEL_MATCH = 22,  // annuler la recherche de partie (client -> serveur)
    PUSH_MATCH_FOUND = 23,  // adversaire trouve (serveur -> client)
    // messages de jeu
    REQ_MOVE = 30,          // jouer un coup (client -> serveur)
    RES_MOVE_ERROR = 31,    // coup invalide (serveur -> client)
    PUSH_GRID = 32,         // mise a jour de la grille (serveur -> client)
    PUSH_TURN = 33,         // changement de tour (serveur -> client)
    PUSH_ENDGAME = 34,      // fin de partie (serveur -> client)
    PUSH_ELO_UPDATE = 35,   // mise a jour ELO (serveur -> client)
    // messages de gestion des amis
    REQ_ADD_FRIEND = 40,    // ajouter un ami (ancien protocole, non utilise)
    RES_FRIEND_ADDED = 41,  // ami ajoute avec succes (serveur -> client)
    REQ_FRIEND_LIST = 42,   // demander la liste d'amis (client -> serveur)
    PUSH_FRIEND_LIST = 43,  // liste d'amis (serveur -> client)
    REQ_CHALLENGE = 44,     // envoyer un defi a un ami (client -> serveur)
    PUSH_CHALLENGE = 45,    // defi recu (serveur -> client)
    RES_CHALLENGE_ACCEPT = 46,      // reponse au defi (client -> serveur)
    REQ_FRIEND_REQUEST  = 47,       // demande d'ami (client -> serveur)
    PUSH_FRIEND_REQUEST = 48,       // demande d'ami recue (serveur -> client)
    RES_FRIEND_REQUEST  = 49,       // reponse a la demande d'ami (client -> serveur)
    REQ_REMOVE_FRIEND   = 53,       // supprimer un ami (client -> serveur)
    RES_FRIEND_REMOVED  = 54,       // ami supprime (serveur -> client)
    // messages de tournoi
    REQ_JOIN_TOURNAMENT    = 50,    // rejoindre un tournoi (client -> serveur)
    PUSH_TOURNAMENT_STATE  = 51,    // etat du tournoi (serveur -> client)
    RES_TOURNAMENT_JOINED  = 52,    // inscription au tournoi reussie (serveur -> client)
    RES_TOURNAMENT_ERROR   = 55,    // erreur de tournoi (serveur -> client)
    // messages de statut et mode ELO
    REQ_CHANGE_STATE = 60,   // changer le statut social (client -> serveur)
    PUSH_CHOOSE_ELO  = 61,   // demander le choix du mode ELO (serveur -> client)
    REQ_SET_ELO_MODE = 62,   // definir le mode ELO de la partie (client -> serveur)
    // messages du classement
    REQ_LEADERBOARD  = 70,   // demander le classement ELO (client -> serveur)
    PUSH_LEADERBOARD = 71,   // classement ELO (serveur -> client)
    // message d'erreur generique
    RES_ERROR_STATE = 99     // erreur d'etat (serveur -> client)
} TypeMessage;

// structure d'en-tete de chaque message reseau
typedef struct {
    int type;           // type du message (TypeMessage)
    int payload_size;   // taille du payload en octets
} Header;

#endif
