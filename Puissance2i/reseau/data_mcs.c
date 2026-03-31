/**
 * \file        data_mcs.c
 * \brief       Implementation de la couche Data Representation (bibliotheque MCS)
 * \author      Samir El Khattabi / Erwhann D
 * \date        3 mars 2023 / 31 mars 2026
 * \version     1.0
 */
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "data_mcs.h"

#define RECV_FLAGS 0
#define SEND_FLAGS 0

/* -----------------------------------------------------------------------
 * Primitives DGRAM
 * ----------------------------------------------------------------------- */

static void envoyerDGRAM(socket_t *sockEch, char *msg,
                         char *adrDest, short portDest) {
    struct sockaddr_in addr;
    mcs_adr2struct(&addr, adrDest, portDest);
    sendto(sockEch->fd, msg, strlen(msg) + 1, SEND_FLAGS,
           (struct sockaddr *)&addr, sizeof(addr));
}

static void recevoirDGRAM(socket_t *sockEch, char *msg, int msgSize) {
    socklen_t len = sizeof(sockEch->addrDst);
    int nb = recvfrom(sockEch->fd, msg, msgSize, RECV_FLAGS,
                      (struct sockaddr *)&sockEch->addrDst, &len);
    if (nb <= 0)
        strcpy(msg, "200:QUIT:Deconnexion P2P inattendue");
}

/* -----------------------------------------------------------------------
 * Primitives STREAM
 * ----------------------------------------------------------------------- */

static void envoyerSTREAM(const socket_t *sockEch, char *msg) {
    send(sockEch->fd, msg, strlen(msg) + 1, SEND_FLAGS);
}

static void recevoirSTREAM(const socket_t *sockEch, char *msg, int msgSize) {
    int nb = recv(sockEch->fd, msg, msgSize, RECV_FLAGS);
    if (nb <= 0)
        strcpy(msg, "200:QUIT:Deconnexion Client inattendue");
}

/* -----------------------------------------------------------------------
 * API publique
 * ----------------------------------------------------------------------- */

void mcs_envoyer(socket_t *sockEch, mcs_generic quoi, mcs_pFct serial, ...) {
    mcs_buffer_t buff;
    if (serial != NULL)
        serial(quoi, buff);
    else
        strcpy(buff, (char *)quoi);

    if (sockEch->mode == SOCK_STREAM) {
        envoyerSTREAM(sockEch, buff);
    } else {
        va_list pArg;
        va_start(pArg, serial);
        char  *ipDest   = va_arg(pArg, char *);
        short  portDest = (short)va_arg(pArg, int);
        va_end(pArg);
        envoyerDGRAM(sockEch, buff, ipDest, portDest);
    }
}

void mcs_recevoir(socket_t *sockEch, mcs_generic quoi, mcs_pFct deSerial) {
    mcs_buffer_t buff;
    if (sockEch->mode == SOCK_STREAM)
        recevoirSTREAM(sockEch, buff, MCS_MAX_BUFFER);
    else
        recevoirDGRAM(sockEch, buff, MCS_MAX_BUFFER);

    if (deSerial != NULL)
        deSerial(buff, quoi);
    else
        strcpy((char *)quoi, buff);
}
