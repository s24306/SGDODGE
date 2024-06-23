main: clean debug
	g++ main.cpp -w -lSDL2 -lSDL2_ttf -lSDL2_mixer -o SGDODGE
clean:
	rm -f main

debug:
	echo "Compiling project"

.SILENT:
