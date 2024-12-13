#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

typedef struct HTTP_Server {
	int socket;
	int port;	
} HTTP_Server;

void init_server(HTTP_Server* http_server, int port);
void handle_client_connection(int client_socket, struct Route *route);
void handle_client(int client_socket);
void init_server(HTTP_Server *http_server, int port);
int create_server_socket(int port);
void bind_server_socket(int server_socket, int port);
void start_listening(int server_socket);
void handle_zombie_processes();
int *initialize_shared_memory();
void accept_connections(int server_socket, int *active_connections);
void cleanup_shared_memory(int *active_connections, int shm_id);
int initialize_semaphore()
void cleanup_semaphore(int sem_id);

#endif
