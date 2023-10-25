#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include "server2.h"
#include "awale.h"
#include "client2.h"

static void init(void)
{
#ifdef WIN32
   WSADATA wsa;
   int err = WSAStartup(MAKEWORD(2, 2), &wsa);
   if (err < 0)
   {
      puts("WSAStartup failed !");
      exit(EXIT_FAILURE);
   }
#endif
}

static void end(void)
{
#ifdef WIN32
   WSACleanup();
#endif
}

static void app(void)
{
   /* INIT */

   Client **allUsers = malloc(NBMAXJOUEUR * sizeof(Client *));
   for (int i = 0; i < NBMAXJOUEUR; i++)
   {
      // pas besoin de malloc, les adresses sont celles de localClient
      allUsers[i] = NULL;
   }

   /*
   // maxi 20 joueurs online (si tous les joueurs sont connectés)
   Client **allUsersOnline = malloc(NBMAXJOUEUR * sizeof(Client *));
   for (int i = 0; i < NBMAXJOUEUR; i++)
   {
      // maxi 20 caractères par pseudo
      allUsersOnline[i] = malloc(sizeof(Client));
      allUsersOnline[i] = NULL;
   }
   */
   // faire une liste de joueur Online
   // checker les states pour regarder s'ils sont en attente
   // sinon on les remet en non WAITING

   /* FIN INIT*/

   SOCKET sock = init_connection();
   char buffer[BUF_SIZE];
   /* the index for the array */
   int actual = 0;
   int max = sock;
   /* an array for all clients */
   Client clientslocal[MAX_CLIENTS];

   fd_set rdfs;

   char help[] = {VERT BOLD "Voici la liste des commandes utilisables :\r\n" RESET
                      VERT "  - 0 : jouer\r\n"
                            "  - 1 : afficher tous les pseudos et leur etat\r\n"
                            "  - 2 : afficher tous les pseudos online et leur etat\r\n"
                            "  - 3 : permet de quitter le jeu\r\n"
                            "  - help : affiche les commandes utilisables\r\n" RESET};

   while (1)
   {
      int i = 0;
      FD_ZERO(&rdfs);

      /* add STDIN_FILENO */
      FD_SET(STDIN_FILENO, &rdfs);

      /* add the connection socket */
      FD_SET(sock, &rdfs);

      /* add socket of each client */
      for (i = 0; i < actual; i++)
      {
         FD_SET(clientslocal[i].sock, &rdfs);
      }

      if (select(max + 1, &rdfs, NULL, NULL, NULL) == -1)
      {
         perror("select()");
         exit(errno);
      }

      /* something from standard input : i.e keyboard */
      if (FD_ISSET(STDIN_FILENO, &rdfs))
      {
         /* stop process when type on keyboard */
         break;
      }
      else if (FD_ISSET(sock, &rdfs))
      {
         /* new client */
         SOCKADDR_IN csin = {0};
         size_t sinsize = sizeof csin;
         int csock = accept(sock, (SOCKADDR *)&csin, &sinsize);
         if (csock == SOCKET_ERROR)
         {
            perror("accept()");
            continue;
         }

         /* after connecting the client sends its name */
         if (read_client(csock, buffer) == -1)
         {
            /* disconnected */
            continue;
         }

         /* what is the new maximum fd ? */
         max = csock > max ? csock : max;

         FD_SET(csock, &rdfs);

         Client c = {csock};
         strncpy(c.name, buffer, BUF_SIZE - 1);
         clientslocal[actual] = c;
         actual++;
      }
      else
      {
         int i = 0;
         for (i = 0; i < actual; i++)
         {
            Client *client = &clientslocal[i];
            /* a client is talking */
            if (FD_ISSET(clientslocal[i].sock, &rdfs))
            {
               int c = read_client(clientslocal[i].sock, buffer);

               /* client disconnected */
               if (c == 0)
               {

                  if (client->state == WAITING || client->state == PLAYING_WAITING || client->state == PLAYING)
                  {
                     if (strcmp(client->name, client->game->clients[0]->name) == 0)
                     {
                        write_client(client->game->clients[1]->sock, VERT BOLD "Vous avez gagné par abandon de %s !🥳\r\n" RESET, client->name);
                        client->game->clients[1]->state = MENU;
                        free(client->game->plateau);
                        free(client->game->authorizedMove);
                        free(client->game->points);
                        free(client->game);
                        write_client(client->game->clients[1]->sock, BOLD "\nEntrez une commande ou faites \"help\" pour accéder à la liste des commandes disponibles : " RESET);
                     }
                     else
                     {
                        write_client(client->game->clients[0]->sock, VERT BOLD "Vous avez gagné par abandon de %s !🥳\r\n" RESET, client->name);
                        client->game->clients[0]->state = MENU;
                        write_client(client->game->clients[0]->sock, BOLD "\nEntrez une commande ou faites \"help\" pour accéder à la liste des commandes disponibles : " RESET);
                     }
                  }

                  client->state = DISCONNECTED;
                  write_client(client->sock, BLEU "Vous avez quitté le jeu\r\n" RESET);
                  closesocket(clientslocal[i].sock);
                  remove_client(clientslocal, i, &actual);
                  strncpy(buffer, client->name, BUF_SIZE - 1);
                  strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);
                  // send_message_to_all_clients(clientslocal, *client, actual, buffer, 1);
               }
               else
               {
                  // send_message_to_all_clients(clientslocal, *client, actual, buffer, 0);

                  // Chat : si le message contient "->"
                  // Envoie a tous le monde si le client est dans le lobby ou dans le menu
                  // Envoie a son adversaire si le client est en Game
                  if (strstr(buffer, "->") != NULL)
                  {
                     printf("Il a ecrit CHAT\n");
                     char *message = malloc(1024 * sizeof(char));
                     char *msg = strtok(buffer, "->");
                     sprintf(message, BLEU BOLD "%s : " RESET BLEU "%s\r\n" RESET, client->name, msg);

                     if (client->state == MENU || client->state == LOBBY || client->state == WAITING)
                     {
                        printf("MENU ou LOBBY\n");
                        send_message_to_all_clients(clientslocal, *client, actual, message, 1);
                        // write_client(client->sock, message);
                     }
                     else
                     {
                        printf("EN GAME\n");
                        write_to_players(client->game->clients, message);
                     }

                     free(message);
                  }
                  else if (client->state == DISCONNECTED)
                  {
                     if (strstr(buffer, "inscription") != NULL)
                     {
                        printf("Il a ecrit INSCRIPTION\n");
                        char *username = strtok(buffer, ":");
                        username = strtok(NULL, ":");
                        username = strtok(username, "\r\n");
                        printf("username = %s\r\n", username);
                        sinscrire(username, allUsers, client);

                        write_client(client->sock, "%s", help);
                        /*if (sinscrire(username, allUsers, client) != 0)
                        {
                           write_client(client->sock, "%s", help);
                        }*/
                     }
                  }
                  else if (client->state == MENU)
                  {
                     if (strcmp(buffer, "help\0\n") == 0)
                     {
                        printf("Il a ecrit HELP\n");
                        write_client(client->sock, "%s", help);
                     }
                     else if (strcmp(buffer, "0\0\n") == 0)
                     {
                        printf("Il a ecrit JOUER\n");
                        client->state = LOBBY;
                        write_client(client->sock, "0: defier un joueur\r\n1: attendre une invitation\r\n");
                     }
                     else if (strcmp(buffer, "1\0\n") == 0)
                     {
                        printf("Il a ecrit LISTE PSEUDO\n");
                        char *listePseudo = malloc(NBMAXJOUEUR * (TAILLEMAXCHARJOUEUR + 2) * sizeof(char));
                        listerJoueurNotState(allUsers, listePseudo, NOTEXIST, client);

                        write_client(client->sock, BOLD "Voici la liste des pseudos : \r\n" RESET "%s", listePseudo);

                        free(listePseudo);
                     }
                     else if (strcmp(buffer, "2\0\n") == 0)
                     {
                        printf("Il a ecrit LISTE PSEUDO ONLINE\n");
                        char *listePseudoOnline = malloc(NBMAXJOUEUR * (TAILLEMAXCHARJOUEUR + 2) * sizeof(char));
                        listerJoueurNotState(allUsers, listePseudoOnline, DISCONNECTED, client);

                        write_client(client->sock, BOLD "Voici la liste des pseudos Online : \r\n" RESET "%s", listePseudoOnline);

                        free(listePseudoOnline);
                     }
                     else if (strcmp(buffer, "3\0\n") == 0)
                     {
                        printf("Il a ecrit QUITTER\n");

                        client->state = DISCONNECTED;
                        write_client(client->sock, BLEU "Vous avez quitté le jeu\r\n" RESET);
                        closesocket(clientslocal[i].sock);
                        remove_client(clientslocal, i, &actual);
                        strncpy(buffer, client->name, BUF_SIZE - 1);
                        strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);
                        // send_message_to_all_clients(clientslocal, *client, actual, buffer, 1);
                     }
                     else
                     {
                        // write_client(client->sock, VIOLET BOLD "State : %s\r\n" RESET, client->state);
                        write_client(client->sock, ROUGE BOLD "Commande non reconnue : %s" RESET "\nVeullez réessayer.\r\n", buffer);
                     }
                  }
                  else if (client->state == LOBBY)
                  {
                     if (strcmp(buffer, "0\0\n") == 0)
                     {
                        client->state = REQUESTING;
                        char *listePseudoOnline = malloc(NBMAXJOUEUR * (TAILLEMAXCHARJOUEUR + 2) * sizeof(char));
                        listerJoueurState(allUsers, listePseudoOnline, WAITING, client);
                        write_client(client->sock, BOLD "Voici la liste des joueurs en attente d'invitation: \r\n" RESET);
                        write_client(client->sock, listePseudoOnline);
                        write_client(client->sock, BOLD "Veuillez entrer le pseudo du joueur que vous voulez defier : \r\n" RESET);
                        free(listePseudoOnline);
                        sd
                     }
                     else if (strcmp(buffer, "1\0\n") == 0)
                     {
                        client->state = WAITING;
                        // Ajouter son pseudo a la liste des joueurs WAITING
                        /*
                        for (int j = 0; j < NBMAXJOUEUR; j++)
                        {
                           if (strcmp(allUsersOnline[j]->name, "") == 0)
                           {
                              allUsersOnline[j]->name = client->name;
                              break;
                           }
                        }
                        */

                        write_client(client->sock, VERT BOLD "En attente ...\r\nEnvoyer n'importe quoi pour annuler\r\n" RESET);
                     }
                     else if (strlen(buffer) > 0)
                     {
                        write_client(client->sock, ROUGE BOLD "Commande incorrecte\r\n" RESET);
                     }
                  }
                  else if (client->state == REQUESTING)
                  {
                     if (strlen(buffer) > 0)
                     {
                        int playerFound = 0;
                        // matching client
                        for (int j = 0; j < NBMAXJOUEUR; j++)
                        {
                           if (allUsers[j] != NULL && strcmp(allUsers[j]->name, buffer) == 0)
                           {

                              write_client(client->sock, VIOLET BOLD "En attente de la réponse de %s...\r\n" RESET, buffer);
                              // printf("Je suis : %s->%d et je défie : %s->%d\r\n", client->name, client->state, allUsers[j]->name, allUsers[j]->state);
                              allUsers[j]->state = CHALLENGED;
                              client->state = WAITING_RESPONSE;

                              // on cree la partie et on ajoute les deux joueurs
                              Game *game = malloc(sizeof(Game));
                              game->plateau = malloc(12 * sizeof(int));
                              game->authorizedMove = malloc(12 * sizeof(int));
                              game->points = malloc(2 * sizeof(int));
                              game->clients = malloc(2 * sizeof(Client *));

                              for (int k = 0; k < 12; k++)
                              {
                                 game->plateau[k] = 4;
                              }

                              game->clients[0] = client; // 0 = celui qui request
                              game->clients[1] = allUsers[j];

                              client->game = game;
                              allUsers[j]->game = game;

                              playerFound = 1;
                              write_client(allUsers[j]->sock, "%s vous a défié\r\n0: Accepter\r\n1: Refuser\r\n", client->name);
                              break;
                           }
                        }
                        if (playerFound == 0)
                        {
                           write_client(client->sock, ROUGE BOLD "Joueur introuvable\r\n" RESET);
                           client->state = LOBBY;
                           write_client(client->sock, "0: defier un joueur\r\n1: attendre une invitation\r\n");
                        }
                     }
                  }
                  else if (client->state == WAITING)
                  {
                     if (strcmp(buffer, "0\0\n"))
                     {
                        /*
                        for (int j = 0; j < NBMAXJOUEUR; j++)
                        {
                           if (strcmp(allUsersWaiting[j], client->name) == 0)
                           {
                              allUsersWaiting[j] = ""; // on le supprime de la liste d'attente
                              break;
                           }
                        }
                        */
                        client->state = MENU;
                        write_client(client->sock, BOLD "\nEntrez une commande ou faites \"help\" pour accéder à la liste des commandes disponibles : " RESET);
                     }
                  }
                  /*else if (client->state == CHALLENGED)
                  {
                     client->state = RESPONDING;
                     write_client(client->sock, "%s vous a défié\r\n0: Accepter\r\n1: Refuser\r\n", client->game->clients[0]->name);
                  }*/
                  else if (client->state == WAITING_RESPONSE)
                  {
                  }
                  else if (client->state == CHALLENGED)
                  {
                     if (strcmp(buffer, "0\0\n") == 0)
                     {

                        char *affichagePlateau = malloc(1024 * sizeof(char));
                        // affichage du plateau
                        genererAffPlateau(client->game, affichagePlateau);
                        write_to_players(client->game->clients, affichagePlateau);
                        free(affichagePlateau);

                        // tirer un nombre aleatoire entre 0 et 1 et defini qui commence
                        if (time(0) % 2)
                        {
                           client->id = 0;
                           client->state = PLAYING;
                           client->game->clients[0]->id = 1;
                           client->game->clients[0]->state = WAITING_FOR_PLAY;
                           write_client(client->sock, "Le Hasard a décidé que vous commenciez\r\n");
                           write_client(client->sock, "%s, choisissez une case non-vide (entre 0 et 5): ", client->name);

                           write_client(client->game->clients[0]->sock, free "Le Hasard a décidé que %s commencait\r\n", client->name);
                        }
                        else
                        {
                           client->id = 1;
                           client->state = WAITING_FOR_PLAY;
                           client->game->clients[0]->id = 0;
                           client->game->clients[0]->state = PLAYING;

                           write_client(client->game->clients[0]->sock, "Le Hasard a décidé que vous commenciez\r\n");
                           write_client(client->game->clients[0]->sock, "%s, choisissez une case non-vide (entre 0 et 5): ", client->game->clients[0]->name);

                           write_client(client->sock, "Le Hasard a décidé que %s commencait\r\n", client->game->clients[0]->name);
                        }
                     }
                     else if (strcmp(buffer, "1\0\n") == 0)
                     {
                        client->state = WAITING;
                        client->game->clients[0]->state = MENU;
                        write_client(client->game->clients[0]->sock, ROUGE "%s n'a pas accepté le challenge...'\r\n" RESET, client->name);
                        write_client(client->sock, VERT BOLD "En attente ...\r\nEnvoyer n'importe quoi pour annuler\r\n" RESET);
                        write_client(client->game->clients[0]->sock, BOLD "\nEntrez une commande ou faites \"help\" pour accéder à la liste des commandes disponibles : " RESET);
                     }
                  }
                  else if (client->state == PLAYING)
                  {
                     int caseChoisie = atoi(buffer);
                     char *message = malloc(2048 * sizeof(char));
                     char *affichagePlateau = malloc(1024 * sizeof(char));
                     if (isFinish(client->game, message) == 1)
                     {
                        write_to_players(client->game->clients, message);
                        client->game->clients[0]->state = MENU;
                        client->game->clients[1]->state = MENU;
                        write_to_players(client->game->clients, BOLD "\nEntrez une commande ou faites \"help\" pour accéder à la liste des commandes disponibles : " RESET);
                        free(client->game->plateau);
                        free(client->game->authorizedMove);
                        free(client->game->points);
                        free(client->game);
                     }
                     else
                     {
                        int retour = jouer(client->game, client->id, caseChoisie, message);
                        printf("retour = %d\r\n", retour);
                        if (retour >= 0)
                        {

                           genererAffPlateau(client->game, affichagePlateau);
                           write_to_players(client->game->clients, message);
                           write_to_players(client->game->clients, affichagePlateau);
                           if (client->game->clients[0]->state == PLAYING_WAITING)
                           {
                              client->game->clients[0]->state = PLAYING;
                              write_client(client->game->clients[0]->sock, "%s, choisissez une case non-vide (entre %d et %d): ", client->game->clients[0]->name, 6 * client->game->clients[0]->id, 6 * client->game->clients[0]->id + 5);
                              write_client(client->game->clients[1]->sock, BOLD "C'est au tour de %s" RESET, client->game->clients[0]->name);
                           }
                           else
                           {
                              client->game->clients[1]->state = PLAYING;
                              write_client(client->game->clients[1]->sock, "%s, choisissez une case non-vide (entre %d et %d): ", client->game->clients[1]->name, 6 * client->game->clients[1]->id, 6 * client->game->clients[1]->id + 5);
                              write_client(client->game->clients[0]->sock, BOLD "C'est au tour de %s" RESET, client->game->clients[1]->name);
                           }
                           client->state = PLAYING_WAITING;
                        }
                        else
                        {
                           write_client(client->sock, message);
                        }
                        free(message);
                        free(affichagePlateau);
                     }

                     /*                      // boucle de jeu
                                          while (isFinish(client.game) == 0)
                                          {
                                             // joueur 1
                                             jouer(client.game, 0);
                                             afficherPlateau(client.game, affichagePlateau);
                                             write_to_players(client.game->clients, affichagePlateau);

                                             // joueur 2
                                             jouer(client.game, 1);
                                             afficherPlateau(client.game, affichagePlateau);
                                          }
                        */
                  }
                  else if (client->state == PLAYING_WAITING)
                  {
                     if (strlen(buffer) > 0)
                     {
                        write_client(client->sock, ROUGE BOLD "Ce n'est pas votre tour\r\n" RESET);
                     }
                  }
                  else if (client->state == WAITING_FOR_PLAY && strlen(buffer) > 0)
                  {
                  }
               }
               break;
            }
         }
      }
   }

   clear_clients(clientslocal, actual);
   free(allUsers);
   end_connection(sock);
}

