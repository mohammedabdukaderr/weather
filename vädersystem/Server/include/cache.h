#ifndef CACHE_H
#define CACHE_H

#include "vaderprotokoll.h"
#include <stdbool.h>

// Initialisera cache-system (skapar katalog om den inte finns)
bool initiera_cache(void);

// Läs väderdata från cache
// Returnerar true om giltig cachad data finns, false annars
bool las_fran_cache(const char* stad, const char* landskod, VaderData* resultat);

// Skriv väderdata till cache
bool skriv_till_cache(const char* stad, const char* landskod, const VaderData* data);

// Läs prognos från cache
bool las_prognos_fran_cache(const char* stad, const char* landskod, VaderPrognos* resultat);

// Skriv prognos till cache
bool skriv_prognos_till_cache(const char* stad, const char* landskod, const VaderPrognos* data);

// Rensa gamla cachefiler (äldre än CACHE_GILTIGHETSTID)
void rensa_gammal_cache(void);

#endif // CACHE_H
