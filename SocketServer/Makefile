CCC=g++
# BINS=forked-server modbusd client httpd httpd_1 httpd_2 # echoserv
BINS=tst forked-server
LIBS=-lhiredis
INCS=-I/usr/include/hiredis
CPPFLAGS=-Wwrite-strings

all:	$(BINS)

globals.o:	globals.cpp globals.h
	$(CCC) $(CPPFLAGS) -c -g globals.cpp -o globals.o

forked-server:	forked-server.c helper.o globals.o
	$(CCC) $(CPPFLAGS) -g forked-server.c helper.o globals.o $(INCS) $(LIBS) -o forked-server

helper.o:	helper.c
	$(CCC) -c -g $? -o $@

tst:	tst.cpp globals.o globals.h
	$(CCC) $(CPPFLAGS) -g tst.cpp globals.o -o tst

clean:
	rm -f $(BINS) *.o cscope.out

