/*
Nom du programme : data.c
Auteurs : Erwhann Deiss, Dorian Pazdziej, Matteo Delattre
Description : couche reseau d'envoi et de reception de messages structures
             (header + payload) sur une socket TCP
*/

// inclusions des bibliothèques nécessaires
#include "data.h"
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// fonction envoyer_message : envoie un message structure (header + payload)
// retourne 0 si succes, -1 si erreur
int envoyer_message(int fd, int type, const void* payload, int payload_size) {
    Header h;
    h.type = type;
    h.payload_size = payload_size;

    // envoi du header de maniere atomique
    int n = send(fd, &h, sizeof(Header), 0);
    if (n != sizeof(Header)) return -1;

    // envoi du payload si necessaire
    if (payload_size > 0 && payload != NULL) {
        int total = 0;
        // on envoie le payload en plusieurs fois si necessaire
        while (total < payload_size) {
            n = send(fd, ((char*)payload) + total, payload_size - total, 0);
            if (n <= 0) return -1;
            total += n;
        }
    }
    return 0;
}

// fonction recevoir_message : recoit un message structure (header + payload)
// alloue le payload si sa taille > 0, l'appelant doit le free()
// retourne la taille du payload lue ou -1 en cas d'erreur / deconnexion
int recevoir_message(int fd, Header* header, void** payload) {
    *payload = NULL;

    // attente du header complet
    int n = recv(fd, header, sizeof(Header), MSG_WAITALL);
    if (n <= 0) return -1;  // deconnexion ou erreur

    // attente du payload complet si necessaire
    if (header->payload_size > 0) {
        // securite contre une tentative d'overflow memoire
        if (header->payload_size > 100000) {
            return -1;
        }

        // allocation du payload
        *payload = malloc(header->payload_size);
        if (*payload == NULL) return -1;

        // reception du payload
        n = recv(fd, *payload, header->payload_size, MSG_WAITALL);
        if (n <= 0) {
            free(*payload);
            *payload = NULL;
            return -1;
        }
    }
    return header->payload_size;
}
