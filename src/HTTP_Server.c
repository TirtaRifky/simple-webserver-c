#include "HTTP_Server.h"
#include "Routes.h"
#include "Response.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h> // Include fcntl.h for O_CREAT

void init_server(HTTP_Server *server, int port) {
    server->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server->socket == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    server->address.sin_family = AF_INET;
    server->address.sin_addr.s_addr = INADDR_ANY;
    server->address.sin_port = htons(port);

    if (bind(server->socket, (struct sockaddr *)&server->address, sizeof(server->address)) < 0) {
        perror("bind failed");
        close(server->socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server->socket, 3) < 0) {
        perror("listen");
        close(server->socket);
        exit(EXIT_FAILURE);
    }
}

void handle_client(int client_socket, struct Route *route) {
    char client_msg[4096] = "";

    read(client_socket, client_msg, 4095);

    // parsing client socket header to get HTTP method, route
    char *method = "";
    char *urlRoute = "";
    char *http_version = "";

    char *client_http_header = strtok(client_msg, "\n");

    char *header_token = strtok(client_http_header, " ");
    
    int header_parse_counter = 0;

    while (header_token != NULL) {
        switch (header_parse_counter) {
            case 0:
                method = header_token;
                break;
            case 1:
                urlRoute = header_token;
                break;
            case 2:
                http_version = header_token;
                break;
        }
        header_token = strtok(NULL, " ");
        header_parse_counter++;
    }

    printf("\nURL Route: %s\n", urlRoute);
    printf("Method : %s\n", method);
    printf("HTTP Version: %s\n", http_version);

    char template[100] = "";
    
    if (strstr(urlRoute, "/static/") != NULL) {
        strcat(template, "static/index.css");
    } else {
        struct Route *destination = search(route, urlRoute);
        strcat(template, "templates/");

        if (destination == NULL) {
            strcat(template, "404.html");
        } else {
            strcat(template, destination->value);
        }
    }

    char *response_data = render_static_file(template);

    // Ensure the response data is not NULL
    if (response_data == NULL) {
        response_data = "<html><body><h1>404 Not Found</h1></body></html>";
    }

    char http_header[4096];
    snprintf(http_header, sizeof(http_header), "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\nContent-Type: text/html\r\n\r\n%s", strlen(response_data), response_data);

    send(client_socket, http_header, strlen(http_header), 0);
    close(client_socket);
    free(response_data);
}

void init_semaphore(sem_t **sem, const char *name, unsigned int value) {
    *sem = sem_open(name, O_CREAT, 0644, value);
    if (*sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
}

void wait_semaphore(sem_t *sem) {
    sem_wait(sem);
}

void post_semaphore(sem_t *sem) {
    sem_post(sem);
}

void close_semaphore(sem_t *sem, const char *name) {
    sem_close(sem);
    sem_unlink(name);
}