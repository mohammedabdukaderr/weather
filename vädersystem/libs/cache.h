#ifndef CACHE_H
#define CACHE_H

#include <stddef.h>
#include <time.h>

// Cache entry struktur - en nod i länkad lista
typedef struct CacheEntry {
    char* stad;                  // Stadsnamn (nyckel)
    char* vaderdata;             // Väderdata (JSON)
    time_t tidsstampel;          // När data cachades
    time_t utgangstid;           // När data går ut
    struct CacheEntry* nasta;    // Pekare till nästa entry
} CacheEntry;

// Cache struktur med länkad lista
typedef struct WeatherCache {
    CacheEntry* huvud;           // Första elementet
    size_t storlek;              // Antal element
    size_t max_storlek;          // Max antal element
    char* cache_katalog;         // Katalog för cache-filer
} WeatherCache;

// Cache-funktioner
WeatherCache* cache_skapa(size_t max_storlek, const char* cache_katalog);
void cache_fororsa(WeatherCache** cache);

// CRUD-operationer
char* cache_hamta(WeatherCache* cache, const char* stad);
int cache_lagg_till(WeatherCache* cache, const char* stad, const char* vaderdata, int ttl_sekunder);
int cache_ta_bort(WeatherCache* cache, const char* stad);
void cache_rensa(WeatherCache* cache);

// Filhantering
int cache_spara_till_fil(WeatherCache* cache, const char* filnamn);
int cache_ladda_fran_fil(WeatherCache* cache, const char* filnamn);

// Hjälpfunktioner
int cache_ar_utgangen(CacheEntry* entry);
void cache_rensa_utgangna(WeatherCache* cache);

#endif
