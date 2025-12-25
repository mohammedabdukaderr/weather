# Vädersystem - Komplett Lösning

**Version:** 1.0.0
**Projekttyp:** C/C++ Server-Klient System
**API:** OpenWeatherMap
**Plattform:** Windows & Linux (Cross-platform)

---

## 📋 Projektöversikt

Detta vädersystem är en komplett lösning utvecklad enligt offertspecifikation. Systemet består av:

- **Server (C)**: TCP-server som hanterar flera samtidiga klienter, hämtar väderdata från OpenWeatherMap API och cachear resultat
- **C-klient**: Terminal-baserad klient skriven i C
- **C++-klient**: Objektorienterad terminal-klient skriven i C++
- **Cache-system**: Lokal filbaserad cache för att minska API-anrop
- **Cross-platform**: Fungerar på både Windows och Linux

---

## 🏗️ Projektstruktur

```
vädersystem/
├── Server/                    # Serverapplikation
│   ├── src/
│   │   ├── main.c            # Huvudprogram
│   │   ├── tcp_server.c      # TCP socket-hantering
│   │   ├── vader_api.c       # OpenWeatherMap integration
│   │   ├── cache.c           # Cache-system
│   │   ├── json_helper.c     # JSON-parser
│   │   └── loggning.c        # Loggningssystem
│   └── include/              # Header-filer
│
├── klient_c/                  # C-klient
│   └── src/
│       └── main.c
│
├── klient_cpp/                # C++-klient
│   └── src/
│       └── main.cpp
│
├── gemensam/                  # Gemensamma headers
│   └── include/
│       ├── vaderprotokoll.h  # Protokolldefinitioner
│       └── natverks_abstraktion.h  # Platform-abstraktion
│
├── tests/                     # Testprogram
│   └── test_protokoll.c
│
└── docs/                      # Dokumentation
    ├── README.md
    ├── API_GUIDE.md
    └── ANVANDNING.md
```

---

## 🚀 Snabbstart

### Förutsättningar

- **CMake** 3.15 eller senare
- **GCC/MSVC** kompilator
- **OpenWeatherMap API-nyckel** (gratis på https://openweathermap.org/api)

### Bygga Projektet

#### Windows (Med CMake och MinGW/MSVC):

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

#### Linux:

```bash
mkdir build
cd build
cmake ..
make
```

---

## 📡 Använda Systemet

### 1. Skaffa OpenWeatherMap API-Nyckel

1. Gå till [OpenWeatherMap](https://openweathermap.org/api)
2. Skapa gratis konto
3. Gå till "API keys" i din profil
4. Kopiera din API-nyckel

### 2. Starta Servern

```bash
# Windows
cd build\Server
vaderserver.exe <DIN-API-NYCKEL>

# Linux
cd build/Server
./vaderserver <DIN-API-NYCKEL>
```

**Valfria parametrar:**
- Port (standard: 8080): `./vaderserver <API-NYCKEL> 9000`
- Lognivå (0-3, standard: 1): `./vaderserver <API-NYCKEL> 8080 0`

### 3. Starta C-Klient

```bash
# Windows
cd build\klient_c
vaderklient_c.exe

# Linux
cd build/klient_c
./vaderklient_c
```

**Med serveradress:**
```bash
./vaderklient_c 192.168.1.100 8080
```

### 4. Starta C++-Klient

```bash
# Windows
cd build\klient_cpp
vaderklient_cpp.exe

# Linux
cd build/klient_cpp
./vaderklient_cpp
```

---

## 🧪 Köra Tester

```bash
cd build/tests
./test_protokoll
```

---

## 🔧 Konfiguration

### Server-konfiguration

Redigera [Server/include/konfiguration.h](../Server/include/konfiguration.h):

```c
#define SERVER_PORT 8080              // TCP-port
#define MAX_KLIENTER 32               // Max samtidiga klienter
#define CACHE_GILTIGHETSTID 1800      // Cache giltighet (30 min)
```

### Loggningsnivåer

- **0 (DEBUG)**: Detaljerad information
- **1 (INFO)**: Allmän information (standard)
- **2 (VARNING)**: Varningar
- **3 (FEL)**: Endast fel

---

## 📊 Funktioner

### Server

✅ TCP socket-server med flera samtidiga klienter
✅ OpenWeatherMap API-integration
✅ JSON-parser för väderdata
✅ Filbaserat cache-system (30 min TTL)
✅ Omfattande loggning till fil och konsol
✅ Felhantering och timeout
✅ Cross-platform (Windows/Linux)

### Klienter

✅ Interaktivt menysystem
✅ Hämta aktuellt väder
✅ Hämta väderprognos
✅ Formaterad utskrift i terminalen
✅ Felhantering

### Cache-system

✅ Lokal filcache i `./cache/`
✅ Automatisk utgångskontroll (30 min)
✅ Periodisk rensning av gamla filer
✅ Minskar API-anrop och svarstid

---

## 🌐 Protokoll

Systemet använder ett binärt protokoll över TCP:

### Meddelandetyper

1. **MEDDELANDE_HAMTA_VADER** (1): Begär aktuellt väder
2. **MEDDELANDE_HAMTA_PROGNOS** (2): Begär prognos
3. **MEDDELANDE_VADER_SVAR** (3): Svar med väderdata
4. **MEDDELANDE_FEL** (4): Felmeddelande

### Datastrukturer

Se [vaderprotokoll.h](../gemensam/include/vaderprotokoll.h) för fullständig specifikation.

---

## 🐛 Felsökning

### Problem: "Kunde inte ansluta till server"

**Lösning:**
- Kontrollera att servern körs
- Verifiera IP-adress och port
- Kontrollera brandvägg

### Problem: "API-fel"

**Lösning:**
- Verifiera att API-nyckeln är korrekt
- Kontrollera internetanslutning
- Kontrollera att staden finns i OpenWeatherMap

### Problem: "Compilation errors"

**Lösning:**
- Kontrollera att CMake 3.15+ är installerat
- Verifiera att kompilator finns i PATH
- Kör `cmake .. -G "MinGW Makefiles"` på Windows med MinGW

---

## 📈 Prestanda

- **Cache-träff**: ~5ms responstid
- **API-anrop**: ~200-500ms (beroende på nätverk)
- **Max samtidiga klienter**: 32 (konfigurerbart)
- **Memory footprint**: ~2MB per klient

---

## 📝 Licens

Detta projekt utvecklades som en övning och är fritt att använda och modifiera.

---

## 👥 Projektgrupp

**Utvecklat enligt offert för [Företagsnamn AB]**

- Projektledare: [Ditt Namn]
- Utvecklare: [Ditt Namn]

Email: [din.email@example.com]

---

## 🔗 Externa Beroenden

- **OpenWeatherMap API**: https://openweathermap.org/api
- **CMake**: https://cmake.org/
- **Standard POSIX/Winsock**: Inga externa bibliotek behövs

---

## 📚 Ytterligare Dokumentation

- [API Guide](API_GUIDE.md) - Detaljerad API-dokumentation
- [Användning](ANVANDNING.md) - Steg-för-steg användarguide
- [Arkitektur](ARKITEKTUR.md) - Systemarkitektur och design

---

**Senast uppdaterad:** 2025-12-25
