#ifndef SERVER_H
#define SERVER_H

#ifdef WIN32

#include <winsock2.h>

#elif defined(linux) || defined(__APPLE__)

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <netdb.h>  /* gethostbyname */
#include <stdarg.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#else

#error not defined for this platform

#endif

#define CRLF "\r\n"
#define PORT 1977
#define MAX_CLIENTS 100

#define BUF_SIZE 1024

#define VERT "\e[0;32m"
#define ROUGE "\e[0;31m"
#define JAUNE "\e[1;33m"
#define VIOLET "\e[0;35m"
#define BLEU "\e[0;34m"
#define BOLD "\033[1m"
#define RESET "\033[0m"

#define NBMAXJOUEUR 100
#define TAILLEMAXCHARJOUEUR 20

#include "client2.h"

static void init(void);
static void end(void);
static void app(void);
static int init_connection(void);
static void end_connection(int sock);
static int read_client(SOCKET sock, char *buffer);
static void write_client(SOCKET sock, const char *buffer, ...);
static void send_message_to_all_clients(Client *clients, Client client, int actual, const char *buffer, char from_server);
static void remove_client(Client *clients, int to_remove, int *actual);
static void clear_clients(Client *clients, int actual);

void genererAffPlateau(int *plateau, char **joueur, int *points, int newsockfd, char *affichagePlateau);
void sinscrire(int newsockfd, char *username, char **allUser, char **allUserOnline, char *myUsername);
void listerJoueur(char **allUser, char *listePseudo);

void jouer(int *plateau, char **joueur, int *authorizedMove, int *points, int j);
int isTerrainAdverse(int j, int CaseChoisie);
int updateAuthorizedMove(int *authorizedMove, int *plateau, int j);
int checkStarvation(int *plateau, int *authorizedMove, int j);
int hasWin(int *points, char **joueur);
int isFinish(int *plateau, int *points, int *authorizedMove, int *joueur);

#endif /* guard */
