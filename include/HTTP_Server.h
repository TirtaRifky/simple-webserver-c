#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

typedef struct HTTP_Server {
    int socket;
    int port;    
} HTTP_Server;

extern struct Route *route;

void init_server(HTTP_Server* http_server, int port);
int create_server_socket(int port);
void bind_server_socket(int server_socket, int port);
void start_listening(int server_socket);
void handle_zombie_processes();
void accept_connections(int server_socket, int sem_id);
void handle_client(int client_socket);
int initialize_semaphore();
void wait_semaphore(int sem_id);
void signal_semaphore(int sem_id);
void cleanup_semaphore(int sem_id);

#endif