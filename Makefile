# Copyright © Bart Massey Feb 2004
# ALL RIGHTS RESERVED
# [This program is licensed under the "3-clause ('new') BSD License"]
# Please see the file COPYING in the source
# distribution of this software for license terms.

CC = gcc
CFLAGS = -Wall -O4 -I/usr/include/netpbm

DESTDIR = /usr/local

ppmgrayscreen: ppmgrayscreen.c
	$(CC) $(CFLAGS) -o ppmgrayscreen ppmgrayscreen.c -lnetpbm

install: ppmgrayscreen ppmgrayscreen.1
	cp ppmgrayscreen $(DESTDIR)/bin/
	cp ppmgrayscreen.1 $(DESTDIR)/man/man1/

clean:
	-rm -f ppmgrayscreen
