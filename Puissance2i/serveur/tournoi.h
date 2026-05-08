/*
Nom du fichier : tournoi.h
Auteurs : Erwhann Deiss, Dorian Pazdziej, Matteo Delattre
Description : en-tete du module de gestion du tournoi du jeu puissance 2i
*/

#ifndef TOURNOI_H
#define TOURNOI_H

// inclusion des structures partagees (PayloadTournamentState, MAX_JOUEURS_TOURNOI)
#include "../commun/structures.h"

// enumeration des etats possibles d'un tournoi
typedef enum {
    TOURNOI_ATTENTE      = 0,   // attente des joueurs (inscription ouverte)
    TOURNOI_DEMI_FINALES = 1,   // demi-finales en cours
    TOURNOI_FINALE       = 2,   // finale en cours
    TOURNOI_TERMINE      = 3    // tournoi termine
} EtatTournoi;

// structure interne representant l'etat complet du tournoi cote serveur
typedef struct {
    EtatTournoi etat;                               // phase actuelle du tournoi
    int  nb_joueurs;                                // nombre de joueurs inscrits
    int  ids[MAX_JOUEURS_TOURNOI];                  // identifiants des joueurs
    char pseudos[MAX_JOUEURS_TOURNOI][32];          // pseudos des joueurs
    int  elos[MAX_JOUEURS_TOURNOI];                 // ELO des joueurs

    int match_j1[3];    // joueur 1 de chaque match (SF1, SF2, Finale)
    int match_j2[3];    // joueur 2 de chaque match (SF1, SF2, Finale)
    int vainqueur[3];   // vainqueur de chaque match (0 si pas encore joue)
    int id_partie[3];   // identifiant de partie de chaque match du bracket

    int id_vainqueur_final; // identifiant du vainqueur du tournoi
} TournoiInfo;

// variable globale de l'etat du tournoi (accessible depuis serveur.c)
extern TournoiInfo g_tournoi;

// remet le tournoi a zero (etat d'attente, aucun joueur)
void tournoi_reset(void);

// inscrit un joueur dans le tournoi, retourne 1 si succes, 0 si echec
int tournoi_rejoindre(int id_client, const char *pseudo, int elo);

// retire un joueur du tournoi (seulement si encore en phase d'attente)
void tournoi_quitter(int id_client);

// retourne 1 si le tournoi a atteint son nombre maximal de joueurs
int tournoi_pret(void);

// lance les demi-finales et retourne les affrontements via les pointeurs
void tournoi_demarrer(int *sf1_j1, int *sf1_j2, int *sf2_j1, int *sf2_j2);

// associe un identifiant de partie a un match du bracket
void tournoi_enregistrer_partie(int match_idx, int id_partie);

// traite la fin d'un match de tournoi et fait avancer le bracket
// retourne 1 si la finale doit etre lancee (next_j1, next_j2 remplis)
int tournoi_notifier_fin_partie(int id_partie, int id_vainqueur,
                                 int *next_j1, int *next_j2);

// retourne l'index du match dans le bracket pour un id_partie donne (-1 si inconnu)
int tournoi_get_match_idx(int id_partie);

// construit le payload PayloadTournamentState a envoyer aux clients
void tournoi_construire_etat(PayloadTournamentState *out);

// retourne 1 si le joueur est inscrit dans le tournoi, 0 sinon
int tournoi_est_participant(int id_client);

#endif
