
#ifndef PROFIL_H
#define PROFIL_H

typedef struct {
    char  pseudo[32];
    int   elo;
    int   score;
    int   nb_victoires;
    int   nb_defaites;
    int   nb_nuls;
    int   amis[50];      
    char  pseudos_amis[50][32];
    int   nb_amis;
} ProfilSauvegarde;

int charger_profil(const char *chemin, ProfilSauvegarde *ps);

int sauvegarder_profil(const char *chemin, const ProfilSauvegarde *ps);

#endif 
