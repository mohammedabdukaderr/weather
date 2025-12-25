#ifndef KONFIGURATION_H
#define KONFIGURATION_H

// Server-konfiguration
#define SERVER_PORT 8080                          // TCP-port för servern
#define MAX_KLIENTER 32                           // Max samtidiga klienter
#define BUFFER_STORLEK 4096                       // Bufferstorlek för mottagning
#define TIMEOUT_SEKUNDER 30                       // Timeout för inaktiva klienter

// OpenWeatherMap API-konfiguration
#define API_HOST "api.openweathermap.org"
#define API_PORT 80
#define API_ENDPOINT "/data/2.5/weather"
#define API_FORECAST_ENDPOINT "/data/2.5/forecast"

// Cache-konfiguration
#define CACHE_KATALOG "./cache"                   // Katalog för cachefiler
#define CACHE_GILTIGHETSTID 1800                  // Cache giltighet i sekunder (30 min)

// Logging-konfiguration
typedef enum {
    LOG_NIVA_DEBUG = 0,                           // Detaljerad debug-information
    LOG_NIVA_INFO = 1,                            // Allmän information
    LOG_NIVA_VARNING = 2,                         // Varningar
    LOG_NIVA_FEL = 3                              // Fel
} LogNiva;

extern LogNiva aktuell_log_niva;                  // Global loggningsnivå

#endif // KONFIGURATION_H
