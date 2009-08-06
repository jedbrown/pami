SUBDIRS=examples examples/config
.PHONY: docs clean check ${SUBDIRS}

docs:
	doxygen ./Doxyfile

clean:
	${RM} -r html *~ *.h.o
	for i in ${SUBDIRS} ; do ${MAKE} -C $$i clean ; done

HEADERS   = ${shell echo *.h}
HEADERS_O = ${patsubst %.h,  %.h.o, ${HEADERS}}
check: ${HEADERS_O} examples examples/config xmi_checkall.h.o
xmi_checkall.h.o: ${HEADERS}
	@perl cmp ${CC} $@ ${HEADERS}
	@echo All of {${HEADERS}} parse together
%.h.o: %.h
	@perl cmp ${CC} $@ $^
	@echo "$^" parses
${SUBDIRS}:
	${MAKE} -C $@
