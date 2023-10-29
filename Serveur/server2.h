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

#include "../Serveur/awale.h"
#include "client2.h"

#define CRLF "\r\n"
#define PORT 1977
#define MAX_CLIENTS 100

#define BUF_SIZE 1024

#define NBMAXJOUEUR 100
#define NBMAXOBSERVER 10
#define TAILLEMAXCHARJOUEUR 20

static void init(void);
static void end(void);
static void app(void);
static int init_connection(void);
static void end_connection(int sock);
static int read_client(SOCKET sock, char *buffer);
static void write_client(SOCKET sock, const char *buffer, ...);
static void write_to_players(Client **clients, const char *message, ...);
static void write_to_all_players(Client **clients, const char *message, ...);
static void send_message_to_all_clients(Client *clients, const char *buffer);
static void clear_clients(Client *clients);
static void clear_games(Game **games);

void sinscrire(char *username, Client *allUser, Client *client);
void listerJoueurState(Client *allUser, char *listePseudo, enum States state, Client *client);
void listerJoueurNotState(Client *allUser, char *listePseudo, enum States state, Client *client);
void listerGameEnCours(Game **allGames, char *playedGame);

#endif /* guard */
