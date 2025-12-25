#include "http_server.h"   // Egna funktioner för HTTP-hantering
#include "loggning.h"       // För att logga debug-meddelanden och varningar
#include <string.h>         // För strängfunktioner: strcmp, strchr, strstr, strlen, strncpy, memcpy, memset
#include <stdio.h>          // För sscanf och snprintf

/**
 * Parsar en HTTP-förfrågan från rå textdata
 *
 * @param raadata - Den råa HTTP-texten som kom från klienten (t.ex. "GET /weather HTTP/1.1\r\n...")
 * @param forfragan - Pekare till HttpRequest-struktur där resultatet ska sparas
 * @return true om parsningen lyckades, false vid fel
 *
 * Funktionen analyserar HTTP-förfrågan och extraherar:
 * - Metod (GET, POST, etc.)
 * - Sökväg (t.ex. "/weather")
 * - Query-parametrar (t.ex. "city=Stockholm&country=SE")
 * - Body-data (för POST-förfrågningar)
 */
bool parsa_http_request(const char* raadata, HttpRequest* forfragan) {
    // Nollställ hela request-strukturen för att undvika skräpdata
    memset(forfragan, 0, sizeof(HttpRequest));

    // Buffrar för att temporärt lagra metod och URL under parsning
    char metod_strang[16];   // För "GET", "POST", etc. (max 15 tecken + nullterminator)
    char url[512];           // För hela URL:en (sökväg + eventuella query-parametrar)

    // Parsa första raden i HTTP-requesten: "METOD URL HTTP/VERSION"
    // Exempel: "GET /weather?city=Stockholm HTTP/1.1"
    // %15s läser max 15 tecken för metoden, %511s läser max 511 tecken för URL
    if (sscanf(raadata, "%15s %511s", metod_strang, url) != 2) {
        // Om vi inte kunde läsa både metod och URL är requesten ogiltig
        LOGG_VARNING("Kunde inte parsa HTTP request-rad");
        return false;
    }

    // Identifiera HTTP-metoden genom att jämföra strängen
    if (strcmp(metod_strang, "GET") == 0) {
        forfragan->metod = HTTP_GET;              // GET-förfrågan (hämta data)
    } else if (strcmp(metod_strang, "POST") == 0) {
        forfragan->metod = HTTP_POST;             // POST-förfrågan (skicka data)
    } else {
        forfragan->metod = HTTP_UNKNOWN;          // Okänd metod (PUT, DELETE, etc.)
        return false;
    }

    // Dela upp URL:en i sökväg och query-parametrar
    // Exempel: "/weather?city=Stockholm" delas till "/weather" och "city=Stockholm"
    char* query_start = strchr(url, '?');         // Leta efter '?' som markerar start på query

    if (query_start) {
        // URL:en innehåller query-parametrar (allt efter '?')

        // Beräkna längden på sökvägen (allt före '?')
        size_t sokvag_langd = (size_t)(query_start - url);

        // Säkerställ att sökvägen inte är längre än bufferten kan hantera
        if (sokvag_langd >= sizeof(forfragan->sokvag)) {
            sokvag_langd = sizeof(forfragan->sokvag) - 1;  // Lämna plats för nullterminator
        }

        // Kopiera sökvägen till request-strukturen
        memcpy(forfragan->sokvag, url, sokvag_langd);
        forfragan->sokvag[sokvag_langd] = '\0';   // Lägg till nullterminator

        // Kopiera query-parametrarna (hoppa över '?'-tecknet)
        strncpy(forfragan->query, query_start + 1, sizeof(forfragan->query) - 1);
        forfragan->query[sizeof(forfragan->query) - 1] = '\0';  // Säkerställ nullterminering
    } else {
        // URL:en har inga query-parametrar, bara en sökväg
        strncpy(forfragan->sokvag, url, sizeof(forfragan->sokvag) - 1);
        forfragan->sokvag[sizeof(forfragan->sokvag) - 1] = '\0';
    }

    // För POST-förfrågningar, extrahera body-datan (data som skickades med requesten)
    if (forfragan->metod == HTTP_POST) {
        // HTTP-headers avslutas med "\r\n\r\n", efter det kommer body
        const char* body_start = strstr(raadata, "\r\n\r\n");

        if (body_start) {
            body_start += 4;  // Hoppa över "\r\n\r\n" (4 tecken)

            // Kopiera body-datan till request-strukturen
            strncpy(forfragan->body, body_start, sizeof(forfragan->body) - 1);
            forfragan->body[sizeof(forfragan->body) - 1] = '\0';
        }
    }

    // Logga den parsade requesten för debugging
    LOGG_DEBUG("Parsad HTTP %s %s (query: %s)",
               metod_strang, forfragan->sokvag, forfragan->query);

    return true;  // Parsningen lyckades
}

