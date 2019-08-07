Before you can run 'make', you have to open the Makefile and insert the paths to your sfml and mpg123 installations where indicated.

Then run make. It will build executable 'VinzcPlayer'

export LD_LIBRARY_PATH=/home/path/to/SFML-2.5.1/lib


If SFML is already on your LD_LIBRARY_PATH, invoke program using:

	./VinzcPlayer music_directory
	
nb: for now it read from a directory hardcoded but in future release your just give the directory name

Otherwise invoke using:

	export LD_LIBRARY_PATH=/path-to-sfml-install/lib/ && /VinzcPlayer music_directory

ESCAPE key exits, SPACE pauses and plays.
N : next song
with keyboard : voting buttons AZER
with joystick : voting 0,1,2,3
