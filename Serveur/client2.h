#ifndef CLIENT_H
#define CLIENT_H

#define BUF_SIZE 1024

typedef int SOCKET;

enum States
{
   DISCONNECTED,
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
   RESPONDING,
   NOTEXIST
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
   int *plateau;
   int *authorizedMove;
   int *points;
   Client **clients;
} Game;

#endif /* guard */
