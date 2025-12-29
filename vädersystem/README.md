# 🌤️ Vädersystem - Weather System

En komplett C-baserad vädertjänst med HTTP/JSON server, tre klientapplikationer (C, C++, ESP32) och omfattande testsuite. Hämtar realtidsdata från OpenWeatherMap API med inbyggt cache-system.

**Version**: 1.0.0
**Plattformar**: Windows, Linux, macOS, ESP32
**Språk**: C11, C++11

## ✨ Features

### Server
- ✅ HTTP/1.1 server med TCP socket-hantering
- ✅ JSON REST API med tre endpoints (/, /weather, /forecast)
- ✅ OpenWeatherMap API-integration
- ✅ 30-minuters filbaserad cache (minskar API-anrop)
- ✅ Strukturerad loggning (DEBUG/INFO/VARNING/FEL)
- ✅ Cross-platform (Windows/Linux/macOS)
- ✅ Svenska variabel- och funktionsnamn med pedagogiska kommentarer

### Klienter
- ✅ **C-klient**: Desktop-applikation för terminal
- ✅ **C++-klient**: Modern C++ med klasser, RAII och exceptions
- ✅ **ESP32-klient**: Embedded system med WiFi och FreeRTOS

### Dokumentation & Tester
- ✅ Omfattande dokumentation (README, ARKITEKTUR, UTVECKLING, TESTRAPPORT)
- ✅ Enhetstester för JSON och HTTP
- ✅ Integrationstester
- ✅ 100% testbäckning av kritiska komponenter

## 📁 Projektstruktur

```
vädersystem/
├── src/                    # Server-källkod (7 moduler)
│   ├── main.c             # Huvudloop och HTTP-routing
│   ├── tcp_server.c       # TCP socket-hantering
│   ├── http_server.c      # HTTP-protokoll
│   ├── json_helper.c      # JSON-parsing/generering
│   ├── vader_api.c        # OpenWeatherMap integration
│   ├── cache.c            # Filbaserad cache
│   └── loggning.c         # Loggningssystem
│
├── include/               # Header-filer
│   ├── *.h               # Motsvarande headers
│   ├── vaderprotokoll.h  # Datastrukturer
│   ├── natverks_abstraktion.h  # Cross-platform sockets
│   └── konfiguration.h   # Konfigurationskonstanter
│
├── client/               # Klientapplikationer
│   ├── weather_client.c      # C-klient (desktop)
│   ├── weather_client.cpp    # C++-klient (desktop)
│   └── weather_client_esp32.c # ESP32-klient (embedded)
│
├── tests/                # Testsuite
│   ├── test_json.c      # JSON-tester
│   ├── test_http.c      # HTTP-tester
│   └── run_all_tests.sh # Integrationstester
│
├── Makefile             # Build-konfiguration
├── README.md            # Detta dokument
├── ARKITEKTUR.md        # Systemarkitektur
├── UTVECKLING.md        # Utvecklingsguide
└── TESTRAPPORT.md       # Testresultat

```

## 🚀 Snabbstart

### Förutsättningar

**Windows (WSL rekommenderas):**
```bash
# Installera WSL2 med Ubuntu
wsl --install

# I WSL:
sudo apt-get update
sudo apt-get install build-essential gcc g++ make curl
```

**Linux/macOS:**
```bash
# Debian/Ubuntu
sudo apt-get install build-essential gcc g++ make

# macOS (Homebrew)
brew install gcc make
```

### Bygg och kör

```bash
# 1. Klona (eller navigera till projektet)
cd vädersystem

# 2. Bygg allt (server + båda klienter)
make all

# 3. Hämta API-nyckel
# Registrera dig på https://openweathermap.org/api (gratis)

# 4. Starta servern
make run
# ELLER med egen API-nyckel:
./weather_server DIN_API_NYCKEL 8080 1

# 5. Testa i webbläsare
# Öppna: http://localhost:8080/

# 6. Testa klienter (i ny terminal)
./weather_client Stockholm SE
./weather_client_cpp London GB
```

## 📚 Byggkommandon

| Kommando | Beskrivning |
|----------|-------------|
| `make all` | Bygg server + båda klienter |
| `make server` | Bygg endast servern |
| `make client` | Bygg C-klienten |
| `make client_cpp` | Bygg C++-klienten |
| `make test` | Kör alla enhetstester |
| `make run` | Starta servern |
| `make clean` | Rensa byggfiler |
| `make help` | Visa alla kommandon |

## 🌐 API Endpoints

### 1. Root Endpoint (API-dokumentation)
```http
GET /
```

