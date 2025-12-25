#include "natverks_abstraktion.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ============================================================================
// SERVERKONFIGURATION
// ============================================================================
// Definierar standardvärden för serverns IP-adress och port
// 127.0.0.1 är localhost (loopback-adress som pekar på den egna datorn)
// Port 8080 är en vanlig port för utvecklingsservrar
#define SERVER_ADRESS "127.0.0.1"
#define SERVER_PORT 8080

// ============================================================================
// HJÄLPFUNKTIONER FÖR JSON-PARSING
// ============================================================================
// Dessa funktioner implementerar en enkel JSON-parser för att extrahera
// värden från JSON-strängar. Vi använder inte ett fullt JSON-bibliotek för
// att hålla koden enkel och lärorik.

/**
 * Parsar ett flyttal från en JSON-sträng
 *
 * FUNKTIONALITET:
 * 1. Bygger en söksträng i formatet: "nyckel":
 * 2. Letar efter denna söksträng i JSON-texten
 * 3. Hoppar förbi söksträng till värdet
 * 4. Konverterar texten till ett flyttal med atof()
 *
 * EXEMPEL:
 * JSON: {"temperatur": 23.5, "luftfuktighet": 65}
 * parse_json_float(json, "temperatur") returnerar 23.5
 *
 * @param json - Den fullständiga JSON-strängen att söka i
 * @param nyckel - Namnet på JSON-nyckeln (utan citattecken)
 * @return Flyttalsvärdet, eller 0.0 om nyckeln inte hittas
 */
float parse_json_float(const char* json, const char* nyckel) {
    char sokstrang[128];
    // Bygger söksträng: "nyckel": (med citattecken och kolon)
    snprintf(sokstrang, sizeof(sokstrang), "\"%s\": ", nyckel);

    // Letar efter söksträng i JSON-texten med strstr()
    // strstr() returnerar en pekare till första matchningen, eller NULL
    const char* position = strstr(json, sokstrang);
    if (!position) return 0.0f;  // Nyckeln hittades inte

    // Flytta pekaren förbi söksträng till värdet
    position += strlen(sokstrang);

    // atof() (ASCII to Float) konverterar en sträng till float
    // Exempel: atof("23.5") blir 23.5
    return (float)atof(position);
}

/**
 * Parsar en textsträng från en JSON-sträng
 *
 * FUNKTIONALITET:
 * 1. Bygger en söksträng: "nyckel": "
 * 2. Hittar startpositionen efter detta mönster
 * 3. Hittar slutpositionen (nästa citattecken)
 * 4. Kopierar strängen mellan start och slut
 * 5. Avslutar med null-terminator (\0)
 *
 * EXEMPEL:
 * JSON: {"stad": "Stockholm", "land": "Sverige"}
 * parse_json_string(json, "stad", buffer, 64) skriver "Stockholm" i buffer
 *
 * @param json - Den fullständiga JSON-strängen
 * @param nyckel - Namnet på JSON-nyckeln
 * @param resultat - Buffer där resultatet ska skrivas
 * @param storlek - Maximal storlek på resultatbuffern (för säkerhet)
 */
void parse_json_string(const char* json, const char* nyckel, char* resultat, size_t storlek) {
    char sokstrang[128];
    // Bygger söksträng: "nyckel": " (med tre citattecken totalt)
    snprintf(sokstrang, sizeof(sokstrang), "\"%s\": \"", nyckel);

    // Hitta var värdet börjar
    const char* start = strstr(json, sokstrang);
    if (!start) {
        // Nyckeln hittades inte, returnera tom sträng
        resultat[0] = '\0';
        return;
    }

    // Flytta pekaren förbi söksträng till själva värdet
    start += strlen(sokstrang);

    // Hitta avslutande citattecken med strchr() (string character search)
    const char* slut = strchr(start, '"');
    if (!slut) {
        // Inget avslutande citattecken, returnera tom sträng
        resultat[0] = '\0';
        return;
    }

    // Beräkna längden på strängen mellan citattecknen
    // Pekararitmetik: slut - start ger antal bytes mellan adresserna
    size_t langd = (size_t)(slut - start);

    // Säkerhetscheck: Klipp av om strängen är för lång för buffern
    // Vi behöver plats för null-terminator, därför storlek - 1
    if (langd >= storlek) langd = storlek - 1;

    // Kopiera strängen från JSON till resultatbuffern
    // memcpy är snabbare än strcpy för kända längder
    memcpy(resultat, start, langd);

    // Avsluta strängen med null-terminator (obligatoriskt i C)
    resultat[langd] = '\0';
}

