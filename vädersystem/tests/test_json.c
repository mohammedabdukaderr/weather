// ============================================================================
// ENHETSTESTER FÖR JSON-HELPER
// ============================================================================
// Testar JSON-parsing och generering
// Kompilera: gcc -I../include tests/test_json.c -o test_json
// Kör: ./test_json

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

// Inkludera headers först
#include "../include/vaderprotokoll.h"
#include "../include/json_helper.h"

// Inkludera funktioner vi ska testa (enkelt sätt för unit testing)
#define JSON_HELPER_C  // Förhindra include-guards
#include "../src/json_helper.c"

// Teststatistik
static int tester_totalt = 0;
static int tester_godkanda = 0;

// Makro för att köra test
#define RUN_TEST(test_func) do { \
    printf("Kör %s...\n", #test_func); \
    tester_totalt++; \
    test_func(); \
    tester_godkanda++; \
    printf("  ✓ GODKÄND\n"); \
} while(0)

// ============================================================================
// TESTER FÖR HAMTA_JSON_VARDE
// ============================================================================

void test_hamta_json_varde_enkel_strang() {
    const char* json = "{\"stad\": \"Stockholm\"}";
    char buffer[64];

    bool resultat = hamta_json_varde(json, "stad", buffer, sizeof(buffer));

    assert(resultat == true);
    assert(strcmp(buffer, "Stockholm") == 0);
}

void test_hamta_json_varde_med_mellanslag() {
    const char* json = "{\"beskrivning\": \"Partly cloudy\"}";
    char buffer[64];

    bool resultat = hamta_json_varde(json, "beskrivning", buffer, sizeof(buffer));

    assert(resultat == true);
    assert(strcmp(buffer, "Partly cloudy") == 0);
}

void test_hamta_json_varde_saknas() {
    const char* json = "{\"stad\": \"Stockholm\"}";
    char buffer[64];

    bool resultat = hamta_json_varde(json, "land", buffer, sizeof(buffer));

    assert(resultat == false);
}

void test_hamta_json_varde_tom_strang() {
    const char* json = "{\"test\": \"\"}";
    char buffer[64];

    bool resultat = hamta_json_varde(json, "test", buffer, sizeof(buffer));

    assert(resultat == true);
    assert(strlen(buffer) == 0);
}

// ============================================================================
// TESTER FÖR HAMTA_JSON_FLOAT
// ============================================================================

void test_hamta_json_float_positivt() {
    const char* json = "{\"temperatur\": 23.5}";

    float temp = hamta_json_float(json, "temperatur");

    assert(temp > 23.4 && temp < 23.6);
}

void test_hamta_json_float_negativt() {
    const char* json = "{\"temperatur\": -15.3}";

    float temp = hamta_json_float(json, "temperatur");

    assert(temp < -15.2 && temp > -15.4);
}

void test_hamta_json_float_heltal() {
    const char* json = "{\"luftfuktighet\": 65}";

    float fukt = hamta_json_float(json, "luftfuktighet");

    assert(fukt == 65.0f);
}

void test_hamta_json_float_saknas() {
    const char* json = "{\"temperatur\": 23.5}";

    float vind = hamta_json_float(json, "vindhastighet");

    assert(vind == 0.0f);
}

// ============================================================================
// TESTER FÖR SKAPA_VADER_JSON
// ============================================================================

void test_skapa_vader_json_komplett() {
    VaderData data = {
        .stad = "Stockholm",
        .land = "SE",
        .temperatur = 23.5f,
        .luftfuktighet = 65.0f,
        .vindhastighet = 5.2f,
        .lufttryck = 1013.0f,
        .beskrivning = "Clear sky",
        .ikon_id = "01d",
        .tidsstampel = 1234567890
    };

    char buffer[2048];
    skapa_vader_json(&data, buffer, sizeof(buffer));

    // Kontrollera att alla fält finns
    assert(strstr(buffer, "\"stad\": \"Stockholm\"") != NULL);
    assert(strstr(buffer, "\"land\": \"SE\"") != NULL);
    assert(strstr(buffer, "\"temperatur\": 23.5") != NULL);
    assert(strstr(buffer, "\"luftfuktighet\": 65.0") != NULL);
    assert(strstr(buffer, "\"beskrivning\": \"Clear sky\"") != NULL);
}