static void clear_clients(Client *clients, int actual)
{
   int i = 0;
   for (i = 0; i < actual; i++)
   {
      closesocket(clients[i].sock);
   }
}

static void remove_client(Client *clients, int to_remove, int *actual)
{
   /* we remove the client in the array */
   memmove(clients + to_remove, clients + to_remove + 1, (*actual - to_remove - 1) * sizeof(Client));
   /* number client - 1 */
   (*actual)--;
}

static void send_message_to_all_clients(Client *clients, Client sender, int actual, const char *message, char from_server)
{
   int i = 0;
   for (i = 0; i < actual; i++)
   {
      /* we don't send message to the sender */
      write_client(clients[i].sock, message);
   }
}

static int init_connection(void)
{
   SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
   SOCKADDR_IN sin = {0};

   if (sock == INVALID_SOCKET)
   {
      perror("socket()");
      exit(errno);
   }

   sin.sin_addr.s_addr = htonl(INADDR_ANY);
   sin.sin_port = htons(PORT);
   sin.sin_family = AF_INET;

   if (bind(sock, (SOCKADDR *)&sin, sizeof sin) == SOCKET_ERROR)
   {
      perror("bind()");
      exit(errno);
   }

   if (listen(sock, MAX_CLIENTS) == SOCKET_ERROR)
   {
      perror("listen()");
      exit(errno);
   }

   return sock;
}

