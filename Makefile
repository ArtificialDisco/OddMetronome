RMF = rm -f

CC = gcc
CO = gcc -c

LIBS = -lm -lSDL_image -lSDL_mixer -lSDL_ttf `sdl-config --libs`
CFLAGS = `sdl-config --cflags`

SOURCES = Metronome.cpp \
		  drawing.cpp \
		  button.cpp \
		  init.cpp

HEADERS = metronome.h \
		  drawing.h \
		  button.h \
		  init.h

OBJECTS = Metronome.o \
		  drawing.o \
		  button.o \
		  init.o

TARGET = metronome

.SUFFIXES: .cpp
.cpp.o:
	$(CO) $< -o $@ $(CFLAGS)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LIBS)

clean:
	$(RMF) $(OBJECTS) $(TARGET)

metronome.o: Metronome.cpp metronome.h init.h drawing.h button.h
init.o: init.cpp init.h metronome.h button.h
drawing.o: drawing.cpp drawing.h metronome.h init.h button.h
button.o: button.cpp button.h metronome.h
