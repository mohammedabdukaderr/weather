# Väderklient - C++ Implementation

Standalone väderapplikation med interaktiv meny för 60+ städer.

## 🚀 Snabbstart

```bash
# Bygg och kör (ett kommando!)
make run
```

Det är allt! Nu kan du välja stad från menyn.

## 📋 Användning

### Interaktiv Meny (Rekommenderat)

```bash
make run
```

Välj från:
- **60 svenska städer** (Stockholm, Göteborg, Malmö, Kiruna...)
- **10 internationella** (London, Paris, New York, Tokyo...)
- **Egen stad** (skriv egen stad + landskod)

### Direkt Användning

```bash
# Bygg först
make

# Kör med specifik stad
./weather_client_cpp Stockholm SE
./weather_client_cpp London GB
./weather_client_cpp "New York" US
```

## 🛠️ Byggkommandon

| Kommando | Beskrivning |
|----------|-------------|
| `make` | Bygg klienten |
| `make run` | Kör interaktiv meny |
| `make test` | Testa med Stockholm |
| `make clean` | Rensa byggfiler |
| `make help` | Visa hjälp |

## 📖 Exempel

### Exempel 1: Interaktiv meny
```bash
cd client
make run

# I menyn:
# Tryck 1 för Stockholm
# Tryck 70 för London
# Tryck 0 för egen stad
```

### Exempel 2: Direkt kommando
```bash
make
./weather_client_cpp Paris FR
```

Output:
```
╔═══════════════════════════════════════════════════════╗
║              VÄDERRAPPORT - C++ KLIENT               ║
╚═══════════════════════════════════════════════════════╝

📍 Stad:          Paris, FR
🌡️  Temperatur:   18.5°C
💧 Luftfuktighet: 72%
💨 Vindhastighet: 4.1 m/s
🔽 Lufttryck:     1015 hPa
☁️  Beskrivning:  Partly cloudy
```

## 🌍 Tillgängliga Städer

### Svenska Storstäder
Stockholm, Göteborg, Malmö, Uppsala, Västerås, Örebro, Linköping, Helsingborg, Jönköping, Norrköping, Lund, Umeå, Gävle, Borås, Eskilstuna, Södertälje, Karlstad, Täby, Växjö, Halmstad

### Svenska Mellanstäder
Sundsvall, Luleå, Trollhättan, Kristianstad, Kalmar, Skövde, Karlskrona, Mölndal, Varberg, Åkersberga, Lidingö, Östersund, Borlänge, Tumba, Falun, Skellefteå, Uddevalla, Motala, Landskrona, Örnsköldsvik

### Övriga Svenska Städer
Trelleborg, Ystad, Karlskoga, Piteå, Sandviken, Nyköping, Katrineholm, Värnamo, Enköping, Sollefteå, Kiruna, Visby, Ängelholm, Lerum, Vänersborg, Kungsbacka, Alingsås, Västervik, Kungälv, Hudiksvall

### Internationella Städer
London (GB), Paris (FR), Berlin (DE), New York (US), Tokyo (JP), Dubai (AE), Oslo (NO), Köpenhamn (DK), Helsinki (FI), Reykjavik (IS)

## 🔧 Teknisk Information

- **Språk**: C++11
- **Kompilator**: g++ 7.0+
- **Plattformar**: Windows, Linux, macOS
- **Dependencies**: Endast standardbibliotek

## 📁 Filer

```
client/
├── Makefile              # Build-konfiguration
├── README.md             # Detta dokument
├── weather_client.cpp    # C++ källkod
└── weather_client.c      # C-version (alternativ)
```

## 🐛 Felsökning

### Problem: "g++ command not found"
```bash
# Linux/WSL
sudo apt-get install g++ build-essential

# macOS
brew install gcc
```

### Problem: Kompileringsfel
```bash
# Rensa och bygg om
make clean
make
```

### Problem: "Cannot connect to server"
Klienten kräver att servern körs. Starta servern:
```bash
cd ..
make run
```

## 📚 Mer Information

Se huvudprojektets README för:
- Server-installation
- API-dokumentation
- Fullständig systemarkitektur

## 👨‍💻 Utvecklare

**Mohammed Abdukader**
Chas Academy Student

- GitHub: [@mohammedabdukaderr](https://github.com/mohammedabdukaderr)
- Email: mohammed.abdukader@chasacademy.se
