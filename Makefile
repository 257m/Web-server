all: clean server run

CC = clang
CFLAGS = -Wno-everything

server:
	$(CC) -g $(CFLAGS) server.c -o server

run:
	./server

clean:
	rm -f server

install:
	$(CC) $(CFLAGS) server.c -o server
	cp server ~/../usr/bin/cserver

wasm:
	clang --target=wasm32 -emit-llvm -c -S src/main.c -o src/main.ll
	llc -march=wasm32 -filetype=obj src/main.ll -o src/main.o
	wasm-ld --no-entry --export-all -o public/main.wasm src/main.o
	#cd public && npx static-here
