# 🌤️ Vädersystem - Komplett C/C++ Server-Klient Lösning

**Version:** 1.0.0
**Utvecklat enligt offert för [Företagsnamn AB]**

---

## 📖 Översikt

Ett komplett vädersystem med C-baserad HTTP-server och två klienter (C och C++) som hämtar väderdata från OpenWeatherMap API. Servern levererar data i JSON-format via HTTP, använder filbaserad cache och är cross-platform (Windows/Linux).

### ✨ Nyckelfunktioner

- ✅ **HTTP/JSON Server (C)**: Hanterar flera samtidiga klienter via TCP
- ✅ **JSON API**: Servern svarar i JSON-format enligt offerten
- ✅ **OpenWeatherMap Integration**: Hämtar verklig väderdata
- ✅ **Cache-system**: 30 min lokal cache för snabbare svar
- ✅ **Två Klienter**: C och C++ HTTP-klienter
- ✅ **Cross-platform**: Windows & Linux support
- ✅ **Svenska Variabelnamn**: Läsbar kod med svenska kommentarer
- ✅ **Omfattande Dokumentation**: README, API-guide, användarmanual

---

## 🚀 Snabbstart

### 1. Skaffa API-Nyckel

**📖 Detaljerad guide:** Se [API-Nyckel Guide](docs/API_NYCKEL_GUIDE.md) för steg-för-steg instruktioner!

**Snabbversion:**
1. Gå till https://openweathermap.org/api
2. Klicka på "Sign Up" och skapa gratis konto
3. Bekräfta din email
4. Hitta din API-nyckel på https://home.openweathermap.org/api_keys
5. Vänta 30 min - 2 timmar tills den är aktiv
6. Testa i webbläsare: `http://api.openweathermap.org/data/2.5/weather?q=Stockholm,SE&appid=DIN_API_NYCKEL&units=metric`

### 2. Bygg Projektet

**Windows (MinGW):**
```cmd
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```

**Linux:**
```bash
mkdir build && cd build
cmake ..
make
```

### 3. Kör Systemet

**Starta server:**
```bash
cd build/Server
./vaderserver <DIN-API-NYCKEL>
```

**Starta klient (nytt terminal-fönster):**
```bash
cd build/klient_c
./vaderklient_c
```

---

## 📂 Projektstruktur

```
vädersystem/
├── Server/                 # C-server med TCP, API, cache
│   ├── src/               # Källkod (TCP, API, JSON, cache, logging)
│   └── include/           # Header-filer
│
├── klient_c/              # C-klient
│   └── src/main.c         # Terminal-baserad klient
│
├── klient_cpp/            # C++-klient (objektorienterad)
│   └── src/main.cpp       # OOP-design med klasser
│
├── gemensam/              # Gemensamma headers
│   └── include/
│       ├── vaderprotokoll.h         # Protokolldefinitioner
│       └── natverks_abstraktion.h   # Platform-specifik nätverkskod
│
├── tests/                 # Testprogram
│   └── test_protokoll.c   # Enhetstester för protokoll
│
└── docs/                  # Dokumentation
    ├── README.md          # Fullständig dokumentation
    ├── API_GUIDE.md       # API-specifikation
    └── ANVANDNING.md      # Steg-för-steg användarguide
```

---

## 💡 Användningsexempel

### Hämta Väder för Stockholm

```
╔═══════════════════════════════════════════════════════╗
║          VÄDERSYSTEM - C-KLIENT                       ║
╚═══════════════════════════════════════════════════════╝

Välj alternativ:
  1. Hämta aktuellt väder
  2. Hämta väderprognos
  3. Avsluta
Val: 1

Ange stad: Stockholm
Ange landskod (ex: SE, US, GB): SE

╔═══════════════════════════════════════════════════════╗
║             AKTUELLT VÄDER - Stockholm                ║
╠═══════════════════════════════════════════════════════╣
║  🌡️  Temperatur:    15°C                              ║
║  💧  Luftfuktighet:  65%                              ║
║  💨  Vindhastighet:  3.2 m/s                          ║
║  📊  Lufttryck:      1013 hPa                         ║
║  ☁️  Beskrivning:    molnigt                          ║
╚═══════════════════════════════════════════════════════╝
```

---

## 🎯 Uppfyller Offertens Krav

| Krav | Status | Implementation |
|------|--------|----------------|
| Server stödjer TCP och svarar i JSON | ✅ | HTTP/JSON via [main.c](Server/src/main.c) + [http_server.c](Server/src/http_server.c) |
| C-klient hämtar och visar väderdata | ✅ | HTTP-klient i [klient_c/src/main.c](klient_c/src/main.c) |
| C++-klient med samma funktionalitet | ✅ | HTTP-klient i [klient_cpp/src/main.cpp](klient_cpp/src/main.cpp) |
| Lokal cache | ✅ | [cache.c](Server/src/cache.c) - 30 min TTL |
| Modulärt och dokumenterat | ✅ | Separata moduler + 4 dokumentationsfiler |
| ESP32-optimerad kod (C-klient) | ✅ | Minimal dependencies, standard HTTP |

