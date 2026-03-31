#ifndef PROTOCOL_H
#define PROTOCOL_H

typedef enum {
    REQ_LOGIN = 10,
    RES_LOGIN_OK = 11,
    RES_LOGIN_ERROR = 12,
    REQ_MATCHMAKING = 20,
    RES_WAITING = 21,
    REQ_CANCEL_MATCH = 22,
    PUSH_MATCH_FOUND = 23,
    REQ_MOVE = 30,
    RES_MOVE_ERROR = 31,
    PUSH_GRID = 32,
    PUSH_TURN = 33,
    PUSH_ENDGAME = 34,
    PUSH_ELO_UPDATE = 35,
    REQ_ADD_FRIEND = 40,
    RES_FRIEND_ADDED = 41,
    REQ_FRIEND_LIST = 42,
    PUSH_FRIEND_LIST = 43,
    REQ_CHALLENGE = 44,
    PUSH_CHALLENGE = 45,
    RES_CHALLENGE_ACCEPT = 46,
    REQ_FRIEND_REQUEST  = 47,   /* Client A -> Serveur : demande d'ajout de B */
    PUSH_FRIEND_REQUEST = 48,   /* Serveur -> Client B : A veut t'ajouter     */
    RES_FRIEND_REQUEST  = 49,   /* Client B -> Serveur : accepte/refuse        */
    REQ_REMOVE_FRIEND   = 53,   /* Client -> Serveur : supprimer un ami        */
    RES_FRIEND_REMOVED  = 54,   /* Serveur -> Client : confirmation suppression*/
    REQ_JOIN_TOURNAMENT = 50,
    PUSH_TOURNAMENT_STATE = 51,
    REQ_CHANGE_STATE = 60,   /* Changer statut social (en ligne/occupe/absent) */
    PUSH_CHOOSE_ELO  = 61,   /* Serveur demande si partie classee (challenge) */
    REQ_SET_ELO_MODE = 62,   /* Client repond : 1=avec ELO, 0=partie amicale  */
    REQ_LEADERBOARD  = 70,   /* Client demande le classement ELO               */
    PUSH_LEADERBOARD = 71,   /* Serveur envoie le classement                   */
    RES_ERROR_STATE = 99
} TypeMessage;

typedef struct {
    int type;
    int payload_size;
} Header;

#endif