// ============================================================================
// VISNINGSFUNKTIONER
// ============================================================================

/**
 * Visar väderdata i en formaterad låda i terminalen
 *
 * FUNKTIONALITET:
 * 1. Extraherar alla värden från JSON-svaret med parse-funktionerna
 * 2. Skapar en visuell låda med box-drawing characters (UTF-8)
 * 3. Formaterar och visar alla värden med lämpliga enheter
 *
 * BOX-DRAWING CHARACTERS:
 * ╔═╗  (U+2554, U+2550, U+2557) - Övre hörn och linje
 * ║    (U+2551) - Vertikal linje
 * ╠═╣  (U+2560, U+2550, U+2563) - Mellansektion
 * ╚═╝  (U+255A, U+2550, U+255D) - Nedre hörn
 *
 * FORMATSTRÄNGARNA:
 * %.1f - Flyttal med 1 decimal (temperatur, vindhastighet)
 * %.0f - Flyttal utan decimaler (luftfuktighet, lufttryck)
 * %s   - Textsträng
 * %-24s - Vänsterjusterad sträng med minst 24 tecken bredd
 *
 * @param json - JSON-svaret från servern med väderdata
 */
void visa_vader(const char* json) {
    // Deklarera buffertar för alla värden vi ska extrahera
    char stad[64];              // Stadnamn (max 63 tecken + null)
    char beskrivning[128];      // Väderbeskrivning (ex: "Klart", "Molnigt")
    float temperatur;           // Temperatur i Celsius
    float luftfuktighet;        // Relativ luftfuktighet i procent (0-100)
    float vindhastighet;        // Vindhastighet i meter per sekund
    float lufttryck;            // Lufttryck i hektopascal (hPa)

    // Extrahera alla värden från JSON-svaret
    // Parsningsordningen spelar ingen roll, men vi grupperar liknande typer
    parse_json_string(json, "stad", stad, sizeof(stad));
    temperatur = parse_json_float(json, "temperatur");
    luftfuktighet = parse_json_float(json, "luftfuktighet");
    vindhastighet = parse_json_float(json, "vindhastighet");
    lufttryck = parse_json_float(json, "lufttryck");
    parse_json_string(json, "beskrivning", beskrivning, sizeof(beskrivning));

    // Rita den formaterade lådan
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════╗\n");
    printf("║             AKTUELLT VÄDER - %-24s ║\n", stad);
    printf("╠═══════════════════════════════════════════════════════╣\n");

    // %.1f formaterar flyttal med exakt 1 decimal
    printf("║  🌡️  Temperatur:    %.1f°C\n", temperatur);

    // %.0f formaterar flyttal utan decimaler (rundar av automatiskt)
    printf("║  💧  Luftfuktighet:  %.0f%%\n", luftfuktighet);

    printf("║  💨  Vindhastighet:  %.1f m/s\n", vindhastighet);
    printf("║  📊  Lufttryck:      %.0f hPa\n", lufttryck);

    // %s skriver ut hela strängen
    printf("║  ☁️  Beskrivning:    %s\n", beskrivning);

    printf("╚═══════════════════════════════════════════════════════╝\n");
    printf("\n");
}

// ============================================================================
// HTTP-KOMMUNIKATION
// ============================================================================

