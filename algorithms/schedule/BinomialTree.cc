/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file schedule/BinomialTree.cc
 * \brief The Binomial Schedule for Barrier, Broadcast, and [All]Reduce.
 *
 * Also used by the Rectangle schedule to implement Reduce.
 */

#include "./BinomialTree.h"
#include <new>
/**
 * \brief Setup binomial schedule context
 *
 * Computes important values for later use in building schedule
 * phases/steps. This includes values used to handle non-powers of two.
 * This computes values which require knowing the root node and/or
 * type of collective (_op).
 *
 * \param[in] node	Our node number (index) in ranks[] array,
 *			or position along line of nodes.
 */
void CCMI::Schedule::BinomialTreeSchedule::
setupContext(unsigned &startph, unsigned &nph)
{
  unsigned st, np;

  bool peer = false;
  bool outside = false;
  if(_mynode < _pnranks)
  {
    // peer nodes to the outsiders
    peer = true;
  }
  else if(_mynode < _hnranks)
  {
    // rest of the power of two nodes,
    // or all nodes if a power of two
  }
  else
  {
    // the outsiders - only if non-power of two
    outside = true;
  }
  _recvph = NO_PHASES;
  _sendph = NO_PHASES;

  // Assume (default) we are only performing as a
  // power-of-two (standard binomial).
  st = 1;
  np = _nphbino;
  _auxrecvph = NO_PHASES;
  _auxsendph = NO_PHASES;

  if(peer)
  {
    /* non-power of two */
    switch(_op)
    {
    case BARRIER_OP:
    case ALLREDUCE_OP:
      st = 0;
      np += 2;
      _auxrecvph = st;
      _auxsendph = _nphases - 1;
      break;
    case REDUCE_OP:
      st = 0;
      np += 1;
      _auxrecvph = st;
      break;
    case BROADCAST_OP:
      st = 1;
      np += 1;
      _auxsendph = _nphases - 1;
      break;
    }
  }
  if(outside)
  {
    /* non-power of two */
    switch(_op)
    {
    case BARRIER_OP:
    case ALLREDUCE_OP:
      st = 0;
      np += 2;
      _auxsendph = 0;
      _auxrecvph = _nphases - 1;
      break;
    case REDUCE_OP:
      st = 0;
      np = 1;
      _auxsendph = 0;
      break;
    case BROADCAST_OP:
      st = _nphases - 1;
      np = 1;
      _auxrecvph = st;
      break;
    }
  }
  else
  {
    /* might be power of two */
    switch(_op)
    {
    case BARRIER_OP:
    case ALLREDUCE_OP:
      _sendph = ALL_PHASES;
      _recvph = ALL_PHASES;
      break;
    case REDUCE_OP:
      if(_mynode == 0) // root
      {
        _recvph = ALL_PHASES;
        _sendph = NO_PHASES;
      }
      else
      {
        np = ffs(_mynode) + 1 - st;
        _sendph = st + np - 1;
        _recvph = NOT_SEND_PHASE;
      }
      break;
    case BROADCAST_OP:
      if(_mynode == 0) // root
      {
        _recvph = NO_PHASES;
        _sendph = ALL_PHASES;
      }
      else
      {
        int n = (_nphbino - ffs(_mynode));
        st += n;
        np -= n;
        _recvph = st;
        _sendph = NOT_RECV_PHASE;
      }
      break;
    }
  }
  startph = st;
  nph = np;
}

/**
 * \brief Initialize the final schedule
 *
 * \param[in] root	The root node (rank)
 * \param[in] comm_op	The collective operation to perform
 * \param[out] start	The starting phase for this node
 * \param[out] nph	The number of phases for this node
 * \param[out] nranks	The largest number of steps per phase
 */
void CCMI::Schedule::BinomialTreeSchedule::
init(int root, int comm_op, int &start, int &nph, int &nranks)
{
  unsigned st, np;

  CCMI_assert(comm_op == BARRIER_OP ||
              comm_op == ALLREDUCE_OP ||
              comm_op == REDUCE_OP ||
              comm_op == BROADCAST_OP);

  _op = comm_op;
  if(comm_op == BARRIER_OP || comm_op == ALLREDUCE_OP)
  {
    _rootindex = 0;
  }
  else
  {
    _rootindex = RANK_TO_IDX(root);
  }
  // can't compute this until we know root
  _mynode = RANK_TO_REL(_myrank);

  setupContext(st, np);

  nph = np;
  start = st;
  nranks = np; // maximum number of recvs total
}

#ifndef __GNUC__
/**
 * For reasons that are not quite clear, the IBM XLC++ compiler will
 * not generate the vtable for this object. It appears to be related
 * to the fact that it is never "new"ed, but always a member of
 * another object.
 *
 * This code (which is never called) does "new" the object, causing
 * XLC++ to put the vtable in this file's resultant object file.
 */
void silly()
{
  char buffer[sizeof(CCMI::Schedule::BinomialTreeSchedule)];
  CCMI::Schedule::BinomialTreeSchedule * p = (CCMI::Schedule::BinomialTreeSchedule *) buffer;
  p = new(p) CCMI::Schedule::BinomialTreeSchedule;
}
#endif
