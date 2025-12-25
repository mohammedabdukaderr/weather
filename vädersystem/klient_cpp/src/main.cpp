#include "natverks_abstraktion.h"
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>

using namespace std;

// ============================================================================
// VADERKLIENT-KLASS
// ============================================================================
// Denna klass kapslar in all funktionalitet för att kommunicera med
// väderservern. Objektorienterad programmering (OOP) ger oss:
// - Inkapsling: Data och funktioner grupperas tillsammans
// - Abstraktion: Användaren ser bara publika metoder
// - Återanvändbarhet: Enkelt att skapa flera klientinstanser

class VaderKlient {
private:
    // ========================================================================
    // PRIVATA MEDLEMSVARIABLER
    // ========================================================================
    // Dessa variabler är endast åtkomliga inom klassen
    // Detta skyddar data från oavsiktlig modifiering utifrån

    string server_adress;    // Server-IP (ex: "127.0.0.1")
    int server_port;         // Server-port (ex: 8080)

    // ========================================================================
    // PRIVATA HJÄLPMETODER FÖR JSON-PARSING
    // ========================================================================
    // Dessa metoder är endast tillgängliga inom klassen för intern användning
    // C++ string-klassen gör stränghantering enklare än i C

    /**
     * Parsar en textsträng från JSON
     *
     * SKILLNAD MOT C-VERSION:
     * - Använder C++ string istället för char-array
     * - string::find() istället för strstr()
     * - string::substr() för enkel extrahering av delsträng
     * - Inga manuella pekarberäkningar eller null-terminators
     *
     * FUNKTIONALITET:
     * 1. Bygger söksträng: "nyckel": "
     * 2. Hittar startposition med find()
     * 3. Hittar slutposition (nästa citattecken)
     * 4. Extraherar delsträng med substr()
     *
     * STRING::FIND():
     * - Returnerar positionen där strängen hittas
     * - Returnerar string::npos (konstant för "ej funnen") om ingen match
     *
     * STRING::SUBSTR():
     * - substr(start, längd) extraherar en delsträng
     * - Hanterar automatiskt minnesallokering
     *
     * @param json - JSON-strängen att söka i
     * @param nyckel - Namnet på JSON-nyckeln
     * @return Strängvärdet, eller tom sträng om ej funnen
     */
    string parse_json_string(const string& json, const string& nyckel) {
        // Bygg söksträng med strängkonkatenering (+ operatorn)
        string sokstrang = "\"" + nyckel + "\": \"";

        // Hitta var söksträng börjar i JSON
        size_t start_position = json.find(sokstrang);

        // Kontrollera om söksträng hittades
        // string::npos är en speciell konstant som betyder "ej funnen"
        if (start_position == string::npos) return "";

        // Flytta positionen förbi söksträng till själva värdet
        start_position += sokstrang.length();

        // Hitta avslutande citattecken från startpositionen
        // find(tecken, från_position) söker från given position
        size_t slut_position = json.find('"', start_position);

        // Kontrollera om avslutande citattecken hittades
        if (slut_position == string::npos) return "";

        // Extrahera och returnera delsträngen
        // substr(start, längd) - längd beräknas som slut - start
        return json.substr(start_position, slut_position - start_position);
    }

    /**
     * Parsar ett flyttal från JSON
     *
     * C++ STRING-KONVERTERING:
     * stof() = String TO Float (C++11 funktion)
     * Andra konverteringsfunktioner:
     * - stoi() = String TO Int
     * - stod() = String TO Double
     * - stol() = String TO Long
     *
     * FELHANTERING:
     * stof() kan kasta undantag vid ogiltigt format
     * Vi förlitar oss på att JSON från servern är korrekt formaterad
     * En robust version skulle använda try-catch
     *
     * @param json - JSON-strängen
     * @param nyckel - Namnet på JSON-nyckeln
     * @return Flyttalsvärdet, eller 0.0 om ej funnen
     */
    float parse_json_float(const string& json, const string& nyckel) {
        // Bygg söksträng: "nyckel": (utan citattecken för numeriskt värde)
        string sokstrang = "\"" + nyckel + "\": ";

        // Hitta position för söksträng
        size_t position = json.find(sokstrang);
        if (position == string::npos) return 0.0f;

        // Flytta position förbi söksträng till värdet
        position += sokstrang.length();

        // Extrahera resten av strängen från position och konvertera
        // stof() läser tills den stöter på icke-numeriskt tecken
        // Exempel: stof("23.5,\"key\"") returnerar 23.5
        return stof(json.substr(position));
    }

