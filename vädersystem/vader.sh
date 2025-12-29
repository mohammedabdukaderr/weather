#!/bin/bash
# ============================================================================
# INTERAKTIV VÄDERKLIENT
# ============================================================================
# Enkelt script för att hämta väder utan att behöva skriva långa kommandon

clear
echo "╔═══════════════════════════════════════════════════════╗"
echo "║          VÄLKOMMEN TILL VÄDERSYSTEMET                 ║"
echo "╚═══════════════════════════════════════════════════════╝"
echo ""

# Funktion för att visa väder
show_weather() {
    local city=$1
    local country=$2
    ./client/weather_client_cpp "$city" "$country"
    echo ""
}

while true; do
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "  VÄLJ STAD (eller skriv egen stad/landskod)"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo ""
    echo "  STORSTÄDER (100 000+ invånare):"
    echo "  1) Stockholm          11) Lund"
    echo "  2) Göteborg           12) Umeå"
    echo "  3) Malmö              13) Gävle"
    echo "  4) Uppsala            14) Borås"
    echo "  5) Västerås           15) Eskilstuna"
    echo "  6) Örebro             16) Södertälje"
    echo "  7) Linköping          17) Karlstad"
    echo "  8) Helsingborg        18) Täby"
    echo "  9) Jönköping          19) Växjö"
    echo "  10) Norrköping        20) Halmstad"
    echo ""
    echo "  MELLANSTORA STÄDER (50 000-100 000):"
    echo "  21) Sundsvall         31) Lidingö"
    echo "  22) Luleå             32) Östersund"
    echo "  23) Trollhättan       33) Borlänge"
    echo "  24) Kristianstad      34) Tumba"
    echo "  25) Kalmar            35) Falun"
    echo "  26) Skövde            36) Skellefteå"
    echo "  27) Karlskrona        37) Uddevalla"
    echo "  28) Mölndal           38) Motala"
    echo "  29) Varberg           39) Landskrona"
    echo "  30) Åkersberga        40) Örnsköldsvik"
    echo ""
    echo "  ÖVRIGA SVENSKA STÄDER:"
    echo "  41) Trelleborg        51) Kiruna"
    echo "  42) Ystad             52) Visby"
    echo "  43) Karlskoga         53) Ängelholm"
    echo "  44) Piteå             54) Lerum"
    echo "  45) Sandviken         55) Vänersborg"
    echo "  46) Nyköping          56) Kungsbacka"
    echo "  47) Katrineholm       57) Alingsås"
    echo "  48) Värnamo           58) Västervik"
    echo "  49) Enköping          59) Kungälv"
    echo "  50) Sollefteå         60) Hudiksvall"
    echo ""
    echo "  INTERNATIONELLA STÄDER:"
    echo "  70) London (GB)       75) Dubai (AE)"
    echo "  71) Paris (FR)        76) Oslo (NO)"
    echo "  72) Berlin (DE)       77) Köpenhamn (DK)"
    echo "  73) New York (US)     78) Helsinki (FI)"
    echo "  74) Tokyo (JP)        79) Reykjavik (IS)"
    echo ""
    echo "  EGEN STAD:"
    echo "  0) Skriv egen stad och landskod"
    echo ""
    echo "  ÖVRIGT:"
    echo "  q) Avsluta"
    echo ""
    read -p "  Ditt val: " val

    case $val in
        # Storstäder
        1)  show_weather "Stockholm" "SE" ;;
        2)  show_weather "Gothenburg" "SE" ;;
        3)  show_weather "Malmo" "SE" ;;
        4)  show_weather "Uppsala" "SE" ;;
        5)  show_weather "Vasteras" "SE" ;;
        6)  show_weather "Orebro" "SE" ;;
        7)  show_weather "Linkoping" "SE" ;;
        8)  show_weather "Helsingborg" "SE" ;;
        9)  show_weather "Jonkoping" "SE" ;;
        10) show_weather "Norrkoping" "SE" ;;
        11) show_weather "Lund" "SE" ;;
        12) show_weather "Umea" "SE" ;;
        13) show_weather "Gavle" "SE" ;;
        14) show_weather "Boras" "SE" ;;
        15) show_weather "Eskilstuna" "SE" ;;
        16) show_weather "Sodertalje" "SE" ;;
        17) show_weather "Karlstad" "SE" ;;
        18) show_weather "Taby" "SE" ;;
        19) show_weather "Vaxjo" "SE" ;;
        20) show_weather "Halmstad" "SE" ;;

        # Mellanstora städer
        21) show_weather "Sundsvall" "SE" ;;
        22) show_weather "Lulea" "SE" ;;
        23) show_weather "Trollhattan" "SE" ;;
        24) show_weather "Kristianstad" "SE" ;;
        25) show_weather "Kalmar" "SE" ;;
        26) show_weather "Skovde" "SE" ;;
        27) show_weather "Karlskrona" "SE" ;;
        28) show_weather "Molndal" "SE" ;;
        29) show_weather "Varberg" "SE" ;;
        30) show_weather "Akersberga" "SE" ;;
        31) show_weather "Lidingo" "SE" ;;
        32) show_weather "Ostersund" "SE" ;;
        33) show_weather "Borlange" "SE" ;;
        34) show_weather "Tumba" "SE" ;;
        35) show_weather "Falun" "SE" ;;
        36) show_weather "Skelleftea" "SE" ;;
        37) show_weather "Uddevalla" "SE" ;;
        38) show_weather "Motala" "SE" ;;
        39) show_weather "Landskrona" "SE" ;;
        40) show_weather "Ornskoldsvik" "SE" ;;

        # Övriga svenska städer
        41) show_weather "Trelleborg" "SE" ;;
        42) show_weather "Ystad" "SE" ;;
        43) show_weather "Karlskoga" "SE" ;;
        44) show_weather "Pitea" "SE" ;;
        45) show_weather "Sandviken" "SE" ;;
        46) show_weather "Nykoping" "SE" ;;
        47) show_weather "Katrineholm" "SE" ;;
        48) show_weather "Varnamo" "SE" ;;
        49) show_weather "Enkoping" "SE" ;;
        50) show_weather "Solleftea" "SE" ;;
        51) show_weather "Kiruna" "SE" ;;
        52) show_weather "Visby" "SE" ;;
        53) show_weather "Angelholm" "SE" ;;
        54) show_weather "Lerum" "SE" ;;
        55) show_weather "Vanersborg" "SE" ;;
        56) show_weather "Kungsbacka" "SE" ;;
        57) show_weather "Alingsas" "SE" ;;
        58) show_weather "Vastervik" "SE" ;;
        59) show_weather "Kungalv" "SE" ;;
        60) show_weather "Hudiksvall" "SE" ;;

        # Internationella städer
        70) show_weather "London" "GB" ;;
        71) show_weather "Paris" "FR" ;;
        72) show_weather "Berlin" "DE" ;;
        73) show_weather "New York" "US" ;;
        74) show_weather "Tokyo" "JP" ;;
        75) show_weather "Dubai" "AE" ;;
        76) show_weather "Oslo" "NO" ;;
        77) show_weather "Copenhagen" "DK" ;;
        78) show_weather "Helsinki" "FI" ;;
        79) show_weather "Reykjavik" "IS" ;;

        0)
            echo ""
            read -p "  Ange stad: " custom_city
            read -p "  Ange landskod (t.ex. SE, GB, US): " custom_country
            if [ -n "$custom_city" ] && [ -n "$custom_country" ]; then
                show_weather "$custom_city" "$custom_country"
            else
                echo "  ❌ Du måste ange både stad och landskod!"
                echo ""
            fi
            ;;
        q|Q)
            echo ""
            echo "  👋 Tack för att du använde vädersystemet!"
            echo ""
            exit 0
            ;;
        *)
            echo ""
            echo "  ❌ Ogiltigt val! Välj ett nummer från menyn."
            echo ""
            sleep 2
            clear
            echo "╔═══════════════════════════════════════════════════════╗"
            echo "║          VÄLKOMMEN TILL VÄDERSYSTEMET                ║"
            echo "╚═══════════════════════════════════════════════════════╝"
            echo ""
            ;;
    esac
done
