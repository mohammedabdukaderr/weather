// ============================================================================
// WEATHER CLIENT - C++ VERSION
// ============================================================================
// C++-klient för vädersystem som kommunicerar med HTTP/JSON väderserver
// Använder moderna C++ features: klasser, STL, RAII, exceptions
// Kompatibel med samma server som C-klienten

#include "natverks_abstraktion.h"
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <stdexcept>

// Använd C++ namespace för att undvika namnkonflikter
using namespace std;

// ============================================================================
// KONFIGURATIONSKONSTANTER
// ============================================================================
constexpr const char* SERVER_ADRESS = "127.0.0.1";
constexpr int SERVER_PORT = 8080;
constexpr size_t BUFFER_STORLEK = 8192;

// ============================================================================
// HJÄLPKLASS FÖR JSON-PARSING
// ============================================================================
// Enkel JSON-parser som använder C++ string-funktioner
// I produktion skulle man använda ett bibliotek som nlohmann/json
class JsonParser {
public:
    /**
     * Parsar ett flyttal från JSON-sträng
     *
     * @param json JSON-sträng att söka i
     * @param nyckel Namnet på JSON-nyckeln
     * @return Flyttalsvärde eller 0.0 om inte funnen
     *
     * Exempel: parseFloat("{\"temp\": 23.5}", "temp") returnerar 23.5
     */
    static float parseFloat(const string& json, const string& nyckel) {
        // Använd C++ string för att bygga sökmönster
        string sokstrang = "\"" + nyckel + "\": ";

        // string::find() returnerar position eller string::npos om inte funnen
        size_t pos = json.find(sokstrang);
        if (pos == string::npos) return 0.0f;

        // Hoppa förbi söksträng och konvertera värdet
        pos += sokstrang.length();
        return stof(json.substr(pos));  // stof = string to float
    }

    /**
     * Parsar en textsträng från JSON
     *
     * @param json JSON-sträng att söka i
     * @param nyckel Namnet på JSON-nyckeln
     * @return String-värde eller tom sträng om inte funnen
     *
     * Exempel: parseString("{\"stad\": \"Stockholm\"}", "stad") returnerar "Stockholm"
     */
    static string parseString(const string& json, const string& nyckel) {
        // Sökmönster för string-värden: "nyckel": "
        string sokstrang = "\"" + nyckel + "\": \"";

        size_t start = json.find(sokstrang);
        if (start == string::npos) return "";

        // Flytta till början av värdet
        start += sokstrang.length();

        // Hitta avslutande citattecken
        size_t slut = json.find('"', start);
        if (slut == string::npos) return "";

        // Extrahera substrängen mellan citattecknen
        return json.substr(start, slut - start);
    }
};

// ============================================================================
// VÄDERDATA-KLASS
// ============================================================================
// Representerar väderdata från servern med getters och formaterad utskrift
class VaderData {
private:
    string stad;
    string land;
    float temperatur;
    float luftfuktighet;
    float vindhastighet;
    float lufttryck;
    string beskrivning;
    bool fran_cache;

public:
    // Konstruktor - initialiserar alla medlemsvariabler
    VaderData() : temperatur(0), luftfuktighet(0),
                  vindhastighet(0), lufttryck(0), fran_cache(false) {}

    /**
     * Parsar JSON-svar från servern och fyller i medlemsvariabler
     *
     * @param json_svar JSON-sträng från servern
     *
     * VARFÖR C++ ÄR BRA HÄR:
     * - Använder string istället för char arrays (inga buffertöverskridningar)
     * - JsonParser är en statisk utility-klass (mer organiserat än globala funktioner)
     * - Ingen manuell minneshantering behövs
     */
    void parseJsonSvar(const string& json_svar) {
        stad = JsonParser::parseString(json_svar, "stad");
        land = JsonParser::parseString(json_svar, "land");
        temperatur = JsonParser::parseFloat(json_svar, "temperatur");
        luftfuktighet = JsonParser::parseFloat(json_svar, "luftfuktighet");
        vindhastighet = JsonParser::parseFloat(json_svar, "vindhastighet");
        lufttryck = JsonParser::parseFloat(json_svar, "lufttryck");
        beskrivning = JsonParser::parseString(json_svar, "beskrivning");

        // Kolla om data kommer från cache
        fran_cache = (json_svar.find("\"cachad\": true") != string::npos);
    }

