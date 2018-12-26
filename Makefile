CC = gcc
#Using -Ofast instead of -O3 might result in faster code, but is supported only by newer GCC versions
CFLAGS = -lm -pthread -O3 -march=native -Wall -funroll-loops -Wno-unused-result

.PHONY: all run

all: hwe run

hwe: src/hwe.c
	$(CC) $^ -o $@ $(CFLAGS)

demo/enwik8: | demo/enwik8.zip
	unzip $| -d demo

demo/enwik8.zip:
	wget http://cs.fit.edu/~mmahoney/compression/enwik8.zip -O $@

run: hwe demo/enwik8
	mkdir -p run
	./hwe -train demo/enwik8 -output run/enwik8.emb -size 100 -window 5 -sample 1e-4 -negative 5 -binary 0 -fmode 2 -knfile demo/wordnetlower.tree -iter 2 -threads 32

clean:
	rm -rf hwe run
