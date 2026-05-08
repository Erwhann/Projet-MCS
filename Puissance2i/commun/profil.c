/*
Nom du programme : profil.c
Auteurs : Erwhann Deiss, Dorian Pazdziej, Matteo Delattre
Description : gestion de la sauvegarde et du chargement des profils joueur
*/

// inclusions des bibliothèques nécessaires
#include "profil.h"
#include <stdio.h>
#include <string.h>

// fonction charger_profil : charge un profil depuis un fichier binaire
// retourne 1 si le chargement a reussi, 0 sinon
int charger_profil(const char *chemin, ProfilSauvegarde *ps) {
    // on ouvre le fichier en lecture binaire
    FILE *f = fopen(chemin, "rb");
    if (!f) return 0;

    // on lit la structure entiere
    size_t lu = fread(ps, sizeof(ProfilSauvegarde), 1, f);
    fclose(f);
    return (lu == 1) ? 1 : 0;
}

// fonction sauvegarder_profil : sauvegarde un profil dans un fichier binaire
// retourne 1 si la sauvegarde a reussi, 0 sinon
int sauvegarder_profil(const char *chemin, const ProfilSauvegarde *ps) {
    // on ouvre le fichier en ecriture binaire
    FILE *f = fopen(chemin, "wb");
    if (!f) return 0;

    // on ecrit la structure entiere
    size_t ecrit = fwrite(ps, sizeof(ProfilSauvegarde), 1, f);
    fclose(f);
    return (ecrit == 1) ? 1 : 0;
}
