/*
Nom du programme : amis.c
Auteurs : Erwhann Deiss, Dorian Pazdziej, Matteo Delattre
Description : gestion de la liste d'amis des joueurs cote serveur
             (ajout, suppression, construction de la liste)
*/

// inclusions des bibliothèques nécessaires
#include "amis.h"
#include <string.h>

// fonction interne : recherche un client par son pseudo dans le tableau global
// retourne son index dans le tableau, -1 si non trouve
static int trouver_client_par_pseudo(ClientInfo *clients, int nb_max, const char *pseudo) {
    for (int i = 0; i < nb_max; i++) {
        // on verifie que le slot est occupe et que le pseudo correspond
        if (clients[i].socket != 0 && strcmp(clients[i].pseudo, pseudo) == 0) {
            return i;
        }
    }
    return -1;
}

// fonction ajouter_ami : ajoute un ami au client via le pseudo de l'ami
// retourne 1 si l'ajout a reussi, 0 sinon (introuvable, deja ami, liste pleine)
int ajouter_ami(ClientInfo *client, ClientInfo *clients_global, int nb_max, const char *pseudo_ami) {
    // on cherche l'ami par son pseudo
    int idx = trouver_client_par_pseudo(clients_global, nb_max, pseudo_ami);
    if (idx == -1) return 0;    // ami introuvable

    int id_ami = clients_global[idx].id;
    if (id_ami == client->id) return 0; // on ne peut pas s'ajouter soi-meme

    // on verifie que l'ami n'est pas deja dans la liste
    for (int i = 0; i < client->nb_amis; i++) {
        if (client->amis[i] == id_ami) return 0;
    }

    // on verifie que la liste n'est pas pleine
    if (client->nb_amis >= MAX_AMIS) return 0;

    // on ajoute l'ami
    client->amis[client->nb_amis++] = id_ami;
    return 1;
}

// fonction supprimer_ami : supprime un ami de la liste par son identifiant
// retourne 1 si la suppression a reussi, 0 si l'ami n'etait pas dans la liste
int supprimer_ami(ClientInfo *client, int id_ami) {
    for (int i = 0; i < client->nb_amis; i++) {
        if (client->amis[i] == id_ami) {
            // on decale les amis suivants pour combler le trou
            for (int j = i; j < client->nb_amis - 1; j++)
                client->amis[j] = client->amis[j + 1];
            client->nb_amis--;
            return 1;
        }
    }
    return 0;   // ami non trouve
}

// fonction construire_liste_amis : construit la liste d'amis a envoyer au client
// croise la liste d'amis du client avec les clients connectes pour determiner
// qui est en ligne, son ELO et son statut social
void construire_liste_amis(ClientInfo *client, ClientInfo *clients_global, int nb_max, PayloadFriendList *out) {
    out->nb_amis = 0;

    // on parcourt tous les identifiants d'amis du client
    for (int i = 0; i < client->nb_amis; i++) {
        int id_ami = client->amis[i];
        int en_ligne = 0;
        char pseudo[32] = "???";    // pseudo par defaut si l'ami est hors ligne
        int elo = 0;
        int statut = SOCIAL_ABSENT;

        // on cherche l'ami parmi les clients connectes
        for (int j = 0; j < nb_max; j++) {
            if (clients_global[j].socket != 0 && clients_global[j].id == id_ami) {
                en_ligne = 1;
                strncpy(pseudo, clients_global[j].pseudo, 31);
                elo    = clients_global[j].elo;
                statut = (int)clients_global[j].etat_social;
                break;
            }
        }
        // on remplit l'entree de la liste
        int k = out->nb_amis;
        out->ids[k]     = id_ami;
        out->en_ligne[k] = en_ligne;
        out->elo[k]     = elo;
        out->statut[k]  = statut;
        strncpy(out->pseudos[k], pseudo, 31);
        out->pseudos[k][31] = '\0';
        out->nb_amis++;
    }
}
