#include "vader_api.h"              // Egna funktioner för väder-API
#include "json_helper.h"            // För att parsa JSON-svar från API
#include "loggning.h"                // För att logga debug-meddelanden och varningar
#include "konfiguration.h"           // För API_HOST, API_PORT, API_ENDPOINT, etc.
#include "natverks_abstraktion.h"    // För plattformsoberoende nätverksfunktioner
#include <string.h>                  // För strängfunktioner: strlen, strstr, memcpy, memmove, memset
#include <time.h>                    // För time() - tidsstämplar
#include <stdio.h>                   // För snprintf - formatera strängar

/**
 * Skickar en HTTP GET-förfrågan och tar emot svaret
 *
 * @param host - Värdnamnet att ansluta till (t.ex. "api.openweathermap.org")
 * @param port - Portnummer att ansluta till (vanligtvis 80 för HTTP)
 * @param path - URL-sökväg inklusive query-parametrar (t.ex. "/data/2.5/weather?q=Stockholm")
 * @param svar_buffer - Buffert där HTTP-svaret ska lagras
 * @param buffer_storlek - Storlek på svar-bufferten i bytes
 * @return true om förfrågan lyckades, false vid fel
 *
 * Funktionen etablerar en TCP-anslutning, skickar en HTTP GET-förfrågan,
 * tar emot svaret och extraherar JSON-kroppen. Detta är hjärtat i vår
 * kommunikation med OpenWeatherMap API.
 */
static bool skicka_http_get(const char* host, int port, const char* path,
                             char* svar_buffer, size_t buffer_storlek) {
    // Skapa en socket för nätverkskommunikation
    // AF_INET = IPv4, SOCK_STREAM = TCP-anslutning (tillförlitlig, strömbaserad)
    socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == OGILTIG_SOCKET) {
        LOGG_FEL("Kunde inte skapa socket för HTTP-förfrågan");
        return false;
    }

    // Slå upp värdnamnet (DNS-lookup) för att få IP-adressen
    // gethostbyname returnerar en hostent-struktur med IP-adressinformation
    struct hostent* server = gethostbyname(host);
    if (!server) {
        LOGG_FEL("Kunde inte hitta värd: %s", host);
        stang_socket(sock);  // Stäng socketen innan vi returnerar
        return false;
    }

    // Konfigurera serveradressen för anslutningen
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));  // Nollställ hela strukturen
    server_addr.sin_family = AF_INET;              // IPv4-adressfamilj

    // Kopiera IP-adressen från DNS-svaret till vår adress-struktur
    // h_addr pekar på den första IP-adressen i listan
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, (size_t)server->h_length);

    // Sätt portnummer (konvertera från host byte order till network byte order)
    server_addr.sin_port = htons((uint16_t)port);

    // Försök ansluta till servern
    // connect() etablerar en TCP-anslutning till den angivna adressen
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        LOGG_FEL("Kunde inte ansluta till %s:%d", host, port);
        stang_socket(sock);
        return false;
    }

    // Bygg HTTP GET-förfrågan enligt HTTP/1.1-protokollet
    // Format: METOD SÖKVÄG VERSION\r\nHEADERS\r\n\r\n
    char forfragan[1024];
    snprintf(forfragan, sizeof(forfragan),
             "GET %s HTTP/1.1\r\n"          // Förfrågansrad: metod, sökväg, version
             "Host: %s\r\n"                  // Host-header (obligatorisk i HTTP/1.1)
             "Connection: close\r\n"         // Be servern stänga anslutningen efter svar
             "\r\n",                         // Tom rad markerar slutet på headers
             path, host);

    // Skicka HTTP-förfrågan till servern
    // send() returnerar antal bytes som skickades, eller negativt värde vid fel
    if (send(sock, forfragan, (int)strlen(forfragan), 0) < 0) {
        LOGG_FEL("Kunde inte skicka HTTP-förfrågan");
        stang_socket(sock);
        return false;
    }

    // Ta emot HTTP-svaret från servern
    // Vi tar emot data i en loop eftersom svaret kan komma i flera paket
    int totalt_mottaget = 0;  // Räknare för totalt antal mottagna bytes
    int mottaget;              // Antal bytes mottagna i varje recv()-anrop

    // Loop tills servern stänger anslutningen (recv returnerar 0) eller bufferten är full
    while ((mottaget = recv(sock, svar_buffer + totalt_mottaget,
                            (int)(buffer_storlek - totalt_mottaget - 1), 0)) > 0) {
        totalt_mottaget += mottaget;  // Summera totalt mottagna bytes

        // Kontrollera om bufferten är nästan full (lämna plats för null-terminator)
        if ((size_t)totalt_mottaget >= buffer_storlek - 1) break;
    }

    // Lägg till null-terminator för att göra det en giltig C-sträng
    svar_buffer[totalt_mottaget] = '\0';

    // Stäng socket-anslutningen, vi är klara
    stang_socket(sock);

    // Extrahera JSON-kroppen från HTTP-svaret
    // HTTP-svaret består av: statusrad + headers + tom rad + kropp
    // Vi letar efter "\r\n\r\n" som markerar slutet på headers
    char* kropp = strstr(svar_buffer, "\r\n\r\n");
    if (kropp) {
        kropp += 4;  // Hoppa över "\r\n\r\n" (4 tecken)

        // Flytta JSON-kroppen till början av bufferten
        // Detta gör det enklare att parsa JSON utan att behöva hålla reda på offset
        memmove(svar_buffer, kropp, strlen(kropp) + 1);  // +1 för null-terminator
    }

    // Returnera true om vi fick något svar (även om det kan vara ett felmeddelande)
    return totalt_mottaget > 0;
}

