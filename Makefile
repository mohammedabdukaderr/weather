# Kompilator och flaggor
CC = gcc
CFLAGS = -Wall -std=c11   #flaggar alla error i terminalen
LDFLAGS = -lcurl -ljansson #flaggar alla error från json

# Körbar fil (./vader)
TARGET = vader

# Alla källfiler
SRC = main.c weather.c json_parser.c
OBJ = $(SRC:.c=.o) //skapa objekfiler 

# Standardmål: bygga programmet
all: $(TARGET)

# Länkning av objektfiler till körbar fil
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS) 

# Kompilering av .c-filer till .o-filer
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rensa byggfiler och cache
clean:
	rm -f $(OBJ) $(TARGET) cache.txt

# Bygg och kör programmet
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
