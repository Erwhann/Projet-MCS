#include "amis.h"
#include <string.h>

/* =========================================================
 * amis.c -- Gestion de la liste d'amis côté serveur
 * ========================================================= */

/**
 * Recherche un client par pseudo dans le tableau global.
 * Retourne son index ou -1 si introuvable.
 */
static int trouver_client_par_pseudo(ClientInfo *clients, int nb_max, const char *pseudo) {
    for (int i = 0; i < nb_max; i++) {
        if (clients[i].socket != 0 && strcmp(clients[i].pseudo, pseudo) == 0) {
            return i;
        }
    }
    return -1;
}

int ajouter_ami(ClientInfo *client, ClientInfo *clients_global, int nb_max, const char *pseudo_ami) {
    int idx = trouver_client_par_pseudo(clients_global, nb_max, pseudo_ami);
    if (idx == -1) return 0; /* Joueur introuvable */

    int id_ami = clients_global[idx].id;
    if (id_ami == client->id) return 0; /* On ne peut pas s'ajouter soi-même */

    /* Vérifier si déjà ami */
    for (int i = 0; i < client->nb_amis; i++) {
        if (client->amis[i] == id_ami) return 0;
    }

    if (client->nb_amis >= MAX_AMIS) return 0;

    client->amis[client->nb_amis++] = id_ami;
    return 1;
}

void construire_liste_amis(ClientInfo *client, ClientInfo *clients_global, int nb_max, PayloadFriendList *out) {
    out->nb_amis = 0;

    for (int i = 0; i < client->nb_amis; i++) {
        int id_ami = client->amis[i];
        int en_ligne = 0;
        char pseudo[32] = "???";
        int elo = 0;
        int statut = SOCIAL_ABSENT;
        for (int j = 0; j < nb_max; j++) {
            if (clients_global[j].socket != 0 && clients_global[j].id == id_ami) {
                en_ligne = 1;
                strncpy(pseudo, clients_global[j].pseudo, 31);
                elo    = clients_global[j].elo;
                statut = (int)clients_global[j].etat_social;
                break;
            }
        }
        int k = out->nb_amis;
        out->ids[k]    = id_ami;
        out->en_ligne[k] = en_ligne;
        out->elo[k]    = elo;
        out->statut[k] = statut;
        strncpy(out->pseudos[k], pseudo, 31);
        out->pseudos[k][31] = '\0';
        out->nb_amis++;
    }
}
