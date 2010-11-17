# * ************************************************************************* *
# *                            IBM Confidential                               *
# *                          OCO Source Materials                             *
# *                      IBM XL UPC Alpha Edition, V0.9                       *
# *                                                                           *
# *                      Copyright IBM Corp. 2009, 2010.                      *
# *                                                                           *
# * The source code for this program is not published or otherwise divested   *
# * of its trade secrets, irrespective of what has been deposited with the    *
# * U.S. Copyright Office.                                                    *
# * ************************************************************************* *
VPATH += .. / .. / .. / coll / tspcoll
         INC   += -I.. / .. / .. / coll

                  OBJS += tspcoll_c_intf.o
                          OBJS += tspcoll_communicator.o
                                  OBJS += tspcoll_nbcoll.o
                                          OBJS += tspcoll_shortallreduce.o
                                                  OBJS += tspcoll_longallreduce.o
                                                          OBJS += tspcoll_allreduce_exec.o
                                                                  OBJS += tspcoll_commsplit.o

                                                                          OBJS += tspcoll_scatter.o
#OBJS += tspcoll_gather.o