    /**
     * Skriver ut väderdata i ett snyggt format till konsolen
     *
     * VARFÖR C++ ÄR BRA HÄR:
     * - cout är typ-säkert (ingen risk för printf format-fel)
     * - Enklare strängkonkatenering med operator<<
     * - Automatisk minneshantering
     */
    void skrivUt() const {
        cout << "\n╔═══════════════════════════════════════════════════════╗\n";
        cout << "║              VÄDERRAPPORT - C++ KLIENT               ║\n";
        cout << "╚═══════════════════════════════════════════════════════╝\n\n";

        cout << "📍 Stad:          " << stad << ", " << land << "\n";
        cout << "🌡️  Temperatur:   " << temperatur << "°C\n";
        cout << "💧 Luftfuktighet: " << luftfuktighet << "%\n";
        cout << "💨 Vindhastighet: " << vindhastighet << " m/s\n";
        cout << "🔽 Lufttryck:     " << lufttryck << " hPa\n";
        cout << "☁️  Beskrivning:  " << beskrivning << "\n";

        if (fran_cache) {
            cout << "\n💾 Data från cache (sparad tidigare)\n";
        }

        cout << "\n";
    }
};

// ============================================================================
// NÄTVERKSKLIENT-KLASS
// ============================================================================
// Hanterar TCP-anslutning och HTTP-kommunikation med servern
// Använder RAII (Resource Acquisition Is Initialization) för socket-hantering
class NatverksKlient {
private:
    socket_t socket_fd;
    bool ansluten;

public:
    /**
     * Konstruktor - initialiserar klienten
     *
     * RAII-PRINCIP:
     * - Konstruktorn initialiserar nätverksbiblioteket
     * - Destruktorn städar upp automatiskt
     * - Ingen risk att glömma WSACleanup() eller closesocket()
     */
    NatverksKlient() : socket_fd(OGILTIG_SOCKET), ansluten(false) {
        // Initialisera Winsock (Windows) eller gör ingenting (Linux)
        if (initiera_natverksbibliotek() != 0) {
            throw runtime_error("Kunde inte initiera nätverksbibliotek");
        }
    }

    /**
     * Destruktor - städar upp automatiskt när objektet förstörs
     *
     * Detta är C++:s stora fördel över C - automatisk resurshantering!
     * Vi behöver aldrig explicit anropa detta, C++ gör det åt oss
     */
    ~NatverksKlient() {
        if (ansluten && socket_fd != OGILTIG_SOCKET) {
            stang_socket(socket_fd);
        }
        rensa_natverksbibliotek();
    }

    /**
     * Ansluter till väderservern
     *
     * @param adress IP-adress eller hostname
     * @param port Portnummer att ansluta till
     *
     * THROWS: runtime_error om anslutningen misslyckas
     *
     * VARFÖR EXCEPTIONS ÄR BRA:
     * - Tydlig felhantering utan att kolla return-värden överallt
     * - Destruktorn anropas automatiskt även vid exception
     * - Ingen risk för minnesläckor
     */
    void anslut(const string& adress, int port) {
        // Skapa TCP-socket (IPv4, stream-baserad, TCP-protokoll)
        socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd == OGILTIG_SOCKET) {
            throw runtime_error("Kunde inte skapa socket");
        }

        // Konfigurera serveradress
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;               // IPv4
        server_addr.sin_port = htons(port);             // Port i network byte order

        // Konvertera IP-adress från text till binär form
        #ifdef _WIN32
            server_addr.sin_addr.s_addr = inet_addr(adress.c_str());
        #else
            inet_pton(AF_INET, adress.c_str(), &server_addr.sin_addr);
        #endif

        // Anslut till servern
        if (connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_FEL) {
            stang_socket(socket_fd);
            throw runtime_error("Kunde inte ansluta till server");
        }

