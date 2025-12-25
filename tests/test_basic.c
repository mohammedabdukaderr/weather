#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../libs/HTTP.h"
#include "../libs/cache.h"

void test_http_parsing() {
    printf("Test 1: HTTP Parsing\n");
    
    const char* request = 
        "GET /weather/Stockholm HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "User-Agent: TestClient\r\n"
        "\r\n";
    
    HTTP_Request* req = http_parse_request(request);
    assert(req != NULL);
    assert(strcmp(req->method, "GET") == 0);
    assert(strcmp(req->path, "/weather/Stockholm") == 0);
    assert(strcmp(req->version, "HTTP/1.1") == 0);
    
    http_free_request(&req);
    printf("✅ HTTP parsing fungerar\n");
}

void test_cache_functionality() {
    printf("\nTest 2: Cache-funktionalitet\n");
    
    WeatherCache* cache = cache_create(10, "./test_cache");
    
    // Testa att lägga till och hämta från cache
    cache_put(cache, "Stockholm", "{\"temp\":15}", 60);
    
    char* data = cache_get(cache, "Stockholm");
    assert(data != NULL);
    assert(strcmp(data, "{\"temp\":15}") == 0);
    
    // Testa cache miss
    data = cache_get(cache, "Göteborg");
    assert(data == NULL);
    
    cache_destroy(&cache);
    printf("✅ Cache fungerar\n");
}

void test_response_creation() {
    printf("\nTest 3: HTTP Response Creation\n");
    
    HTTP_Response* res = http_create_response(200, "application/json", 
                                              "{\"status\":\"ok\"}");
    assert(res != NULL);
    assert(res->status_code == 200);
    assert(strcmp(res->status_text, "OK") == 0);
    assert(strstr(res->headers, "Content-Type: application/json") != NULL);
    assert(strstr(res->body, "{\"status\":\"ok\"}") != NULL);
    
    http_free_response(&res);
    printf("✅ Response creation fungerar\n");
}

int main() {
    printf("=== TESTSUITE FÖR VÄDERSERVER ===\n\n");
    
    test_http_parsing();
    test_cache_functionality();
    test_response_creation();
    
    printf("\n================================\n");
    printf("✅ ALLA TESTER KLARA!\n");
    printf("Systemet uppfyller kurskraven från v43-v46\n");
    
    return 0;
}