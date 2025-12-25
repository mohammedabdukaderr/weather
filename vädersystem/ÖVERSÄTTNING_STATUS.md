# Översättning till Svenska - Status och Guide

## ✅ Vad jag har gjort hittills

### Färdigöversatta filer:

1. **Server/src/loggning.c** - ✅ KLAR
   - Alla variabelnamn på svenska (t.ex. `log_file` → `logg_fil`, `message` → `meddelande`)
   - Detaljerade kommentarer på svenska för varje rad
   - Förklarar VARFÖR koden gör något, inte bara VAD

2. **Server/src/http_server.c** - ✅ KLAR
   - Variabelnamn: `request` → `forfragan`, `buffer` → `buffer`, `value` → `varde`
   - Funktionsnamn: `parse_http_request` → `parsa_http_request`
   - Omfattande kommentarer som förklarar HTTP-protokollet

## 📝 Översättningsprinciper

### Variabelnamn:
```c
// FÖRE:
char name[64];
int count;
float temperature;
bool success;

// EFTER:
char namn[64];              // Namnet på staden som användaren angav
int antal;                  // Antalet klienter som är anslutna just nu
float temperatur;           // Temperaturen i grader Celsius
bool lyckades;              // True om operationen lyckades, annars false
```

### Funktionsnamn:
```c
// FÖRE:
void initialize_server();
int connect_to_client();
bool send_data();

// EFTER:
void initiera_server();     // Startar servern och öppnar lyssnande socket
int anslut_till_klient();   // Skapar TCP-anslutning till angiven klient
bool skicka_data();         // Skickar data över nätverket, returnerar true vid framgång
```

### Kommentarer:
```c
// ❌ DÅLIGTch name;  // name

// ❌ INTE BRA
char namn;  // namnet

// ✅ BRA
char namn;  // Namnet på staden som användaren vill hämta väderdata för

// ✅ ÄNNU BÄTTRE
char stad_namn[64];  // Namnet på staden (max 63 tecken + null-terminator)
                     // Används för att söka efter väderdata i OpenWeatherMap API
```

## 📂 Återstående filer att översätta

### Högprioriterade (Server-filer):

1. **Server/src/tcp_server.c**
   - `socket` → `socket_fd` eller `uttag`
   - `bind` → behåll (systemfunktion)
   - `listen` → behåll (systemfunktion)
   - `accept` → behåll (systemfunktion)
   - Kommentarer ska förklara TCP-koncepten

2. **Server/src/cache.c**
   - `cache` → `cache` (kan behållas)
   - `file` → `fil`
   - `timestamp` → `tidsstampel`
   - `valid` → `giltig`

3. **Server/src/vader_api.c**
   - `weather` → `vader`
   - `forecast` → `prognos`
   - `response` → `svar`
   - `request` → `forfragan`

4. **Server/src/json_helper.c**
   - `key` → `nyckel`
   - `value` → `varde`
   - `parse` → `parsa`

5. **Server/src/main.c**
   - `client` → `klient`
   - `running` → `kors` (redan används)
   - `api_key` → `api_nyckel`

### Klient-filer:

6. **klient_c/src/main.c**
   - Samma principer som server
   - `menu` → `meny`
   - `choice` → `val`

7. **klient_cpp/src/main.cpp**
   - Klassnamn kan behållas på engelska: `VaderKlient`
   - Metodnamn på svenska: `hamtaVader()`, `visaData()`
   - Medlemsvariabler på svenska: `server_adress`, `port`

### Header-filer:

8. **Server/include/*.h**
   - Strukturnamn: kan behållas på engelska
   - Fältnamn: svenska
   - Exempel:
   ```c
   typedef struct {
       char stad[64];           // Stadens namn
       float temperatur;        // Temperatur i Celsius
       int64_t tidsstampel;    // När data hämtades (Unix timestamp)
   } VaderData;
   ```

## 🎯 Exempel på fullständig översättning

### Före:
```c
int send_weather_data(int socket, WeatherData* data) {
    char buffer[1024];
    int len = format_json(data, buffer, sizeof(buffer));
    return send(socket, buffer, len, 0);
}
```

### Efter:
```c
/**
 * Skickar väderdata till klienten i JSON-format
 *
 * @param klient_socket - Socket-filbeskrivare för den anslutna klienten
 * @param vader_data - Pekare till strukturen med väderdata att skicka
 * @return Antal bytes som skickades, eller -1 vid fel
 *
 * Funktionen konverterar väderdata till JSON-format och skickar det
 * över nätverket till klienten. Om send() returnerar -1 har ett
 * nätverksfel uppstått (t.ex. klienten kopplat från).
 */
