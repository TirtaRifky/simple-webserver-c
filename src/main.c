#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "HTTP_Server.h"
#include "Routes.h"

int main() {
    // initiate HTTP_Server
    HTTP_Server http_server;
    init_server(&http_server, 6969);

    int client_socket;
    
    // registering Routes
    struct Route *route = initRoute("/", "index.html"); 
    addRoute(route, "/about", "about.html");

    printf("\n====================================\n");
    printf("=========ALL AVAILABLE ROUTES========\n");
    // display all available routes
    inorder(route);

    while (1) {
        client_socket = accept(http_server.socket, NULL, NULL);
        if (client_socket < 0) {
            perror("accept");
            continue;
        }

        // Create a child process to handle the client
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            close(http_server.socket);
            handle_client(client_socket, route);
            exit(0);
        } else if (pid > 0) {
            // Parent process
            close(client_socket);
        } else {
            perror("fork");
        }
    }

    return 0;
}