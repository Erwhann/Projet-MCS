/*
Nom du fichier : matchmaking.h
Auteurs : Erwhann Deiss, Dorian Pazdziej, Matteo Delattre
Description : en-tete du module de matchmaking du jeu puissance 2i
*/

#ifndef MATCHMAKING_H
#define MATCHMAKING_H
// inclusion des structures partagees (ClientInfo, ETAT_MATCHMAKING)
#include "../commun/structures.h"

// cherche deux joueurs en attente de matchmaking dans le tableau des clients
// si une paire est trouvee, retourne leurs IDs dans id_c1 et id_c2 et retourne 1
// retourne 0 si pas assez de joueurs disponibles
int matchmake(ClientInfo* clients, int nb_clients_max, int* id_c1, int* id_c2);

#endif
