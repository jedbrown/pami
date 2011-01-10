/*
 * \file algorithms/schedule/MultinomialMap.h
 * \brief ???
 */

#ifndef __algorithms_schedule_MultinomialMap_h__
#define __algorithms_schedule_MultinomialMap_h__

#include "algorithms/interfaces/Schedule.h"

namespace CCMI
{
  namespace Schedule
  {

    ///
    /// \brief The Map interface for the Multinomial schedule. The
    ///        multinomial schedule just operates on ranks 0-N, where
    ///        0-m nodes for a multinomial tree of radix k. The remaining
    ///        nodes are auxilliary nodes that send messages to the
    ///        internal nodes.
    ///
    template <class T> class MultinomialMap
    {
      public:
        MultinomialMap () {}

        void setRoot (unsigned r )
        {
          static_cast<T*>(this)->setRoot(r);
        }

	///
        /// \brief Do we have aux phases in addition to binomial
        ///
        bool hasAuxPhases   ()
        {
          return static_cast<T*>(this)->hasAuxPhases();
        }

        ///
        /// \brief Is the rank an auxilary processor
        /// \param [in] rank the rank of the processor
        ///
        bool isAuxProc   ()
        {
          return static_cast<T*>(this)->isAuxProc();
        }

        ///
        /// \brief Is the rank a peer processor that takes the data from
        ///        the auxillary processor and participates in the
        ///        binomial collective. At the end of the collective
        ///        operation result will be returned to the auxillary
        ///        processor
        /// \param [in] the rank of the processor
        ///
        bool isPeerProc  ()
        {
          return static_cast<T*>(this)->isPeerProc();
        }

        ///
        /// \brief Get the aux processor for this peer processor
        /// \param [in] the rank of the aux processor
        /// \retval the rank of the peer processor
        ///
        void  getAuxForPeer (unsigned *ranks, unsigned &nranks)
        {
          return static_cast<T*>(this)->getAuxForPeer(ranks, nranks);
        }

        ///
        /// \brief Get the rank of the peer processor for the aux processor
        /// \param [in] The rank of the peer processor
        /// \retval the rank of the aux processor
        ///
        unsigned getPeerForAux ()
        {
          return static_cast<T*>(this)->getPeerForAux();
        }

        ///
        /// \brief Convert the rank to the global rank for the msend
        ///        interface
        ///
        unsigned getGlobalRank (unsigned relrank)
        {
          return static_cast<T*>(this)->getGlobalRank(relrank);
        }

        ///
        /// \brief Get the root of the collective
        ///
        unsigned getRoot ()
        {
          return static_cast<T*>(this)->getRoot();
        }

        ///
        /// \brief Get my rank in the collective
        ///
        unsigned getMyRank ()
        {
          return static_cast<T*>(this)->getMyRank();
        }

        unsigned getNumRanks ()
        {
          return static_cast<T*>(this)->getNumRanks();
        }

    };  //Multinomial map

    class TopologyMap : public MultinomialMap<TopologyMap>
    {
      public:
        TopologyMap () {}

        TopologyMap (unsigned myrank, PAMI::Topology *topology) : _myindex(myrank), _topology(topology)
        {
          unsigned nph = 0;

          for (unsigned i = topology->size(); i > 1; i >>= 1)
            {
              nph++;
            }

          _hnranks = 1 << nph;
          _rootindex = 0;
        }

        ~TopologyMap () {}

	bool hasAuxPhases () {
	  size_t size = _topology->size();
	  return ((size & (size-1)) != 0);
	}	  

        void setRoot (unsigned gr)
        {
          _rootindex = _topology->rank2Index((pami_task_t)gr);
        }

        ///
        /// \brief Is the rank an auxilary processor
        ///
        bool isAuxProc   ()
        {
          if (getMyRank() >= _hnranks)
            return true;

          return false;
        }

