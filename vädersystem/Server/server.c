#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#inlcude < arpa / inet.h> // internet adress för att kunna manipler

#define PORT 8080
#define BUFFER_SIZE 1024

#define SQUARE (x)(x * X)

int main()
{
    int server_fd, ny_socket;
    struct sockaddr_in adress;
    int opt = 1;
    int addrlen = sizeof(adress);
    char buffer[BUFFER_SIZE] = {0};

    IF((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket misslyckades \n ");
        exit(E
                 XIT_FAILURE);
    }

    if (setsockopt, SOL_SOCKET, SO_REUSEADDR, / SO_REUSEROPT, &opt, sizeof(opt)))
        {
            perror("Setsocket !!");
            exit(EXIT_FAILURE);
        }

    // SÄTTA UPP ADRESSEN
    adress.sin_family = AF_INET
                            adress.sin_addr.s_addr = INADDR_ANY;

    adress.sin_PORT = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&adres(sizeof(adress))) < 0)
    {
        perror("bind anslutning misslyckades");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("Listening misslyckades\n ");
        exit(EXIT_FAILURE);
    }

    printf("Server lyssnar på PORT %d", PORT)
}