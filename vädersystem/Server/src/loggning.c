#include "loggning.h"
#include <stdarg.h>  // För variabla argumentlistor (va_list, va_start, va_end)
#include <string.h>  // För stränghantering (strrchr, strftime)

// Global variabel som håller den aktuella loggningsnivån
// Endast meddelanden med denna nivå eller högre kommer att skrivas ut
LogNiva aktuell_log_niva = LOG_NIVA_INFO;

// Filpekare till loggfilen där alla meddelanden sparas
static FILE* logg_fil = NULL;

/**
 * Initierar loggningssystemet
 *
 * @param niva - Den minimala nivå som ska loggas (DEBUG, INFO, VARNING, FEL)
 *
 * Funktionen öppnar en loggfil (vaderserver.log) i läget "append" (a)
 * så att tidigare loggmeddelanden inte skrivs över. Om filen inte kan
 * öppnas fortsätter programmet men loggar endast till konsolen.
 */
void initiera_loggning(LogNiva niva) {
    // Sätt den globala loggningsnivån till det värde som skickats in
    aktuell_log_niva = niva;

    // Försök öppna loggfilen för att lägga till meddelanden (append mode)
    logg_fil = fopen("vaderserver.log", "a");

    // Om filen inte kunde öppnas, skriv en varning till stderr
    if (!logg_fil) {
        fprintf(stderr, "VARNING: Kunde inte öppna loggfil, loggar endast till konsol\n");
    }
}

/**
 * Skriv ett loggmeddelande till både konsol och fil
 *
 * @param niva - Meddelandets prioritetsnivå (DEBUG, INFO, VARNING, FEL)
 * @param fil - Namnet på källkodsfilen som anropar funktionen (__FILE__)
 * @param rad - Radnumret i källkoden där funktionen anropas (__LINE__)
 * @param format - Formatsträng (som printf) för meddelandet
 * @param ... - Variabla argument som motsvarar formatsträngen
 *
 * Funktionen formaterar meddelandet med tidsstämpel, nivå, fil och radnummer
 * och skriver det till både stdout/stderr och loggfilen.
 */
void skriv_logg(LogNiva niva, const char* fil, int rad, const char* format, ...) {
    // Om meddelandets nivå är lägre än den aktuella loggningsnivån,
    // hoppa över detta meddelande (för att minska output)
    if (niva < aktuell_log_niva) {
        return;
    }

    // Hämta aktuell systemtid för att tidsstämpla meddelandet
    time_t nu = time(NULL);                      // Hämta nuvarande tid i sekunder sedan 1970
    struct tm* tid_info = localtime(&nu);        // Konvertera till lokal tid (år, månad, dag, etc)
    char tid_strang[64];                         // Buffer för den formaterade tidssträngen

    // Formatera tiden som "YYYY-MM-DD HH:MM:SS" (ex: "2025-12-25 15:30:45")
    strftime(tid_strang, sizeof(tid_strang), "%Y-%m-%d %H:%M:%S", tid_info);

    // Array med textrepresentationer av loggningsnivåerna
    const char* niva_texter[] = {"DEBUG", "INFO", "VARNING", "FEL"};

    // Välj rätt text baserat på nivån, eller "OKÄND" om nivån är ogiltig
    const char* niva_str = (niva <= LOG_NIVA_FEL) ? niva_texter[niva] : "OKÄND";

    // Extrahera endast filnamnet från den fullständiga sökvägen
    // t.ex. från "/path/to/fil.c" eller "C:\path\to\fil.c" får vi bara "fil.c"
    const char* filnamn = strrchr(fil, '/');      // Leta efter sista '/' (Linux/Mac)
    if (!filnamn) {
        filnamn = strrchr(fil, '\\');             // Om inte hittad, leta efter '\' (Windows)
    }
    filnamn = filnamn ? filnamn + 1 : fil;        // Hoppa förbi '/' eller '\', eller använd hela om ingen hittades

    // Bygg själva loggmeddelandet från de variabla argumenten
    char meddelande[1024];                        // Buffer för det färdiga meddelandet
    va_list argument_lista;                       // Lista för de variabla argumenten
    va_start(argument_lista, format);             // Initialisera listan från 'format' och framåt
    vsnprintf(meddelande, sizeof(meddelande), format, argument_lista);  // Formatera meddelandet
    va_end(argument_lista);                       // Städa upp argumentlistan

    // Välj output-ström beroende på allvarlighetsgrad
    // Fel ska gå till stderr (standard error), övriga till stdout (standard output)
    FILE* ut_strom = (niva >= LOG_NIVA_FEL) ? stderr : stdout;

    // Skriv det kompletta loggmeddelandet till konsolen
    // Format: [Tidsstämpel] [Nivå] filnamn:rad - meddelande
    fprintf(ut_strom, "[%s] [%s] %s:%d - %s\n",
            tid_strang, niva_str, filnamn, rad, meddelande);

    // Om loggfilen är öppen, skriv samma meddelande dit också
    if (logg_fil) {
        fprintf(logg_fil, "[%s] [%s] %s:%d - %s\n",
                tid_strang, niva_str, filnamn, rad, meddelande);

        // Töm filbufferten omedelbart så att meddelandet verkligen skrivs till disk
        // Detta är viktigt om programmet kraschar - då finns meddelandet kvar
        fflush(logg_fil);
    }
}

/**
 * Stäng loggningssystemet och frigör resurser
 *
 * Stänger loggfilen och sätter pekaren till NULL för att förhindra
 * att stängd fil används av misstag senare.
 */
void stang_loggning(void) {
    // Om loggfilen är öppen, stäng den
    if (logg_fil) {
        fclose(logg_fil);              // Stäng filen och skriv ut eventuella buffrade data
        logg_fil = NULL;               // Sätt pekaren till NULL för att markera att den är stängd
    }
}
