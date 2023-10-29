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

   /* an array for all clients */
   Client allUsers[NBMAXJOUEUR];
   Game **allGames = malloc(NBMAXJOUEUR / 2 * sizeof(Game *));

   for (int i = 0; i < NBMAXJOUEUR; i++)
   {
      strcpy(allUsers[i].name, "");
      allUsers[i].state = NOTEXIST;
   }

   for (int i = 0; i < NBMAXJOUEUR / 2; i++)
   {
      allGames[i] = malloc(sizeof(Game));
      allGames[i]->gameID = -1;
   }

   /* FIN INIT*/

   SOCKET sock = init_connection();
   char buffer[BUF_SIZE];
   int max = sock;
   fd_set rdfs;

   char help[] = {VERT BOLD " ---   MENU   ---\r\n" RESET BOLD "Voici la liste des commandes utilisables :\r\n" RESET
                      VERT "  - 0 : " RESET "jouer\r\n" VERT "  - 1 : " RESET "afficher tous les pseudos et leur etat\r\n" VERT "  - 2 : " RESET "afficher tous les pseudos online et leur etat\r\n" VERT "  - 3 : " RESET "observer une partie\r\n" VERT "  - 4 : " RESET "consulter/éditer sa bio\r\n" VERT "  - 5 : " RESET "consulter la bio d'un autre joueur\r\n" VERT "  - 6 : " RESET "permet de quitter le jeu\r\n" VERT "  - help : " RESET "affiche les commandes utilisables\r\n" BOLD VERT " A tout moment" RESET " vous pouvez" VERT " tchatter" RESET " avec votre adversaire (si vous êtes en jeu) ou avec tous les utilisateurs connectés" VERT " en utilisant '->' " RESET "avant d'envoyer votre message\r\n"};

   char menu_bio[] = {VERT "0: " RESET "consulter votre bio\r\n" VERT "1: " RESET "modifier votre bio\r\n" VERT "2: " RESET "retourner au menu\r\n"};
   while (1)
   {
      int i = 0;
      FD_ZERO(&rdfs);

      /* add STDIN_FILENO */
      FD_SET(STDIN_FILENO, &rdfs);

      /* add the connection socket */
      FD_SET(sock, &rdfs);

      /* add socket of each client */
      for (i = 0; i < NBMAXJOUEUR; i++)
      {
         if (allUsers[i].state == NOTEXIST)
         {
            break;
         }
         if (allUsers[i].state != DISCONNECTED)
         {
            FD_SET(allUsers[i].sock, &rdfs);
         }
      }

      if (select(max + 1, &rdfs, NULL, NULL, NULL) == -1)
      {
         perror("select()");
         clear_games(allGames);
         clear_clients(allUsers);
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
         sinscrire(buffer, allUsers, &c);
      }
      else
      {
         int i = 0;
         for (i = 0; i < NBMAXJOUEUR; i++)
         {
            Client *client = &allUsers[i];

            if (client->state == NOTEXIST)
            {
               break;
            }
            else
            {

               /* a client is talking */
               if (FD_ISSET(allUsers[i].sock, &rdfs))
               {
                  int c = read_client(allUsers[i].sock, buffer);

                  /* client disconnected */
                  if (c == 0)
                  {

                     if (client->state == PLAYING_WAITING || client->state == PLAYING)
                     {
                        if (strcmp(client->name, client->game->players[0]->name) == 0)
                        {
                           write_client(client->game->players[1]->sock, VERT BOLD "Vous avez gagné par abandon de %s !🥳\r\n\r\n" RESET, client->name);
                           write_to_all_players(client->game->observers, VERT BOLD "%s a gagné par abandon de %s !🥳\r\n\r\n" RESET, client->game->players[1]->name, client->name);
                           client->game->players[1]->state = MENU;
                           write_to_all_players(client->game->observers, "%s", help);
                           int i = 0;
                           for (i = 0; i < NBMAXOBSERVER; i++)
                           {
                              if (client->game->observers[i] != NULL && client->game->observers[i]->state == OBSERVING)
                              {
                                 client->game->observers[i]->state = MENU;
                                 client->game->observers[i]->game = NULL;
                              }
                           }
                           write_client(client->game->players[1]->sock, "%s", help);
                        }

                        else
                        {
                           write_client(client->game->players[0]->sock, VERT BOLD "Vous avez gagné par abandon de %s !🥳\r\n\r\n" RESET, client->name);
                           write_to_all_players(client->game->observers, VERT BOLD "%s a gagné par abandon de %s !🥳\r\n\r\n" RESET, client->game->players[0]->name, client->name);
                           client->game->players[0]->state = MENU;
                           write_to_all_players(client->game->observers, "%s", help);
                           int i = 0;
                           for (i = 0; i < NBMAXOBSERVER; i++)
                           {
                              if (client->game->observers[i] != NULL && client->game->observers[i]->state == OBSERVING)
                              {
                                 client->game->observers[i]->state = MENU;
                                 client->game->observers[i]->game = NULL;
                              }
                           }
                           write_client(client->game->players[0]->sock, "%s", help);
                        }
                        int gameIDloc = client->game->gameID;
                        free(client->game);
                        allGames[gameIDloc]->gameID = -1;
                     }
                     else if (client->state == OBSERVING)
                     {
                        for (i = 0; i < NBMAXOBSERVER; i++)
                        {
                           if (client->game->observers[i] != NULL && strcmp(client->name, client->game->observers[i]->name) == 0)
                           {
                              client->game->observers[i] = NULL;
                              client->game->nbObservers--;
                              client->game = NULL;
                              break;
                           }
                        }
                     }

                     client->state = DISCONNECTED;
                     write_client(client->sock, BLEU "Vous avez quitté le jeu\r\n" RESET);
                     closesocket(client->sock);
                  }
                  else
                  {
                     // Chat : si le message contient "->"
                     // Envoie a tous le monde si le client est dans le lobby ou dans le menu
                     // Envoie a son adversaire si le client est en Game
                     if (strstr(buffer, "->") != NULL)
                     {
                        printf("Il a ecrit CHAT\n");
                        char *message = malloc(1024 * sizeof(char));
                        char *msg = strtok(buffer, "->");
                        sprintf(message, CYAN BOLD "%s : " RESET CYAN "%s\r\n" RESET, client->name, msg);

                        if (client->state == PLAYING || client->state == PLAYING_WAITING || client->state == CHALLENGED || client->state == WAITING_RESPONSE || client->state == OBSERVING || client->state == REQUESTING)
                        {
                           printf("EN GAME\n");
                           write_to_players(client->game->players, message);
                           write_to_all_players(client->game->observers, message);
                        }
                        else
                        {
                           printf("MENU LOBBY ou WAITING\n");
                           send_message_to_all_clients(allUsers, message);
                        }

                        free(message);
                     }
                     else if (client->state == REGISTERING)
                     {
                        if (strcmp(buffer, "OK") == 0)
                        {
                           write_client(client->sock, "%s", help);
                           client->state = MENU;
                           client->bio[0] = 0;
                        }
                     }
                     else if (client->state == MENU)
                     {
                        if (strcmp(buffer, "help\0") == 0)
                        {
                           printf("Il a ecrit HELP\n");
                           write_client(client->sock, "%s", help);
                        }
                        else if (strcmp(buffer, "0\0") == 0)
                        {
                           printf("Il a ecrit JOUER\n");
                           client->state = LOBBY;
                           write_client(client->sock, VERT "0: " RESET "défier un joueur\r\n" VERT "1: " RESET "attendre une invitation\r\n" VERT "2: " RESET "retourner au menu\r\n");
                        }
                        else if (strcmp(buffer, "1\0") == 0)
                        {
                           printf("Il a ecrit LISTE PSEUDO\n");
                           char *listePseudo = malloc(NBMAXJOUEUR * BUF_SIZE * sizeof(char));
                           listerJoueurNotState(allUsers, listePseudo, NOTEXIST, client);

                           write_client(client->sock, BOLD "Voici la liste des pseudos : \r\n" RESET "%s", listePseudo);
                           // on reinsitiallise listePseudo
                           listePseudo[0] = '\0';
                           free(listePseudo);
                        }
                        else if (strcmp(buffer, "2\0") == 0)
                        {
                           printf("Il a ecrit LISTE PSEUDO ONLINE\n");
                           char *listePseudoOnline = malloc(NBMAXJOUEUR * BUF_SIZE * sizeof(char));
                           listerJoueurNotState(allUsers, listePseudoOnline, DISCONNECTED, client);

                           write_client(client->sock, BOLD "Voici la liste des pseudos Online : \r\n" RESET "%s", listePseudoOnline);

                           listePseudoOnline[0] = 0;
                           free(listePseudoOnline);
                        }
                        else if (strcmp(buffer, "3\0") == 0)
                        {
                           printf("Il a ecrit OBSERVER\n");
                           char *listeGameEnCours = malloc(BUF_SIZE * sizeof(char));
                           listerGameEnCours(allGames, listeGameEnCours);
                           if (strlen(listeGameEnCours) > 1)
                           {
                              write_client(client->sock, BOLD "Voici la liste des games en cours : \r\n" RESET "%s", listeGameEnCours);
                              write_client(client->sock, BOLD "Veuillez entrer le numero de la game que vous voulez observer :" RESET);
                              client->state = CHOOSING_GAME;
                           }
                           else
                           {
                              write_client(client->sock, VIOLET BOLD "Aucune game en cours\r\n" RESET);
                              write_client(client->sock, "%s", help);
                              client->state = MENU;
                           }
                           listeGameEnCours[0] = 0;
                           free(listeGameEnCours);
                        }
                        else if (strcmp(buffer, "4\0") == 0)
                        {
                           printf("Il a écrit BIO\n");
                           client->state = MENU_BIO;
                           write_client(client->sock, "%s", menu_bio);
                        }
                        else if (strcmp(buffer, "5\0") == 0)
                        {
                           printf("Il a écrit LECTURE BIO\n");
                           char *listePseudos = malloc(NBMAXJOUEUR * BUF_SIZE * sizeof(char));
                           listerJoueurNotState(allUsers, listePseudos, NOTEXIST, client);
                           write_client(client->sock, BOLD "Voici la liste des pseudos : \r\n" RESET "%s\r\nChoisissez un pseudo : \r\n", listePseudos);
                           client->state = CHOOSING_BIO;
                           listePseudos[0] = '\0';
                           free(listePseudos);
                        }
                        else if (strcmp(buffer, "6\0") == 0)
                        {
                           printf("Il a ecrit QUITTER\n");

                           client->state = DISCONNECTED;
                           write_client(client->sock, BLEU "Vous avez quitté le jeu\r\n" RESET);
                           closesocket(client->sock);
                        }
                        else
                        {
                           write_client(client->sock, ROUGE BOLD "Commande non reconnue : %s" RESET "\nVeullez réessayer.\r\n", buffer);
                        }
                     }
                     else if (client->state == LOBBY)
                     {
                        if (strcmp(buffer, "0\0") == 0)
                        {
                           client->state = REQUESTING;
                           char *listePseudoOnline = malloc(NBMAXJOUEUR * BUF_SIZE * sizeof(char));
                           listerJoueurState(allUsers, listePseudoOnline, WAITING, client);
                           if (strlen(listePseudoOnline) > 1)
                           {
                              write_client(client->sock, BOLD "Voici la liste des joueurs en attente d'invitation:\r\n" RESET);
                              write_client(client->sock, listePseudoOnline);
                              write_client(client->sock, BOLD "Veuillez entrer le pseudo du joueur que vous voulez defier :" RESET);
                           }
                           else
                           {
                              write_client(client->sock, VIOLET BOLD "Aucun joueur dans la file d'attente\r\n" RESET);
                              write_client(client->sock, "%s", help);
                              client->state = MENU;
                           }

                           listePseudoOnline[0] = 0;
                           free(listePseudoOnline);
                        }
                        else if (strcmp(buffer, "1\0") == 0)
                        {
                           client->state = WAITING;
                           write_client(client->sock, VIOLET BOLD "En attente ...\r\n" RESET BOLD "Envoyer n'importe quoi pour annuler\r\n" RESET);
                        }
                        else if (strcmp(buffer, "2\0") == 0)
                        {
                           client->state = MENU;
                           write_client(client->sock, "%s", help);
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
                              if (strcmp(allUsers[j].name, buffer) == 0)
                              {
                                 // le joueur s'est deconnecté entre temps
                                 if (allUsers[j].state != WAITING)
                                 {
                                    write_client(client->sock, VIOLET BOLD "%s est parti de la WAITING LIST...\r\n" RESET, buffer);
                                 }
                                 else
                                 {
                                    write_client(client->sock, VIOLET BOLD "En attente de la réponse de %s...\r\n" RESET, buffer);
                                    allUsers[j].state = CHALLENGED;
                                    client->state = WAITING_RESPONSE;

                                    // on cree la partie et on ajoute les deux joueurs
                                    Game *game = malloc(sizeof(Game));
                                    for (int k = 0; k < 12; k++)
                                    {
                                       game->plateau[k] = 4;
                                    }

                                    game->players[0] = client; // 0 = celui qui request
                                    game->players[1] = &allUsers[j];

                                    client->game = game;
                                    allUsers[j].game = game;

                                    playerFound = 1;
                                    write_client(allUsers[j].sock, VIOLET BOLD "%s vous a défié\r\n" RESET "0: Accepter\r\n1: Refuser\r\n", client->name);
                                    break;
                                 }
                              }
                           }
                           if (playerFound == 0)
                           {
                              write_client(client->sock, ROUGE BOLD "Joueur introuvable\r\n" RESET);
                              client->state = LOBBY;
                              write_client(client->sock, VERT "0:" RESET " defier un joueur\r\n" VERT "1:" RESET " attendre une invitation\r\n");
                           }
                        }
                     }
                     else if (client->state == WAITING)
                     {
                        if (strlen(buffer) > 0)
                        {
                           client->state = MENU;
                           write_client(client->sock, "%s", help);
                        }
                     }
                     else if (client->state == MENU_BIO)
                     {
                        if (strcmp(buffer, "0\0") == 0)
                        {
                           write_client(client->sock, VERT BOLD "Votre bio :\r\n" RESET);
                           write_client(client->sock, "%s\r\n", client->bio);
                           write_client(client->sock, "%s", menu_bio);
                        }
                        else if (strcmp(buffer, "1\0") == 0)
                        {
                           client->state = WRITING_BIO;
                           write_client(client->sock, VERT BOLD "Ecrivez votre bio :\r\n" RESET);
                        }
                        else if (strcmp(buffer, "2\0") == 0)
                        {
                           client->state = MENU;
                           write_client(client->sock, "%s", help);
                        }
                        else
                        {
                           write_client(client->sock, ROUGE BOLD "Choix incrorrect !\r\n" RESET);
                        }
                     }
                     else if (client->state == WRITING_BIO)
                     {
                        if (strlen(buffer) > 0)
                        {
                           strcpy(client->bio, buffer);
                           write_client(client->sock, VERT BOLD "Votre bio a bien été modifiée !\r\n" RESET);
                           client->state = MENU_BIO;
                           write_client(client->sock, "%s", menu_bio);
                        }
                     }
                     else if (client->state == CHOOSING_BIO)
                     {
                        if (strlen(buffer) > 0)
                        {
                           int playerFound = 0;
                           for (int j = 0; j < NBMAXJOUEUR; j++)
                           {
                              if (strcmp(allUsers[j].name, buffer) == 0)
                              {
                                 write_client(client->sock, VERT BOLD "Bio de %s :" RESET " %s\r\n", allUsers[j].name, allUsers[j].bio);
                                 playerFound = 1;
                                 break;
                              }
                           }

                           if (playerFound == 0)
                           {
                              write_client(client->sock, ROUGE BOLD "Joueur introuvable\r\n" RESET);
                           }

                           write_client(client->sock, VERT "0: " RESET "Lire une autre bio\r\n" VERT "1: " RESET "retourner au menu\r\n");
                           client->state = READING_BIO;
                        }
                     }
                     else if (client->state == READING_BIO)
                     {
                        if (strlen(buffer) > 0)
                        {
                           if (strcmp(buffer, "0") == 0)
                           {
                              char *listePseudos = malloc(NBMAXJOUEUR * BUF_SIZE * sizeof(char));
                              listerJoueurNotState(allUsers, listePseudos, NOTEXIST, client);
                              write_client(client->sock, BOLD "Voici la liste des pseudos : \r\n" RESET "%s\r\nChoisissez un pseudo : \r\n", listePseudos);
                              client->state = CHOOSING_BIO;
                              listePseudos[0] = '\0';
                              free(listePseudos);
                           }
                           else if (strcmp(buffer, "1") == 0)
                           {
                              write_client(client->sock, "%s", help);
                              client->state = MENU;
                           }
                           else if (strlen(buffer) > 0)
                           {
                              write_client(client->sock, ROUGE BOLD "Commande incorrecte\r\n" RESET);
                           }
                        }
                     }
                     else if (client->state == CHALLENGED)
                     {
                        if (strcmp(buffer, "0\0") == 0)
                        {

                           char *affichagePlateau = malloc(1024 * sizeof(char));
                           // affichage du plateau
                           genererAffPlateau(client->game, affichagePlateau);
                           write_to_players(client->game->players, affichagePlateau);
                           free(affichagePlateau);

                           // tirer un nombre aleatoire entre 0 et 1 et defini qui commence
                           if (time(0) % 2)
                           {
                              client->id = 0;
                              client->state = PLAYING;
                              client->game->players[0]->id = 1;
                              client->game->players[0]->state = PLAYING_WAITING;

                              write_client(client->sock, "Le Hasard a décidé que vous commenciez\r\n");
                              write_client(client->sock, "%s, choisissez une case non-vide (entre 0 et 5): ", client->name);

                              write_client(client->game->players[0]->sock, "Le Hasard a décidé que %s commencait\r\n", client->name);
                           }
                           else
                           {
                              client->id = 1;
                              client->state = PLAYING_WAITING;
                              client->game->players[0]->id = 0;
                              client->game->players[0]->state = PLAYING;

                              write_client(client->game->players[0]->sock, "Le Hasard a décidé que vous commenciez\r\n");
                              write_client(client->game->players[0]->sock, "%s, choisissez une case non-vide (entre 0 et 5): ", client->game->players[0]->name);

                              write_client(client->sock, "Le Hasard a décidé que %s commencait\r\n", client->game->players[0]->name);
                           }

                           for (int j = 0; j < NBMAXJOUEUR / 2; j++)
                           {
                              if (allGames[j]->gameID == -1)
                              {
                                 allGames[j] = client->game;
                                 allGames[j]->gameID = j;
                                 break;
                              }
                           }
                        }
                        else if (strcmp(buffer, "1\0") == 0)
                        {
                           client->state = WAITING;
                           client->game->players[0]->state = MENU;
                           write_client(client->game->players[0]->sock, ROUGE "%s n'a pas accepté le challenge...'\r\n" RESET, client->name);
                           write_client(client->sock, VERT BOLD "En attente ...\r\nEnvoyer n'importe quoi pour annuler\r\n" RESET);

                           write_client(client->game->players[0]->sock, "%s", help);
                        }
                     }
                     else if (client->state == CHOOSING_GAME)
                     {
                        if (strlen(buffer) > 0)
                        {
                           int gameFound = 0;
                           int k = atoi(buffer);

                           if (k != 0)
                           {
                              k--;

                              if (allGames[k]->gameID == 0)
                              {
                                 allGames[k]->observers[allGames[k]->nbObservers++] = client;
                                 client->state = OBSERVING;
                                 client->game = allGames[k];
                                 write_client(client->sock, "Vous observez la partie " JAUNE "%s" RESET " (%d pts) VS " VIOLET "%s" RESET " (%d pts)\r\n", allGames[k]->players[0]->name, allGames[k]->points[0], allGames[k]->players[1]->name, allGames[k]->points[1]);
                                 write_client(client->sock, "Si vous souhaitez quitter, tapez" ROUGE " 'quitter' \r\n" RESET);

                                 gameFound = 1;
                              }

                              if (gameFound == 0)
                              {
                                 write_client(client->sock, ROUGE BOLD "Partie introuvable\r\n" RESET);
                                 client->state = MENU;
                                 write_client(client->sock, "%s", help);
                              }
                           }
                           else
                           {
                              write_client(client->sock, ROUGE BOLD "Saisie incorrecte\r\n" RESET);
                              client->state = MENU;
                              write_client(client->sock, "%s", help);
                           }
                        }
                     }
                     else if (client->state == OBSERVING)
                     {
                        if (strcmp(buffer, "quitter") == 0)
                        {
                           int i = 0;
                           for (i = 0; i < NBMAXOBSERVER; i++)
                           {
                              if (client->game->observers[i] != NULL && strcmp(client->game->observers[i]->name, client->name) == 0)
                              {
                                 client->game->observers[i] = NULL;
                                 break;
                              }
                           }
                           client->game->nbObservers--;
                           client->game = NULL;
                           client->state = MENU;
                           write_client(client->sock, "%s", help);
                        }
                        else if (strlen(buffer) > 0)
                        {
                           write_client(client->sock, ROUGE BOLD "En tant qu'observateur, vous ne pouvez pas prendre part au jeu...\r\nVous pouvez tout de même tchatter !" RESET);
                        }
                     }
                     else if (client->state == PLAYING)
                     {
                        int caseChoisie = atoi(buffer);
                        char *message = malloc(2048 * sizeof(char));
                        char *affichagePlateau = malloc(1024 * sizeof(char));
                        message[0] = 0;
                        affichagePlateau[0] = 0;

                        int retour = jouer(client->game, client->id, caseChoisie, message);
                        printf("retour = %d\r\n", retour);
                        if (retour >= 0)
                        {
                           if (isFinish(client->game, message) == 1)
                           {
                              write_to_players(client->game->players, message);
                              client->game->players[0]->state = MENU;
                              client->game->players[1]->state = MENU;
                              int i = 0;
                              for (i = 0; i < NBMAXOBSERVER; i++)
                              {
                                 if (client->game->observers[i] != NULL && client->game->observers[i]->state == OBSERVING)
                                 {
                                    client->game->observers[i]->state = MENU;
                                    client->game->observers[i]->game = NULL;
                                 }
                              }
                              write_to_players(client->game->players, "%s", help);
                              write_to_all_players(client->game->observers, "%s", help);

                              int gameIDloc = client->game->gameID;
                              free(client->game);
                              allGames[gameIDloc]->gameID = -1;
                           }
                           else
                           {
                              genererAffPlateau(client->game, affichagePlateau);
                              write_to_players(client->game->players, message);
                              write_to_players(client->game->players, affichagePlateau);
                              write_to_all_players(client->game->observers, "%s", message);
                              write_to_all_players(client->game->observers, affichagePlateau);
                              if (checkStarvation(client->game, 0) == 1 || checkStarvation(client->game, 1))
                              {
                                 char *starvation = malloc(1024 * sizeof(char));
                                 starvation[0] = 0;
                                 strcat(starvation, "\e[0;31m\033[1mStarvation !\033[0m\r\n");
                                 write_to_players(client->game->players, starvation);
                                 write_to_all_players(client->game->observers, starvation);
                                 free(starvation);
                              }
                              if (client->game->players[0]->state == PLAYING_WAITING)
                              {
                                 client->game->players[0]->state = PLAYING;
                                 write_client(client->game->players[0]->sock, "%s, choisissez une case non-vide (entre %d et %d): ", client->game->players[0]->name, 6 * client->game->players[0]->id, 6 * client->game->players[0]->id + 5);
                                 write_client(client->game->players[1]->sock, BOLD "C'est au tour de %s" RESET, client->game->players[0]->name);
                                 write_to_all_players(client->game->observers, BOLD "C'est au tour de %s" RESET, client->game->players[0]->name);
                              }
                              else
                              {
                                 client->game->players[1]->state = PLAYING;
                                 write_client(client->game->players[1]->sock, "%s, choisissez une case non-vide (entre %d et %d): ", client->game->players[1]->name, 6 * client->game->players[1]->id, 6 * client->game->players[1]->id + 5);
                                 write_client(client->game->players[0]->sock, BOLD "C'est au tour de %s" RESET, client->game->players[1]->name);
                                 write_to_all_players(client->game->observers, BOLD "C'est au tour de %s" RESET, client->game->players[1]->name);
                              }
                              client->state = PLAYING_WAITING;
                           }
                        }
                        else
                        {
                           write_client(client->sock, message);
                        }
                        free(message);
                        free(affichagePlateau);
                     }
                     else if (client->state == PLAYING_WAITING)
                     {
                        if (strlen(buffer) > 0)
                        {
                           write_client(client->sock, ROUGE BOLD "Ce n'est pas votre tour\r\n" RESET);
                        }
                     }
                  }
                  break;
               }
            }
         }
      }
   }

   clear_games(allGames);
   clear_clients(allUsers);
   end_connection(sock);
}

