#ifndef TOURNOI_H
#define TOURNOI_H

#include "../commun/structures.h"

typedef enum {
    TOURNOI_ATTENTE      = 0,  
    TOURNOI_DEMI_FINALES = 1,  
    TOURNOI_FINALE       = 2, 
    TOURNOI_TERMINE      = 3  
} EtatTournoi;


typedef struct {
    EtatTournoi etat;
    int  nb_joueurs;
    int  ids[MAX_JOUEURS_TOURNOI];
    char pseudos[MAX_JOUEURS_TOURNOI][32];
    int  elos[MAX_JOUEURS_TOURNOI];

    int match_j1[3];  
    int match_j2[3];   
    int vainqueur[3];  
    int id_partie[3];  

    int id_vainqueur_final;
} TournoiInfo;

extern TournoiInfo g_tournoi;

void tournoi_reset(void);

int tournoi_rejoindre(int id_client, const char *pseudo, int elo);

void tournoi_quitter(int id_client);

int tournoi_pret(void);

void tournoi_demarrer(int *sf1_j1, int *sf1_j2, int *sf2_j1, int *sf2_j2);

void tournoi_enregistrer_partie(int match_idx, int id_partie);

int tournoi_notifier_fin_partie(int id_partie, int id_vainqueur,
                                 int *next_j1, int *next_j2);

int tournoi_get_match_idx(int id_partie);

void tournoi_construire_etat(PayloadTournamentState *out);

int tournoi_est_participant(int id_client);

#endif
