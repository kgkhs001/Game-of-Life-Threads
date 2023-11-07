all: addem life

addem: addem.c
	gcc addem.c -o addem -lpthread

life: life.c
	gcc life.c -o life -lpthread

clean:
	rm -f addem
	rm -f life
