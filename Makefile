CDEBUG = -DNDEBUG
CFLAGS = -O2 $(shell pkg-config --cflags gtk+-2.0) $(CDEBUG)
CC = gcc
ifeq ($(shell uname -m),x86_64)
  #CODE = 32bit
endif
ifeq ($(CODE),32bit)
  CFLAGS += -m32
  ORG_PLUGIN = /opt/Adobe/flash-player32/libflashplayer.so
else
  ORG_PLUGIN = /opt/Adobe/flash-player/flash-plugin/libflashplayer.so
endif
DESTDIR = /opt/fph

all: fph.libflashplayer.so libfph.so

fph.libflashplayer.so: $(ORG_PLUGIN) replace-symbol.sed
	./replace-symbol.sed $(ORG_PLUGIN) > $@

libfph.so: libfph.c
	$(CC) -shared -fPIC -o $@ $(CFLAGS) $<

debug: CDEBUG =
debug: all

install: all
	install -d $(DESTDIR)
	install -s libfph.so $(DESTDIR)
	install fph.libflashplayer.so $(DESTDIR)

clean:
	rm -f libfph.so fph.libflashplayer.so
