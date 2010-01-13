/* ************************************************************************* */
/*                            IBM Confidential                               */
/*                          OCO Source Materials                             */
/*                      IBM XL UPC Alpha Edition, V0.9                       */
/*                                                                           */
/*                      Copyright IBM Corp. 2009, 2010.                      */
/*                                                                           */
/* The source code for this program is not published or otherwise divested   */
/* of its trade secrets, irrespective of what has been deposited with the    */
/* U.S. Copyright Office.                                                    */
/* ************************************************************************* */
/**
 * \file algorithms/protocols/tspcoll/tspcoll_commsplit.cc
 * \brief ???
 */

#include "interface/Communicator.h"

struct Split
{
  Split (int c, int r, int g) { color=c; rank=r; absrank=g; }
  int color, rank, absrank;
};

int TSPColl::Communicator::split (int color, int rank, int * proclist)
{
  /* -------------------------------------------------------------- */
  /* prepare local buffer for allgather operation                   */
  /* prepare destination buffer for allgather operation             */
  /* -------------------------------------------------------------- */

  Split * dstbuf = (Split *) malloc (sizeof(Split) * this->size());
  Split   sndbuf(color, rank, this->absrankof(this->rank()));
  if (!dstbuf) CCMI_FATALERROR (-1, "comm_split: allocation error");

  /* -------------------------------------------------------------- */
  /*      allgather (localbuffer, tempbuffer, 3 * sizeof(int))      */
  /* -------------------------------------------------------------- */

  this->allgather (&sndbuf, dstbuf, sizeof(Split));

  /* -------------------------------------------------------------- */
  /* collect list of global processor IDs proc enumeration buffer   */
  /* -------------------------------------------------------------- */

  for (int i=0; i<this->size(); i++) proclist[i] = -1;

  int commsize = 0;
  for (int i=0; i<this->size(); i++)
    if (dstbuf[i].color == color)
      {
        proclist[dstbuf[i].rank] = dstbuf[i].absrank;
        if (dstbuf[i].rank >= commsize) commsize = dstbuf[i].rank + 1;
      }
  free (dstbuf);

  /* -------------------------------------------------------------- */
  /*             test new proc list for consistency                 */
  /* -------------------------------------------------------------- */

  for (int i=0; i<commsize; i++)
    if (proclist[i] == -1)
        CCMI_FATALERROR (-1, "SPLIT: Invalid new communicator");

  return commsize;
}
