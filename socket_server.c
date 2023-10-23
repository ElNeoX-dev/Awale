/* Serveur sockets TCP
 * affichage de ce qui arrive sur la socket
 *    socket_server port (port > 1024 sauf root)
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdarg.h>

#define VERT "\e[0;32m"
#define ROUGE "\e[0;31m"
#define JAUNE "\e[1;33m"
#define VIOLET "\e[0;35m"
#define BLEU "\e[0;34m"
#define BOLD "\033[1m"
#define RESET "\033[0m"

#define NBMAXJOUEUR 20
#define TAILLEMAXCHARJOUEUR 20

void genererAffPlateau(int *plateau, char **joueur, int *points, int newsockfd, char *affichagePlateau);
void sinscrire(int newsockfd, char *username, char **allUser, char **allUserOnline, char *myUsername);
void listerJoueur(int newsockfd, char **allUser, char *listePseudo);
void printStdEtSocket(int newsockfd, const char *message, ...);

int main(int argc, char **argv)
{

  // maxi 20 joueurs
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

  char datas[] = "hello\n";
  int sockfd, newsockfd, clilen, chilpid, ok, nleft, nbwriten;
  char c;
  struct sockaddr_in cli_addr, serv_addr;

  if (argc != 2)
  {
    printf("usage: socket_server port\n");
    exit(0);
  }

  printf("server starting...\n");

  /* ouverture du socket */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
  {
    printf("impossible d'ouvrir le socket\n");
    exit(0);
  }

  /* initialisation des parametres */
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(atoi(argv[1]));

  /* effecture le bind */
  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("impossible de faire le bind\n");
    exit(0);
  }

  /* petit initialisation */
  listen(sockfd, 1);

  /* attend la connection d'un client */
  clilen = sizeof(cli_addr);
  signal(SIGCHLD, SIG_IGN);
  while (1)
  {
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    int pid = fork();
    if (pid == 0) /* c’est le fils */
    {
      close(sockfd); /* socket inutile pour le fils */

      /* traiter la communication */
      if (newsockfd < 0)
      {
        printf("accept error\n");
        exit(0);
      }
      printf("connection accepted\n");

      int bytes_received = 0;
      size_t data_len = 0;
      char buffer[1024];
      char received_data[1024];
      char *message = malloc(2048 * sizeof(char));
      char *myUsername = malloc(TAILLEMAXCHARJOUEUR * sizeof(char));

      while (1)
      {

        // si ce qui est lu est égal à "action\n" alors on execute la commande
        // sinon on affiche le message d'erreur
        bytes_received = recv(newsockfd, buffer, sizeof(buffer), 0);

        if (bytes_received <= 0)
        {
          break; // Fin de la connexion
        }

        for (int i = 0; i < bytes_received; i++)
        {
          if (buffer[i] == '\n')
          {
            // Si le caractère '\n' est trouvé, cela signifie que nous avons reçu une ligne complète
            received_data[data_len] = '\0'; // Terminer la chaîne de caractères
            printf("Ligne reçue: %s\n", received_data);

            if (strcmp(received_data, "afficher\0\n") == 0)
            {
              printf("Il a ecrit afficher\n");
              char *affichagePlateau = malloc(1024 * sizeof(char));
              genererAffPlateau(plateau, joueur, points, newsockfd, affichagePlateau);
              printStdEtSocket(newsockfd, affichagePlateau);
              free(affichagePlateau);
            }
            else if (strcmp(received_data, "action\0\n") == 0)
            {
              printf("Il a ecrit ACTION\n");

              printStdEtSocket(newsockfd, ROUGE BOLD "/!\\ Choix non valide !\r\n" RESET);
            }
            else if (strstr(received_data, "inscription") != NULL)
            {
              printf("Il a ecrit INSCRIPTION\n");
              char *username = strtok(received_data, ":");
              username = strtok(NULL, ":");
              printf("username = %s\r\n", username);

              sinscrire(newsockfd, username, allUser, allUserOnline, myUsername);
            }
            else if (strcmp(received_data, "listepseudo\0\n") == 0)
            {
              printf("Il a ecrit LISTE PSEUDO\n");
              char *listePseudo = malloc(NBMAXJOUEUR * (TAILLEMAXCHARJOUEUR + 2) * sizeof(char));
              listerJoueur(newsockfd, allUser, listePseudo);

              printStdEtSocket(newsockfd, BOLD "Voici la liste des pseudos : \r\n" RESET "%s", listePseudo);

              free(listePseudo);
            }
            else if (strcmp(received_data, "listepseudoonline\0\n") == 0)
            {
              printf("Il a ecrit LISTE PSEUDO ONLINE\n");
              char *listePseudoOnline = malloc(NBMAXJOUEUR * (TAILLEMAXCHARJOUEUR + 2) * sizeof(char));
              listerJoueur(newsockfd, allUserOnline, listePseudoOnline);

              printStdEtSocket(newsockfd, BOLD "Voici la liste des pseudos : \r\n" RESET "%s", listePseudoOnline);

              free(listePseudoOnline);
            }
            else if (strcmp(received_data, "quitter\0\n") == 0)
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

              printStdEtSocket(newsockfd, BLEU BOLD "Vous avez quitté la partie.\r\n" RESET);
              close(newsockfd);
              exit(0);
            }
            else if (strcmp(received_data, "help\0\n") == 0)
            {
              printf("Il a ecrit HELP\n");

              char help[] = {VERT BOLD "Voici la liste des commandes utilisables :\r\n" RESET
                                 VERT "  - afficher : affiche le plateau de jeu\r\n"
                                       "  - action : permet de jouer un coup\r\n"
                                       "  - help : affiche la liste des commandes utilisables\r\n"
                                       "  - quitter : permet de quitter le jeu\r\n" RESET};

              printStdEtSocket(newsockfd, "%s", help);
            }
            else
            {
              printStdEtSocket(newsockfd, ROUGE BOLD "Commande non reconnue : %s" RESET "\nVeullez réessayer.\r\n", received_data);
            }

            // Réinitialisez received_data pour la prochaine ligne
            data_len = 0;
          }
          else
          {
            received_data[data_len++] = buffer[i];
          }
        }

        /*
        char buffer[1024 * sizeof(char)];
        bzero(buffer, sizeof(buffer));
        read(newsockfd, buffer, sizeof(buffer));

        if (strcmp(buffer, "afficher\n") == 0)
        {
          printf("Il a ecrit afficher\n");

          // ecrire dans le socket le message
          char *message = malloc(1024 * sizeof(char));
          afficherPlateau(plateau, joueur, points, message);
          write(newsockfd, message, strlen(message));
        }
        else if (strcmp(buffer, "action\n") == 0)
        {
          printf("Il a ecrit ACTION\n");

          // ecrire dans le socket le message
          char *message = "\e[0;31m\033[1m/!\\ Choix non valide !\033[0m\r\n";
          write(newsockfd, message, strlen(message));
        }
        else
        {
          printf("message recu: %s\n", buffer);
          write(newsockfd, buffer, strlen(buffer));
        }

        bzero(buffer, sizeof(buffer));
        */
      }

      close(newsockfd);
      exit(0); /* on force la terminaison du fils */
    }
    else /* c’est le pere */
    {
      close(newsockfd); /* socket inutile pour le pere */
    }
  }

  close(sockfd);

  /*  attention il s'agit d'une boucle infinie
   *  le socket nn'est jamais ferme !
   */

  return 1;
}
/*
void afficherPlateau(int *plateau, char **joueur, int *points, char *message)
{

  char *buffer = malloc(1024 * sizeof(char));
  printf("\e[1;33m\033[1mJoueur 1 : %s\033[0m\r\n", joueur[0]);
  sprintf(buffer, "\e[1;33m\033[1mJoueur 1 : %s\033[0m\r\n", joueur[0]);
  strcat(message, buffer);

  printf("\e[0;35m\033[1mJoueur 2 : %s\033[0m\r\n", joueur[1]);
  sprintf(buffer, "\e[0;35m\033[1mJoueur 2 : %s\033[0m\r\n", joueur[1]);
  strcat(message, buffer);
  printf("\r\n");

  sprintf(buffer, "\r\n");
  strcat(message, buffer);

  int i = 0;
  printf("Case:  –00– –01– –02– –03– –04– –05– \r\n");
  sprintf(buffer, "Case:  –00– –01– –02– –03– –04– –05– \r\n");
  strcat(message, buffer);

  printf("––––– –––––––––––––––––––––––––––––––   –––––––––– \r\n");
  sprintf(buffer, "––––– –––––––––––––––––––––––––––––––   –––––––––– \r\n");
  strcat(message, buffer);

  printf("\e[1;33m\033[1mJ1 :  ");
  sprintf(buffer, "\e[1;33m\033[1mJ1 :  ");
  strcat(message, buffer);

  for (i = 0; i < 6; i++)
  {
    if (plateau[i] < 10)
    {
      printf("| 0%d ", plateau[i]);
      sprintf(buffer, "| 0%d ", plateau[i]);
      strcat(message, buffer);
    }
    else
    {
      printf("| %d ", plateau[i]);
      sprintf(buffer, "| %d ", plateau[i]);
      strcat(message, buffer);
    }
  }
  printf("|     %d pts\033[0m\r\n", points[0]);
  sprintf(buffer, "|     %d pts\033[0m\r\n", points[0]);
  strcat(message, buffer);

  printf("––––– –––––––––––––––––––––––––––––––   –––––––––– \r\n");
  sprintf(buffer, "––––– –––––––––––––––––––––––––––––––   –––––––––– \r\n");
  strcat(message, buffer);

  printf("\e[0;35m\033[1mJ2 :  ");
  sprintf(buffer, "\e[0;35m\033[1mJ2 :  ");
  strcat(message, buffer);

  for (i = 6; i < 12; i++)
  {
    if (plateau[i] < 10)
    {
      printf("| 0%d ", plateau[i]);
      sprintf(buffer, "| 0%d ", plateau[i]);
      strcat(message, buffer);
    }
    else
    {
      printf("| %d ", plateau[i]);
      sprintf(buffer, "| %d ", plateau[i]);
      strcat(message, buffer);
    }
  }

  printf("|     %d pts\033[0m\r\n", points[1]);
  sprintf(buffer, "|     %d pts\033[0m\r\n", points[1]);
  strcat(message, buffer);

  printf("––––– –––––––––––––––––––––––––––––––   –––––––––– \r\n");
  sprintf(buffer, "––––– –––––––––––––––––––––––––––––––   –––––––––– \r\n");
  strcat(message, buffer);

  printf("Case:  –06– –07– –08– –09– –10– –11– \r\n\r\n");
  sprintf(buffer, "Case:  –06– –07– –08– –09– –10– –11– \r\n\r\n");
  strcat(message, buffer);

  strcat(message, "\0");

  free(buffer);
}
*/
/*
void afficherPlateau(int *plateau, char **joueur, int *points, int newsockfd)
{
  char *message = malloc(1024 * sizeof(char));
  printStdEtSocket(newsockfd, JAUNE BOLD "Joueur 1 : %s\r\n" RESET, joueur[0]);
  printStdEtSocket(newsockfd, VIOLET BOLD "Joueur 2 : %s\r\n" RESET, joueur[1]);

  printStdEtSocket(newsockfd, "\r\n");

  int i = 0;
  printStdEtSocket(newsockfd, "Case:  –00– –01– –02– –03– –04– –05– \r\n");
  printStdEtSocket(newsockfd, "––––– –––––––––––––––––––––––––––––––   –––––––––– \r\n");
  printStdEtSocket(newsockfd, JAUNE BOLD "J1 :  ");
  for (i = 0; i < 6; i++)
  {
    if (plateau[i] < 10)
    {
      printStdEtSocket(newsockfd, "| 0%d ", plateau[i]);
    }
    else
    {
      printStdEtSocket(newsockfd, "| %d ", plateau[i]);
    }
  }
  printStdEtSocket(newsockfd, "|     %d pts\r\n" RESET, points[0]);
  printStdEtSocket(newsockfd, "––––– –––––––––––––––––––––––––––––––   –––––––––– \r\n");

  printStdEtSocket(newsockfd, VIOLET BOLD "J2 :  ");
  for (i = 6; i < 12; i++)
  {
    if (plateau[i] < 10)
    {
      printStdEtSocket(newsockfd, "| 0%d ", plateau[i]);
    }
    else
    {
      printStdEtSocket(newsockfd, "| %d ", plateau[i]);
    }
  }

  printStdEtSocket(newsockfd, "|     %d pts\r\n" RESET, points[1]);
  printStdEtSocket(newsockfd, "––––– –––––––––––––––––––––––––––––––   –––––––––– \r\n");
  printStdEtSocket(newsockfd, "Case:  –06– –07– –08– –09– –10– –11– \r\n\r\n");
}
*/
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

    if (strcmp(allUser[j], username) == 0)
    {
      // joueur deja inscrit -> Welcome back
      strcpy(allUserOnline[j], username);
      strcpy(myUsername, username);
      joueurNonInscrit = 0;
      printStdEtSocket(newsockfd, "2");
      break;
    }
    else if (strcmp(allUserOnline[j], username) == 0)
    {
      // joueur deja en ligne -> erreur faut un autre nom d'utilisateur
      joueurNonInscrit = 0;
      printStdEtSocket(newsockfd, "0");
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
    printStdEtSocket(newsockfd, "1");
  }
}

void listerJoueur(int newsockfd, char **allUser, char *listePseudo)
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

void printStdEtSocket(int newsockfd, const char *message, ...)
{
  va_list args;
  va_start(args, message);

  // Créez un tampon pour formater le message
  char buffer[1024]; // Vous pouvez ajuster la taille du tampon en fonction de vos besoins

  // Formatez le message en utilisant vsnprintf
  vsnprintf(buffer, sizeof(buffer), message, args);

  // Affichez le message à la sortie standard
  printf("%s", buffer);
  write(newsockfd, buffer, strlen(buffer) + 1); // le +1 correspond au '\0'

  va_end(args);
}
