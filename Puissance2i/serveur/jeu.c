/*
Nom du programme : jeu.c
Auteurs : Erwhann Deiss, Dorian Pazdziej, Matteo Delattre
Description : logique du jeu puissance 4 (initialisation de la grille,
             jeu de coup, verification de victoire et match nul)
*/

// inclusions des bibliothèques nécessaires
#include "jeu.h"
#include <stdio.h>

// fonction init_grille : initialise la grille de jeu avec des 0
// paramètre : grille a initialiser (tableau 6x7)
void init_grille(int grille[6][7]) {
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 7; j++) {
            grille[i][j] = 0;   // 0 signifie case vide
        }
    }
}

// fonction jouer_coup : place le jeton du joueur dans la colonne donnee
// le jeton tombe au bas de la colonne (gravite simulee)
// retourne 1 si le coup est valide, 0 si la colonne est pleine ou invalide
int jouer_coup(int grille[6][7], int colonne, int id_joueur) {
    if (colonne < 0 || colonne >= 7) return 0;  // colonne hors limites
    // on remonte la colonne pour trouver la premiere case libre
    for (int i = 5; i >= 0; i--) {
        if (grille[i][colonne] == 0) {
            grille[i][colonne] = id_joueur;
            return 1;   // coup joue avec succes
        }
    }
    return 0;   // colonne pleine
}

// fonction verifier_victoire : verifie si le joueur a aligne 4 jetons
// retourne 1 si victoire, 0 sinon
int verifier_victoire(int grille[6][7], int id_joueur) {
    // verification horizontale
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 4; j++) {
            if (grille[i][j] == id_joueur && grille[i][j+1] == id_joueur &&
                grille[i][j+2] == id_joueur && grille[i][j+3] == id_joueur)
                return 1;
        }
    }
    // verification verticale
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 7; j++) {
            if (grille[i][j] == id_joueur && grille[i+1][j] == id_joueur &&
                grille[i+2][j] == id_joueur && grille[i+3][j] == id_joueur)
                return 1;
        }
    }
    // verification diagonale droite (haut-gauche vers bas-droite)
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            if (grille[i][j] == id_joueur && grille[i+1][j+1] == id_joueur &&
                grille[i+2][j+2] == id_joueur && grille[i+3][j+3] == id_joueur)
                return 1;
        }
    }
    // verification diagonale gauche (haut-droite vers bas-gauche)
    for (int i = 0; i < 3; i++) {
        for (int j = 3; j < 7; j++) {
            if (grille[i][j] == id_joueur && grille[i+1][j-1] == id_joueur &&
                grille[i+2][j-2] == id_joueur && grille[i+3][j-3] == id_joueur)
                return 1;
        }
    }
    return 0;
}

// fonction verifier_match_nul : verifie si la grille est entierement remplie
// retourne 1 si match nul (grille pleine), 0 s'il reste des cases libres
int verifier_match_nul(int grille[6][7]) {
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 7; j++) {
            if (grille[i][j] == 0) return 0;   // il reste des cases libres
        }
    }
    return 1;   // grille pleine : match nul
}
