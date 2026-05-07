
#include "profil.h"
#include <stdio.h>
#include <string.h>

int charger_profil(const char *chemin, ProfilSauvegarde *ps) {
    FILE *f = fopen(chemin, "rb");
    if (!f) return 0; 

    size_t lu = fread(ps, sizeof(ProfilSauvegarde), 1, f);
    fclose(f);
    return (lu == 1) ? 1 : 0;
}

int sauvegarder_profil(const char *chemin, const ProfilSauvegarde *ps) {
    FILE *f = fopen(chemin, "wb");
    if (!f) return 0;

    size_t ecrit = fwrite(ps, sizeof(ProfilSauvegarde), 1, f);
    fclose(f);
    return (ecrit == 1) ? 1 : 0;
}
