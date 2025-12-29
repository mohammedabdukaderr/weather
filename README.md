# Mohammed Abdukader - Chas Academy Projekt

Detta repository innehåller skolprojekt från Chas Academy.

## 🌤️ Huvudprojekt: Vädersystem

**📁 Gå till:** [`vädersystem/`](vädersystem/)

Komplett C-baserad väderapplikation med HTTP/JSON server och tre klienter.

### Snabbstart

```bash
cd vädersystem
make all
make run
```

### Dokumentation

- [README](vädersystem/README.md) - Installation och användning
- [ARKITEKTUR](vädersystem/ARKITEKTUR.md) - Systemarkitektur
- [UTVECKLING](vädersystem/UTVECKLING.md) - Utvecklingsguide
- [TESTRAPPORT](vädersystem/TESTRAPPORT.md) - Testresultat

## Projektstruktur

```
mohammed/
├── vädersystem/          ← HUVUDPROJEKT
│   ├── src/             # Server-kod (7 moduler)
│   ├── client/          # 3 klientapplikationer
│   ├── tests/           # Testsuite (26 tester)
│   ├── include/         # Header-filer
│   ├── Makefile         # Build-system
│   └── *.md             # Dokumentation
│
└── README.md            # Detta dokument
```

## Teknologi

- **Språk**: C11, C++11
- **Plattformar**: Windows, Linux, macOS, ESP32
- **Protokoll**: HTTP/1.1, TCP/IP, JSON
- **API**: OpenWeatherMap REST API

## Kontakt

**Mohammed Abdukader**
Chas Academy Student

- Email: mohammed.abdukader@chasacademy.se
- GitHub: [@mohammedabdukaderr](https://github.com/mohammedabdukaderr)
- Repository: https://github.com/mohammedabdukaderr/weather
