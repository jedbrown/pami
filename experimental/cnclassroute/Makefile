COPTS =
CFLAGS = -g

HDRS = cnclassroute.h testlib.h
OBJS = cnclassroute.o
BINS = testroots testcomms
SPARSE_BINS = testsparse
ifdef SPARSE
BINS += $(SPARSE_BINS)
COPTS += -DSUPPORT_SPARSE_RECTANGLE
# need to force re-compile of cnclassroute.o ...
endif

all: $(BINS)

%: %.c cnclassroute.o $(HDRS)
	$(CC) $(CFLAGS) -o $@ $< cnclassroute.o $(COPTS)

cnclassroute.o: cnclassroute.c cnclassroute.h
	$(CC) $(CFLAGS) -c -o $@ $< $(COPTS)

clean:
	rm -f $(OBJS)

clobber: clean
	rm -f $(BINS) $(SPARSE_BINS)
