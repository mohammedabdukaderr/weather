# 🔑 API-Nyckel Guide - OpenWeatherMap

## 📍 Snabbstart

Din API-nyckel behövs för att servern ska kunna hämta väderdata. Denna guide visar exakt hur du skaffar den.

---

## 🚀 Steg-för-Steg: Skaffa API-Nyckel

### Steg 1: Gå till OpenWeatherMap

Öppna din webbläsare och gå till:
```
https://openweathermap.org/api
```

### Steg 2: Skapa Gratis Konto

1. Klicka på **"Sign Up"** eller **"Get API Key"**
2. Eller gå direkt till: https://home.openweathermap.org/users/sign_up

### Steg 3: Fyll i Registreringsformuläret

Fyll i följande information:
- **Username:** Välj ett användarnamn (t.ex. "mittanvändarnamn")
- **Email:** Din email-adress
- **Password:** Ett starkt lösenord
- **Bekräfta password**

Kryssa i "I am 16 years old and over" och "I agree with Privacy Policy..."

Klicka på **"Create Account"**

### Steg 4: Bekräfta Din Email

1. Öppna din email-inkorg
2. Leta efter email från OpenWeatherMap med ämne "Please confirm your email"
3. Klicka på bekräftelselänken i emailet
4. Du kommer nu till din **Dashboard**

### Steg 5: Hitta Din API-Nyckel

Efter inloggning hamnar du på:
```
https://home.openweathermap.org/
```

**Alternativ A - Direkt från Dashboard:**
1. Du ser automatiskt din API-nyckel under "API keys"
2. Den ser ut ungefär så här: `abc123def456ghi789jkl012mno345pq`

**Alternativ B - Via API Keys-sidan:**
1. Klicka på din profil (uppe till höger)
2. Välj **"My API keys"** från menyn
3. Eller gå direkt till: https://home.openweathermap.org/api_keys

### Steg 6: Kopiera API-Nyckeln

1. Du ser en tabell med "Key" kolumnen
2. Din standard API-nyckel är redan skapad (namnet "Default")
3. Klicka på **kopiera-ikonen** eller markera och kopiera texten
4. API-nyckeln är 32 tecken lång (a-f, 0-9)

**Exempel på hur en API-nyckel ser ut:**
```
a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6
```
*(Detta är bara ett exempel, din kommer vara annorlunda)*

---

## ⏳ VIKTIGT: Väntetid!

### ⚠️ API-nyckeln är inte aktiv direkt!

Efter att du skapat ditt konto:
- **Väntetid:** Upp till **2 timmar** (oftast 10-30 minuter)
- **Status:** Under denna tid fungerar inte API-nyckeln

**Kontrollera status:**
1. Gå till: https://home.openweathermap.org/api_keys
2. Titta under kolumnen "Status"
3. **"Active"** = Redo att använda ✅
4. **"Processing"** eller tom = Vänta lite till ⏳

---

## 💡 Testa Din API-Nyckel

### Test 1: Direkt i Webbläsaren

Öppna denna URL i din webbläsare (byt ut `DIN_API_NYCKEL`):
```
http://api.openweathermap.org/data/2.5/weather?q=Stockholm,SE&appid=DIN_API_NYCKEL&units=metric&lang=sv
```

**Om den fungerar ser du:**
```json
{
  "coord": {"lon": 18.0686, "lat": 59.3293},
  "weather": [{"description": "lätt regn"}],
  "main": {
    "temp": 15.5,
    "humidity": 65,
    "pressure": 1013
  },
  "name": "Stockholm"
}
```

**Om den INTE fungerar ser du:**
```json
{
  "cod": 401,
  "message": "Invalid API key. Please see http://openweathermap.org/faq#error401 for more info."
}
```
→ **Lösning:** Vänta längre, API-nyckeln är inte aktiv än

### Test 2: Med curl (Terminal)

```bash
curl "http://api.openweathermap.org/data/2.5/weather?q=Stockholm,SE&appid=DIN_API_NYCKEL&units=metric&lang=sv"
```

---

## 🎯 Använda API-Nyckeln i Projektet

### Alternativ 1: Som Kommandoradsargument (Rekommenderas)

När du startar servern:
```bash
cd "c:\Users\moham\Downloads\mohammed\vädersystem\Server"
./vaderserver.exe a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6
```

Byt ut `a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6` mot din riktiga API-nyckel!

### Alternativ 2: Miljövariabel

**Windows:**
```cmd
set OPENWEATHER_API_KEY=a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6
./vaderserver.exe %OPENWEATHER_API_KEY%
```

**Linux/Mac:**
```bash
export OPENWEATHER_API_KEY=a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6
./vaderserver $OPENWEATHER_API_KEY
```

### Alternativ 3: Konfigurationsfil (För Utveckling)

Skapa en fil: `api_nyckel.txt`
```
a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6
```

Sedan i koden (vader_api.c):
```c
FILE* fil = fopen("api_nyckel.txt", "r");
if (fil) {
    fscanf(fil, "%s", api_nyckel);
    fclose(fil);
}
```

**⚠️ VIKTIGT:** Lägg ALDRIG api_nyckel.txt i git!

Lägg till i `.gitignore`:
```
api_nyckel.txt
*.key
```

---

## 📊 Gratis Plan Begränsningar

När du har gratis konto:

