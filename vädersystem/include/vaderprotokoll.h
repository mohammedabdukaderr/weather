#ifndef VADERPROTOKOLL_H
#define VADERPROTOKOLL_H

#include <stdint.h>
#include <stdbool.h>

// Väderdata struktur
typedef struct {
    char stad[64];
    float temperatur;
    float luftfuktighet;
    float vindhastighet;
    float lufttryck;
    char beskrivning[128];
    char ikon_id[16];
    int64_t tidsstampel;
} VaderData;

// Väderprognos struktur
typedef struct {
    int antal_dagar;
    VaderData dagar[5];
} VaderPrognos;

#endif // VADERPROTOKOLL_H
