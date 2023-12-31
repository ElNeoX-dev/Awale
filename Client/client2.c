#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

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

static void app(const char *address, const char *name)
{
   SOCKET sock = init_connection(address);
   char buffer[BUF_SIZE];
   int isRegistered = 0;
   fd_set rdfs;
   char registerMsg[BUF_SIZE];

   strcat(registerMsg, "inscription:");
   strcat(registerMsg, name);
   strcat(registerMsg, "\r\n");
   /* send our name */
   write_server(sock, name);

   // printf(BOLD BLEU "Bienvenue %s sur notre serveur de jeu Awale. Appuyez sur entrer pour continuer\r\n" RESET);

   int firstTime = 0;

   while (1)
   {
      FD_ZERO(&rdfs);

      /* add STDIN_FILENO */
      FD_SET(STDIN_FILENO, &rdfs);

      /* add the socket */
      FD_SET(sock, &rdfs);

      if (select(sock + 1, &rdfs, NULL, NULL, NULL) == -1)
      {
         perror("select()");
         exit(errno);
      }

      /* something from standard input : i.e keyboard */
      if (FD_ISSET(STDIN_FILENO, &rdfs))
      {
         fgets(buffer, BUF_SIZE - 1, stdin);
         {
            char *p = NULL;
            p = strstr(buffer, "\n");
            if (p != NULL)
            {
               *p = 0;
            }
            else
            {
               /* fclean */
               buffer[BUF_SIZE - 1] = 0;
            }
         }
         // printf("--%s--\r\n", buffer);
         write_server(sock, buffer);
      }
      else if (FD_ISSET(sock, &rdfs))
      {
         int n = read_server(sock, buffer);
         /* server down */
         if (n == 0)
         {
            printf("Server disconnected !\n");
            break;
         }
         if (isRegistered == 0)
         {
            int retour = sinscrire(buffer, name, sock);
            if (retour == 0)
            {
               printf("s'inscrire : %d", retour);
               exit(1);
            }
            else
            {
               isRegistered = 1;
            }
         }
         else
         {
            puts(buffer);
         }
      }
   }

   end_connection(sock);
}

static int init_connection(const char *address)
{
   SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
   SOCKADDR_IN sin = {0};
   struct hostent *hostinfo;

   if (sock == INVALID_SOCKET)
   {
      perror("socket()");
      exit(errno);
   }

   hostinfo = gethostbyname(address);
   if (hostinfo == NULL)
   {
      fprintf(stderr, "Unknown host %s.\n", address);
      exit(EXIT_FAILURE);
   }

   sin.sin_addr = *(IN_ADDR *)hostinfo->h_addr;
   sin.sin_port = htons(PORT);
   sin.sin_family = AF_INET;

   if (connect(sock, (SOCKADDR *)&sin, sizeof(SOCKADDR)) == SOCKET_ERROR)
   {
      perror("connect()");
      exit(errno);
   }

   return sock;
}

static void end_connection(int sock)
{
   closesocket(sock);
}

static int read_server(SOCKET sock, char *buffer)
{
   int n = 0;

   if ((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
   {
      perror("recv()");
      exit(errno);
   }

   buffer[n] = 0;

   return n;
}

int sinscrire(char *received_data, const char *username, SOCKET sock)
{
   // regarde la valeur de retour
   // si retour == 1, ok = 1 -> utilisateur crée
   // si retour == 2, welcome back -> utilisateur déjà crée
   // si retour == 0, erreur -> utilisateur déjà connecté
   if (strstr(received_data, "Inscription") != NULL)
   {
      printf(BOLD "Bienvenue %s, nous venons de creer votre utilisateur\r\n\r\n" RESET, username);
      write_server(sock, "OK");
      return 1;
   }
   else if (strstr(received_data, "WelcomeBack") != NULL)
   {
      printf(BOLD "Welcome back %s\r\n\r\n" RESET, username);
      write_server(sock, "OK");
      return 1;
   }
   else
   {
      printf("%s", received_data);
      printf(BOLD "Cet utilisateur est" ROUGE " déjà connecté\r\n" RESET);
      return 0;
   }
}

static void write_server(SOCKET sock, const char *buffer)
{
   if (send(sock, buffer, strlen(buffer), 0) < 0)
   {
      perror("send()");
      exit(errno);
   }
}

int main(int argc, char **argv)
{
   if (argc < 2)
   {
      printf("Usage : %s [address] [pseudo]\n", argv[0]);
      return EXIT_FAILURE;
   }

   init();

   app(argv[1], argv[2]);

   end();

   return EXIT_SUCCESS;
}
