#ifndef MATCHMAKING_H
#define MATCHMAKING_H
#include "../commun/structures.h"

int matchmake(ClientInfo* clients, int nb_clients_max, int* id_c1, int* id_c2);

#endif
