#ifndef AWALE_H
#define AWALE_H

#include "client2.h"
#include "prettytext.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <netdb.h>  /* gethostbyname */
#include <stdarg.h>
#include "../Serveur/awale.h"
#include "client2.h"
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
#define errno 1
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

void genererAffPlateau(Game *game, char *affichagePlateau);
void jouer(Game *game, int j);
int isTerrainAdverse(int j, int CaseChoisie);
int updateAuthorizedMove(Game *game, int j);
int checkStarvation(Game *game, int j);
// int hasWin(int *points, char **joueur);
int isFinish(Game *game);
static void write_client(SOCKET sock, const char *message, ...);
static void write_to_players(Client **clients, const char *message, ...);

#endif