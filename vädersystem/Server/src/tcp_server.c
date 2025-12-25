#include "tcp_server.h"      // TCP-serverfunktioner och datastrukturer
#include "loggning.h"         // För loggning av händelser och fel
#include "konfiguration.h"    // Konfigurationskonstanter (MAX_KLIENTER, portar, etc.)
#include <string.h>           // För memset (nollställning av minnesområden)
#include <stdio.h>            // För snprintf och annan I/O

/**
 * Initierar och startar en TCP-server
 *
 * @param server - Pekare till TcpServer-struktur där serverdata ska lagras
 * @param port - Portnummer att lyssna på (t.ex. 8080)
 * @return 0 vid framgång, -1 vid fel
 *
 * Funktionen skapar en TCP-socket, binder den till angiven port och
 * börjar lyssna efter inkommande anslutningar. Servern kan hantera
 * upp till MAX_KLIENTER samtidiga anslutningar.
 */
int initiera_tcp_server(TcpServer* server, int port) {
    LOGG_INFO("Initierar TCP-server på port %d", port);

    // Initiera nätverksbiblioteket (krävs på Windows för Winsock, no-op på Linux)
    if (initiera_natverksbibliotek() != 0) {
        LOGG_FEL("Kunde inte initialisera nätverksbibliotek");
        return -1;
    }

    // Spara porten i server-strukturen för framtida referens
    server->port = port;

    // Servern är inte igång än (sätts till true när listen() lyckas)
    server->kors = false;

    // Skapa en TCP-socket (AF_INET = IPv4, SOCK_STREAM = TCP, IPPROTO_TCP = TCP-protokoll)
    server->lyssnar_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Kontrollera om socket-skapandet lyckades
    if (server->lyssnar_socket == OGILTIG_SOCKET) {
        LOGG_FEL("Kunde inte skapa socket: fel %d", hamta_senaste_socket_fel());
        rensa_natverksbibliotek();  // Städa upp nätverksbiblioteket
        return -1;
    }

    // Sätt socket-option SO_REUSEADDR för att kunna återanvända porten omedelbart
    // Detta förhindrar "Address already in use" fel när servern startas om snabbt
    int ja = 1;  // 1 = aktivera optionen
    if (setsockopt(server->lyssnar_socket, SOL_SOCKET, SO_REUSEADDR,
                   (const char*)&ja, sizeof(ja)) < 0) {
        LOGG_VARNING("Kunde inte sätta SO_REUSEADDR");
        // Fortsätt ändå - inte kritiskt
    }

    // Förbered serveradressen (IP och port)
    struct sockaddr_in server_adress;

    // Nollställ hela strukturen för att undvika skräpdata
    memset(&server_adress, 0, sizeof(server_adress));

    // Sätt adressfamilj till IPv4
    server_adress.sin_family = AF_INET;

    // Lyssna på alla nätverksgränssnitt (0.0.0.0)
    // INADDR_ANY betyder att servern accepterar anslutningar från alla IP-adresser
    server_adress.sin_addr.s_addr = INADDR_ANY;

    // Sätt portnumret (konverterat till network byte order med htons)
    // htons = "host to network short" - konverterar från CPU:ns byteordning till nätverksordning
    server_adress.sin_port = htons((uint16_t)port);

    // Bind socketen till adressen (IP och port)
    // Detta "reserverar" porten för vår server
    if (bind(server->lyssnar_socket, (struct sockaddr*)&server_adress,
             sizeof(server_adress)) == SOCKET_FEL) {
        LOGG_FEL("Kunde inte binda socket till port %d: fel %d",
                 port, hamta_senaste_socket_fel());
        stang_socket(server->lyssnar_socket);  // Stäng socketen
        rensa_natverksbibliotek();              // Städa upp
        return -1;
    }

    // Börja lyssna efter inkommande anslutningar
    // MAX_KLIENTER definierar hur många anslutningar som kan vänta i kö
    if (listen(server->lyssnar_socket, MAX_KLIENTER) == SOCKET_FEL) {
        LOGG_FEL("Kunde inte lyssna på socket: fel %d", hamta_senaste_socket_fel());
        stang_socket(server->lyssnar_socket);
        rensa_natverksbibliotek();
        return -1;
    }

    // Servern är nu igång och redo att acceptera klienter
    server->kors = true;
    LOGG_INFO("TCP-server lyssnar på port %d", port);
    return 0;  // Framgång
}

