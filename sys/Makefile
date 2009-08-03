SUBDIRS=examples/config
.PHONY: docs clean check ${SUBDIRS}
docs:
	doxygen ./Doxyfile

clean:
	${RM} -r html *~ *.h.o
	for i in ${SUBDIRS} ; do ${MAKE} -C $$i clean ; done

HEADERS   = ${shell echo *.h}
HEADERS_O = ${patsubst %.h,  %.h.o, ${HEADERS}}
check: examples/config ${HEADERS_O}
	(						\
		for i in ${HEADERS} ; do		\
			 echo "#include \"$$i\"";	\
		 done;					\
		 echo "void foo(){;}"			\
	) | ${CC} -x c -c -o xmi_checkall.h.o -
%.h.o: %.h
	(					\
		echo "#include \"$^\"";		\
		echo "void foo(){;}";		\
	) | ${CC} -x c -c -o $@ -
${SUBDIRS}:
	${MAKE} -C $@