| Begränsning | Värde |
|-------------|-------|
| **API-anrop per minut** | 60 |
| **API-anrop per dag** | Obegränsat |
| **Data uppdatering** | Var 10:e minut |
| **Historisk data** | Nej |
| **Kostnad** | $0 (Gratis) |

**Tack vare cache-systemet:**
- Servern cachar data i 30 minuter
- Du använder MAX 48 API-anrop per dag per stad
- Du kan hantera många klienter utan problem

---

## 🔒 Säkerhet: Skydda Din API-Nyckel

### ✅ GÖR:
- ✅ Spara API-nyckeln i miljövariabler
- ✅ Använd `.gitignore` för nyckelfiler
- ✅ Dela aldrig API-nyckeln publikt
- ✅ Regenerera nyckeln om den läckt

### ❌ GÖR INTE:
- ❌ Hårdkoda API-nyckeln i källkoden
- ❌ Committa nyckeln till git
- ❌ Dela nyckeln på GitHub, Discord, etc.
- ❌ Inkludera nyckeln i screenshots

### Regenerera Komprometterad Nyckel:

Om din nyckel läckt:
1. Gå till: https://home.openweathermap.org/api_keys
2. Klicka på **🗑️ Delete** vid den gamla nyckeln
3. Klicka på **"Generate"** för att skapa ny nyckel
4. Uppdatera din server med nya nyckeln

---

## 🆙 Uppgradera Till Betald Plan (Valfritt)

Om du behöver mer:

### Professional Plan ($40/månad)
- 1,000 anrop/minut
- Minutuppdateringar
- Historisk data
- 5-dagars prognos med 3-timmars steg

### Enterprise Plan (Kontakta dem)
- Obegränsade anrop
- Dedikerad support
- SLA-garantier

**För detta projekt:** Gratis plan är mer än tillräcklig! 👍

---

## 🐛 Felsökning

### Problem 1: "401 Unauthorized"
**Orsak:** API-nyckeln är ogiltig eller inte aktiv
**Lösning:**
1. Vänta 2 timmar efter registrering
2. Kontrollera stavning (32 tecken, inga mellanslag)
3. Verifiera status på https://home.openweathermap.org/api_keys

### Problem 2: "429 Too Many Requests"
**Orsak:** Du har överskridit 60 anrop/minut
**Lösning:**
- Cache-systemet ska förhindra detta
- Vänta 1 minut och försök igen
- Kontrollera att cache fungerar (kolla loggarna)

### Problem 3: "404 Not Found - Stad"
**Orsak:** Staden finns inte i OpenWeatherMap-databasen
**Lösning:**
- Testa med engelska namn: "Stockholm" istället för "Sthlm"
- Inkludera landskod: `Stockholm,SE`
- Lista städer: https://openweathermap.org/find

### Problem 4: Kan inte hitta min API-nyckel
**Lösning:**
1. Logga in på: https://home.openweathermap.org/
2. Klicka på ditt användarnamn (uppe till höger)
3. Välj "My API keys"
4. Kopiera nyckeln under kolumnen "Key"

---

## 📱 Mobil App (Valfritt)

OpenWeatherMap har också en app där du kan se dina API-nycklar:
- **iOS:** https://apps.apple.com/app/id1552097524
- **Android:** https://play.google.com/store/apps/details?id=uk.co.openweather

---

## 🔗 Användbara Länkar

| Resurs | URL |
|--------|-----|
| **Skapa konto** | https://home.openweathermap.org/users/sign_up |
| **Logga in** | https://home.openweathermap.org/users/sign_in |
| **Mina API-nycklar** | https://home.openweathermap.org/api_keys |
| **API-dokumentation** | https://openweathermap.org/api |
| **Status-sida** | https://openweathermap.org/api/status |
| **FAQ** | https://openweathermap.org/faq |
| **Support** | https://home.openweathermap.org/questions |

---

## ✅ Checklista: Klar Att Använda

Bocka av dessa innan du startar servern:

- [ ] Skapat konto på OpenWeatherMap
- [ ] Bekräftat email-adress
- [ ] Hittat min API-nyckel på dashboard
- [ ] Kopierat hela API-nyckeln (32 tecken)
- [ ] Väntat minst 30 minuter (helst 2 timmar)
- [ ] Testat API-nyckeln i webbläsaren
- [ ] Sett JSON-svar (inte "401 Unauthorized")
- [ ] API-nyckel status är "Active"

**När alla är bockade → Du är redo att köra servern! 🎉**

---

## 📞 Exempel: Komplett Körning

```bash
# 1. Navigera till Server-mappen
cd "c:\Users\moham\Downloads\mohammed\vädersystem\Server"

# 2. Bygg servern (om inte redan gjort)
make

# 3. Starta med din API-nyckel
./vaderserver.exe a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6

# Förväntad output:
# [2025-12-25 16:00:00] [INFO] main.c:245 - === VÄDERSERVER STARTAR ===
# [2025-12-25 16:00:00] [INFO] tcp_server.c:45 - TCP-server initierad på port 8080
# [2025-12-25 16:00:00] [INFO] main.c:250 - Servern lyssnar på http://localhost:8080
```

---

**Lycka till! Om du fastnar, kolla FAQ eller kontakta OpenWeatherMap support. 🚀**

**Skapad:** 2025-12-25
**För projekt:** Vädersystem C/C++ Server-Klient
