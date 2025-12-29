// ============================================================================
// WEATHER CLIENT FOR ESP32
// ============================================================================
// ESP32-anpassad väderklient för mikrokontroller
// Använder ESP-IDF:s WiFi och socket API
// Optimerad för låg minnesanvändning och batteridrift
//
// KRAV:
// - ESP-IDF v4.0 eller senare
// - WiFi-anslutning
// - Konfigurerad SSID och lösenord
//
// MINNESANVÄNDNING:
// - Stack: ~2KB
// - Heap: ~4KB (för HTTP-buffertar)
// - Total: ~6KB (passar ESP32 med 520KB RAM)

#ifdef ESP32

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include <string.h>
#include <stdio.h>

// ============================================================================
// KONFIGURATIONSKONSTANTER
// ============================================================================
// WIFI-inställningar - ÄNDRA DESSA till ditt nätverk
#define WIFI_SSID      "DITT_WIFI_NAMN"
#define WIFI_PASSWORD  "DITT_WIFI_LOSENORD"

// Server-inställningar
#define SERVER_HOST    "192.168.1.100"  // ÄNDRA till din servers IP
#define SERVER_PORT    8080
#define DEFAULT_STAD   "Stockholm"
#define DEFAULT_LAND   "SE"

// Minnesbegränsningar för ESP32
#define HTTP_BUFFER_SIZE    2048  // Reducerat från 8192 (sparar 6KB RAM)
#define JSON_BUFFER_SIZE    1024  // Reducerat från 4096 (sparar 3KB RAM)
#define STRING_BUFFER_SIZE  64    // För små strängar

// WiFi-händelser
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

// Loggning
static const char *TAG = "VADER_KLIENT";

// ============================================================================
// GLOBALA VARIABLER
// ============================================================================
// Event group för WiFi-status (FreeRTOS-mekanism för synkronisering)
static EventGroupHandle_t s_wifi_event_group;

// Räknare för WiFi-återanslutningsförsök
static int s_retry_num = 0;
#define MAX_RETRY 5

// ============================================================================
// VÄDERDATA-STRUKTUR
// ============================================================================
/**
 * Kompakt struktur för väderdata
 * Designad för minimal minnesanvändning på ESP32
 * Total storlek: ~280 bytes
 */
typedef struct {
    char stad[STRING_BUFFER_SIZE];
    char land[STRING_BUFFER_SIZE];
    float temperatur;
    float luftfuktighet;
    float vindhastighet;
    float lufttryck;
    char beskrivning[128];
} VaderData;

// ============================================================================
// WIFI-HANTERING
// ============================================================================
/**
 * WiFi-händelsehanterare
 *
 * VARFÖR DETTA BEHÖVS:
 * ESP32 använder event-driven programmering för WiFi
 * Vi registrerar denna callback som anropas vid WiFi-händelser
 *
 * @param arg Användardata (ej använd här)
 * @param event_base Event-typ (WiFi eller IP)
 * @param event_id Specifik händelse
 * @param event_data Data associerad med händelsen
 */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        // WiFi startad, försök ansluta
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        // Tappade WiFi-anslutning
        if (s_retry_num < MAX_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Försöker återansluta till WiFi... (%d/%d)", s_retry_num, MAX_RETRY);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGE(TAG, "WiFi-anslutning misslyckades efter %d försök", MAX_RETRY);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        // Fick IP-adress från DHCP
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Fick IP-adress: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/**
 * Initialiserar och ansluter till WiFi
 *
 * FREERTOS-KONCEPT:
 * - Event groups används för att vänta på WiFi-anslutning
 * - Trådsäkert och låg strömförbrukning
 *
 * @return ESP_OK om lyckad anslutning, annars felkod
 */
static esp_err_t wifi_init_sta(void) {
    // Skapa event group för WiFi-status
    s_wifi_event_group = xEventGroupCreate();

    // Initialisera TCP/IP-stacken
    ESP_ERROR_CHECK(esp_netif_init());

    // Skapa default event loop (FreeRTOS event system)
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    // Initialisera WiFi med defaultkonfiguration
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Registrera event-hanterare för WiFi och IP-händelser
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    // Konfigurera WiFi med SSID och lösenord
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,  // WPA2-säkerhet
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));           // Station mode (klient)
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi-initialisering klar, ansluter till %s...", WIFI_SSID);

    // Vänta på WiFi-anslutning eller fel
    // xEventGroupWaitBits() blockar tills en av bitarna sätts
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);  // Vänta för evigt

    // Kolla vilket resultat vi fick
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "✓ Ansluten till WiFi: %s", WIFI_SSID);
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "✗ Kunde inte ansluta till WiFi: %s", WIFI_SSID);
        return ESP_FAIL;
    } else {
        ESP_LOGE(TAG, "✗ Oväntat WiFi-fel");
        return ESP_ERR_TIMEOUT;
    }
}

