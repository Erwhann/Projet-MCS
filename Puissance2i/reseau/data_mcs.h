/**
 * \file        data_mcs.h
 * \brief       Specification de la couche Data Representation (bibliotheque MCS)
 * \author      Samir El Khattabi / Erwhann D
 * \date        3 mars 2023 / 31 mars 2026
 * \version     1.0
 *
 * Copie de la librairie MCS integree dans Puissance2i/reseau/.
 * Fournit les fonctions d'envoi/reception generiques (STREAM et DGRAM)
 * basees sur le type socket_t de session_mcs.h.
 */
#ifndef DATA_MCS_H
#define DATA_MCS_H

#include "session_mcs.h"

#define MCS_MAX_BUFFER 1024

typedef char mcs_buffer_t[MCS_MAX_BUFFER];

typedef void *mcs_generic;

typedef void (*mcs_pFct)(mcs_generic, mcs_generic);


/**
 * Envoie une requete/reponse sur une socket (STREAM ou DGRAM).
 *
 * \param sockEch  Socket d'echange a utiliser pour l'envoi.
 * \param quoi     Requete/reponse a serialiser (ou chaine si serial==NULL).
 * \param serial   Pointeur sur la fonction de serialisation, ou NULL si
 *                 quoi est directement une chaine de caracteres.
 * \note  En mode DGRAM, appeler avec deux arguments supplementaires :
 *        char *adrDest, short portDest.
 */
void mcs_envoyer(socket_t *sockEch, mcs_generic quoi, mcs_pFct serial, ...);

/**
 * Recoit une requete/reponse sur une socket (STREAM ou DGRAM).
 *
 * \param sockEch   Socket d'echange a utiliser pour la reception.
 * \param quoi      Zone memoire dans laquelle deserialiser les donnees recues.
 * \param deSerial  Pointeur sur la fonction de deserialisation, ou NULL si
 *                  quoi est directement une chaine de caracteres.
 */
void mcs_recevoir(socket_t *sockEch, mcs_generic quoi, mcs_pFct deSerial);

#endif 
