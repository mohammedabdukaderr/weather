#!/bin/bash
# ============================================================================
# TESTSCRIPT - Kör alla tester
# ============================================================================
# Användning: ./tests/run_all_tests.sh

set -e  # Avsluta vid första fel

echo "╔═══════════════════════════════════════════════════════╗"
echo "║          VÄDERSYSTEM - TESTKÖRNING                   ║"
echo "╚═══════════════════════════════════════════════════════╝"
echo ""

# Färger för output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Räknare
TOTAL_TESTS=0
PASSED_TESTS=0

# ============================================================================
# ENHETSTESTER
# ============================================================================
echo "🧪 ENHETSTESTER"
echo "────────────────────────────────────────────────────────"
echo ""

# Test 1: JSON Helper
echo "  [1/2] Kompilerar test_json..."
gcc -Wall -Wextra -I../include tests/test_json.c -o tests/test_json 2>&1 || {
    echo -e "${RED}✗ Kompilering misslyckades${NC}"
    exit 1
}

echo "  [1/2] Kör test_json..."
if ./tests/test_json; then
    echo -e "${GREEN}✓ JSON-tester godkända${NC}\n"
    ((PASSED_TESTS++))
else
    echo -e "${RED}✗ JSON-tester misslyckades${NC}\n"
fi
((TOTAL_TESTS++))

# Test 2: HTTP Server
echo "  [2/2] Kompilerar test_http..."
gcc -Wall -Wextra -I../include tests/test_http.c -o tests/test_http 2>&1 || {
    echo -e "${RED}✗ Kompilering misslyckades${NC}"
    exit 1
}

echo "  [2/2] Kör test_http..."
if ./tests/test_http; then
    echo -e "${GREEN}✓ HTTP-tester godkända${NC}\n"
    ((PASSED_TESTS++))
else
    echo -e "${RED}✗ HTTP-tester misslyckades${NC}\n"
fi
((TOTAL_TESTS++))

# ============================================================================
# INTEGRATIONSTESTER
# ============================================================================
echo ""
echo "🔗 INTEGRATIONSTESTER"
echo "────────────────────────────────────────────────────────"
echo ""

# Bygg servern först
echo "  Bygger servern..."
make clean > /dev/null 2>&1
make > /dev/null 2>&1 || {
    echo -e "${RED}✗ Server-kompilering misslyckades${NC}"
    exit 1
}

# Starta server i bakgrunden
echo "  Startar testserver på port 8081..."
API_KEY="test_api_key_12345"
./weather_server $API_KEY 8081 3 > /dev/null 2>&1 &
SERVER_PID=$!
sleep 2  # Vänta på att servern startar

# Test 3: Root endpoint
echo "  [1/4] Testar GET /..."
RESPONSE=$(curl -s http://localhost:8081/ 2>&1)
if echo "$RESPONSE" | grep -q "Vädersystem API"; then
    echo -e "${GREEN}✓ Root endpoint fungerar${NC}"
    ((PASSED_TESTS++))
else
    echo -e "${RED}✗ Root endpoint misslyckades${NC}"
fi
((TOTAL_TESTS++))

# Test 4: Weather endpoint (utan API kommer ge fel, men endpoint ska svara)
echo "  [2/4] Testar GET /weather..."
HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" "http://localhost:8081/weather?city=Stockholm&country=SE" 2>&1)
if [ "$HTTP_CODE" = "200" ] || [ "$HTTP_CODE" = "500" ]; then
    echo -e "${GREEN}✓ Weather endpoint svarar${NC}"
    ((PASSED_TESTS++))
else
    echo -e "${RED}✗ Weather endpoint svarar inte (kod: $HTTP_CODE)${NC}"
fi
((TOTAL_TESTS++))

# Test 5: Forecast endpoint
echo "  [3/4] Testar GET /forecast..."
HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" "http://localhost:8081/forecast?city=Stockholm&country=SE" 2>&1)
if [ "$HTTP_CODE" = "200" ] || [ "$HTTP_CODE" = "500" ]; then
    echo -e "${GREEN}✓ Forecast endpoint svarar${NC}"
    ((PASSED_TESTS++))
else
    echo -e "${RED}✗ Forecast endpoint svarar inte (kod: $HTTP_CODE)${NC}"
fi
((TOTAL_TESTS++))

# Test 6: 404 för ogiltig endpoint
echo "  [4/4] Testar 404-hantering..."
HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" "http://localhost:8081/invalid" 2>&1)
if [ "$HTTP_CODE" = "404" ]; then
    echo -e "${GREEN}✓ 404-hantering fungerar${NC}"
    ((PASSED_TESTS++))
else
    echo -e "${RED}✗ 404-hantering misslyckades (kod: $HTTP_CODE)${NC}"
fi
((TOTAL_TESTS++))

# Stoppa servern
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null

# ============================================================================
# SAMMANFATTNING
# ============================================================================
echo ""
echo "╔═══════════════════════════════════════════════════════╗"
echo "║                   TESTSAMMANFATTNING                  ║"
echo "╚═══════════════════════════════════════════════════════╝"
echo ""
echo "  Totalt antal tester:   $TOTAL_TESTS"
echo "  Godkända tester:       $PASSED_TESTS"
echo "  Misslyckade tester:    $((TOTAL_TESTS - PASSED_TESTS))"
echo ""

if [ $PASSED_TESTS -eq $TOTAL_TESTS ]; then
    echo -e "${GREEN}✓ ALLA TESTER GODKÄNDA!${NC}"
    echo ""
    exit 0
else
    echo -e "${RED}✗ $((TOTAL_TESTS - PASSED_TESTS)) TESTER MISSLYCKADES${NC}"
    echo ""
    exit 1
fi
