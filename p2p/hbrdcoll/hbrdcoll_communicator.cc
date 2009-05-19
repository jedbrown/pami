/* ************************************************************************* */
/*                            IBM Confidential                               */
/*                          OCO Source Materials                             */
/*                      IBM XL UPC Alpha Edition, V0.9                       */
/*                                                                           */
/*                      Copyright IBM Corp. 2005, 2007.                      */
/*                                                                           */
/* The source code for this program is not published or otherwise divested   */
/* of its trade secrets, irrespective of what has been deposited with the    */
/* U.S. Copyright Office.                                                    */
/* ************************************************************************* */

#include "./Communicator.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* ************************************************************************ */
/*                     Communicator constructor                             */
/* ************************************************************************ */

HybridColl::Communicator::Communicator (int r, int s, 
					TSPColl::Communicator * tspcomm,
					SMPColl::Communicator * smpcomm): 
  _rank (r), _size (s), _tspcomm(tspcomm), _smpcomm (smpcomm)
{
}


#define COURSEOF(thrd) ((thrd)/(_BF*_ncomms))
#define COMMOF(thrd)   (((thrd)/_BF)%_ncomms)
#define VIRTOF(thrd)   (((thrd)%_BF)+(_BF*COURSEOF(thrd)))

/* ************************************************************************ */
/*                     Blocked communicator constructor                     */
/* ************************************************************************ */

HybridColl::BC_Comm::BC_Comm (int BF, int ncomms,
			      TSPColl::Communicator * tspcomm,
			      SMPColl::Communicator * smpcomm):
  Communicator (0, 0, tspcomm, smpcomm), _BF(BF), _ncomms(ncomms)
{
  _rank = VIRTOF (PGASRT_MYTHREAD);
  _mycomm = COMMOF (PGASRT_MYTHREAD);
  int c = COMMOF(PGASRT_THREADS);
  if (c < _mycomm)
      _size = _BF * (COURSEOF(PGASRT_THREADS));
  else if (c == _mycomm)
      _size = _BF * COURSEOF(PGASRT_THREADS) + (PGASRT_THREADS%_BF);
  else
      _size = _BF * (COURSEOF(PGASRT_THREADS)+1);
}

/* ************************************************************************ */
/*    absolute rank corresponding to virtual rank in *my* communicator      */
/* ************************************************************************ */
/*  rank/BF == block corresponding to rank                                  */
/*  (rank/BF) * BF * ncomms = course                                        */
/* ************************************************************************ */

int HybridColl::BC_Comm::absrankof (int rank) const
{

  return
    (rank/_BF)*_BF*_ncomms +    /* current course of rank               */
    _mycomm * _BF +             /* block in course of *my* communicator */
    (rank%_BF);                 /* rank's phase in block                */
}

/* ************************************************************************ */
/*    virtual rank of a particular absolute rank                            */
/* ************************************************************************ */

int HybridColl::BC_Comm::virtrankof (int rank) const
{
  if (COMMOF(rank)==_mycomm) return VIRTOF(rank);
  return -1;
}

/* ************************************************************************ */
/* virtual tsp rank in current communicator                                 */
/* ************************************************************************ */

int HybridColl::BC_Comm::to_tsp     (int rank) const
{
  int absrank = absrankof (rank);
  int abstsprank = absrank / PGASRT_SMPTHREADS;
  int virttsprank = _tspcomm->virtrankof (abstsprank);
  return virttsprank;
}
