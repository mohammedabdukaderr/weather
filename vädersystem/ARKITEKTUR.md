# Systemarkitektur - Vädersystem

## Översikt

Detta dokument beskriver systemarkitekturen för vädersystemet, som består av en C-baserad HTTP/JSON-server och tre klienter (C, C++, ESP32). Systemet är designat för att vara modulärt, skalbart och portabelt över olika plattformar.

## Systembild

```
┌─────────────────────────────────────────────────────────────┐
│                    EXTERNA TJÄNSTER                         │
│  ┌───────────────────────────────────────────────────────┐  │
│  │       OpenWeatherMap REST API                         │  │
│  │  - Current Weather API                                │  │
│  │  - 5-day Forecast API                                 │  │
│  └───────────────────────────────────────────────────────┘  │
└──────────────────────────┬──────────────────────────────────┘
                           │ HTTPS
                           ▼
┌─────────────────────────────────────────────────────────────┐
│                    VÄDERSERVER (C)                          │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │  TCP Server  │─▶│ HTTP Parser  │─▶│  JSON Helper │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
│         │                                                    │
│         ▼                                                    │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │    Router    │─▶│  Vader API   │◀─│   Cache      │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
│         │                                                    │
│  ┌──────────────┐                                           │
│  │   Loggning   │                                           │
│  └──────────────┘                                           │
└─────────────────────────────────────────────────────────────┘
                           │ HTTP/JSON
                           ▼
┌─────────────────────────────────────────────────────────────┐
│                        KLIENTER                             │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │  C-klient    │  │  C++-klient  │  │ ESP32-klient │      │
│  │  (Desktop)   │  │  (Desktop)   │  │ (Embedded)   │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
└─────────────────────────────────────────────────────────────┘
```

## Komponentarkitektur

### Server-komponenter

#### 1. TCP Server (`src/tcp_server.c`)
**Ansvar**: Lågnivå nätverkskommunikation

**Funktionalitet**:
- Skapar TCP-socket och binder till port
- Lyssnar efter inkommande anslutningar
- Accepterar klientanslutningar
- Plattformsoberoende socket-abstraktion

**Beroenden**:
- `natverks_abstraktion.h` - Cross-platform socket API
- `loggning.h` - Felrapportering

**API**:
```c
typedef struct {
    socket_t socket_fd;
    int port;
} TcpServer;

int initiera_tcp_server(TcpServer* server, int port);
socket_t acceptera_klient(TcpServer* server);
void stang_tcp_server(TcpServer* server);
```

#### 2. HTTP Server (`src/http_server.c`)
**Ansvar**: HTTP-protokollhantering

**Funktionalitet**:
- Parsar HTTP-requests (GET, POST)
- Extraherar URL-sökväg och query-parametrar
- Skapar HTTP-responses med korrekt format
- Hanterar HTTP-statuskoder (200, 400, 404, 500)

**API**:
```c
typedef enum {
    HTTP_GET,
    HTTP_POST,
    HTTP_UNKNOWN
} HttpMetod;

typedef struct {
    HttpMetod metod;
    char sokvag[256];
    char query[512];
    char body[1024];
} HttpRequest;

bool parsa_http_request(const char* rådata, HttpRequest* request);
void skapa_http_response(char* buffer, size_t buffer_storlek,
                         int statuskod, const char* json_data);
bool hamta_query_parameter(const char* query, const char* parameter_namn,
                           char* värde, size_t värde_storlek);
```

#### 3. JSON Helper (`src/json_helper.c`)
**Ansvar**: JSON-generering och parsing

**Funktionalitet**:
- Extraherar värden från JSON-strängar
- Skapar JSON-objekt för väderdata och prognoser
- Lightweight implementation utan externa bibliotek

**API**:
```c
bool hamta_json_varde(const char* json, const char* nyckel,
                      char* varde, size_t varde_storlek);
float hamta_json_float(const char* json, const char* nyckel);
void skapa_vader_json(const VaderData* data, char* buffer, size_t storlek);
void skapa_prognos_json(const VaderPrognos* prognos, char* buffer, size_t storlek);
```

#### 4. Väder API (`src/vader_api.c`)
**Ansvar**: Integration med OpenWeatherMap

**Funktionalitet**:
- HTTP-klient för att anropa OpenWeatherMap API
- Parsar JSON-svar från API
- Hanterar API-fel och timeout
- Stödjer både current weather och forecast

