default: main

main:
	clear 
	gcc main.c gui.c network.c `pkg-config --libs --cflags gtk+-3.0` -export-dynamic -g -lm -lSDL2 -lSDL2_image -o client
	gcc server.c -o server -lm

run: main
	./client 
debug: main
	gdb ./client	
server: main
	./server
s: server	
clean:
	rm -f client server