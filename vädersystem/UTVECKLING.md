# Vidareutvecklingsguide - Vädersystem

Detta dokument är avsett för utvecklare som ska vidareutveckla, underhålla eller bidra till vädersystemet.

## Innehåll
- [Kom igång](#kom-igång)
- [Projektstruktur](#projektstruktur)
- [Utvecklingsmiljö](#utvecklingsmiljö)
- [Kodstandard](#kodstandard)
- [Lägga till nya funktioner](#lägga-till-nya-funktioner)
- [Testning](#testning)
- [Felsökning](#felsökning)
- [Planerade förbättringar](#planerade-förbättringar)
- [Bidrag](#bidrag)

## Kom igång

### Förutsättningar

**Obligatoriskt**:
- GCC 7.0+ eller kompat

ibel C11-kompilator
- Make 4.0+
- Git

**Rekommenderat**:
- GDB (debugging)
- Valgrind (minnesläckedetektering)
- cppcheck (statisk kodanalys)

### Klona och bygg

```bash
# Klona repository
git clone https://github.com/mohammedabdukaderr/weather.git
cd weather

# Bygg alla komponenter
make all

# Testa servern
make run
```

## Projektstruktur

```
vädersystem/
├── src/                    # Server-källkod
│   ├── main.c             # Huvudloop och routing
│   ├── tcp_server.c       # TCP socket-hantering
│   ├── http_server.c      # HTTP-protokoll
│   ├── json_helper.c      # JSON-parsing/generering
│   ├── vader_api.c        # OpenWeatherMap integration
│   ├── cache.c            # Filbaserad cache
│   └── loggning.c         # Loggningssystem
│
├── include/               # Header-filer
│   ├── *.h               # Alla motsvarande headers
│   ├── vaderprotokoll.h  # Datastrukturer
│   ├── natverks_abstraktion.h  # Cross-platform sockets
│   └── konfiguration.h   # Konfigurationskonstanter
│
├── client/               # Klient-applikationer
│   ├── weather_client.c      # C-klient (desktop)
│   ├── weather_client.cpp    # C++-klient (desktop)
│   └── weather_client_esp32.c # ESP32-klient (embedded)
│
├── tests/                # Testfiler (ska skapas)
│   ├── test_json.c
│   ├── test_cache.c
│   └── test_http.c
│
├── cache/                # Cache-katalog (skapas runtime)
│   └── *.cache          # Cache-filer
│
├── docs/                 # Extra dokumentation
│
├── Makefile             # Build-konfiguration
├── README.md            # Användarmanual
├── ARKITEKTUR.md        # Systemarkitektur
├── UTVECKLING.md        # Detta dokument
└── .gitignore          # Git-exkluderingar
```

## Utvecklingsmiljö

### Rekommenderade IDE:er

**VSCode**:
```json
// .vscode/settings.json
{
  "C_Cpp.default.cStandard": "c11",
  "C_Cpp.default.includePath": ["${workspaceFolder}/include"],
  "files.associations": {
    "*.h": "c"
  }
}
```

**Vim/Neovim**:
```vim
" .vimrc
set path+=include
set wildignore+=*.o,*.cache
```

### Debug-bygge

```bash
# Bygg med debug-symboler (redan default i Makefile)
make clean && make

# Kör med GDB
gdb ./weather_server
(gdb) run API_KEY 8080 0
(gdb) break tcp_server.c:42
(gdb) continue
```

### Minnesläckor

```bash
# Valgrind för minnesläck-detektering
valgrind --leak-check=full --show-leak-kinds=all \
  ./weather_server API_KEY 8080 1

# Förväntat resultat: "no leaks are possible"
```

## Kodstandard

### Namnkonventioner

**Svenska namn överallt**:
- Funktioner: `hamta_vader_data()`, `parsa_http_request()`
- Variabler: `vader_data`, `klient_socket`, `json_buffer`
- Konstanter: `SERVER_PORT`, `BUFFER_STORLEK`
- Structs: `VaderData`, `HttpRequest`, `TcpServer`

**Undan tag**:
- C++-kod kan använda engelska för STL-kompatibilitet
- ESP32-kod följer ESP-IDF konventioner

### Kommentarstil

**Funktionsdokumentation**:
```c
/**
 * Kort beskrivning av funktionen
 *
 * VARFÖR FUNKTIONEN FINNS:
 * Längre förklaring av syftet och designbeslut
 *
 * @param param1 Beskrivning av parameter
 * @param param2 Beskrivning av parameter
 * @return Beskrivning av returvärde
 *
 * EXEMPEL:
 * kod_exempel();
 */
```

**Inline-kommentarer**:
```c
// VARFÖR, inte VAD
int resultat = atoi(buffer);  // atoi() konverterar sträng till int
```

### Felhantering

```c
// Returnera bool för success/failure
bool hamta_data(const char* stad, VaderData* data) {
    if (!stad || !data) {
        LOGG_FEL("Ogiltiga parametrar");
        return false;
    }

    // ... implementation

    if (fel_uppstod) {
        LOGG_VARNING("Kunde inte hämta data för %s", stad);
        return false;
    }

    return true;
}

// Anropa och kolla resultat
if (!hamta_data("Stockholm", &data)) {
    // Hantera fel
}
```

### Minneshantering

```c
// ALLTID kolla malloc-resultat
char* buffer = malloc(storlek);
if (!buffer) {
    LOGG_FEL("Minnesallokering misslyckades");
    return NULL;
}

// ALLTID free när klar
free(buffer);
buffer = NULL;  // Förhindra use-after-free
```

## Lägga till nya funktioner

### Exempel: Lägg till ny HTTP-endpoint

**1. Definiera endpoint i `src/main.c`**:

```c
// Efter befintliga endpoints i hantera_http_klient()
} else if (strcmp(request.sokvag, "/air-quality") == 0 && request.metod == HTTP_GET) {
    char stad[64] = {0};
    char landskod[3] = "SE";

    if (!hamta_query_parameter(request.query, "city", stad, sizeof(stad))) {
        skapa_fel_json(400, "Parameter 'city' saknas", json_buffer, sizeof(json_buffer));
        skapa_http_response(svar_buffer, sizeof(svar_buffer), 400, json_buffer);
        send(klient_socket, svar_buffer, (int)strlen(svar_buffer), 0);
        stang_socket(klient_socket);
        return;
    }

    hamta_query_parameter(request.query, "country", landskod, sizeof(landskod));

    LOGG_INFO("HTTP GET /air-quality?city=%s&country=%s", stad, landskod);

    // Implementera logik här
    // ...
}
```

**2. Skapa ny API-funktion i `src/vader_api.c`**:

```c
bool hamta_luftkvalitet(const char* stad, const char* landskod,
                        const char* api_nyckel, LuftkvalitetData* data) {
    // Implementation
}
```

**3. Lägg till datastruktur i `include/vaderprotokoll.h`**:

```c
typedef struct {
    float aqi;              // Air Quality Index
    float pm25;             // Particulate Matter 2.5
    float pm10;             // Particulate Matter 10
    int64_t tidsstampel;
} LuftkvalitetData;
```

**4. Uppdatera cache** i `src/cache.c`:

```c
bool las_luftkvalitet_fran_cache(const char* stad, const char* landskod,
                                  LuftkvalitetData* data);
void skriv_luftkvalitet_till_cache(const char* stad, const char* landskod,
                                    const LuftkvalitetData* data);
```

**5. Testa**:

```bash
make clean && make
./weather_server API_KEY 8080 0
curl "http://localhost:8080/air-quality?city=Stockholm&country=SE"
```

### Exempel: Lägg till ny konfiguration

**1. Lägg till i `include/konfiguration.h`**:

```c
#define CACHE_TTL_SEKUNDER 1800  // 30 minuter (ny)
#define MAX_CACHE_SIZE_MB 100    // Maximal cache-storlek (ny)
```

**2. Använd i relevanta filer**:

```c
// I cache.c
if (difftime(nu, cache_tid) > CACHE_TTL_SEKUNDER) {
    return false;  // Cache för gammal
}
```

## Testning

### Enhetstester

**Skapa testfil** `tests/test_json.c`:

```c
#include "../src/json_helper.c"  // Inkludera implementation
#include <assert.h>
#include <stdio.h>

void test_hamta_json_varde() {
    const char* json = "{\"stad\": \"Stockholm\", \"temp\": 23.5}";
    char buffer[64];

    bool resultat = hamta_json_varde(json, "stad", buffer, sizeof(buffer));
    assert(resultat == true);
    assert(strcmp(buffer, "Stockholm") == 0);

    printf("✓ test_hamta_json_varde\n");
}

void test_hamta_json_float() {
    const char* json = "{\"temp\": 23.5}";
    float temp = hamta_json_float(json, "temp");

    assert(temp > 23.4 && temp < 23.6);

    printf("✓ test_hamta_json_float\n");
}

int main() {
    test_hamta_json_varde();
    test_hamta_json_float();

    printf("\nAlla tester godkända!\n");
    return 0;
}
```

**Kompilera och kör**:

```bash
gcc -Iinclude tests/test_json.c -o test_json
./test_json
```

### Integrationstester

**Testscript** `tests/integration_test.sh`:

```bash
#!/bin/bash

API_KEY="test_key_123"
PORT=8081

# Starta server i bakgrunden
./weather_server $API_KEY $PORT 1 &
SERVER_PID=$!
sleep 2

# Testa endpoints
echo "Testar GET /..."
curl -s http://localhost:$PORT/ | grep "Vädersystem API" || exit 1

echo "Testar GET /weather..."
curl -s "http://localhost:$PORT/weather?city=Stockholm&country=SE" | grep "stad" || exit 1

echo "Testar 404..."
curl -s http://localhost:$PORT/invalid | grep "404" || exit 1

# Stoppa server
kill $SERVER_PID

echo "✓ Alla integrationstester godkända"
```

### Manuell testning

**Testplan**:
1. Starta server: `make run`
2. Testa varje endpoint i webbläsare
3. Testa C-klient: `./weather_client Stockholm SE`
4. Testa C++-klient: `./weather_client_cpp Stockholm SE`
5. Kontrollera cache: `ls -lh cache/`
6. Kontrollera loggar: `cat vaderserver.log`

## Felsökning

### Debug-utskrifter

```c
// Använd DEBUG-nivå för detaljerad information
LOGG_DEBUG("Variable värde: %d, pekare: %p", var, ptr);

// I produktion, använd INFO eller högre
LOGG_INFO("Server startad på port %d", port);
```

### Vanliga problem

**Problem: "Address already in use"**
```bash
# Hitta process på port 8080
netstat -tulpn | grep 8080

# Döda processen
kill -9 <PID>
```

**Problem: Segmentation fault**
```bash
# Kör med GDB
gdb ./weather_server
(gdb) run API_KEY 8080 0
# När den crashar:
(gdb) backtrace
(gdb) print variabel_namn
```

**Problem: Minnesläcka**
```bash
valgrind --leak-check=full ./weather_server API_KEY 8080 1
```

## Planerade förbättringar

### Kort sikt (nästa sprint)

- [ ] **Multi-threading för servern**
  - Problem: Endast en klient i taget
  - Lösning: pthread pool eller fork() för varje klient
  - Fil: `src/main.c`, `src/tcp_server.c`

- [ ] **HTTPS-stöd**
  - Problem: Okrypterad kommunikation
  - Lösning: OpenSSL/mbedTLS integration
  - Fil: Ny `src/ssl_server.c`

- [ ] **Databas istället för filcache**
  - Problem: Filsystem-begränsningar
  - Lösning: SQLite för cache
  - Fil: Ny `src/cache_db.c`

### Medellång sikt

- [ ] **WebSocket-support**
  - Mål: Realtidsuppdateringar
  - Användning: Push-notifikationer vid väderändring

- [ ] **REST API-klientbibliotek**
  - Mål: Förenkla integration
  - Språk: Python, JavaScript

- [ ] **Docker-containerisering**
  - Mål: Enkel deployment
  - Fil: Ny `Dockerfile`

### Lång sikt

- [ ] **Mikrotjänstarkitektur**
  - Cache-service
  - API-gateway
  - Weather-service

- [ ] **Kubernetes-orchestrering**
- [ ] **GraphQL API**

## ESP32-utveckling

### Sätta upp ESP-IDF

```bash
# Installera ESP-IDF
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh
. ./export.sh

# Skapa projekt
idf.py create-project vader_esp32
cp client/weather_client_esp32.c vader_esp32/main/main.c
```

### Konfigurera WiFi

**menuconfig**:
```bash
idf.py menuconfig
# Component config → Weather Client →
#   WiFi SSID: "DITT_WIFI"
#   WiFi Password: "DITT_LOSENORD"
```

**Eller i kod** (`weather_client_esp32.c`):
```c
#define WIFI_SSID      "DITT_WIFI_NAMN"
#define WIFI_PASSWORD  "DITT_WIFI_LOSENORD"
```

### Bygga och flasha

```bash
# Bygg
idf.py build

# Flasha till ESP32
idf.py -p /dev/ttyUSB0 flash

# Övervaka seriell output
idf.py -p /dev/ttyUSB0 monitor
```

### Minnesoptimering för ESP32

```c
// Använd stack-allokering istället för heap
char buffer[256];  // Stack (snabbt, auto-free)

// Istället för:
char* buffer = malloc(256);  // Heap (långsamt, manuell free)
```

## Bidrag

### Pull Request-process

1. **Forka** repository
2. **Skapa branch**: `git checkout -b feature/ny-funktion`
3. **Commit**: `git commit -m "Lägg till ny funktion"`
4. **Push**: `git push origin feature/ny-funktion`
5. **Skapa Pull Request** på GitHub

### Commit-meddelanden

```
Typ: Kort beskrivning (max 50 tecken)

Längre beskrivning om behövs (wrappas vid 72 tecken).
Förklara VARFÖR ändringen görs, inte VAD som ändrats.

Fixes: #123
```

**Typer**:
- `feat`: Ny funktion
- `fix`: Buggfix
- `docs`: Dokumentation
- `refactor`: Kodförbättring utan funktionsändring
- `test`: Lägga till/ändra tester
- `perf`: Prestandaförbättring

### Code Review-kriterier

- [ ] Kod följer kodstandard (svenska namn, kommentarer)
- [ ] Alla tester passerar
- [ ] Ingen minnesläcka (Valgrind)
- [ ] Dokumentation uppdaterad
- [ ] Kompilerar utan varningar

## Resurser

### Externa API:er

- [OpenWeatherMap API Docs](https://openweathermap.org/api)
- [OpenWeatherMap Current Weather](https://openweathermap.org/current)
- [OpenWeatherMap 5-day Forecast](https://openweathermap.org/forecast5)

### C-programmering

- [C11 Standard](http://www.open-std.org/jtc1/sc22/wg14/)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/)

### Verktyg

- [GDB Tutorial](https://www.cs.cmu.edu/~gilpin/tutorial/)
- [Valgrind Quick Start](https://valgrind.org/docs/manual/quick-start.html)
- [Make Manual](https://www.gnu.org/software/make/manual/)

## Support

- **Issues**: https://github.com/mohammedabdukaderr/weather/issues
- **Email**: mohammed.abdukader@chasacademy.se
- **Dokumentation**: Se README.md och ARKITEKTUR.md
