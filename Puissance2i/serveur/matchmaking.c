/*
Nom du programme : matchmaking.c
Auteurs : Erwhann Deiss, Dorian Pazdziej, Matteo Delattre
Description : algorithme de matchmaking du jeu puissance 2i
             (appariement des joueurs en attente de partie)
*/

// inclusion de l'en-tete du module
#include "matchmaking.h"

// fonction matchmake : cherche deux joueurs en etat ETAT_MATCHMAKING
// et retourne leurs identifiants via id_c1 et id_c2
// retourne 1 si une paire a ete trouvee, 0 sinon
int matchmake(ClientInfo* clients, int nb_clients_max, int* id_c1, int* id_c2) {
    int first = -1;     // index du premier joueur en attente trouve
    for (int i = 0; i < nb_clients_max; i++) {
        // on cherche les clients connectes en attente de matchmaking
        if (clients[i].socket != 0 && clients[i].etat == ETAT_MATCHMAKING) {
            if (first == -1) {
                // premier joueur trouve : on le memorise
                first = i;
            } else {
                // deuxieme joueur trouve : on a une paire, on retourne leurs IDs
                *id_c1 = clients[first].id;
                *id_c2 = clients[i].id;
                return 1;   // paire trouvee
            }
        }
    }
    return 0;   // pas assez de joueurs en attente
}
