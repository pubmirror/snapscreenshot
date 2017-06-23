VERSION = 1.0.14.3
ARCHDIR = archives/
ARCHNAME = snapscreenshot-$(VERSION)
ARCHFILES = snapscreenshot.cc argh.h argh.c examplerun \
            README autoptr configure progdesc.php COPYING \
            snapscreenshot.1

CC=g++
CPP=gcc
CXX=g++
LD=g++
CPPFLAGS=-DVERSION="\"$(VERSION)\""
CXXFLAGS=-Wall -W -pedantic
CFLAGS=-Wall -W -pedantic -O3
INSTALL=install
BINDIR=/usr/local/bin
MANDIR=/usr/local/man

PROG=snapscreenshot
OBJS=snapscreenshot.o argh.o

INSTALLPROGS=${PROG}
INSTALLMANS=snapscreenshot.1

all: ${PROG}
${PROG}: ${OBJS}
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f ${OBJS}
distclean: clean
	rm -f *~ *.bak

include depfun.mak
