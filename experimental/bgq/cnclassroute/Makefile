# The following should point to the BGQ work area that contains the hwi/spi include files
WORK = /home/dougmill/cvs/bgq/work
SPI = $(WORK)/spi
HWI = $(WORK)/hwi
COPTS =
CFLAGS = -g -I$(WORK) -I../../../ -I$(SPI)/include/kernel/cnk -I$(SPI)/include/mu/xmi

HDRS = testlib.h
OBJS =
BINS = testroots testcomms testsparse testalloc

all: $(BINS)

%: %.c $(OBJS) $(HDRS)
	$(CC) $(CFLAGS) -o $@ $< $(OBJS) $(COPTS)

cnclassroute.o: cnclassroute.c cnclassroute.h
	$(CC) $(CFLAGS) -c -o $@ $< $(COPTS)

clean:
	rm -f $(OBJS)

clobber: clean
	rm -f $(BINS)