/**
 * Hämtar aktuellt väder från OpenWeatherMap API
 *
 * @param stad - Stadens namn (t.ex. "Stockholm")
 * @param landskod - Landskod (t.ex. "SE" för Sverige)
 * @param api_nyckel - Din OpenWeatherMap API-nyckel
 * @param resultat - Pekare till VaderData-struktur där resultatet ska lagras
 * @return true om väderdata hämtades och parsades, false vid fel
 *
 * Funktionen bygger en API-URL med de angivna parametrarna och hämtar
 * aktuellt väder från OpenWeatherMap. Den returnerar data i metriska enheter
 * (Celsius, meter/sekund) och med svenska beskrivningar.
 */
bool hamta_aktuellt_vader(const char* stad, const char* landskod,
                          const char* api_nyckel, VaderData* resultat) {
    LOGG_INFO("Hämtar väder för %s, %s från OpenWeatherMap", stad, landskod);

    // Bygg API-URL med alla nödvändiga parametrar
    // q = query (stad,landskod), appid = API-nyckel, units = metriska enheter, lang = språk
    char url[512];
    snprintf(url, sizeof(url),
             "%s?q=%s,%s&appid=%s&units=metric&lang=sv",
             API_ENDPOINT,  // Basvägen, t.ex. "/data/2.5/weather"
             stad,          // Stadens namn
             landskod,      // Landskod (ISO 3166)
             api_nyckel);   // Din API-nyckel från OpenWeatherMap

    // Skicka HTTP-förfrågan till OpenWeatherMap och ta emot JSON-svaret
    char svar[8192];  // Buffer för att lagra API-svaret (behöver vara tillräckligt stor för JSON)
    if (!skicka_http_get(API_HOST, API_PORT, url, svar, sizeof(svar))) {
        LOGG_FEL("Kunde inte hämta väderdata från API");
        return false;
    }

    // Parsa JSON-svaret och fyll i resultat-strukturen
    return parsa_vader_json(svar, resultat);
}

/**
 * Parsar JSON-data från OpenWeatherMap current weather API
 *
 * @param json_data - JSON-strängen att parsa
 * @param resultat - Pekare till VaderData-struktur där resultatet ska lagras
 * @return true om parsningen lyckades, false vid fel
 *
 * Funktionen extraherar relevanta väderfält från OpenWeatherMap:s JSON-struktur.
 * JSON-strukturen ser ut ungefär så här:
 * {
 *   "name": "Stockholm",
 *   "main": {"temp": 15.5, "humidity": 65, "pressure": 1013},
 *   "wind": {"speed": 3.5},
 *   "weather": [{"description": "lätt regn", "icon": "10d"}]
 * }
 */
