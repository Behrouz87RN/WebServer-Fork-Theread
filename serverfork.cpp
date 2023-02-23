#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <arpa/inet.h>
#define BUFF_SIZE (1024)

void error_msg(const char *msg, bool halt_flag)
{
  perror(msg);
  if (halt_flag)
    exit(-1);
}

void connectionC(int client)
{
  char recv_buf[1024];
  int temp;

  struct tm *ptr;
  time_t local;
  char times[30];

  FILE *html_file;
  FILE *output_file;
  char *send_buf;
  char file_name[100];
  char *file_data;
  char errormsg[69];
  char output[100];
  int outputsize;
  int recv_len;
  int file_size;
  recv_len = 0;
  file_size = 0;

  bzero(recv_buf, sizeof(recv_buf));
  temp = recv(client, recv_buf, sizeof(recv_buf), 0);

  temp = temp / sizeof(char);
  recv_buf[temp - 2] = '\0';
  // printf("%s \n", recv_buf);

  if ((strncmp(recv_buf, "GET ", 4)) != 0)
  { // check to make sure that a GET request was made
    strcpy(errormsg, "Invalid Command Entered\nPlease Use The Format: GET <file_name.html>\n");
    send(client, errormsg, 69, 0);
  }

  else
  {
    recv_len = strlen(recv_buf);
    int i;
    int j = 0; // remove the forward slash if neccissary
    for (i = 4; i < recv_len; i++, j++)
    {
      if ((recv_buf[i] == '\0') || (recv_buf[i] == '\n') || (recv_buf[i] == ' '))
      { // If the end of the file path is reached, break.
        break;
      }
      else if (recv_buf[i] == '/')
      { // do nothing except increment i to jump the forward slash
        --j;
      }
      else
      {
        file_name[j] = recv_buf[i];
      }
    }
    file_name[j] = '\0';
    // printf("%s", file_name);

    html_file = fopen(file_name, "r");
    if (html_file == NULL)
    { // check to make sure fopen worked, if there was no file, return error
      send_buf = (char *)malloc(24);
      strcpy(send_buf, "HTTP/1.1 404 Not Found\n\n");
      send(client, send_buf, 24, 0);
    }
    else
    {
      time(&local); // Get the current time
      ptr = localtime(&local);
      strftime(times, 30, "%a, %d %b %Y %X %Z", ptr);

      fseek(html_file, 0, SEEK_END);
      file_size = ftell(html_file); // get the byte offset of the pointer(the size of the file)
      fseek(html_file, 0, SEEK_SET);

      // printf("file_size: %d\n",file_size);
      char headString[200] = {0};
      sprintf(headString, "HTTP/1.1 200 OK\nDate: %s\nContent Length: %d\nConnection: close\nContent-Type: text/html\n\n", times, file_size); // format and create string for output to client

      char send_buf_temp[strlen(headString) + file_size];
      strcpy(send_buf_temp, headString);

      char *linebuffer = NULL;
      size_t len = 0;
      ssize_t characters;

      while ((characters = getline(&linebuffer, &len, html_file)) != -1)
      {
        // printf("getLine: %s\n", linebuffer);
        strcat(send_buf_temp, linebuffer);
      }
      free(linebuffer);

      // printf("last char: %c ...\n", send_buf_temp[file_size-1]);
      if (file_size > 0 && send_buf_temp[file_size - 1] == '\n')
      {
        // printf("last char is new line");
        send_buf_temp[file_size - 1] = '\0';
      }

      int a = strlen(send_buf_temp);
      // printf("a: %d\n",a);																														
      send_buf = (char *)malloc(a);
      strcpy(send_buf, send_buf_temp);
      // printf("%s\n",file_data);																									
      int resend = send(client, send_buf, a, 0);
      // if (resend > 0 ){printf("Data sent\n");}	
      free(file_data);
      // printf("free data\n");	
    }
    free(send_buf);
    fclose(html_file);
  }

  close(client);
  // printf("closed connection\n");
  fflush(stdout);
  // printf("fflush\n");
  // printf("DONE\n");
}

// server socket
int create_server_socket(bool non_blocking, char *destHost, char *destPort)
{
  const int port = atoi(destPort);

  struct addrinfo hints, *destAddInfo = NULL;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC; // Allow IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_protocol = 0;
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;

  // Get the address information
  int rc = getaddrinfo(destHost, destPort, &hints, &destAddInfo);
  if (rc != 0)
  {
    printf("Host not found --> %s\n", gai_strerror(rc));
    if (rc == EAI_SYSTEM)
      perror("getaddrinfo() failed");
    return -1;
  }

  // socket for the server.
  int sock = socket(destAddInfo->ai_family, destAddInfo->ai_socktype, destAddInfo->ai_protocol);
  if (sock < 0)
    error_msg("Problem with socket call", true);

  if (non_blocking)
    fcntl(sock, F_SETFL, O_NONBLOCK);

  if (bind(sock, destAddInfo->ai_addr, destAddInfo->ai_addrlen) < 0)
    error_msg("Problem with bind call", true);

  // Listen
  fprintf(stderr, "Listening for requests on port %i...\n", port);
  if (listen(sock, 100) < 0)
    error_msg("Problem with listen call", true);

  return sock;
}

void announce_client(struct in_addr *addr)
{
  char buffer[BUFF_SIZE + 1];

  inet_ntop(AF_INET, addr, buffer, sizeof(buffer));
  // fprintf(stderr, "Client connected from %s...\n", buffer);
}

void generate_echo_response(char request[], char response[])
{
  bzero(response, sizeof(response));
  strcpy(response, "HTTP/1.1 200 OK\n");
  strcat(response, "Content-Type: text/*\n");
  strcat(response, "Accept-Ranges: bytes\n");
  strcat(response, "Connection: close\n\n");
  strcat(response, request);
}

int main(int argc, char *argv[])
{
  char delim[] = ":";
  char *Desthost = strtok(argv[1], delim);
  char *Destport = strtok(NULL, delim);

  // Avoid zombies
  signal(SIGCHLD, SIG_IGN);

  char buffer[BUFF_SIZE + 1];

  struct sockaddr_in client_addr;
  socklen_t len = sizeof(struct sockaddr_in);

  // server socket connections
  int sock = create_server_socket(false, Desthost, Destport);

  while (true)
  {
    int client = accept(sock,
                        (struct sockaddr *)&client_addr,
                        &len);
    if (client < 0)
      error_msg("Problem with accept call", true);

    announce_client(&client_addr.sin_addr);

    // Fork a process to handle client and resume awaiting new requests.
    pid_t pid = fork();
    if (pid < 0)
      error_msg("Problem with fork call", false);

    // If fork succeeds, it returns 0 to the child
    if (0 == pid)
    {
      // Close the child's inherited copy of the listening socket
      close(sock);

      connectionC(client);

      close(client);
      exit(0); // terminate child process
    }
    else // parent rather than child
      close(client);
  }

  return 0;
}
