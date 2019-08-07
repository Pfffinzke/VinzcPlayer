
CC=g++
CFLAGS=-std=c++11 -g -Wl,--no-undefined

# IMPORTANT
# replace <TODO> with your installation directories
#
SFML=~/00_perso/dev/SFML-2.5.1
MPG123=/usr/local

INCLUDESFML=$(SFML)/include
INCLUDEMPG123=$(MPG123)/include

LIBSFML=$(SFML)/lib
LIBMPG123=$(MPG123)/lib

INCLUDES := -I $(INCLUDEMPG123) -I $(INCLUDESFML)
# for non-debug builds
LIBS     := -L $(LIBMPG123) -lmpg123 -L $(LIBSFML) -l sfml-graphics -l sfml-window -l sfml-system -l sfml-audio -lid3 -ljsoncpp
# for debug builds
#LIBS     := -L $(LIBMPG123) -lmpg123 -L $(LIBSFML) -lsfml-graphics-d -lsfml-window-d -lsfml-system-d -lsfml-audio-d -lid3 -ljsoncpp

AUDIOEXE := VinzcPlayer

$(AUDIOEXE): Main.o
	$(CC) $(CFLAGS) $< -o $@ SoundFileReaderMp3.o tagHelper.o $(LIBS)

Main.o: mp3reader.cpp mp3reader.hpp SoundFileReaderMp3.o tagHelper.o
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@ -l/id3lib/bin/Release/id3lib

tagHelper.o: tagHelper.cpp tagHelper.h
	$(CC) $(CFLAGS) -c $< -o $@ -l/id3lib/bin/Release/id3lib

SoundFileReaderMp3.o: SoundFileReaderMp3.cpp SoundFileReaderMp3.hpp
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@
 
clean:
	rm $(AUDIOEXE) *.o