all: build run

build:
	gcc -m32 main.c -o demo_elf

run:
	./demo_elf

clean:
	rm -f demo_elf