bool parsa_vader_json(const char* json_data, VaderData* resultat) {
    LOGG_DEBUG("Parsar väder-JSON");

    // Kontrollera efter felmeddelanden från API:et
    // OpenWeatherMap returnerar "cod":"404" om staden inte hittades
    // Både string-format ("404") och number-format (404) kan förekomma
    if (strstr(json_data, "\"cod\":\"404\"") || strstr(json_data, "\"cod\":404")) {
        LOGG_VARNING("Stad inte hittad i API-svar");
        return false;
    }

    // Hämta stadens namn från JSON
    // "name" är en top-level nyckel som innehåller stadens officiella namn
    if (!json_hamta_strang(json_data, "name", resultat->stad, sizeof(resultat->stad))) {
        LOGG_VARNING("Kunde inte hitta stadnamn i JSON");
        return false;
    }

    // Hitta "main"-objektet som innehåller temperatur, luftfuktighet och tryck
    // Vi söker efter "main":{ och parserar innehållet i detta objekt
    const char* main_obj = strstr(json_data, "\"main\":{");
    if (main_obj) {
        // Hämta temperatur i Celsius (returneras som double, konverteras till float)
        resultat->temperatur = (float)json_hamta_nummer(main_obj, "temp");

        // Hämta luftfuktighet i procent (0-100)
        resultat->luftfuktighet = (float)json_hamta_nummer(main_obj, "humidity");

        // Hämta lufttryck i hPa (hektopascal, samma som millibar)
        resultat->lufttryck = (float)json_hamta_nummer(main_obj, "pressure");
    }

    // Hitta "wind"-objektet som innehåller vindinformation
    const char* wind_obj = strstr(json_data, "\"wind\":{");
    if (wind_obj) {
        // Hämta vindhastighet i meter per sekund (m/s)
        resultat->vindhastighet = (float)json_hamta_nummer(wind_obj, "speed");
    }

    // Hitta väderbeskrivning från "weather"-arrayen
    // "weather" är en array med väderförhållanden (vanligtvis bara ett element)
    // Vi hämtar det första objektet i arrayen
    const char* weather_array = json_hamta_forsta_array_objekt(json_data, "weather");
    if (weather_array) {
        // Hämta textbeskrivning på svenska (t.ex. "lätt regn", "klart", "molnigt")
        json_hamta_strang(weather_array, "description",
                         resultat->beskrivning, sizeof(resultat->beskrivning));

        // Hämta ikon-ID som beskriver väderförhållandena (t.ex. "01d", "10n")
        // Detta kan användas för att visa en passande väderikon
        // Formatet är: nummer + d/n (day/night), t.ex. "01d" = klar himmel, dag
        json_hamta_strang(weather_array, "icon",
                         resultat->ikon_id, sizeof(resultat->ikon_id));
    }

    // Sätt tidsstämpel till nuvarande tid
    // Detta används för att avgöra när cache-data blir för gammal
    resultat->tidsstampel = time(NULL);

    // Logga framgångsrik parsning med viktig information
    LOGG_INFO("Parsade väder: %s, %.1f°C, %s",
              resultat->stad, resultat->temperatur, resultat->beskrivning);
    return true;
}

/**
 * Hämtar väderprognos från OpenWeatherMap API
 *
 * @param stad - Stadens namn (t.ex. "Stockholm")
 * @param landskod - Landskod (t.ex. "SE" för Sverige)
 * @param api_nyckel - Din OpenWeatherMap API-nyckel
 * @param resultat - Pekare till VaderPrognos-struktur där resultatet ska lagras
 * @return Antal dagar i prognosen, eller 0 vid fel
 *
 * Funktionen hämtar 5-dagarsprognos från OpenWeatherMap med data för var 3:e timme.
 * API:et returnerar upp till 40 datapunkter (5 dagar * 8 datapunkter per dag).
 */
