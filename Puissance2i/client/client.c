/*
Nom du programme : client.c
Auteurs : Erwhann Deiss, Dorian Pazdziej, Matteo Delattre
Description : programme client du jeu puissance 2i qui permet de se connecter au
serveur
*/

// inclusions des bibliothèques nécessaires
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#include "../commun/profil.h"
#include "../commun/protocol.h"
#include "../commun/structures.h"
#include "../reseau/data.h"
#include "../reseau/session.h"
#include "ihm.h"

// port par défaut
#define SRV_PORT_DEF 5000

// chemin du fichier profil
static char g_chemin_profil[256] = {0};
static ClientInfo g_moi;

// fonction de sauvegarde du profil
static void sauvegarder(void) {
  // si le chemin du fichier profil est vide, on ne sauvegarde pas
  if (g_chemin_profil[0] == '\0')
    return;
  // on copie les données du profil dans une structure de sauvegarde
  ProfilSauvegarde ps;
  // on met à 0 la structure de sauvegarde pour éviter les erreurs
  memset(&ps, 0, sizeof(ps));
  // on copie les données du profil dans la structure de sauvegarde
  strncpy(ps.pseudo, g_moi.pseudo, 31);

  // on copie les données du profil dans la structure de sauvegarde
  ps.elo = g_moi.elo;
  ps.score = g_moi.score;
  ps.nb_victoires = g_moi.nb_victoires;
  ps.nb_defaites = g_moi.nb_defaites;
  ps.nb_nuls = g_moi.nb_nuls;
  ps.nb_amis = g_moi.nb_amis;
  // on copie les amis
  for (int i = 0; i < g_moi.nb_amis && i < MAX_AMIS; i++)
    ps.amis[i] = g_moi.amis[i];
  // on sauvegarde le profil
  sauvegarder_profil(g_chemin_profil, &ps);
}

// fonction principale qui va permettre de lancer le client
int main(int argc, char **argv) {
  // si il n'y a pas d'argument, on affiche l'usage
  if (argc < 2) {
    fprintf(stderr, "Usage : %s <IP_SERVEUR> [PORT] [profil.dat]\n", argv[0]);
    return 1;
  }
  // on definit les variables
  const char *ip = argv[1];
  int port = (argc >= 3) ? atoi(argv[2]) : SRV_PORT_DEF;
  const char *fichier_profil = (argc >= 4) ? argv[3] : NULL;

  // on met à 0 la structure de sauvegarde pour éviter les erreurs
  ProfilSauvegarde ps_sauve;
  memset(&ps_sauve, 0, sizeof(ps_sauve));
  // si il y a un fichier profil, on charge le profil
  int profil_charge = 0;
  // on charge le profil
  if (fichier_profil) {
    // on copie le chemin du fichier profil
    strncpy(g_chemin_profil, fichier_profil, 255);
    profil_charge = charger_profil(fichier_profil, &ps_sauve);
  }
  // on se connecte au serveur
  int sock = connecter_serveur(ip, port);
  // si la connexion echoue, on affiche un message d'erreur
  if (sock < 0) {
    fprintf(stderr, "Erreur : Serveur inaccessible (%s:%d). Fermeture.\n", ip,
            port);
    return 1;
  }
  // on initialise l'interface graphique
  init_ihm();
  // on demande le pseudo
  char pseudo[32] = {0};
  demander_pseudo_ncurses(pseudo, profil_charge ? ps_sauve.pseudo : "");
  // si le pseudo est vide, on affiche un message d'erreur
  if (pseudo[0] == '\0') {
    if (profil_charge)
      strncpy(pseudo, ps_sauve.pseudo, 31);
    else {
      fin_ihm();
      fprintf(stderr, "Pseudo vide.\n");
      return 1;
    }
  }
  // on se connecte au serveur avec le pseudo
  PayloadLogin pl;
  strncpy(pl.pseudo, pseudo, 31);
  pl.pseudo[31] = '\0';
  envoyer_message(sock, REQ_LOGIN, &pl, sizeof(pl));
  // on recoit le message du serveur
  Header h;
  void *payload = NULL;
  // on verifie si la connexion a reussi
  if (recevoir_message(sock, &h, &payload) < 0 || h.type != RES_LOGIN_OK) {
    fin_ihm();
    // si la connexion a echoue, on affiche un message d'erreur
    fprintf(stderr, "Connexion refusée (pseudo déjà utilisé ?). Fermeture.\n");
    // si il y a un payload, on le free
    if (payload)
      free(payload);
    fermer_socket(sock);
    return 1;
  }
  // on initialise les variables
  memset(&g_moi, 0, sizeof(g_moi));
  strncpy(g_moi.pseudo, pseudo, 31);
  g_moi.etat = ETAT_MENU;
  // si le message est un message de connexion reussie
  if (h.type == RES_LOGIN_OK) {
    PayloadLoginOK *pok = (PayloadLoginOK *)payload;
    // on recoit l'id du joueur
    g_moi.id = pok->id_joueur;
    // on recoit l'elo du joueur
    g_moi.elo = profil_charge ? ps_sauve.elo : pok->elo;
    // on recoit le score du joueur
    g_moi.score = profil_charge ? ps_sauve.score : pok->score;
    // si il y a un profil charge, on recoit les donnees du profil
    if (profil_charge) {
      g_moi.nb_victoires = ps_sauve.nb_victoires;
      g_moi.nb_defaites = ps_sauve.nb_defaites;
      g_moi.nb_nuls = ps_sauve.nb_nuls;
      g_moi.nb_amis = ps_sauve.nb_amis;
      for (int i = 0; i < ps_sauve.nb_amis && i < MAX_AMIS; i++)
        g_moi.amis[i] = ps_sauve.amis[i];
    }
  }
  // si il y a un payload, on le free
  if (payload) {
    free(payload);
    payload = NULL;
  }
  // on initialise les variables
  PartieInfo partie_courante = {0};
  PayloadFriendList amis = {0};
  int etat_ihm = IHM_MENU;
  int challenge_en_attente = 0;
  int challenger_id = 0;
  // on affiche le menu
  dessiner_menu(&g_moi);
  // on boucle tant que la connexion n'est pas perdue
  while (1) {
    // on initialise les variables
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(sock, &rfds);
    struct timeval tv = {0, 0};
    // on verifie si il y a un message du serveur
    if (select(sock + 1, &rfds, NULL, NULL, &tv) > 0) {
      // on recoit le message
      Header nh;
      void *np = NULL;
      // si la connexion est perdue, on affiche un message d'erreur
      if (recevoir_message(sock, &nh, &np) < 0) {
        dessiner_notification("Connexion perdue avec le serveur.");
        break;
      }
      // on traite le message
      traiter_message_serveur(&nh, np, &g_moi, &partie_courante, &amis,
                              &etat_ihm, &challenge_en_attente, &challenger_id);
      // si il y a un payload, on le free
      if (np)
        free(np);
    }
    // on recoit la saisie de l'utilisateur
    int ch = getch();
    // si il y a une saisie
    if (ch != ERR) {
      // on traite la saisie
      traiter_saisie(ch, &g_moi, &partie_courante, &amis, &etat_ihm,
                     &challenge_en_attente, challenger_id, sock);
    }
  }
  // on sauvegarde le profil
  sauvegarder();
  // on ferme l'ihm
  fin_ihm();
  // on ferme la connexion
  fermer_socket(sock);
  // on retourne 0
  return 0;
}