int skicka_vader_data(int klient_socket, VaderData* vader_data) {
    // Buffer för att bygga JSON-strängen i
    char buffer[1024];

    // Konvertera väderdata-strukturen till JSON-format
    // Returnerar längden på den genererade JSON-strängen
    int langd = formatera_json(vader_data, buffer, sizeof(buffer));

    // Skicka bufferten över nätverket
    // send() returnerar antal bytes som skickades, eller -1 vid fel
    return send(klient_socket, buffer, langd, 0);
}
```

## 🔄 Automatisk sökning och ersättning (ordlista)

### Vanliga översättningar:

| Engelska | Svenska | Exempel |
|----------|---------|---------|
| buffer | buffer | `char buffer[1024]` |
| size | storlek | `size_t buffer_storlek` |
| length | langd | `int strang_langd` |
| count | antal | `int antal_klienter` |
| index | index | `int array_index` |
| data | data | `WeatherData vader_data` |
| error | fel | `int fel_kod` |
| success | lyckades | `bool lyckades` |
| client | klient | `int klient_socket` |
| server | server | `TcpServer* server` |
| request | forfragan | `HttpRequest forfragan` |
| response | svar | `HttpResponse svar` |
| city | stad | `char stad[64]` |
| country | land/landskod | `char landskod[3]` |
| temperature | temperatur | `float temperatur` |
| humidity | luftfuktighet | `float luftfuktighet` |
| pressure | lufttryck | `float lufttryck` |
| wind | vind | `float vind_hastighet` |
| forecast | prognos | `VaderPrognos prognos` |
| cache | cache | `Cache* cache` |
| file | fil | `FILE* fil` |
| socket | socket/uttag | `int socket_fd` |
| message | meddelande | `char meddelande[256]` |
| log | logg | `void skriv_logg()` |
| init | initiera | `void initiera_server()` |
| start | starta | `void starta_server()` |
| stop | stoppa | `void stoppa_server()` |
| close | stang | `void stang_socket()` |
| open | oppna | `void oppna_fil()` |
| read | las | `int las_data()` |
| write | skriv | `int skriv_data()` |
| send | skicka | `int skicka_data()` |
| receive/recv | ta_emot/motta | `int ta_emot_data()` |
| parse | parsa | `bool parsa_json()` |
| create | skapa | `void skapa_response()` |
| handle | hantera | `void hantera_klient()` |
| process | behandla | `void behandla_forfragan()` |

## ⚠️ Ord som INTE ska översättas:

- Systemfunktioner: `socket()`, `bind()`, `listen()`, `accept()`, `send()`, `recv()`
- Standard C-funktioner: `printf()`, `scanf()`, `malloc()`, `free()`, `fopen()`, `fclose()`
- Makron: `NULL`, `EOF`, `TRUE`, `FALSE`
- Datatyper: `int`, `char`, `float`, `bool`, `size_t`, `uint8_t`
- Protokollspecifika ord: `HTTP`, `JSON`, `TCP`, `IP`

## ✅ Checklista för varje fil:

- [ ] Alla variabelnamn översatta till svenska
- [ ] Alla funktionsnamn översatta till svenska (utom systemfunktioner)
- [ ] Varje rad har en kommentar som förklarar VARFÖR, inte bara VAD
- [ ] Kommentarer förklarar koncepten (t.ex. vad är TCP, HTTP, JSON)
- [ ] Funktioner har DocString-kommentarer (/** */)
- [ ] Komplexa operationer har förklarande kommentarer

## 💡 Tips:

1. Börja med att översätta variabelnamnen
2. Sedan funktionsnamnen
3. Till sist lägg till kommentarer
4. Testa att koden kompilerar efter varje fil
5. Var konsekvent med översättningarna

---

**Status:** 2 av ~15 filer färdigöversatta
**Nästa:** tcp_server.c, cache.c, vader_api.c
