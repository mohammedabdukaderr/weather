# 🧪 Test-instruktioner för Vädersystemet

## ⚠️ Förutsättningar

För att kunna bygga och testa projektet behöver du:

### Windows:
```cmd
# Installera MinGW (GCC för Windows)
# Ladda ner från: https://sourceforge.net/projects/mingw-w64/

# ELLER installera via Chocolatey:
choco install mingw

# Verifiera installation:
gcc --version
make --version
```

### Linux:
```bash
sudo apt-get update
sudo apt-get install build-essential gcc make
```

---

## 🏗️ Bygginstruktioner

### Alternativ 1: Med Make (Rekommenderas)

```bash
cd "c:\Users\moham\Downloads\mohammed\vädersystem\Server"

# Rensa gamla filer
make clean

# Bygg servern
make

# Kör servern med test API-nyckel
make run
```

### Alternativ 2: Manuell kompilering med GCC

```bash
cd "c:\Users\moham\Downloads\mohammed\vädersystem\Server"

# Kompilera alla filer
gcc -Wall -Wextra -Iinclude -g -std=c11 -c src/loggning.c -o src/loggning.o
gcc -Wall -Wextra -Iinclude -g -std=c11 -c src/http_server.c -o src/http_server.o
gcc -Wall -Wextra -Iinclude -g -std=c11 -c src/tcp_server.c -o src/tcp_server.o
gcc -Wall -Wextra -Iinclude -g -std=c11 -c src/cache.c -o src/cache.o
gcc -Wall -Wextra -Iinclude -g -std=c11 -c src/json_helper.c -o src/json_helper.o
gcc -Wall -Wextra -Iinclude -g -std=c11 -c src/vader_api.c -o src/vader_api.o
gcc -Wall -Wextra -Iinclude -g -std=c11 -c src/main.c -o src/main.o

# Länka ihop allt
gcc -Wall -Wextra -Iinclude -g -std=c11 -o vaderserver.exe src/main.o src/loggning.o src/http_server.o src/tcp_server.o src/cache.o src/json_helper.o src/vader_api.o -lws2_32
```

---

## 🚀 Starta Servern

### Skaffa API-nyckel först:
1. Gå till https://openweathermap.org/api
2. Skapa gratis konto
3. Hämta din API-nyckel (kan ta upp till 2 timmar att aktiveras)

### Kör servern:
```bash
# Med standard port (8080) och INFO-loggning
./vaderserver.exe DIN_API_NYCKEL

# Med egen port och DEBUG-loggning
./vaderserver.exe DIN_API_NYCKEL 9000 0

# Loggningsnivåer:
# 0 = DEBUG (mycket detaljer)
# 1 = INFO (standard)
# 2 = VARNING
# 3 = FEL (minimal output)
```

**Förväntad output när servern startar:**
```
[2025-12-25 16:00:00] [INFO] main.c:245 - === VÄDERSERVER STARTAR ===
[2025-12-25 16:00:00] [INFO] tcp_server.c:45 - TCP-server initierad på port 8080
[2025-12-25 16:00:00] [INFO] main.c:250 - Servern lyssnar på http://localhost:8080
[2025-12-25 16:00:00] [INFO] main.c:251 - Tryck Ctrl+C för att stoppa
[2025-12-25 16:00:00] [INFO] tcp_server.c:88 - Väntar på klientanslutningar...
```

---

## 🧪 Testa med olika klienter

### Test 1: HTTP GET med curl

```bash
# Öppna nytt terminal-fönster

# Testa aktuellt väder för Stockholm
curl "http://localhost:8080/weather?city=Stockholm&country=SE"

# Testa väderprognos
curl "http://localhost:8080/forecast?city=Stockholm&country=SE"
```

**Förväntad JSON-respons:**
```json
{
  "stad": "Stockholm",
  "temperatur": 15.5,
  "luftfuktighet": 65,
  "vindhastighet": 3.2,
  "lufttryck": 1013,
  "beskrivning": "lätt regn",
  "ikon_id": "10d",
  "tidsstampel": 1703520000
}
```

### Test 2: C-klienten

```bash
# Bygg C-klienten först
cd "c:\Users\moham\Downloads\mohammed\vädersystem\klient_c"

# Kompilera
gcc -Wall -Wextra -Iinclude -I../gemensam/include -std=c11 src/main.c -o vaderklient_c.exe -lws2_32

# Kör klienten
./vaderklient_c.exe localhost 8080
```

**Förväntad klient-output:**
```
╔═══════════════════════════════════════════════════════╗
║          VÄDERSYSTEM - C-KLIENT                       ║
╚═══════════════════════════════════════════════════════╝

Ansluten till server: localhost:8080

Välj alternativ:
  1. Hämta aktuellt väder
  2. Hämta väderprognos
  3. Avsluta
Val: 1

Ange stad: Stockholm
Ange landskod (ex: SE, US, GB): SE

Skickar förfrågan...

╔═══════════════════════════════════════════════════════╗
║             AKTUELLT VÄDER - Stockholm                ║
╠═══════════════════════════════════════════════════════╣
║  🌡️  Temperatur:    15.5°C                            ║
║  💧  Luftfuktighet:  65%                              ║
║  💨  Vindhastighet:  3.2 m/s                          ║
║  📊  Lufttryck:      1013 hPa                         ║
║  ☁️  Beskrivning:    lätt regn                        ║
╚═══════════════════════════════════════════════════════╝
```

