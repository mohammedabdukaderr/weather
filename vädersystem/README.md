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

**⚠️ Security Note:** Never commit your API key to Git!

**Recommended: Use environment variables**
```bash
# Windows
set WEATHER_API_KEY=your_api_key_here
./weather_server.exe %WEATHER_API_KEY%

# Linux/Mac
export WEATHER_API_KEY=your_api_key_here
./weather_server $WEATHER_API_KEY
```

### 2. Start Server

```bash
./weather_server.exe YOUR_API_KEY
```

Server will listen on `http://localhost:8080`

**Error Handling:**
- Server logs errors to `vaderserver.log`
- Invalid API key → Check OpenWeatherMap dashboard
- Port already in use → Change port in `include/konfiguration.h`
- Connection errors → Check firewall settings

### 3. Test with Browser

```
http://localhost:8080/weather?city=Stockholm&country=SE
```

Expected response:
- Status 200 + JSON weather data = Success ✅
- Status 401 = Invalid API key ❌
- Status 404 = City not found ❌
- Status 500 = Server error (check logs) ❌

### 4. Run Client

```bash
./weather_client.exe localhost 8080
```

The client will display:
- Current temperature
- Humidity
- Wind speed
- Air pressure
- Weather description
- Cached data indicator

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

## Troubleshooting

### Common Issues

**1. "gcc: command not found"**
- Install MinGW (Windows) or build-essential (Linux)
- Add to PATH environment variable

**2. "Address already in use"**
```bash
# Find process using port 8080
netstat -ano | findstr :8080

# Kill the process (Windows)
taskkill /PID <PID> /F

# Kill the process (Linux)
kill -9 <PID>
```

**3. "Invalid API key" (HTTP 401)**
- Wait 2 hours after registration (API key activation time)
- Verify key at https://home.openweathermap.org/api_keys
- Check key is active

**4. "City not found" (HTTP 404)**
- Use English city names
- Include country code: `?city=Stockholm&country=SE`
- Check spelling

**5. Compilation errors**
- Ensure all source files in `src/` and headers in `include/`
- Check C11 support: `gcc --version`
- Clean and rebuild: `make clean && make`

### Debug Mode

Enable detailed logging:
```bash
./weather_server.exe YOUR_API_KEY 8080 0
```

Check logs:
```bash
# Windows
type vaderserver.log

# Linux
cat vaderserver.log
```

## License

This project was developed as part of Chas Academy coursework.

## Author

Mohammed Abdukader - Chas Academy Student

**Repository:** https://github.com/mohammedabdukaderr/weather
