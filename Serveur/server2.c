#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

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

   Client **allUser = malloc(NBMAXJOUEUR * sizeof(Client *));
   for (int i = 0; i < NBMAXJOUEUR; i++)
   {
      // maxi 20 caractères par pseudo
      allUser[i] = malloc(sizeof(Client));
      allUser[i] = NULL;
   }

   /*
   // maxi 20 joueurs online (si tous les joueurs sont connectés)
   Client **allUserOnline = malloc(NBMAXJOUEUR * sizeof(Client *));
   for (int i = 0; i < NBMAXJOUEUR; i++)
   {
      // maxi 20 caractères par pseudo
      allUserOnline[i] = malloc(sizeof(Client));
      allUserOnline[i] = NULL;
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
            /* a client is talking */
            if (FD_ISSET(clientslocal[i].sock, &rdfs))
            {
               Client *client = &clientslocal[i];
               int c = read_client(clientslocal[i].sock, buffer);
               /* client disconnected */
               if (c == 0)
               {
                  client->state = DISCONNECTED;
                  closesocket(clientslocal[i].sock);
                  remove_client(clientslocal, i, &actual);
                  strncpy(buffer, client->name, BUF_SIZE - 1);
                  strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);
                  send_message_to_all_clients(clientslocal, *client, actual, buffer, 1);
               }
               else
               {
                  send_message_to_all_clients(clientslocal, *client, actual, buffer, 0);
                  Client *client = &clientslocal[i];

                  if (client->state == MENU)
                  {
                     if (strstr(buffer, "inscription") != NULL)
                     {
                        printf("Il a ecrit INSCRIPTION\n");
                        char *username = strtok(buffer, ":");
                        username = strtok(NULL, ":");
                        username = strtok(username, "\r\n");
                        printf("username = %s\r\n", username);

                        sinscrire(username, allUser, client);
                     }
                     else if (strcmp(buffer, "afficher\0\n") == 0)
                     {
                        printf("Il a ecrit afficher\n");
                        /*char *affichagePlateau = malloc(1024 * sizeof(char));
                        genererAffPlateau(plateau, joueur, points, clients[i].sock, affichagePlateau);
                        write_client(clients[i].sock, affichagePlateau);
                        free(affichagePlateau);*/
                     }
                     else if (strcmp(buffer, "action\0\n") == 0)
                     {
                        printf("Il a ecrit ACTION\n");

                        write_client(client->sock, ROUGE BOLD "/!\\ Choix non valide !\r\n" RESET);
                     }
                     else if (strcmp(buffer, "listepseudo\0\n") == 0)
                     {
                        printf("Il a ecrit LISTE PSEUDO\n");
                        char *listePseudo = malloc(NBMAXJOUEUR * (TAILLEMAXCHARJOUEUR + 2) * sizeof(char));
                        listerJoueurNotState(allUser, listePseudo, NOTEXIST, client);

                        write_client(client->sock, BOLD "Voici la liste des pseudos : \r\n" RESET "%s", listePseudo);

                        free(listePseudo);
                     }
                     else if (strcmp(buffer, "listepseudoonline\0\n") == 0)
                     {
                        printf("Il a ecrit LISTE PSEUDO ONLINE\n");
                        char *listePseudoOnline = malloc(NBMAXJOUEUR * (TAILLEMAXCHARJOUEUR + 2) * sizeof(char));
                        listerJoueurNotState(allUser, listePseudoOnline, DISCONNECTED, client);

                        write_client(client->sock, BOLD "Voici la liste des pseudos Online : \r\n" RESET "%s", listePseudoOnline);

                        free(listePseudoOnline);
                     }
                     else if (strcmp(buffer, "jouer\0\n") == 0)
                     {
                        printf("Il a ecrit JOUER\n");
                        client->state = LOBBY;
                        write_client(client->sock, "0: defier un joueur\r\n1: attendre une invitation\r\n");
                     }
                     else if (strcmp(buffer, "quitter\0\n") == 0)
                     {
                        printf("Il a ecrit QUITTER\n");

                        client->state = DISCONNECTED;
                        write_client(clientslocal[i].sock, BLEU BOLD "Vous avez quitté la partie.\r\n" RESET);
                        closesocket(clientslocal[i].sock);
                        remove_client(clientslocal, i, &actual);
                        strncpy(buffer, client->name, BUF_SIZE - 1);
                        strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);
                        send_message_to_all_clients(clientslocal, *client, actual, buffer, 1);
                     }
                     else if (strcmp(buffer, "help\0\n") == 0)
                     {
                        printf("Il a ecrit HELP\n");

                        char help[] = {VERT BOLD "Voici la liste des commandes utilisables :\r\n" RESET
                                           VERT "  - afficher : affiche le plateau de jeu\r\n"
                                                 "  - action : permet de jouer un coup\r\n"
                                                 "  - help : affiche la liste des commandes utilisables\r\n"
                                                 "  - quitter : permet de quitter le jeu\r\n" RESET};

                        write_client(client->sock, "%s", help);
                     }
                     else
                     {
                        write_client(client->sock, VIOLET BOLD "State : %s\r\n" RESET, client->state);
                        write_client(client->sock, ROUGE BOLD "Commande non reconnue : %s" RESET "\nVeullez réessayer.\r\n", buffer);
                     }
                  }
                  else if (client->state == LOBBY)
                  {
                     if (strcmp(buffer, "0\0\n") == 0)
                     {
                        client->state = REQUESTING;
                        char *listePseudoOnline = malloc(NBMAXJOUEUR * (TAILLEMAXCHARJOUEUR + 2) * sizeof(char));
                        listerJoueurState(allUser, listePseudoOnline, WAITING, client);
                        write_client(client->sock, BOLD "Voici la liste des joueurs en attente d'invitation: \r\n" RESET);
                        write_client(client->sock, listePseudoOnline);
                        write_client(client->sock, BOLD "Veuillez entrer le pseudo du joueur que vous voulez defier : \r\n" RESET);
                        free(listePseudoOnline);
                     }
                     else if (strcmp(buffer, "1\0\n") == 0)
                     {
                        client->state = WAITING;
                        // Ajouter son pseudo a la liste des joueurs WAITING
                        /*
                        for (int j = 0; j < NBMAXJOUEUR; j++)
                        {
                           if (strcmp(allUserOnline[j]->name, "") == 0)
                           {
                              allUserOnline[j]->name = client->name;
                              break;
                           }
                        }
                        */

                        write_client(client->sock, VERT BOLD "En attente ...\r\nEnvoyer n'importe quoi pour annuler\r\n" RESET);
                     }
                     else if (strlen(buffer) > 0)
                     {
                        write_client(client->sock, "Commande incorrecte\r\n");
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
                           if (allUser[j] != NULL && strcmp(allUser[j]->name, buffer) == 0)
                           {
                              allUser[j]->state = CHALLENGED;
                              client->state = WAITING_RESPONSE;

                              // on cree la partie et on ajoute les deux joueurs
                              Game game;
                              game.plateau = malloc(12 * sizeof(int));
                              game.authorizedMove = malloc(12 * sizeof(int));
                              game.points = malloc(2 * sizeof(int));
                              game.clients = malloc(2 * sizeof(Client *));

                              game.clients[0] = client; // 0 = celui qui request
                              game.clients[1] = allUser[j];

                              client->game = &game;
                              allUser[j]->game = &game;

                              playerFound = 1;
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
                           if (strcmp(allUserWaiting[j], client->name) == 0)
                           {
                              allUserWaiting[j] = ""; // on le supprime de la liste d'attente
                              break;
                           }
                        }
                        */
                        client->state = MENU;
                        write_client(client->sock, BOLD "\nEntrez une commande ou faites \"help\" pour accéder à la liste des commandes disponibles : " RESET);
                     }
                  }
                  else if (client->state == CHALLENGED)
                  {
                     client->state = RESPONDING;
                     write_client(client->sock, "**Machin** vous a défié\r\n0: Accepter\r\n1: Refuser\r\n");
                  }
                  else if (client->state == WAITING_RESPONSE)
                  {
                  }
                  else if (client->state == RESPONDING)
                  {
                     if (strcmp(buffer, "0\0\n") == 0)
                     {
                        /*
                        Game game;
                        game.plateau = malloc(12 * sizeof(int));
                        game.authorizedMove = malloc(12 * sizeof(int));
                        game.points = malloc(2 * sizeof(int));
                        game.clients = malloc(2 * sizeof(Client *));
                        */

                        // tirer un nombre aleatoire entre 0 et 1 et defini s'il est le joueur 0 ou 1
                        if (rand() % 2)
                        {
                           client->id = 0;
                           client->game->clients[0]->id = 1;
                        }
                        else
                        {
                           client->id = 1;
                           client->game->clients[0]->id = 0;
                        }

                        client->state = PLAYING;
                     }
                     else if (strcmp(buffer, "1\0\n") == 0)
                     {
                        client->state = WAITING;
                     }
                  }
                  else if (client->state == PLAYING)
                  {
                     char *affichagePlateau = malloc(1024 * sizeof(char));
                     // affichage du plateau
                     genererAffPlateau(client->game, affichagePlateau);
                     write_to_players(client->game->clients, affichagePlateau);
                     jouer(client->game, client->id);

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
                                          } */
                  }
                  else if (client->state == PLAYING_WAITING)
                  {
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

static void send_message_to_all_clients(Client *clients, Client sender, int actual, const char *buffer, char from_server)
{
   int i = 0;
   char message[BUF_SIZE];
   message[0] = 0;
   for (i = 0; i < actual; i++)
   {
      /* we don't send message to the sender */
      if (sender.sock != clients[i].sock)
      {
         if (from_server == 0)
         {
            strncpy(message, sender.name, BUF_SIZE - 1);
            strncat(message, " : ", sizeof message - strlen(message) - 1);
         }
         strncat(message, buffer, sizeof message - strlen(message) - 1);
         write_client(clients[i].sock, message);
      }
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

int main(int argc, char **argv)
{
   init();

   app();

   end();

   return EXIT_SUCCESS;
}

void sinscrire(char *username, Client **allUser, Client *client)
{
   int joueurNonInscrit = 1; // 0 = inscrit ; 1 = non inscrit
   for (int j = 0; j < NBMAXJOUEUR; j++)
   {
      // printf("allUser[j] = %s\r\n", allUser[j]);
      if (allUser[j] == NULL)
      {
         break;
      }
      else if (allUser[j]->state != DISCONNECTED && strcmp(allUser[j]->name, username) == 0)
      {
         // joueur deja en ligne -> erreur faut un autre nom d'utilisateur
         joueurNonInscrit = 0;
         write_client(client->sock, "0");
         break;
      }
      else if (allUser[j]->state == DISCONNECTED && strcmp(allUser[j]->name, username) == 0)
      {
         // joueur deja inscrit -> Welcome back
         strcpy(client->name, username);
         joueurNonInscrit = 0;
         write_client(client->sock, "2");
         client->state = MENU;

         break;
      }
   }

   if (joueurNonInscrit == 1)
   {
      int writeAllUser = 0;
      int writeAllUserOnline = 0;
      // joueur non inscrit -> inscription
      for (int j = 0; j < NBMAXJOUEUR; j++)
      {
         if (allUser[j] == NULL)
         {
            allUser[j] = client;
            client->state = MENU;
            break;
         }
      }
      strcpy(client->name, username);
      write_client(client->sock, "1");
   }
}

void listerJoueurState(Client **allUser, char *listePseudo, enum States state, Client *client)
{
   for (int j = 0; j < NBMAXJOUEUR; j++)
   {
      if (allUser[j] == NULL)
      {
         break;
      }
      else if (allUser[j]->state == state && allUser[j]->name == client->name)
      {
         strcat(listePseudo, VERT);
         strcat(listePseudo, allUser[j]->name);
         strcat(listePseudo, " (you)" RESET);
         strcat(listePseudo, "\r\n");
      }
      else if (allUser[j]->state == state)
      {
         strcat(listePseudo, allUser[j]->name);
         strcat(listePseudo, "\r\n");
      }
   }
}

void listerJoueurNotState(Client **allUser, char *listePseudo, enum States state, Client *client)
{
   for (int j = 0; j < NBMAXJOUEUR; j++)
   {
      if (allUser[j] == NULL)
      {
         break;
      }
      else if (allUser[j]->state != state && allUser[j]->name == client->name)
      {
         strcat(listePseudo, VERT);
         strcat(listePseudo, allUser[j]->name);
         strcat(listePseudo, " (you)" RESET);
         strcat(listePseudo, "\r\n");
      }
      else if (allUser[j]->state != state)
      {
         strcat(listePseudo, allUser[j]->name);
         strcat(listePseudo, "\r\n");
      }
   }
}

/*
void listerJoueurWaiting(Client **allUserOnline, char *listePseudo)
{
   for (int j = 0; j < NBMAXJOUEUR; j++)
   {
      if (allUserOnline[j]->state == WAITING)
      {
         if (strcmp(allUserOnline[j]->name, "") != 0)
         {
            strcat(listePseudo, allUserOnline[j]->name);
            strcat(listePseudo, "\r\n");
         }
      }
   }
}
*/