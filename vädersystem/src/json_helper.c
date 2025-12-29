#include <stdio.h>
#include "json_helper.h"  // Egna funktioner för JSON-parsning
#include <string.h>        // För strängfunktioner: strstr, strlen, strchr, memcpy
#include <stdlib.h>        // För strto*-funktioner: strtod, strtol
#include <ctype.h>         // För isspace (kontrollera whitespace)

/**
 * Hittar värdet för en given nyckel i en JSON-sträng
 *
 * @param json - JSON-strängen att söka i
 * @param nyckel - Nyckeln att leta efter (utan citattecken)
 * @return Pekare till början av värdet, eller NULL om nyckeln inte hittades
 *
 * Funktionen söker efter mönstret "nyckel": i JSON-strängen och returnerar
 * en pekare till positionen direkt efter kolon där värdet börjar.
 * Detta är en hjälpfunktion som används av de andra json_hamta_*-funktionerna.
 *
 * Exempel: För JSON {"stad":"Stockholm","temp":15.5}
 *          json_hamta_varde(json, "stad") returnerar pekare till "Stockholm"
 *          json_hamta_varde(json, "temp") returnerar pekare till 15.5
 */
const char* json_hamta_varde(const char* json, const char* nyckel) {
    // Säkerhetskontroll: om json eller nyckel är NULL, returnera NULL
    if (!json || !nyckel) return NULL;

    // Bygg söksträngen i formatet "nyckel": (med citattecken och kolon)
    // Detta är hur nycklar formateras i JSON-objekt
    char sokstrang[128];
    snprintf(sokstrang, sizeof(sokstrang), "\"%s\":", nyckel);

    // Sök efter den exakta söksträngen i JSON-datan
    const char* pos = strstr(json, sokstrang);
    if (!pos) return NULL;  // Nyckeln hittades inte

    // Gå förbi själva nyckeln och kolonet för att hamna vid värdet
    // Exempel: "stad":"Stockholm" -> pekar nu på "Stockholm"
    pos += strlen(sokstrang);

    // Hoppa över eventuella whitespace-tecken (mellanslag, tab, newline)
    // JSON tillåter whitespace mellan kolon och värde: "stad": "Stockholm"
    while (*pos && isspace((unsigned char)*pos)) pos++;

    // Returnera pekare till början av värdet
    return pos;
}

/**
 * Hämtar ett numeriskt värde (flyttal) från JSON
 *
 * @param json - JSON-strängen att söka i
 * @param nyckel - Nyckeln vars värde ska hämtas
 * @return Värdet som double, eller 0.0 om nyckeln inte hittades
 *
 * Funktionen extraherar decimaltal från JSON. Den kan hantera både
 * heltal och flyttal: 15, 15.5, -5.3, 0.001, etc.
 *
 * Exempel: För JSON {"temperatur":15.5}
 *          json_hamta_nummer(json, "temperatur") returnerar 15.5
 */
double json_hamta_nummer(const char* json, const char* nyckel) {
    // Hitta positionen där värdet börjar
    const char* varde_pos = json_hamta_varde(json, nyckel);
    if (!varde_pos) return 0.0;  // Nyckeln hittades inte

    // Konvertera strängen till ett flyttal med strtod
    // strtod läser siffror från strängen och stoppar vid första icke-numeriska tecknet
    // Exempel: "15.5," läses som 15.5 (kommat ignoreras)
    return strtod(varde_pos, NULL);
}

/**
 * Hämtar ett heltalsvärde från JSON
 *
 * @param json - JSON-strängen att söka i
 * @param nyckel - Nyckeln vars värde ska hämtas
 * @return Värdet som int, eller 0 om nyckeln inte hittades
 *
 * Funktionen extraherar heltal från JSON. Den kan hantera både
 * positiva och negativa tal: 100, -42, 0, etc.
 *
 * Exempel: För JSON {"luftfuktighet":65}
 *          json_hamta_heltal(json, "luftfuktighet") returnerar 65
 */
