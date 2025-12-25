#include "cache.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>

// Skapar en ny cache
WeatherCache* cache_skapa(size_t max_storlek, const char* cache_katalog) {
    WeatherCache* cache = malloc(sizeof(WeatherCache));
    cache->huvud = NULL;
    cache->storlek = 0;
    cache->max_storlek = max_storlek;

    // Allokera minne för cache_katalog
    if (cache_katalog) {
        cache->cache_katalog = strdup(cache_katalog);
        // Skapa katalogen om den inte finns
        mkdir(cache_katalog, 0755);
    } else {
        cache->cache_katalog = strdup("./cache");
        mkdir("./cache", 0755);
    }

    return cache;
}

// Hämtar data från cache
char* cache_hamta(WeatherCache* cache, const char* stad) {
    if (!cache || !stad) return NULL;

    CacheEntry* nuvarande = cache->huvud;
    while (nuvarande) {
        if (strcmp(nuvarande->stad, stad) == 0) {
            // Kolla om data har gått ut
            if (time(NULL) < nuvarande->utgangstid) {
                return nuvarande->vaderdata;
            } else {
                return NULL; // Data har gått ut
            }
        }
        nuvarande = nuvarande->nasta;
    }

    return NULL; // Hittades inte
}

// Lägger till data i cache
int cache_lagg_till(WeatherCache* cache, const char* stad, const char* vaderdata, int ttl_sekunder) {
    if (!cache || !stad || !vaderdata) return 0;

    // Kolla om staden redan finns, uppdatera i så fall
    CacheEntry* nuvarande = cache->huvud;
    while (nuvarande) {
        if (strcmp(nuvarande->stad, stad) == 0) {
            free(nuvarande->vaderdata);
            nuvarande->vaderdata = strdup(vaderdata);
            nuvarande->tidsstampel = time(NULL);
            nuvarande->utgangstid = time(NULL) + ttl_sekunder;
            return 1;
        }
        nuvarande = nuvarande->nasta;
    }

    // Skapa ny entry
    CacheEntry* ny_entry = malloc(sizeof(CacheEntry));
    ny_entry->stad = strdup(stad);
    ny_entry->vaderdata = strdup(vaderdata);
    ny_entry->tidsstampel = time(NULL);
    ny_entry->utgangstid = time(NULL) + ttl_sekunder;
    ny_entry->nasta = cache->huvud;

    cache->huvud = ny_entry;
    cache->storlek++;

    return 1;
}

// Sparar cache till fil
int cache_spara_till_fil(WeatherCache* cache, const char* filnamn) {
    if (!cache || !filnamn) return 0;

    char hel_stig[512];
    snprintf(hel_stig, sizeof(hel_stig), "%s/%s", cache->cache_katalog, filnamn);

    FILE* fil = fopen(hel_stig, "w");
    if (!fil) return 0;

    // Skriv cache-storlek
    fprintf(fil, "CACHE_STORLEK:%zu\n", cache->storlek);

    // Gå igenom länkad lista och skriv varje entry
    CacheEntry* nuvarande = cache->huvud;
    while (nuvarande) {
        fprintf(fil, "ENTRY_START\n");
        fprintf(fil, "STAD:%s\n", nuvarande->stad);
        fprintf(fil, "TIDSSTAMPEL:%ld\n", (long)nuvarande->tidsstampel);
        fprintf(fil, "UTGANGSTID:%ld\n", (long)nuvarande->utgangstid);
        fprintf(fil, "DATA:%s\n", nuvarande->vaderdata);
        fprintf(fil, "ENTRY_END\n");
        nuvarande = nuvarande->nasta;
    }

    fclose(fil);
    return 1;
}

// Förstör cache och frigör minne
void cache_fororsa(WeatherCache** cache_ptr) {
    if (!cache_ptr || !*cache_ptr) return;

    WeatherCache* cache = *cache_ptr;
    CacheEntry* nuvarande = cache->huvud;

    // Frigör alla entries
    while (nuvarande) {
        CacheEntry* nasta = nuvarande->nasta;
        free(nuvarande->stad);
        free(nuvarande->vaderdata);
        free(nuvarande);
        nuvarande = nasta;
    }

    // Frigör cache_katalog
    free(cache->cache_katalog);

    // Frigör själva cache-strukturen
    free(cache);
    *cache_ptr = NULL;
}