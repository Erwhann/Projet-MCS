/*
Nom du fichier : session.h
Auteurs : Erwhann Deiss, Dorian Pazdziej, Matteo Delattre
Description : en-tete de la gestion des sockets TCP du jeu puissance 2i
*/

#ifndef SESSION_H
#define SESSION_H

// inclusions des bibliothèques nécessaires pour les sockets
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

// cree une socket d'ecoute TCP sur le port donne
int creer_serveur(int port);
// accepte une connexion entrante sur la socket d'ecoute
int accepter_client(int fd_ecoute, struct sockaddr_in *addrClt);
// connecte le client au serveur dont l'adresse et le port sont donnes
int connecter_serveur(const char *adresse, int port);
// ferme proprement une socket
void fermer_socket(int fd);

#endif
