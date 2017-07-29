#Simple makefile for all programs

TARGETS= TServer TClient TDummy
CC=gcc
CFLAGS = -Wall -g -std=c99 -pthread

all: clean $(TARGETS)

$(TARGETS):
	$(CC) $(CFLAGS) $@.c -o $@

clean:
	rm -f $(TARGETS)
