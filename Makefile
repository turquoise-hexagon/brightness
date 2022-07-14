CC ?= gcc

CFLAGS  += -pedantic -Wall -Wextra
LDFLAGS +=

PREFIX ?= $(DESTDIR)/usr
UDVDIR  = $(DESTDIR)/etc/udev/rules.d
BINDIR  =  $(PREFIX)/bin
MANDIR  =  $(PREFIX)/share/man/man1

BINSRC = $(wildcard *.c)
MANSRC = $(wildcard *.rst)
UDV = 90-brightness.rules
BIN = $(BINSRC:.c=)
MAN = $(MANSRC:.rst=.1.gz)

all : CFLAGS += -O3 -march=native
all : $(BIN) $(MAN)

debug : CFLAGS += -O3 -g
debug : $(BIN) $(MAN)

$(BIN) : $(BINSRC)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

$(MAN) : $(MANSRC)
	rst2man.py $< | gzip -9 > $@

clean :
	rm -f $(BIN)
	rm -f $(MAN)

install : all
	install -Dm644 $(UDV) -t $(UDVDIR)
	install -Dm755 $(BIN) -t $(BINDIR)
	install -Dm644 $(MAN) -t $(MANDIR)

uninstall :
	rm -f $(UDVDIR)/$(UDV)
	rm -f $(BINDIR)/$(BIN)
	rm -f $(MANDIR)/$(MAN)

.PHONY : all debug clean install uninstall
