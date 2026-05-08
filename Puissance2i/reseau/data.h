/*
Nom du fichier : data.h
Auteurs : Erwhann Deiss, Dorian Pazdziej, Matteo Delattre
Description : en-tete de la couche reseau d'envoi/reception de messages structures
*/

#ifndef DATA_H
#define DATA_H

// inclusion du protocole reseau (Header, TypeMessage)
#include "../commun/protocol.h"

// envoie un message structure (Header suivi du Payload) sur le descripteur fd
// retourne 0 si succes, -1 si erreur
int envoyer_message(int fd, int type, const void* payload, int payload_size);

// recoit un message structure sur le descripteur fd
// alloue dynamiquement le payload si sa taille > 0 (l'appelant doit le free())
// retourne la taille du payload lue, ou -1 en cas d'erreur / deconnexion
int recevoir_message(int fd, Header* header, void** payload);

#endif
