CC     = gcc
CFLAGS = -Wall -Wextra -g -I./commun -I./reseau -I./serveur -I./client

SRCS_SRV = serveur/serveur.c serveur/jeu.c serveur/matchmaking.c \
           serveur/elo.c serveur/amis.c \
           reseau/session.c reseau/data.c \
           commun/profil.c
OBJS_SRV = $(SRCS_SRV:.c=.o)

SRCS_CLT = client/client.c client/ihm.c \
           reseau/session.c reseau/data.c \
           commun/profil.c
OBJS_CLT = $(SRCS_CLT:.c=.o)

all: bin/serveur bin/client

bin/serveur: $(OBJS_SRV)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $^ -lm

bin/client: $(OBJS_CLT)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $^ -lncurses

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS_SRV) $(OBJS_CLT) bin/serveur bin/client
