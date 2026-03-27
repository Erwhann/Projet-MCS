/* =========================================================
 * profil.h -- Persistance du profil joueur en fichier binaire
 *
 * Format : struct ProfilSauvegarde sérialisée telle quelle
 * (portable entre exécutions sur la même machine).
 * ========================================================= */
#ifndef PROFIL_H
#define PROFIL_H

/* Données persistantes du joueur (sous-ensemble de ClientInfo) */
typedef struct {
    char  pseudo[32];
    int   elo;
    int   score;
    int   nb_victoires;
    int   nb_defaites;
    int   nb_nuls;
    int   amis[50];         /* IDs persistants des amis (pour persistance entre sessions) */
    char  pseudos_amis[50][32]; /* Pseudos correspondants */
    int   nb_amis;
} ProfilSauvegarde;

/**
 * Charge un profil depuis `chemin`.
 * Retourne 1 si le fichier existait et a été lu, 0 sinon (nouveau joueur).
 */
int charger_profil(const char *chemin, ProfilSauvegarde *ps);

/**
 * Sauvegarde le profil dans `chemin`.
 * Retourne 1 si succès, 0 sinon.
 */
int sauvegarder_profil(const char *chemin, const ProfilSauvegarde *ps);

#endif /* PROFIL_H */
