#ifndef WEATHER_H           // Förhindrar att samma kod inkluderas flera gånger
#define WEATHER_H           // Markerar att denna header nu är definierad

#include <stddef.h>         // Inkluderar size_t för minnesstorlekar
#include <time.h>           // Inkluderar time_t för tidshantering


#define CITY_COUNT 16       // Antal städer som finns i programmet
#define CITY_NAME_LEN 30    // Max antal tecken för ett stadsnamn
#define URL_SIZE 200        // Max storlek på URL-sträng
#define CACHE_FILE "cache.txt"  // Filnamn där cache sparas
#define CACHE_MAX_AGE 3600  // Max ålder på cache i sekunder (1 timme)
#define JSON_SIZE 2048      // Max storlek på JSON-data



typedef struct {
    char name[CITY_NAME_LEN];  // Stadens namn
    double lat;                // Latitud
    double lon;                // Longitud
} City;


typedef struct { // Struct för buffert som lagrar CURL-data
    char *data;       // Pekare till minne på heapen
    size_t size;      // Storlek på data
} MemBuffer;


typedef struct { //// Struct för fångad väderdata
    char city[CITY_NAME_LEN];  // Stadens namn
    char json[JSON_SIZE];      // JSON-data
    time_t time;               // Tidsstämpel när datan hämtades
} Cache;



int visa_vader();  // Visar huvudmenyn med alla städer


int valjstad(void); // Låter användaren välja stad


void ta_vader(int choice); //  Hämtar väderdata för vald stad


size_t curl_callback(void *data, size_t size, size_t nmemb, void *userp); // Callback-funktion för CURL (för att läsa inkommande data)

#endif // WEATHER_H