    // ========================================================================
    // PRIVATA METODER FÖR HTTP-KOMMUNIKATION
    // ========================================================================

    /**
     * Skickar en HTTP GET-request till servern
     *
     * C++ OSTRINGSTREAM:
     * ostringstream är en strömklass för att bygga strängar
     * Fungerar som cout, men output går till en sträng istället för skärmen
     * Fördelar:
     * - Automatisk konvertering av olika typer
     * - Enkel sammansättning av strängar
     * - Type-safe (kompilatorn kontrollerar typer)
     *
     * OPERATOR<<:
     * Vänstershift-operatorn (<<) är överlagrad för output-strömmar
     * "stream << värde" lägger till värde i strömmen
     * Kan kedjas: stream << värde1 << värde2 << värde3
     *
     * @param sock - Socket-descriptorn
     * @param sokvag - URL-sökvägen (ex: "/weather?city=Stockholm&country=SE")
     * @return true vid framgång, false vid fel
     */
    bool skicka_http_get(socket_t sock, const string& sokvag) {
        // Skapa en strängström för att bygga HTTP-requesten
        ostringstream forfragan;

        // Bygg HTTP GET-request genom att strömma in text
        // Kedjade <<-operatorer gör koden läsbar och effektiv
        forfragan << "GET " << sokvag << " HTTP/1.1\r\n"        // Request-rad
                  << "Host: localhost\r\n"                      // Host header
                  << "Connection: close\r\n"                    // Stäng efter svar
                  << "\r\n";                                    // Tom rad avslutar headers

        // Konvertera strömmen till en C++ string
        string forfragan_str = forfragan.str();

        // Skicka över socketen
        // c_str() konverterar C++ string till C-style char* (nödvändigt för send())
        // Returnera true om send() lyckades (returnerar positivt värde)
        return send(sock, forfragan_str.c_str(), (int)forfragan_str.length(), 0) > 0;
    }

    /**
     * Tar emot HTTP-svar och extraherar JSON-body
     *
     * MINNESHANTERING:
     * C++ string hanterar minne automatiskt via RAII (Resource Acquisition Is Initialization)
     * När string-objektet går ur scope, frigörs minnet automatiskt
     * Detta är säkrare än C's manuella minneshantering
     *
     * STRING-KONSTRUKTOR:
     * string(char*) skapar en C++ string från C-style char array
     * Kopierar data så vi kan säkert återanvända buffern
     *
     * @param sock - Socket-descriptorn
     * @return JSON-body som string, eller tom sträng vid fel
     */
    string ta_emot_http_svar(socket_t sock) {
        char buffer[8192];          // Stack-allokerad buffer för HTTP-svar (8 KB)
        int totalt_mottaget = 0;    // Räknare för totalt antal bytes
        int mottaget_denna_gang;    // Bytes från senaste recv()

        // Mottagningsloop: Ta emot data tills anslutningen stängs
        // recv() returnerar:
        // - Positivt tal: Antal mottagna bytes
        // - 0: Anslutningen stängdes av servern
        // - Negativt tal: Fel uppstod
        while ((mottaget_denna_gang = recv(sock, buffer + totalt_mottaget,
                                           sizeof(buffer) - totalt_mottaget - 1, 0)) > 0) {
            // Lägg till mottagna bytes till totalen
            totalt_mottaget += mottaget_denna_gang;

            // Säkerhetscheck: Stoppa om buffern är full
            // -1 för att spara plats för null-terminator
            if (totalt_mottaget >= (int)sizeof(buffer) - 1) break;
        }

        // Kontrollera om vi fick någon data
        if (totalt_mottaget <= 0) return "";

        // Null-terminera buffern så vi kan skapa en C++ string
        buffer[totalt_mottaget] = '\0';

        // Skapa C++ string från buffern
        // String-konstruktorn kopierar data från char array
        string http_svar(buffer);

        // Hitta var HTTP-body börjar (efter \r\n\r\n)
        // find() returnerar position, eller string::npos om ej funnen
        size_t body_start = http_svar.find("\r\n\r\n");
        if (body_start == string::npos) return "";

        // Extrahera och returnera JSON-body
        // substr(position) returnerar allt från position till slutet
        // +4 för att hoppa över \r\n\r\n (4 tecken)
        return http_svar.substr(body_start + 4);
    }

public:
    // ========================================================================
    // PUBLIKA METODER - KLASSENS GRÄNSSNITT
    // ========================================================================
    // Dessa metoder kan anropas utifrån och utgör klassens API

