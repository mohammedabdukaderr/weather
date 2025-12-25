#ifndef VADER_API_H
#define VADER_API_H

#include "vaderprotokoll.h"
#include <stdbool.h>

// Hämta aktuellt väder från OpenWeatherMap API
// Returnerar true vid framgång, false vid fel
bool hamta_aktuellt_vader(const char* stad, const char* landskod,
                          const char* api_nyckel, VaderData* resultat);

// Hämta väderprognos från OpenWeatherMap API
// Returnerar antal dagar som hämtades (0 vid fel)
int hamta_vader_prognos(const char* stad, const char* landskod,
                        const char* api_nyckel, VaderPrognos* resultat);

// Hjälpfunktion: Parsa JSON-svar från OpenWeatherMap
bool parsa_vader_json(const char* json_data, VaderData* resultat);

// Hjälpfunktion: Parsa JSON-prognos från OpenWeatherMap
int parsa_prognos_json(const char* json_data, VaderPrognos* resultat);

#endif // VADER_API_H
