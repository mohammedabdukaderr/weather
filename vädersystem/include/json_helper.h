#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include <stdbool.h>
#include <stddef.h>

// Enkel JSON-hjälpbibliotek för att parsa OpenWeatherMap-svar
// Begränsat till de fält vi behöver

// Hitta ett värde i JSON med nyckel
// Returnerar pekare till värdet (utan citattecken) eller NULL om inte hittat
const char* json_hamta_varde(const char* json, const char* nyckel);

// Hitta ett numeriskt värde i JSON
// Returnerar värdet som double, eller 0.0 om inte hittat
double json_hamta_nummer(const char* json, const char* nyckel);

// Hitta en sträng i JSON
// Kopierar strängen till buffer (utan citattecken), returnerar true vid framgång
bool json_hamta_strang(const char* json, const char* nyckel, char* buffer, size_t storlek);

// Hitta ett heltal i JSON
int json_hamta_heltal(const char* json, const char* nyckel);

// Hitta första objektet i en array
// Returnerar pekare till objektets start eller NULL
const char* json_hamta_forsta_array_objekt(const char* json, const char* array_nyckel);

#endif // JSON_HELPER_H
