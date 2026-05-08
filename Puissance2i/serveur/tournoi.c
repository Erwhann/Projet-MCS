/*
Nom du programme : tournoi.c
Auteurs : Erwhann Deiss, Dorian Pazdziej, Matteo Delattre
Description : gestion du tournoi a elimination directe du jeu puissance 2i
             (inscription, demarrage, avancement du bracket, fin de tournoi)
*/

// inclusions des bibliothèques nécessaires
#include "tournoi.h"
#include <string.h>
#include <stdio.h>

// variable globale contenant l'etat courant du tournoi
TournoiInfo g_tournoi;

// fonction tournoi_reset : remet le tournoi a zero (etat initial)
void tournoi_reset(void) {
    memset(&g_tournoi, 0, sizeof(g_tournoi));
    // on initialise les identifiants de partie a -1 (aucune partie)
    for (int i = 0; i < 3; i++)
        g_tournoi.id_partie[i] = -1;
    g_tournoi.etat = TOURNOI_ATTENTE;
}

// fonction tournoi_rejoindre : inscrit un joueur dans le tournoi
// retourne 1 si l'inscription a reussi, 0 sinon (tournoi demarre, plein, deja inscrit)
int tournoi_rejoindre(int id_client, const char *pseudo, int elo) {
    // on ne peut rejoindre que si le tournoi est en attente
    if (g_tournoi.etat != TOURNOI_ATTENTE)
        return 0;
    // on verifie que le tournoi n'est pas complet
    if (g_tournoi.nb_joueurs >= MAX_JOUEURS_TOURNOI)
        return 0;
    // on verifie que le joueur n'est pas deja inscrit
    for (int i = 0; i < g_tournoi.nb_joueurs; i++)
        if (g_tournoi.ids[i] == id_client) return 0;

    // on ajoute le joueur au tournoi
    int n = g_tournoi.nb_joueurs;
    g_tournoi.ids[n] = id_client;
    strncpy(g_tournoi.pseudos[n], pseudo, 31);
    g_tournoi.pseudos[n][31] = '\0';
    g_tournoi.elos[n] = elo;
    g_tournoi.nb_joueurs++;
    printf("[TOURNOI] %s inscrit (%d/%d)\n", pseudo,
           g_tournoi.nb_joueurs, MAX_JOUEURS_TOURNOI);
    return 1;
}

// fonction tournoi_pret : verifie si le tournoi a atteint son nombre maximum de joueurs
// retourne 1 si le tournoi peut demarrer, 0 sinon
int tournoi_pret(void) {
    return g_tournoi.nb_joueurs == MAX_JOUEURS_TOURNOI;
}

// fonction tournoi_quitter : retire un joueur du tournoi (seulement en phase d'attente)
void tournoi_quitter(int id_client) {
    if (g_tournoi.etat != TOURNOI_ATTENTE) return;
    for (int i = 0; i < g_tournoi.nb_joueurs; i++) {
        if (g_tournoi.ids[i] == id_client) {
            // on decale les joueurs suivants pour combler le trou
            for (int j = i; j < g_tournoi.nb_joueurs - 1; j++) {
                g_tournoi.ids[j] = g_tournoi.ids[j+1];
                strncpy(g_tournoi.pseudos[j], g_tournoi.pseudos[j+1], 32);
                g_tournoi.elos[j] = g_tournoi.elos[j+1];
            }
            g_tournoi.nb_joueurs--;
            printf("[TOURNOI] Joueur ID=%d retire. Reste (%d/%d)\n", id_client, g_tournoi.nb_joueurs, MAX_JOUEURS_TOURNOI);
            break;
        }
    }
}

// fonction tournoi_demarrer : lance le tournoi en definissant les demi-finales
// joueurs[0] vs joueurs[1] en SF1, joueurs[2] vs joueurs[3] en SF2
void tournoi_demarrer(int *sf1_j1, int *sf1_j2, int *sf2_j1, int *sf2_j2) {
    g_tournoi.etat = TOURNOI_DEMI_FINALES;

    // on definit les affrontements des demi-finales
    g_tournoi.match_j1[0] = g_tournoi.ids[0];
    g_tournoi.match_j2[0] = g_tournoi.ids[1];

    g_tournoi.match_j1[1] = g_tournoi.ids[2];
    g_tournoi.match_j2[1] = g_tournoi.ids[3];

    // on retourne les identifiants des joueurs via les pointeurs
    *sf1_j1 = g_tournoi.match_j1[0];
    *sf1_j2 = g_tournoi.match_j2[0];
    *sf2_j1 = g_tournoi.match_j1[1];
    *sf2_j2 = g_tournoi.match_j2[1];

    printf("[TOURNOI] Demi-finales lancees : %d vs %d  |  %d vs %d\n",
           *sf1_j1, *sf1_j2, *sf2_j1, *sf2_j2);
}

