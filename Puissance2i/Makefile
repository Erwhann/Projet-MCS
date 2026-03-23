CC = gcc
CFLAGS = -Wall -Wextra -I./commun -I./reseau -g

# Fichiers communs
RESEAU_SRC = reseau/session.c reseau/data.c

# Serveur
SERVEUR_SRC = serveur/serveur.c serveur/matchmaking.c serveur/elo.c serveur/tournoi.c serveur/amis.c serveur/jeu.c
SERVEUR_OBJ = $(SERVEUR_SRC:.c=.o) $(RESEAU_SRC:.c=.o)

# Client
CLIENT_SRC = client/client.c client/ihm.c client/menu.c
CLIENT_OBJ = $(CLIENT_SRC:.c=.o) $(RESEAU_SRC:.c=.o)

# Executables
EXEC_SERVEUR = bin/serveur
EXEC_CLIENT = bin/client

all: bin_dir $(EXEC_SERVEUR) $(EXEC_CLIENT)

bin_dir:
	mkdir -p bin

$(EXEC_SERVEUR): $(SERVEUR_OBJ)
	$(CC) -o $@ $^

$(EXEC_CLIENT): $(CLIENT_OBJ)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SERVEUR_OBJ) $(CLIENT_OBJ)

mrproper: clean
	rm -rf bin
