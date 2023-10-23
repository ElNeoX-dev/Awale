#ifndef CLIENT_H
#define CLIENT_H

#include "server2.h"

typedef struct
{
   SOCKET sock;
   char name[BUF_SIZE];
}Client;

enum States {
   MENU,
   PLAYING,
   WAITING,
   REQUESTING,
   RESPONDING
};


#endif /* guard */

