/*
Nom du fichier : elo.h
Auteurs : Erwhann Deiss, Dorian Pazdziej, Matteo Delattre
Description : en-tete du module de calcul ELO du jeu puissance 2i
*/

#ifndef ELO_H
#define ELO_H

// calcule les nouveaux ELO du vainqueur et du perdant apres une partie
// si match_nul == 1, les deux joueurs sont traites a egalite
void calculer_elo(int elo_vainqueur, int elo_perdant, int* nv_elo_v, int* nv_elo_p, int match_nul);

#endif
