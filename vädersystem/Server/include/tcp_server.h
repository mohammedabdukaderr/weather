#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "natverks_abstraktion.h"
#include <stdbool.h>

// Struktur för TCP-server
typedef struct {
    socket_t lyssnar_socket;                      // Socket för inkommande anslutningar
    int port;                                     // Port att lyssna på
    bool kors;                                    // True om servern körs
} TcpServer;

// Initialisera TCP-server
int initiera_tcp_server(TcpServer* server, int port);

// Vänta på inkommande anslutningar (blockerande)
socket_t acceptera_klient(TcpServer* server);

// Stäng TCP-server
void stang_tcp_server(TcpServer* server);

#endif // TCP_SERVER_H
