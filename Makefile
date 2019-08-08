
CC=g++
CFLAGS=-std=c++11 -g -Wl,--no-undefined -w

# IMPORTANT
# replace <TODO> with your installation directories
#
SFML=~/00_perso/dev/SFML-2.5.1
MPG123=/usr/local

INCLUDESFML=$(SFML)/include
INCLUDEMPG123=$(MPG123)/include
#INCLUDEVLC=$(VLC)/include

LIBSFML=$(SFML)/lib
LIBMPG123=$(MPG123)/lib

INCLUDES := -I $(INCLUDEMPG123) -I $(INCLUDESFML)
# for non-debug builds
LIBS     := -L $(LIBMPG123) -lmpg123 -L $(LIBSFML) -l sfml-graphics -l sfml-window -l sfml-system -l sfml-audio -lid3 -ljsoncpp -lmagic -lvlc
# for debug builds
#LIBS     := -L $(LIBMPG123) -lmpg123 -L $(LIBSFML) -lsfml-graphics-d -lsfml-window-d -lsfml-system-d -lsfml-audio-d -lid3 -ljsoncpp

AUDIOEXE := VinzcPlayer

$(AUDIOEXE): Main.o
	$(CC) $(CFLAGS) $< -o $@ multimedia.o tagHelper.o $(LIBS)

Main.o: mp3reader.cpp mp3reader.hpp multimedia.o tagHelper.o
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@ -l/id3lib/bin/Release/id3lib

tagHelper.o: tagHelper.cpp tagHelper.h
	$(CC) $(CFLAGS) -c $< -o $@ -l/id3lib/bin/Release/id3lib

multimedia.o: multimedia.cpp multimedia.hpp
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@
 
clean:
	rm $(AUDIOEXE) *.o
