#include "cache.h"         // Egna funktioner för cache-hantering
#include "loggning.h"       // För att logga debug-meddelanden och varningar
#include "konfiguration.h"  // För CACHE_KATALOG och CACHE_GILTIGHETSTID
#include <stdio.h>          // För filhantering: fopen, fread, fwrite, fclose
#include <string.h>         // För strängfunktioner: strcmp
#include <time.h>           // För tidshantering: time(), tidsstämplar

#ifdef _WIN32
    #include <direct.h>     // Windows-specifik: för _mkdir
    #define mkdir(dir, mode) _mkdir(dir)  // Anpassa mkdir för Windows (ignorerar mode)
    #include <sys/stat.h>   // För stat-struktur (filinformation)
#else
    #include <sys/stat.h>   // Unix/Linux: för stat-struktur och mkdir
    #include <sys/types.h>  // Unix/Linux: för datatyper som används av systemanrop
    #include <dirent.h>     // Unix/Linux: för att läsa kataloginnehåll (opendir, readdir)
    #include <unistd.h>     // Unix/Linux: för unlink (ta bort filer)
#endif

/**
 * Hjälpfunktion: Skapar ett standardiserat filnamn för cache-filer
 *
 * @param stad - Stadens namn (t.ex. "Stockholm")
 * @param landskod - Landskod (t.ex. "SE" för Sverige)
 * @param typ - Typ av cache ("vader" eller "prognos")
 * @param buffer - Buffert där det skapade filnamnet ska lagras
 * @param storlek - Storlek på bufferten i bytes
 *
 * Funktionen skapar filnamn enligt mönstret: "./cache/Stockholm_SE_vader.cache"
 * Detta gör att varje stad och land får sin egen cache-fil, och vi kan skilja
 * mellan aktuellt väder och prognoser.
 */
static void skapa_cache_filnamn(const char* stad, const char* landskod,
                                 const char* typ, char* buffer, size_t storlek) {
    // Formatera filnamnet: katalog/stad_landskod_typ.cache
    // Exempel: "./cache/Stockholm_SE_vader.cache"
    snprintf(buffer, storlek, "%s/%s_%s_%s.cache",
             CACHE_KATALOG, stad, landskod, typ);
}

/**
 * Initierar cache-systemet genom att skapa cache-katalogen
 *
 * @return true om initieringen lyckades, false vid fel
 *
 * Funktionen kontrollerar om cache-katalogen finns. Om den inte finns
 * skapas den automatiskt. Detta säkerställer att vi kan spara cache-filer
 * utan att programmet kraschar.
 */
bool initiera_cache(void) {
    // stat-struktur för att hämta information om filer/kataloger
    // Nollställ den för att undvika skräpdata
    struct stat st = {0};

    // Kontrollera om cache-katalogen redan finns
    // stat returnerar 0 om filen/katalogen finns, -1 annars
    if (stat(CACHE_KATALOG, &st) == -1) {
        // Katalogen finns inte, vi måste skapa den

        // mkdir skapar katalogen med rättigheter 0755 (ägare: rwx, andra: rx)
        // På Windows ignoreras mode-parametern (därför redefinieringen ovan)
        if (mkdir(CACHE_KATALOG, 0755) != 0) {
            LOGG_FEL("Kunde inte skapa cache-katalog: %s", CACHE_KATALOG);
            return false;
        }

        LOGG_INFO("Skapade cache-katalog: %s", CACHE_KATALOG);
    }

    return true;
}

/**
 * Läser cachad väderdata från fil
 *
 * @param stad - Stadens namn att söka cache för
 * @param landskod - Landskod att söka cache för
 * @param resultat - Pekare till VaderData-struktur där data ska lagras
 * @return true om giltig cache hittades, false vid cache miss eller utgången cache
 *
 * Funktionen försöker läsa väderdata från en cache-fil. Om filen finns
 * kontrollerar den också om datan är tillräckligt färsk (inom CACHE_GILTIGHETSTID).
 * Om cache är för gammal returneras false så att nytt API-anrop kan göras.
 */
