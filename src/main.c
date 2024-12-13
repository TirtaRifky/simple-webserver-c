#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/wait.h> // Tambahkan header ini
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

    // Create semaphore
    sem_t *sem;
    init_semaphore(&sem, "/client_semaphore", 10);

    while (1) {
        // Wait for semaphore
        wait_semaphore(sem);

        client_socket = accept(http_server.socket, NULL, NULL);
        if (client_socket < 0) {
            perror("accept");
            post_semaphore(sem); // Release semaphore if accept fails
            continue;
        }

        // Create a child process to handle the client
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            close(http_server.socket);
            handle_client(client_socket, route);
            close(client_socket);
            post_semaphore(sem); // Release semaphore when done
            exit(0);
        } else if (pid > 0) {
            // Parent process
            close(client_socket);
        } else {
            perror("fork");
            post_semaphore(sem); // Release semaphore if fork fails
        }

        // Clean up finished child processes
        while (waitpid(-1, NULL, WNOHANG) > 0);
    }

    // Close semaphore
    close_semaphore(sem, "/client_semaphore");

    return 0;
}