    /**
     * Konstruktor - Initialiserar VaderKlient-objektet
     *
     * KONSTRUKTORER I C++:
     * - Speciell metod som anropas när objekt skapas
     * - Samma namn som klassen
     * - Används för att initialisera medlemsvariabler
     * - Kan kasta undantag vid fel
     *
     * INITIALISERINGSLISTA:
     * : server_adress(adress), server_port(port)
     * Detta är den moderna C++ sättet att initialisera medlemmar
     * Fördelar över tilldelning i konstruktor-kropp:
     * - Mer effektivt (direkt initialisering istället för default + tilldelning)
     * - Nödvändigt för const och referensmedlemmar
     * - Klarare kod som visar initialisering
     *
     * THROW:
     * runtime_error är ett standardundantag för körtidsfel
     * Kastas om nätverksinitialisering misslyckas
     * Anroparen måste fånga detta med try-catch
     *
     * @param adress - Server-IP-adress
     * @param port - Server-portnummer
     * @throws runtime_error om nätverksinitialisering misslyckas
     */
    VaderKlient(const string& adress, int port)
        : server_adress(adress), server_port(port) {

        // Initialisera plattformsspecifikt nätverksbibliotek
        // Windows: WSAStartup för Winsock2
        // Linux: Gör ingenting
        if (initiera_natverksbibliotek() != 0) {
            // Kasta undantag vid fel
            // Detta avbryter objektskapandet
            throw runtime_error("Kunde inte initialisera nätverksbibliotek");
        }
    }

    /**
     * Destruktor - Städar upp när objektet förstörs
     *
     * DESTRUKTORER I C++:
     * - Anropas automatiskt när objekt går ur scope eller tas bort
     * - Namnet är ~KlassNamn
     * - Används för att frigöra resurser (RAII-pattern)
     * - Ska INTE kasta undantag
     *
     * RAII (Resource Acquisition Is Initialization):
     * - Resurser allokeras i konstruktorn
     * - Resurser frigörs i destruktorn
     * - Garanterar att städning alltid sker (även vid undantag)
     *
     * Här städar vi upp nätverksbiblioteket när klienten förstörs
     */
    ~VaderKlient() {
        // Städa upp nätverksbibliotek
        // Windows: WSACleanup
        // Linux: Gör ingenting
        rensa_natverksbibliotek();
    }

    /**
     * Hämtar aktuell väderinformation för en stad
     *
     * METODFLÖDE:
     * 1. Etablera TCP-anslutning till servern
     * 2. Bygg URL med query-parametrar
     * 3. Skicka HTTP GET-request
     * 4. Ta emot HTTP-svar
     * 5. Extrahera och validera JSON-body
     * 6. Kontrollera om svar innehåller fel
     * 7. Visa väderdata i formaterad form
     * 8. Stäng anslutningen
     *
     * FELHANTERING:
     * - Returnerar false vid varje steg som misslyckas
     * - Skriver felmeddelanden till cerr (standard error stream)
     * - Stänger alltid socket innan return
     *
     * @param stad - Stadnamn (ex: "Stockholm")
     * @param landskod - ISO 3166-1 alpha-2 landskod (ex: "SE")
     * @return true vid framgång, false vid fel
     */
    bool hamtaVader(const string& stad, const string& landskod) {
        // Etablera TCP-anslutning till servern
        socket_t sock = anslut();
        if (sock == OGILTIG_SOCKET) return false;

        // Bygg URL med query-parametrar
        // + operatorn konkatenerar C++ strings
        // Format: /weather?city=Stockholm&country=SE
        string url = "/weather?city=" + stad + "&country=" + landskod;

        // Skicka HTTP GET-request
        if (!skicka_http_get(sock, url)) {
            // cerr är standard error stream (motsvarar stderr i C)
            cerr << "FEL: Kunde inte skicka request" << endl;
            stang_socket(sock);
            return false;
        }

        // Ta emot HTTP-svar och extrahera JSON-body
        string json_svar = ta_emot_http_svar(sock);

        // Stäng socket-anslutningen
        // Görs tidigt för att frigöra resurser även om fel uppstår
        stang_socket(sock);

        // Validera att vi fick ett svar
        // empty() returnerar true om strängen är tom
        if (json_svar.empty()) {
            cerr << "FEL: Tomt svar från server" << endl;
            return false;
        }

        // Kontrollera om servern returnerade ett fel
        // find() returnerar string::npos om substrängen inte hittas
        // != string::npos betyder "hittades"
        if (json_svar.find("\"fel\": true") != string::npos) {
            // Extrahera felmeddelandet från JSON
            string felmeddelande = parse_json_string(json_svar, "meddelande");
            cerr << "\nFEL från server: " << felmeddelande << endl;
            return false;
        }

        // Visa väderdata i formaterad låda
        visaVader(json_svar);
        return true;
    }

