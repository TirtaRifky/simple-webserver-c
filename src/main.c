#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "HTTP_Server.h"
#include "Routes.h"
#include "Response.h"

extern struct Route *route;

int main() {
    // initiate HTTP_Server
    HTTP_Server http_server;
    init_server(&http_server, 6969);

    // registering Routes
    route = initRoute("/", "index.html");
    addRoute(route, "/about", "about.html");

    printf("\n====================================\n");
    printf("=========ALL AVAILABLE ROUTES========\n");
    // display all available routes
    inorder(route);

    // Server loop handled in init_server
    return 0;
}