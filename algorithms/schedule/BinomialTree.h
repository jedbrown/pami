/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/schedule/BinomialTree.h
 * \brief The Binomial Schedule for Barrier, Broadcast, and [All]Reduce.
 *
 * Also used by the Rectangle schedule to implement Reduce.
 */

#ifndef __algorithms_schedule_BinomialTree_h__
#define __algorithms_schedule_BinomialTree_h__
#define HAVE_BINO_LINE_SCHED	// until branch is merged into main

#include "algorithms/interfaces/Schedule.h"
#include "Global.h"
#include <strings.h>  //needed for ffs on AIX
#include "util/ccmi_util.h"
namespace CCMI
{
  namespace Schedule
  {

    //-------------------------------------------------------------
    //------ Supports a generic barrier and allreduce -------------
    //------ The number of phases is logrithmic with a max of 20 --
    //-------------------------------------------------------------
    template <class T_SysDep>
    class BinomialTreeSchedule : public Interfaces::Schedule
    {
      unsigned _nphases;  /// \brief Number of phases total
      unsigned _nphbino;  /// \brief Num of phases in pow-of-2 sec
      short _op;    /// \brief Collective op

      unsigned _myrank; /// \brief my rank, externally
      unsigned _mynode; /// \brief my node index, internally
      unsigned _rootindex;  /// \brief index of root node in ranks[]
      unsigned _nranks; /// \brief Number of ranks[]
      unsigned _rank0;  /// \brief First rank, or "0" if ranks[]
      unsigned _hnranks;  /// \brief Number of ranks in pow-2 sec
      unsigned _pnranks;  /// \brief num pow-2 ranks with peers outside
      unsigned _treemask; /// \brief Mask for wrapping nodes
      unsigned *_ranks; /// \brief ranks[]
      unsigned _sendph; /// \brief sending phase for this node
      unsigned _recvph; /// \brief recving phase for this node
      unsigned _auxsendph;  /// \brief outside send phase
      unsigned _auxrecvph;  /// \brief outside recv phase

      void setupContext(unsigned &start, unsigned &nphases);

      /*
       */

/**
 * @defgroup PHASE_CODES Codes used to represent implicite phases
 *@{
 */
#define UNDEFINED_PHASE	((unsigned)-1)	///< Undefined phase
#define ALL_PHASES	((unsigned)-2)	///< All phases in bino
#define NO_PHASES	((unsigned)-3)	///< No phase
#define NOT_RECV_PHASE	((unsigned)-4)	///< All except recv phases
#define NOT_SEND_PHASE	NOT_RECV_PHASE	///< All except send phases
/*@}*/

/**
 * \brief Test whether node is "outside" the power-of-two set
 * \param[in] node	Node index to test, relative to root
 * \return	true or false
 */
#define IS_OUTSIDE(node)	((node) >= _hnranks)
/**
 * \brief Convert a peer node to its "outside" equivalent
 * \param[in] node	Node index to convert, relative to root
 * \return	Converted node
 */
#define TO_OUTSIDE(node)	((node) + _hnranks)
/**
 * \brief Test whether node is a peer to the "outside" nodes
 * \param[in] node	Node index to test, relative to root
 * \return	true or false
 */
#define IS_PEER(node)		((node) < _pnranks)
/**
 * \brief Convert an "outside" node to its peer
 * \param[in] node	Node index to convert, relative to root
 * \return	Converted node
 */
#define TO_PEER(node)		((node) - _hnranks)

/**
 * \brief Convert a node to its "parent" in phase
 * \param[in] node	Node to convert, relative to root
 * \param[in] phase	Phase of algorithm
 * \return	Converted node
 */
#define TO_PARENT(node, phase)	\
        (((node) - (1 << (phase))) & _treemask)
/**
 * \brief Convert a node to its "child" in phase
 * \param[in] node	Node to convert, relative to root
 * \param[in] phase	Phase of algorithm
 * \return	Converted node
 */
#define TO_CHILD(node, phase)	\
        (((node) + (1 << (phase))) & _treemask)

      /**
       * \brief Convert a rank to _ranks[] index
       * \param[in] rank	Rank as used in communications
       * \return	Node index, position in _ranks[]
       */
      inline unsigned RANK_TO_IDX(unsigned rank)
      {
        unsigned i;
        if(_ranks)
        {
          for(i = 0; i < _nranks && _ranks[i] != rank; i++)
          CCMI_assert(i < _nranks);
        }
        else
        {
          i = rank - _rank0;
        }
        return i;
      }

      /**
       * \brief Convert a rank to relative-to-root index
       * \param[in] rank	Rank as used in communications
       * \return	Node index, relative to root
       */
      inline unsigned RANK_TO_REL(unsigned rank)
      {
        unsigned i = RANK_TO_IDX(rank);
        if(i >= _rootindex)
        {
          i -= _rootindex;
        }
        else
        {
          i += _nranks - _rootindex;
        }
        return i;
      }