    /**
     * Hämtar väderprognos för en stad
     *
     * Funktionaliteten är nästan identisk med hamtaVader(),
     * men använder /forecast endpoint istället för /weather
     *
     * FÖRENKLAD IMPLEMENTATION:
     * Denna version visar bara råa JSON-data
     * En fullständig version skulle parsa prognosdata och visa formaterat
     * Prognosdata innehåller arrays med framtida väderdata
     *
     * @param stad - Stadnamn
     * @param landskod - Landskod
     * @return true vid framgång, false vid fel
     */
    bool hamtaPrognos(const string& stad, const string& landskod) {
        // Anslut till servern
        socket_t sock = anslut();
        if (sock == OGILTIG_SOCKET) return false;

        // Bygg URL för prognos-endpoint
        string url = "/forecast?city=" + stad + "&country=" + landskod;

        // Skicka request
        if (!skicka_http_get(sock, url)) {
            cerr << "FEL: Kunde inte skicka request" << endl;
            stang_socket(sock);
            return false;
        }

        // Ta emot svar
        string json_svar = ta_emot_http_svar(sock);
        stang_socket(sock);

        // Validera svar
        if (json_svar.empty()) {
            cerr << "FEL: Tomt svar" << endl;
            return false;
        }

        // Kontrollera fel
        if (json_svar.find("\"fel\": true") != string::npos) {
            string felmeddelande = parse_json_string(json_svar, "meddelande");
            cerr << "\nFEL: " << felmeddelande << endl;
            return false;
        }

        // Visa prognos (förenklad - bara råa JSON)
        // cout är standard output stream
        // endl skickar newline och flushar bufferten
        cout << "\nPrognos (JSON):\n" << json_svar << endl;
        return true;
    }

private:
    // ========================================================================
    // PRIVATA HJÄLPMETODER FÖR ANSLUTNING OCH VISNING
    // ========================================================================

    /**
     * Etablerar TCP-anslutning till väderservern
     *
     * SOCKET PROGRAMMING:
     * 1. socket() - Skapa socket-descriptor
     * 2. Konfigurera server-adress i sockaddr_in
     * 3. connect() - Etablera TCP-anslutning
     * 4. Returnera socket för kommunikation
     *
     * SOCKADDR_IN-STRUKTUR (från C):
     * struct sockaddr_in {
     *     short sin_family;        // Adressfamilj (AF_INET)
     *     unsigned short sin_port; // Portnummer (network byte order)
     *     struct in_addr sin_addr; // IP-adress
     *     char sin_zero[8];        // Padding
     * };
     *
     * NETWORK BYTE ORDER:
     * Nätverkskommunikation använder big-endian byte order
     * htons() = Host TO Network Short - konverterar portnummer
     * inet_addr()/inet_pton() - konverterar IP-adress
     *
     * STATIC_CAST:
     * C++-stil casting som är säkrare än C-style (type)
     * Kompilatorn kontrollerar att konverteringen är giltig
     *
     * @return Socket-descriptor vid framgång, OGILTIG_SOCKET vid fel
     */
    socket_t anslut() {
        // Skapa TCP-socket
        // AF_INET = IPv4, SOCK_STREAM = TCP
        socket_t sock = socket(AF_INET, SOCK_STREAM, 0);

        if (sock == OGILTIG_SOCKET) {
            cerr << "FEL: Kunde inte skapa socket" << endl;
            return OGILTIG_SOCKET;
        }

        // Förbered server-adressstruktur
        struct sockaddr_in server_adress_info;

        // Nollställ all minne i strukturen (viktigt!)
        // memset är från C, men fungerar i C++
        memset(&server_adress_info, 0, sizeof(server_adress_info));

        // Konfigurera adressfamilj
        server_adress_info.sin_family = AF_INET;

        // Konfigurera port med byte order konvertering
        // static_cast är C++ sätt att göra typkonvertering
        server_adress_info.sin_port = htons(static_cast<uint16_t>(server_port));

        // Konfigurera IP-adress (plattformsberoende)
#ifdef _WIN32
        // Windows: inet_addr() konverterar "127.0.0.1" till binär IP
        // c_str() ger C-style const char* från C++ string
        server_adress_info.sin_addr.s_addr = inet_addr(server_adress.c_str());
#else
        // Linux/Unix: inet_pton() är modernare och stöder IPv6
        // pton = Presentation TO Network
        inet_pton(AF_INET, server_adress.c_str(), &server_adress_info.sin_addr);
#endif

        // Informera användaren
        cout << "Ansluter till http://" << server_adress << ":" << server_port << "..." << endl;

        // Etablera TCP-anslutning (tre-vägs handslag: SYN, SYN-ACK, ACK)
        // connect() blockerar tills anslutning etableras eller timeout
        if (connect(sock, (struct sockaddr*)&server_adress_info, sizeof(server_adress_info)) < 0) {
            cerr << "FEL: Kunde inte ansluta" << endl;
            stang_socket(sock);
            return OGILTIG_SOCKET;
        }

        // Framgång!
        cout << "✓ Ansluten!" << endl;
        return sock;
    }

