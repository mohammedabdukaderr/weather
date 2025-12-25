#include <stdio.h>
#include <math.h>
#include <string.h>

int main()
{
    char name[20] = "";
    int age;
    int area;
    int nummer1;
    int nummer2;
    int radie;
    char resultat;

    printf("Hej vad heter du ?\n");
    fgets(name, sizeof(name), stdin);
    getchar();

    printf("Hur gammal är du?: \n ");
    scanf("%d", &age);

    printf("Skiv in ett nummer %s", name);
    scanf("%d", &nummer1);

    printf("skriv en till nummer för att räkna ut arean av en cirkel:n");
    scanf("%d", &nummer2);

    printf("skriv radie\n ");

    scanf("%d", &radie);

    printf("Skriv 1 för att räkna ut och 2 exit");
    scanf("%d", &area);

    while (1)
    {
        resultat = nummer1 * nummer2 * pow(radie, 2);
        printf("arean av cikel är är %d", resultat);
    }
    return 0;
}