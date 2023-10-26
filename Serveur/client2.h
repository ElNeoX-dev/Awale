#ifndef CLIENT_H
#define CLIENT_H

#define BUF_SIZE 1024

typedef int SOCKET;

enum States
{
   NOTEXIST,
   DISCONNECTED,
   REGISTERING,
   MENU,
   LOBBY,
   PLAYING,
   HASPLAYED,
   WAITING_FOR_PLAY,
   PLAYING_WAITING,
   CHALLENGED,
   WAITING_RESPONSE,
   WAITING,
   REQUESTING,
   RESPONDING
};

typedef struct Client
{
   SOCKET sock;
   char name[BUF_SIZE];
   enum States state;
   struct Game *game;
   int id;
} Client;

typedef struct Game
{
   int plateau[12];
   int authorizedMove[12];
   int points[2];
   Client *clients[2];
} Game;

#endif /* guard */
