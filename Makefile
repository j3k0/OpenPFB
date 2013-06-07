INSTALLDIR=/usr
#INSTALLDIR=/home/hydre2/opt

CPP=g++
CPPFLAGS=-g -Wall -O3
LD=g++
# LDFLAGS=-lstdc++

all: libOpenPfb.so

OpenPfb.o: OpenPfb.cpp OpenPfb.h
	${CPP} ${CPPFLAGS} -fPIC -c OpenPfb.cpp -o OpenPfb.o

test_OpenPfb.o: test_OpenPfb.cpp OpenPfb.h

libOpenPfb.so: OpenPfb.o
	ld -G -o libOpenPfb.so OpenPfb.o

test: test_openpfb

test_openpfb: test_OpenPfb.o libOpenPfb.so
	${LD} ${LDFLAGS} -lOpenPfb test_OpenPfb.o -o test_openpfb

clean:
	@rm -fv *.o *~ test_openpfb *.so

install: libOpenPfb.so OpenPfb.h
	@cp -v libOpenPfb.so ${INSTALLDIR}/lib/
	@cp -v OpenPfb.h ${INSTALLDIR}/include

uninstall:
	@rm -fv ${INSTALLDIR}/lib/libOpenPfb.so
	@rm -fv ${INSTALLDIR}/include/OpenPfb.h
