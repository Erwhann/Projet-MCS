CC     = gcc
CFLAGS = -Wall -Wextra -g -I./commun -I./reseau -I./serveur -I./client

SRCS_SRV = serveur/serveur.c serveur/jeu.c serveur/matchmaking.c \
           serveur/elo.c serveur/amis.c serveur/tournoi.c \
           reseau/session.c reseau/data.c \
           commun/profil.c
OBJS_SRV = $(SRCS_SRV:.c=.o)

SRCS_CLT = client/client.c client/ihm.c \
           reseau/session.c reseau/data.c \
           commun/profil.c
OBJS_CLT = $(SRCS_CLT:.c=.o)

SRCS_TEST = test/test_client.c \
            reseau/session.c reseau/data.c \
            commun/profil.c
OBJS_TEST = $(SRCS_TEST:.c=.o)

all: bin/serveur bin/client bin/test_client

bin/serveur: $(OBJS_SRV)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $^ -lm

bin/client: $(OBJS_CLT)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $^ -lncurses

bin/test_client: $(OBJS_TEST)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS_SRV) $(OBJS_CLT) $(OBJS_TEST) bin/serveur bin/client bin/test_client test/test_client.o

# init : remise a zero totale (binaires + objets + profils joueurs)
init: clean
	rm -f profils/*.dat
	@echo "Projet reinitialise : binaires, objets et profils supprimes."
