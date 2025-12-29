#define _DEFAULT_SOURCE      // För usleep() på Linux
#include "tcp_server.h"      // För TCP-serverfunktionalitet
#include "vader_api.h"       // För att hämta väderdata från OpenWeatherMap
#include "cache.h"           // För att cacha väderdata lokalt
#include "loggning.h"        // För loggningssystem
#include "konfiguration.h"   // För SERVER_PORT och andra konfigurationer
#include "http_server.h"     // För att parsa och skapa HTTP-meddelanden
#include <stdio.h>           // För fprintf, snprintf
#include <string.h>          // För strcmp, strlen
#include <signal.h>          // För signal-hantering (Ctrl+C)
#include <stdbool.h>         // För bool, true, false
#include <stdlib.h>          // För atoi
#include <unistd.h>          // För usleep (Unix/Linux)

// Global flagga för att kontrollera serverns huvudloop
// Sätts till false när användaren trycker Ctrl+C för att stoppa servern
// volatile gör att kompilatorn inte optimerar bort variabeln
static volatile bool kors = true;

/**
 * Signal-hanterare för Ctrl+C (SIGINT) och SIGTERM
 *
 * @param signal - Signal-nummer som orsakade anropet (oanvänd i vår implementation)
 *
 * Funktionen anropas när användaren trycker Ctrl+C eller när systemet
 * skickar en SIGTERM-signal. Den sätter kors-flaggan till false vilket
 * gör att huvudloopen avslutas och servern stängs ned på ett snyggt sätt.
 */
void signal_hanterare(int signal) {
    (void)signal;  // Markera parametern som avsiktligt oanvänd för att undvika kompilatorvarningar
    LOGG_INFO("Mottog stoppSignal, stänger servern...");
    kors = false;  // Detta gör att huvudloopen i main() avslutas
}

/**
 * Skapar en JSON-representation av väderdata
 *
 * @param data - Pekare till VaderData-struktur med väderinfo
 * @param json_buffer - Buffert där JSON-strängen ska skapas
 * @param storlek - Storlek på bufferten i bytes
 *
 * Funktionen formaterar väderdata som en välformad JSON-struktur.
 * Resultatet kan skickas direkt till HTTP-klienter som förstår JSON.
 *
 * Exempel på output:
 * {
 *   "stad": "Stockholm",
 *   "temperatur": 15.5,
 *   "luftfuktighet": 65,
 *   "vindhastighet": 3.2,
 *   "lufttryck": 1013,
 *   "beskrivning": "lätt regn",
 *   "ikon_id": "10d",
 *   "tidsstampel": 1234567890
 * }
 */
void skapa_vader_json(const VaderData* data, char* json_buffer, size_t storlek) {
    snprintf(json_buffer, storlek,
             "{\n"
             "  \"stad\": \"%s\",\n"                  // Stadens namn som text
             "  \"land\": \"%s\",\n"                  // Landskod (t.ex. SE, GB, US)
             "  \"temperatur\": %.1f,\n"              // Temperatur med 1 decimal (Celsius)
             "  \"luftfuktighet\": %.0f,\n"           // Luftfuktighet utan decimaler (%)
             "  \"vindhastighet\": %.1f,\n"           // Vindhastighet med 1 decimal (m/s)
             "  \"lufttryck\": %.0f,\n"               // Lufttryck utan decimaler (hPa)
             "  \"beskrivning\": \"%s\",\n"           // Textbeskrivning av vädret
             "  \"ikon_id\": \"%s\",\n"               // Ikon-ID för väderikoner
             "  \"tidsstampel\": %lld\n"              // Unix-tidsstämpel (sekunder sedan 1970)
             "}",
             data->stad,
             data->land,
             data->temperatur,
             data->luftfuktighet,
             data->vindhastighet,
             data->lufttryck,
             data->beskrivning,
             data->ikon_id,
             (long long)data->tidsstampel);
}

/**
 * Skapar en JSON-representation av prognosdata
 *
 * @param prognos - Pekare till VaderPrognos-struktur med prognos för flera dagar
 * @param json_buffer - Buffert där JSON-strängen ska skapas
 * @param storlek - Storlek på bufferten i bytes
 *
 * Funktionen formaterar prognosdata som en JSON-array med objekt för varje dag.
 * Denna struktur gör det enkelt för klienter att iterera genom dagarna.
 *
 * Exempel på output:
 * {
 *   "antal_dagar": 2,
 *   "dagar": [
 *     { "stad": "Stockholm", "temperatur": 15.5, ... },
 *     { "stad": "Stockholm", "temperatur": 16.2, ... }
 *   ]
 * }
 */
