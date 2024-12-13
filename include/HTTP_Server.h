#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <netinet/in.h>
#include "Routes.h"

typedef struct {
    int socket;
    struct sockaddr_in server_addr;
} HTTP_Server;

void init_server(HTTP_Server *server, int port);
void handle_client(int client_socket, struct Route *route);

#endif // HTTP_SERVER_H