bool las_fran_cache(const char* stad, const char* landskod, VaderData* resultat) {
    // Bygg filnamnet för denna specifika stads väder-cache
    char filnamn[256];
    skapa_cache_filnamn(stad, landskod, "vader", filnamn, sizeof(filnamn));

    // Försök öppna cache-filen i binärläsläge ("rb" = read binary)
    // Vi använder binärläge för att läsa VaderData-strukturen direkt
    FILE* fil = fopen(filnamn, "rb");
    if (!fil) {
        // Filen finns inte - detta är en "cache miss"
        LOGG_DEBUG("Cache miss: %s", filnamn);
        return false;
    }

    // Läs väderdata-strukturen direkt från filen
    // fread läser sizeof(VaderData) bytes och fyller resultat-strukturen
    // Returnerar antal element som lyckades läsas (ska vara 1)
    size_t last = fread(resultat, sizeof(VaderData), 1, fil);
    fclose(fil);  // Stäng filen direkt efter läsning

    if (last != 1) {
        // Kunde inte läsa hela strukturen - filen kan vara korrupt
        LOGG_VARNING("Kunde inte läsa cache-fil: %s", filnamn);
        return false;
    }

    // Kontrollera om cache-datan fortfarande är giltig
    // Vi jämför tiden nu mot tidsstämpeln i den cachade datan
    time_t nu = time(NULL);  // Hämta nuvarande tid i sekunder sedan 1970

    if ((nu - resultat->tidsstampel) > CACHE_GILTIGHETSTID) {
        // Cache är för gammal (äldre än CACHE_GILTIGHETSTID sekunder)
        // Logga hur gammal cachen var så vi kan felsöka
        LOGG_DEBUG("Cache utgången: %s (ålder: %ld sekunder)",
                   filnamn, (long)(nu - resultat->tidsstampel));
        return false;
    }

    // Cache är giltig! Logga framgång och hur färsk datan är
    LOGG_INFO("Cache hit: %s (ålder: %ld sekunder)",
              filnamn, (long)(nu - resultat->tidsstampel));
    return true;
}

/**
 * Skriver väderdata till cache-fil
 *
 * @param stad - Stadens namn att cacha för
 * @param landskod - Landskod att cacha för
 * @param data - Pekare till VaderData-struktur som ska sparas
 * @return true om skrivningen lyckades, false vid fel
 *
 * Funktionen sparar väderdata till en binär cache-fil. Detta gör att
 * nästa gång samma stad efterfrågas kan vi läsa från cache istället
 * för att göra ett nytt API-anrop, vilket sparar tid och API-krediter.
 */
bool skriv_till_cache(const char* stad, const char* landskod, const VaderData* data) {
    // Bygg filnamnet för denna specifika stads väder-cache
    char filnamn[256];
    skapa_cache_filnamn(stad, landskod, "vader", filnamn, sizeof(filnamn));

    // Öppna filen för binärskrivning ("wb" = write binary)
    // Om filen redan finns skrivs den över med ny data
    FILE* fil = fopen(filnamn, "wb");
    if (!fil) {
        LOGG_VARNING("Kunde inte öppna cache-fil för skrivning: %s", filnamn);
        return false;
    }

    // Skriv hela VaderData-strukturen direkt till filen
    // fwrite returnerar antal element som skrevs (ska vara 1)
    size_t skrivet = fwrite(data, sizeof(VaderData), 1, fil);
    fclose(fil);  // Stäng filen direkt efter skrivning

    if (skrivet != 1) {
        // Kunde inte skriva hela strukturen - disken kan vara full
        LOGG_VARNING("Kunde inte skriva till cache-fil: %s", filnamn);
        return false;
    }

    // Skrivning lyckades! Logga för debugging
    LOGG_DEBUG("Cachade väderdata: %s", filnamn);
    return true;
}

/**
 * Läser cachad prognosdata från fil
 *
 * @param stad - Stadens namn att söka cache för
 * @param landskod - Landskod att söka cache för
 * @param resultat - Pekare till VaderPrognos-struktur där data ska lagras
 * @return true om giltig cache hittades, false vid cache miss eller utgången cache
 *
 * Funktionen fungerar som las_fran_cache men för prognosdata istället.
 * Eftersom prognoser innehåller flera dagar använder vi första dagens
 * tidsstämpel för att avgöra om cachen är giltig.
 */
bool las_prognos_fran_cache(const char* stad, const char* landskod, VaderPrognos* resultat) {
    // Bygg filnamnet för denna specifika stads prognos-cache
    char filnamn[256];
    skapa_cache_filnamn(stad, landskod, "prognos", filnamn, sizeof(filnamn));

    // Försök öppna cache-filen i binärläsläge
    FILE* fil = fopen(filnamn, "rb");
    if (!fil) {
        LOGG_DEBUG("Cache miss: %s", filnamn);
        return false;
    }

    // Läs prognos-strukturen direkt från filen
    size_t last = fread(resultat, sizeof(VaderPrognos), 1, fil);
    fclose(fil);

    if (last != 1) {
        LOGG_VARNING("Kunde inte läsa cache-fil: %s", filnamn);
        return false;
    }

    // Kontrollera om cache är giltig genom att kolla första dagens tidsstämpel
    // Vi använder första dagen eftersom det är den mest relevanta
    if (resultat->antal_dagar > 0) {
        time_t nu = time(NULL);

        // Jämför nuvarande tid med första dagens tidsstämpel
        if ((nu - resultat->dagar[0].tidsstampel) > CACHE_GILTIGHETSTID) {
            LOGG_DEBUG("Cache utgången: %s", filnamn);
            return false;
        }
    }

    LOGG_INFO("Cache hit: %s", filnamn);
    return true;
}

