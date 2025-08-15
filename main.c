// minimal TCP http server
// gcc -O2 -Wall -Wextra -std=c11 main.c -o main
// ./main

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

#include "src/config.h"
#include "src/http.c"

int main(int argc, char** argv) {
    printf("-----p0syd0n's HTTP server-----\n\n");
    printf("Command line parameters enabled?: %d\n", USEPARAMS);
    if (USEPARAMS == 0) {
        printf("!!        This is a test build        !!\n");
        printf("!! Using default ( compiled ) options !!\n");
        printf("Routes: %d\n", ROUTES);
        printf("List Files: %d\n", LISTFILES);
    } else if (USEPARAMS == 1 && argc != 3) {
        printf("Usage: %s <routes> <listfiles>\n\n", argv[0]);
        printf("<routes> : Whether the server should use pre-compiled routes instead of just serving files\n");
        printf("<listfiles> : Whether the server should display a list of files if no filename is provided in the request\n");
        exit(0);
    } else if (USEPARAMS == 1 && argc == 3) {
        printf("Routes: %d\n", *argv[1] - '0');
        printf("List Files: %d\n", *argv[2] - '0');
    }
    printf("\n-------------------------------\n\n\n\n\n");
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0) {
      perror("socket");
      exit(1);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        exit(1);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        close(server_fd);
        exit(1);
    }

    printf("Listening on port %d...\n", PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        printf("Client connected\n");

        // Placeholder read/write
        char buf[REQUEST_MAX_SIZE];
        ssize_t n = read(client_fd, buf, sizeof(buf) - 1);

        if (n > 0) {
            buf[n] = '\0';
            handle_request(client_fd, buf, argv[1], argv[2]);
            close(client_fd);
        }

    }

    close(server_fd);
    return 0;
}
