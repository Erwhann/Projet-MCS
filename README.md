# Puissance2i (Projet MCS)

Puissance2i est un jeu et une application d'architecture client-serveur (développé en C). Il intègre des fonctionnalités réseaux et sociales avancées telles que le matchmaking, la gestion du score ELO, un système d'amis et l'organisation de tournois.

## Architecture

Le code source est séparé selon la structure logique suivante :

```text
Puissance2i/
│
├── serveur/
│   ├── serveur.c
│   ├── matchmaking.c
│   ├── elo.c
│   ├── tournoi.c
│   ├── amis.c
│   ├── jeu.c
│
├── client/
│   ├── client.c
│   ├── ihm.c
│   ├── menu.c
│
├── reseau/
│   ├── session.c
│   ├── session.h
│   ├── data.c
│   ├── data.h
│
├── commun/
│   ├── protocol.h
│   ├── structures.h
```

## Compilation

Un `Makefile` est fourni dans le dossier `Puissance2i/` pour automatiser la compilation du client et du serveur.

```bash
cd Puissance2i
make
```

Les exécutables générés se trouveront dans le répertoire `bin/`.

### Options de compilation :
- `make` : Compile tout le projet (serveur + client).
- `make clean` : Supprime les fichiers objets intermédiaires (`.o`).

## Exécution

Le projet nécessitant deux entités pour fonctionner en réseau, vous devez d'abord lancer le serveur, puis connecter les clients.

### 1. Démarrer le Serveur

Démarrez le serveur dans votre terminal principal :

```bash
cd Puissance2i
./bin/serveur
```

Ou depuis le raccourci à mettre sur le bureau : "Puissance2i.desktop", le lancer et sélectionner lancer le serveur

! Attention il faut modifier les paths dans le fichier Puissance2i.desktop afin qu'il convienne à votre ordinateur

### 2. Démarrer un ou plusieurs Clients

Dans d'autres terminaux, lancez le programme client pour vous connecter au serveur :

```bash
cd Puissance2i
# l'adresse est configurable :
./bin/client [IP_ADDRESS] 8080
```

Ou depuis le raccourci à mettre sur le bureau : "Puissance2i.desktop", le lancer et sélectionner lancer le client puis entrer l'ip du serveur 

! Attention il faut modifier les paths dans le fichier Puissance2i.desktop afin qu'il convienne à votre ordinateur

### 3. Générer la documentation doxygen

```bash
doxygen Doxyfile

#Pour lancer dans le navigateur la documentation : 
xdg-open doxygen_doc/html/index.html

```


