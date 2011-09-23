/*
 * \file algorithms/executor/ScheduleCache.h
 * \brief ???
 */

#ifndef __algorithms_executor_ScheduleCache_h__
#define __algorithms_executor_ScheduleCache_h__

#include "algorithms/interfaces/Schedule.h"
#include "Topology.h"

namespace CCMI
{
  namespace Executor
  {
    ///\brief The ScheduleCache caches the schedule so that it does
    ///not have to be read in every call to the collective.
    class ScheduleCache
    {
      protected:
        unsigned          _start;      //Start phase
        unsigned          _nphases;    //Number of phases

        unsigned          _ntotalsrcranks;   //total number of source ranks across all phases
        unsigned          _ntotaldstranks;  //total number of destintation ranks across all phases

	unsigned          _lastCombinePhase; //The last phase that
					     //receives data before
					     //the lastReducePhase

	unsigned         *_nextActivePhaseVec; //The next phase that has
	                                       //either a send or a
	                                       //receive

        ///A vector of source topologies with one for each phase
        PAMI::Topology          ** _srctopologies;

        ///A vector of destination topologies with one of each phase
        PAMI::Topology          ** _dsttopologies;

        PAMI::Topology             _srcuniontopology; //The uninon of all sources
        PAMI::Topology             _dstuniontopology; //The union of all destinations

        pami_task_t             * _srcranks;  //Cache buffer of source ranks
        pami_task_t             * _dstranks;  //Cache buffer of destination ranks

        ///Buffer to cache the schedule
        char                   * _cachebuf;
        ///Size of the cache buffer
        unsigned                 _cachesize;

        //The root of broadcast and reduce collectives
        int                      _oldroot;
        int                      _root;
        //The collective operation (barrier, broadcast, allreduce, reduce)
        unsigned             _operation;
        Interfaces::Schedule * _schedule;

        static const size_t SC_MAXRANKS  = 128;

        ///Allocate routing that allocates memory for src phases and dst
        ///phases
        void allocate (unsigned nphases, unsigned nsrc, unsigned ndst);

      public:
        ScheduleCache () :
            _cachebuf(NULL),
            _cachesize(0),
            _oldroot(-2),
            _root(-1),
            _operation((unsigned) - 1),
            _schedule(NULL)
        {
        }

        virtual ~ScheduleCache() { if (_cachebuf) CCMI_Free (_cachebuf); }

        void operator delete(void * p)
        {
          CCMI_abort();
        }

        bool init(int op)
        {
          if (_oldroot == _root)
            return false;

          _operation = op;
          _oldroot      = _root;

          int start, nph;
	  _nextActivePhaseVec = NULL;
          _schedule->init (_root, _operation, start, nph);
          _start = start;
          _nphases = nph;
	  _lastCombinePhase = (unsigned)-1;

          TRACE_SCHEDULE((stderr, "<%p>Executor::Barrier::ScheduleCache::init _start %d, nph %d\n",
                        this, _start, _nphases));

          unsigned ntotal_src = 0, ntotal_dst = 0, count = 0;
	  unsigned lastReducePhase = getLastReducePhase();

          for (count = _start; count < (_start + _nphases); count ++)
            {
              pami_task_t srcranks[SC_MAXRANKS], dstranks[SC_MAXRANKS];
              PAMI::Topology src_topology(srcranks, SC_MAXRANKS);
              PAMI::Topology dst_topology(dstranks, SC_MAXRANKS);

              _schedule->getSrcTopology(count, &src_topology);
              ntotal_src += src_topology.size();

	      //Find the last phase that receives data before the
	      //reductions are over
	      if (src_topology.size() > 0 && count <= lastReducePhase)
		_lastCombinePhase = count;

              _schedule->getDstTopology(count, &dst_topology);
              ntotal_dst += dst_topology.size();

              TRACE_SCHEDULE((stderr, "Schedule Cache take_1 phase %d ndst %zu dstrank %u/%u\n", count,
                              dst_topology.size(), dst_topology.index2Rank(0),  dstranks[0]));
            }

          _ntotalsrcranks = ntotal_src;
          _ntotaldstranks = ntotal_dst;

          allocate (_start + _nphases, ntotal_src, ntotal_dst);

          unsigned srcindex = 0, dstindex = 0;

          for (count = _start; count < (_start + _nphases); count ++)
            {
              TRACE_SCHEDULE((stderr, "Schedule Cache : construct topology of size src %d dst %d\n", ntotal_src - srcindex, ntotal_dst - dstindex));

              new (_srctopologies[count]) PAMI::Topology (_srcranks + srcindex, ntotal_src - srcindex);
              new (_dsttopologies[count]) PAMI::Topology (_dstranks + dstindex, ntotal_dst - dstindex);

              _schedule->getSrcTopology(count, _srctopologies[count]);
              _schedule->getDstTopology(count, _dsttopologies[count]);
              srcindex += _srctopologies[count]->size();
              dstindex += _dsttopologies[count]->size();
            }
	  
	  //Build the next active phase list
	  unsigned endphase = _start + _nphases - 1;
	  unsigned next_active = endphase;	  	  
	  int p = 0;
	  for (p = (int)endphase; p >= (int)_start; p--) {
	    _nextActivePhaseVec[p] = next_active;
	    if (getNumSrcRanks(p) > 0 || getNumDstRanks(p) > 0) 
	      next_active = p; //Use current phase as active phase
	                       //for previous phases
	  }
	  
          //_schedule->getSrcUnionTopology(&_srcuniontopology);
          //_schedule->getDstUnionTopology(&_dstuniontopology);

#ifdef CCMI_DEBUG_SCHEDULE

//#warning CCMI DEBUG SCHEDULE
          for ( count = _start; count < (_start + _nphases); count ++)
            if (getSrcTopology(count)->size() > 0)
              {
                pami_task_t *srcranks = 0;
                getSrcTopology(count)->rankList(&srcranks);
                TRACE_SCHEDULE((stderr, "Schedule Cache take_2 phase %d nsrc %zu srcrank %d\n", count, _srctopologies[count]->size(), srcranks[0]));
              }

          for ( count = _start; count < (_start + _nphases); count ++)
            if (getDstTopology(count)->size() > 0)
              {
                pami_task_t *dstranks = 0;
                getDstTopology(count)->rankList(&dstranks);
                TRACE_SCHEDULE ((stderr, "Schedule Cache take_2 phase %d ndst %zu dstrank %d\n", count, _dsttopologies[count]->size(), dstranks[0]));
              }

#endif
          return true;
        }

