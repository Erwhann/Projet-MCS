#include "jeu.h"
#include <stdio.h>

void init_grille(int grille[6][7]) {
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 7; j++) {
            grille[i][j] = 0;
        }
    }
}

int jouer_coup(int grille[6][7], int colonne, int id_joueur) {
    if (colonne < 0 || colonne >= 7) return 0; // Col invalide
    for (int i = 5; i >= 0; i--) { // De bas en haut
        if (grille[i][colonne] == 0) {
            grille[i][colonne] = id_joueur;
            return 1; // Succès
        }
    }
    return 0; // Colonne pleine
}

int verifier_victoire(int grille[6][7], int id_joueur) {
    // Horizontale
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 4; j++) {
            if (grille[i][j] == id_joueur && grille[i][j+1] == id_joueur && 
                grille[i][j+2] == id_joueur && grille[i][j+3] == id_joueur)
                return 1;
        }
    }
    // Verticale
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 7; j++) {
            if (grille[i][j] == id_joueur && grille[i+1][j] == id_joueur && 
                grille[i+2][j] == id_joueur && grille[i+3][j] == id_joueur)
                return 1;
        }
    }
    // Diagonale droite
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            if (grille[i][j] == id_joueur && grille[i+1][j+1] == id_joueur && 
                grille[i+2][j+2] == id_joueur && grille[i+3][j+3] == id_joueur)
                return 1;
        }
    }
    // Diagonale gauche
    for (int i = 0; i < 3; i++) {
        for (int j = 3; j < 7; j++) {
            if (grille[i][j] == id_joueur && grille[i+1][j-1] == id_joueur && 
                grille[i+2][j-2] == id_joueur && grille[i+3][j-3] == id_joueur)
                return 1;
        }
    }
    return 0;
}

int verifier_match_nul(int grille[6][7]) {
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 7; j++) {
            if (grille[i][j] == 0) return 0; // Il reste des cases
        }
    }
    return 1; // Plein
}
