# Tests automatiques — Puissance 2i

Ce dossier contient le client de test (`test_client.c`) qui simule un joueur
automatique pour tester le serveur sans interface graphique.

## Compilation

Depuis la racine du projet :

```bash
make bin/test_client
# ou avec tout le projet :
make
```

L'exécutable est généré dans `bin/test_client`.

---

## Syntaxe générale

```bash
./bin/test_client <IP_SERVEUR> [PORT] [MODE] [PSEUDO]
```

| Paramètre    | Défaut      | Description                          |
|-------------|------------|--------------------------------------|
| IP_SERVEUR  | —          | IP ou nom d'hôte du serveur (requis) |
| PORT        | 5000       | Port d'écoute du serveur             |
| MODE        | random     | Stratégie de jeu du bot              |
| PSEUDO      | TestBot    | Pseudo utilisé pour se connecter     |

---

## Modes disponibles

| Mode         | Description                                                        |
|-------------|---------------------------------------------------------------------|
| `win`        | Joue toujours colonne 4 → victoire verticale garantie en 4 coups  |
| `lose`       | Alterne col 1 et col 7 → jetons dispersés, ne gagne jamais        |
| `fill`       | Remplit la grille colonne par colonne → force le match nul         |
| `overflow`   | Joue toujours col 4 même quand elle est pleine → teste RES_MOVE_ERROR |
| `random`     | Coups aléatoires (défaut)                                          |
| `idle`       | Se connecte sans jouer → teste timeout / déconnexion               |
| `tournament` | Rejoint le tournoi et joue en mode win                             |

> Chaque bot joue **une seule partie** puis se déconnecte automatiquement.  
> Exception : le mode `tournament` reste connecté jusqu'à la fin du bracket.

---

## Scénarios de test

Définir d'abord la variable `IP` dans ton terminal, puis copier-coller les commandes directement :

```bash
export IP=10.137.187.224
```

---

### Test 1 — Victoire / Défaite (ELO impacté)

BotWin gagne en 4 coups (colonne 4), BotLose ne peut pas aligner 4 jetons.

```bash
./bin/test_client $IP 5000 win  BotWin  &
./bin/test_client $IP 5000 lose BotLose
```

**Vérifie :** `PUSH_ENDGAME`, calcul ELO (BotWin monte, BotLose descend).

---

### Test 2 — Match nul (grille pleine)

Les deux bots remplissent la grille colonne par colonne sans jamais gagner.

```bash
./bin/test_client $IP 5000 fill BotFill1 &
./bin/test_client $IP 5000 fill BotFill2
```

**Vérifie :** détection de grille pleine, `PUSH_ENDGAME` avec `id_vainqueur == 0`.

---

### Test 3 — Colonne pleine / overflow

BotOverflow insiste dans la même colonne pleine pour générer des `RES_MOVE_ERROR`.

```bash
./bin/test_client $IP 5000 overflow BotOverflow &
./bin/test_client $IP 5000 win      BotWin
```

**Vérifie :** le serveur rejette les coups invalides avec `RES_MOVE_ERROR` sans planter.

---

### Test 4 — Déconnexion en cours de partie (forfait)

BotIdle se connecte et ne joue jamais, BotWin doit gagner par forfait à la déco.

```bash
./bin/test_client $IP 5000 win  BotWin &
./bin/test_client $IP 5000 idle BotIdle
# Arrêter BotIdle après quelques secondes :
kill %2
```

**Vérifie :** `nettoyer_client()` côté serveur, BotWin reçoit `PUSH_ENDGAME` victoire par forfait.

---

### Test 5 — Partie aléatoire simple

```bash
./bin/test_client $IP 5000 random BotA &
./bin/test_client $IP 5000 random BotB
```

**Vérifie :** déroulement normal d'une partie avec des coups variés.

---

### Test 6 — Tournoi complet (4 bots)

Les 4 bots s'inscrivent, le serveur lance automatiquement les demi-finales
puis la finale dès que les 4 joueurs sont présents.

```bash
./bin/test_client $IP 5000 tournament T1 &
./bin/test_client $IP 5000 tournament T2 &
./bin/test_client $IP 5000 tournament T3 &
./bin/test_client $IP 5000 tournament T4
```

**Vérifie :** inscription, `PUSH_TOURNAMENT_STATE`, demi-finales, finale,
`PUSH_ENDGAME` à chaque match, vainqueur final.

---

### Test 7 — Défi entre amis (challenge)

Lancer un vrai client IHM sur une machine, puis un bot en mode random.
Le vrai client envoie un défi au bot via la liste d'amis.
Le bot l'accepte automatiquement et choisit une partie sans ELO.

```bash
./bin/test_client $IP 5000 random BotChallenge
# Puis depuis le client IHM : [2] Amis → [d] Défier BotChallenge
```

**Vérifie :** `REQ_CHALLENGE`, `PUSH_CHALLENGE`, `RES_CHALLENGE_ACCEPT`,
`PUSH_CHOOSE_ELO`, `REQ_SET_ELO_MODE`.

---

## Réinitialiser les profils entre les tests

```bash
make init
# Equivalent a : make clean + rm -f profils/*.dat
```