/**
 * Skickar en HTTP GET-request till servern
 *
 * HTTP REQUEST-FORMAT:
 * HTTP requests består av:
 * 1. Request-rad: Metod (GET/POST) + Sökväg + HTTP-version
 * 2. Headers: Metadata om requesten (Host, Connection, etc.)
 * 3. Tom rad: \r\n\r\n signalerar slut på headers
 * 4. Body: (tom för GET-requests)
 *
 * EXEMPEL PÅ GENERERAD REQUEST:
 * GET /weather?city=Stockholm&country=SE HTTP/1.1\r\n
 * Host: localhost\r\n
 * Connection: close\r\n
 * \r\n
 *
 * LINE ENDINGS:
 * HTTP-standarden kräver \r\n (Carriage Return + Line Feed)
 * Detta är viktigt - servern förväntar sig dessa exakta tecken
 *
 * @param sock - Socket-descriptorn för den öppna anslutningen
 * @param sokvag - URL-sökvägen (ex: "/weather?city=Stockholm&country=SE")
 * @return Antal skickade bytes, eller negativt tal vid fel
 */
int skicka_http_get(socket_t sock, const char* sokvag) {
    char forfragan[1024];

    // Bygg HTTP GET-request med snprintf för säker strängformatering
    // snprintf begränsar output till sizeof(forfragan) för att undvika buffer overflow
    snprintf(forfragan, sizeof(forfragan),
             "GET %s HTTP/1.1\r\n"           // Request-rad: metod, sökväg, version
             "Host: localhost\r\n"           // Host header (obligatorisk i HTTP/1.1)
             "Connection: close\r\n"         // Stäng anslutning efter svar (enkel HTTP)
             "\r\n",                         // Tom rad markerar slut på headers
             sokvag);

    // send() skickar data över socketen
    // Parametrar: socket, data, längd, flags (0 = inga speciella flaggor)
    // Returnerar antal skickade bytes, eller negativt tal vid fel
    // Vi castar strlen till int eftersom send() förväntar sig int på Windows
    return send(sock, forfragan, (int)strlen(forfragan), 0);
}

/**
 * Tar emot HTTP-svar från servern och extraherar JSON-body
 *
 * HTTP RESPONSE-FORMAT:
 * 1. Status-rad: HTTP/1.1 200 OK
 * 2. Headers: Content-Type, Content-Length, etc.
 * 3. Tom rad: \r\n\r\n markerar slut på headers
 * 4. Body: JSON-data (det vi är intresserade av)
 *
 * EXEMPEL PÅ HTTP-SVAR:
 * HTTP/1.1 200 OK\r\n
 * Content-Type: application/json\r\n
 * Content-Length: 156\r\n
 * \r\n
 * {"stad":"Stockholm","temperatur":23.5,...}
 *
 * MOTTAGNINGSLOOP:
 * recv() kan ta emot data i små bitar, därför loopar vi tills vi fått allt
 * recv() returnerar 0 när servern stänger anslutningen (Connection: close)
 *
 * @param sock - Socket-descriptorn
 * @param json_buffer - Buffer där JSON-body ska lagras
 * @param buffer_storlek - Maximal storlek på json_buffer
 * @return Längden på JSON-body, eller -1 vid fel
 */
