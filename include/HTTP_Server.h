#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <netinet/in.h>
#include <semaphore.h>
#include "Routes.h" // Include Routes.h to ensure struct Route is declared

typedef struct {
    int socket;
    struct sockaddr_in address;
} HTTP_Server;

void init_server(HTTP_Server *server, int port);
void handle_client(int client_socket, struct Route *route);
void init_semaphore(sem_t **sem, const char *name, unsigned int value);
void wait_semaphore(sem_t *sem);
void post_semaphore(sem_t *sem);
void close_semaphore(sem_t *sem, const char *name);
void configure_signal_handling();
void sigchld_handler(int signo);

#endif // HTTP_SERVER_H