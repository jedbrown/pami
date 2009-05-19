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

VPATH	+= ../../coll/test
INC	+= -I../../coll

APPS    += tspcoll_alltests
APPS    += tspcoll_split
APPS	+= tspcoll_block
APPS	+= tspcoll_ar_latency
APPS	+= tspcoll_bcasttest
APPS    += smpcoll_alltests
APPS	+= smpcoll_ar_test
APPS	+= smpcoll_block
#APPS	+= smpcoll_split
APPS	+= smpcoll_block
APPS	+= hbrdcoll_alltests
APPS	+= hbrdcoll_ar_test
APPS	+= hbrdcoll_block
#APPS	+= hbrdcoll_split
