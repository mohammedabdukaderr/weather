#ifndef WEATHER_H           // Förhindrar att samma kod inkluderas flera gånger
#define WEATHER_H           // Markerar att denna header nu är definierad

#include <stddef.h>         // Inkluderar size_t för minnesstorlekar
#include <time.h>           // Inkluderar time_t för tidshantering

#define CITY_COUNT 16       // Definierar konstant för antal städer (16 stycken)
#define CITY_NAME_LEN 30    // Definierar max längd på stadens namn (30 tecken)
#define URL_SIZE 200        // Definierar max storlek på URL-sträng (200 tecken)
#define CACHE_FILE "cache.txt"  // Definierar filnamn för cachad data
#define CACHE_MAX_AGE 3600  // Definierar max ålder på cache i sekunder (1 timme)

typedef struct {            // Definierar City-struct för stadsinformation
    char name[CITY_NAME_LEN];  // Fält för stadens namn
    double lat;             // Fält för latitud (nord-syd position)
    double lon;             // Fält för longitud (öst-väst position)
} City;                     // Namn på struct-typen

typedef struct {            // Definierar MemBuffer-struct för minneshantering
    char *data;             // Pekare till allokerat minne på heapen
    size_t size;            // Storlek på datan i minnet
} MemBuffer;                // Namn på struct-typen

typedef struct {            // Definierar Cache-struct för cachad väderdata
    char city[CITY_NAME_LEN];  // Fält för stadens namn i cache
    char json[2048];        // Fält för JSON-data (max 2048 tecken)
    time_t time;            // Fält för när datan hämtades (tidsstämpel)
} Cache;                    // Namn på struct-typen

void show_menu(void);       // Deklarerar funktion för att visa menyn
int get_choice(void);       // Deklarerar funktion för att läsa användarval
void get_weather(int choice); // Deklarerar funktion för att hämta väder

#endif                      // Avslutar header-guard