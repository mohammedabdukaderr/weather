CC = gcc                     # Definierar GCC som kompilator
CFLAGS = -Wall -std=c11      # Definierar kompileringsflaggor
LDFLAGS = -lcurl             # Definierar länkarflaggor (CURL-biblioteket)
TARGET = vader               # Definierar namn på körbar fil

all: $(TARGET)               # Standardmål: bygger programmet

$(TARGET): main.o weather.o  # Regel för att länka objektfiler
	$(CC) $^ -o $@ $(LDFLAGS)  # Kommando för att länka

%.o: %.c weather.h           # Regel för att kompilera .c-filer
	$(CC) $(CFLAGS) -c $< -o $@  # Kommando för att kompilera

clean:                       # Mål för att rensa byggfiler
	rm -f *.o $(TARGET) cache.txt  # Tar bort objektfiler, körbar fil och cache

run: $(TARGET)               # Mål för att bygga och köra
	./$(TARGET)              # 0Kommando för att köra programmet

.PHONY: all clean run        # Markerar dessa mål som "phony" (inte filnamn)