int ta_emot_http_svar(socket_t sock, char* json_buffer, size_t buffer_storlek) {
    char buffer[8192];          // Temporär buffer för hela HTTP-svaret (8 KB)
    int totalt_mottaget = 0;    // Räknare för totalt antal mottagna bytes
    int mottaget_denna_gång;    // Antal bytes från senaste recv()-anropet

    // Mottagningsloop: Ta emot data tills servern stänger anslutningen
    // recv() returnerar antal mottagna bytes, 0 vid anslutning stängd, -1 vid fel
    while ((mottaget_denna_gång = recv(sock, buffer + totalt_mottaget,
                                       sizeof(buffer) - totalt_mottaget - 1, 0)) > 0) {
        // Addera till total
        totalt_mottaget += mottaget_denna_gång;

        // Säkerhetscheck: Sluta om buffern blir full
        // Vi reserverar 1 byte för null-terminator
        if (totalt_mottaget >= (int)sizeof(buffer) - 1) break;
    }

    // Kontrollera om vi fick någon data
    if (totalt_mottaget <= 0) return -1;

    // Avsluta strängen med null-terminator så vi kan använda strängfunktioner
    buffer[totalt_mottaget] = '\0';

    // Hitta början på JSON-body efter HTTP-headers
    // HTTP-headers avslutas alltid med \r\n\r\n (tom rad)
    // strstr() hittar denna sekvens i svaret
    char* body_start = strstr(buffer, "\r\n\r\n");
    if (!body_start) return -1;  // Ogiltigt HTTP-svar

    // Flytta pekaren förbi \r\n\r\n (4 tecken) till början av body
    body_start += 4;

    // Kopiera JSON-body till användarens buffer
    // strncpy kopierar max buffer_storlek - 1 tecken (plats för null-terminator)
    strncpy(json_buffer, body_start, buffer_storlek - 1);

    // Säkerställ null-terminering
    json_buffer[buffer_storlek - 1] = '\0';

    // Returnera längden på JSON-body
    return (int)strlen(json_buffer);
}

// ============================================================================
// NÄTVERKSANSLUTNING
// ============================================================================

/**
 * Skapar en TCP-anslutning till servern
 *
 * SOCKETS OCH TCP/IP:
 * En socket är en ändpunkt för tvåvägskommunikation över nätverk
 * TCP (Transmission Control Protocol) garanterar tillförlitlig dataöverföring
 *
 * ANSLUTNINGSPROCESS:
 * 1. socket() - Skapa en socket-descriptor
 * 2. Fyll i server-adress (IP + port) i sockaddr_in-struktur
 * 3. connect() - Etablera TCP-anslutning (tre-vägs handslag)
 * 4. Använd send()/recv() för kommunikation
 * 5. Stäng socket när klar
 *
 * SOCKADDR_IN-STRUKTUR:
 * sin_family   - Adressfamilj (AF_INET = IPv4)
 * sin_port     - Portnummer i network byte order (big-endian)
 * sin_addr     - IP-adress som 32-bitars heltal
 *
 * BYTE ORDER:
 * Nätverk använder big-endian (network byte order)
 * Datorer kan vara big-endian eller little-endian (host byte order)
 * htons() = Host TO Network Short (konverterar 16-bitars portnummer)
 *
 * @param adress - IP-adress som sträng (ex: "127.0.0.1")
 * @param port - Portnummer (ex: 8080)
 * @return Socket-descriptor vid framgång, OGILTIG_SOCKET vid fel
 */
socket_t anslut_till_server(const char* adress, int port) {
    // Skapa en socket för TCP-kommunikation
    // AF_INET = IPv4 adressfamilj
    // SOCK_STREAM = Tillförlitlig, tvåvägs, anslutningsbaserad byteström (TCP)
    // 0 = Använd standardprotokoll för SOCK_STREAM (TCP)
    socket_t sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == OGILTIG_SOCKET) {
        fprintf(stderr, "FEL: Kunde inte skapa socket\n");
        return OGILTIG_SOCKET;
    }

    // Skapa och fyll i server-adressstruktur
    struct sockaddr_in server_adress_info;

    // memset() nollställer all minne i strukturen (viktigt för säkerhet)
    memset(&server_adress_info, 0, sizeof(server_adress_info));

    // Fyll i adressfamilj: IPv4
    server_adress_info.sin_family = AF_INET;

    // Fyll i port: Konvertera från host till network byte order
    // htons = Host TO Network Short (16-bitars värde)
    // Vi castar till uint16_t för att säkerställa rätt storlek
    server_adress_info.sin_port = htons((uint16_t)port);

    // Konvertera IP-adress från text till binär form
    // Detta är plattformsberoende:
