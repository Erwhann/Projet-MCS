#ifndef JEU_H
#define JEU_H
#include "../commun/structures.h"

void init_grille(int grille[6][7]);
int jouer_coup(int grille[6][7], int colonne, int id_joueur);
int verifier_victoire(int grille[6][7], int id_joueur);
int verifier_match_nul(int grille[6][7]);

#endif
