// ============================================================================
// ENHETSTESTER FÖR HTTP-SERVER
// ============================================================================
// Testar HTTP-parsing och response-generering
// Kompilera: gcc -I../include tests/test_http.c -o test_http
// Kör: ./test_http

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "../src/http_server.c"

static int tester_totalt = 0;
static int tester_godkanda = 0;

#define RUN_TEST(test_func) do { \
    printf("Kör %s...\n", #test_func); \
    tester_totalt++; \
    test_func(); \
    tester_godkanda++; \
    printf("  ✓ GODKÄND\n"); \
} while(0)

// ============================================================================
// TESTER FÖR PARSA_HTTP_REQUEST
// ============================================================================

void test_parsa_http_get_enkel() {
    const char* rådata =
        "GET /weather HTTP/1.1\r\n"
        "Host: localhost\r\n\r\n";

    HttpRequest request;
    bool resultat = parsa_http_request(rådata, &request);

    assert(resultat == true);
    assert(request.metod == HTTP_GET);
    assert(strcmp(request.sokvag, "/weather") == 0);
    assert(strlen(request.query) == 0);
}

void test_parsa_http_get_med_query() {
    const char* rådata =
        "GET /weather?city=Stockholm&country=SE HTTP/1.1\r\n"
        "Host: localhost\r\n\r\n";

    HttpRequest request;
    bool resultat = parsa_http_request(rådata, &request);

    assert(resultat == true);
    assert(request.metod == HTTP_GET);
    assert(strcmp(request.sokvag, "/weather") == 0);
    assert(strcmp(request.query, "city=Stockholm&country=SE") == 0);
}

void test_parsa_http_post() {
    const char* rådata =
        "POST /api HTTP/1.1\r\n"
        "Content-Length: 13\r\n\r\n"
        "{\"test\": 1}";

    HttpRequest request;
    bool resultat = parsa_http_request(rådata, &request);

    assert(resultat == true);
    assert(request.metod == HTTP_POST);
    assert(strcmp(request.sokvag, "/api") == 0);
}

void test_parsa_http_ogiltig() {
    const char* rådata = "INVALID REQUEST";

    HttpRequest request;
    bool resultat = parsa_http_request(rådata, &request);

    assert(resultat == false);
}

void test_parsa_http_root() {
    const char* rådata =
        "GET / HTTP/1.1\r\n"
        "Host: localhost\r\n\r\n";

    HttpRequest request;
    bool resultat = parsa_http_request(rådata, &request);

    assert(resultat == true);
    assert(strcmp(request.sokvag, "/") == 0);
}

// ============================================================================
// TESTER FÖR HAMTA_QUERY_PARAMETER
// ============================================================================

void test_hamta_query_parameter_enkel() {
    const char* query = "city=Stockholm";
    char värde[64];

    bool resultat = hamta_query_parameter(query, "city", värde, sizeof(värde));

    assert(resultat == true);
    assert(strcmp(värde, "Stockholm") == 0);
}

void test_hamta_query_parameter_flera() {
    const char* query = "city=Stockholm&country=SE&temp=23";
    char city[64], country[64], temp[64];

    assert(hamta_query_parameter(query, "city", city, sizeof(city)) == true);
    assert(strcmp(city, "Stockholm") == 0);

    assert(hamta_query_parameter(query, "country", country, sizeof(country)) == true);
    assert(strcmp(country, "SE") == 0);

    assert(hamta_query_parameter(query, "temp", temp, sizeof(temp)) == true);
    assert(strcmp(temp, "23") == 0);
}

void test_hamta_query_parameter_saknas() {
    const char* query = "city=Stockholm&country=SE";
    char värde[64];

    bool resultat = hamta_query_parameter(query, "temp", värde, sizeof(värde));

    assert(resultat == false);
}

void test_hamta_query_parameter_tom() {
    const char* query = "";
    char värde[64];

    bool resultat = hamta_query_parameter(query, "city", värde, sizeof(värde));

    assert(resultat == false);
}

