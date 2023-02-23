#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <pthread.h>
#include <arpa/inet.h>
#define BUFF_SIZE (1024)


void error_msg(const char *msg, bool halt_flag)
{
  perror(msg);
  if (halt_flag)
    exit(-1); // -1 for abnormal termination 
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
  // if (temp < 0)
  // { //check for
  // }

  temp = temp / sizeof(char);
  recv_buf[temp - 2] = '\0';
  // printf("%s \n", recv_buf);

  if ((strncmp(recv_buf, "GET ", 4)) != 0)
  { // check to make sure that a GET request was made
    strcpy(errormsg, "Invalid Command Entered\nPlease Use The Format: GET <file_name.html>\n");
    send(client, errormsg, 69, 0);
  }
  // Other wise the name of the file is copied character by charcter.
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
        file_name[j] = recv_buf[i]; // copy the file name character by character
      }
    }
    file_name[j] = '\0'; // add a null terminator to the end of the file name string
    // printf("%s", file_name);

    html_file = fopen(file_name, "r"); // open the html file
    if (html_file == NULL)
    { // check to make sure fopen worked(if there was a file) if there was no file, return 404 message
      send_buf = (char *)malloc(24);
      strcpy(send_buf, "HTTP/1.1 404 Not Found\n\n");
      send(client, send_buf, 24, 0);
    }
    else
    {               // If the file did open without errors
      time(&local); // Get the current time and date
      ptr = localtime(&local);
      strftime(times, 30, "%a, %d %b %Y %X %Z", ptr);

      fseek(html_file, 0, SEEK_END); 
      file_size = ftell(html_file);  // get the size of the file
      fseek(html_file, 0, SEEK_SET); 
      // printf("file_size: %d\n",file_size);
      char headString[200] = {0};
      sprintf(headString, "HTTP/1.1 200 OK\nDate: %s\nContent Length: %d\nConnection: close\nContent-Type: text/html\n\n", times, file_size); 

      char send_buf_temp[strlen(headString) + file_size];
      strcpy(send_buf_temp, headString);

      char *linebuffer;
      size_t len = 0;
      ssize_t characters;

      // if ((linebuffer = malloc(256)) != NULL) { len = 256;}

      while ((characters = getline(&linebuffer, &len, html_file)) != -1)
      {
        // printf("getLine: %s\n", linebuffer);
        strcat(send_buf_temp, linebuffer);
      }
      if (linebuffer != NULL)
      {
        free(linebuffer);
      }

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
  // printf("exiting thread, %d \n", client);
  pthread_exit(NULL);
  // printf("DONE\n");
}

// server socket
int create_server_socket(bool non_blocking, char *destHost, char *destPort)
{
  const int port = atoi(destPort);
  // structure for socket
  struct addrinfo hints, *destAddInfo = NULL;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_protocol = 0;
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;

  // Get the address information for the server using getaddrinfo().
  int rc = getaddrinfo(destHost, destPort, &hints, &destAddInfo);
  if (rc != 0)
  {
    printf("Host not found --> %s\n", gai_strerror(rc));
    if (rc == EAI_SYSTEM)
      perror("getaddrinfo() failed");
    return -1;
  }

  // socket
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

  //Dump client's IP (in dotted-decimal format) to stderr. 
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

void *handle_client(void *client_ptr)
{
  pthread_detach(pthread_self());

  int client = *((int *)client_ptr);

  connectionC(client);

  close(client);

  return NULL;
}

int main(int argc, char *argv[])
{
  char delim[] = ":";
  char *Desthost = strtok(argv[1], delim);
  char *Destport = strtok(NULL, delim);

  /* Buffer for I/O operations. */
  char buffer[BUFF_SIZE + 1];

  /* Arguments for the accept call. */
  struct sockaddr_in client_addr;
  socklen_t len = sizeof(struct sockaddr_in);

  // server socket to connection
  int sock = create_server_socket(false, Desthost, Destport);

  // Accept
  while (true)
  {
    // Get the client socket for reading/writing.
    int client = accept(sock, (struct sockaddr *)&client_addr, &len);
    if (client < 0)
      error_msg("Problem accepting a client request", true);

    announce_client(&client_addr.sin_addr);

    pthread_t tid;
    // printf("creating thread, %d \n", tid);
    pthread_create(&tid, NULL, handle_client, &client);
    pthread_join(tid, NULL);
  }

  close(sock);
  return 0;
}