### Test 3: C++-klienten

```bash
cd "c:\Users\moham\Downloads\mohammed\vädersystem\klient_cpp"

# Kompilera
g++ -Wall -Wextra -Iinclude -I../gemensam/include -std=c++17 src/main.cpp -o vaderklient_cpp.exe -lws2_32

# Kör
./vaderklient_cpp.exe localhost 8080
```

---

## ✅ Verifikationstester

### Test 1: Cache-funktionalitet

1. Starta servern
2. Hämta väder för Stockholm (första gången)
   - Servern bör logga: "Hämtar från OpenWeatherMap API"
3. Hämta väder för Stockholm igen (inom 30 min)
   - Servern bör logga: "Använder cachad data"

**Server-loggar att kontrollera:**
```
[INFO] vader_api.c:123 - Hämtar väder från API: Stockholm, SE
[INFO] cache.c:67 - Cachad data sparad för: Stockholm_SE
[INFO] cache.c:45 - Cachad data hittad för: Stockholm_SE
[INFO] main.c:178 - Använder cachad data
```

### Test 2: Felhantering

#### Ogiltig stad:
```bash
curl "http://localhost:8080/weather?city=XYZ123&country=XX"
```

**Förväntad respons:**
```json
HTTP/1.1 404 Not Found
{"felmeddelande": "Kunde inte hämta väderdata"}
```

#### Saknad parameter:
```bash
curl "http://localhost:8080/weather?city=Stockholm"
```

**Förväntad respons:**
```json
HTTP/1.1 400 Bad Request
{"felmeddelande": "Saknas required parameters: city och country"}
```

### Test 3: Flera samtidiga klienter

```bash
# Terminal 1:
curl "http://localhost:8080/weather?city=Stockholm&country=SE" &

# Terminal 2:
curl "http://localhost:8080/weather?city=London&country=GB" &

# Terminal 3:
curl "http://localhost:8080/weather?city=Paris&country=FR" &
```

**Servern bör hantera alla förfrågningar parallellt och logga:**
```
[INFO] tcp_server.c:95 - Ny klient ansluten från: 127.0.0.1:xxxxx
[INFO] tcp_server.c:95 - Ny klient ansluten från: 127.0.0.1:xxxxx
[INFO] tcp_server.c:95 - Ny klient ansluten från: 127.0.0.1:xxxxx
```

### Test 4: Server shutdown

```bash
# Tryck Ctrl+C i server-terminalen
```

**Förväntad output:**
```
[INFO] main.c:245 - Mottog stoppSignal, stänger servern...
[INFO] tcp_server.c:112 - Stänger TCP-server...
[INFO] loggning.c:108 - Loggningssystem stängt
```

---

## 🐛 Felsökning

### Problem: "Address already in use"
**Lösning:**
```bash
# Hitta processen som använder porten
netstat -ano | findstr :8080

# Döda processen (Windows)
taskkill /PID <PID> /F
```

### Problem: "Could not connect to OpenWeatherMap API"
**Lösningar:**
1. Kontrollera internetanslutning
2. Vänta 2 timmar efter API-nyckel registrering
3. Verifiera API-nyckeln på openweathermap.org
4. Kontrollera brandväggsinställningar

### Problem: "Kunde inte skapa socket"
**Lösning:**
```bash
# Windows: Kör som administratör
# Linux: Kontrollera brandvägg
sudo ufw allow 8080
```

### Problem: Cache växer för stort
**Lösning:**
```bash
# Rensa cache-mappen manuellt
rm -rf weather_cache/*

# Eller modifiera CACHE_GILTIGHETSTID i konfiguration.h
```

---

## 📊 Förväntat Resultat

När alla tester passerar bör du se:

✅ Server startar utan fel
✅ Loggfil skapas: `vaderserver.log`
✅ Cache-mapp skapas: `weather_cache/`
✅ HTTP-förfrågningar ger JSON-svar
✅ Cache fungerar (andra anropet snabbare)
✅ Flera klienter kan ansluta samtidigt
✅ Servern stängs ned snyggt med Ctrl+C

---

## 📝 Kodverifiering Utförd

Följande har verifierats utan kompilator:

✅ Alla include-filer finns i `include/`
✅ Alla källfiler finns i `src/`
✅ Makefile uppdaterad för ny struktur
✅ Svenska variabelnamn genom hela koden
✅ Utförliga kommentarer på svenska
✅ Korrekt include-guards i header-filer
✅ Konsistent kodstil (C11/C++17)
✅ Funktionsdeklarationer matchar definitioner

---

## 🔍 Nästa Steg

För att faktiskt köra programmet:

1. **Installera GCC/MinGW** (se förutsättningar ovan)
2. **Bygg projektet** med Make eller manuellt
3. **Skaffa API-nyckel** från OpenWeatherMap
4. **Kör testerna** enligt instruktionerna ovan
5. **Rapportera eventuella fel** så fixar vi dem

---

**Skapad:** 2025-12-25
**Testmiljö:** Windows 11 med MinGW
**Alternativ miljö:** Linux med GCC