void test_hamta_query_parameter_med_mellanslag() {
    const char* query = "city=New York&country=US";
    char city[64];

    bool resultat = hamta_query_parameter(query, "city", city, sizeof(city));

    assert(resultat == true);
    // Note: URL encoding skulle göra "New York" till "New%20York"
    // Men för enkel implementation accepterar vi mellanslag
}

// ============================================================================
// TESTER FÖR SKAPA_HTTP_RESPONSE
// ============================================================================

void test_skapa_http_response_200() {
    char buffer[512];
    const char* json = "{\"test\": \"data\"}";

    skapa_http_response(buffer, sizeof(buffer), 200, json);

    assert(strstr(buffer, "HTTP/1.1 200 OK") != NULL);
    assert(strstr(buffer, "Content-Type: application/json") != NULL);
    assert(strstr(buffer, "{\"test\": \"data\"}") != NULL);
}

void test_skapa_http_response_404() {
    char buffer[512];
    const char* json = "{\"error\": \"Not found\"}";

    skapa_http_response(buffer, sizeof(buffer), 404, json);

    assert(strstr(buffer, "HTTP/1.1 404 Not Found") != NULL);
    assert(strstr(buffer, "{\"error\": \"Not found\"}") != NULL);
}

void test_skapa_http_response_500() {
    char buffer[512];
    const char* json = "{\"error\": \"Internal error\"}";

    skapa_http_response(buffer, sizeof(buffer), 500, json);

    assert(strstr(buffer, "HTTP/1.1 500 Internal Server Error") != NULL);
}

void test_skapa_http_response_headers() {
    char buffer[512];
    const char* json = "{}";

    skapa_http_response(buffer, sizeof(buffer), 200, json);

    // Kontrollera att alla headers finns
    assert(strstr(buffer, "Content-Type:") != NULL);
    assert(strstr(buffer, "Content-Length:") != NULL);
    assert(strstr(buffer, "Connection: close") != NULL);
    assert(strstr(buffer, "\r\n\r\n") != NULL);  // Header-body separator
}

// ============================================================================
// HUVUDFUNKTION
// ============================================================================

int main(void) {
    printf("\n╔═══════════════════════════════════════════════════════╗\n");
    printf("║          ENHETSTESTER FÖR HTTP-SERVER               ║\n");
    printf("╚═══════════════════════════════════════════════════════╝\n\n");

    // Tester för parsa_http_request
    RUN_TEST(test_parsa_http_get_enkel);
    RUN_TEST(test_parsa_http_get_med_query);
    RUN_TEST(test_parsa_http_post);
    RUN_TEST(test_parsa_http_ogiltig);
    RUN_TEST(test_parsa_http_root);

    // Tester för hamta_query_parameter
    RUN_TEST(test_hamta_query_parameter_enkel);
    RUN_TEST(test_hamta_query_parameter_flera);
    RUN_TEST(test_hamta_query_parameter_saknas);
    RUN_TEST(test_hamta_query_parameter_tom);
    RUN_TEST(test_hamta_query_parameter_med_mellanslag);

    // Tester för skapa_http_response
    RUN_TEST(test_skapa_http_response_200);
    RUN_TEST(test_skapa_http_response_404);
    RUN_TEST(test_skapa_http_response_500);
    RUN_TEST(test_skapa_http_response_headers);

    // Visa resultat
    printf("\n╔═══════════════════════════════════════════════════════╗\n");
    printf("║                   TESTRESULTAT                       ║\n");
    printf("╚═══════════════════════════════════════════════════════╝\n");
    printf("  Totalt:        %d tester\n", tester_totalt);
    printf("  Godkända:      %d tester\n", tester_godkanda);
    printf("  Misslyckade:   %d tester\n", tester_totalt - tester_godkanda);

    if (tester_godkanda == tester_totalt) {
        printf("\n  ✓ ALLA TESTER GODKÄNDA!\n\n");
        return 0;
    } else {
        printf("\n  ✗ VISSA TESTER MISSLYCKADES\n\n");
        return 1;
    }
}
