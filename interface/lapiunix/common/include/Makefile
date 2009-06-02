# * ************************************************************************* *
# *                            IBM Confidential                               *
# *                          OCO Source Materials                             *
# *                      IBM XL UPC Alpha Edition, V0.9                       *
# *                                                                           *
# *                      Copyright IBM Corp. 2005, 2007.                      *
# *                                                                           *
# * The source code for this program is not published or otherwise divested   *
# * of its trade secrets, irrespective of what has been deposited with the    *
# * U.S. Copyright Office.                                                    *
# * ************************************************************************* *



include ../Make.rules

# TODO: remove from the final distribution
INSTALLABLE_HDRS = \
pgasrt_config.h pgasrt.h pgasrt_hbrdcoll.h pgasrt_hbrd.h \
pgasrt_misc.h pgasrt_smpcoll.h pgasrt_smp.h pgasrt_tspcoll.h \
pgasrt_tsp.h pgasrt_types.h xlupc_types.h

default: install

install:
	@-mkdir -p $(PGASRT_INSTALL_INC)
	@-for file in ${INSTALLABLE_HDRS}; do \
	  cp -f $$file $(PGASRT_INSTALL_INC); \
	done

clean:
	@echo "done"

