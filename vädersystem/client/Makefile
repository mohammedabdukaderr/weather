# Makefile för Väderklient
# ============================================================================
# Byggkommandon för C++ väderklieneten
# Användning:
#   make           - Bygg C++-klienten (default)
#   make run       - Kör interaktiv vädermeny
#   make client    - Bygg C++-klienten
#   make test      - Testa klient med Stockholm
#   make clean     - Rensa byggfiler
#   make help      - Visa hjälp
# ============================================================================

# Kompilatorkonfiguration
CXX = g++
CXXFLAGS = -Wall -Wextra -I../include -std=c++11 -g

# Plattformsspecifika inställningar
ifeq ($(OS),Windows_NT)
    LDFLAGS = -lws2_32
    CLIENT_CPP = weather_client_cpp.exe
    VADER_SCRIPT = vader.sh
    RM = del /Q
else
    LDFLAGS =
    CLIENT_CPP = weather_client_cpp
    VADER_SCRIPT = vader.sh
    RM = rm -f
endif

# Källfiler
CLIENT_CPP_SRC = weather_client.cpp
CLIENT_CPP_OBJ = weather_client.o

# ============================================================================
# PHONY TARGETS
# ============================================================================
.PHONY: all client run test clean help

# ============================================================================
# HUVUDMÅL
# ============================================================================

# Default: bygg klient
all: client

# Bygg C++-klient
client: $(CLIENT_CPP)
	@echo ""
	@echo "✅ C++-klient byggd: $(CLIENT_CPP)"
	@echo ""
	@echo "💡 Kräver att servern körs på localhost:8080"
	@echo "💡 Starta server: cd .. && make run"
	@echo ""
	@echo "Kör 'make run' för interaktiv meny"
	@echo "Kör './$(CLIENT_CPP) Stockholm SE' för direkt användning"
	@echo ""

# ============================================================================
# BYGGREGLER
# ============================================================================

# Länka C++-klient
$(CLIENT_CPP): $(CLIENT_CPP_OBJ)
	@echo "Länkar C++-klient..."
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Kompilera C++-fil
weather_client.o: weather_client.cpp
	@echo "Kompilerar $<..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# ============================================================================
# KÖRNING
# ============================================================================

# Kör interaktiv vädermeny (HUVUDKOMMANDO)
run: client
	@echo ""
	@echo "╔═══════════════════════════════════════════════════════╗"
	@echo "║          STARTAR INTERAKTIV VÄDERKLIENT              ║"
	@echo "╚═══════════════════════════════════════════════════════╝"
	@echo ""
	@echo "💡 Ansluter till server på localhost:8080"
	@echo "💡 Servern hämtar data från OpenWeatherMap API"
	@echo "💡 Skriv stad-namnet för att få vädret!"
	@echo "💡 Exempel: 'Luleå' eller 'London GB'"
	@echo ""
	@echo "⚠️  OBS: Kontrollera att servern körs först!"
	@echo "   Starta server: cd .. && make run"
	@echo ""
	@./$(CLIENT_CPP)

# Testa klient med Stockholm
test: client
	@echo ""
	@echo "╔═══════════════════════════════════════════════════════╗"
	@echo "║          TESTAR VÄDERKLIENT                          ║"
	@echo "╚═══════════════════════════════════════════════════════╝"
	@echo ""
	@echo "📍 Hämtar väder för Stockholm, SE..."
	@echo ""
	@./$(CLIENT_CPP) Stockholm SE

# ============================================================================
# RENSNING
# ============================================================================

clean:
	@echo "🧹 Rensar byggfiler..."
	$(RM) $(CLIENT_CPP_OBJ) $(CLIENT_CPP)
	@echo "✅ Rensning klar"

# ============================================================================
# HJÄLP
# ============================================================================

help:
	@echo ""
	@echo "╔═══════════════════════════════════════════════════════╗"
	@echo "║          VÄDERKLIENT MAKEFILE - HJÄLP                ║"
	@echo "╚═══════════════════════════════════════════════════════╝"
	@echo ""
	@echo "BYGGKOMMANDON:"
	@echo "  make              - Bygg C++-klienten (default)"
	@echo "  make client       - Bygg C++-klienten"
	@echo ""
	@echo "KÖRNING:"
	@echo "  make run          - Kör interaktiv vädermeny (60+ städer)"
	@echo "  make test         - Testa klient med Stockholm"
	@echo ""
	@echo "RENSNING:"
	@echo "  make clean        - Rensa byggfiler"
	@echo ""
	@echo "MANUELL ANVÄNDNING:"
	@echo "  ./$(CLIENT_CPP) [stad] [landskod]"
	@echo ""
	@echo "EXEMPEL:"
	@echo "  make run                           # Interaktiv meny"
	@echo "  ./$(CLIENT_CPP) London GB          # London"
	@echo "  ./$(CLIENT_CPP) Paris FR           # Paris"
	@echo "  ./$(CLIENT_CPP) \"New York\" US      # New York"
	@echo ""
	@echo "TILLGÄNGLIGA STÄDER I MENYN:"
	@echo "  - 60 svenska städer (Stockholm, Göteborg, Malmö...)"
	@echo "  - 10 internationella (London, Paris, Berlin...)"
	@echo "  - Egen stad (skriv egen stad + landskod)"
	@echo ""
