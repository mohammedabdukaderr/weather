
#ifndef TCP_H
#define TCP_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stddef.h>

// TCP_SERVER struktur med alla inställningar
typedef struct {
    int socket_fd;                // Filbeskrivare för socket
    struct sockaddr_in address;   // Serveradress
    uint16_t port;                // Portnummer
    int max_clients;              // Max antal samtidiga klienter
    int is_running;               // Flagga för körning
    char* bind_address;           // Pekare till bind-adress (valfritt)
} TCP_Server;

// TCP_CLIENT struktur för klientanslutningar
typedef struct {
    int socket_fd;                // Filbeskrivare
    struct sockaddr_in address;   // Klientadress
    char ip[INET_ADDRSTRLEN];     // IP som sträng
    uint16_t port;                // Klientens port
} TCP_Client;

// Serverfunktioner
TCP_Server* tcp_create_server(uint16_t port, int max_clients);
int tcp_start_server(TCP_Server* server);
int tcp_set_timeout(TCP_Server* server, int seconds);
TCP_Client* tcp_accept_client(TCP_Server* server);
void tcp_close_client(TCP_Client** client);

// Dataöverföring
ssize_t tcp_send(int socket_fd, const void* data, size_t length);
ssize_t tcp_receive(int socket_fd, void* buffer, size_t buffer_size);
ssize_t tcp_send_all(int socket_fd, const void* data, size_t length);

// Städning
void tcp_shutdown_server(TCP_Server* server);
void tcp_destroy_server(TCP_Server** server);

#endif