static void clear_clients(Client *clients)
{
   int i = 0;
   for (i = 0; i < NBMAXJOUEUR; i++)
   {
      if ((strcmp(clients[i].name, "") == 0) && clients[i].state != DISCONNECTED && clients[i].sock != INVALID_SOCKET)
      {
         closesocket(clients[i].sock);
      }
   }
}

static void send_message_to_all_clients(Client *clients, const char *message)
{
   int i = 0;
   for (i = 0; i < NBMAXJOUEUR; i++)
   {
      /* we don't send message to the sender */
      if (clients[i].state != NOTEXIST)
      {
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

   if (listen(sock, NBMAXJOUEUR) == SOCKET_ERROR)
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
   char buffer[1024];

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

static void write_to_players(Client **players, const char *message, ...)
{
   va_list args;
   va_start(args, message);

   // Créez un tampon pour formater le message
   char buffer[1024];

   // Formatez le message en utilisant vsnprintf
   vsnprintf(buffer, sizeof(buffer), message, args);

   int i = 0;
   // a changer en fonction du nombre d'observateurs
   for (i = 0; i < 2; i++)
   {
      write_client(players[i]->sock, buffer);
   }
   va_end(args);
}

static void write_to_all_players(Client **players, const char *message, ...)
{
   va_list args;
   va_start(args, message);

   // Créez un tampon pour formater le message
   char buffer[1024];

   // Formatez le message en utilisant vsnprintf
   vsnprintf(buffer, sizeof(buffer), message, args);

   int i = 0;
   // pb si nbmaxobserver < nbmaxjoueur : pas tout le monde a un msg
   for (i = 0; i < NBMAXOBSERVER; i++)
   {
      if (players[i] != NULL && players[i]->state != NOTEXIST && players[i]->state != DISCONNECTED)
      {
         write_client(players[i]->sock, buffer);
      }
   }
   va_end(args);
}

static void clear_games(Game **games)
{
   int i = 0;
   for (i = 0; i < NBMAXJOUEUR / 2; i++)
   {
      free(games[i]);
   }
   free(games);
}

int main(int argc, char **argv)
{
   printf(BOLD "Server started\n" RESET);
   init();

   app();

   end();
   printf(BOLD "Server stopped\n" RESET);

   return EXIT_SUCCESS;
}

void sinscrire(char *username, Client *allUsers, Client *client)
{
   for (int j = 0; j < NBMAXJOUEUR; j++)
   {
      if (allUsers[j].state == DISCONNECTED && strcmp(username, allUsers[j].name) == 0)
      {
         // joueur deja inscrit -> Welcome back
         allUsers[j] = *client;
         allUsers[j].state = REGISTERING;
         write_client(client->sock, "WelcomeBack\r\n");

         break;
      }
      else if (allUsers[j].state != NOTEXIST && allUsers[j].state != DISCONNECTED && strcmp(username, allUsers[j].name) == 0)
      {
         // joueur deja en ligne -> erreur faut un autre nom d'utilisateur
         write_client(client->sock, "DejaEnLigne\r\n");

         break;
      }
      else if (allUsers[j].state == NOTEXIST)
      {
         // joueur non inscrit -> inscription
         allUsers[j] = *client;
         allUsers[j].state = REGISTERING;
         write_client(client->sock, "Inscription\r\n");
         break;
      }
   }
}

void listerJoueurState(Client *allUsers, char *listePseudo, enum States state, Client *client)
{
   for (int j = 0; j < NBMAXJOUEUR; j++)
   {
      if (allUsers[j].state == NOTEXIST)
      {
         break;
      }
      else if (allUsers[j].state == state && allUsers[j].name == client->name)
      {
         strcat(listePseudo, JAUNE);
         strcat(listePseudo, allUsers[j].name);
         strcat(listePseudo, " (you)" RESET);
         strcat(listePseudo, "\r\n");
      }
      else if (allUsers[j].state == state)
      {
         strcat(listePseudo, allUsers[j].name);

         if (allUsers[j].state == PLAYING_WAITING || allUsers[j].state == PLAYING)
         {
            strcat(listePseudo, " (en jeu)" RESET);
         }
         else if (allUsers[j].state == OBSERVING)
         {
            strcat(listePseudo, " (regarde une partie)" RESET);
         }

         strcat(listePseudo, "\r\n");
      }
   }
}

void listerJoueurNotState(Client *allUsers, char *listePseudo, enum States state, Client *client)
{
   for (int j = 0; j < NBMAXJOUEUR; j++)
   {

      if (allUsers[j].state == NOTEXIST)
      {
         break;
      }
      else if (allUsers[j].state != state && allUsers[j].name == client->name)
      {
         strcat(listePseudo, JAUNE);
         strcat(listePseudo, allUsers[j].name);
         strcat(listePseudo, " (you)" RESET);
         strcat(listePseudo, "\r\n");
      }
      else if (allUsers[j].state != state)
      {
         strcat(listePseudo, allUsers[j].name);

         if (allUsers[j].state == PLAYING_WAITING || allUsers[j].state == PLAYING)
         {
            strcat(listePseudo, " (en jeu)" RESET);
         }
         else if (allUsers[j].state == OBSERVING)
         {
            strcat(listePseudo, " (regarde une partie)" RESET);
         }

         strcat(listePseudo, "\r\n");
      }
   }
}

void listerGameEnCours(Game **allGames, char *playedGame)
{
   char *buffer = malloc(1024 * sizeof(char));
   playedGame[0] = 0;
   for (int j = 0; j < NBMAXJOUEUR / 2; j++)
   {
      if (allGames[j]->gameID != -1)
      {
         sprintf(buffer, VERT BOLD "%d: " RESET JAUNE "%s" RESET " (%d pts) VS " VIOLET "%s" RESET " (%d pts)\r\n", j + 1, allGames[j]->players[0]->name, allGames[j]->points[0], allGames[j]->players[1]->name, allGames[j]->points[1]);
         strcat(playedGame, buffer);
      }
   }

   free(buffer);
}