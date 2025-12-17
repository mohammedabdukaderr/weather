#include <jansson.h>
#include <stdio.h>
#include "json_parser.h"

// Funktion som parsar JSON och skriver ut aktuell väderdata
void skriv_ut_väder_json(const char *json_text) {
    json_error_t fel;
    json_t *rot = json_loads(json_text, 0, &fel);  // Parsar JSON-text

    if (!rot) {
        fprintf(stderr, "Fel vid parsning av JSON: %s\n", fel.text);
        return;
    }

    json_t *aktuellt_väder = json_object_get(rot, "current_weather");  // Hämtar current_weather
    if (!aktuellt_väder) {
        fprintf(stderr, "Ingen aktuell väderdata!\n");
        json_decref(rot);
        return;
    }

    double temperatur = json_real_value(json_object_get(aktuellt_väder, "temperature"));
    double vindhastighet = json_real_value(json_object_get(aktuellt_väder, "windspeed"));
    int vindriktning = (int)json_integer_value(json_object_get(aktuellt_väder, "winddirection"));

    printf("Temperatur: %.1f°C\n", temperatur);
    printf("Vindhastighet: %.1f km/h\n", vindhastighet);
    printf("Vindriktning: %d°\n", vindriktning);

    json_decref(rot);  // Frigör minnet
}
