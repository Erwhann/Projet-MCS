/*
Nom du programme : elo.c
Auteurs : Erwhann Deiss, Dorian Pazdziej, Matteo Delattre
Description : calcul du nouveau score ELO des deux joueurs apres une partie
*/

// inclusions des bibliothèques nécessaires
#include "elo.h"
#include <math.h>

// fonction calculer_elo : calcule les nouveaux scores ELO des deux joueurs
// paramètres :
//   elo_vainqueur : ELO du vainqueur (ou joueur 1 en cas de match nul)
//   elo_perdant   : ELO du perdant (ou joueur 2 en cas de match nul)
//   nv_elo_v      : pointeur vers le nouvel ELO du vainqueur (sortie)
//   nv_elo_p      : pointeur vers le nouvel ELO du perdant (sortie)
//   match_nul     : 1 si la partie est nulle, 0 sinon
void calculer_elo(int elo_vainqueur, int elo_perdant, int* nv_elo_v, int* nv_elo_p, int match_nul) {
    float k = 32.0;     // facteur K : amplitude maximale de variation ELO
    // calcul des probabilites de victoire esperees (formule ELO standard)
    float proba_v = 1.0 / (1.0 + pow(10, (elo_perdant - elo_vainqueur) / 400.0));
    float proba_p = 1.0 / (1.0 + pow(10, (elo_vainqueur - elo_perdant) / 400.0));

    // score reel obtenu : 1 en cas de victoire, 0.5 en cas de match nul, 0 en cas de defaite
    float score_v = match_nul ? 0.5 : 1.0;
    float score_p = match_nul ? 0.5 : 0.0;

    // calcul des nouveaux ELO avec la formule : Elo_nouveau = Elo_ancien + K * (score_reel - score_espere)
    *nv_elo_v = elo_vainqueur + (int)(k * (score_v - proba_v));
    *nv_elo_p = elo_perdant + (int)(k * (score_p - proba_p));
}
