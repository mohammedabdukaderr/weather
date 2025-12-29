#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "natverks_abstraktion.h"
#include <stdbool.h>

// HTTP-metoder
typedef enum {
    HTTP_GET,
    HTTP_POST,
    HTTP_UNKNOWN
} HttpMetod;

// HTTP-request struktur
typedef struct {
    HttpMetod metod;               // HTTP-metod (GET, POST)
    char sokvag[256];              // URL-sökväg (ex: "/weather")
    char query[512];               // Query-parametrar (ex: "city=Stockholm&country=SE")
    char body[1024];               // Request body (för POST)
} HttpRequest;

// Parsa HTTP-request från rå data
bool parsa_http_request(const char* rådata, HttpRequest* request);

// Skapa HTTP-response med JSON-data
void skapa_http_response(char* buffer, size_t buffer_storlek,
                         int statuskod, const char* json_data);

// Hämta query-parameter värde (ex: "city" från "city=Stockholm&country=SE")
bool hamta_query_parameter(const char* query, const char* parameter_namn,
                           char* värde, size_t värde_storlek);

#endif // HTTP_SERVER_H
