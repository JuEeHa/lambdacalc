DESTDIR ?=
PREFIX ?= /usr/local
EXEC_PREFIX ?= $(PREFIX)
BINDIR ?= $(DESTDIR)$(EXEC_PREFIX)/bin

CFLAGS = -Os -pipe -std=c11 -pedantic -Wall -Wextra
CPPFLAGS =
LDFLAGS =

all: lambdacalc

lambdacalc: lambdacalc.o
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $<

.SUFFIXES:
.SUFFIXES: .o .c

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f *.o lambdacalc

install:
	install lambdacalc $(BINDIR)

.PHONY: all clean install
