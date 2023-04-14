all: clean server

CC = clang
CFLAGS = -g -Wno-everything -pthread -lm

server:
	$(CC) $(CFLAGS) server.c -o server
	./server

clean:
	rm -f server

wasm:
	clang --target=wasm32 -emit-llvm -c -S src/main.c -o src/main.ll
	llc -march=wasm32 -filetype=obj src/main.ll -o src/main.o
	wasm-ld --no-entry --export-all -o public/main.wasm src/main.o
	#cd public && npx static-here
