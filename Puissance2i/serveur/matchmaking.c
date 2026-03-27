#include "matchmaking.h"

int matchmake(ClientInfo* clients, int nb_clients_max, int* id_c1, int* id_c2) {
    int first = -1;
    for (int i = 0; i < nb_clients_max; i++) {
        if (clients[i].socket != 0 && clients[i].etat == ETAT_MATCHMAKING) {
            if (first == -1) {
                first = i;
            } else {
                *id_c1 = clients[first].id;
                *id_c2 = clients[i].id;
                return 1; // Paire trouvée
            }
        }
    }
    return 0;
}
