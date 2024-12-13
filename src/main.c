#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <fcntl.h>
#include "HTTP_Server.h"
#include "Routes.h"

int main() {
    // Inisialisasi HTTP_Server
    HTTP_Server http_server;
    init_server(&http_server, 6969);

    int client_socket;

    // Konfigurasi penanganan sinyal untuk zombie process
    configure_signal_handling();

    // Registrasi Routes
    struct Route *route = initRoute("/", "index.html");
    addRoute(route, "/about", "about.html");

    printf("\n====================================\n");
    printf("========= ALL AVAILABLE ROUTES ========\n");
    // Tampilkan semua route yang tersedia
    inorder(route);

    // Inisialisasi semaphore
    sem_t *sem;
    init_semaphore(&sem, "/client_semaphore", 10);

    while (1) {
        // Tunggu semaphore
        wait_semaphore(sem);

        // Terima koneksi klien
        client_socket = accept(http_server.socket, NULL, NULL);
        if (client_socket < 0) {
            perror("accept");
            post_semaphore(sem); // Lepaskan semaphore jika gagal
            continue;
        }

        // Fork proses anak untuk menangani klien
        pid_t pid = fork();
        if (pid == 0) {
            // Proses anak
            close(http_server.socket);
            handle_client(client_socket, route);
            close(client_socket);
            post_semaphore(sem); // Lepaskan semaphore setelah selesai
            exit(0);
        } else if (pid > 0) {
            // Proses induk
            close(client_socket);
        } else {
            perror("fork");
            post_semaphore(sem); // Lepaskan semaphore jika fork gagal
        }

        // Proses anak yang selesai akan ditangani oleh `SIGCHLD` handler
    }

    // Tutup semaphore
    close_semaphore(sem, "/client_semaphore");

    return 0;
}
