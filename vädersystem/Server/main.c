#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "../libs/TCP.h"
#include "../libs/HTTP.h"
#include "../libs/cache.h"

volatile sig_atomic_t running = 1;
WeatherCache* weather_cache = NULL;

void handle_signal(int sig) {
    running = 0;
    printf("\nStänger servern snyggt...\n");
}

// Hämtar väderdata (simulerat - i verkligheten från API)
char* fetch_weather(const char* city) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    
    char* json = malloc(512);
    snprintf(json, 512,
             "{\n"
             "  \"city\": \"%s\",\n"
             "  \"temperature\": %.1f,\n"
             "  \"conditions\": \"%s\",\n"
             "  \"humidity\": %d,\n"
             "  \"timestamp\": \"%04d-%02d-%02d %02d:%02d:%02d\",\n"
             "  \"source\": \"väderserver-v1.0\"\n"
             "}",
             city,
             15.0 + (rand() % 150) / 10.0,  // Random temp 15-30°C
             (rand() % 2) ? "Soligt" : "Molnigt",
             40 + (rand() % 40),  // 40-80% humidity
             tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
             tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
    
    return json;
}

// Hanterar en HTTP-förfrågan
void handle_request(int client_fd, const char* request) {
    HTTP_Request* req = http_parse_request(request);
    if (!req) {
        HTTP_Response* res = http_create_response(400, "text/plain", "Bad Request");
        char* response_str = http_response_to_string(res);
        tcp_send(client_fd, response_str, strlen(response_str));
        http_free_response(&res);
        free(response_str);
        return;
    }
    
    // Kolla om det är en väderförfrågan
    if (strcmp(req->method, "GET") == 0 && strncmp(req->path, "/weather/", 9) == 0) {
        const char* city = req->path + 9; // Skip "/weather/"
        
        // Försök hämta från cache först
        char* cached_data = cache_hamta(weather_cache, city);
        char* weather_json;
        int from_cache = 0;

        if (cached_data) {
            weather_json = strdup(cached_data);
            from_cache = 1;
            printf("Cache HIT för %s\n", city);
        } else {
            weather_json = fetch_weather(city);
            cache_lagg_till(weather_cache, city, weather_json, 300); // 5 min cache
            printf("Cache MISS för %s\n", city);
        }
        
        HTTP_Response* res = http_create_response(200, "application/json", weather_json);
        if (from_cache) {
            // Lägg till cache-header
            char new_headers[1024];
            snprintf(new_headers, sizeof(new_headers),
                     "%s"
                     "X-Cache: HIT\r\n"
                     "Cache-Control: max-age=300\r\n",
                     res->headers);
            free(res->headers);
            res->headers = strdup(new_headers);
        }
        
        char* response_str = http_response_to_string(res);
        tcp_send(client_fd, response_str, strlen(response_str));
        
        free(weather_json);
        http_free_response(&res);
        free(response_str);
    }
    else if (strcmp(req->path, "/health") == 0) {
        HTTP_Response* res = http_create_response(200, "application/json", 
            "{\"status\":\"healthy\",\"cache_size\":0}");
        char* response_str = http_response_to_string(res);
        tcp_send(client_fd, response_str, strlen(response_str));
        http_free_response(&res);
        free(response_str);
    }
    else {
        HTTP_Response* res = http_create_response(404, "text/plain", "Not Found");
        char* response_str = http_response_to_string(res);
        tcp_send(client_fd, response_str, strlen(response_str));
        http_free_response(&res);
        free(response_str);
    }
    
    http_free_request(&req);
}

int main(int argc, char** argv) {
    signal(SIGINT, handle_signal);
    
    printf("=== VÄDERSERVER v1.0 ===\n");
    printf("Bygger på kurskrav från v43-v46\n");
    printf("Innehåller: TCP/HTTP libs, cache, filhantering\n\n");
    
    // Initiera cache
    weather_cache = cache_skapa(100, "./weather_cache");
    
    // Skapa och starta TCP-server
    TCP_Server* server = tcp_create_server(8080, 10);
    if (!server || tcp_start_server(server) != 0) {
        fprintf(stderr, "Kunde inte starta server\n");
        return 1;
    }
    
    printf("Server kör på http://localhost:8080\n");
    printf("Endpoints:\n");
    printf("  GET /weather/{stad}    - Väder för specifik stad\n");
    printf("  GET /health           - Server health check\n");
    printf("Tryck Ctrl+C för att avsluta\n\n");
    
    // Huvudloop
    while (running) {
        TCP_Client* client = tcp_accept_client(server);
        if (client) {
            char buffer[4096];
            ssize_t received = tcp_receive(client->socket_fd, buffer, sizeof(buffer));
            
            if (received > 0) {
                handle_request(client->socket_fd, buffer);
            }
            
            tcp_close_client(&client);
        }
        
        usleep(10000); // 10ms
    }
    
    // Städning
    printf("\nSparar cache till fil...\n");
    cache_spara_till_fil(weather_cache, "weather_cache.dat");

    cache_fororsa(&weather_cache);
    tcp_destroy_server(&server);
    
    printf("Server avslutad\n");
    return 0;
}
