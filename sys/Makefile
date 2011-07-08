HEADERS   = ${shell echo *.h}
HEADERS_O = ${patsubst %.h,  %.h.o, ${HEADERS}}
INSTALL_DIR ?= .
.PHONY: docs clean check ${SUBDIRS} default all install

default all install: check docs

docs:
	${RM} -r ${INSTALL_DIR}/doc
	( cat ./Doxyfile; echo "OUTPUT_DIRECTORY=${INSTALL_DIR}/doc" ) | doxygen - > /dev/null
	@echo
	@echo "OUTPUT_DIRECTORY=${INSTALL_DIR}/doc"

check: ${HEADERS_O} pami_checkall.h.o
pami_checkall.h.o: ${HEADERS}
	@perl cmp ${CC} $@ ${HEADERS}
	@echo All of {${HEADERS}} parse together
${SUBDIRS}:
	${MAKE} -C $@

clean:
	${RM} -r ${INSTALL_DIR}/doc *~ *.h.o
	for i in ${SUBDIRS} ; do ${MAKE} -C $$i clean ; done

%.h.o: %.h
	@perl cmp ${CC} $@ $^
	@echo "$^" parses
%.txt: %.pod
	podchecker $^
	pod2text $^ > $@
