#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Pekare-exempel: Funktion som tar en pekare till en sträng
char* skapa_kopia(const char* original);  // Pekare IN
void ändra_sträng(char** sträng);         // Pek-Pekare IN/UT

// Loggningsfunktioner
void logg_info(const char* meddelande, ...);
void logg_fel(const char* fil, int rad, const char* meddelande, ...);

#endif