int json_hamta_heltal(const char* json, const char* nyckel) {
    // Hitta positionen där värdet börjar
    const char* varde_pos = json_hamta_varde(json, nyckel);
    if (!varde_pos) return 0;  // Nyckeln hittades inte

    // Konvertera strängen till ett heltal med strtol
    // Tredje parametern (10) betyder bas 10 (decimalt tal)
    // Returnvärdet castas till int (strtol returnerar long)
    return (int)strtol(varde_pos, NULL, 10);
}

/**
 * Hämtar ett strängvärde från JSON (utan citattecken)
 *
 * @param json - JSON-strängen att söka i
 * @param nyckel - Nyckeln vars värde ska hämtas
 * @param buffer - Buffert där strängen ska kopieras
 * @param storlek - Storlek på bufferten i bytes
 * @return true om strängen hittades och kopierades, false vid fel
 *
 * Funktionen extraherar textsträngar från JSON och tar bort citattecknen.
 * Den kopierar bara innehållet mellan citattecknen till bufferten.
 *
 * Exempel: För JSON {"stad":"Stockholm"}
 *          json_hamta_strang(json, "stad", buf, sizeof(buf)) kopierar "Stockholm" till buf
 *          (utan citattecknen)
 */
bool json_hamta_strang(const char* json, const char* nyckel, char* buffer, size_t storlek) {
    // Hitta positionen där värdet börjar
    const char* varde_pos = json_hamta_varde(json, nyckel);
    if (!varde_pos) return false;  // Nyckeln hittades inte

    // I JSON är strängar omgivna av citattecken: "värde"
    // Kontrollera att värdet börjar med ett öppnande citattecken
    if (*varde_pos != '"') return false;  // Inte en sträng

    varde_pos++;  // Hoppa över öppnande citattecken, pekar nu på första tecknet i strängen

    // Hitta det stängande citattecknet som markerar slutet på strängen
    const char* slut = strchr(varde_pos, '"');
    if (!slut) return false;  // Inget stängande citattecken hittades (ogiltig JSON)

    // Beräkna längden på strängen (avståndet mellan start och slut)
    size_t langd = (size_t)(slut - varde_pos);

    // Säkerställ att strängen får plats i bufferten
    // Vi måste lämna plats för null-terminatorn (\0)
    if (langd >= storlek) langd = storlek - 1;

    // Kopiera strängdatan till bufferten
    memcpy(buffer, varde_pos, langd);

    // Lägg till null-terminator för att göra det en korrekt C-sträng
    buffer[langd] = '\0';

    return true;  // Framgång
}

/**
 * Hittar det första objektet i en JSON-array
 *
 * @param json - JSON-strängen att söka i
 * @param array_nyckel - Nyckeln för arrayen
 * @return Pekare till början av första objektet i arrayen, eller NULL vid fel
 *
 * Funktionen letar efter en array i JSON och returnerar en pekare till
 * det första objektet i den arrayen. Detta är användbart för att parsa
 * listor av objekt i JSON-svar från API:er.
 *
 * Exempel: För JSON {"weather":[{"id":800,"main":"Clear"},{"id":801}]}
 *          json_hamta_forsta_array_objekt(json, "weather") returnerar pekare till {"id":800,...}
 */
const char* json_hamta_forsta_array_objekt(const char* json, const char* array_nyckel) {
    // Hitta positionen där array-värdet börjar
    const char* array_pos = json_hamta_varde(json, array_nyckel);
    if (!array_pos) return NULL;  // Array-nyckeln hittades inte

    // I JSON börjar en array med hakparentes: [
    // Kontrollera att värdet verkligen är en array
    if (*array_pos != '[') return NULL;  // Inte en array

    array_pos++;  // Hoppa över öppnande hakparentes, pekar nu inuti arrayen

    // Hoppa över eventuella whitespace-tecken efter öppnande [
    // JSON tillåter whitespace: [ {"id":1}, {"id":2} ]
    while (*array_pos && isspace((unsigned char)*array_pos)) array_pos++;

    // I JSON börjar objekt med klammerparentes: {
    // Första objektet i arrayen ska börja med {
    if (*array_pos != '{') return NULL;  // Tomt eller inte ett objekt

    // Returnera pekare till början av första objektet
    return array_pos;
}
