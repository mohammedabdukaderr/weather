# Vädersystem API Guide

## Översikt

Detta dokument beskriver det binära protokollet som används mellan klienter och server i vädersystemet.

---

## Protokollversion

**Aktuell version:** 1

Protokollversionen säkerställer kompatibilitet mellan klient och server.

---

## Meddelandeformat

Alla meddelanden består av två delar:

1. **ProtokollHuvud** (4 bytes)
2. **Meddelandedata** (variabel storlek)

### ProtokollHuvud

```c
typedef struct {
    uint8_t version;      // Protokollversion (alltid 1)
    uint8_t typ;          // MeddelandeTyp
    uint16_t langd;       // Längd på data som följer (big-endian)
} __attribute__((packed)) ProtokollHuvud;
```

**Storlek:** 4 bytes
**Byteordning:** `langd` är i big-endian (nätverksordning)

---

## Meddelandetyper

### 1. MEDDELANDE_HAMTA_VADER (1)

**Riktning:** Klient → Server
**Beskrivning:** Begär aktuellt väder för en stad

**Data:**
```c
typedef struct {
    char stad[64];         // Stadens namn (UTF-8)
    char landskod[3];      // Landskod (ex: "SE", "US", "GB")
} VaderForfragan;
```

**Exempel:**
- stad: "Stockholm"
- landskod: "SE"

---

### 2. MEDDELANDE_HAMTA_PROGNOS (2)

**Riktning:** Klient → Server
**Beskrivning:** Begär väderprognos för en stad

**Data:** Samma som MEDDELANDE_HAMTA_VADER (VaderForfragan)

---

### 3. MEDDELANDE_VADER_SVAR (3)

**Riktning:** Server → Klient
**Beskrivning:** Svar med väderdata eller prognos

**Data (aktuellt väder):**
```c
typedef struct {
    char stad[64];              // Stadens namn
    float temperatur;           // Temperatur i Celsius
    float luftfuktighet;        // Luftfuktighet i procent (0-100)
    float vindhastighet;        // Vindhastighet i m/s
    float lufttryck;            // Lufttryck i hPa
    char beskrivning[128];      // Textbeskrivning (ex: "klart väder")
    char ikon_id[16];           // OpenWeatherMap ikon-ID
    int64_t tidsstampel;        // Unix timestamp när data hämtades
} VaderData;
```

**Data (prognos):**
```c
typedef struct {
    int antal_dagar;            // Antal prognosdagar (1-5)
    VaderData dagar[5];         // Array med väderdata per dag
} VaderPrognos;
```

---

### 4. MEDDELANDE_FEL (4)

**Riktning:** Server → Klient
**Beskrivning:** Felmeddelande från server

**Data:**
```c
typedef struct {
    int felkod;                 // Felkod (se nedan)
    char meddelande[256];       // Mänskligt läsbart felmeddelande
} FelMeddelande;
```

---

## Felkoder

| Kod | Konstant | Beskrivning |
|-----|----------|-------------|
| 0 | FEL_OK | Ingen fel |
| 1 | FEL_OGILTIG_STAD | Stad kunde inte hittas |
| 2 | FEL_API_FEL | Fel vid kommunikation med väder-API |
| 3 | FEL_NÄTVERKSFEL | Nätverksfel |
| 4 | FEL_TIMEOUT | Timeout vid API-anrop |
| 99 | FEL_OKÄNT | Okänt fel |

---

## Kommunikationsflöde

### Scenario 1: Hämta Aktuellt Väder

**Klient → Server:**
```
ProtokollHuvud {
    version = 1
    typ = MEDDELANDE_HAMTA_VADER (1)
    langd = sizeof(VaderForfragan) = 67
}
VaderForfragan {
    stad = "Stockholm"
    landskod = "SE"
}
```

**Server → Klient (framgång):**
```
ProtokollHuvud {
    version = 1
    typ = MEDDELANDE_VADER_SVAR (3)
    langd = sizeof(VaderData)
}
VaderData {
    stad = "Stockholm"
    temperatur = 15.5
    luftfuktighet = 65.0
    vindhastighet = 3.2
    lufttryck = 1013.0
    beskrivning = "molnigt"
    ikon_id = "04d"
    tidsstampel = 1735142400
}
```

**Server → Klient (fel):**
```
ProtokollHuvud {
    version = 1
    typ = MEDDELANDE_FEL (4)
    langd = sizeof(FelMeddelande)
}
FelMeddelande {
    felkod = FEL_OGILTIG_STAD (1)
    meddelande = "Stad 'XYZ' hittades inte"
}
```

---

### Scenario 2: Hämta Prognos

**Klient → Server:**
```
ProtokollHuvud {
    version = 1
    typ = MEDDELANDE_HAMTA_PROGNOS (2)
    langd = sizeof(VaderForfragan)
}
VaderForfragan {
    stad = "Paris"
    landskod = "FR"
}
```