    /**
     * Visar väderdata i en formaterad visuell låda
     *
     * C++ OUTPUT STREAMS:
     * cout << värde - Skriver värde till standard output
     * endl - Skickar newline (\n) och flushar bufferten
     *
     * FORMATERING:
     * C++ streams har inte samma formateringskontroll som printf
     * Vi använder typkonvertering för att kontrollera decimaler:
     * - (int)luft - Konverterar till heltal (tar bort decimaler)
     * - vind utan konvertering - Visar alla decimaler
     *
     * BOX-DRAWING:
     * Använder Unicode box-drawing characters för visuell presentation
     * Kräver UTF-8 stöd i terminalen
     *
     * PADDING:
     * För att hålla lådans bredd konsistent måste vi padda stadnamn
     * Vi loopar och lägger till mellanslag tills vi når 24 tecken
     *
     * @param json - JSON-svar med väderdata
     */
    void visaVader(const string& json) {
        // Extrahera alla värden från JSON
        string stad = parse_json_string(json, "stad");
        float temperatur = parse_json_float(json, "temperatur");
        float luftfuktighet = parse_json_float(json, "luftfuktighet");
        float vindhastighet = parse_json_float(json, "vindhastighet");
        float lufttryck = parse_json_float(json, "lufttryck");
        string beskrivning = parse_json_string(json, "beskrivning");

        // Rita formaterad låda med väderdata
        cout << "\n";
        cout << "╔═══════════════════════════════════════════════════════╗" << endl;

        // Header med stadnamn och padding
        cout << "║             AKTUELLT VÄDER - " << stad;

        // Lägg till padding för att hålla lådans bredd
        // size_t är en unsigned heltaltyp för storlekar
        // length() returnerar antal tecken i strängen
        for (size_t i = stad.length(); i < 24; i++) cout << " ";

        cout << "║" << endl;
        cout << "╠═══════════════════════════════════════════════════════╣" << endl;

        // Väderdata med ikoner och enheter
        // Temperatur visas med alla decimaler som finns
        cout << "║  🌡️  Temperatur:    " << temperatur << "°C" << endl;

        // Luftfuktighet konverteras till int för att ta bort decimaler
        cout << "║  💧  Luftfuktighet:  " << (int)luftfuktighet << "%" << endl;

        cout << "║  💨  Vindhastighet:  " << vindhastighet << " m/s" << endl;

        // Lufttryck konverteras till int
        cout << "║  📊  Lufttryck:      " << (int)lufttryck << " hPa" << endl;

        cout << "║  ☁️  Beskrivning:    " << beskrivning << endl;
        cout << "╚═══════════════════════════════════════════════════════╝" << endl;
        cout << "\n";
    }
};

// ============================================================================
// HUVUDPROGRAM
// ============================================================================