/**
 * Accepterar en väntande klientanslutning
 *
 * @param server - Pekare till den lyssn ande servern
 * @return Socket-descriptor för den nya klienten, eller OGILTIG_SOCKET vid fel
 *
 * Funktionen blockerar tills en klient ansluter (om inte socketen är icke-blockerande).
 * När en klient ansluter skapar funktionen en ny socket för kommunikation med
 * just den klienten, medan server->lyssnar_socket fortsätter lyssna efter fler.
 */
socket_t acceptera_klient(TcpServer* server) {
    // Struktur för att lagra klientens adressinformation (IP och port)
    struct sockaddr_in klient_adress;
    socklen_t klient_adress_langd = sizeof(klient_adress);

    // Acceptera en väntande anslutning från kön
    // accept() skapar en ny socket för kommunikation med denna specifika klient
    // Den ursprungliga lyssnar_socket fortsätter ta emot nya anslutningar
    socket_t klient_socket = accept(server->lyssnar_socket,
                                     (struct sockaddr*)&klient_adress,
                                     &klient_adress_langd);

    // Kontrollera om accept() misslyckades
    if (klient_socket == OGILTIG_SOCKET) {
        int fel_kod = hamta_senaste_socket_fel();

        // Vissa felkoder är "normala" och ska inte loggas som fel
#ifdef _WIN32
        // På Windows: WSAEWOULDBLOCK betyder att ingen klient väntar (icke-blockerande socket)
        if (fel_kod != WSAEWOULDBLOCK) {
#else
        // På Linux/Unix: EWOULDBLOCK eller EAGAIN betyder detsamma
        if (fel_kod != EWOULDBLOCK && fel_kod != EAGAIN) {
#endif
            LOGG_FEL("Kunde inte acceptera klient: fel %d", fel_kod);
        }
        return OGILTIG_SOCKET;  // Ingen klient accepterades
    }

    // Konvertera klientens IP-adress från binärt format till läsbar sträng
    char klient_ip[INET_ADDRSTRLEN];  // Buffer för IP-strängen (t.ex. "192.168.1.100")
    inet_ntop(AF_INET, &klient_adress.sin_addr, klient_ip, sizeof(klient_ip));

    // Logga information om den nya klienten
    // ntohs konverterar portnumret från network byte order till host byte order
    LOGG_INFO("Ny klient ansluten från %s:%d",
              klient_ip, ntohs(klient_adress.sin_port));

    return klient_socket;  // Returnera socketen för kommunikation med klienten
}

/**
 * Stänger TCP-servern och frigör resurser
 *
 * @param server - Pekare till servern som ska stängas
 *
 * Funktionen stänger lyssnande socket och rensar nätverksbiblioteket.
 * Efter detta kan servern inte längre acceptera nya anslutningar.
 * Befintliga klientanslutningar påverkas inte av detta.
 */
void stang_tcp_server(TcpServer* server) {
    // Kontrollera om servern faktiskt körs innan vi stänger den
    if (server->kors) {
        LOGG_INFO("Stänger TCP-server");

        // Stäng den lyssnande socketen (inga fler anslutningar accepteras)
        stang_socket(server->lyssnar_socket);

        // Markera att servern inte längre körs
        server->kors = false;

        // Rensa nätverksbiblioteket (speciellt viktigt på Windows)
        rensa_natverksbibliotek();
    }
}
