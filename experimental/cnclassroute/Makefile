CFLAGS = -g

HDRS = cnclassroute.h testlib.h
OBJS = cnclassroute.o
BINS = testroots

all: $(BINS)

%: %.c cnclassroute.o $(HDRS)
	$(CC) $(CFLAGS) -o $@ $< cnclassroute.o

cnclassroute.o: cnclassroute.c cnclassroute.h

clean:
	rm -f $(OBJS)

clobber: clean
	rm -f $(BINS)
