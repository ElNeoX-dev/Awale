#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "server2.h"
#include "client2.h"
#include "./../awale.h"

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

   char **allUser = malloc(NBMAXJOUEUR * sizeof(char *));
   for (int i = 0; i < NBMAXJOUEUR; i++)
   {
      // maxi 20 caractères par pseudo
      allUser[i] = malloc(TAILLEMAXCHARJOUEUR * sizeof(char));
   }

   // maxi 20 joueurs online (si tous les joueurs sont connectés)
   char **allUserOnline = malloc(NBMAXJOUEUR * sizeof(char *));
   for (int i = 0; i < NBMAXJOUEUR; i++)
   {
      // maxi 20 caractères par pseudo
      allUserOnline[i] = malloc(TAILLEMAXCHARJOUEUR * sizeof(char));
   }

   // initialisation des pseudos
   for (int i = 0; i < 5; i++)
   {
      strcpy(allUser[i], "Pseudo");
      strcpy(allUserOnline[i], "PseudoOnl");
   }

   int *plateau = malloc(12 * sizeof(int));
   int *points = malloc(2 * sizeof(int));
   char *joueur[2] = {"Tim", "Hugo"};
   int i = 0;

   // initialisation du plateau
   for (i = 0; i < 12; i++)
   {
      plateau[i] = 4;
   }

   for (i = 6; i < 12; i++)
   {
      plateau[i] = 0;
   }

   char *myUsername = malloc(TAILLEMAXCHARJOUEUR * sizeof(char));

   /* FIN INIT*/

   SOCKET sock = init_connection();
   char buffer[BUF_SIZE];
   /* the index for the array */
   int actual = 0;
   int max = sock;
   /* an array for all clients */
   Client clients[MAX_CLIENTS];

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
         FD_SET(clients[i].sock, &rdfs);
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
         clients[actual] = c;
         actual++;
      }
      else
      {
         int i = 0;
         for (i = 0; i < actual; i++)
         {
            /* a client is talking */
            if (FD_ISSET(clients[i].sock, &rdfs))
            {
               Client client = clients[i];
               int c = read_client(clients[i].sock, buffer);
               /* client disconnected */
               if (c == 0)
               {
                  closesocket(clients[i].sock);
                  remove_client(clients, i, &actual);
                  strncpy(buffer, client.name, BUF_SIZE - 1);
                  strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);
                  send_message_to_all_clients(clients, client, actual, buffer, 1);
               }
               else
               {
                  send_message_to_all_clients(clients, client, actual, buffer, 0);

                  if (strcmp(buffer, "afficher\0\n") == 0)
                  {
                     printf("Il a ecrit afficher\n");
                     char *affichagePlateau = malloc(1024 * sizeof(char));
                     genererAffPlateau(plateau, joueur, points, clients[i].sock, affichagePlateau);
                     write_client(clients[i].sock, affichagePlateau);
                     free(affichagePlateau);
                  }
                  else if (strcmp(buffer, "action\0\n") == 0)
                  {
                     printf("Il a ecrit ACTION\n");

                     write_client(clients[i].sock, ROUGE BOLD "/!\\ Choix non valide !\r\n" RESET);
                  }
                  else if (strstr(buffer, "inscription") != NULL)
                  {
                     printf("Il a ecrit INSCRIPTION\n");
                     char *username = strtok(buffer, ":");
                     username = strtok(NULL, ":");
                     printf("username = %s\r\n", username);

                     sinscrire(clients[i].sock, username, allUser, allUserOnline, myUsername);
                  }
                  else if (strcmp(buffer, "listepseudo\0\n") == 0)
                  {
                     printf("Il a ecrit LISTE PSEUDO\n");
                     char *listePseudo = malloc(NBMAXJOUEUR * (TAILLEMAXCHARJOUEUR + 2) * sizeof(char));
                     listerJoueur(allUser, listePseudo);

                     write_client(clients[i].sock, BOLD "Voici la liste des pseudos : \r\n" RESET "%s", listePseudo);

                     free(listePseudo);
                  }
                  else if (strcmp(buffer, "listepseudoonline\0\n") == 0)
                  {
                     printf("Il a ecrit LISTE PSEUDO ONLINE\n");
                     char *listePseudoOnline = malloc(NBMAXJOUEUR * (TAILLEMAXCHARJOUEUR + 2) * sizeof(char));
                     listerJoueur(allUserOnline, listePseudoOnline);

                     write_client(clients[i].sock, BOLD "Voici la liste des pseudos : \r\n" RESET "%s", listePseudoOnline);

                     free(listePseudoOnline);
                  }
                  else if (strcmp(buffer, "quitter\0\n") == 0)
                  {
                     printf("Il a ecrit QUITTER\n");

                     // se deconnecter
                     for (int j = 0; j < NBMAXJOUEUR; j++)
                     {
                        if (strcmp(allUserOnline[j], myUsername) == 0)
                        {
                           strcpy(allUserOnline[j], "");
                           break;
                        }
                     }

                     write_client(clients[i].sock, BLEU BOLD "Vous avez quitté la partie.\r\n" RESET);
                     closesocket(clients[i].sock);
                     remove_client(clients, i, &actual);
                     strncpy(buffer, client.name, BUF_SIZE - 1);
                     strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);
                     send_message_to_all_clients(clients, client, actual, buffer, 1);
                  }
                  else if (strcmp(buffer, "help\0\n") == 0)
                  {
                     printf("Il a ecrit HELP\n");

                     char help[] = {VERT BOLD "Voici la liste des commandes utilisables :\r\n" RESET
                                        VERT "  - afficher : affiche le plateau de jeu\r\n"
                                              "  - action : permet de jouer un coup\r\n"
                                              "  - help : affiche la liste des commandes utilisables\r\n"
                                              "  - quitter : permet de quitter le jeu\r\n" RESET};

                     write_client(clients[i].sock, "%s", help);
                  }
                  else
                  {
                     write_client(clients[i].sock, ROUGE BOLD "Commande non reconnue : %s" RESET "\nVeullez réessayer.\r\n", buffer);
                  }
               }
               break;
            }
         }
      }
   }

   clear_clients(clients, actual);
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

