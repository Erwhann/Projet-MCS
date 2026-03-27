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
    int fd;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return -1;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_aton(adresse, &addr.sin_addr) == 0) {
        return -1;
    }
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        close(fd);
        return -1;
    }
    return fd;
}

void fermer_socket(int fd) {
    if (fd >= 0) {
        close(fd);
    }
}
