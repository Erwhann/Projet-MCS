/*
Nom du fichier : profil.h
Auteurs : Erwhann Deiss, Dorian Pazdziej, Matteo Delattre
Description : en-tete de la gestion des profils joueur (sauvegarde sur disque)
*/

#ifndef PROFIL_H
#define PROFIL_H

// structure de sauvegarde du profil d'un joueur
typedef struct {
    char  pseudo[32];       // pseudonyme du joueur
    int   elo;              // score ELO du joueur
    int   score;            // score total du joueur
    int   nb_victoires;     // nombre de victoires
    int   nb_defaites;      // nombre de defaites
    int   nb_nuls;          // nombre de matchs nuls
    int   amis[50];         // tableau des identifiants des amis
    char  pseudos_amis[50][32]; // tableau des pseudos des amis
    int   nb_amis;          // nombre d'amis
} ProfilSauvegarde;

// charge un profil depuis un fichier binaire, retourne 1 si succes
int charger_profil(const char *chemin, ProfilSauvegarde *ps);

// sauvegarde un profil dans un fichier binaire, retourne 1 si succes
int sauvegarder_profil(const char *chemin, const ProfilSauvegarde *ps);

#endif
