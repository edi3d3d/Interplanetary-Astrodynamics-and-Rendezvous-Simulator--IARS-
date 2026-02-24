CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -O0 $(shell sdl2-config --cflags)
LDFLAGS=
LIBS=$(shell sdl2-config --libs) -lGL -lGLU -lm
TARGET=main
SRC=main.c camera.c vec3.c quaternions.c

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS) $(LIBS)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)
