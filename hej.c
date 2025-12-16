#include <stdio.h>
#include <stdlib.h>

int main (){
    char *buffer = malloc(100);
    if (buffer == NULL) return 1;
    printf("Hej det har allkoretat i minnen\n "); 
    free (buffer);
    printf("Minnet har frigjort \n");
    


    return 0;

}