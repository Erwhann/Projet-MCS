#include "elo.h"
#include <math.h>

void calculer_elo(int elo_vainqueur, int elo_perdant, int* nv_elo_v, int* nv_elo_p, int match_nul) {
    float k = 32.0;
    float proba_v = 1.0 / (1.0 + pow(10, (elo_perdant - elo_vainqueur) / 400.0));
    float proba_p = 1.0 / (1.0 + pow(10, (elo_vainqueur - elo_perdant) / 400.0));
    
    float score_v = match_nul ? 0.5 : 1.0;
    float score_p = match_nul ? 0.5 : 0.0;
    
    *nv_elo_v = elo_vainqueur + (int)(k * (score_v - proba_v));
    *nv_elo_p = elo_perdant + (int)(k * (score_p - proba_p));
}
