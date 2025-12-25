#ifndef HTTP_H
#define HTTP_H

#include <stdlib.h>

// Definiera struct OCH typedef på samma gång
typedef struct HTTP_Request {
    char* method;
    char* path;
    char* version;
    char* headers;
    char* body;
} HTTP_Request;

// Samma för Response
typedef struct HTTP_Response {
    int status_code;
    char* status_text;
    char* headers;
    char* body;
} HTTP_Response;

// Request-funktioner
HTTP_Request* http_parse(const char* raw_request);
HTTP_Request* http_parse_request(const char* raw_request);
void http_free_request(HTTP_Request** req);

// Response-funktioner
HTTP_Response* http_create_response(int status, const char* content_type, const char* body);
char* http_response_to_string(HTTP_Response* res);
void http_free_response(HTTP_Response** res);

#endif
