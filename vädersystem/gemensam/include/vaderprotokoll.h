#ifndef VADERPROTOKOLL_H
#define VADERPROTOKOLL_H

#include <stdint.h>

// Protokollversion
#define PROTOKOLL_VERSION 1

// Meddelandetyper mellan klient och server
typedef enum {
    MEDDELANDE_HAMTA_VADER = 1,        // Klient begär aktuellt väder
    MEDDELANDE_HAMTA_PROGNOS = 2,      // Klient begär väderprognos
    MEDDELANDE_VADER_SVAR = 3,         // Server skickar väderdata
    MEDDELANDE_FEL = 4                 // Felmeddelande
} MeddelandeTyp;

// Protokollhuvud (skickas alltid först)
typedef struct {
    uint8_t version;                   // Protokollversion
    uint8_t typ;                       // MeddelandeTyp
    uint16_t langd;                    // Längd på data som följer
} __attribute__((packed)) ProtokollHuvud;

// Väderförfrågan från klient
typedef struct {
    char stad[64];                     // Stad att hämta väder för (UTF-8)
    char landskod[3];                  // Landskod (ex: "SE", "US")
} __attribute__((packed)) VaderForfragan;

// Väderdata från server
typedef struct {
    char stad[64];                     // Stadens namn
    float temperatur;                  // Temperatur i Celsius
    float luftfuktighet;               // Luftfuktighet i procent
    float vindhastig het;               // Vindhastighet i m/s
    float lufttryck;                   // Lufttryck i hPa
    char beskrivning[128];             // Textbeskrivning av vädret
    char ikon_id[16];                  // Ikon-ID från väder-API
    int64_t tidsstampel;               // Unix timestamp för data
} __attribute__((packed)) VaderData;

// Prognos (upp till 5 dagar)
typedef struct {
    int antal_dagar;                   // Antal prognosdagar (max 5)
    VaderData dagar[5];                // Array med väderdata per dag
} __attribute__((packed)) VaderPrognos;

// Felmeddelande
typedef struct {
    int felkod;                        // Felkod (se nedan)
    char meddelande[256];              // Felbeskrivning
} __attribute__((packed)) FelMeddelande;

// Felkoder
#define FEL_OK 0
#define FEL_OGILTIG_STAD 1
#define FEL_API_FEL 2
#define FEL_NÄTVERKSFEL 3
#define FEL_TIMEOUT 4
#define FEL_OKÄNT 99

// Hjälpfunktioner för nätverksbyteordning (big-endian)
static inline uint16_t hton16(uint16_t varde) {
    return ((varde & 0xFF) << 8) | ((varde >> 8) & 0xFF);
}

static inline uint16_t ntoh16(uint16_t varde) {
    return hton16(varde);  // Samma operation
}

#endif // VADERPROTOKOLL_H