// ============================================================================
// JSON-PARSING FÖR ESP32
// ============================================================================
/**
 * Enkel JSON-parser optimerad för ESP32
 * Använder minimal minnesanvändning genom att jobba direkt på bufferten
 */
static float parse_json_float(const char* json, const char* nyckel) {
    char sokstrang[64];
    snprintf(sokstrang, sizeof(sokstrang), "\"%s\": ", nyckel);

    const char* position = strstr(json, sokstrang);
    if (!position) return 0.0f;

    position += strlen(sokstrang);
    return atof(position);
}

static void parse_json_string(const char* json, const char* nyckel,
                              char* resultat, size_t storlek) {
    char sokstrang[64];
    snprintf(sokstrang, sizeof(sokstrang), "\"%s\": \"", nyckel);

    const char* start = strstr(json, sokstrang);
    if (!start) {
        resultat[0] = '\0';
        return;
    }

    start += strlen(sokstrang);
    const char* slut = strchr(start, '"');
    if (!slut) {
        resultat[0] = '\0';
        return;
    }

    size_t langd = slut - start;
    if (langd >= storlek) langd = storlek - 1;

    strncpy(resultat, start, langd);
    resultat[langd] = '\0';
}

// ============================================================================
// HTTP-KLIENT FÖR ESP32
// ============================================================================
/**
 * Hämtar väderdata från servern via HTTP
 *
 * ESP32 LWIP SOCKET API:
 * - Använder POSIX-liknande socket API (kompatibelt med standard C)
 * - lwip = Lightweight IP stack (optimerad för embedded systems)
 *
 * @param stad Stad att hämta väder för
 * @param landskod Landskod (t.ex. "SE")
 * @param vader_data Pekare till struktur där resultatet ska sparas
 * @return ESP_OK om lyckat, annars felkod
 */
static esp_err_t hamta_vader_data(const char* stad, const char* landskod,
                                  VaderData* vader_data) {
    int sock = -1;
    struct sockaddr_in server_addr;
    char http_request[512];
    char http_response[HTTP_BUFFER_SIZE];

    ESP_LOGI(TAG, "Hämtar väderdata för %s, %s...", stad, landskod);

    // Skapa TCP-socket
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Kunde inte skapa socket: errno %d", errno);
        return ESP_FAIL;
    }

    // Konfigurera server-adress
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_HOST, &server_addr.sin_addr);

    // Anslut till servern
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        ESP_LOGE(TAG, "Socket connect misslyckades: errno %d", errno);
        close(sock);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "✓ Ansluten till server %s:%d", SERVER_HOST, SERVER_PORT);

    // Bygg HTTP GET-request
    snprintf(http_request, sizeof(http_request),
             "GET /weather?city=%s&country=%s HTTP/1.1\r\n"
             "Host: %s:%d\r\n"
             "Connection: close\r\n\r\n",
             stad, landskod, SERVER_HOST, SERVER_PORT);

    // Skicka request
    if (send(sock, http_request, strlen(http_request), 0) < 0) {
        ESP_LOGE(TAG, "Send misslyckades: errno %d", errno);
        close(sock);
        return ESP_FAIL;
    }

    // Ta emot svar
    int total_mottaget = 0;
    int mottaget;
    while ((mottaget = recv(sock, http_response + total_mottaget,
                           sizeof(http_response) - total_mottaget - 1, 0)) > 0) {
        total_mottaget += mottaget;
    }

    http_response[total_mottaget] = '\0';
    close(sock);

    // Extrahera JSON-body från HTTP-svar
    char* json_start = strstr(http_response, "\r\n\r\n");
    if (!json_start) {
        ESP_LOGE(TAG, "Kunde inte hitta JSON i HTTP-svar");
        return ESP_FAIL;
    }
    json_start += 4;  // Hoppa över \r\n\r\n

    // Parsa JSON-data
    parse_json_string(json_start, "stad", vader_data->stad, sizeof(vader_data->stad));
    parse_json_string(json_start, "land", vader_data->land, sizeof(vader_data->land));
    vader_data->temperatur = parse_json_float(json_start, "temperatur");
    vader_data->luftfuktighet = parse_json_float(json_start, "luftfuktighet");
    vader_data->vindhastighet = parse_json_float(json_start, "vindhastighet");
    vader_data->lufttryck = parse_json_float(json_start, "lufttryck");
    parse_json_string(json_start, "beskrivning", vader_data->beskrivning,
                     sizeof(vader_data->beskrivning));

    ESP_LOGI(TAG, "✓ Väderdata mottagen och parsad");
    return ESP_OK;
}

