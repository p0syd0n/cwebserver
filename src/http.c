#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>
#include <dirent.h>
#include "config.h"

int send_error(int client_fd) {
  printf("----------\nSending Internal Server Error to client!!\n");
  char *response =
    "HTTP/1.1 500 Internal Server Error\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n"
    "\r\n"
    "505 Internal Server Error\r\n";

    int bytes_sent = send(client_fd, response, strlen(response), 0);
    printf("Sent %d bytes. \n----------\n", bytes_sent);
    close(client_fd);
    return -1;
}

int filelist(int client_fd) {
    char *response_headers =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n"
    "\r\n";

    char* response_final = (char*)calloc(MAXHEADERSIZE+29+strlen(HTMLPATH)+1, 1); // Account for the headers, HTMLPATH, and the html wrappings for the HTMLPATH (29) (but this one will probably not matter - MAXHEADERSIZE is pretty big), and for the null terminator
    strcpy(response_final, response_headers);
    strcat(response_final, "<h1>Contents of ");
    strcat(response_final, HTMLPATH);
    strcat(response_final, "</h1><br><hr>");
    DIR *dir = opendir(HTMLPATH);
    if (!dir) { perror("filelist Error opening directory"); return 1; }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        size_t memory_needed = strlen(response_final) + strlen(entry->d_name)*2 + 20 + 1; // Account for the length of what we have so far, for two of the filenames (one for link, one for label), bytes for html (20), and for null terminator.
        response_final = realloc(response_final, memory_needed);

        strcat(response_final, "<a href=\"/");
        strcat(response_final, entry->d_name);
        strcat(response_final, "\">");
        strcat(response_final, entry->d_name);
        strcat(response_final, "</a>");
        strcat(response_final, "<br>");
        // printf("%s\n", entry->d_name);
    }

    closedir(dir);
    size_t bytes_to_be_sent = strlen(response_final);
    size_t bytes_sent = 0;
    while (bytes_sent  < bytes_to_be_sent) {
      bytes_sent += send(client_fd, response_final+bytes_sent, bytes_to_be_sent-bytes_sent, 0);
    }
    close(client_fd);
    
    return 0;
}

int read_file(char* filename, char* filecontents) {
  char* filepath = malloc(strlen(filename)+strlen(HTMLPATH) + 1); // null terminator
  if (filepath == NULL)  { perror("read_file Error allocating memory"); return -1; } 
  strcpy(filepath, HTMLPATH);
  strcat(filepath, filename);
  printf("Opening filepath %s\n", filepath);
  FILE* thefile = fopen(filepath, "r");
  if (thefile == NULL)  {
    if (errno == ENOENT) {
      perror("File not found");
      strcpy(filepath, HTMLPATH);
      strcat(filepath, "404.html");
      thefile = fopen(filepath, "r");
      if (thefile == NULL) { perror("read_file Error opening 404 file for 404 error"); return -1; }
    } else {
      perror("read_file Error opening file");
      exit(-1);
    }
  } 
  free(filepath);
  int bytes_read = fread(filecontents, 1, MAXFILECONTENTS, thefile);
  if (bytes_read < 0) {
    if (ferror(thefile)) {
      perror("read_file Error reading file");
      return -1;
    }
    perror("Somehow, we read negative bytes without having the stream error. Weird");
    return -1;
  }

  fclose(thefile);
  return bytes_read;
}

int build_response(char* filename, char* fullresponse) {
  
  char* filecontents = calloc(1, MAXFILECONTENTS);
  if (filecontents == NULL) { perror("build_response Error allocating memory"); return -1; }
  int bytes_read = read_file(filename, filecontents);
  if (bytes_read < 0) {
    perror("build_response Error recieved from read_file");
    return -1;
  }
  printf("%d bytes read\n", bytes_read);

  char *responseheaders =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n"
    "\r\n";
  strcpy(fullresponse, responseheaders);
  strcat(fullresponse, filecontents);
  free(filecontents);
  return strlen(fullresponse);
}

int handle_request(int client_fd, char* buffer, char* routes_, char* listfiles_) {
  int routes = ROUTES;
  int listfiles = LISTFILES;
  if (USEPARAMS) {
    routes = *routes_ - '0';
    listfiles = *listfiles_ - '0';
  }

  char* path = malloc(REQUEST_MAX_SIZE);
  char* path_ptr = path;
  if (path == NULL) { perror("handle_request Error allocating memory"); return send_error(client_fd); }
  char* needle = "/";
  path = strstr(buffer, needle);
  path = strtok(path, " ");
  printf("Path requested: %s\n", path);
  char* fullresponse = malloc(MAXHEADERSIZE+MAXFILECONTENTS+1); // for null terminator but if we're flush with the limit we got other issues than null termination
  if (fullresponse == NULL) { perror("handle_request Error allocating memory"); return send_error(client_fd); }

  char* filename = malloc(MAXFILENAME);
  if (filename == NULL) { perror("handle_request Error allocating memory"); return send_error(client_fd); }
  if (routes) {
    if (strcmp(path, "/") == 0) {
      strcpy(filename, "main.html");
    }
    // More routes go here
    else {
      strcpy(filename, "404.html");
    }
  } else {
    if (strcmp(path+1, "") == 0) {
      if (listfiles) {
        return filelist(client_fd);
      } else {
        strcpy(filename, "404.html");
      }
    } else {
      strcpy(filename, path+1);
    }
  }

  free(path_ptr);

  printf("Serving file: %s\n", filename);
  int bytes_in_response = build_response(filename, fullresponse);
  if (bytes_in_response < 0) { perror("handle_request Error recieved from build_response"); return send_error(client_fd); }
  free(filename);
  int bytes_sent = send(client_fd, fullresponse, strlen(fullresponse), 0);
  free(fullresponse);
  printf("Sent %d/%d bytes\n\n", bytes_sent, bytes_in_response);
  return bytes_sent;
}

