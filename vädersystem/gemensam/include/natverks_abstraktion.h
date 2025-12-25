#ifndef NATVERKS_ABSTRAKTION_H
#define NATVERKS_ABSTRAKTION_H

// Platform-specifika includes och definitioner för nätverk

#ifdef _WIN32
    // Windows Socket API (Winsock2)
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")  // Länka Winsock-bibliotek

    typedef SOCKET socket_t;            // Windows använder SOCKET typ
    #define OGILTIG_SOCKET INVALID_SOCKET
    #define SOCKET_FEL SOCKET_ERROR
    #define stang_socket closesocket    // Windows använder closesocket()

    // Windows-specifik initialisering
    static inline int initiera_natverksbibliotek() {
        WSADATA wsa_data;
        return WSAStartup(MAKEWORD(2, 2), &wsa_data);  // Returnerar 0 vid framgång
    }

    static inline void rensa_natverksbibliotek() {
        WSACleanup();
    }

    static inline int hamta_senaste_socket_fel() {
        return WSAGetLastError();
    }

#else
    // POSIX Socket API (Linux, macOS, BSD)
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <netinet/in.h>
    #include <netdb.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <errno.h>
    #include <fcntl.h>

    typedef int socket_t;               // Linux använder int för sockets
    #define OGILTIG_SOCKET -1
    #define SOCKET_FEL -1
    #define stang_socket close          // Linux använder close()

    // Linux behöver ingen speciell initialisering
    static inline int initiera_natverksbibliotek() {
        return 0;  // Alltid lyckas på Linux
    }

    static inline void rensa_natverksbibliotek() {
        // Ingen cleanup behövs på Linux
    }

    static inline int hamta_senaste_socket_fel() {
        return errno;
    }

#endif

// Gemensamma hjälpfunktioner

// Sätt socket till icke-blockerande läge
static inline int satt_icke_blockerande(socket_t socket_fd) {
#ifdef _WIN32
    u_long lage = 1;  // 1 = icke-blockerande
    return ioctlsocket(socket_fd, FIONBIO, &lage);
#else
    int flaggor = fcntl(socket_fd, F_GETFL, 0);
    if (flaggor == -1) return -1;
    return fcntl(socket_fd, F_SETFL, flaggor | O_NONBLOCK);
#endif
}

// Sätt socket till blockerande läge
static inline int satt_blockerande(socket_t socket_fd) {
#ifdef _WIN32
    u_long laige = 0;  // 0 = blockerande
    return ioctlsocket(socket_fd, FIONBIO, &laige);
#else
    int flaggor = fcntl(socket_fd, F_GETFL, 0);
    if (flaggor == -1) return -1;
    return fcntl(socket_fd, F_SETFL, flaggor & ~O_NONBLOCK);
#endif
}

#endif // NATVERKS_ABSTRAKTION_H