void skapa_prognos_json(const VaderPrognos* prognos, char* json_buffer, size_t storlek) {
    char* ptr = json_buffer;         // Pekare som flyttas framåt i bufferten när vi skriver
    size_t kvarvarande = storlek;    // Håller reda på hur mycket plats som finns kvar

    // Skriv början på JSON-objektet med antal dagar
    int skrivet = snprintf(ptr, kvarvarande,
                           "{\n"
                           "  \"antal_dagar\": %d,\n"
                           "  \"dagar\": [\n",
                           prognos->antal_dagar);
    ptr += skrivet;          // Flytta pekaren framåt
    kvarvarande -= skrivet;  // Minska kvarvarande plats

    // Iterera genom alla dagar i prognosen (max 5 dagar)
    for (int i = 0; i < prognos->antal_dagar && i < 5; i++) {
        const VaderData* dag = &prognos->dagar[i];  // Pekare till aktuell dags väderdata

        // Skriv JSON-objekt för denna dag
        skrivet = snprintf(ptr, kvarvarande,
                          "    {\n"
                          "      \"stad\": \"%s\",\n"
                          "      \"land\": \"%s\",\n"
                          "      \"temperatur\": %.1f,\n"
                          "      \"luftfuktighet\": %.0f,\n"
                          "      \"vindhastighet\": %.1f,\n"
                          "      \"lufttryck\": %.0f,\n"
                          "      \"beskrivning\": \"%s\",\n"
                          "      \"ikon_id\": \"%s\",\n"
                          "      \"tidsstampel\": %lld\n"
                          "    }%s\n",                      // Komma efter alla utom sista objektet
                          dag->stad,
                          dag->land,
                          dag->temperatur,
                          dag->luftfuktighet,
                          dag->vindhastighet,
                          dag->lufttryck,
                          dag->beskrivning,
                          dag->ikon_id,
                          (long long)dag->tidsstampel,
                          (i < prognos->antal_dagar - 1) ? "," : "");  // Komma om inte sista
        ptr += skrivet;
        kvarvarande -= skrivet;
    }

    // Stäng JSON-strukturen (array och objekt)
    snprintf(ptr, kvarvarande, "  ]\n}");
}

/**
 * Skapar ett JSON-felmeddelande
 *
 * @param felkod - HTTP-felkod (400, 404, 500, etc.)
 * @param meddelande - Beskrivande felmeddelande
 * @param json_buffer - Buffert där JSON-felet ska skapas
 * @param storlek - Storlek på bufferten i bytes
 *
 * Funktionen skapar ett standardiserat JSON-felmeddelande som kan
 * skickas till klienter när något går fel. Detta gör felhanteringen
 * konsekvent och lätt att tolka för klienter.
 *
 * Exempel på output:
 * {
 *   "fel": true,
 *   "felkod": 404,
 *   "meddelande": "Endpoint hittades inte"
 * }
 */
void skapa_fel_json(int felkod, const char* meddelande, char* json_buffer, size_t storlek) {
    snprintf(json_buffer, storlek,
             "{\n"
             "  \"fel\": true,\n"                 // Boolean flagga som indikerar fel
             "  \"felkod\": %d,\n"                 // HTTP-statuskod
             "  \"meddelande\": \"%s\"\n"          // Beskrivande felmeddelande
             "}",
             felkod, meddelande);
}

/**
 * Hanterar en HTTP-klient som anslutit till servern
 *
 * @param klient_socket - Socket-descriptor för klientanslutningen
 * @param api_nyckel - OpenWeatherMap API-nyckel för att hämta väderdata
 *
 * Funktionen läser HTTP-förfrågan från klienten, avgör vilken endpoint som
 * efterfrågas (/weather eller /forecast), hämtar data (från cache eller API),
 * och skickar tillbaka ett HTTP-svar med JSON-data.
 *
 * Flöde:
 * 1. Ta emot HTTP-request från klient
 * 2. Parsa request för att få metod, sökväg och parametrar
 * 3. Kontrollera cache för data
 * 4. Om cache miss, hämta från OpenWeatherMap API
 * 5. Cacha ny data
 * 6. Skicka HTTP-svar med JSON till klient
 * 7. Stäng klientanslutningen
 */
