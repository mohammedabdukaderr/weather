#include <stdio.h>          // Inkluderar standard I/O-funktioner
#include <stdlib.h>         // Inkluderar minneshanteringsfunktioner
#include <string.h>         // Inkluderar stränghanteringsfunktioner
#include <curl/curl.h>      // Inkluderar CURL-biblioteket för internet
#include <time.h>           // Inkluderar tidsfunktioner
#include "weather.h"        // Inkluderar vår egen header-fil

City cities[CITY_COUNT] = { // Definierar och initierar array med städer
    {"Stockholm", 59.3293, 18.0686},   // Initierar första staden
    {"Göteborg", 57.7089, 11.9746},    // Initierar andra staden
    {"Malmö", 55.6050, 13.0038},       // Initierar tredje staden
    {"Uppsala", 59.8586, 17.6389},     // Initierar fjärde staden
    {"Västerås", 59.6099, 16.5448},    // Initierar femte staden
    {"Örebro", 59.2741, 15.2066},      // Initierar sjätte staden
    {"Linköping", 58.4109, 15.6216},   // Initierar sjunde staden
    {"Helsingborg", 56.0465, 12.6945}, // Initierar åttonde staden
    {"Jönköping", 57.7815, 14.1562},   // Initierar nionde staden
    {"Norrköping", 58.5877, 16.1924},  // Initierar tionde staden
    {"Lund", 55.7047, 13.1910},        // Initierar elfte staden
    {"Gävle", 60.6749, 17.1413},       // Initierar tolfte staden
    {"Sundsvall", 62.3908, 17.3069},   // Initierar trettonde staden
    {"Umeå", 63.8258, 20.2630},        // Initierar fjortonde staden
    {"Luleå", 65.5848, 22.1567},       // Initierar femtonde staden
    {"Kiruna", 67.8558, 20.2253}       // Initierar sextonde staden
};

size_t curl_callback(void *data, size_t size, size_t nmemb, void *userp) {  // CURL callback-funktion
    size_t total = size * nmemb;          // Beräknar total storlek på inkommande data
    MemBuffer *buf = (MemBuffer*)userp;   // Konverterar userp till vår MemBuffer-struct
    
    char *new = realloc(buf->data, buf->size + total + 1);  // Allokerar mer minne på heapen
    if (!new) return 0;                   // Returnerar 0 om minnesallokering misslyckades
    
    buf->data = new;                      // Uppdaterar pekaren till det nya minnet
    memcpy(buf->data + buf->size, data, total);  // Kopierar ny data till slutet av gamla
    buf->size += total;                   // Uppdaterar total storlek
    buf->data[buf->size] = '\0';          // Lägger till null-terminator
    
    return total;                         // Returnerar antal hanterade bytes till CURL
}

void show_menu(void) {                    // Funktion för att visa huvudmenyn
    printf("\n=== VÄDERPROGRAM ===\n");   // Skriver ut programtiteln
    for (int i = 0; i < CITY_COUNT; i++) {  // Loopar genom alla städer
        printf("%2d. %s\n", i+1, cities[i].name);  // Skriver ut nummer och stad
    }
}

int get_choice(void) {                    // Funktion för att läsa användarens val
    int choice;                           // Variabel för att lagra valet
    printf("\nVälj stad (1-%d): ", CITY_COUNT);  // Ber användaren välja stad
    scanf("%d", &choice);                 // Läser ett heltal från tangentbordet
    getchar();                            // Rensar newline-tecken från bufferten
    
    if (choice < 1 || choice > CITY_COUNT) {  // Kontrollerar om valet är giltigt
        printf("Ogiltigt val, använder 1.\n");  // Meddelar om ogiltigt val
        return 1;                           // Returnerar standardvärde (Stockholm)
    }
    return choice;                         // Returnerar det giltiga valet
}

int check_cache(int city_idx, Cache *cache) {  // Funktion för att kontrollera cache
    FILE *f = fopen(CACHE_FILE, "r");      // Öppnar cache-filen för läsning
    if (!f) return 0;                      // Returnerar 0 om filen inte finns
    
    fscanf(f, "%s %ld %[^\n]", cache->city, &cache->time, cache->json);  // Läser cache-data
    fclose(f);                             // Stänger filen
    
    if (strcmp(cache->city, cities[city_idx].name) != 0) return 0;  // Kollar om rätt stad
    if (time(NULL) - cache->time > CACHE_MAX_AGE) return 0;  // Kollar om cache är för gammal
    
    printf("Hittade cachad data (%.0f min gammal). Använda? (j/n): ",  // Frågar användaren
           (time(NULL) - cache->time) / 60.0);  // Visar cache-ålder i minuter
    
    char answer;                           // Variabel för användarens svar
    scanf("%c", &answer);                  // Läser ett tecken från tangentbordet
    getchar();                             // Rensar bufferten
    return (answer == 'j' || answer == 'J');  // Returnerar 1 om ja, annars 0
}

void save_cache(int city_idx, const char *json) {  // Funktion för att spara cache
    FILE *f = fopen(CACHE_FILE, "w");      // Öppnar cache-filen för skrivning
    if (!f) return;                        // Avbryt om filen inte kunde öppnas
    fprintf(f, "%s %ld %s\n", cities[city_idx].name, time(NULL), json);  // Skriver cache
    fclose(f);                             // Stänger filen
}

void get_weather(int choice) {             // Huvudfunktion för att hämta väder
    int idx = choice - 1;                  // Konverterar val (1-16) till index (0-15)
    Cache cache;                           // Skapar Cache-struct för cachad data
    
    if (check_cache(idx, &cache)) {        // Kontrollerar om cachad data finns och är giltig
        printf("\n=== Väder för %s ===\n", cache.city);  // Skriver ut rubrik
        printf("%s\n", cache.json);        // Skriver ut cachad JSON-data
        return;                            // Avslutar funktionen (använder cache)
    }
    
    char url[URL_SIZE];                    // Array för att bygga URL
    snprintf(url, sizeof(url),             // Bygger API-URL med koordinater
             "https://api.open-meteo.com/v1/forecast?"
             "latitude=%.4f&longitude=%.4f&current_weather=true",
             cities[idx].lat, cities[idx].lon);
    
    CURL *curl = curl_easy_init();         // Initierar CURL-session
    if (!curl) {                           // Kontrollerar om CURL initierades
        printf("Fel: Kan inte ansluta.\n");  // Skriver felmeddelande
        return;                            // Avslutar funktionen
    }
    
    MemBuffer buf = {NULL, 0};             // Initierar MemBuffer med NULL och storlek 0
    buf.data = malloc(1);                  // Allokerar 1 byte initialt minne
    
    curl_easy_setopt(curl, CURLOPT_URL, url);  // Sätter URL för CURL
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback);  // Sätter callback-funktion
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);  // Skickar pekare till vår buffer
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);  // Sätter timeout till 5 sekunder
    
    printf("Hämtar väder för %s...\n", cities[idx].name);  // Informerar användaren
    
    if (curl_easy_perform(curl) == CURLE_OK && buf.data) {  // Utför HTTP-anrop och kontrollerar resultat
        printf("\n=== Väder för %s ===\n", cities[idx].name);  // Skriver ut rubrik
        printf("%s\n", buf.data);           // Skriver ut JSON-data från API
        save_cache(idx, buf.data);          // Sparar datan i cache för framtiden
    } else {
        printf("Kunde inte hämta data.\n");  // Meddelar om misslyckat anrop
    }
    
    curl_easy_cleanup(curl);                // Städar upp CURL-resurser
    free(buf.data);                         // Frigör minnet på heapen
}