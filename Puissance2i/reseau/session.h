#ifndef SESSION_H
#define SESSION_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

int creer_serveur(int port);
int accepter_client(int fd_ecoute, struct sockaddr_in *addrClt);
int connecter_serveur(const char *adresse, int port);
void fermer_socket(int fd);

#endif