void hantera_http_klient(socket_t klient_socket, const char* api_nyckel) {
    char buffer[BUFFER_STORLEK];  // Buffer för HTTP-request från klient
    char svar_buffer[8192];        // Buffer för att bygga HTTP-svar
    char json_buffer[4096];        // Buffer för JSON-data

    // Ta emot HTTP-request från klienten
    // recv() returnerar antal mottagna bytes, eller <= 0 vid fel/stängd anslutning
    int mottaget = recv(klient_socket, buffer, sizeof(buffer) - 1, 0);
    if (mottaget <= 0) {
        LOGG_VARNING("Mottog ingen data från klient");
        stang_socket(klient_socket);
        return;
    }
    buffer[mottaget] = '\0';  // Null-terminera för att göra det en giltig C-sträng

    // Parsa HTTP-requesten till en strukturerad form
    HttpRequest request;
    if (!parsa_http_request(buffer, &request)) {
        // Om parsningen misslyckas, skicka 400 Bad Request
        LOGG_VARNING("Ogiltig HTTP-request");
        skapa_fel_json(400, "Ogiltig HTTP-request", json_buffer, sizeof(json_buffer));
        skapa_http_response(svar_buffer, sizeof(svar_buffer), 400, json_buffer);
        send(klient_socket, svar_buffer, (int)strlen(svar_buffer), 0);
        stang_socket(klient_socket);
        return;
    }

    // Hantera /weather endpoint - Hämta aktuellt väder
    if (strcmp(request.sokvag, "/weather") == 0 && request.metod == HTTP_GET) {
        char stad[64] = {0};        // Buffer för stadens namn
        char landskod[3] = "SE";    // Standardvärde: Sverige

        // Extrahera 'city'-parametern från query-strängen (obligatorisk)
        if (!hamta_query_parameter(request.query, "city", stad, sizeof(stad))) {
            skapa_fel_json(400, "Parameter 'city' saknas", json_buffer, sizeof(json_buffer));
            skapa_http_response(svar_buffer, sizeof(svar_buffer), 400, json_buffer);
            send(klient_socket, svar_buffer, (int)strlen(svar_buffer), 0);
            stang_socket(klient_socket);
            return;
        }

        // Extrahera 'country'-parametern om den finns (valfri, standard SE)
        hamta_query_parameter(request.query, "country", landskod, sizeof(landskod));

        LOGG_INFO("HTTP GET /weather?city=%s&country=%s", stad, landskod);

        VaderData vader_data;
        bool lyckades = false;

        // Försök hämta från cache först (snabbare och sparar API-anrop)
        if (las_fran_cache(stad, landskod, &vader_data)) {
            lyckades = true;
            LOGG_INFO("Använder cachad data");
        } else {
            // Cache miss - hämta från OpenWeatherMap API
            if (hamta_aktuellt_vader(stad, landskod, api_nyckel, &vader_data)) {
                lyckades = true;
                // Spara i cache för framtida förfrågningar
                skriv_till_cache(stad, landskod, &vader_data);
            }
        }

        // Skapa HTTP-svar baserat på om vi lyckades hämta data
        if (lyckades) {
            // 200 OK med väderdata som JSON
            skapa_vader_json(&vader_data, json_buffer, sizeof(json_buffer));
            skapa_http_response(svar_buffer, sizeof(svar_buffer), 200, json_buffer);
        } else {
            // 500 Internal Server Error om API-anropet misslyckades
            skapa_fel_json(500, "Kunde inte hämta väderdata", json_buffer, sizeof(json_buffer));
            skapa_http_response(svar_buffer, sizeof(svar_buffer), 500, json_buffer);
        }

        send(klient_socket, svar_buffer, (int)strlen(svar_buffer), 0);

    // Hantera /forecast endpoint - Hämta väderprognos
    } else if (strcmp(request.sokvag, "/forecast") == 0 && request.metod == HTTP_GET) {
        char stad[64] = {0};
        char landskod[3] = "SE";

        // Extrahera 'city'-parametern (obligatorisk)
        if (!hamta_query_parameter(request.query, "city", stad, sizeof(stad))) {
            skapa_fel_json(400, "Parameter 'city' saknas", json_buffer, sizeof(json_buffer));
            skapa_http_response(svar_buffer, sizeof(svar_buffer), 400, json_buffer);
            send(klient_socket, svar_buffer, (int)strlen(svar_buffer), 0);
            stang_socket(klient_socket);
            return;
        }

        // Extrahera 'country'-parametern (valfri)
        hamta_query_parameter(request.query, "country", landskod, sizeof(landskod));

        LOGG_INFO("HTTP GET /forecast?city=%s&country=%s", stad, landskod);

        VaderPrognos prognos;
        bool lyckades = false;

        // Försök cache först
        if (las_prognos_fran_cache(stad, landskod, &prognos)) {
            lyckades = true;
        } else {
            // Cache miss - hämta från API
            if (hamta_vader_prognos(stad, landskod, api_nyckel, &prognos) > 0) {
                lyckades = true;
                skriv_prognos_till_cache(stad, landskod, &prognos);
            }
        }

        // Skapa HTTP-svar
        if (lyckades) {
            skapa_prognos_json(&prognos, json_buffer, sizeof(json_buffer));
            skapa_http_response(svar_buffer, sizeof(svar_buffer), 200, json_buffer);
        } else {
            skapa_fel_json(500, "Kunde inte hämta prognos", json_buffer, sizeof(json_buffer));
            skapa_http_response(svar_buffer, sizeof(svar_buffer), 500, json_buffer);
        }

        send(klient_socket, svar_buffer, (int)strlen(svar_buffer), 0);

    // Hantera root endpoint (/) - Visa API-dokumentation
    } else if (strcmp(request.sokvag, "/") == 0 && request.metod == HTTP_GET) {
        LOGG_INFO("HTTP GET / (API-dokumentation)");

        // Skapa ett välkomstmeddelande med tillgängliga endpoints
        snprintf(json_buffer, sizeof(json_buffer),
                 "{\n"
                 "  \"service\": \"Vädersystem API\",\n"
                 "  \"version\": \"1.0.0\",\n"
                 "  \"beskrivning\": \"HTTP/JSON väder-API med OpenWeatherMap integration\",\n"
                 "  \"endpoints\": [\n"
                 "    {\n"
                 "      \"metod\": \"GET\",\n"
                 "      \"sokvag\": \"/weather\",\n"
                 "      \"parametrar\": \"city (obligatorisk), country (valfri, standard: SE)\",\n"
                 "      \"exempel\": \"/weather?city=Stockholm&country=SE\",\n"
                 "      \"beskrivning\": \"Hämta aktuellt väder för en stad\"\n"
                 "    },\n"
                 "    {\n"
                 "      \"metod\": \"GET\",\n"
                 "      \"sokvag\": \"/forecast\",\n"
                 "      \"parametrar\": \"city (obligatorisk), country (valfri, standard: SE)\",\n"
                 "      \"exempel\": \"/forecast?city=Stockholm&country=SE\",\n"
                 "      \"beskrivning\": \"Hämta 5-dagars väderprognos för en stad\"\n"
                 "    }\n"
                 "  ],\n"
                 "  \"cache\": \"30 minuter TTL\",\n"
                 "  \"landskoder\": \"ISO 3166-1 alpha-2 (SE, GB, US, FR, etc.)\"\n"
                 "}");

        skapa_http_response(svar_buffer, sizeof(svar_buffer), 200, json_buffer);
        send(klient_socket, svar_buffer, (int)strlen(svar_buffer), 0);

    } else {
        // Okänd endpoint eller metod - skicka 404 Not Found med hjälpsam information
        LOGG_VARNING("Okänd endpoint: %s", request.sokvag);

        // Ge användaren en hint om tillgängliga endpoints
        snprintf(json_buffer, sizeof(json_buffer),
                 "{\n"
                 "  \"fel\": true,\n"
                 "  \"felkod\": 404,\n"
                 "  \"meddelande\": \"Endpoint hittades inte: %s\",\n"
                 "  \"tillgangliga_endpoints\": [\n"
                 "    \"GET /\",\n"
                 "    \"GET /weather?city=STAD&country=LANDSKOD\",\n"
                 "    \"GET /forecast?city=STAD&country=LANDSKOD\"\n"
                 "  ]\n"
                 "}",
                 request.sokvag);

        skapa_http_response(svar_buffer, sizeof(svar_buffer), 404, json_buffer);
        send(klient_socket, svar_buffer, (int)strlen(svar_buffer), 0);
    }

    // Stäng klientanslutningen när vi är klara
    stang_socket(klient_socket);
}

