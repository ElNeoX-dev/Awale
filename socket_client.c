/* Client pour les sockets
 *    socket_client ip_server port
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define VERT "\e[0;32m"
#define ROUGE "\e[0;31m"
#define JAUNE "\e[1;33m"
#define VIOLET "\e[0;35m"
#define BLEU "\e[0;34m"
#define BOLD "\033[1m"
#define RESET "\033[0m"

int sinscrire(int sockfd);

int main(int argc, char **argv)
{
  int sockfd, newsockfd, clilen, chilpid, ok, nleft, nbwriten;
  char c;
  struct sockaddr_in cli_addr, serv_addr;

  if (argc != 3)
  {
    printf("usage  socket_client server port\n");
    exit(0);
  }

  /*
   *  partie client
   */
  printf("client starting\n");

  /* initialise la structure de donnee */
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
  serv_addr.sin_port = htons(atoi(argv[2]));

  /* ouvre le socket */
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("socket error\n");
    exit(0);
  }

  /* effectue la connection */
  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("socket error\n");
    exit(0);
  }

  int readwrite = 1; // read = 0 ; write = 1
  char buffer[1024];

  int inscrit = 0; // 0 = non inscrit ; 1 = inscrit
  inscrit = sinscrire(sockfd);

  while (1)
  {

    /*while (readwrite == 1)
    { */
    /*c = getchar();

    if (c == EOF)
    {
      close(sockfd);
      printf("Socket fermée");
      exit(0);
    }

    write(sockfd, &c, 1);

    if (c == '\n')
    {
      readwrite = 0;
    }*/
    printf(BOLD "\nEntrez une commande ou faites \"help\" pour accéder à la liste des commandes disponibles : " RESET);
    fgets(buffer, sizeof(buffer), stdin);
    write(sockfd, buffer, strlen(buffer));

    // readwrite = 0;
    // }

    int bytes_received = 0;
    size_t data_len = 0;
    char received_data[1024];

    /*while (readwrite == 0)
    {*/
    bzero(buffer, sizeof(buffer)); // Réinitialisez le tampon
    bytes_received = recv(sockfd, buffer, sizeof(buffer), 0);

    if (bytes_received <= 0)
    {
      printf("Connection closed : byte received < 0\n");
      break; // Fin de la connexion
    }

    for (int i = 0; i < bytes_received; i++)
    {
      // printf("buffer[i] = %c\n", buffer[i]);
      if (buffer[i] == '\0')
      {
        received_data[data_len] = '\0'; // Terminer la chaîne de caractères
        // printf("Ligne reçue: %s\n", received_data);
        printf("%s", received_data);
        // readwrite = 1;
        data_len = 0;
      }
      else
      {
        received_data[data_len++] = buffer[i];
      }
      //}

      /* //Si jamais on ne veut faire qu'un write par commande
      bytes_received = recv(sockfd, buffer, sizeof(buffer), 0);

      if (bytes_received <= 0)
      {
        printf("Connection closed : byte received < 0\n");
        break; // Fin de la connexion
      }

      // Ajouter une boucle pour vider le buffer
      for (int i = 0; i < bytes_received; i++)
      {
        received_data[data_len++] = buffer[i];
      }

      // Vérifier si la fin du message a été atteinte
      if (received_data[data_len] == '\0')
      {
        printf("%s", received_data);
        readwrite = 1;
        data_len = 0;
        memset(received_data, 0, sizeof(received_data));
      }
      */
    }

    if (strstr(received_data, "quitté") != NULL)
    {
      close(sockfd);
      break;
    }
  }

  return 1;
}

int sinscrire(int sockfd)
{
  char username[1024];
  int ok = 0;

  while (ok == 0)
  {
    printf(BOLD "\nEntrez votre nom d'utilisateur : " RESET);
    fgets(username, sizeof(username), stdin);
    char *message = malloc(strlen("inscription:") + strlen(username));
    strcat(message, "inscription:");
    strcat(message, username);
    write(sockfd, message, strlen(message));

    int buffer[1024];
    int received_data[1024];
    int bytes_received = recv(sockfd, buffer, sizeof(buffer), 0);
    int data_len = 0;

    if (bytes_received <= 0)
    {
      printf("Connection closed : byte received < 0\n");
      break; // Fin de la connexion
    }

    // Ajouter une boucle pour vider le buffer
    for (int i = 0; i < bytes_received; i++)
    {
      received_data[data_len++] = buffer[i];
    }

    // Vérifier si la fin du message a été atteinte
    if (received_data[data_len] == '\0')
    {

      // regarde la valeur de retour
      // si retour == 1, ok = 1 -> utilisateur crée
      // si retour == 2, welcome back -> utilisateur déjà crée
      // si retour == 0, erreur -> utilisateur déjà connecté
      if (strcmp(received_data, "1") == 0)
      {
        ok = 1;
        printf("Bienvenue %s, nous venons de creer votre utilisateur", username);
      }
      else if (strcmp(received_data, "2") == 0)
      {
        ok = 1;
        printf("Welcome back %s", username);
      }
      else
      {
        printf("Cet utilisateur est déjà connecté");
      }

      free(message);
    }
  }

  return ok;
}