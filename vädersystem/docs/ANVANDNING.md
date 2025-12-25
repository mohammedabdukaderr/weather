# Vädersystem - Användarguide

En steg-för-steg guide för att komma igång med vädersystemet.

---

## Del 1: Skaffa OpenWeatherMap API-Nyckel

### Steg 1: Skapa Konto

1. Öppna webbläsare och gå till: **https://openweathermap.org/api**
2. Klicka på **"Sign Up"** (uppe till höger)
3. Fyll i:
   - Username (användarnamn)
   - Email
   - Password (lösenord)
4. Bekräfta via email

### Steg 2: Få Din API-Nyckel

1. Logga in på OpenWeatherMap
2. Gå till: **https://home.openweathermap.org/api_keys**
3. Kopiera din **"Key"** (lång sträng med bokstäver och siffror)
4. Spara den säkert (du behöver den för att starta servern)

**Exempel API-nyckel:**
```
a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6
```

**OBS:** Det kan ta upp till 2 timmar innan nyckeln aktiveras!

---

## Del 2: Bygga Projektet

### Windows (Med MinGW eller Visual Studio)

#### Alternativ A: MinGW

1. Öppna Command Prompt
2. Navigera till projektmappen:
```cmd
cd C:\Users\moham\Downloads\mohammed\vädersystem
```

3. Skapa build-katalog:
```cmd
mkdir build
cd build
```

4. Generera byggsystem:
```cmd
cmake .. -G "MinGW Makefiles"
```

5. Bygg projektet:
```cmd
mingw32-make
```

#### Alternativ B: Visual Studio

1. Öppna Command Prompt
2. Navigera till projektet och skapa build:
```cmd
cd C:\Users\moham\Downloads\mohammed\vädersystem
mkdir build
cd build
cmake ..
```

3. Bygg med Visual Studio:
```cmd
cmake --build . --config Release
```

---

### Linux

1. Öppna terminal
2. Navigera till projektet:
```bash
cd /path/to/vädersystem
```

3. Bygg projektet:
```bash
mkdir build
cd build
cmake ..
make
```

**Troubleshooting Linux:**
Om du får "cmake not found":
```bash
sudo apt-get install cmake build-essential
```

---

## Del 3: Starta Servern

### Windows

1. Öppna Command Prompt
2. Gå till server-katalogen:
```cmd
cd C:\Users\moham\Downloads\mohammed\vädersystem\build\Server
```

3. Starta servern med din API-nyckel:
```cmd
vaderserver.exe a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6
```

**Ersätt** `a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6` med din riktiga API-nyckel!

### Linux

```bash
cd build/Server
./vaderserver a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6
```

### Förväntad Output

```
[2025-12-25 15:30:00] [INFO] main.c:123 - === Vädersystem Server ===
[2025-12-25 15:30:00] [INFO] main.c:124 - Startar server på port 8080
[2025-12-25 15:30:00] [INFO] cache.c:45 - Skapade cache-katalog: ./cache
[2025-12-25 15:30:00] [INFO] tcp_server.c:78 - TCP-server lyssnar på port 8080
[2025-12-25 15:30:00] [INFO] main.c:156 - Server lyssnar på port 8080. Tryck Ctrl+C för att stoppa.
```

**Servern körs nu!** Lämna terminalfönstret öppet.

---

## Del 4: Använda C-Klienten

### Starta Klienten

**Windows:**
```cmd
cd C:\Users\moham\Downloads\mohammed\vädersystem\build\klient_c
vaderklient_c.exe
```

**Linux:**
```bash
cd build/klient_c
./vaderklient_c
```

### Interaktiv Användning

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

Ansluter till server 127.0.0.1:8080...
Ansluten till server!

╔═══════════════════════════════════════════════════════╗
║             AKTUELLT VÄDER - Stockholm
╠═══════════════════════════════════════════════════════╣
║  🌡️  Temperatur:    15°C
║  💧  Luftfuktighet:  65%
║  💨  Vindhastighet:  3.2 m/s
║  📊  Lufttryck:      1013 hPa
║  ☁️  Beskrivning:    molnigt
╚═══════════════════════════════════════════════════════╝
```

### Hämta Väder för Andra Städer

**Svenska städer:**
- Stockholm, SE
- Göteborg, SE
- Malmö, SE
- Uppsala, SE

**Internationella städer:**
- London, GB
- Paris, FR
- New York, US
- Tokyo, JP
- Berlin, DE

### Avsluta Klienten

```
Val: 3

