#ifndef AMIS_H
#define AMIS_H

/* =========================================================
 * amis.h -- Gestion de la liste d'amis côté serveur
 * ========================================================= */
#include "../commun/structures.h"

/**
 * Ajoute l'ami de pseudo `pseudo_ami` dans la liste du client `client`.
 * Cherche l'ID dans le tableau global `clients`.
 * Retourne 1 si succès, 0 sinon (introuvable ou déjà ami).
 */
int ajouter_ami(ClientInfo *client, ClientInfo *clients_global, int nb_max, const char *pseudo_ami);

/**
 * Supprime l'ami d'ID id_ami de la liste du client.
 * Retourne 1 si trouve et supprime, 0 sinon.
 */
int supprimer_ami(ClientInfo *client, int id_ami);

/**
 * Construit le payload de liste d'amis du client donné,
 * en croisant avec l'état actuel des clients connectés.
 */
void construire_liste_amis(ClientInfo *client, ClientInfo *clients_global, int nb_max, PayloadFriendList *out);

#endif /* AMIS_H */
