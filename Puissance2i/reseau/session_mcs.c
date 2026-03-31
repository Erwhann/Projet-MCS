/**
 * \file        session_mcs.c
 * \brief       Implementation de la couche Session (bibliotheque MCS)
 * \author      Samir El Khattabi / Erwhann D
 * \date        3 mars 2023 / 31 mars 2026
 * \version     1.0
 */
#include "session_mcs.h"

void mcs_adr2struct(struct sockaddr_in *addr, char *adrIP, short port) {
    memset(addr, 0, sizeof(struct sockaddr_in));
    addr->sin_family      = AF_INET;
    addr->sin_port        = htons(port);
    addr->sin_addr.s_addr = inet_addr(adrIP);
}

socket_t mcs_creerSocket(int mode) {
    socket_t sock;
    sock.mode = mode;
    MCS_CHECK(sock.fd = socket(AF_INET, mode, 0), "socket()");
    return sock;
}

socket_t mcs_creerSocketAdr(int mode, char *adrIP, short port) {
    socket_t sock = mcs_creerSocket(mode);
    mcs_adr2struct(&sock.addrLoc, adrIP, port);
    MCS_CHECK(bind(sock.fd, (struct sockaddr *)&sock.addrLoc,
                   sizeof(sock.addrLoc)), "bind()");
    return sock;
}

socket_t mcs_creerSocketEcoute(char *adrIP, short port) {
    socket_t sock = mcs_creerSocket(SOCK_STREAM);
    int opt = 1;
    setsockopt(sock.fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    mcs_adr2struct(&sock.addrLoc, adrIP, port);
    MCS_CHECK(bind(sock.fd, (struct sockaddr *)&sock.addrLoc,
                   sizeof(sock.addrLoc)), "bind()");
    MCS_CHECK(listen(sock.fd, 5), "listen()");
    return sock;
}

socket_t mcs_accepterClt(const socket_t sockEcoute) {
    socket_t sockDial;
    socklen_t len = sizeof(sockDial.addrDst);
    sockDial.mode = sockEcoute.mode;
    MCS_CHECK(sockDial.fd = accept(sockEcoute.fd,
                                   (struct sockaddr *)&sockDial.addrDst,
                                   &len), "accept()");
    return sockDial;
}

socket_t mcs_connecterClt2Srv(char *adrIP, short port) {
    socket_t sock = mcs_creerSocket(SOCK_STREAM);
    mcs_adr2struct(&sock.addrDst, adrIP, port);
    MCS_CHECK(connect(sock.fd, (struct sockaddr *)&sock.addrDst,
                      sizeof(sock.addrDst)), "connect()");
    return sock;
}
