/*
Nom du programme : session.c
Auteurs : Erwhann Deiss, Dorian Pazdziej, Matteo Delattre
Description : gestion des sockets TCP (creation serveur, connexion client,
             acceptation de connexions entrantes)
*/

// inclusions des bibliothèques nécessaires
#include "session.h"
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// fonction creer_serveur : cree et configure une socket d'ecoute TCP
// retourne le descripteur de la socket, exit() en cas d'erreur fatale
int creer_serveur(int port) {
    int fd;
    // creation de la socket TCP
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    // option SO_REUSEADDR pour eviter le TIME_WAIT au redemarrage
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // configuration de l'adresse du serveur
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;  // toutes les interfaces reseau
    addr.sin_port = htons(port);

    // attachement de la socket a l'adresse
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        exit(1);
    }
    // mise en ecoute de la socket
    if (listen(fd, 10) == -1) {
        perror("listen");
        exit(1);
    }
    return fd;
}

// fonction accepter_client : accepte une connexion entrante
// retourne le descripteur de la socket du client, -1 si erreur
int accepter_client(int fd_ecoute, struct sockaddr_in *addrClt) {
    socklen_t len = sizeof(*addrClt);
    int fdClt = accept(fd_ecoute, (struct sockaddr *)addrClt, &len);
    if (fdClt == -1) {
        perror("accept");
    }
    return fdClt;
}

// fonction connecter_serveur : connecte le client a un serveur par nom/IP et port
// utilise getaddrinfo pour resoudre les noms d'hotes
// retourne le descripteur de la socket, -1 si echec
int connecter_serveur(const char *adresse, int port) {
    char port_str[8];
    snprintf(port_str, sizeof(port_str), "%d", port);

    // resolution de l'adresse du serveur (supporte IP et nom d'hote)
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(adresse, port_str, &hints, &res) != 0) {
        fprintf(stderr, "Erreur : resolution de '%s' impossible.\n", adresse);
        return -1;
    }

    // creation de la socket et connexion au serveur
    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd == -1) { perror("socket"); freeaddrinfo(res); return -1; }

    if (connect(fd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("connect");
        close(fd);
        freeaddrinfo(res);
        return -1;
    }
    freeaddrinfo(res);
    return fd;
}

// fonction fermer_socket : ferme proprement une socket
void fermer_socket(int fd) {
    if (fd >= 0) {
        close(fd);
    }
}
