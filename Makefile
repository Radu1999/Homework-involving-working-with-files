build:
	gcc -g archiver.c -o archiver
run: build
	./archiver
clean: build
	rm archiver