// ============================================================================
// DISPLAY-FUNKTION
// ============================================================================
/**
 * Visar väderdata i terminalen
 * Använder ESP_LOGI för loggning (skrivs ut på seriell konsol)
 */
static void visa_vader_data(const VaderData* vader) {
    printf("\n╔═══════════════════════════════════════════════════════╗\n");
    printf("║            VÄDERRAPPORT - ESP32 KLIENT               ║\n");
    printf("╚═══════════════════════════════════════════════════════╝\n\n");

    printf("📍 Stad:          %s, %s\n", vader->stad, vader->land);
    printf("🌡️  Temperatur:   %.1f°C\n", vader->temperatur);
    printf("💧 Luftfuktighet: %.0f%%\n", vader->luftfuktighet);
    printf("💨 Vindhastighet: %.1f m/s\n", vader->vindhastighet);
    printf("🔽 Lufttryck:     %.0f hPa\n", vader->lufttryck);
    printf("☁️  Beskrivning:  %s\n\n", vader->beskrivning);
}

// ============================================================================
// HUVUDUPPGIFT (FREERTOS TASK)
// ============================================================================
/**
 * FreeRTOS-uppgift som hämtar och visar väderdata
 *
 * FREERTOS-KONCEPT:
 * - Uppgifter (tasks) är som trådar men lättare
 * - vTaskDelay() släpper CPU:n för att spara ström
 * - Perfekt för periodisk datahämtning
 */
static void vader_task(void *pvParameters) {
    VaderData vader_data;

    while (1) {
        // Hämta väderdata
        if (hamta_vader_data(DEFAULT_STAD, DEFAULT_LAND, &vader_data) == ESP_OK) {
            visa_vader_data(&vader_data);
        } else {
            ESP_LOGE(TAG, "Misslyckades att hämta väderdata");
        }

        // Vänta 10 minuter innan nästa hämtning (sparar ström och API-anrop)
        // vTaskDelay() tar tick count, portTICK_PERIOD_MS = ms per tick
        vTaskDelay(10 * 60 * 1000 / portTICK_PERIOD_MS);
    }
}

// ============================================================================
// HUVUDFUNKTION (APP_MAIN)
// ============================================================================
/**
 * ESP32:s startpunkt (motsvarar main() för vanlig C)
 *
 * INITIALISERINGSORDNING:
 * 1. NVS (Non-Volatile Storage) - för WiFi-konfiguration
 * 2. WiFi - anslut till nätverk
 * 3. Starta väder-uppgift
 */
void app_main(void) {
    printf("\n╔═══════════════════════════════════════════════════════╗\n");
    printf("║          ESP32 VÄDERKLIENT - VERSION 1.0.0           ║\n");
    printf("╚═══════════════════════════════════════════════════════╝\n\n");

    // Initialisera NVS (krävs för WiFi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Anslut till WiFi
    if (wifi_init_sta() != ESP_OK) {
        ESP_LOGE(TAG, "WiFi-initialisering misslyckades!");
        return;
    }

    // Skapa och starta väder-uppgift
    // Stack: 4096 bytes, Prioritet: 5, Ingen handle behövs
    xTaskCreate(vader_task, "vader_task", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "ESP32 väderklient startad!");
}

#else
// Dummy main för icke-ESP32 plattformar
#include <stdio.h>
int main(void) {
    printf("Denna kod är endast för ESP32-plattformen.\n");
    printf("Kompilera med ESP-IDF för att köra på ESP32.\n");
    return 1;
}
#endif
