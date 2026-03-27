#ifndef DATA_H
#define DATA_H

#include "../commun/protocol.h"

// Envoie un message structuré (Header suivi du Payload)
int envoyer_message(int fd, int type, const void* payload, int payload_size);

// Reçoit un message structuré (alloue le payload si > 0, l'appelant doit le free())
// Retourne la taille du payload lue ou -1 en cas d'erreur / déconnexion
int recevoir_message(int fd, Header* header, void** payload);

#endif
