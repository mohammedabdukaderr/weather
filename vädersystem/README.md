# 🌤️ Weather System

A C-based weather application with HTTP/JSON server and client implementation. Fetches real-time weather data from OpenWeatherMap API with built-in caching.

## Features

- HTTP/JSON server with TCP socket handling
- OpenWeatherMap API integration
- 30-minute file-based cache system
- C client with terminal UI
- Cross-platform support (Windows/Linux)
- Swedish variable names and comments

## Project Structure

```
vädersystem/
├── src/              # Server source files
│   ├── main.c       # Main server with HTTP routing
│   ├── loggning.c   # Logging system
│   ├── http_server.c
│   ├── tcp_server.c
│   ├── cache.c
│   ├── json_helper.c
│   └── vader_api.c
├── include/          # Header files
├── client/           # Client application
└── Makefile
```

## Prerequisites

**Windows:**
- MinGW (GCC for Windows)
- Make

**Linux:**
```bash
sudo apt-get install build-essential gcc make
```

## Building

```bash
# Build server and client
make

# Build server only
make server

# Build client only
make client

# Clean build files
make clean
```

## Running

### 1. Get API Key

Sign up at [OpenWeatherMap](https://openweathermap.org/api) and get a free API key.

### 2. Start Server

```bash
./weather_server.exe YOUR_API_KEY
```

Server will listen on `http://localhost:8080`

### 3. Test with Browser

```
http://localhost:8080/weather?city=Stockholm&country=SE
```

### 4. Run Client

```bash
./weather_client.exe localhost 8080
```

## API Endpoints

**Get current weather:**
```
GET /weather?city=CITY&country=COUNTRY_CODE
```

**Get forecast:**
```
GET /forecast?city=CITY&country=COUNTRY_CODE
```

**Response format (JSON):**
```json
{
  "stad": "Stockholm",
  "temperatur": 15.5,
  "luftfuktighet": 65,
  "vindhastighet": 3.2,
  "lufttryck": 1013,
  "beskrivning": "molnigt",
  "ikon_id": "04d",
  "tidsstampel": 1735145678
}
```

## Configuration

Edit `include/konfiguration.h`:

```c
#define SERVER_PORT 8080              // TCP port
#define MAX_KLIENTER 32               // Max simultaneous clients
#define CACHE_GILTIGHETSTID 1800      // Cache TTL (seconds)
```

## Cache System

- Data cached for 30 minutes
- Automatic cleanup of expired cache
- Reduces API calls and improves response time

## Development

**Logging levels:**
```bash
./weather_server.exe API_KEY 8080 0  # DEBUG
./weather_server.exe API_KEY 8080 1  # INFO (default)
./weather_server.exe API_KEY 8080 2  # WARNING
./weather_server.exe API_KEY 8080 3  # ERROR
```

## License

This project was developed as part of Chas Academy coursework.

## Author

Mohammed Abdukader - Chas Academy Student

**Repository:** https://github.com/mohammedabdukaderr/weather