static void end_connection(int sock)
{
   closesocket(sock);
}

static int read_client(SOCKET sock, char *buffer)
{
   int n = 0;

   if ((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
   {
      perror("recv()");
      /* if recv error we disonnect the client */
      n = 0;
   }

   buffer[n] = 0;

   return n;
}

static void write_client(SOCKET sock, const char *message, ...)
{
   va_list args;
   va_start(args, message);

   // Créez un tampon pour formater le message
   char buffer[1024]; // Vous pouvez ajuster la taille du tampon en fonction de vos besoins

   // Formatez le message en utilisant vsnprintf
   vsnprintf(buffer, sizeof(buffer), message, args);

   // Affichez le message à la sortie standard
   printf("%s", buffer);

   if (send(sock, buffer, strlen(buffer), 0) < 0)
   {
      perror("send()");
      va_end(args);
      exit(errno);
   }
   va_end(args);
}

static void challenge(Client *requestingClient, Client *challengedClient)
{
   if (challengedClient->state == WAITING)
   {
      requestingClient->state = REQUESTING;
      challengedClient->state = RESPONDING;
      char *message = malloc(1024 * sizeof(char));
      strcat(message, requestingClient->name);
      strcat(message, " is challenging you\r\n");
      strcat(message, "0: Refuse, 1:Accept\r\n");
      write_client(requestingClient->sock, "Waiting for response\r\n");
      write_client(challengedClient->sock, message);
   }
   else
   {
      write_client(requestingClient->sock, "This player is not available\r\n");
   }
}

static void write_to_players(Client **clients, const char *message, ...)
{
   va_list args;
   va_start(args, message);

   // Créez un tampon pour formater le message
   char buffer[1024]; // Vous pouvez ajuster la taille du tampon en fonction de vos besoins

   // Formatez le message en utilisant vsnprintf
   vsnprintf(buffer, sizeof(buffer), message, args);

   int i = 0;
   // a changer en fonction du nombre d'observateurs
   for (i = 0; i < 2; i++)
   {
      write_client(clients[i]->sock, buffer);
   }
   va_end(args);
}

static void write_to_all_players(Client **clients, const char *message, ...)
{
   va_list args;
   va_start(args, message);

   // Créez un tampon pour formater le message
   char buffer[1024]; // Vous pouvez ajuster la taille du tampon en fonction de vos besoins

   // Formatez le message en utilisant vsnprintf
   vsnprintf(buffer, sizeof(buffer), message, args);

   int i = 0;
   // a changer en fonction du nombre d'observateurs
   for (i = 0; i < NBMAXJOUEUR; i++)
   {
      if (clients[i] != NULL)
      {
         write_client(clients[i]->sock, buffer);
      }
   }
   va_end(args);
}

int main(int argc, char **argv)
{
   init();

   app();

   end();

   return EXIT_SUCCESS;
}

void sinscrire(char *username, Client **allUsers, Client *client)
{
   int joueurNonInscrit = 1; // 0 = inscrit ; 1 = non inscrit
   for (int j = 0; j < NBMAXJOUEUR; j++)
   {
      // printf("allUsers[j] = %s\r\n", allUsers[j]);
      if (allUsers[j] == NULL)
      {
         break;
      }
      else if (allUsers[j]->state == DISCONNECTED && strcmp(username, allUsers[j]->name) == 0)
      {
         // joueur deja inscrit -> Welcome back
         strcpy(client->name, username);
         joueurNonInscrit = 0;
         write_client(client->sock, "2\r\n");
         client->state = MENU;

         break;
         // return 2;
      }
      else if (allUsers[j]->state != DISCONNECTED && strcmp(username, allUsers[j]->name) == 0)
      {
         // joueur deja en ligne -> erreur faut un autre nom d'utilisateur
         joueurNonInscrit = 0;
         write_client(client->sock, "0\r\n");

         break;
         // return 0;
      }
   }

   if (joueurNonInscrit == 1)
   {
      int writeAllUsers = 0;
      int writeAllUsersOnline = 0;
      // joueur non inscrit -> inscription
      for (int j = 0; j < NBMAXJOUEUR; j++)
      {
         if (allUsers[j] == NULL)
         {
            allUsers[j] = client;
            client->state = MENU;
            break;
         }
      }
      strcpy(client->name, username);
      write_client(client->sock, "1\r\n");
   }

   // return 1;
}

void listerJoueurState(Client **allUsers, char *listePseudo, enum States state, Client *client)
{
   for (int j = 0; j < NBMAXJOUEUR; j++)
   {
      if (allUsers[j] == NULL)
      {
         break;
      }
      else if (allUsers[j]->state == state && allUsers[j]->name == client->name)
      {
         strcat(listePseudo, VERT);
         strcat(listePseudo, allUsers[j]->name);
         strcat(listePseudo, " (you)" RESET);
         strcat(listePseudo, "\r\n");
      }
      else if (allUsers[j]->state == state)
      {
         strcat(listePseudo, allUsers[j]->name);
         strcat(listePseudo, "\r\n");
      }
   }
}

void listerJoueurNotState(Client **allUsers, char *listePseudo, enum States state, Client *client)
{
   for (int j = 0; j < NBMAXJOUEUR; j++)
   {
      if (allUsers[j] == NULL)
      {
         break;
      }
      else if (allUsers[j]->state != state && allUsers[j]->name == client->name)
      {
         strcat(listePseudo, VERT);
         strcat(listePseudo, allUsers[j]->name);
         strcat(listePseudo, " (you)" RESET);
         strcat(listePseudo, "\r\n");
      }
      else if (allUsers[j]->state != state)
      {
         strcat(listePseudo, allUsers[j]->name);
         strcat(listePseudo, "\r\n");
      }
   }
}

/*
void listerJoueurWaiting(Client **allUsersOnline, char *listePseudo)
{
   for (int j = 0; j < NBMAXJOUEUR; j++)
   {
      if (allUsersOnline[j]->state == WAITING)
      {
         if (strcmp(allUsersOnline[j]->name, "") != 0)
         {
            strcat(listePseudo, allUsersOnline[j]->name);
            strcat(listePseudo, "\r\n");
         }
      }
   }
}
*/