// fonction tournoi_enregistrer_partie : associe une partie a un match du bracket
void tournoi_enregistrer_partie(int match_idx, int id_partie) {
    if (match_idx >= 0 && match_idx < 3)
        g_tournoi.id_partie[match_idx] = id_partie;
}

// fonction tournoi_get_match_idx : retourne l'index du match dans le bracket
// pour l'identifiant de partie donne, -1 si la partie ne fait pas partie du tournoi
int tournoi_get_match_idx(int id_partie) {
    for (int i = 0; i < 3; i++)
        if (g_tournoi.id_partie[i] == id_partie) return i;
    return -1;
}

// fonction tournoi_notifier_fin_partie : traite la fin d'un match de tournoi
// met a jour le bracket et prepare la prochaine partie si necessaire
// retourne 1 si une finale doit etre lancee (next_j1 et next_j2 remplis), 0 sinon
int tournoi_notifier_fin_partie(int id_partie, int id_vainqueur,
                                 int *next_j1, int *next_j2) {
    int midx = tournoi_get_match_idx(id_partie);
    if (midx == -1) return 0;

    // on enregistre le vainqueur du match
    g_tournoi.vainqueur[midx] = id_vainqueur;
    printf("[TOURNOI] Match %d termine, vainqueur ID=%d\n", midx, id_vainqueur);

    // si les deux demi-finales sont terminees, on lance la finale
    if (midx == 0 || midx == 1) {
        if (g_tournoi.vainqueur[0] != 0 && g_tournoi.vainqueur[1] != 0) {
            g_tournoi.etat = TOURNOI_FINALE;
            g_tournoi.match_j1[2] = g_tournoi.vainqueur[0];
            g_tournoi.match_j2[2] = g_tournoi.vainqueur[1];
            *next_j1 = g_tournoi.match_j1[2];
            *next_j2 = g_tournoi.match_j2[2];
            printf("[TOURNOI] Finale : ID %d vs ID %d\n", *next_j1, *next_j2);
            return 1;   // la finale doit etre lancee
        }
        return 0;   // on attend la fin de l'autre demi-finale
    }

    // si la finale est terminee, on cloture le tournoi
    if (midx == 2) {
        g_tournoi.etat = TOURNOI_TERMINE;
        g_tournoi.id_vainqueur_final = id_vainqueur;
        printf("[TOURNOI] Vainqueur du tournoi : ID=%d\n", id_vainqueur);
        return 0;
    }

    return 0;
}

// fonction tournoi_construire_etat : construit le payload d'etat du tournoi
// a envoyer aux clients participants
void tournoi_construire_etat(PayloadTournamentState *out) {
    memset(out, 0, sizeof(*out));
    out->etat       = (int)g_tournoi.etat;
    out->nb_joueurs = g_tournoi.nb_joueurs;

    // on copie les donnees des participants
    for (int i = 0; i < g_tournoi.nb_joueurs; i++) {
        strncpy(out->pseudos[i], g_tournoi.pseudos[i], 31);
        out->pseudos[i][31] = '\0';
        out->elos[i] = g_tournoi.elos[i];
    }

    // on copie les vainqueurs des demi-finales et de la finale
    out->vainqueur_sf1   = g_tournoi.vainqueur[0];
    out->vainqueur_sf2   = g_tournoi.vainqueur[1];
    out->vainqueur_final = g_tournoi.vainqueur[2];

    // on construit le message d'etat selon la phase du tournoi
    switch (g_tournoi.etat) {
    case TOURNOI_ATTENTE:
        snprintf(out->msg, sizeof(out->msg), "En attente : %d/%d joueurs",
                 g_tournoi.nb_joueurs, MAX_JOUEURS_TOURNOI);
        break;
    case TOURNOI_DEMI_FINALES:
        snprintf(out->msg, sizeof(out->msg), "Demi-finales en cours...");
        break;
    case TOURNOI_FINALE:
        snprintf(out->msg, sizeof(out->msg), "FINALE en cours !");
        break;
    case TOURNOI_TERMINE: {
        // on retrouve le pseudo du vainqueur final
        const char *pseudo_v = "?";
        for (int i = 0; i < g_tournoi.nb_joueurs; i++) {
            if (g_tournoi.ids[i] == g_tournoi.id_vainqueur_final) {
                pseudo_v = g_tournoi.pseudos[i];
                break;
            }
        }
        snprintf(out->msg, sizeof(out->msg), "Vainqueur : %s !", pseudo_v);
        break;
    }
    }
}

// fonction tournoi_est_participant : verifie si un joueur est inscrit dans le tournoi
// retourne 1 si le joueur est participant, 0 sinon
int tournoi_est_participant(int id_client) {
    for (int i = 0; i < g_tournoi.nb_joueurs; i++)
        if (g_tournoi.ids[i] == id_client) return 1;
    return 0;
}
