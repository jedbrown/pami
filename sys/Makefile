SUBDIRS   = examples examples/config
HEADERS   = ${shell echo *.h}
HEADERS_O = ${patsubst %.h,  %.h.o, ${HEADERS}}
.PHONY: docs clean check ${SUBDIRS} default all install

default all install: check docs

docs:
	${RM} -r html
	doxygen ./Doxyfile

check: ${HEADERS_O} examples examples/config xmi_checkall.h.o
xmi_checkall.h.o: ${HEADERS}
	@perl cmp ${CC} $@ ${HEADERS}
	@echo All of {${HEADERS}} parse together
${SUBDIRS}:
	${MAKE} -C $@

clean:
	${RM} -r html *~ *.h.o
	for i in ${SUBDIRS} ; do ${MAKE} -C $$i clean ; done

%.h.o: %.h
	@perl cmp ${CC} $@ $^
	@echo "$^" parses
%.txt: %.pod
	podchecker $^
	pod2text $^ > $@
