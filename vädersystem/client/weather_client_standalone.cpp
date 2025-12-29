// ============================================================================
// STANDALONE WEATHER CLIENT - C++ VERSION
// ============================================================================
// Självständig väderklient som hämtar data DIREKT från OpenWeatherMap API
// Kräver INGEN lokal server - fungerar helt fristående
// Enligt offertens krav: klienten ska kunna användas oberoende av servern

#include "natverks_abstraktion.h"
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <stdexcept>

using namespace std;

// ============================================================================
// KONFIGURATIONSKONSTANTER
// ============================================================================
constexpr const char* API_HOST = "api.openweathermap.org";
constexpr int API_PORT = 80;
constexpr const char* API_KEY = "12a60b63ba819b7b7dc13d6ef249d5a1";
constexpr size_t BUFFER_STORLEK = 16384;

// ============================================================================
// JSON-PARSER
// ============================================================================
class JsonParser {
public:
    static float parseFloat(const string& json, const string& nyckel) {
        string sokstrang = "\"" + nyckel + "\":";
        size_t pos = json.find(sokstrang);
        if (pos == string::npos) return 0.0f;
        pos += sokstrang.length();

        // Hoppa över mellanslag
        while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t')) pos++;

        return stof(json.substr(pos));
    }

    static string parseString(const string& json, const string& nyckel) {
        string sokstrang = "\"" + nyckel + "\":\"";
        size_t start = json.find(sokstrang);
        if (start == string::npos) return "";

        start += sokstrang.length();
        size_t slut = json.find('"', start);
        if (slut == string::npos) return "";

        return json.substr(start, slut - start);
    }
};

// ============================================================================
// VÄDERDATA
// ============================================================================
class VaderData {
private:
    string stad;
    string land;
    float temperatur;
    float luftfuktighet;
    float vindhastighet;
    float lufttryck;
    string beskrivning;

public:
    VaderData() : temperatur(0), luftfuktighet(0),
                  vindhastighet(0), lufttryck(0) {}

    void parseFromOpenWeather(const string& json_svar, const string& stad_namn, const string& land_kod) {
        // OpenWeatherMap returnerar JSON i formatet:
        // {"main":{"temp":23.5,"humidity":65},"weather":[{"description":"clear sky"}],"wind":{"speed":5.2}}

        stad = stad_namn;
        land = land_kod;

        // Parsa temperatur (Kelvin -> Celsius)
        temperatur = JsonParser::parseFloat(json_svar, "temp") - 273.15f;
        luftfuktighet = JsonParser::parseFloat(json_svar, "humidity");
        vindhastighet = JsonParser::parseFloat(json_svar, "speed");
        lufttryck = JsonParser::parseFloat(json_svar, "pressure");
        beskrivning = JsonParser::parseString(json_svar, "description");
    }

    void skrivUt() const {
        cout << "\n╔═══════════════════════════════════════════════════════╗\n";
        cout << "║              VÄDERRAPPORT - C++ KLIENT               ║\n";
        cout << "╚═══════════════════════════════════════════════════════╝\n\n";

        cout << "📍 Stad:          " << stad << ", " << land << "\n";
        cout << "🌡️  Temperatur:   " << temperatur << "°C\n";
        cout << "💧 Luftfuktighet: " << luftfuktighet << "%\n";
        cout << "💨 Vindhastighet: " << vindhastighet << " m/s\n";
        cout << "🔽 Lufttryck:     " << lufttryck << " hPa\n";
        cout << "☁️  Beskrivning:  " << beskrivning << "\n\n";
    }
};

// ============================================================================
// NÄTVERKSKLIENT
// ============================================================================
class NatverksKlient {
private:
    socket_t socket_fd;
    bool ansluten;

public:
    NatverksKlient() : socket_fd(OGILTIG_SOCKET), ansluten(false) {
        if (initiera_natverksbibliotek() != 0) {
            throw runtime_error("Kunde inte initiera nätverksbibliotek");
        }
    }

    ~NatverksKlient() {
        if (ansluten && socket_fd != OGILTIG_SOCKET) {
            stang_socket(socket_fd);
        }
        rensa_natverksbibliotek();
    }

    string hamtaVader(const string& stad, const string& landskod) {
        // Skapa socket
        socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd == OGILTIG_SOCKET) {
            throw runtime_error("Kunde inte skapa socket");
        }

        // Hämta IP-adress för api.openweathermap.org
        struct hostent* server = gethostbyname(API_HOST);
        if (!server) {
            throw runtime_error("Kunde inte hitta API-server");
        }

        // Förbered serveradress
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(API_PORT);
        memcpy(&server_addr.sin_addr, server->h_addr_list[0], server->h_length);

        // Anslut till OpenWeatherMap API
        if (connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            throw runtime_error("Kunde inte ansluta till OpenWeatherMap API");
        }
        ansluten = true;

        // Bygg HTTP GET-request till OpenWeatherMap
        ostringstream request_stream;
        request_stream << "GET /data/2.5/weather?q=" << stad << "," << landskod
                      << "&appid=" << API_KEY << " HTTP/1.1\r\n"
                      << "Host: " << API_HOST << "\r\n"
                      << "Connection: close\r\n\r\n";

        string request = request_stream.str();

        // Skicka request
        if (send(socket_fd, request.c_str(), request.length(), 0) < 0) {
            throw runtime_error("Kunde inte skicka request");
        }

        // Ta emot svar
        char buffer[BUFFER_STORLEK];
        string svar;
        int bytes_mottagna;

        while ((bytes_mottagna = recv(socket_fd, buffer, sizeof(buffer) - 1, 0)) > 0) {
            buffer[bytes_mottagna] = '\0';
            svar += buffer;
        }

        if (svar.empty()) {
            throw runtime_error("Tomt svar från API");
        }

        // Hitta JSON-delen (efter HTTP-headers)
        size_t json_start = svar.find("\r\n\r\n");
        if (json_start == string::npos) {
            throw runtime_error("Ogiltigt API-svar");
        }

        return svar.substr(json_start + 4);
    }
};

// ============================================================================
// HUVUDPROGRAM
// ============================================================================
int main(int argc, char* argv[]) {
    // Standardvärden
    string stad = "Stockholm";
    string landskod = "SE";

    // Läs kommandoradsargument
    if (argc >= 2) stad = argv[1];
    if (argc >= 3) landskod = argv[2];

    try {
        cout << "\n🌍 Hämtar väder från OpenWeatherMap API...\n";
        cout << "📡 API: " << API_HOST << "\n";
        cout << "📍 Stad: " << stad << ", " << landskod << "\n";

        // Skapa nätverksklient
        NatverksKlient klient;

        // Hämta väderdata direkt från OpenWeatherMap
        string json_svar = klient.hamtaVader(stad, landskod);

        // Kontrollera om API returnerade fel
        if (json_svar.find("\"cod\":\"404\"") != string::npos) {
            cerr << "\n❌ FEL: Staden '" << stad << "' hittades inte!\n";
            cerr << "Kontrollera stavningen och försök igen.\n\n";
            return 1;
        }

        if (json_svar.find("\"cod\":401") != string::npos) {
            cerr << "\n❌ FEL: Ogiltig API-nyckel!\n";
            cerr << "Vänligen kontrollera API-nyckeln.\n\n";
            return 1;
        }

        // Parsa och visa väderdata
        VaderData vader;
        vader.parseFromOpenWeather(json_svar, stad, landskod);
        vader.skrivUt();

        return 0;

    } catch (const exception& e) {
        cerr << "\n❌ FEL: " << e.what() << "\n\n";
        return 1;
    }
}