**API**:
```c
bool hamta_aktuellt_vader(const char* stad, const char* landskod,
                         const char* api_nyckel, VaderData* data);
int hamta_vader_prognos(const char* stad, const char* landskod,
                       const char* api_nyckel, VaderPrognos* prognos);
```

#### 5. Cache (`src/cache.c`)
**Ansvar**: Lokal datacache för att minska API-anrop

**Funktionalitet**:
- Filbaserad cache i `cache/` mapp
- TTL (Time To Live) på 30 minuter
- Automatisk upprensning av gamla filer
- Separata cache-filer per stad

**Cache-struktur**:
```
cache/
├── stockholm_se_current.cache
├── stockholm_se_forecast.cache
├── london_gb_current.cache
└── ...
```

**Cache-filformat**:
```
[Unix timestamp]
[JSON-data]
```

**API**:
```c
bool initiera_cache(void);
bool las_fran_cache(const char* stad, const char* landskod, VaderData* data);
void skriv_till_cache(const char* stad, const char* landskod, const VaderData* data);
bool las_prognos_fran_cache(const char* stad, const char* landskod, VaderPrognos* prognos);
void skriv_prognos_till_cache(const char* stad, const char* landskod, const VaderPrognos* prognos);
```

#### 6. Loggning (`src/loggning.c`)
**Ansvar**: Systemloggning och felsökning

**Funktionalitet**:
- Fyra lognivåer: DEBUG, INFO, VARNING, FEL
- Loggar till både fil och konsol
- Tidsstämplar för varje loggpost
- Filnamn och radnummer för debug

**Loggformat**:
```
[2025-01-15 14:23:45] [INFO] tcp_server.c:42 - Server lyssnar på port 8080
```

**API**:
```c
typedef enum {
    LOG_NIVA_DEBUG = 0,
    LOG_NIVA_INFO = 1,
    LOG_NIVA_VARNING = 2,
    LOG_NIVA_FEL = 3
} LogNiva;

void initiera_loggning(LogNiva niva);
void stang_loggning(void);

// Makron för bekväm loggning
LOGG_DEBUG(format, ...);
LOGG_INFO(format, ...);
LOGG_VARNING(format, ...);
LOGG_FEL(format, ...);
```

#### 7. Main Router (`src/main.c`)
**Ansvar**: Routing och orchestration

**Funktionalitet**:
- HTTP-endpoint routing
- Request-hantering
- Signal-hantering (Ctrl+C)
- Huvudloop för servern

**Endpoints**:
```
GET /                               → API-dokumentation
GET /weather?city=X&country=Y       → Aktuellt väder
GET /forecast?city=X&country=Y      → 5-dagars prognos
```

### Klientkomponenter

#### 1. C-klient (`client/weather_client.c`)
**Plattform**: Windows, Linux, macOS

**Funktionalitet**:
- TCP socket-kommunikation
- HTTP GET-requests
- JSON-parsing
- Terminal-baserad UI

**Användning**:
```bash
./weather_client Stockholm SE
```

#### 2. C++-klient (`client/weather_client.cpp`)
**Plattform**: Windows, Linux, macOS

**Funktionalitet**:
- Samma som C-klient men med C++-features
- Klasser för VaderData och NatverksKlient
- STL string och iostream
- RAII för resurshantering
- Exception-hantering

**Fördelar över C-klient**:
- Typ-säkerhet
- Automatisk minneshantering
- Mer lättläst kod
- Modern C++ best practices

#### 3. ESP32-klient (`client/weather_client_esp32.c`)
**Plattform**: ESP32 mikrokontroller

**Funktionalitet**:
- ESP-IDF WiFi-integration
- FreeRTOS tasks
- Optimerad minnesanvändning (6KB total)
- Periodisk datahämtning (10 min intervall)
- Lågströmsdrift

**Krav**:
- ESP-IDF v4.0+
- WiFi-konfiguration
- 6KB RAM minimum

## Dataflöde

### Request-flöde (Client → Server)

```
1. Klient skickar HTTP GET-request
   GET /weather?city=Stockholm&country=SE HTTP/1.1

2. TCP Server tar emot data
   ↓
3. HTTP Parser extraherar metod, sökväg, parametrar
   ↓
4. Router identifierar endpoint (/weather)
   ↓
5. Cache-kontroll (har vi färsk data?)
   │
   ├─ Cache HIT → Returnera från cache
   │
   └─ Cache MISS → Fortsätt till API
      ↓
6. Väder API anropar OpenWeatherMap
   ↓
7. JSON Helper parsar API-svar
   ↓
8. Cache sparar ny data
   ↓
9. JSON Helper skapar svar-JSON
   ↓
10. HTTP Server skapar HTTP-response
    ↓
11. TCP Server skickar till klient
```

