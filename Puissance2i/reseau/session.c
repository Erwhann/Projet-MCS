#include "session.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int creer_serveur(int port) {
    int fd;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        exit(1);
    }
    if (listen(fd, 10) == -1) {
        perror("listen");
        exit(1);
    }
    return fd;
}

int accepter_client(int fd_ecoute, struct sockaddr_in *addrClt) {
    socklen_t len = sizeof(*addrClt);
    int fdClt = accept(fd_ecoute, (struct sockaddr *)addrClt, &len);
    if (fdClt == -1) {
        perror("accept");
    }
    return fdClt;
}

int connecter_serveur(const char *adresse, int port) {
    char port_str[8];
    snprintf(port_str, sizeof(port_str), "%d", port);

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(adresse, port_str, &hints, &res) != 0) {
        fprintf(stderr, "Erreur : resolution de '%s' impossible.\n", adresse);
        return -1;
    }

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

void fermer_socket(int fd) {
    if (fd >= 0) {
        close(fd);
    }
}