/**
 * Huvudfunktion - Programmets startpunkt
 *
 * @param argc - Antal kommandoradsargument
 * @param argv - Array med kommandoradsargument
 * @return 0 vid normal avslutning, 1 vid fel
 *
 * Funktionen initierar alla delsystem (loggning, cache, TCP-server),
 * startar huvudloopen som accepterar klienter och hanterar requests,
 * och stänger ned allt på ett snyggt sätt vid Ctrl+C.
 *
 * Kommandoradsargument:
 *   argv[1] - OpenWeatherMap API-nyckel (obligatorisk)
 *   argv[2] - Portnummer (valfritt, standard från konfiguration.h)
 *   argv[3] - Lognivå 0-3 (valfritt, 0=DEBUG, 1=INFO, 2=VARNING, 3=FEL)
 */
int main(int argc, char* argv[]) {
    // Kontrollera att API-nyckel har angetts
    if (argc < 2) {
        fprintf(stderr, "Användning: %s <OpenWeatherMap-API-nyckel> [port] [lognivå]\n", argv[0]);
        fprintf(stderr, "Lognivå: 0=DEBUG, 1=INFO, 2=VARNING, 3=FEL (standard: 1)\n");
        fprintf(stderr, "\nExempel:\n");
        fprintf(stderr, "  %s abc123xyz456\n", argv[0]);
        fprintf(stderr, "  %s abc123xyz456 8080 0\n", argv[0]);
        return 1;
    }

    // Läs in kommandoradsargument
    const char* api_nyckel = argv[1];  // Första argumentet är API-nyckeln
    int port = (argc > 2) ? atoi(argv[2]) : SERVER_PORT;  // Andra arg = port, annars standard
    LogNiva log_niva = (argc > 3) ? (LogNiva)atoi(argv[3]) : LOG_NIVA_INFO;  // Tredje arg = lognivå

    // Initialisera loggningssystemet med vald nivå
    initiera_loggning(log_niva);

    // Skriv ut en snygg välkomstbanner
    LOGG_INFO("╔═══════════════════════════════════════════════════════╗");
    LOGG_INFO("║         VÄDERSYSTEM SERVER - HTTP/JSON               ║");
    LOGG_INFO("║                  Version 1.0.0                        ║");
    LOGG_INFO("╚═══════════════════════════════════════════════════════╝");
    LOGG_INFO("Startar server på port %d", port);

    // Initialisera cache-systemet (skapar cache-katalog om den inte finns)
    if (!initiera_cache()) {
        LOGG_VARNING("Cache-initialisering misslyckades, fortsätter utan cache");
        // Vi fortsätter ändå - servern fungerar utan cache, bara långsammare
    }

    // Registrera signal-hanterare för att fånga Ctrl+C
    signal(SIGINT, signal_hanterare);   // SIGINT = Ctrl+C på alla plattformar
#ifndef _WIN32
    signal(SIGTERM, signal_hanterare);  // SIGTERM = avslutningssignal från systemet (Unix/Linux)
#endif

    // Initialisera TCP-server och börja lyssna på anslutningar
    TcpServer server;
    if (initiera_tcp_server(&server, port) != 0) {
        LOGG_FEL("Kunde inte starta TCP-server");
        stang_loggning();
        return 1;
    }

    // Skriv ut användbar information om servern
    LOGG_INFO("");
    LOGG_INFO("✓ Server lyssnar på http://localhost:%d", port);
    LOGG_INFO("✓ Endpoints:");
    LOGG_INFO("  GET /weather?city=Stockholm&country=SE");
    LOGG_INFO("  GET /forecast?city=Stockholm&country=SE");
    LOGG_INFO("");
    LOGG_INFO("Tryck Ctrl+C för att stoppa servern");
    LOGG_INFO("");

    // Huvudloop - Kör tills användaren trycker Ctrl+C
    int klient_raknare = 0;  // Räknare för antal hanterade klienter
    while (kors) {
        // Acceptera en ny klientanslutning (blockerar tills klient ansluter)
        socket_t klient = acceptera_klient(&server);

        if (klient != OGILTIG_SOCKET) {
            // Logga att en klient har anslutit
            LOGG_INFO("🔌 Ny klient anslöt (#%d)", klient_raknare + 1);

            // Hantera klientens HTTP-request och skicka svar
            hantera_http_klient(klient, api_nyckel);

            // Rensa gammal cache var 10:e klient för att hålla cache-katalogen fräsch
            // Detta förhindrar att gamla filer samlas och tar upp diskutrymme
            if (++klient_raknare % 10 == 0) {
                rensa_gammal_cache();
            }
        }

        // Kort paus för att inte spinna CPU:n för hårt
        // Detta ger också andra processer chans att köra
#ifdef _WIN32
        Sleep(10);              // Windows: Sleep tar millisekunder
#else
        usleep(10000);          // Unix/Linux: usleep tar mikrosekunder (10000 µs = 10 ms)
#endif
    }

    // Stäng ned servern på ett snyggt sätt
    stang_tcp_server(&server);
    LOGG_INFO("Server stoppad");
    stang_loggning();

    return 0;  // Framgångsrik avslutning
}
