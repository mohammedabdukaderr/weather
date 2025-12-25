#include "TCP.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>

// Skapar en ny TCP-server
TCP_Server* tcp_create_server(uint16_t port, int max_clients) {
    TCP_Server* server = malloc(sizeof(TCP_Server));
    if (!server) return NULL;

    server->port = port;
    server->max_clients = max_clients;
    server->is_running = 0;
    server->bind_address = NULL;
    server->socket_fd = -1;

    // Nollställ address-strukturen
    memset(&server->address, 0, sizeof(server->address));

    return server;
}

// Startar TCP-servern
int tcp_start_server(TCP_Server* server) {
    if (!server) return -1;

    // Skapa socket
    server->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->socket_fd < 0) {
        perror("socket() misslyckades");
        return -1;
    }

    // Sätt socket-optioner för att återanvända adressen
    int opt = 1;
    if (setsockopt(server->socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt() misslyckades");
        close(server->socket_fd);
        return -1;
    }

    // Konfigurera serveradress
    server->address.sin_family = AF_INET;
    server->address.sin_addr.s_addr = INADDR_ANY;
    server->address.sin_port = htons(server->port);

    // Bind socket till port
    if (bind(server->socket_fd, (struct sockaddr*)&server->address, sizeof(server->address)) < 0) {
        perror("bind() misslyckades");
        close(server->socket_fd);
        return -1;
    }

    // Lyssna på inkommande anslutningar
    if (listen(server->socket_fd, server->max_clients) < 0) {
        perror("listen() misslyckades");
        close(server->socket_fd);
        return -1;
    }

    server->is_running = 1;
    return 0;
}

// Sätt timeout för server socket
int tcp_set_timeout(TCP_Server* server, int seconds) {
    if (!server || server->socket_fd < 0) return -1;

    struct timeval timeout;
    timeout.tv_sec = seconds;
    timeout.tv_usec = 0;

    if (setsockopt(server->socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt(SO_RCVTIMEO) misslyckades");
        return -1;
    }

    return 0;
}

// Accepterar en ny klient
TCP_Client* tcp_accept_client(TCP_Server* server) {
    if (!server || !server->is_running) return NULL;

    TCP_Client* client = malloc(sizeof(TCP_Client));
    if (!client) return NULL;

    socklen_t addr_len = sizeof(client->address);
    client->socket_fd = accept(server->socket_fd, (struct sockaddr*)&client->address, &addr_len);

    if (client->socket_fd < 0) {
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            perror("accept() misslyckades");
        }
        free(client);
        return NULL;
    }

    // Konvertera IP-adress till sträng
    inet_ntop(AF_INET, &client->address.sin_addr, client->ip, INET_ADDRSTRLEN);
    client->port = ntohs(client->address.sin_port);

    return client;
}

// Stänger en klientanslutning
void tcp_close_client(TCP_Client** client_ptr) {
    if (!client_ptr || !*client_ptr) return;

    TCP_Client* client = *client_ptr;

    if (client->socket_fd >= 0) {
        close(client->socket_fd);
    }

    free(client);
    *client_ptr = NULL;
}

// Skickar data via TCP
ssize_t tcp_send(int socket_fd, const void* data, size_t length) {
    if (socket_fd < 0 || !data || length == 0) return -1;

    ssize_t sent = send(socket_fd, data, length, 0);
    if (sent < 0) {
        perror("send() misslyckades");
    }

    return sent;
}

// Tar emot data via TCP
ssize_t tcp_receive(int socket_fd, void* buffer, size_t buffer_size) {
    if (socket_fd < 0 || !buffer || buffer_size == 0) return -1;

    ssize_t received = recv(socket_fd, buffer, buffer_size - 1, 0);

    if (received < 0) {
        perror("recv() misslyckades");
        return -1;
    }

    // Null-terminera bufferten om det är text
    if (received < (ssize_t)buffer_size) {
        ((char*)buffer)[received] = '\0';
    }

    return received;
}

// Skickar all data (loop tills allt är skickat)
ssize_t tcp_send_all(int socket_fd, const void* data, size_t length) {
    if (socket_fd < 0 || !data || length == 0) return -1;

    size_t total_sent = 0;
    const char* ptr = (const char*)data;

    while (total_sent < length) {
        ssize_t sent = send(socket_fd, ptr + total_sent, length - total_sent, 0);

        if (sent < 0) {
            perror("send() misslyckades");
            return -1;
        }

        if (sent == 0) {
            break; // Anslutningen stängd
        }

        total_sent += sent;
    }

    return total_sent;
}

// Stänger av servern (stoppar accept)
void tcp_shutdown_server(TCP_Server* server) {
    if (!server) return;

    server->is_running = 0;

    if (server->socket_fd >= 0) {
        shutdown(server->socket_fd, SHUT_RDWR);
    }
}

// Förstör server och frigör minne
void tcp_destroy_server(TCP_Server** server_ptr) {
    if (!server_ptr || !*server_ptr) return;

    TCP_Server* server = *server_ptr;

    tcp_shutdown_server(server);

    if (server->socket_fd >= 0) {
        close(server->socket_fd);
    }

    if (server->bind_address) {
        free(server->bind_address);
    }

    free(server);
    *server_ptr = NULL;
}