      /**
       * \brief Convert a relative-to-root index into a rank
       * \param[in] node	Node index, relative to root
       * \return	Rank for use in communications
       */
      inline unsigned REL_TO_RANK(unsigned node)
      {
        node += _rootindex;
        if(node >= _nranks) node -= _nranks;
        return(_ranks ? _ranks[node] : node + _rank0);
      }

#define NODE_PARENT	true	///< Use node's parent
#define NODE_CHILD	false	///< Use node's child

      /**
       * \brief Get next node in schedule
       *
       * Returns the node we should communicate with in this phase
       *
       * \param[in] parent	whether to go left or right...
       * \param[in] ph	phase to work on
       * \return	Next node in algorithm
       */
      inline unsigned NEXT_NODE(bool parent, unsigned ph)
      {
        unsigned nod;
        nod = _mynode;
        if(ph == _auxsendph || ph == _auxrecvph)
        {
          // This phase is only for communication
          // between outside and peer. Don't care
          // about 'parent' here - simply flip to
          // other side of what we are.
          if(IS_OUTSIDE(nod))
          {
            nod = TO_PEER(nod);
          }
          else if(IS_PEER(nod))
          {
            nod = TO_OUTSIDE(nod);
          }
        }
        else
        {
          // Binomial part of operation. All of these
          // phases involve a power-of-two set of nodes.
          // Phase numbers start at one here, but the
          // algorithm requires zero.
          ph -= 1;
          if(_op == BROADCAST_OP)
          {
            // Broadcast is a reflection of reduce.
            // invert the 'binomial' phase and
            // concept of parent.
            ph = _nphbino - ph - 1;
            parent = !parent;
          }
          if(parent)
          {
            nod = TO_PARENT(nod, ph);
          }
          else // child
          {
            nod = TO_CHILD(nod, ph);
          }
        }
        return nod;
      }

      /**
       * \brief Create Binomial Schedule for a simple range of indices (or coords on a line)
       *
       * Also called from Rectangle.h when doing a reduce along an axis of the rectangle.
       *
       * \param[in] x		Our coord on axis
       * \param[in] x0	Lowest coord on axis
       * \param[in] xN	Highest coord on axis
       * \param[in] ranks	(optional) Pointer to list of ranks (saved until later)
       * \return	nothing
       */
      inline void initBinoSched(unsigned x, unsigned x0, unsigned xN,
                                unsigned *ranks = NULL)
      {
        _rank0 = x0;
        _nranks = xN - x0 + 1;
        _ranks = ranks;
        _op = -1;

        /* find my index - my place in rank list */
        _myrank = x;

        _nphases = getMaxPhases(NULL, _nranks, &_nphbino);
        _hnranks = (1 << _nphbino); // threshold for special handling
        _pnranks = (_nranks - _hnranks);  // might be zero
        _treemask = _hnranks - 1;
      }

    public:

      static unsigned getMaxPhases(T_SysDep *sysdep, unsigned nranks,
                                   unsigned *nbino = NULL)
      {
        unsigned nph;

        /* figure out the number of phases */
        nph = 0;
        if(nranks)
        {
          for(unsigned i = nranks; i > 1; i >>= 1)
          {
            nph++;
          }
        }
        if(nbino)
        {
          *nbino = nph;
        }
        nph += 1; // power of two starts on phase 1 anyway.
        if((nranks & (nranks - 1)) != 0)
        {
          nph += 1; // non-power of two adds phase 0 and phase N+1
        }
        return nph;
      }

      /**
       * \brief Constructor for Binomial Schedule
       */
      BinomialTreeSchedule () :
      Schedule (),
      _nphases(0)
      {
      }

      /**
       * \brief Constructor for list of ranks
       *
       * \param[in] sysdep	Simple sysdep for ranks
       * \param[in] nranks	Number of ranks in list
       * \param[in] ranks	Ranks list
       */
      BinomialTreeSchedule(T_SysDep *sysdep, unsigned nranks,
                           unsigned *ranks);

      /**
       * \brief Constructor for line (simple coords)
       *
       * \param[in] me	My coord on line
       * \param[in] min	Minimum coord on line
       * \param[in] max	Maximum coord on line
       */
      BinomialTreeSchedule(unsigned me, unsigned min, unsigned max);

      /**
       * \brief Initialize Binomial Schedule
       *
       * \param[in] root		Root node of collective
       * \param[in] comm_op		Collective operation to do
       * \param[out] startphase	Starting phase for this node
       * \param[out] nphases		Number of phases for this node
       * \param[out] nranks		Max num steps per phase
       */
      virtual void init(int root, int comm_op, int &startphase,
                        int &nphases, int &nranks);

