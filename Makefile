CC = gcc
CFLAGS = -Wall -O2
TARGET = ntpp
OBJ = ntpp.o

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man/man1

all: options $(TARGET)

options:
	@echo $(TARGET) build options:
	@echo "CFLAGS	= $(CFLAGS)"
	@echo "CC	= $(CC)"

$(TARGET):$(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

install:
	install -m 0755 $(TARGET) $(BINDIR)
	install -m 0644 ntpp.1 $(MANDIR)

clean:
	rm -f $(TARGET) $(OBJ)