#ifdef _WIN32
    // Windows: inet_addr() konverterar "127.0.0.1" till 32-bitars värde
    // Returnerar adressen i network byte order
    server_adress_info.sin_addr.s_addr = inet_addr(adress);
#else
    // Linux/Unix: inet_pton() är modernare och mer flexibel
    // pton = Presentation TO Network
    // Stöder både IPv4 och IPv6
    inet_pton(AF_INET, adress, &server_adress_info.sin_addr);
#endif

    // Informera användaren om anslutningsförsök
    printf("Ansluter till http://%s:%d...\n", adress, port);

    // Etablera TCP-anslutning till servern
    // connect() initierar TCP tre-vägs handslag (SYN, SYN-ACK, ACK)
    // Detta blockerar tills anslutning etableras eller timeout
    // Vi castar sockaddr_in* till sockaddr* (generisk socket-adress)
    if (connect(sock, (struct sockaddr*)&server_adress_info, sizeof(server_adress_info)) < 0) {
        fprintf(stderr, "FEL: Kunde inte ansluta till server\n");
        fprintf(stderr, "Kontrollera att servern körs på %s:%d\n", adress, port);

        // Stäng socket vid fel
        stang_socket(sock);
        return OGILTIG_SOCKET;
    }

    // Anslutning etablerad!
    printf("✓ Ansluten!\n");
    return sock;
}

// ============================================================================
// HUVUDPROGRAM
// ============================================================================

/**
 * Huvudfunktion - Programmets ingångspunkt
 *
 * PROGRAMFLÖDE:
 * 1. Initialisera nätverksbibliotek (Windows kräver WSAStartup)
 * 2. Visa välkomstmeddelande
 * 3. Läs server-adress från kommandoradsargument eller använd default
 * 4. Huvudloop: Visa meny och hantera användarval
 * 5. Städa upp och stäng programmet
 *
 * KOMMANDORADSARGUMENT:
 * argv[0] = Programmets namn
 * argv[1] = Server-adress (frivillig)
 * argv[2] = Server-port (frivillig)
 *
 * MINNESHANTERING:
 * C kräver manuell minneshantering. Vi använder stack-allokerade buffertar
 * för enkelhetens skull (ingen dynamisk allokering med malloc/free).
 *
 * @param argc - Antal kommandoradsargument (inkl. programnamn)
 * @param argv - Array av argumentsträngar
 * @return 0 vid framgång, 1 vid fel
 */