int hamta_vader_prognos(const char* stad, const char* landskod,
                        const char* api_nyckel, VaderPrognos* resultat) {
    LOGG_INFO("Hämtar prognos för %s, %s", stad, landskod);

    // Bygg API-URL för 5-dagarsprognosen
    // cnt=40 begär maximalt antal datapunkter (5 dagar * 8 per dag = 40)
    char url[512];
    snprintf(url, sizeof(url),
             "%s?q=%s,%s&appid=%s&units=metric&lang=sv&cnt=40",
             API_FORECAST_ENDPOINT,  // Basvägen för prognoser, t.ex. "/data/2.5/forecast"
             stad,
             landskod,
             api_nyckel);

    // Större buffer behövs för prognosdata eftersom JSON-svaret är mycket större
    // Prognos-JSON innehåller 40 objekt med väderdata istället för bara ett
    char svar[16384];  // 16 KB buffer
    if (!skicka_http_get(API_HOST, API_PORT, url, svar, sizeof(svar))) {
        LOGG_FEL("Kunde inte hämta prognos från API");
        return 0;  // Returnera 0 dagar vid fel
    }

    // Parsa JSON-svaret och returnera antal dagar som parsades
    return parsa_prognos_json(svar, resultat);
}

/**
 * Parsar prognosdata från OpenWeatherMap forecast API
 *
 * @param json_data - JSON-strängen att parsa
 * @param resultat - Pekare till VaderPrognos-struktur där resultatet ska lagras
 * @return Antal dagar i prognosen, eller 0 vid fel
 *
 * OBS: Detta är en förenklad implementation som endast tar det första objektet.
 * En fullständig implementation skulle parsa alla 40 datapunkter och gruppera
 * dem per dag, eller ta var 8:e datapunkt för att få en datapunkt per dag.
 *
 * JSON-strukturen för prognos:
 * {
 *   "list": [
 *     { "dt": 1234567890, "main": {...}, "weather": [...], ... },
 *     { "dt": 1234578790, "main": {...}, "weather": [...], ... },
 *     ...
 *   ],
 *   "city": {"name": "Stockholm", ...}
 * }
 */
int parsa_prognos_json(const char* json_data, VaderPrognos* resultat) {
    LOGG_DEBUG("Parsar prognos-JSON");

    // Nollställ resultat-strukturen för att undvika skräpdata
    memset(resultat, 0, sizeof(VaderPrognos));

    // OpenWeatherMap ger 3-timmars intervaller i prognosen
    // För att få en datapunkt per dag skulle vi behöva ta var 8:e post (8 * 3h = 24h)
    // Detta är en förenklad implementation som bara tar första posten

    // Hitta "list"-arrayen som innehåller alla prognosdatapunkter
    const char* list_start = strstr(json_data, "\"list\":[");
    if (!list_start) {
        LOGG_VARNING("Kunde inte hitta prognoslista i JSON");
        return 0;
    }

    // För enkelhetens skull använder vi bara första väderposten
    // En fullständig implementation skulle iterera genom alla 40 poster
    // och antingen gruppera dem per dag eller ta var 8:e post
    const char* forsta_post = strchr(list_start, '{');  // Hitta första objektet efter '['
    if (forsta_post) {
        // Återanvänd väder-parsningsfunktionen för att parsa första datapunkten
        // Prognosdata har samma struktur som aktuellt väder
        parsa_vader_json(forsta_post, &resultat->dagar[0]);
        resultat->antal_dagar = 1;  // Vi har bara en dag i denna förenklade version

        // Hämta stadens namn från "city"-objektet istället för "name"
        // I prognos-JSON ligger stadinformationen i ett separat "city"-objekt
        const char* city_obj = strstr(json_data, "\"city\":{");
        if (city_obj) {
            json_hamta_strang(city_obj, "name",
                             resultat->dagar[0].stad,
                             sizeof(resultat->dagar[0].stad));
        }
    }

    LOGG_INFO("Parsade %d dagars prognos", resultat->antal_dagar);
    return resultat->antal_dagar;
}
