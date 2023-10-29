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
#include <errno.h>
#include "../Serveur/awale.h"

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

void genererAffPlateau(Game *game, char *affichagePlateau);
int jouer(Game *game, int j, int caseChoisie, char *message);
int isTerrainAdverse(int j, int CaseChoisie);
int updateAuthorizedMove(Game *game, int j);
int checkStarvation(Game *game, int j);
int isFinish(Game *game, char *message);

#endif