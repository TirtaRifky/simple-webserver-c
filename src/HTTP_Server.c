#include "HTTP_Server.h"
#include "Response.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

// Inisialisasi server
void init_server(HTTP_Server *server, int port)
{
    server->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server->socket == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    server->address.sin_family = AF_INET;
    server->address.sin_addr.s_addr = INADDR_ANY;
    server->address.sin_port = htons(port);

    if (bind(server->socket, (struct sockaddr *)&server->address, sizeof(server->address)) < 0)
    {
        perror("bind failed");
        close(server->socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server->socket, 3) < 0)
    {
        perror("listen");
        close(server->socket);
        exit(EXIT_FAILURE);
    }
}

// Penanganan client
void handle_client(int client_socket, struct Route *route)
{
    char client_msg[4096] = "";
    char response[4096] = "";

    read(client_socket, client_msg, 4095);
    strcpy(response, client_msg);

    // Parsing HTTP request
    char *method = "";
    char *urlRoute = "";
    char *http_version = "";

    char *client_http_header = strtok(client_msg, "\n");
    char *header_token = strtok(client_http_header, " ");
    int header_parse_counter = 0;

    while (header_token != NULL)
    {
        switch (header_parse_counter)
        {
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

    if (strstr(urlRoute, "/static/") != NULL)
    {
        strcat(template, "static/index.css");
    }
    else
    {
        struct Route *destination = search(route, urlRoute);
        strcat(template, "templates/");

        if (destination == NULL)
        {
            strcat(template, "404.html");
        }
        else
        {
            strcat(template, destination->value);
        }
    }

    char *response_data = render_static_file(template);

    // Pastikan respons tidak NULL
    if (response_data == NULL)
    {
        response_data = "<html><body><h1>404 Not Found</h1></body></html>";
    }

    if (strcmp(method, "PUT") == 0)
    {
        // Extract the body of the PUT request
        char *body = strstr(response, "\r\n\r\n");
        if (body != NULL)
        {
            body += 4; // Skip the "\r\n\r\n"

            // Extract the file name from the URL
            char *file_name = urlRoute + 1; // Skip the leading '/'

            // Create the full path by prepending "/file/"
            char full_path[4096];
            snprintf(full_path, sizeof(full_path), "file/%s", file_name);

            // Open the file for writing
            FILE *file = fopen(full_path, "w");
            if (file != NULL)
            {
                // Write the body to the file
                fprintf(file, "%s", body);
                fclose(file);

                // Log the PUT data to the server console
                printf("Data : %s\n\n", body);

                // Send a success response to the client
                char http_header[4096];
                snprintf(http_header, sizeof(http_header), "HTTP/1.1 200 OK\r\n\r\nData : %s\n", body);
                send(client_socket, http_header, strlen(http_header), 0);
            }
            else
            {
                // Handle case where the file could not be opened
                char http_header[4096];
                snprintf(http_header, sizeof(http_header), "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
                send(client_socket, http_header, strlen(http_header), 0);
            }
        }
        else
        {
            // Handle case where body is not found
            char http_header[4096];
            snprintf(http_header, sizeof(http_header), "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
            send(client_socket, http_header, strlen(http_header), 0);
        }
    }
    else if (strcmp(method, "POST") == 0 && strcmp(urlRoute, "/echo") == 0)
    {
        // Extract the body of the POST request
        char *body = strstr(response, "\r\n\r\n");
        if (body != NULL)
        {
            body += 4; // Skip the "\r\n\r\n"
            printf("POST body from client: %s\n", body);

            // Echo the message back to the client with a newline
            char http_header[4096];
            snprintf(http_header, sizeof(http_header), "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\n\r\n%s\n\n", strlen(body) + 1, body);
            send(client_socket, http_header, strlen(http_header), 0);
        }
        else
        {
            // Handle case where body is not found
            char http_header[4096];
            snprintf(http_header, sizeof(http_header), "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
            send(client_socket, http_header, strlen(http_header), 0);
        }
    }
    else if (strcmp(method, "DELETE") == 0) {
        // Attempt to delete the specified file
        char *file_name = urlRoute + 1; // Skip the leading '/'

        // Create the full path by prepending "/file/"
        char full_path[4096];
        snprintf(full_path, sizeof(full_path), "file/%s", file_name);
    
        if (remove(full_path) == 0) {
            // Log the DELETE action to the server console
            printf("DELETE request for %s succeeded\n", file_name);

            // Send response to client
            char http_header[4096];
            snprintf(http_header, sizeof(http_header), "HTTP/1.1 200 OK\r\n\r\nDELETE request for %s succeeded\n", file_name);
            send(client_socket, http_header, strlen(http_header), 0);
        } else {
            // Handle case where file cannot be deleted
            char http_header[4096];
            snprintf(http_header, sizeof(http_header), "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
            send(client_socket, http_header, strlen(http_header), 0);
        }
    }
    else
    {
        char http_header[4096];
        snprintf(http_header, sizeof(http_header), "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\nContent-Type: text/html\r\n\r\n%s", strlen(response_data), response_data);
        send(client_socket, http_header, strlen(http_header), 0);
    }

    close(client_socket);
    free(response_data);
}

// Konfigurasi semaphore
void init_semaphore(sem_t **sem, const char *name, unsigned int value)
{
    *sem = sem_open(name, O_CREAT, 0644, value);
    if (*sem == SEM_FAILED)
    {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
}

void wait_semaphore(sem_t *sem)
{
    sem_wait(sem);
}

void post_semaphore(sem_t *sem)
{
    sem_post(sem);
}

void close_semaphore(sem_t *sem, const char *name)
{
    sem_close(sem);
    sem_unlink(name);
}

// Penanganan zombie process
void sigchld_handler(int signo)
{
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
}

// Konfigurasi sinyal
void configure_signal_handling()
{
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;

    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}
