#ifndef NATVERKS_ABSTRAKTION_H
#define NATVERKS_ABSTRAKTION_H

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET socket_t;
    #define OGILTIG_SOCKET INVALID_SOCKET
    #define SOCKET_FEL SOCKET_ERROR
    #define stang_socket closesocket
    
    static inline int initiera_natverksbibliotek(void) {
        WSADATA wsa_data;
        return WSAStartup(MAKEWORD(2, 2), &wsa_data);
    }
    
    static inline void rensa_natverksbibliotek(void) {
        WSACleanup();
    }
    
    static inline int hamta_senaste_socket_fel(void) {
        return WSAGetLastError();
    }
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <errno.h>
    
    typedef int socket_t;
    #define OGILTIG_SOCKET -1
    #define SOCKET_FEL -1
    #define stang_socket close
    
    static inline int initiera_natverksbibliotek(void) {
        return 0;
    }
    
    static inline void rensa_natverksbibliotek(void) {
    }
    
    static inline int hamta_senaste_socket_fel(void) {
        return errno;
    }
#endif

#endif // NATVERKS_ABSTRAKTION_H