### Cache-strategi

```
┌─────────────┐
│  Request    │
└──────┬──────┘
       │
       ▼
┌─────────────────┐     JA      ┌─────────────┐
│  Cache finns?   │────────────▶│ Data färsk? │
└─────────────────┘             └──────┬──────┘
       │ NEJ                           │ NEJ
       │                               │
       │      JA ◀────────────────────┘
       │
       ▼
┌─────────────────┐
│  Anropa API     │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Spara i cache  │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Returnera data │
└─────────────────┘
```

**TTL (Time To Live)**: 30 minuter
- Fördelar: Minskar API-anrop, snabbare svar, mindre kostnad
- Nackdelar: Data kan vara upp till 30 min gammal

## Plattformsoberoende

### Nätverksabstraktion (`include/natverks_abstraktion.h`)

Hanterar skillnader mellan Windows Winsock och POSIX sockets:

```c
#ifdef _WIN32
    typedef SOCKET socket_t;
    #define OGILTIG_SOCKET INVALID_SOCKET
    #define stang_socket closesocket
    // Windows-specifika funktioner
#else
    typedef int socket_t;
    #define OGILTIG_SOCKET -1
    #define stang_socket close
    // Unix/Linux-funktioner
#endif
```

### Kompilering per plattform

**Makefile** detekterar automatiskt plattform:

```makefile
ifeq ($(OS),Windows_NT)
    LDFLAGS = -lws2_32        # Windows Winsock
    SERVER = weather_server.exe
else
    LDFLAGS =                 # Linux/macOS (inget behövs)
    SERVER = weather_server
endif
```

## Säkerhet

### API-nyckelhantering
- Aldrig hardkodad i källkod
- Skickas som kommandoradsargument
- `.gitignore` blockerar `api_key.txt`
- Rekommendation: Miljövariabler

### HTTP-säkerhet
- Ingen autentisering för klienter (lokalt nätverk)
- API-nyckel används endast server→OpenWeatherMap
- Input-validering för query-parametrar
- Buffer overflow-skydd med storlek-parametrar

## Prestanda

### Minnesutnyttjande

| Komponent | Stack | Heap | Total |
|-----------|-------|------|-------|
| Server huvudprocess | 8KB | 20KB | 28KB |
| Per klient-hantering | 16KB | 8KB | 24KB |
| C-klient | 4KB | 8KB | 12KB |
| C++-klient | 4KB | 12KB | 16KB |
| ESP32-klient | 2KB | 4KB | 6KB |

### Svarstider (typiskt)

| Operation | Tid | Not |
|-----------|-----|-----|
| Cache HIT | <5ms | Läser lokal fil |
| Cache MISS | 200-500ms | API-anrop + nätverk |
| API timeout | 5s | Konfigurerbart |

### Skalbarhet

**Nuvarande begränsningar**:
- En klient i taget (blocking accept)
- Ingen connection pooling

**Framtida förbättringar**:
- Multi-threading för samtidiga klienter
- Connection pooling
- Load balancing

## Felsökning

### Debug-lägen

**Server**:
```bash
./weather_server API_KEY 8080 0  # DEBUG-nivå
```

**Loggfiler**:
- `vaderserver.log` - Alla serverhändelser
- Tidsstämplar och filnamn för varje händelse

### Vanliga problem

Se [TROUBLESHOOTING.md](README.md#troubleshooting) för detaljer.

## Teknisk stack

| Komponent | Teknologi | Version |
|-----------|-----------|---------|
| Språk (Server) | C | C11 |
| Språk (C++-klient) | C++ | C++11 |
| Kompilator | GCC | 7.0+ |
| Build-system | Make | 4.0+ |
| Nätverksprotokoll | TCP/IP | - |
| Applikationsprotokoll | HTTP/1.1 | - |
| Dataformat | JSON | - |
| OS-support | Windows, Linux, macOS, ESP32 | - |

## Vidareutveckling

Se [UTVECKLING.md](UTVECKLING.md) för:
- Planerade features
- Arkitektoniska förbättringar
- Skalbarhetsstrategi
- ESP32-optimeringar
