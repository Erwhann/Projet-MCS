/*
Nom du fichier : amis.h
Auteurs : Erwhann Deiss, Dorian Pazdziej, Matteo Delattre
Description : en-tete de la gestion de la liste d'amis cote serveur
*/

#ifndef AMIS_H
#define AMIS_H

// inclusion des structures partagees (ClientInfo, PayloadFriendList, MAX_AMIS)
#include "../commun/structures.h"

// ajoute un ami au client par le pseudo de l'ami cible
// retourne 1 si succes, 0 si echec (introuvable, deja ami, liste pleine)
int ajouter_ami(ClientInfo *client, ClientInfo *clients_global, int nb_max, const char *pseudo_ami);

// supprime un ami de la liste par son identifiant
// retourne 1 si succes, 0 si l'ami n'etait pas dans la liste
int supprimer_ami(ClientInfo *client, int id_ami);

// construit la structure PayloadFriendList a envoyer au client
// (croise la liste d'amis avec les clients connectes)
void construire_liste_amis(ClientInfo *client, ClientInfo *clients_global, int nb_max, PayloadFriendList *out);

#endif
