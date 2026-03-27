#include "data.h"
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int envoyer_message(int fd, int type, const void* payload, int payload_size) {
    Header h;
    h.type = type;
    h.payload_size = payload_size;

    // Envoi du Header atomique
    int n = send(fd, &h, sizeof(Header), 0);
    if (n != sizeof(Header)) return -1;

    // Envoi du Payload si nécessaire
    if (payload_size > 0 && payload != NULL) {
        int total = 0;
        while (total < payload_size) {
            n = send(fd, ((char*)payload) + total, payload_size - total, 0);
            if (n <= 0) return -1;
            total += n;
        }
    }
    return 0;
}

int recevoir_message(int fd, Header* header, void** payload) {
    *payload = NULL;

    // Attente du Header complet
    int n = recv(fd, header, sizeof(Header), MSG_WAITALL);
    if (n <= 0) return -1; // Déconnexion ou erreur

    // Attente du Payload complet si nécessaire
    if (header->payload_size > 0) {
        // Sécurité contre tentative d'overflow
        if (header->payload_size > 100000) { 
            return -1; 
        }

        *payload = malloc(header->payload_size);
        if (*payload == NULL) return -1;

        n = recv(fd, *payload, header->payload_size, MSG_WAITALL);
        if (n <= 0) {
            free(*payload);
            *payload = NULL;
            return -1;
        }
    }
    return header->payload_size;
}
