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

VPATH	+= ../../smp

OBJS	+= smp_thread.o
OBJS    += smp_thread_stub.o
OBJS	+= smp_barrier.o
OBJS    += smp_fence.o