/**
 * Skriver prognosdata till cache-fil
 *
 * @param stad - Stadens namn att cacha för
 * @param landskod - Landskod att cacha för
 * @param data - Pekare till VaderPrognos-struktur som ska sparas
 * @return true om skrivningen lyckades, false vid fel
 *
 * Funktionen sparar en hel prognos-struktur (med flera dagar) till cache.
 * Detta gör att vi kan återanvända prognosdata utan att göra nya API-anrop.
 */
bool skriv_prognos_till_cache(const char* stad, const char* landskod,
                               const VaderPrognos* data) {
    // Bygg filnamnet för denna specifika stads prognos-cache
    char filnamn[256];
    skapa_cache_filnamn(stad, landskod, "prognos", filnamn, sizeof(filnamn));

    // Öppna filen för binärskrivning
    FILE* fil = fopen(filnamn, "wb");
    if (!fil) {
        LOGG_VARNING("Kunde inte öppna cache-fil för skrivning: %s", filnamn);
        return false;
    }

    // Skriv hela VaderPrognos-strukturen till filen
    size_t skrivet = fwrite(data, sizeof(VaderPrognos), 1, fil);
    fclose(fil);

    if (skrivet != 1) {
        LOGG_VARNING("Kunde inte skriva till cache-fil: %s", filnamn);
        return false;
    }

    LOGG_DEBUG("Cachade prognos: %s", filnamn);
    return true;
}

/**
 * Rensar gamla cache-filer från cache-katalogen
 *
 * Funktionen går igenom alla filer i cache-katalogen och tar bort de som
 * är äldre än CACHE_GILTIGHETSTID. Detta förhindrar att cache-katalogen
 * växer obegränsat med gamla, oanvända filer.
 *
 * OBS: Windows-versionen är inte implementerad eftersom FindFirstFile/FindNextFile
 * kräver mer kod. På Windows görs ingen automatisk cache-rensning.
 */
void rensa_gammal_cache(void) {
#ifdef _WIN32
    // Windows-implementation är förenklad - skulle behöva FindFirstFile/FindNextFile API
    // Detta kräver mer komplex kod än Unix-versionen, så vi hoppar över det för nu
    LOGG_DEBUG("Cache-rensning inte implementerad för Windows");
#else
    // Unix/Linux-implementation använder opendir/readdir för att lista filer

    // Öppna cache-katalogen för att läsa dess innehåll
    DIR* dir = opendir(CACHE_KATALOG);
    if (!dir) {
        LOGG_VARNING("Kunde inte öppna cache-katalog för rensning");
        return;
    }

    struct dirent* post;  // Struktur för varje katalogpost (fil/underkatalog)
    time_t nu = time(NULL);  // Hämta nuvarande tid för att jämföra fiåldrar
    int rensade = 0;  // Räknare för antal borttagna filer

    // Iterera genom alla poster i katalogen
    while ((post = readdir(dir)) != NULL) {
        // Hoppa över specialposterna "." (aktuell katalog) och ".." (föräldrakatalog)
        // Dessa är inte riktiga filer och kan inte tas bort
        if (strcmp(post->d_name, ".") == 0 || strcmp(post->d_name, "..") == 0) {
            continue;
        }

        // Bygg den fullständiga sökvägen till filen
        // Vi behöver fullständig sökväg för stat() och unlink()
        char sokvag[512];
        snprintf(sokvag, sizeof(sokvag), "%s/%s", CACHE_KATALOG, post->d_name);

        // Hämta filinformation (storlek, tidsstämpel, rättigheter, etc.)
        struct stat fil_info;
        if (stat(sokvag, &fil_info) == 0) {
            // Kontrollera om filen är äldre än CACHE_GILTIGHETSTID
            // st_mtime är tiden då filen senast modifierades
            if ((nu - fil_info.st_mtime) > CACHE_GILTIGHETSTID) {
                // Filen är för gammal, ta bort den med unlink()
                if (unlink(sokvag) == 0) {
                    LOGG_DEBUG("Rensade gammal cache-fil: %s", post->d_name);
                    rensade++;
                }
            }
        }
    }

    // Stäng katalogen när vi är klara
    closedir(dir);

    // Logga resultat om några filer rensades
    if (rensade > 0) {
        LOGG_INFO("Rensade %d gamla cache-filer", rensade);
    }
#endif
}
