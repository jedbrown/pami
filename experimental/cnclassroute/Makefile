CFLAGS = -g

HDRS = cnclassroute.h
OBJS = cnclassroute.o
BINS = test

all: $(BINS)

test: test.c cnclassroute.o cnclassroute.h

cnclassroute.o: cnclassroute.c $(HDRS)

clean:
	rm -f $(OBJS)

clobber: clean
	rm -f $(BINS)
