#include "HTTP_Server.h"
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include "Routes.h"
#include "Response.h"

#define MAX_CONNECTIONS 10 // Batas koneksi maksimum
#define BACKLOG 5          // Jumlah koneksi dalam antrean

struct Route *route;

// Fungsi untuk inisialisasi semaphore
int initialize_semaphore() {
    key_t key = ftok("semfile", 65);
    int sem_id = semget(key, 1, 0666 | IPC_CREAT);
    semctl(sem_id, 0, SETVAL, MAX_CONNECTIONS);
    return sem_id;
}

// Fungsi untuk menunggu semaphore
void wait_semaphore(int sem_id) {
    struct sembuf sb = {0, -1, 0};
    semop(sem_id, &sb, 1);
}

// Fungsi untuk melepaskan semaphore
void signal_semaphore(int sem_id) {
    struct sembuf sb = {0, 1, 0};
    semop(sem_id, &sb, 1);
}

// Fungsi utama untuk inisialisasi server
void init_server(HTTP_Server *http_server, int port) {
    http_server->port = port;

    // Buat socket server
    int server_socket = create_server_socket(port);

    // Bind server socket ke port
    bind_server_socket(server_socket, port);

    // Mulai listening
    start_listening(server_socket);

    // Tangani zombie processes
    handle_zombie_processes();

    // Inisialisasi semaphore
    int sem_id = initialize_semaphore();

    // Simpan server socket ke struct
    http_server->socket = server_socket;

    printf("HTTP Server Initialized\nPort: %d\n", http_server->port);

    // Loop untuk menerima koneksi
    accept_connections(server_socket, sem_id);

    // Cleanup resources
    cleanup_semaphore(sem_id);
}

// Membuat server socket
int create_server_socket(int port) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    return server_socket;
}

// Melakukan binding socket ke alamat dan port
void bind_server_socket(int server_socket, int port) {
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }
}

// Memulai proses listening
void start_listening(int server_socket) {
    if (listen(server_socket, BACKLOG) == -1) {
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }
}

// Menangani zombie process dengan signal
void handle_zombie_processes() {
    signal(SIGCHLD, SIG_IGN);
}

// Fungsi untuk menerima koneksi klien
void accept_connections(int server_socket, int sem_id) {
    while (1) {
        wait_semaphore(sem_id);
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0) {
            perror("Failed to accept connection");
            signal_semaphore(sem_id);
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("Failed to fork");
            close(client_socket);
            signal_semaphore(sem_id);
            continue;
        }

        if (pid == 0) {
            // Proses anak
            close(server_socket);
            handle_client(client_socket);
            close(client_socket);
            signal_semaphore(sem_id);
            exit(0);
        } else {
            // Proses induk
            close(client_socket);
        }
    }
}

// Fungsi untuk menangani klien
void handle_client(int client_socket) {
    char client_msg[4096] = "";
    int bytes_read = read(client_socket, client_msg, 4095);
    if (bytes_read < 0) {
        perror("Failed to read from client");
        return;
    }
    client_msg[bytes_read] = '\0'; // Null-terminate the string
    printf("Received message: %s\n", client_msg);

    // Echo the received message back to the client
    int bytes_sent = send(client_socket, client_msg, bytes_read, 0);
    if (bytes_sent < 0) {
        perror("Failed to send to client");
    }

    // parsing client socket header to get HTTP method, route
    char *method = "";
    char *urlRoute = "";

    char *client_http_header = strtok(client_msg, "\n");
        
    printf("\n\n%s\n\n", client_http_header);

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
        }
        header_token = strtok(NULL, " ");
        header_parse_counter++;
    }

    printf("The method is %s\n", method);
    printf("The route is %s\n", urlRoute);

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

    char http_header[4096] = "HTTP/1.1 200 OK\r\n\r\n";

    strcat(http_header, response_data);
    strcat(http_header, "\r\n\r\n");

    send(client_socket, http_header, sizeof(http_header), 0);
    close(client_socket);
    free(response_data);
}

void cleanup_semaphore(int sem_id) {
    semctl(sem_id, 0, IPC_RMID);
}