#include "HTTP_Server.h"
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_CONNECTIONS 10 // Batas koneksi maksimum
#define BACKLOG 5          // Jumlah koneksi dalam antrean


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

    // Inisialisasi shared memory
    int *active_connections = initialize_shared_memory();

    // Simpan server socket ke struct
    http_server->socket = server_socket;

    printf("HTTP Server Initialized\nPort: %d\n", http_server->port);

    // Loop untuk menerima koneksi
    accept_connections(server_socket, active_connections);

	cleanup_shared_memory(active_connections, shm_id);
    cleanup_semaphore(sem_id);
}

void cleanup_shared_memory(int *active_connections, int shm_id) {
    if (shmdt(active_connections) == -1) {
        perror("Failed to detach shared memory");
    }
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("Failed to remove shared memory");
    }
}

void cleanup_semaphore(int sem_id) {
    if (semctl(sem_id, 0, IPC_RMID) == -1) {
        perror("Failed to remove semaphore");
    }
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

// Inisialisasi shared memory untuk melacak koneksi aktif
int *initialize_shared_memory() {
    int shm_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("Failed to create shared memory");
        exit(EXIT_FAILURE);
    }
    int *active_connections = (int *)shmat(shm_id, NULL, 0);
    if (active_connections == (void *)-1) {
        perror("Failed to attach shared memory");
        exit(EXIT_FAILURE);
    }
    *active_connections = 0;
    return active_connections;
}

int initialize_semaphore() {
    int sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    if (sem_id == -1) {
        perror("Failed to create semaphore");
        exit(EXIT_FAILURE);
    }
    semctl(sem_id, 0, SETVAL, 1); // Inisialisasi nilai semaphore ke 1
    return sem_id;
}

void lock_semaphore(int sem_id) {
    struct sembuf sb = {0, -1, 0}; // Operasi P (wait)
    semop(sem_id, &sb, 1);
}

void unlock_semaphore(int sem_id) {
    struct sembuf sb = {0, 1, 0}; // Operasi V (signal)
    semop(sem_id, &sb, 1);
}

// Menerima koneksi dari klien
void accept_connections(int server_socket, int *active_connections, int shm_id, int sem_id) {
    while (1) {
        struct sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);

        int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_len);
        if (client_socket == -1) {
            perror("Connection acceptance failed");
            continue;
        }

        if (*active_connections >= MAX_CONNECTIONS) {
            printf("Server busy, rejecting connection.\n");
            close(client_socket);
            continue;
        }

        pid_t pid = fork();
        if (pid == 0) { // Proses anak
            lock_semaphore(sem_id);
            (*active_connections)++;
            unlock_semaphore(sem_id);

            close(server_socket);
            handle_client(client_socket);
            close(client_socket);

            lock_semaphore(sem_id);
            (*active_connections)--;
            unlock_semaphore(sem_id);

            exit(0);
        } else if (pid > 0) { // Proses induk
            close(client_socket);
        } else {
            perror("Fork failed");
        }
    }
}

// Fungsi untuk menangani klien
void handle_client(int client_socket) {
    char buffer[1024];
    recv(client_socket, buffer, sizeof(buffer), 0);
    printf("Client Request: %s\n", buffer);

    // Respons sederhana
    char response[] = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
    send(client_socket, response, sizeof(response), 0);
}

void handle_client_connection(int client_socket, struct Route *route) {
    char client_msg[4096] = "";

    read(client_socket, client_msg, 4095);
    printf("Client Request:\n%s\n", client_msg);

    char method[10] = {0};
    char urlRoute[100] = {0};
    char *client_http_header = strtok(client_msg, "\n");
    if (client_http_header == NULL) {
        perror("Failed to parse HTTP header");
        close(client_socket);
        return;
    }
    printf("\nHTTP Header: %s\n\n", client_http_header);

    char *header_token = strtok(client_http_header, " ");
    int header_parse_counter = 0;

    while (header_token != NULL) {
        if (header_parse_counter == 0) {
            strncpy(method, header_token, sizeof(method) - 1);
        } else if (header_parse_counter == 1) {
            strncpy(urlRoute, header_token, sizeof(urlRoute) - 1);
        }
        header_token = strtok(NULL, " ");
        header_parse_counter++;
    }

    printf("Method: %s\n", method);
    printf("Route: %s\n", urlRoute);

    char template[100] = {0};
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

    send(client_socket, http_header, strlen(http_header), 0);
    free(response_data);
}