        ansluten = true;
        cout << "✓ Ansluten till server " << adress << ":" << port << "\n\n";
    }

    /**
     * Skickar HTTP GET-request och tar emot svar
     *
     * @param stad Stad att hämta väder för
     * @param landskod Landskod (t.ex. "SE" för Sverige)
     * @return JSON-svar från servern
     *
     * HTTP-REQUEST FORMAT:
     * GET /weather?city=Stockholm&country=SE HTTP/1.1\r\n
     * Host: localhost:8080\r\n
     * Connection: close\r\n
     * \r\n
     */
    string hamtaVader(const string& stad, const string& landskod) {
        // Använd stringstream för att bygga HTTP-request (mer elegant än sprintf)
        ostringstream request_builder;
        request_builder << "GET /weather?city=" << stad
                       << "&country=" << landskod << " HTTP/1.1\r\n"
                       << "Host: localhost:" << SERVER_PORT << "\r\n"
                       << "Connection: close\r\n\r\n";

        string request = request_builder.str();

        // Skicka request till servern
        if (send(socket_fd, request.c_str(), request.length(), 0) == SOCKET_FEL) {
            throw runtime_error("Kunde inte skicka request till server");
        }

        // Ta emot svar från servern
        char buffer[BUFFER_STORLEK];
        string svar;
        int mottaget;

        // Läs tills servern stänger anslutningen eller bufferten är full
        while ((mottaget = recv(socket_fd, buffer, sizeof(buffer) - 1, 0)) > 0) {
            buffer[mottaget] = '\0';
            svar += buffer;  // C++ string-konkatenering
        }

        if (mottaget == SOCKET_FEL) {
            throw runtime_error("Fel vid mottagning av data");
        }

        // Extrahera JSON-body från HTTP-svaret
        // HTTP-svar har format: headers\r\n\r\nbody
        size_t body_start = svar.find("\r\n\r\n");
        if (body_start != string::npos) {
            return svar.substr(body_start + 4);  // Hoppa över \r\n\r\n
        }

        return svar;
    }
};

// ============================================================================
// HUVUDFUNKTION
// ============================================================================
/**
 * Programmets startpunkt
 *
 * KOMMANDORADSARGUMENT:
 * - argv[1]: Stad (obligatorisk)
 * - argv[2]: Landskod (valfri, standard "SE")
 *
 * EXEMPEL:
 * ./weather_client Stockholm SE
 * ./weather_client London GB
 * ./weather_client Paris FR
 */
int main(int argc, char* argv[]) {
    // Om kommandoradsargument finns, kör direkt
    if (argc > 1) {
        try {
            string stad = argv[1];
            string landskod = (argc > 2) ? argv[2] : "SE";

            cout << "╔═══════════════════════════════════════════════════════╗\n";
            cout << "║          VÄDERKLIENT - C++ VERSION 1.0.0             ║\n";
            cout << "╚═══════════════════════════════════════════════════════╝\n\n";

            NatverksKlient klient;
            klient.anslut(SERVER_ADRESS, SERVER_PORT);

            cout << "Hämtar väderdata för " << stad << ", " << landskod << "...\n";
            string json_svar = klient.hamtaVader(stad, landskod);

            VaderData vader;
            vader.parseJsonSvar(json_svar);
            vader.skrivUt();

            return 0;

        } catch (const exception& e) {
            cerr << "\n❌ FEL: " << e.what() << "\n\n";
            return 1;
        }
    }

    // Interaktivt läge
    cout << "╔═══════════════════════════════════════════════════════╗\n";
    cout << "║          INTERAKTIV VÄDERKLIENT                      ║\n";
    cout << "╚═══════════════════════════════════════════════════════╝\n\n";
    cout << "💡 Ansluter till lokal väderserver på " << SERVER_ADRESS << ":" << SERVER_PORT << "\n";
    cout << "💡 Servern hämtar data från OpenWeatherMap API\n\n";

    while (true) {
        cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        cout << "Vad vill du göra?\n";
        cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

        string input;
        cout << "Skriv stad (eller 'exit' för att avsluta): ";
        getline(cin, input);

        // Trim whitespace
        size_t start = input.find_first_not_of(" \t");
        size_t end = input.find_last_not_of(" \t");
        if (start != string::npos && end != string::npos) {
            input = input.substr(start, end - start + 1);
        }

        if (input == "exit" || input == "quit" || input == "q") {
            cout << "\n👋 Tack för att du använde väderklienten!\n\n";
            break;
        }

        if (input.empty()) {
            continue;
        }

        // Parse stad och landskod
        string stad = input;
        string landskod = "SE";

        size_t space_pos = input.find(' ');
        if (space_pos != string::npos) {
            stad = input.substr(0, space_pos);
            landskod = input.substr(space_pos + 1);
            size_t lk_start = landskod.find_first_not_of(" \t");
            if (lk_start != string::npos) {
                landskod = landskod.substr(lk_start);
            }
        }

        // Hämta väder från servern
        try {
            NatverksKlient klient;
            klient.anslut(SERVER_ADRESS, SERVER_PORT);

            cout << "Hämtar väderdata för " << stad << ", " << landskod << "...\n";
            string json_svar = klient.hamtaVader(stad, landskod);

            VaderData vader;
            vader.parseJsonSvar(json_svar);
            vader.skrivUt();

        } catch (const exception& e) {
            cerr << "\n❌ FEL: " << e.what() << "\n";
            cerr << "Kontrollera att servern körs på " << SERVER_ADRESS << ":" << SERVER_PORT << "\n\n";
        }
    }

    return 0;
}
