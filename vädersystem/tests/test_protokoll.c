#include "vaderprotokoll.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

// Testfunktion: Kontrollera protokollhuvud-storlek
void test_protokollhuvud_storlek() {
    printf("Test: Protokollhuvud storlek... ");
    assert(sizeof(ProtokollHuvud) == 4);  // 1 + 1 + 2 bytes
    printf("OK\n");
}

// Testfunktion: Kontrollera väderdata-struktur
void test_vaderdata_struktur() {
    printf("Test: VaderData struktur... ");

    VaderData data;
    memset(&data, 0, sizeof(data));

    // Sätt testdata
    strcpy(data.stad, "Stockholm");
    data.temperatur = 15.5f;
    data.luftfuktighet = 65.0f;
    data.vindhastighet = 3.2f;
    data.lufttryck = 1013.0f;
    strcpy(data.beskrivning, "Klart väder");
    strcpy(data.ikon_id, "01d");
    data.tidsstampel = 1234567890;

    // Verifiera
    assert(strcmp(data.stad, "Stockholm") == 0);
    assert(data.temperatur > 15.0f && data.temperatur < 16.0f);
    assert(strcmp(data.beskrivning, "Klart väder") == 0);

    printf("OK\n");
}

// Testfunktion: Byteordning-konvertering
void test_byteordning() {
    printf("Test: Byteordning (hton16/ntoh16)... ");

    uint16_t original = 0x1234;
    uint16_t konverterad = hton16(original);
    uint16_t tillbaka = ntoh16(konverterad);

    assert(original == tillbaka);

    printf("OK\n");
}

// Testfunktion: Meddelandetyper
void test_meddelandetyper() {
    printf("Test: Meddelandetyper... ");

    assert(MEDDELANDE_HAMTA_VADER == 1);
    assert(MEDDELANDE_HAMTA_PROGNOS == 2);
    assert(MEDDELANDE_VADER_SVAR == 3);
    assert(MEDDELANDE_FEL == 4);

    printf("OK\n");
}

// Testfunktion: Felkoder
void test_felkoder() {
    printf("Test: Felkoder... ");

    assert(FEL_OK == 0);
    assert(FEL_OGILTIG_STAD == 1);
    assert(FEL_API_FEL == 2);
    assert(FEL_NÄTVERKSFEL == 3);
    assert(FEL_TIMEOUT == 4);
    assert(FEL_OKÄNT == 99);

    printf("OK\n");
}

// Testfunktion: VaderPrognos struktur
void test_vaderprognos() {
    printf("Test: VaderPrognos struktur... ");

    VaderPrognos prognos;
    memset(&prognos, 0, sizeof(prognos));

    prognos.antal_dagar = 3;
    strcpy(prognos.dagar[0].stad, "Stockholm");
    prognos.dagar[0].temperatur = 15.0f;

    strcpy(prognos.dagar[1].stad, "Stockholm");
    prognos.dagar[1].temperatur = 18.0f;

    strcpy(prognos.dagar[2].stad, "Stockholm");
    prognos.dagar[2].temperatur = 20.0f;

    assert(prognos.antal_dagar == 3);
    assert(prognos.dagar[0].temperatur == 15.0f);
    assert(prognos.dagar[2].temperatur == 20.0f);

    printf("OK\n");
}

// Huvudprogram
int main() {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════╗\n");
    printf("║        VÄDERSYSTEM - PROTOKOLLTESTER                  ║\n");
    printf("╚═══════════════════════════════════════════════════════╝\n");
    printf("\n");

    // Kör alla tester
    test_protokollhuvud_storlek();
    test_vaderdata_struktur();
    test_byteordning();
    test_meddelandetyper();
    test_felkoder();
    test_vaderprognos();

    printf("\n");
    printf("✓ Alla tester godkända!\n");
    printf("\n");

    return 0;
}
