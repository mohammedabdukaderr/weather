#include <stdio.h>      // Standard in-/utmatning (printf, scanf)
#include <stdlib.h>     // Minneshantering (malloc, realloc, free)
#include <string.h>     // Strängfunktioner (memcpy, snprintf)
#include <curl/curl.h>  // CURL-bibliotek för HTTP-anrop
#include "weather.h"    // Egna strukturer och konstanter (City, CITY_COUNT, URL_SIZE)
#include "json_parser.h"  // Funktion för att skriva ut väder från JSON

// Definierar alla städer och deras koordinater i en array
City cities[CITY_COUNT] = {
    {"Stockholm", 59.3293, 18.0686},
    {"Göteborg", 57.7089, 11.9746},
    {"Malmö", 55.6050, 13.0038},
    {"Uppsala", 59.8586, 17.6389},
    {"Västerås", 59.6099, 16.5448},
    {"Örebro", 59.2741, 15.2066},
    {"Linköping", 58.4109, 15.6216},
    {"Helsingborg", 56.0465, 12.6945},
    {"Jönköping", 57.7815, 14.1562},
    {"Norrköping", 58.5877, 16.1924},
    {"Lund", 55.7047, 13.1910},
    {"Gävle", 60.6749, 17.1413},
    {"Sundsvall", 62.3908, 17.3069},
    {"Umeå", 63.8258, 20.2630},
    {"Luleå", 65.5848, 22.1567},
    {"Kiruna", 67.8558, 20.2253}
};

// Skriver ut rubrik och listar alla städer i arrayen
int visa_vader()
{
    int antal_städer = sizeof(cities) / sizeof(cities[0]); // Beräknar antal städer i arrayen
    printf("\n=== VÄDERPROGRAM ===\n");                      // Skriver ut programrubrik
    for (int i = 0; i < antal_städer; i++)                  // Loopar igenom alla städer
    {
        printf("%2d. %s\n\n", i + 1, cities[i].name);       // Skriver ut stadens nummer och namn
    }
    return 0;                                               // Avslutar funktionen korrekt
}

// Struktur som används för att lagra data som hämtas med CURL
typedef struct {
    char *data;        // Pekare till buffert där data lagras
    size_t storlek;    // Hur mycket data som lagrats hittills
} Buffert;

// Callback-funktion som CURL anropar när data tas emot
size_t curl_callback(void *data, size_t size, size_t nmemb, void *userp) {
    size_t total = size * nmemb;            // Total mängd mottagen data i byte
    Buffert *buf = (Buffert*)userp;         // Pekare till vår buffertstruktur

    // Utökar bufferten för att få plats med ny data
    char *ny = realloc(buf->data, buf->storlek + total + 1);
    if (!ny) return 0;                       // Avbryt om minnesallokering misslyckas

    buf->data = ny;                          // Uppdaterar buffertpekaren
    memcpy(buf->data + buf->storlek, data, total); // Kopierar in ny data
    buf->storlek += total;                  // Uppdaterar buffertens storlek
    buf->data[buf->storlek] = '\0';          // Nullterminerar strängen

    return total;                            // Returnerar antal byte som hanterats
}

// Låter användaren välja stad via menyval
int valjstad(void) {
    int val;
    printf("Välj stad (1-%d): ", CITY_COUNT); // Visar giltigt intervall
    if (scanf("%d", &val) != 1) {              // Kontrollerar att inmatningen är ett tal
        while (getchar() != '\n');             // Tömmer inmatningsbufferten
        return 1;                              // Standardval: Stockholm
    }
    if (val < 1 || val > CITY_COUNT) {         // Kontrollerar att valet är giltigt
        printf("Ogiltigt val. Använder standard: Stockholm\n");
        return 1;
    }
    return val;                                // Returnerar användarens val
}

// Hämtar väderdata för vald stad från Open-Meteo API
void ta_vader(int choice) {
    int idx = choice - 1;                      // Omvandlar till 0-baserat index
    char url[URL_SIZE];                        // Buffert för API-URL

    // Skapar URL med vald stads latitud och longitud
    snprintf(url, sizeof(url),
             "https://api.open-meteo.com/v1/forecast?"
             "latitude=%.4f&longitude=%.4f&current_weather=true",
             cities[idx].lat, cities[idx].lon);

    CURL *curl = curl_easy_init();              // Initierar CURL
    if (!curl) {
        printf("Fel: Kan inte ansluta.\n");
        return;
    }

    Buffert buf = {NULL, 0};                    // Initierar buffert
    buf.data = malloc(1);                       // Allokerar startminne

    // Ställer in CURL-alternativ
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

    printf("Hämtar väderdata för %s...\n", cities[idx].name); // Statusmeddelande

    // Utför HTTP-anropet och skriver ut väder om det lyckas
    if (curl_easy_perform(curl) == CURLE_OK && buf.data) {
        skriv_ut_väder_json(buf.data);          // Tolkar och skriver ut JSON-data
    } else {
        printf("Kunde inte hämta data.\n");
    }

    curl_easy_cleanup(curl);                    // Städar upp CURL
    free(buf.data);                             // Frigör buffertminne
}

// Visar huvudmeny med alla städer
