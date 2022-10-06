CC=gcc
CFLAGS=-Wall -I.

mkcrypt: mkcrypt.c
	$(CC) $(CFLAGS) -o $@ $< -lcrypt

clean:
	rm -rf *~ a.out core mkcrypt

install: mkcrypt
	install -m 755 -o root -g root mkcrypt /usr/local/bin