void test_skapa_vader_json_svenska_tecken() {
    VaderData data = {
        .stad = "Göteborg",  // ö
        .land = "SE",
        .beskrivning = "Lätt regn"  // ä
    };

    char buffer[2048];
    skapa_vader_json(&data, buffer, sizeof(buffer));

    // JSON ska innehålla städen (kan vara escaped eller UTF-8)
    assert(strstr(buffer, "teborg") != NULL);  // Kolla att "Göteborg" finns (utan ö)
}

// ============================================================================
// TESTER FÖR SKAPA_PROGNOS_JSON
// ============================================================================

void test_skapa_prognos_json_flera_dagar() {
    VaderPrognos prognos;
    prognos.antal_dagar = 3;

    // Dag 1
    strcpy(prognos.dagar[0].stad, "Stockholm");
    strcpy(prognos.dagar[0].land, "SE");
    prognos.dagar[0].temperatur = 20.0f;
    strcpy(prognos.dagar[0].beskrivning, "Sunny");

    // Dag 2
    strcpy(prognos.dagar[1].stad, "Stockholm");
    strcpy(prognos.dagar[1].land, "SE");
    prognos.dagar[1].temperatur = 18.0f;
    strcpy(prognos.dagar[1].beskrivning, "Cloudy");

    // Dag 3
    strcpy(prognos.dagar[2].stad, "Stockholm");
    strcpy(prognos.dagar[2].land, "SE");
    prognos.dagar[2].temperatur = 22.0f;
    strcpy(prognos.dagar[2].beskrivning, "Rainy");

    char buffer[4096];
    skapa_prognos_json(&prognos, buffer, sizeof(buffer));

    // Kontrollera att alla dagar finns
    assert(strstr(buffer, "\"temperatur\": 20.0") != NULL);
    assert(strstr(buffer, "\"temperatur\": 18.0") != NULL);
    assert(strstr(buffer, "\"temperatur\": 22.0") != NULL);
    assert(strstr(buffer, "\"Sunny\"") != NULL);
    assert(strstr(buffer, "\"Cloudy\"") != NULL);
    assert(strstr(buffer, "\"Rainy\"") != NULL);
}

void test_skapa_prognos_json_tom() {
    VaderPrognos prognos;
    prognos.antal_dagar = 0;

    char buffer[4096];
    skapa_prognos_json(&prognos, buffer, sizeof(buffer));

    // Ska vara en tom array
    assert(strstr(buffer, "\"dagar\": []") != NULL);
}

// ============================================================================
// HUVUDFUNKTION
// ============================================================================

int main(void) {
    printf("\n╔═══════════════════════════════════════════════════════╗\n");
    printf("║          ENHETSTESTER FÖR JSON-HELPER               ║\n");
    printf("╚═══════════════════════════════════════════════════════╝\n\n");

    // Kör alla tester
    RUN_TEST(test_hamta_json_varde_enkel_strang);
    RUN_TEST(test_hamta_json_varde_med_mellanslag);
    RUN_TEST(test_hamta_json_varde_saknas);
    RUN_TEST(test_hamta_json_varde_tom_strang);

    RUN_TEST(test_hamta_json_float_positivt);
    RUN_TEST(test_hamta_json_float_negativt);
    RUN_TEST(test_hamta_json_float_heltal);
    RUN_TEST(test_hamta_json_float_saknas);

    RUN_TEST(test_skapa_vader_json_komplett);
    RUN_TEST(test_skapa_vader_json_svenska_tecken);

    RUN_TEST(test_skapa_prognos_json_flera_dagar);
    RUN_TEST(test_skapa_prognos_json_tom);

    // Visa resultat
    printf("\n╔═══════════════════════════════════════════════════════╗\n");
    printf("║                   TESTRESULTAT                       ║\n");
    printf("╚═══════════════════════════════════════════════════════╝\n");
    printf("  Totalt:    %d tester\n", tester_totalt);
    printf("  Godkända:  %d tester\n", tester_godkanda);
    printf("  Misslyckade: %d tester\n", tester_totalt - tester_godkanda);

    if (tester_godkanda == tester_totalt) {
        printf("\n  ✓ ALLA TESTER GODKÄNDA!\n\n");
        return 0;
    } else {
        printf("\n  ✗ VISSA TESTER MISSLYCKADES\n\n");
        return 1;
    }
}
