.PHONY: all compile install remove clean

all: compile install
	
compile:
	gcc main.c -o xga-painter `sdl2-config --cflags --libs`
	
install:
	sudo cp xga-painter /bin

remove:
	sudo rm -f /bin/xga-painter
	
clean:
	rm xga-painter
