CC = gcc
#Using -Ofast instead of -O3 might result in faster code, but is supported only by newer GCC versions
CFLAGS = -lm -pthread -O3 -march=native -Wall -funroll-loops -Wno-unused-result

all: src/hwe.c

hwe: src/hwe.c
	$(CC) $^ -o $@ $(CFLAGS)

clean:
	rm -rf hwe