**Respons:**
```json
{
  "service": "Vädersystem API",
  "version": "1.0.0",
  "endpoints": [...]
}
```

### 2. Aktuellt väder
```http
GET /weather?city=CITY&country=COUNTRY_CODE
```

**Parametrar:**
- `city` (obligatorisk): Stadens namn (på engelska)
- `country` (valfri): Landskod (ISO 3166-1 alpha-2, default: SE)

**Exempel:**
```bash
curl "http://localhost:8080/weather?city=Stockholm&country=SE"
```

**Respons:**
```json
{
  "stad": "Stockholm",
  "land": "SE",
  "temperatur": 23.5,
  "luftfuktighet": 65.0,
  "vindhastighet": 5.2,
  "lufttryck": 1013.0,
  "beskrivning": "Clear sky"
}
```

### 3. 5-dagars prognos
```http
GET /forecast?city=CITY&country=COUNTRY_CODE
```

**Respons:**
```json
{
  "stad": "Stockholm",
  "antal_dagar": 5,
  "dagar": [
    {"temperatur": 20.0, "beskrivning": "Sunny"},
    ...
  ]
}
```

## 🖥️ Klientanvändning

### C-klient

```bash
# Syntax
./weather_client [stad] [landskod]

# Exempel
./weather_client Stockholm SE
./weather_client London GB
./weather_client Paris FR
```

**Output:**
```
╔═══════════════════════════════════════════════════════╗
║              VÄDERRAPPORT - C KLIENT                 ║
╚═══════════════════════════════════════════════════════╝

📍 Stad:          Stockholm, SE
🌡️  Temperatur:   23.5°C
💧 Luftfuktighet: 65%
💨 Vindhastighet: 5.2 m/s
🔽 Lufttryck:     1013 hPa
☁️  Beskrivning:  Clear sky
```

### C++-klient

Samma användning som C-klienten men med moderna C++-features:
- Klasser och RAII
- Exception-hantering
- STL strings
- Automatisk resurshantering

```bash
./weather_client_cpp Stockholm SE
```

### ESP32-klient

Se [ESP32-guide](#esp32-utveckling) för detaljerade instruktioner.

## ⚙️ Konfiguration

### Serverpor

t
```bash
# Standard: port 8080
./weather_server API_KEY 8080 1

# Anpassad port
./weather_server API_KEY 3000 1
```

### Lognivåer
```bash
./weather_server API_KEY 8080 0  # DEBUG (mycket detaljerat)
./weather_server API_KEY 8080 1  # INFO (standard)
./weather_server API_KEY 8080 2  # VARNING
./weather_server API_KEY 8080 3  # FEL (minimal loggning)
```

### Cache-konfiguration

Cache-filer sparas i `cache/` och har en TTL på 30 minuter.

**Manuell cache-rensning:**
```bash
make clean-all  # Rensar både byggfiler och cache
# ELLER
rm -rf cache/
```

## 🔒 Säkerhet

### API-nyckelhantering

**✅ REKOMMENDERAT - Miljövariabel:**
```bash
# Linux/macOS/WSL
export WEATHER_API_KEY="your_api_key_here"
./weather_server $WEATHER_API_KEY 8080 1

# Windows CMD
set WEATHER_API_KEY=your_api_key_here
weather_server.exe %WEATHER_API_KEY% 8080 1
```

**❌ UNDVIK - Hardkodad:**
```c
// ALDRIG göra detta:
const char* api_key = "abc123xyz456";
```

**.gitignore** skyddar följande:
```
api_key.txt
.api_key
*.key
secrets.txt
```

## 🧪 Testning

### Enhetstester
```bash
make test
```

Kör:
- JSON-parsing och generering (12 tester)
- HTTP-request och response (14 tester)

### Integrationstester
```bash
# Linux/macOS/WSL
chmod +x tests/run_all_tests.sh
./tests/run_all_tests.sh
```

Testar:
- Server-start
- Alla HTTP-endpoints
- Cache-funktionalitet
- Felhantering (404, 500)

### Manuell testning
```bash
# Starta server
make run

# I ny terminal, testa endpoints
curl http://localhost:8080/
curl "http://localhost:8080/weather?city=Stockholm&country=SE"
curl "http://localhost:8080/forecast?city=Stockholm&country=SE"

# Testa 404
curl http://localhost:8080/invalid

# Testa klient
./weather_client Stockholm SE
```

## 🐛 Felsökning

### Problem: "Address already in use"
```bash
# Hitta process på port 8080
# Linux/macOS:
lsof -i :8080
kill -9 <PID>

# Windows WSL:
netstat -ano | findstr :8080
taskkill /PID <PID> /F
```

### Problem: "Kunde inte ansluta till server"
1. Kontrollera att servern körs: `ps aux | grep weather_server`
2. Kontrollera port: Server bör visa "lyssnar på port 8080"
3. Testa med curl: `curl http://localhost:8080/`

### Problem: "Invalid API key" (HTTP 401)
1. Vänta 2 timmar efter registrering (aktiveringstid)
2. Verifiera nyckel: https://home.openweathermap.org/api_keys
3. Kontrollera att nyckeln är aktiv

### Problem: Kompileringsfel
```bash
# Rensa och bygg om
make clean
make all

# Kontrollera GCC-version (behöver 7.0+)
gcc --version

# Kontrollera att alla headers finns
ls -l include/
```

### Debug-läge
```bash
# Kör med DEBUG-loggning
./weather_server API_KEY 8080 0

# Granska loggfil
cat vaderserver.log
tail -f vaderserver.log  # Följ live
```

## 🛠️ ESP32-utveckling

### Installation av ESP-IDF

```bash
# Klona ESP-IDF
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh
. ./export.sh
```

### Konfigurera WiFi

Redigera `client/weather_client_esp32.c`:
```c
#define WIFI_SSID      "DITT_WIFI_NAMN"
#define WIFI_PASSWORD  "DITT_WIFI_LOSENORD"
#define SERVER_HOST    "192.168.1.100"  // Din servers IP
```

### Bygg och flasha

```bash
# Skapa ESP-IDF projekt
idf.py create-project vader_esp32
cp client/weather_client_esp32.c vader_esp32/main/main.c

# Bygg
idf.py build

# Flasha till ESP32 (byt /dev/ttyUSB0 till din port)
idf.py -p /dev/ttyUSB0 flash

# Övervaka output
idf.py -p /dev/ttyUSB0 monitor
```

### Minnesanvändning ESP32
- Stack: ~2KB
- Heap: ~4KB
- Total: ~6KB (passar ESP32 med 520KB RAM)

## 📖 Dokumentation

| Fil | Innehåll |
|-----|----------|
| [README.md](README.md) | Detta dokument - Installation och användning |
| [ARKITEKTUR.md](ARKITEKTUR.md) | Systemarkitektur, komponenter, dataflöde |
| [UTVECKLING.md](UTVECKLING.md) | Utvecklingsguide, kodstandard, bidrag |
| [TESTRAPPORT.md](TESTRAPPORT.md) | Fullständig testrapport med resultat |

## 🤝 Bidrag

Se [UTVECKLING.md](UTVECKLING.md) för:
- Kodstandard
- Pull request-process
- Commit-konventioner
- Planerade features

## 📊 Teknisk specifikation

| Komponent | Teknologi |
|-----------|-----------|
| Språk (Server) | C11 |
| Språk (C++ Klient) | C++11 |
| Kompilator | GCC 7.0+ |
| Build | GNU Make 4.0+ |
| Protokoll | TCP/IP, HTTP/1.1 |
| Data-format | JSON |
| API | OpenWeatherMap REST API |
| Cache | Filbaserad, 30 min TTL |
| Plattformar | Windows, Linux, macOS, ESP32 |

## 📈 Prestanda

| Mätpunkt | Värde |
|----------|-------|
| Cache HIT-svarstid | <5ms |
| API-anrop (cache MISS) | 200-500ms |
| Server-minnesanvändning | ~28MB |
| C-klient-minne | ~12MB |
| C++-klient-minne | ~16MB |
| ESP32-klient-minne | ~6KB |

## 📄 Licens

Detta projekt utvecklades som en del av Chas Academy-kursen.

## 👨‍💻 Författare

**Mohammed Abdukader**
Chas Academy Student

- GitHub: [@mohammedabdukaderr](https://github.com/mohammedabdukaderr)
- Repository: https://github.com/mohammedabdukaderr/weather
- Email: mohammed.abdukader@chasacademy.se

## 🙏 Tack till

- OpenWeatherMap för gratis API
- Chas Academy för utbildning
- C-programmering community

## 📝 Changelog

### Version 1.0.0 (2025-01-15)
- ✅ Initial release
- ✅ HTTP/JSON server med cache
- ✅ C-klient
- ✅ C++-klient
- ✅ ESP32-klient
- ✅ Komplett dokumentation
- ✅ Testsuite med 38 tester

---

**Status**: ✅ Produktionsklar för lokal användning

För support, öppna ett issue på: https://github.com/mohammedabdukaderr/weather/issues