Hejdå!
```

---

## Del 5: Använda C++-Klienten

C++-klienten fungerar identiskt med C-klienten men är skriven i C++ med objektorienterad design.

**Windows:**
```cmd
cd C:\Users\moham\Downloads\mohammed\vädersystem\build\klient_cpp
vaderklient_cpp.exe
```

**Linux:**
```bash
cd build/klient_cpp
./vaderklient_cpp
```

Användningen är samma som C-klienten.

---

## Del 6: Ansluta från Annan Dator

### På Serverdatorn

1. Hitta din IP-adress:

**Windows:**
```cmd
ipconfig
```
Leta efter "IPv4 Address" (ex: 192.168.1.100)

**Linux:**
```bash
ip addr show
```
eller
```bash
hostname -I
```

2. Starta servern (se Del 3)

3. Öppna brandvägg för port 8080:

**Windows:**
- Kontrollpanelen → Windows Defender-brandvägg
- Avancerade inställningar → Inkommande regler
- Ny regel → Port → TCP 8080

**Linux (ufw):**
```bash
sudo ufw allow 8080/tcp
```

### På Klientdatorn

Kör klienten med serveradressen:

```bash
# Ersätt 192.168.1.100 med serverns IP
./vaderklient_c 192.168.1.100 8080
```

---

## Del 7: Förstå Cache-systemet

### Så Fungerar Cachen

Servern sparar väderdata lokalt i `./cache/` katalogen:

```
cache/
├── Stockholm_SE_vader.cache
├── Paris_FR_vader.cache
└── London_GB_vader.cache
```

**Giltighetstid:** 30 minuter

**Första förfrågan:**
```
[2025-12-25 15:30:05] [INFO] cache.c:67 - Cache miss: ./cache/Stockholm_SE_vader.cache
[2025-12-25 15:30:05] [INFO] vader_api.c:123 - Hämtar väder för Stockholm, SE från OpenWeatherMap
(tar ~300ms)
```

**Andra förfrågan (inom 30 min):**
```
[2025-12-25 15:32:00] [INFO] cache.c:72 - Cache hit: ./cache/Stockholm_SE_vader.cache (ålder: 115 sekunder)
(tar ~5ms)
```

### Fördelar med Cache

- ⚡ **Snabbare:** 5ms istället för 300ms
- 💰 **Sparar API-kvot:** Gratis-nivån har 1000 anrop/dag
- 🌐 **Fungerar offline:** (inom cache-tiden)

---

## Del 8: Loggfiler

Servern loggar all aktivitet till `vaderserver.log`:

```log
[2025-12-25 15:30:00] [INFO] tcp_server.c:78 - TCP-server lyssnar på port 8080
[2025-12-25 15:30:05] [INFO] tcp_server.c:92 - Ny klient ansluten från 127.0.0.1:54321
[2025-12-25 15:30:05] [INFO] main.c:67 - Förfrågan om väder för: Stockholm, SE
[2025-12-25 15:30:05] [INFO] vader_api.c:145 - Parsade väder: Stockholm, 15.5°C, molnigt
[2025-12-25 15:30:05] [INFO] main.c:89 - Skickade väderdata till klient
```

**Ändra lognivå:**

```bash
# 0=DEBUG (mycket detaljer)
./vaderserver API_KEY 8080 0

# 1=INFO (standard)
./vaderserver API_KEY 8080 1

# 2=VARNING (endast varningar)
./vaderserver API_KEY 8080 2

# 3=FEL (endast fel)
./vaderserver API_KEY 8080 3
```

---

## Del 9: Felsökning

### Problem: "API-fel" från server

**Möjliga orsaker:**

1. **API-nyckel inte aktiverad än**
   - Vänta 2 timmar efter registrering
   - Kontrollera status på openweathermap.org

2. **Felaktig API-nyckel**
   - Kopiera nyckeln direkt från openweathermap.org
   - Kontrollera inga extra mellanslag

3. **Stad finns inte**
   - Försök med engelska stavning: "Gothenburg" istället för "Göteborg"
   - Lägg till landskod: "Stockholm, SE"

4. **Kvot uppnådd**
   - Gratis-nivå: 1000 anrop/dag
   - Vänta till nästa dag eller uppgradera

### Problem: "Kunde inte ansluta till server"

**Kontrollera:**

1. **Servern körs:**
```bash
# Servern ska visa "lyssnar på port 8080"
```

2. **Rätt IP och port:**
```bash
# Klient: ./vaderklient_c 127.0.0.1 8080
# Server ska köra på samma IP och port
```

3. **Brandvägg:**
- Windows: Tillåt vaderserver.exe
- Linux: `sudo ufw allow 8080/tcp`

### Problem: "Compilation error"

**CMake inte funnen:**
```bash
# Windows
choco install cmake

# Linux
sudo apt-get install cmake
```

**Kompilator saknas:**
```bash
# Linux
sudo apt-get install build-essential

# Windows: Installera MinGW eller Visual Studio
```

---

## Del 10: Stänga Av Systemet

### Stoppa Servern

**Tryck:** `Ctrl+C` i serverns terminal

```
^C
[2025-12-25 16:00:00] [INFO] main.c:178 - Mottog stoppSignal, stänger servern...
[2025-12-25 16:00:00] [INFO] tcp_server.c:105 - Stänger TCP-server
[2025-12-25 16:00:00] [INFO] main.c:195 - Server stoppad
```

### Rensa Cache

Om du vill radera all cachad data:

**Windows:**
```cmd
rmdir /s cache
```

**Linux:**
```bash
rm -rf cache/
```

---

## Tips & Tricks

### Automatisk Start av Server (Linux)

Skapa systemd service:

```bash
sudo nano /etc/systemd/system/vaderserver.service
```

```ini
[Unit]
Description=Väder Server
After=network.target

[Service]
Type=simple
User=youruser
WorkingDirectory=/path/to/build/Server
ExecStart=/path/to/build/Server/vaderserver YOUR_API_KEY
Restart=always

[Install]
WantedBy=multi-user.target
```

Aktivera:
```bash
sudo systemctl enable vaderserver
sudo systemctl start vaderserver
```

---

**Lycka till med ditt vädersystem!** 🌤️

**Support:** [din.email@example.com]
