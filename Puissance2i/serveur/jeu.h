/*
Nom du fichier : jeu.h
Auteurs : Erwhann Deiss, Dorian Pazdziej, Matteo Delattre
Description : en-tete de la logique du jeu puissance 4
*/

#ifndef JEU_H
#define JEU_H
// inclusion des structures partagees (PartieInfo)
#include "../commun/structures.h"

// initialise la grille de jeu avec des cases vides (0)
void init_grille(int grille[6][7]);
// place le jeton du joueur dans la colonne indiquee (gravite simulee)
// retourne 1 si le coup est valide, 0 si colonne pleine ou invalide
int jouer_coup(int grille[6][7], int colonne, int id_joueur);
// verifie si le joueur a aligne 4 jetons
// retourne 1 si victoire, 0 sinon
int verifier_victoire(int grille[6][7], int id_joueur);
// verifie si la grille est entierement remplie (match nul)
// retourne 1 si match nul, 0 s'il reste des cases libres
int verifier_match_nul(int grille[6][7]);

#endif