        unsigned getNumTotalSrcRanks() { return _ntotalsrcranks; }
        unsigned getNumTotalDstRanks() { return _ntotaldstranks; }

        unsigned getNumSrcRanks(unsigned p) { return getSrcTopology(p)->size(); }
        unsigned getNumDstRanks(unsigned p) { return getDstTopology(p)->size(); }

        PAMI::Topology  *getSrcTopology (unsigned phase)
        {
          //if ((phase < _start) || (phase >= _start + _nphases))
	  //fprintf(stderr, "<%p>phase not in range %d, %d, %d\n", this, phase, _start, _start + _nphases);
          CCMI_assert ((phase >= _start) && (phase < _start + _nphases));
          return _srctopologies[phase];
        }

        PAMI::Topology  *getDstTopology (unsigned phase)
        {
          CCMI_assert ((phase >= _start) && (phase < _start + _nphases));
          return _dsttopologies[phase];
        }

        unsigned  getStartPhase()
        {
          return _start;
        }

        unsigned  getNumPhases()
        {
          return _nphases;
        }

        unsigned  getEndPhase ()
        {
          return _start + _nphases - 1;
        }

	unsigned getLastCombinePhase() {
	  return _lastCombinePhase;  //-1U if no data is combined on this rank
	}

        unsigned  getLastReducePhase()
        {
          unsigned p = _schedule->getLastReducePhase();
          // If the schedule doesn't support it (-1), hardcode to end phase
          return p==-1U?_start + _nphases - 1:p;
        }

	unsigned getNextActivePhase(unsigned p) {
	  CCMI_assert ((p >= _start) && (p < _start + _nphases));
	  return _nextActivePhaseVec[p];
	}

        void setRoot(int root)
        {
          _root = root;
        }

        int  getRoot()
        {
          return _root;
        }

        void setSchedule(Interfaces::Schedule *schedule)
        {
          _schedule = schedule;
        }

        Interfaces::Schedule   *getSchedule() { return _schedule; }
    };
  };
};

///
/// \brief Allocate buffer to cache the schedule
///
inline void CCMI::Executor::ScheduleCache::allocate
(unsigned             nphases,
 unsigned             nsrc,
 unsigned             ndst)
{
  //Compute space for Topology pointers and topologies, srcranks and dstranks,
  //and next active phase vector
  unsigned buf_size = 2 * (sizeof(PAMI::Topology *) + sizeof(pami_topology_t)) * nphases + (nsrc + ndst) * sizeof(pami_task_t) + nphases * sizeof(int);

  if (_cachesize < buf_size)
    {
      if (_cachebuf != NULL)
        CCMI_Free (_cachebuf);

      CCMI_Alloc (_cachebuf, buf_size);
      _cachesize = buf_size;

      memset (_cachebuf, 0, _cachesize);
    }

  unsigned offset = 0, count = 0;
  _srctopologies =  (PAMI::Topology **)(_cachebuf + offset);
  offset += nphases * sizeof(PAMI::Topology *);

  for (count = 0; count < nphases; count ++)
    {
      _srctopologies[count] = (PAMI::Topology *)(_cachebuf + offset);
      offset += sizeof(pami_topology_t);
    }

  _dsttopologies =  (PAMI::Topology **)(_cachebuf + offset);
  offset += nphases * sizeof(PAMI::Topology *);

  for (count = 0; count < nphases; count ++)
    {
      _dsttopologies[count] = (PAMI::Topology *)(_cachebuf + offset);
      offset += sizeof(pami_topology_t);
    }

  _srcranks   =  (pami_task_t *)(_cachebuf + offset);
  offset     += nsrc * sizeof(pami_task_t);

  _dstranks   =  (pami_task_t *)(_cachebuf + offset);
  offset     += ndst * sizeof(pami_task_t);

  _nextActivePhaseVec = (unsigned *) (_cachebuf + offset);
  offset     += nphases * sizeof(int);

  CCMI_assert (offset == buf_size);
}

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
