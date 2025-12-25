#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "../libs/HTTP.h"

// Ansluter till vår väderserver
int connect_to_server(const char* ip, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        return -1;
    }
    
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        return -1;
    }
    
    return sockfd;
}

// Skickar en HTTP-förfrågan och visar resultatet
void get_weather_for_city(const char* ip, int port, const char* city) {
    int sockfd = connect_to_server(ip, port);
    if (sockfd < 0) {
        printf("Kunde inte ansluta till servern\n");
        return;
    }
    
    // Skapa HTTP-förfrågan
    char request[512];
    snprintf(request, sizeof(request),
             "GET /weather/%s HTTP/1.1\r\n"
             "Host: %s:%d\r\n"
             "User-Agent: VäderKlient/1.0\r\n"
             "Accept: application/json\r\n"
             "Connection: close\r\n"
             "\r\n",
             city, ip, port);
    
    // Skicka förfrågan
    if (send(sockfd, request, strlen(request), 0) < 0) {
        perror("send");
        close(sockfd);
        return;
    }
    
    // Ta emot svar
    char response[4096];
    ssize_t total_received = 0;
    ssize_t received;
    
    while ((received = recv(sockfd, response + total_received, 
                           sizeof(response) - total_received - 1, 0)) > 0) {
        total_received += received;
        if (total_received >= sizeof(response) - 1) break;
    }
    
    response[total_received] = '\0';
    
    // Parsa HTTP-svaret
    char* body_start = strstr(response, "\r\n\r\n");
    if (body_start) {
        body_start += 4; // Skip headers
        printf("\n=== Väder för %s ===\n", city);
        printf("%s\n", body_start);
        printf("===========================\n");
    } else {
        printf("Ogiltigt svar från servern\n");
    }
    
    close(sockfd);
}

int main(int argc, char* argv[]) {
    printf("=== VÄDERKLIENT ===\n");
    printf("Ansluter till vår egen väderserver\n\n");
    
    char* server_ip = "127.0.0.1";
    int server_port = 8080;
    char* city = "Stockholm";
    
    // Läs kommandoradsargument
    if (argc > 1) city = argv[1];
    if (argc > 2) server_ip = argv[2];
    if (argc > 3) server_port = atoi(argv[3]);
    
    printf("Server: %s:%d\n", server_ip, server_port);
    printf("Stad:   %s\n\n", city);
    
    get_weather_for_city(server_ip, server_port, city);
    
    // Testa flera städer om ingen angavs
    if (argc == 1) {
        printf("\n--- Testar flera städer ---\n");
        const char* cities[] = {"Göteborg", "Malmö", "Uppsala", NULL};
        
        for (int i = 0; cities[i]; i++) {
            sleep(1); // Vänta lite mellan förfrågningar
            get_weather_for_city(server_ip, server_port, cities[i]);
        }
    }
    
    return 0;
}