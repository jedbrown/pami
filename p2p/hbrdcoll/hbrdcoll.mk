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

VPATH += ../../../coll/hbrdcoll
INC   += -I ../../../coll

OBJS	+= hbrdcoll_communicator_impl.o
OBJS	+= hbrdcoll_communicator.o
OBJS	+= hbrdcoll_c_intf.o
