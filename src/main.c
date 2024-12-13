#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "HTTP_Server.h"
#include "Routes.h"
#include "Response.h"

// Prototipe fungsi
// void handle_client_connection(int client_socket, struct Route *route); // Tidak diperlukan lagi

int main() {
    // Inisialisasi HTTP_Server
    HTTP_Server http_server;
    init_server(&http_server, 6969);

    printf("HTTP Server is running on port: %d\n", http_server.port);

    // Registrasi Routes
    struct Route *route = initRoute("/", "index.html");
    addRoute(route, "/about", "about.html");

    // Server akan berjalan dan menangani koneksi di dalam init_server
    return 0;
}