/**
 * \file        session_mcs.h
 * \brief       Specification de la couche Session (bibliotheque MCS)
 * \author      Samir El Khattabi / Erwhann D
 * \date        3 mars 2023 / 31 mars 2026
 * \version     1.0
 *
 * Copie de la librairie MCS integree dans Puissance2i/reseau/.
 * Fournit le type socket_t et les primitives de creation/connexion de sockets.
 */
#ifndef SESSION_MCS_H
#define SESSION_MCS_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>


#define MCS_CHECK(sts, msg) if ((sts)==-1) { perror(msg); exit(-1); }

#define MCS_PAUSE(msg) printf("%s [Appuyez sur entree pour continuer]", msg); getchar();

/**
 * \struct  socket_mcs
 * \brief   Socket avec mode, addresses locale et distante.
 */
struct socket_mcs {
    int fd;                       /**< descripteur de la socket           */
    int mode;                     /**< SOCK_STREAM ou SOCK_DGRAM          */
    struct sockaddr_in addrLoc;   /**< adresse locale de la socket        */
    struct sockaddr_in addrDst;   /**< adresse distante de la socket      */
};

typedef struct socket_mcs socket_t;


void mcs_adr2struct(struct sockaddr_in *addr, char *adrIP, short port);

socket_t mcs_creerSocket(int mode);

socket_t mcs_creerSocketAdr(int mode, char *adrIP, short port);

socket_t mcs_creerSocketEcoute(char *adrIP, short port);

socket_t mcs_accepterClt(const socket_t sockEcoute);

socket_t mcs_connecterClt2Srv(char *adrIP, short port);

#endif 
