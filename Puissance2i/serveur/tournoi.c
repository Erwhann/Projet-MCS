
#include "tournoi.h"
#include <string.h>
#include <stdio.h>

TournoiInfo g_tournoi;

void tournoi_reset(void) {
    memset(&g_tournoi, 0, sizeof(g_tournoi));
    for (int i = 0; i < 3; i++)
        g_tournoi.id_partie[i] = -1;
    g_tournoi.etat = TOURNOI_ATTENTE;
}

int tournoi_rejoindre(int id_client, const char *pseudo, int elo) {
    if (g_tournoi.etat != TOURNOI_ATTENTE)
        return 0;
    if (g_tournoi.nb_joueurs >= MAX_JOUEURS_TOURNOI)
        return 0;
    for (int i = 0; i < g_tournoi.nb_joueurs; i++)
        if (g_tournoi.ids[i] == id_client) return 0;

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

int tournoi_pret(void) {
    return g_tournoi.nb_joueurs == MAX_JOUEURS_TOURNOI;
}

void tournoi_quitter(int id_client) {
    if (g_tournoi.etat != TOURNOI_ATTENTE) return;
    for (int i = 0; i < g_tournoi.nb_joueurs; i++) {
        if (g_tournoi.ids[i] == id_client) {
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

void tournoi_demarrer(int *sf1_j1, int *sf1_j2, int *sf2_j1, int *sf2_j2) {
    g_tournoi.etat = TOURNOI_DEMI_FINALES;

    g_tournoi.match_j1[0] = g_tournoi.ids[0];
    g_tournoi.match_j2[0] = g_tournoi.ids[1];

    g_tournoi.match_j1[1] = g_tournoi.ids[2];
    g_tournoi.match_j2[1] = g_tournoi.ids[3];

    *sf1_j1 = g_tournoi.match_j1[0];
    *sf1_j2 = g_tournoi.match_j2[0];
    *sf2_j1 = g_tournoi.match_j1[1];
    *sf2_j2 = g_tournoi.match_j2[1];

    printf("[TOURNOI] Demi-finales lancees : %d vs %d  |  %d vs %d\n",
           *sf1_j1, *sf1_j2, *sf2_j1, *sf2_j2);
}

void tournoi_enregistrer_partie(int match_idx, int id_partie) {
    if (match_idx >= 0 && match_idx < 3)
        g_tournoi.id_partie[match_idx] = id_partie;
}

int tournoi_get_match_idx(int id_partie) {
    for (int i = 0; i < 3; i++)
        if (g_tournoi.id_partie[i] == id_partie) return i;
    return -1;
}

int tournoi_notifier_fin_partie(int id_partie, int id_vainqueur,
                                 int *next_j1, int *next_j2) {
    int midx = tournoi_get_match_idx(id_partie);
    if (midx == -1) return 0;

    g_tournoi.vainqueur[midx] = id_vainqueur;
    printf("[TOURNOI] Match %d termine, vainqueur ID=%d\n", midx, id_vainqueur);

    if (midx == 0 || midx == 1) {
        if (g_tournoi.vainqueur[0] != 0 && g_tournoi.vainqueur[1] != 0) {
            g_tournoi.etat = TOURNOI_FINALE;
            g_tournoi.match_j1[2] = g_tournoi.vainqueur[0];
            g_tournoi.match_j2[2] = g_tournoi.vainqueur[1];
            *next_j1 = g_tournoi.match_j1[2];
            *next_j2 = g_tournoi.match_j2[2];
            printf("[TOURNOI] Finale : ID %d vs ID %d\n", *next_j1, *next_j2);
            return 1;
        }
        return 0;
    }

    if (midx == 2) {
        g_tournoi.etat = TOURNOI_TERMINE;
        g_tournoi.id_vainqueur_final = id_vainqueur;
        printf("[TOURNOI] Vainqueur du tournoi : ID=%d\n", id_vainqueur);
        return 0;
    }

    return 0;
}

void tournoi_construire_etat(PayloadTournamentState *out) {
    memset(out, 0, sizeof(*out));
    out->etat       = (int)g_tournoi.etat;
    out->nb_joueurs = g_tournoi.nb_joueurs;

    for (int i = 0; i < g_tournoi.nb_joueurs; i++) {
        strncpy(out->pseudos[i], g_tournoi.pseudos[i], 31);
        out->pseudos[i][31] = '\0';
        out->elos[i] = g_tournoi.elos[i];
    }

    out->vainqueur_sf1   = g_tournoi.vainqueur[0];
    out->vainqueur_sf2   = g_tournoi.vainqueur[1];
    out->vainqueur_final = g_tournoi.vainqueur[2];

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

int tournoi_est_participant(int id_client) {
    for (int i = 0; i < g_tournoi.nb_joueurs; i++)
        if (g_tournoi.ids[i] == id_client) return 1;
    return 0;
}
