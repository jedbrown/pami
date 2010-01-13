/*
 * \file algorithms/schedule/MultinomialTree.h
 * \brief ???
 */

#ifndef __algorithms_schedule_MultinomialTree_h__
#define __algorithms_schedule_MultinomialTree_h__

#include "algorithms/interfaces/Schedule.h"
#include "MultinomialMap.h"

namespace CCMI
{
  namespace Schedule
  {

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
 * \brief Convert a node to its peer in the binomial tree
 * \param[in] node	Node to convert, relative to root
 * \param[in] phase	Phase of algorithm
 * \return	Converted node
 */
#define BINO(nodes, nranks, mynode, phase, nph, radix, lradix)	\
    nranks = 0;							\
    for (unsigned r=1; r < radix; r ++)				\
      nodes[nranks++] = (mynode ^ (r << (phase * lradix)));

    //-------------------------------------------------------------
    //------ Supports a generic barrier and allreduce -------------
    //------ The number of phases is logrithmic with a max of 20 --
    //-------------------------------------------------------------
    template <class M> class MultinomialTreeT : public Interfaces::Schedule
    {
    protected:

      void setupContext(unsigned &start, unsigned &nphases);

      /**
       * \brief Get next node in schedule
       *
       * Returns the node we should communicate with in this phase
       *
       * \param[in] parent	whether to go left or right...
       * \param[in] ph	phase to work on
       * \return	Next node in algorithm
       */
      inline void NEXT_NODES(bool parent, unsigned ph, unsigned *nodes, unsigned &nranks)
      {
	// This phase is only for communication
	// between outside and peer. Don't care
	// about 'parent' here - simply flip to
	// other side of what we are.
        if( ((ph == _auxsendph) && !parent) ||
	    ((ph == _auxrecvph) &&  parent) )
        {
	  if (_map.isAuxProc(_map.getMyRank())) {
	    nodes[0] = _map.getPeerForAux(_map.getMyRank());
	    nranks = 1;
	  }
	  else if (_map.isPeerProc(_map.getMyRank())) {
	    nodes[0] = _map.getAuxForPeer(_map.getMyRank());
	    nranks = 1;
	  }
	}
        else if (ph != _auxrecvph && ph != _auxsendph)
        {
          // Multinomial part of operation. All of these
          // phases involve a power-of-two set of nodes.
          // Phase numbers start at one here, but the
          // algorithm requires zero.
          ph -= 1;

	  BINO(nodes, nranks, _map.getMyRank(), ph, _nphbino, _radix, _logradix);
	  CCMI_assert(nranks >= 1);
        }

        if (nranks > 0)
          TRACE_ERR ((stderr, "%d: phase %d, node %d\n", _map.getMyRank(), ph, nodes[0]));
      }

      /**
       * \brief Create Multinomial Schedule for a simple range of indices (or coords on a line)
       *
       * Also called from Rectangle.h when doing a reduce along an axis of the rectangle.
       *
       * \param[in] x		Our coord on axis
       * \param[in] x0	Lowest coord on axis
       * \param[in] xN	Highest coord on axis
       * \param[in] ranks	(optional) Pointer to list of ranks (saved until later)
       * \return	nothing
       */
      inline void initBinoSched()
      {
	_nranks = _map.getNumRanks();
        _op = -1;

	_radix = getRadix (_nranks);
	_logradix = 1;

	if (_radix == 8)
	  _logradix = 3;
	else if (_radix == 4)
	  _logradix = 2;

        _maxphases = getMaxPhases(_nranks, &_nphbino);
        _hnranks = (1 << (_nphbino * _logradix)); // threshold for special handling
      }

    public:

      ///
      /// \brief Enable a higher radix binomial algorithm
      ///
      static unsigned getRadix (unsigned nranks) {
	int nph = 0;
	int radix = 2;

	for(unsigned i = nranks; i > 1; i >>= 1) {
	  nph++;
	}

	//Capping the number of phases
	if ((nranks <= 4096) && (nph % 3) == 0)  //multiple of 3
	  radix = 8;
	else if ((nph & 1) == 0) //multiple of 2
	  radix = 4;

	return radix;
      }

      static unsigned getMaxPhases(unsigned nranks, unsigned *nbino = NULL)
      {
        unsigned nph;
	unsigned radix = getRadix (nranks);

        /* figure out the number of phases */
        nph = 0;
        if(nranks)
        {
	  for(unsigned i = nranks; i > 1; i >>= 1) {
	    nph++;
	  }

	  if (radix == 8)
	    nph /= 3;
	  else if (radix == 4)
	    nph /= 2;
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
       * \brief Constructor for Multinomial Schedule
       */
      MultinomialTreeT () :
      Schedule (),
      _maxphases(0)
      {
      }

      /**
       * \brief Constructor for list of ranks
       *
       * \param[in] nranks	Number of ranks in list
       * \param[in] ranks	Ranks list
       */
      MultinomialTreeT (unsigned myrank, XMI::Topology *topo, unsigned c=0);


      /**
       * \brief Constructor for list of ranks
       *
       * \param[in] ranks	Ranks list
       * \param[in] nranks	Number of ranks in list
       */
      MultinomialTreeT (unsigned myrank, size_t *ranks, unsigned nranks);

      /**
       * \brief Initialize Multinomial Schedule
       *
       * \param[in] root		Root node of collective
       * \param[in] comm_op		Collective operation to do
       * \param[out] startphase	Starting phase for this node
       * \param[out] nphases		Number of phases for this node
       * \param[out] nranks		Max num steps per phase
       */
      virtual void init(int root, int comm_op, int &startphase,
                        int &nphases);

      /**
       * \brief Get source phase steps
       *
       * \param[in] phase	Phase to work on
       * \param[INOUT] srcranks are filled in the topology
       */
      virtual void getSrcTopology(unsigned phase, XMI::Topology *topology)
      {
	unsigned *srcranks;
	xmi_result_t rc = topology->rankList(&srcranks);
	CCMI_assert (rc == XMI_SUCCESS);
	CCMI_assert(srcranks != NULL);

	unsigned nsrc = 0;
        if((phase >= 1 && phase <= _nphbino && (_recvph == ALL_PHASES ||
                                                (_recvph == NOT_SEND_PHASE && phase != _sendph) ||
                                                phase == _recvph)) || phase == _auxrecvph)
        {
          NEXT_NODES(PARENT, phase, srcranks, nsrc);
        }
	CCMI_assert (nsrc <= topology->size());

	for (unsigned count = 0; count < nsrc; count ++) {
	  srcranks[count] = _map.getGlobalRank(srcranks[count]);
	}

	//Convert to a list topology
	new (topology) XMI::Topology (srcranks, nsrc);
      }

      /**
       * \brief Get destination phase steps
       *
       * \param[in] phase	Phase to work on
       * \param[INOUT] dstranks are filled in the topology
       */
      virtual void getDstTopology(unsigned phase, XMI::Topology *topology)
      {
	unsigned *dstranks;
	xmi_result_t rc = topology->rankList(&dstranks);
	CCMI_assert (rc == XMI_SUCCESS);
	CCMI_assert(dstranks != NULL);

	unsigned ndst = 0;
        if((phase >= 1 && phase <= _nphbino && (_sendph == ALL_PHASES ||
                                                (_sendph == NOT_RECV_PHASE && phase != _recvph) ||
                                                phase == _sendph)) || phase == _auxsendph)
        {
          NEXT_NODES(CHILD, phase, dstranks, ndst);
        }

	CCMI_assert (ndst <= topology->size());
	for (unsigned count = 0; count < ndst; count ++)
	{
	  dstranks[count]   = _map.getGlobalRank(dstranks[count]);
	  TRACE_ERR ((stderr, "%d: phase %d, index %d node %d\n", _map.getMyRank(),phase,count,dstranks[count]));
	}

	//Convert to a list topology of the accurate size
	new (topology) XMI::Topology (dstranks, ndst);
      }

      /**
       * \brief Get the union of all sources across all phases
       * \param[INOUT] topology : the union of all sources
       */
      virtual void getSrcUnionTopology (XMI::Topology *topology) {
	unsigned *srcranks;
	xmi_result_t rc = topology->rankList(&srcranks);
	CCMI_assert (rc == XMI_SUCCESS);
	CCMI_assert(srcranks != NULL);

	unsigned ntotal_src = 0;
	for (unsigned phase = _startphase; phase < _startphase + _nphases; phase++) {
	  unsigned nsrc = 0;
	  if((phase >= 1 && phase <= _nphbino && (_recvph == ALL_PHASES ||
						  (_recvph == NOT_SEND_PHASE && phase != _sendph) ||
						  phase == _recvph)) || phase == _auxrecvph)
	  {
	    NEXT_NODES(PARENT, phase, srcranks + ntotal_src, nsrc);
	    ntotal_src += nsrc;
	  }
	  CCMI_assert (ntotal_src <= topology->size());
	}

	for (unsigned count = 0; count < ntotal_src; count ++) {
	  srcranks[count] = _map.getGlobalRank(srcranks[count]);
	}

	//Convert to a list topology
	new (topology) XMI::Topology (srcranks, ntotal_src);
      }

      /**
       * \brief Get the union of all destinations across all phases
       * \param[INOUT] topology : the union of all sources
       */
      virtual void getDstUnionTopology (XMI::Topology *topology) {
	unsigned *dstranks;
	xmi_result_t rc = topology->rankList(&dstranks);
	CCMI_assert (rc == XMI_SUCCESS);
	CCMI_assert(dstranks != NULL);

	unsigned ntotal_dst = 0;
	for (unsigned phase = _startphase; phase < _startphase + _nphases; phase++) {
	  unsigned ndst = 0;
	  if((phase >= 1 && phase <= _nphbino && (_sendph == ALL_PHASES ||
						  (_sendph == NOT_RECV_PHASE && phase != _recvph) ||
						  phase == _sendph)) || phase == _auxsendph)
	  {
	    NEXT_NODES(CHILD, phase, dstranks + ntotal_dst, ndst);
	  }
	  ntotal_dst += ndst;
	  CCMI_assert (ntotal_dst <= topology->size());
	}

	for (unsigned count = 0; count < ntotal_dst; count ++) {
	  dstranks[count]   = _map.getGlobalRank(dstranks[count]);
	  TRACE_ERR ((stderr, "%d: phase %d, index %d node %d\n", _map.getMyRank(),phase,count,dstranks[count]));
	}

	//Convert to a list topology of the accurate size
	new (topology) XMI::Topology (dstranks, ntotal_dst);
      }

    protected:
      unsigned     _maxphases;  /// \brief Number of phases total
      unsigned     _nphbino;    /// \brief Num of phases in pow-of-2 sec
      unsigned     _op;         /// \brief Collective op
      unsigned     _radix;      /// \brief Radix of the collective operation (default 2)
      unsigned     _logradix;   /// \brief Log of radix of the collective operation (default 1)
      unsigned     _nranks;     /// \brief Number of ranks[]
      unsigned     _hnranks;    /// \brief Number of ranks in pow-2 sec
      unsigned     _sendph;     /// \brief sending phase for this node
      unsigned     _recvph;     /// \brief recving phase for this node
      unsigned     _auxsendph;  /// \brief outside send phase
      unsigned     _auxrecvph;  /// \brief outside recv phase
      unsigned     _startphase; /// \brief the start phase after init is called
      unsigned     _nphases;    /// \brief the total number of phases after init is called

      XMI::Topology     _topology;  /// \brief goes away when geoemetries store topologies
      M            _map;

      static const bool PARENT = true;
      static const bool CHILD  = false;

    };    //Multinomial Tree Schedule

    typedef MultinomialTreeT<LinearMap>  LinearMultinomial;
    typedef MultinomialTreeT<ListMap> ListMultinomial;
  };   //Schedule
}; //CCMI


//--------------------------------------------------------------
//------  Multinomial Schedule Functions -------------------------
//--------------------------------------------------------------

/**
 * \brief Constructor for list of ranks
 *
 * \param[in] myrank	My rank in COMM_WORLD
 * \param[in] topology	topology across which the binomial must be performed
 */
template <class M>
inline CCMI::Schedule::MultinomialTreeT<M>::
MultinomialTreeT(unsigned myrank, XMI::Topology *topology, unsigned c):_map(myrank, topology)
{
  initBinoSched();
}


/**
 * \brief Constructor for list of ranks
 *
 * \param[in] myrank	My rank in COMM_WORLD
 * \param[in] nranks	Number of ranks in list
 * \param[in] ranks	Ranks list
 */
template <class M>
inline CCMI::Schedule::MultinomialTreeT<M>::
MultinomialTreeT(unsigned myrank, size_t *ranks, unsigned nranks):_topology(ranks, nranks), _map()
{
  CCMI_assert (_topology.type() == XMI_LIST_TOPOLOGY);

  new (&_map) M (myrank, &_topology);
  initBinoSched();
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
template <class M>
void CCMI::Schedule::MultinomialTreeT<M>::
setupContext(unsigned &startph, unsigned &nph)
{
  unsigned st, np;

  _recvph = NO_PHASES;
  _sendph = NO_PHASES;

  // Assume (default) we are only performing as a
  // power-of-two (standard binomial).
  st = 1;
  np = _nphbino;
  _auxrecvph = NO_PHASES;
  _auxsendph = NO_PHASES;

  if(_map.isPeerProc(_map.getMyRank()))
  {
    /* non-power of two */
    switch(_op)
    {
    case BARRIER_OP:
    case ALLREDUCE_OP:
      st = 0;
      np += 2;
      _auxrecvph = st;
      _auxsendph = _maxphases - 1;
      break;
    case REDUCE_OP:
      st = 0;
      np += 1;
      _auxrecvph = st;
      break;
    case BROADCAST_OP:
      st = 1;
      np += 1;
      _auxsendph = _maxphases - 1;
      break;
    }
  }

  if(_map.isAuxProc(_map.getMyRank()))
  {
    /* non-power of two */
    switch(_op)
    {
    case BARRIER_OP:
    case ALLREDUCE_OP:
      st = 0;
      np += 2;
      _auxsendph = 0;
      _auxrecvph = _maxphases - 1;
      break;
    case REDUCE_OP:
      st = 0;
      np = 1;
      _auxsendph = 0;
      break;
    case BROADCAST_OP:
      st = _maxphases - 1;
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
      if(_map.getMyRank() == 0) // root
      {
        _recvph = ALL_PHASES;
        _sendph = NO_PHASES;
      }
      else
      {
        //np = ffs(_map.getMyRank()) + 1 - st;
        //_sendph = st + np - 1;

	int distance = _map.getMyRank() + 1;
	int n = 0, d = 1;
	//We are computing Log(distance)
	while (d < distance) {
	  d *= _radix; //radix of the collective
	  n ++;
	}

	np -= n;  //np - n -1 ?
	_sendph = st + np - 1;
        _recvph = NOT_SEND_PHASE;
      }
      break;
    case BROADCAST_OP:
      if(_map.getMyRank() == 0) // root
      {
        _recvph = NO_PHASES;
        _sendph = ALL_PHASES;
      }
      else
      {
        //int n = (_nphbino - ffs(_map.getMyRank()));
	int distance = _map.getMyRank() + 1;
	int n = 0, d = 1;
	//We are computing Log(distance)
	while (d < distance) {
	  d *= _radix; //radix of the collective
	  n ++;
	}
	n --; //first phase is the recv phase
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
template <class M>
void CCMI::Schedule::MultinomialTreeT<M>::
init(int root, int comm_op, int &start, int &nph)
{
  CCMI_assert(comm_op == BARRIER_OP ||
              comm_op == ALLREDUCE_OP ||
              comm_op == REDUCE_OP ||
              comm_op == BROADCAST_OP);

  _op = comm_op;

  if (comm_op == REDUCE_OP ||
      comm_op == BROADCAST_OP)
    _map.setRoot (root);

  setupContext(_startphase, _nphases);

  nph   = _nphases;
  start = _startphase;
}


#endif /* !__multinomial_tree_schedule__ */
