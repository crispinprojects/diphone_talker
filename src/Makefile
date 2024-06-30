#--------------------------------------------------------------
# Diphone Talker Makefile
# Author: Alan Crispin <crispinalan@gmail.com> 
# Date: February 2024
# Changelog: 0.01
#----------------------------------------------------------------

CC ?= gcc
PKGCONFIG = $(shell which pkg-config)
CFLAGS = $(shell $(PKGCONFIG) --cflags gtk4)
LIBS = $(shell $(PKGCONFIG) --libs gtk4) -lm  -lasound

SRC = main.c dictionary.c diphone.c

OBJS = $(SRC:.c=.o)

all: talker

%.o: %.c
	$(CC) -c -o $(@F) $(CFLAGS) $<

talker: $(OBJS)
	$(CC) -o $(@F) $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS)
	rm -f talker