**Server → Klient:**
```
ProtokollHuvud {
    version = 1
    typ = MEDDELANDE_VADER_SVAR (3)
    langd = sizeof(VaderPrognos)
}
VaderPrognos {
    antal_dagar = 3
    dagar[0] = { ... }
    dagar[1] = { ... }
    dagar[2] = { ... }
}
```

---

## Implementeringsexempel

### C Klient - Skicka Förfrågan

```c
#include "vaderprotokoll.h"
#include "natverks_abstraktion.h"

socket_t sock = anslut_till_server("127.0.0.1", 8080);

// Skapa huvud
ProtokollHuvud huvud;
huvud.version = PROTOKOLL_VERSION;
huvud.typ = MEDDELANDE_HAMTA_VADER;
huvud.langd = hton16(sizeof(VaderForfragan));  // Konvertera till big-endian

// Skapa förfrågan
VaderForfragan forfragan;
strcpy(forfragan.stad, "Stockholm");
strcpy(forfragan.landskod, "SE");

// Skicka
send(sock, &huvud, sizeof(huvud), 0);
send(sock, &forfragan, sizeof(forfragan), 0);

// Ta emot svar
ProtokollHuvud svar_huvud;
recv(sock, &svar_huvud, sizeof(svar_huvud), 0);

if (svar_huvud.typ == MEDDELANDE_VADER_SVAR) {
    VaderData data;
    recv(sock, &data, sizeof(data), 0);
    printf("Temperatur: %.1f°C\n", data.temperatur);
} else if (svar_huvud.typ == MEDDELANDE_FEL) {
    FelMeddelande fel;
    recv(sock, &fel, sizeof(fel), 0);
    printf("Fel: %s\n", fel.meddelande);
}
```

---

## Byteordning

**Viktigt:** Endast `ProtokollHuvud.langd` använder big-endian (nätverksordning). Alla andra fält använder host byte order.

Använd hjälpfunktionerna:
- `hton16(uint16_t)` - Host to Network (little → big endian)
- `ntoh16(uint16_t)` - Network to Host (big → little endian)

---

## Säkerhet

### Ingen Autentisering

**Nuvarande implementation:** Systemet har ingen autentisering. Vem som helst kan ansluta till servern.

**Rekommendation för produktion:**
- Lägg till API-nyckel i ProtokollHuvud
- Implementera TLS/SSL för krypterad kommunikation
- Rate limiting för att förhindra missbruk

---

## Cache-beteende

Servern cachar väderdata i 30 minuter (konfigurerat i `CACHE_GILTIGHETSTID`).

**Fördel:**
- Snabbare svar (5ms vs 200-500ms)
- Minskar API-anrop till OpenWeatherMap
- Sparar API-kvot

**Nackdel:**
- Data kan vara upp till 30 minuter gammal

**Kontrollera cache-ålder:**
Kontrollera `VaderData.tidsstampel` för att se när data hämtades.

---

## Utöka Protokollet

### Lägga till Nya Meddelandetyper

1. Lägg till ny konstant i `vaderprotokoll.h`:
```c
#define MEDDELANDE_HAMTA_HISTORIK 5
```

2. Definiera datastruktur:
```c
typedef struct {
    char stad[64];
    char landskod[3];
    int64_t fran_datum;
    int64_t till_datum;
} HistorikForfragan;
```

3. Implementera hantering i server (`main.c`)

4. Implementera i klient

---

## OpenWeatherMap Integration

### API-endpoints som används

**Aktuellt väder:**
```
GET http://api.openweathermap.org/data/2.5/weather
    ?q={stad},{landskod}
    &appid={API_NYCKEL}
    &units=metric
    &lang=sv
```

**Prognos:**
```
GET http://api.openweathermap.org/data/2.5/forecast
    ?q={stad},{landskod}
    &appid={API_NYCKEL}
    &units=metric
    &lang=sv
    &cnt=40
```

### JSON till VaderData Mapping

| OpenWeatherMap JSON | VaderData Fält |
|---------------------|----------------|
| `name` | `stad` |
| `main.temp` | `temperatur` |
| `main.humidity` | `luftfuktighet` |
| `wind.speed` | `vindhastighet` |
| `main.pressure` | `lufttryck` |
| `weather[0].description` | `beskrivning` |
| `weather[0].icon` | `ikon_id` |

---

## Tekniska Detaljer

### Struct Packing

Alla protokollstrukturer använder `__attribute__((packed))` för att säkerställa ingen padding:

```c
typedef struct {
    uint8_t version;
    uint8_t typ;
    uint16_t langd;
} __attribute__((packed)) ProtokollHuvud;
```

Detta garanterar att strukturen är exakt 4 bytes på alla plattformar.

---

## Felsökning

### Problem: "Protokollversion stöds ej"

**Orsak:** Klient och server har olika protokollversioner
**Lösning:** Kompilera om både klient och server

### Problem: "Inkomplett data mottagen"

**Orsak:** Nätverksproblem eller fel bufferstorlek
**Lösning:** Implementera retry-logik eller kontrollera recv() return värde

---

**Senast uppdaterad:** 2025-12-25
