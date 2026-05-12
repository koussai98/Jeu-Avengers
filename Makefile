all:
	gcc src/*.c -o game -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lm

clean:
	rm -f game