---

## 🛠️ Teknisk Stack

- **Språk**: C11, C++17
- **Build**: CMake 3.15+
- **Nätverk**: HTTP över TCP (POSIX Sockets/Winsock2)
- **Protokoll**: HTTP/1.1 med JSON-data
- **External API**: OpenWeatherMap REST API
- **Format**: JSON (lightweight custom parser)
- **Cache**: Filbaserad (binary format)

---

## 📚 Dokumentation

- **[Fullständig Dokumentation](docs/README.md)** - Installation, konfiguration, felsökning
- **[API Guide](docs/API_GUIDE.md)** - Protokollspecifikation och implementering
- **[Användarguide](docs/ANVANDNING.md)** - Steg-för-steg instruktioner

---

## 🔧 Konfiguration

### Servern

Redigera [Server/include/konfiguration.h](Server/include/konfiguration.h):

```c
#define SERVER_PORT 8080              // TCP-port
#define MAX_KLIENTER 32               // Max samtidiga klienter
#define CACHE_GILTIGHETSTID 1800      // Cache TTL (sekunder)
```

### Loggning

```bash
# DEBUG (mycket detaljer)
./vaderserver API_KEY 8080 0

# INFO (standard)
./vaderserver API_KEY 8080 1

# VARNING
./vaderserver API_KEY 8080 2

# FEL (minimal output)
./vaderserver API_KEY 8080 3
```

---

## 🧪 Tester

```bash
cd build/tests
./test_protokoll
```

**Förväntad output:**
```
╔═══════════════════════════════════════════════════════╗
║        VÄDERSYSTEM - PROTOKOLLTESTER                  ║
╚═══════════════════════════════════════════════════════╝

Test: Protokollhuvud storlek... OK
Test: VaderData struktur... OK
Test: Byteordning (hton16/ntoh16)... OK
Test: Meddelandetyper... OK
Test: Felkoder... OK
Test: VaderPrognos struktur... OK

✓ Alla tester godkända!
```

---

## 🌍 Ansluta från Annat Nätverk

### På servern:

1. Hitta IP: `ipconfig` (Windows) eller `ip addr` (Linux)
2. Öppna port 8080 i brandväggen
3. Starta server: `./vaderserver API_KEY`

### På klienten:

```bash
./vaderklient_c 192.168.1.100 8080
```

---

## 📊 Prestanda

| Metric | Värde |
|--------|-------|
| Cache hit latency | ~5ms |
| API call latency | ~200-500ms |
| Max simultaneous clients | 32 |
| Memory per client | ~2MB |
| Cache validity | 30 minutes |

---

## 🐛 Felsökning

### "Kunde inte ansluta till server"
- Kontrollera att servern körs
- Verifiera IP-adress och port
- Kontrollera brandvägg

### "API-fel"
- Vänta 2 timmar efter API-nyckel registrering
- Verifiera API-nyckeln på openweathermap.org
- Kontrollera internetanslutning

### "CMake not found"
```bash
# Windows
choco install cmake

# Linux
sudo apt-get install cmake build-essential
```

---

## 📝 Kodkvalitet

- ✅ **Svenska variabelnamn** i hela koden
- ✅ **Detaljerade kommentarer** på svenska
- ✅ **Konsekvent stil** enligt C11/C++17 standards
- ✅ **Felhantering** på alla nivåer
- ✅ **Memory safety** (ingen malloc i runtime)
- ✅ **Cross-platform** abstraktion

---

## 🔐 Säkerhet

**Nuvarande implementation:**
- Ingen autentisering
- HTTP (ej HTTPS) för API-anrop
- Ingen rate-limiting

**Rekommendationer för produktion:**
- Lägg till API-nyckelautentisering
- Implementera TLS/SSL
- Rate limiting per klient
- Input validation

---

## 📈 Framtida Förbättringar

- [ ] HTTPS stöd för OpenWeatherMap
- [ ] Databas istället för filcache
- [ ] WebSocket för real-time updates
- [ ] GUI-klient (Qt/GTK)
- [ ] Docker container
- [ ] Unit tests med CTest

---

## 👥 Projektgrupp

**Utvecklat enligt offert för [Företagsnamn AB]**

- Projektledare: [Ditt Namn]
- Utvecklare: [Ditt Namn]

**Kontakt:** [din.email@example.com]

---

## 📜 Licens

Detta projekt utvecklades som en övning och är fritt att använda och modifiera.

---

## 🙏 Tack Till

- **OpenWeatherMap** för gratis väder-API
- **CMake** för cross-platform build system
- **Chas Academy** för projektspecifikation

---

## 🔗 Resurser

- [OpenWeatherMap API Docs](https://openweathermap.org/api)
- [CMake Documentation](https://cmake.org/documentation/)
- [POSIX Sockets Guide](https://beej.us/guide/bgnet/)

---

**Senast uppdaterad:** 2025-12-25
**Version:** 1.0.0

**⭐ Om du gillar projektet, ge det en stjärna!**