        ///
        /// \brief Is the rank a peer processor that takes the data from
        ///        the auxillary processor and participates in the
        ///        binomial collective. At the end of the collective
        ///        operation result will be returned to the auxillary
        ///        processor
        ///
        bool isPeerProc  ()
        {
          if (getMyRank() < (_topology->size() - _hnranks))
            return true;

          return false;
        }

        ///
        /// \brief Get the aux processor for this peer processor
        /// \param [in] the rank of the aux processor
        /// \retval the rank of the peer processor
        ///
        void  getAuxForPeer (unsigned *ranks, unsigned &nranks)
        {
	  CCMI_assert (isPeerProc());
	  nranks = 1;
          ranks[0] = getMyRank() + _hnranks;
        }

        ///
        /// \brief Get the rank of the peer processor for the aux processor
        /// \param [in] The rank of the peer processor
        /// \retval the rank of the aux processor
        ///
        unsigned getPeerForAux ()
        {
	  CCMI_assert (isAuxProc());
          return getMyRank() - _hnranks;
        }

        ///
        /// \brief Convert the rank to the global rank for the msend
        ///        interface
        ///
        unsigned getGlobalRank (unsigned relrank)
        {
          relrank += _rootindex;
          return _topology->index2Rank(relrank);
        }

        ///
        /// \brief Get my rank in the collective
        ///
        unsigned getMyRank ()
        {
          if (_myindex >= _rootindex)
            return _myindex - _rootindex;

          return _myindex + _topology->size() - _rootindex;
        }

        unsigned getNumRanks () { return _topology->size(); }

      protected:
        size_t              _hnranks;   /** Nearest power of 2 */
        size_t              _rootindex; /** Index of the root */
        size_t              _myindex;   /** Index of my node */
        PAMI::Topology      * _topology;
    };

    class NodeOptTopoMap : public MultinomialMap<NodeOptTopoMap>
    {
      public:
        NodeOptTopoMap () {}

        NodeOptTopoMap (unsigned myindex, PAMI::Topology *topology) : _myindex(myindex), _topology(topology)
        {
          unsigned nph = 0;
          _topology->subTopologyLocalToMe(&_local_topology);
	  
	  ////////////////--- Begin Build Master Topology ----- ////////////////
	  
	  pami_task_t *tmp_ranks = (pami_task_t *)malloc (topology->size() * sizeof(pami_task_t));

	  if (topology->type() == PAMI_LIST_TOPOLOGY) {
	    pami_task_t *ranks = NULL;
	    topology->rankList(&ranks);
	    
	    PAMI_assert (ranks != NULL);
	    PAMI_assert (topology != NULL);	    
	    memcpy (tmp_ranks, ranks, topology->size() * sizeof(pami_task_t));
	  }
	  else if(topology->type() == PAMI_RANGE_TOPOLOGY) {
	    pami_task_t start, end;
	    topology->rankRange(&start, &end);
	    for (size_t i = 0;  i < topology->size(); i++) {
	      tmp_ranks[i] = start + i;
	    }	   
	  }
	  else 
	    //Unsupported topology
	    CCMI_abort();
	    
	  /// Do an O(N^2) search where the first rank on a node is
	  /// treated as master
	  size_t i = 0, j = 0;
	  for (i = 0; i < topology->size(); i ++) 
	    //Verify that this rank is not peer for a past rank
	    if (tmp_ranks[i] != (pami_task_t)-1)	      
	      for (j = i+1; j < topology->size(); j ++)
		//Verify this rank is not peer of some other node
		if (tmp_ranks[j] != (pami_task_t)-1)
		  if (__global.mapping.isPeer (tmp_ranks[i], tmp_ranks[j]))
		    tmp_ranks[j] = (pami_task_t)-1;
	  
	  size_t n_masters = 0;
	  for (i = 0; i < topology->size(); i ++) 
	    if (tmp_ranks[i] != (pami_task_t)-1)
	      n_masters ++;
	  
	  pami_task_t *masters = (pami_task_t *)malloc (n_masters * sizeof(pami_task_t));
	  n_masters = 0;
	  for (i = 0; i < topology->size(); i ++) 
	    if (tmp_ranks[i] != (pami_task_t)-1) {
	      masters[n_masters ++] = tmp_ranks[i];
	      //fprintf (stderr,"Master Topology member %ld %d\n", n_masters, tmp_ranks[i]);
	    }

	  new (&_master_topology) PAMI::Topology (masters, n_masters);
	  free (tmp_ranks);

	  ////////////////--- End Build Master Topology ----- ////////////////

	  pami_task_t *local_ranks = NULL;
	  _local_topology.rankList(&local_ranks);

	  _myrank = __global.mapping.task();
	  _local_master = local_ranks[0];
	  _master_index = _master_topology.rank2Index(_local_master);
	  //fprintf (stderr, "My rank %ld Local Master %ld Master Index %ld\n", _myrank, _local_master, _master_index);

          for (unsigned i = _master_topology.size(); i > 1; i >>= 1)
          {
	    nph++;
	  }
	  //Current we do not support non powers of two
	  PAMI_assert (_master_topology.size() == (size_t)(1 << nph));	  
	  _hnranks = 1 << nph;
	  _rootindex = 0;
        }
	