      /**
       * \brief Get source phase steps
       *
       * \param[in] phase	Phase to work on
       * \param[out] dstpes	Rank(s) to receive from
       * \param[out] ndst	Number of steps (ranks)
       * \param[out] subtasks	Operations to perform with ranks
       */
      virtual void getSrcPeList(unsigned phase, unsigned *srcpes,
                                unsigned &nsrc, unsigned *subtasks=NULL)
      {
        unsigned src = _nranks;
        if((phase >= 1 && phase <= _nphbino && (_recvph == ALL_PHASES ||
                                                (_recvph == NOT_SEND_PHASE && phase != _sendph) ||
                                                phase == _recvph)) || phase == _auxrecvph)
        {
          src = NEXT_NODE(NODE_PARENT, phase);
        }
        if(src < _nranks)
        {
          *srcpes = REL_TO_RANK(src);
          if(subtasks)
          {
            if(_mynode >= _hnranks ||
               _op == BROADCAST_OP ||
               _op == BARRIER_OP)
            {
              *subtasks = PAMI_PT_TO_PT_SUBTASK;
            }
            else
            {
              *subtasks = PAMI_COMBINE_SUBTASK;
            }
          }
          nsrc = 1;
        }
        else
        {
          nsrc = 0;
        }
      }

      /**
       * \brief Get destination phase steps
       *
       * \param[in] phase	Phase to work on
       * \param[out] dstpes	Rank(s) to send to
       * \param[out] ndst	Number of steps (ranks)
       * \param[out] subtasks	Operations to perform to ranks
       */
      virtual void getDstPeList(unsigned phase, unsigned *dstpes,
                                unsigned &ndst, unsigned *subtasks)
      {
        unsigned dst = _nranks;
        if((phase >= 1 && phase <= _nphbino && (_sendph == ALL_PHASES ||
                                                (_sendph == NOT_RECV_PHASE && phase != _recvph) ||
                                                phase == _sendph)) || phase == _auxsendph)
        {
          dst = NEXT_NODE(NODE_CHILD, phase);
        }
        if(dst < _nranks)
        {
          *dstpes = REL_TO_RANK(dst);
          *subtasks = PAMI_PT_TO_PT_SUBTASK;
          ndst = 1;
        }
        else
        {
          ndst = 0;
        }
      }
      virtual void
      init(int root, int op, int &startphase, int &nphases) { PAMI_abort();}
      virtual void
      getSrcTopology (unsigned phase, PAMI::Topology *topology){ PAMI_abort();}
      virtual void
      getDstTopology (unsigned phase, PAMI::Topology *topology){ PAMI_abort();}
      virtual pami_result_t
      getSrcUnionTopology (PAMI::Topology *topology){ PAMI_abort(); return PAMI_SUCCESS;}
      virtual pami_result_t
      getDstUnionTopology (PAMI::Topology *topology){ PAMI_abort(); return PAMI_SUCCESS;}


    };    //Binomial Tree Schedule
  };   //Schedule
}; //CCMI


//--------------------------------------------------------------
//------  Binomial Schedule Functions -------------------------
//--------------------------------------------------------------

/**
 * \brief Constructor for list of ranks
 *
 * \param[in] sysdep	Simple sysdep for ranks
 * \param[in] nranks	Number of ranks in list
 * \param[in] ranks	Ranks list
 */
template <class T_SysDep>
inline CCMI::Schedule::BinomialTreeSchedule<T_SysDep>::
BinomialTreeSchedule(T_SysDep *sysdep, unsigned nranks, unsigned *ranks) : Schedule()
{

  CCMI_assert(nranks > 0);
  CCMI_assert(ranks != NULL);

  /* find my index - my place in rank list */
  unsigned rank = __global.mapping.task();
  CCMI::Schedule::BinomialTreeSchedule<T_SysDep>::
  initBinoSched(rank, 0, nranks - 1, ranks);
}

/**
 * \brief Constructor for line (simple coords)
 *
 * \param[in] x		My coord on line
 * \param[in] x0	Minimum coord on line
 * \param[in] xN	Maximum coord on line
 */
template <class T_SysDep>
inline CCMI::Schedule::BinomialTreeSchedule<T_SysDep>::
BinomialTreeSchedule(unsigned x, unsigned x0, unsigned xN) : Schedule()
{

  CCMI::Schedule::BinomialTreeSchedule<T_SysDep>::
  initBinoSched(x, x0, xN);
}
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
template <class T_SysDep>
inline void CCMI::Schedule::BinomialTreeSchedule<T_SysDep>::
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
template <class T_SysDep>
inline void CCMI::Schedule::BinomialTreeSchedule<T_SysDep>::
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

#endif /* !__binomial_tree_schedule__ */
