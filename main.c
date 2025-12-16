#include <stdio.h>          // Inkluderar standard I/O-funktioner
#include "weather.h"        // Inkluderar vår egen header-fil

int main(void) {            // Huvudfunktion - programmets startpunkt
    printf("Välkommen till Väderprogrammet!!!\n");  // Skriver välkomstmeddelande
    
    while (1) {             // Oändlig loop för att köra programmet flera gånger
        show_menu();        // Anropar funktion för att visa menyn
        int choice = get_choice();  // Anropar funktion för att läsa användarval
        
        if (choice == 0) break;     // Bryter loopen om användaren valde 0 (avsluta)
        
        get_weather(choice);        // Anropar funktion för att hämta väderdata
        
        printf("\nVill du söka igen? (j för ja, annat för nej): ");  // Frågar om ny sökning
        char again;                 // Variabel för användarens svar
        scanf("%c", &again);        // Läser ett tecken från tangentbordet
        getchar();                  // Rensar bufferten
        
        if (again != 'j' && again != 'J') break;  // Bryter loopen om inte 'j' eller 'J'
    }
    
    printf("\nTack för att du använde Väderprogrammet!\n");  // Skriver avslutningsmeddelande
    return 0;                    // Returnerar 0 för att indikera framgång
}