        ~NodeOptTopoMap () {}

	bool hasAuxPhases () {
	  return (_master_topology.size() != _topology->size());
	}

        void setRoot (unsigned gr)
        {
	  CCMI_abort(); //Currently, we only support barrier and allreduce
          _rootindex = _topology->rank2Index((pami_task_t)gr);
        }

        ///
        /// \brief Is the rank an auxilary processor
        /// \param [in] rank the rank of the processor
        ///
        bool isAuxProc   ()
        {
          if (_myrank != _local_master)
            return true;

          return false;
        }

        ///
        /// \brief Is the rank a peer processor that takes the data from
        ///        the auxillary processor and participates in the
        ///        binomial collective. At the end of the collective
        ///        operation result will be returned to the auxillary
        ///        processor
        /// \param [in] the rank of the processor
        ///
        bool isPeerProc  ()
        {
          if (_myrank == _local_master && _local_topology.size() > 1)
            return true;
	  
          return false;
        }

        ///
        /// \brief Get the aux processor for this peer processor
        /// \param [in] the rank of the aux processor
        /// \retval the rank of the peer processor
        ///
        void  getAuxForPeer (unsigned *ranks, unsigned &nranks)
        {
	  nranks = 0;
          for (size_t i = 1; i < _local_topology.size(); i++) 
	    //1 ... master_size -1 is for the binomial tree and the rest are peer ranks
	    ranks[nranks ++] = (unsigned)(_master_topology.size() + i);	  
        }

        ///
        /// \brief Get the rank of the peer processor for the aux processor
        /// \param [in] The rank of the peer processor
        /// \retval the rank of the aux processor
        ///
        unsigned  getPeerForAux ()
        {
	  return _master_index;
        }

        ///
        /// \brief Convert the rank to the global rank for the msend
        ///        interface
        ///
        unsigned getGlobalRank (unsigned relrank)
        {
	  unsigned grank = 0;
	  if (relrank < _master_topology.size())
	    grank = _master_topology.index2Rank((size_t)relrank);
	  else
	    grank = _local_topology.index2Rank((size_t)relrank - _master_topology.size());

	  //fprintf(stderr, "Global rank %d = %d\n", relrank, grank);
	  return grank;
        }

        ///
        /// \brief Get my rank in the collective
        ///
        unsigned getMyRank ()
        {
          return _master_index;
        }

        unsigned getNumRanks () { return _master_topology.size(); }

      protected:
        unsigned            _hnranks;   /** Nearest power of 2 */
        unsigned            _rootindex; /** Index of the root */
        unsigned            _myindex;   /** Index of my node */
        size_t              _myrank;    /** Global rank of my node */
	size_t              _master_index;  /** Index of the master task */
	size_t              _local_master;  /*  Global rank of this task's master*/
        PAMI::Topology    * _topology;
	PAMI::Topology      _master_topology;
	PAMI::Topology      _local_topology;
    };
  };
};

#endif