/**
 * Huvudfunktion - Programmets startpunkt
 *
 * PROGRAMSTRUKTUR:
 * 1. Läs kommandoradsargument
 * 2. Skapa VaderKlient-objekt (initialiserar nätverksbibliotek)
 * 3. Huvudloop: Visa meny och hantera användarval
 * 4. Destruktor anropas automatiskt när klient går ur scope (städar upp)
 *
 * TRY-CATCH BLOCK:
 * try {
 *     kod som kan kasta undantag
 * } catch (undantagstyp& e) {
 *     felhantering
 * }
 *
 * UNDANTAGSHANTERING I C++:
 * - Separerar normal kod från felhantering
 * - throw kastar undantag
 * - catch fångar undantag
 * - exception är basklassen för standardundantag
 * - what() returnerar felbeskrivning
 *
 * CIN/COUT:
 * cin >> variabel - Läser input från stdin
 * cout << värde - Skriver output till stdout
 * cin.ignore() - Rensar input-bufferten
 * getline(cin, string) - Läser en hel rad
 *
 * CONST:
 * const exception& e - Referens till konstant exception
 * & = Referens (undvik kopiering)
 * const = Kan inte modifieras
 *
 * @param argc - Antal kommandoradsargument
 * @param argv - Array av argument-strängar
 * @return 0 vid framgång, 1 vid fel
 */
int main(int argc, char* argv[]) {
    try {
        // Läs server-konfiguration från kommandoradsargument eller använd default
        // Ternär operator: villkor ? om_sant : om_falskt
        string server_adress = (argc > 1) ? argv[1] : "127.0.0.1";

        // atoi() (ASCII TO Integer) konverterar C-string till int
        int server_port = (argc > 2) ? atoi(argv[2]) : 8080;

        // Visa välkomstmeddelande
        cout << "╔═══════════════════════════════════════════════════════╗" << endl;
        cout << "║       VÄDERSYSTEM - C++-KLIENT (HTTP/JSON)            ║" << endl;
        cout << "╚═══════════════════════════════════════════════════════╝\n" << endl;

        // Skapa klient-objekt
        // Konstruktorn initialiserar nätverksbibliotek
        // Kan kasta runtime_error om initialisering misslyckas
        VaderKlient klient(server_adress, server_port);

        // Huvudloop - Kör tills användaren väljer att avsluta
        while (true) {
            // Visa huvudmeny
            cout << "\nVälj alternativ:" << endl;
            cout << "  1. Hämta aktuellt väder" << endl;
            cout << "  2. Hämta väderprognos" << endl;
            cout << "  3. Avsluta" << endl;
            cout << "Val: ";

            // Läs användarens val
            int val;

            // cin >> val läser ett heltal från standard input
            // Operator>> är överlagrad för olika typer
            cin >> val;

            // cin.ignore() rensar input-bufferten
            // Tar bort nyradstecknet som lämnades av >>
            // Viktigt innan getline() för att undvika att läsa tom rad
            cin.ignore();

            // Kontrollera om användaren vill avsluta
            if (val == 3) break;  // Bryt ur loopen

            // Validera input
            if (val != 1 && val != 2) {
                cout << "Ogiltigt val" << endl;
                continue;  // Gå till nästa iteration
            }

            // Läs stadnamn och landskod från användaren
            string stad, landskod;

            cout << "Ange stad: ";
            // getline() läser en hel rad inkl. mellanslag
            // cin är input-strömmen
            // stad är string-variabeln där input lagras
            getline(cin, stad);

            cout << "Ange landskod (ex: SE, US, GB): ";
            getline(cin, landskod);

            // Om landskod är tom, använd Sverige som standard
            // empty() returnerar true för tom sträng
            if (landskod.empty()) landskod = "SE";

            // Anropa lämplig metod baserat på användarens val
            if (val == 1) {
                // Hämta aktuellt väder
                klient.hamtaVader(stad, landskod);
            } else {
                // Hämta väderprognos
                klient.hamtaPrognos(stad, landskod);
            }
        }

        // Visa avslutningsmeddelande
        cout << "\nHejdå!" << endl;

        // När vi lämnar try-blocket förstörs klient-objektet automatiskt
        // Destruktorn anropas och städar upp nätverksbibliotek (RAII)

    } catch (const exception& e) {
        // Fånga alla standardundantag
        // const exception& = konstant referens (ingen kopiering)
        // e.what() returnerar felbeskrivning som const char*

        cerr << "UNDANTAG: " << e.what() << endl;
        return 1;  // Avsluta med felkod
    }

    return 0;  // Framgångsrikt avslutande
}
