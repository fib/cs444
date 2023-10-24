CC=cc
CFLAGS=-Wall -O2

all: raid diar

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f a.out *.part? *.2
