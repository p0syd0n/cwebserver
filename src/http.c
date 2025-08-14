#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "config.h"


int read_file(char* filename, char* filecontents) {
  char* filepath = malloc(strlen(filename)+strlen(HTMLPATH) + 1); // null terminator
  if (filepath == NULL)  { perror("read_file Error allocating memory"); exit(-1); } 
  strcpy(filepath, HTMLPATH);
  strcat(filepath, filename);
  
  FILE* thefile = fopen(filepath, "r");
  if (thefile == NULL)  { perror("read_file Error opening file"); exit(-1); } 

  size_t one_grab_size = 1024;
  char* filebuffer = calloc(MAXFILECONTENTS, 1);
  char* tempbuffer = malloc(one_grab_size);
  while (fgets(tempbuffer, one_grab_size, thefile) != NULL) {
    strncat(filebuffer, tempbuffer, one_grab_size);
  }
  strcpy(filecontents, filebuffer);
  return 1;
}

int build_response(char* filename, char* fullresponse) {
  char* filecontents = malloc(MAXFILECONTENTS);
  if (filecontents == NULL) { perror("build_response Error allocating memory"); exit(-1); }
  read_file(filename, filecontents);
  char *responseheaders =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n"
    "\r\n";
  strcpy(fullresponse, responseheaders);
  strcat(fullresponse, filecontents);
  return 1;
}

int handle_request(int client_fd, char* buffer) {
    char* path = malloc(REQUEST_MAX_SIZE);
    if (path == NULL) { perror("handle_request Error allocating memory"); exit(-1); }
    char* needle = "/";
    path = strstr(buffer, needle);
    path = strtok(path, " ");
    printf("Path requested: %s\n", path);
    char* fullresponse = malloc(MAXHEADERSIZE+MAXFILECONTENTS+1); // for null terminator but if we're flush with the limit we got other issues than null termination
    if (fullresponse == NULL) { perror("handle_request Error allocating memory"); exit(-1); }

    char* filename = malloc(MAXFILENAME);
    if (filename == NULL) { perror("handle_request Error allocating memory"); exit(-1); }

    if (strcmp(path, "/") == 0) {
      strcpy(filename, "main.html");
    } else if (strcmp(path, "/error") == 0) {
      strcpy(filename, "error.html");
    } else {
      strcpy(filename, "404.html");
    }
    
    printf("Serving file: %s\n", filename);
    build_response(filename, fullresponse);
    int bytes = send(client_fd, fullresponse, strlen(fullresponse), 0);
    printf("Sending: %d bytes\n\n", bytes);
    return bytes;
}

