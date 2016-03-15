CCC=g++
# BINS=forked-server modbusd client httpd httpd_1 httpd_2 # echoserv
BINS=forked-server

all:	$(BINS)

forked-server:	forked-server.c helper.o
	$(CCC) -g forked-server.c helper.o -o forked-server

helper.o:	helper.c
	$(CCC) -c -g $? -o $@

clean:
	rm -f $(BINS) *.o cscope.out