int main(int argc, char* argv[]) {
    // Initialisera plattformsspecifikt nätverksbibliotek
    // Windows: Initialiserar Winsock2
    // Linux: Gör ingenting (sockets fungerar direkt)
    if (initiera_natverksbibliotek() != 0) {
        fprintf(stderr, "FEL: Kunde inte initialisera nätverksbibliotek\n");
        return 1;
    }

    // Visa välkomstmeddelande med box-drawing characters
    printf("╔═══════════════════════════════════════════════════════╗\n");
    printf("║          VÄDERSYSTEM - C-KLIENT (HTTP/JSON)           ║\n");
    printf("╚═══════════════════════════════════════════════════════╝\n\n");

    // Läs server-konfiguration från kommandoradsargument eller använd default
    // Ternär operator: (villkor) ? värde_om_sant : värde_om_falskt
    const char* server_adress = (argc > 1) ? argv[1] : SERVER_ADRESS;
    int server_port = (argc > 2) ? atoi(argv[2]) : SERVER_PORT;

    // Huvudloop - Kör tills användaren väljer att avsluta
    while (1) {
        // Visa huvudmeny
        printf("\nVälj alternativ:\n");
        printf("  1. Hämta aktuellt väder\n");
        printf("  2. Hämta väderprognos\n");
        printf("  3. Avsluta\n");
        printf("Val: ");

        // Läs användarens val
        int val;
        // scanf returnerar antal framgångsrikt lästa värden
        // Om användaren skriver "abc" istället för ett tal, returnerar scanf 0
        if (scanf("%d", &val) != 1) {
            fprintf(stderr, "Ogiltigt val\n");
            // Rensa input-buffern genom att läsa till nyrad
            // Detta förhindrar oändlig loop vid ogiltigt input
            while (getchar() != '\n');
            continue;  // Gå till nästa iteration i loopen
        }
        // Rensa eventuell kvarvarande input (inkl. nyradstecknet)
        while (getchar() != '\n');

        // Hantera avsluta-val
        if (val == 3) break;  // Bryt ur loopen och avsluta programmet

        // Validera att valet är 1 eller 2
        if (val != 1 && val != 2) {
            printf("Ogiltigt val, försök igen\n");
            continue;
        }

        // Läs stadnamn och landskod från användaren
        char stad[64], landskod[8];

        printf("Ange stad: ");
        // fgets läser en hel rad inkl. nyradstecken (säkrare än scanf)
        if (fgets(stad, sizeof(stad), stdin) == NULL) continue;
        // Ta bort nyradstecknet genom att ersätta det med null-terminator
        // strcspn returnerar index för första matchningen av tecknen i andra argumentet
        stad[strcspn(stad, "\n")] = '\0';

        printf("Ange landskod (ex: SE, US, GB): ");
        if (fgets(landskod, sizeof(landskod), stdin) == NULL) continue;
        landskod[strcspn(landskod, "\n")] = '\0';

        // Om användaren inte angav landskod, använd Sverige som standard
        if (strlen(landskod) == 0) {
            strcpy(landskod, "SE");  // Kopiera "SE" till landskod-buffern
        }

        // Anslut till servern
        // Varje request skapar en ny anslutning (Connection: close)
        socket_t sock = anslut_till_server(server_adress, server_port);
        if (sock == OGILTIG_SOCKET) continue;  // Anslutning misslyckades, försök igen

        // Bygg URL baserat på användarens val
        char url[512];
        if (val == 1) {
            // Hämta aktuellt väder
            // Query-parametrar i URL: ?nyckel1=värde1&nyckel2=värde2
            snprintf(url, sizeof(url), "/weather?city=%s&country=%s", stad, landskod);
        } else {
            // Hämta väderprognos
            snprintf(url, sizeof(url), "/forecast?city=%s&country=%s", stad, landskod);
        }

        // Skicka HTTP GET-request
        if (skicka_http_get(sock, url) < 0) {
            fprintf(stderr, "FEL: Kunde inte skicka request\n");
            stang_socket(sock);
            continue;
        }

        // Ta emot HTTP-svar och extrahera JSON-body
        char json_svar[4096];  // Buffer för JSON-data (4 KB)
        if (ta_emot_http_svar(sock, json_svar, sizeof(json_svar)) < 0) {
            fprintf(stderr, "FEL: Kunde inte ta emot svar\n");
            stang_socket(sock);
            continue;
        }

        // Kontrollera om servern returnerade ett felmeddelande
        // Fel-svar har formatet: {"fel": true, "meddelande": "..."}
        if (strstr(json_svar, "\"fel\": true")) {
            char felmeddelande[256];
            parse_json_string(json_svar, "meddelande", felmeddelande, sizeof(felmeddelande));
            fprintf(stderr, "\nFEL från server: %s\n", felmeddelande);
        } else if (val == 1) {
            // Visa väderdata i formaterad låda
            visa_vader(json_svar);
        } else {
            // Visa väderprognos (förenklad - visar bara råa JSON-data)
            // En fullständig implementation skulle parsa och formatera prognosdata
            printf("\nPrognos (JSON):\n%s\n", json_svar);
        }

        // Stäng socket-anslutningen
        stang_socket(sock);
    }

    // Avslutningsmeddelande
    printf("\nHejdå!\n");

    // Städa upp nätverksbibliotek
    // Windows: Avslutar Winsock (WSACleanup)
    // Linux: Gör ingenting
    rensa_natverksbibliotek();

    return 0;  // Framgångsrikt avslutande
}
