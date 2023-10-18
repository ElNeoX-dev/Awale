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
#include <netdb.h>

#define MAX_BUFFER_SIZE 1024

int main(int argc, char **argv)
{
  int sockfd, newsockfd, clilen, chilpid, ok, nleft, nbwriten;
  char c;
  unsigned long int hour;
  struct sockaddr_in cli_addr, serv_addr;
  char buffer[MAX_BUFFER_SIZE];
  char http_request[] = "GET / HTTP/1.0\r\n\r\n";

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

  /* repete dans le socket tout ce qu'il entend */

  /*   while(1)
    {
      read(sockfd, &hour, sizeof(unsigned long int));
      time_t hourConv = ntohl(hour);
      printf("hour %zu", hourConv);
      break;
    } */

  /*   while (1) {c=getchar();write (sockfd,&c,1);
      if(c == EOF)
      {
          printf("EOF\n");
          close(sockfd);
          return 0;
      }

    } */

  write(sockfd, http_request, strlen(http_request));

  while (read(sockfd, buffer, MAX_BUFFER_SIZE - 1) > 0)
  {
    printf("%s", buffer);
  }
  close(sockfd);
  exit(0);

  return 1;
}
