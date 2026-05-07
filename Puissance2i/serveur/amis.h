#ifndef AMIS_H
#define AMIS_H

#include "../commun/structures.h"


int ajouter_ami(ClientInfo *client, ClientInfo *clients_global, int nb_max, const char *pseudo_ami);

int supprimer_ami(ClientInfo *client, int id_ami);

void construire_liste_amis(ClientInfo *client, ClientInfo *clients_global, int nb_max, PayloadFriendList *out);

#endif
