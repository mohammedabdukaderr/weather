#ifndef LOGGNING_H
#define LOGGNING_H

#include "konfiguration.h"
#include <stdio.h>
#include <time.h>

// Makron för enkel loggning med filnamn och radnummer
#define LOGG_DEBUG(format, ...) \
    skriv_logg(LOG_NIVA_DEBUG, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define LOGG_INFO(format, ...) \
    skriv_logg(LOG_NIVA_INFO, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define LOGG_VARNING(format, ...) \
    skriv_logg(LOG_NIVA_VARNING, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define LOGG_FEL(format, ...) \
    skriv_logg(LOG_NIVA_FEL, __FILE__, __LINE__, format, ##__VA_ARGS__)

// Initialisera loggningssystem
void initiera_loggning(LogNiva niva);

// Skriv loggmeddelande
void skriv_logg(LogNiva niva, const char* fil, int rad, const char* format, ...);

// Stäng loggningssystem
void stang_loggning(void);

#endif // LOGGNING_H
