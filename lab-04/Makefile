.PHONY: all clean

all: pc rw

pc: pc.c
	$(CC) $(CFLAGS) $^ -o build/$@

rw: rw.c
	$(CC) $(CFLAGS) $^ -o build/$@

clean:
	$(RM) build/*
