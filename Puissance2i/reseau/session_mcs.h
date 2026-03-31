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

/* -----------------------------------------------------------------------
 * Macros utilitaires
 * ----------------------------------------------------------------------- */

/** Verifie que sts != -1 ; affiche msg et quitte sinon. */
#define MCS_CHECK(sts, msg) if ((sts)==-1) { perror(msg); exit(-1); }

/** Affiche msg et attend une entree clavier. */
#define MCS_PAUSE(msg) printf("%s [Appuyez sur entree pour continuer]", msg); getchar();

/* -----------------------------------------------------------------------
 * Structure socket_t
 * ----------------------------------------------------------------------- */

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

/* -----------------------------------------------------------------------
 * Prototypes
 * ----------------------------------------------------------------------- */

/** Remplit une sockaddr_in a partir d'une IP et d'un port. */
void mcs_adr2struct(struct sockaddr_in *addr, char *adrIP, short port);

/** Cree une socket TCP ou UDP (sans adresse). */
socket_t mcs_creerSocket(int mode);

/** Cree une socket liee a une adresse locale. */
socket_t mcs_creerSocketAdr(int mode, char *adrIP, short port);

/** Cree une socket d'ecoute TCP prete a accepter des clients. */
socket_t mcs_creerSocketEcoute(char *adrIP, short port);

/** Accepte un client en attente sur une socket d'ecoute. */
socket_t mcs_accepterClt(const socket_t sockEcoute);

/** Cree une socket TCP et se connecte au serveur indique. */
socket_t mcs_connecterClt2Srv(char *adrIP, short port);

#endif /* SESSION_MCS_H */
