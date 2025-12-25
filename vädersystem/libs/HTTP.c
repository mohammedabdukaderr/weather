#include "HTTP.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// Skapar en ny HTTP-förfrågan med malloc
HTTP_Request* http_parse(const char* raw_request) {
    if (!raw_request) return NULL;
    
    // Allokerar minne för strukturen (pekare skapas)
    HTTP_Request* req = malloc(sizeof(HTTP_Request));
    if (!req) return NULL;
    
    // Initiera alla pekare till NULL
    req->method = NULL;
    req->path = NULL;
    req->version = NULL;
    req->headers = NULL;
    req->body = NULL;
    
    // Exempel på parsing - hitta första mellanslaget
    const char* space = strchr(raw_request, ' ');
    if (space) {
        // Allokera minne för metoden
        size_t method_len = space - raw_request;
        req->method = malloc(method_len + 1);
        strncpy(req->method, raw_request, method_len);
        req->method[method_len] = '\0';
    }
    
    return req;
}

// Frigör minne - tar en pek-pekare för att sätta till NULL
void http_free_request(HTTP_Request** req_ptr) {
    if (!req_ptr || !*req_ptr) return;
    
    HTTP_Request* req = *req_ptr;
    
    // Frigör alla pekare inuti
    if (req->method) free(req->method);
    if (req->path) free(req->path);
    if (req->version) free(req->version);
    if (req->headers) free(req->headers);
    if (req->body) free(req->body);
    
    // Frigör strukturen och sätt pekaren till NULL
    free(req);
    *req_ptr = NULL;  // Viktigt för att undvika "dangling pointers"
}

// Skapar ett HTTP-svar
HTTP_Response* http_create_response(int status, const char* content_type, const char* body) {
    HTTP_Response* res = malloc(sizeof(HTTP_Response));
    if (!res) return NULL;

    res->status_code = status;

    // Sätt status-text baserat på kod
    switch(status) {
        case 200: res->status_text = strdup("OK"); break;
        case 400: res->status_text = strdup("Bad Request"); break;
        case 404: res->status_text = strdup("Not Found"); break;
        case 500: res->status_text = strdup("Internal Server Error"); break;
        default: res->status_text = strdup("Unknown"); break;
    }

    // Allokera minne för headers
    size_t headers_size = 512;
    res->headers = malloc(headers_size);

    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char time_buffer[64];
    strftime(time_buffer, sizeof(time_buffer), "%a, %d %b %Y %H:%M:%S %Z", tm_info);

    // Bygg headers
    snprintf(res->headers, headers_size,
             "Date: %s\r\n"
             "Server: VäderServer/1.0\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n",
             time_buffer, content_type, strlen(body));

    // Kopiera body
    res->body = strdup(body);

    return res;
}

// Parsar en HTTP-förfrågan (mer avancerad)
HTTP_Request* http_parse_request(const char* raw_request) {
    HTTP_Request* req = malloc(sizeof(HTTP_Request));
    memset(req, 0, sizeof(HTTP_Request));

    char* copy = strdup(raw_request);
    char* line = strtok(copy, "\r\n");

    // Parse första raden: METHOD PATH VERSION
    if (line) {
        char method[16], path[256], version[16];
        if (sscanf(line, "%15s %255s %15s", method, path, version) == 3) {
            req->method = strdup(method);
            req->path = strdup(path);
            req->version = strdup(version);
        }
    }

    // Parse headers
    size_t headers_size = 1024;
    req->headers = malloc(headers_size);
    req->headers[0] = '\0';

    while ((line = strtok(NULL, "\r\n")) && strlen(line) > 0) {
        strncat(req->headers, line, headers_size - strlen(req->headers) - 1);
        strncat(req->headers, "\r\n", headers_size - strlen(req->headers) - 1);
    }

    // Body finns efter en tom rad
    char* body_start = strstr(raw_request, "\r\n\r\n");
    if (body_start) {
        body_start += 4; // Skippa \r\n\r\n
        if (strlen(body_start) > 0) {
            req->body = strdup(body_start);
        }
    }

    free(copy);
    return req;
}

// Konverterar HTTP Response till sträng
char* http_response_to_string(HTTP_Response* res) {
    if (!res) return NULL;

    // Beräkna total storlek
    size_t total_size = 256 + strlen(res->headers) + strlen(res->body);
    char* response = malloc(total_size);

    // Bygg HTTP-responsen
    snprintf(response, total_size,
             "HTTP/1.1 %d %s\r\n"
             "%s"
             "\r\n"
             "%s",
             res->status_code,
             res->status_text,
             res->headers,
             res->body);

    return response;
}

// Frigör HTTP Response
void http_free_response(HTTP_Response** res_ptr) {
    if (!res_ptr || !*res_ptr) return;

    HTTP_Response* res = *res_ptr;

    if (res->status_text) free(res->status_text);
    if (res->headers) free(res->headers);
    if (res->body) free(res->body);

    free(res);
    *res_ptr = NULL;
}