void genererAffPlateau(int *plateau, char **joueur, int *points, int newsockfd, char *affichagePlateau)
{
   char buffer[1024];
   sprintf(buffer, JAUNE BOLD "Joueur 1 : %s\r\n" RESET, joueur[0]);
   strcat(affichagePlateau, buffer);
   sprintf(buffer, VIOLET BOLD "Joueur 2 : %s\r\n" RESET, joueur[1]);
   strcat(affichagePlateau, buffer);

   sprintf(buffer, "\r\n");
   strcat(affichagePlateau, buffer);

   int i = 0;
   sprintf(buffer, "Case:  –00– –01– –02– –03– –04– –05– \r\n");
   strcat(affichagePlateau, buffer);
   sprintf(buffer, "––––– –––––––––––––––––––––––––––––––   –––––––––– \r\n");
   strcat(affichagePlateau, buffer);
   sprintf(buffer, JAUNE BOLD "J1 :  ");
   strcat(affichagePlateau, buffer);
   for (i = 0; i < 6; i++)
   {
      if (plateau[i] < 10)
      {
         sprintf(buffer, "| 0%d ", plateau[i]);
         strcat(affichagePlateau, buffer);
      }
      else
      {
         sprintf(buffer, "| %d ", plateau[i]);
         strcat(affichagePlateau, buffer);
      }
   }
   sprintf(buffer, "|     %d pts\r\n" RESET, points[0]);
   strcat(affichagePlateau, buffer);
   sprintf(buffer, "––––– –––––––––––––––––––––––––––––––   –––––––––– \r\n");
   strcat(affichagePlateau, buffer);

   sprintf(buffer, VIOLET BOLD "J2 :  ");
   strcat(affichagePlateau, buffer);
   for (i = 6; i < 12; i++)
   {
      if (plateau[i] < 10)
      {
         sprintf(buffer, "| 0%d ", plateau[i]);
         strcat(affichagePlateau, buffer);
      }
      else
      {
         sprintf(buffer, "| %d ", plateau[i]);
         strcat(affichagePlateau, buffer);
      }
   }

   sprintf(buffer, "|     %d pts\r\n" RESET, points[1]);
   strcat(affichagePlateau, buffer);
   sprintf(buffer, "––––– –––––––––––––––––––––––––––––––   –––––––––– \r\n");
   strcat(affichagePlateau, buffer);
   sprintf(buffer, "Case:  –06– –07– –08– –09– –10– –11– \r\n\r\n");
   strcat(affichagePlateau, buffer);
}

void sinscrire(int newsockfd, char *username, char **allUser, char **allUserOnline, char *myUsername)
{
   int joueurNonInscrit = 1; // 0 = inscrit ; 1 = non inscrit
   for (int j = 0; j < NBMAXJOUEUR; j++)
   {
      // printf("allUser[j] = %s\r\n", allUser[j]);

      if (strcmp(allUserOnline[j], username) == 0)
      {
         // joueur deja en ligne -> erreur faut un autre nom d'utilisateur
         joueurNonInscrit = 0;
         write_client(newsockfd, "0");
         break;
      }
      else if (strcmp(allUser[j], username) == 0)
      {
         // joueur deja inscrit -> Welcome back
         strcpy(allUserOnline[j], username);
         strcpy(myUsername, username);
         joueurNonInscrit = 0;
         write_client(newsockfd, "2");
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
         if (strcmp(allUser[j], "") == 0 && writeAllUser == 0)
         {
            strcpy(allUser[j], username);
            writeAllUser = 1;
         }
         if (strcmp(allUserOnline[j], "") == 0 && writeAllUserOnline == 0)
         {
            strcpy(allUserOnline[j], username);
            writeAllUserOnline = 1;
         }
         // printf("2allUser[j] = %s\r\n", allUser[j]);
         if (writeAllUser == 1 && writeAllUserOnline == 1)
         {
            break;
         }
      }
      strcpy(myUsername, username);
      write_client(newsockfd, "1");
   }
}

void listerJoueur(char **allUser, char *listePseudo)
{
   for (int j = 0; j < NBMAXJOUEUR; j++)
   {
      if (strcmp(allUser[j], "") != 0)
      {
         strcat(listePseudo, allUser[j]);
         strcat(listePseudo, "\r\n");
      }
   }
}
