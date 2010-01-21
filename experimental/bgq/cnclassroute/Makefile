# The following should point to the BGQ work area that contains the hwi/spi include files
WORK = /home/dougmill/cvs/bgq/work
SPI = $(WORK)/spi
HWI = $(WORK)/hwi
COPTS =
CFLAGS = -g -I$(SPI)/include -I$(HWI)/include -I$(WORK) -I../../../

HDRS = cnclassroute.h testlib.h
OBJS = cnclassroute.o
BINS = testroots testcomms testsparse

all: $(BINS)

%: %.c cnclassroute.o $(HDRS)
	$(CC) $(CFLAGS) -o $@ $< cnclassroute.o $(COPTS)

cnclassroute.o: cnclassroute.c cnclassroute.h
	$(CC) $(CFLAGS) -c -o $@ $< $(COPTS)

clean:
	rm -f $(OBJS)

clobber: clean
	rm -f $(BINS)