/**
 * Skapar ett komplett HTTP-svar med JSON-data
 *
 * @param buffer - Buffert där HTTP-svaret ska skrivas
 * @param buffer_storlek - Storlek på bufferten i bytes
 * @param statuskod - HTTP-statuskod (200 = OK, 404 = Not Found, 500 = Server Error, etc.)
 * @param json_data - JSON-strängen som ska skickas i body (kan vara NULL)
 *
 * Funktionen bygger ett komplett HTTP-svar med headers och body.
 * Exempel på genererat svar:
 * HTTP/1.1 200 OK
 * Content-Type: application/json; charset=utf-8
 * Content-Length: 42
 * Connection: close
 * Server: Vaderserver/1.0
 *
 * {"stad":"Stockholm","temperatur":15.5}
 */
void skapa_http_response(char* buffer, size_t buffer_storlek,
                         int statuskod, const char* json_data) {
    // Välj lämplig statustext baserat på statuskoden
    const char* status_text;
    switch (statuskod) {
        case 200: status_text = "OK"; break;                          // Allt gick bra
        case 400: status_text = "Bad Request"; break;                 // Klienten skickade ogiltig förfrågan
        case 404: status_text = "Not Found"; break;                   // Resursen finns inte
        case 500: status_text = "Internal Server Error"; break;       // Serverfel
        default: status_text = "Unknown"; break;                      // Okänd statuskod
    }

    // Beräkna längden på JSON-datan (0 om ingen data finns)
    size_t json_langd = json_data ? strlen(json_data) : 0;

    // Bygg det kompletta HTTP-svaret med alla headers
    snprintf(buffer, buffer_storlek,
             "HTTP/1.1 %d %s\r\n"                                     // Statusrad (t.ex. "HTTP/1.1 200 OK")
             "Content-Type: application/json; charset=utf-8\r\n"      // Typ av innehåll (JSON med UTF-8)
             "Content-Length: %zu\r\n"                                // Längd på body i bytes
             "Connection: close\r\n"                                  // Stäng anslutning efter svar
             "Server: Vaderserver/1.0\r\n"                           // Serveridentifikation
             "\r\n"                                                   // Tom rad markerar slut på headers
             "%s",                                                    // JSON-data (body)
             statuskod, status_text,
             json_langd,
             json_data ? json_data : "");  // Använd tom sträng om json_data är NULL
}

/**
 * Hämtar värdet av en specifik query-parameter från en query-sträng
 *
 * @param query - Query-strängen (t.ex. "city=Stockholm&country=SE")
 * @param parameter_namn - Namnet på parametern att hämta (t.ex. "city")
 * @param varde - Buffert där parameterns värde ska sparas
 * @param varde_storlek - Storlek på värde-bufferten
 * @return true om parametern hittades, false annars
 *
 * Exempel: Om query = "city=Stockholm&country=SE" och parameter_namn = "city"
 * då kommer varde att sättas till "Stockholm"
 */
bool hamta_query_parameter(const char* query, const char* parameter_namn,
                           char* varde, size_t varde_storlek) {
    // Säkerhetscheck: Om någon pekare är NULL, returnera false
    if (!query || !parameter_namn || !varde) {
        return false;
    }

    // Bygg söksträng i formatet "parameter=" (t.ex. "city=")
    char sokstrang[128];
    snprintf(sokstrang, sizeof(sokstrang), "%s=", parameter_namn);

    // Leta efter denna söksträng i query-strängen
    const char* start = strstr(query, sokstrang);

    // Om parametern inte finns i query-strängen
    if (!start) {
        return false;
    }

    // Hoppa förbi "parameter=" för att komma till värdet
    start += strlen(sokstrang);

    // Hitta slutet på värdet (antingen '&' för nästa parameter, eller slutet av strängen)
    const char* slut = strchr(start, '&');

    // Beräkna längden på värdet
    size_t langd;
    if (slut) {
        // Värdet avslutas med '&' (det finns fler parametrar efter)
        langd = (size_t)(slut - start);
    } else {
        // Värdet går till slutet av strängen (sista parametern)
        langd = strlen(start);
    }

    // Säkerställ att värdet inte är längre än bufferten kan hantera
    if (langd >= varde_storlek) {
        langd = varde_storlek - 1;  // Lämna plats för nullterminator
    }

    // Kopiera värdet till output-bufferten
    memcpy(varde, start, langd);
    varde[langd] = '\0';  // Lägg till nullterminator

    return true;  // Parametern hittades och kopierades
}
