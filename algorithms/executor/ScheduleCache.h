/*
 * \file algorithms/executor/ScheduleCache.h
 * \brief ???
 */

#ifndef __algorithms_executor_ScheduleCache_h__
#define __algorithms_executor_ScheduleCache_h__

#include "algorithms/interfaces/Schedule.h"
#include "Topology.h"

#undef TRACE_SCACHE
#define TRACE_SCACHE(x)

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

        ///A vector of source topologies with one for each phase
        PAMI::Topology          ** _srctopologies;

        ///A vector of destination topologies with one of each phase
        PAMI::Topology          ** _dsttopologies;

        PAMI::Topology             _srcuniontopology; //The uninon of all sources
        PAMI::Topology             _dstuniontopology; //The union of all destinations

        size_t                 * _srcranks;  //Cache buffer of source ranks
        size_t                 * _dstranks;  //Cache buffer of destination ranks

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

        static const size_t SC_MAXPHASES = 32;
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
          _schedule->init (_root, _operation, start, nph);
          _start = start;
          _nphases = nph;

          TRACE_SCACHE((stderr, "<%X>Executor::Barrier::ScheduleCache::init _start %d, nph %d\n",
                        (int) this, _start, _nphases));

          CCMI_assert(_start + _nphases  <=  SC_MAXPHASES);

          unsigned ntotal_src = 0, ntotal_dst = 0, count = 0;

          for (count = _start; count < (_start + _nphases); count ++)
            {
              size_t srcranks[SC_MAXRANKS], dstranks[SC_MAXRANKS];
              PAMI::Topology src_topology((pami_task_t *)&srcranks, SC_MAXRANKS);
              PAMI::Topology dst_topology((pami_task_t *)&dstranks, SC_MAXRANKS);

              _schedule->getSrcTopology(count, &src_topology);
              ntotal_src += src_topology.size();

              _schedule->getDstTopology(count, &dst_topology);
              ntotal_dst += dst_topology.size();

              TRACE_SCHEDULE((stderr, "Schedule Cache take_1 phase %d ndst %zu dstrank %u/%zu\n", count,
                              dst_topology.size(), dst_topology.index2Rank(0),  dstranks[0]));
            }

          _ntotalsrcranks = ntotal_src;
          _ntotaldstranks = ntotal_dst;

          allocate (_start + _nphases, ntotal_src, ntotal_dst);

          unsigned srcindex = 0, dstindex = 0;

          for (count = _start; count < (_start + _nphases); count ++)
            {
              TRACE_SCHEDULE((stderr, "Schedule Cache : construct topology of size src %d dst %d\n", ntotal_src - srcindex, ntotal_dst - dstindex));

              new (_srctopologies[count]) PAMI::Topology ((pami_task_t*)_srcranks + srcindex, ntotal_src - srcindex);
              new (_dsttopologies[count]) PAMI::Topology ((pami_task_t*)_dstranks + dstindex, ntotal_dst - dstindex);

              CCMI_assert (_srctopologies[count]->type() == PAMI_LIST_TOPOLOGY);
              CCMI_assert (_dsttopologies[count]->type() == PAMI_LIST_TOPOLOGY);

              _schedule->getSrcTopology(count, _srctopologies[count]);
              _schedule->getDstTopology(count, _dsttopologies[count]);
              srcindex += _srctopologies[count]->size();
              dstindex += _dsttopologies[count]->size();
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
          if ((phase < _start) || (phase >= _start + _nphases))
            fprintf(stderr, "<%p>phase not in range %d, %d, %d\n", this, phase, _start, _start + _nphases);

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

        unsigned  getLastReducePhase()
        {
          //Curently hardcode to end phase
          return _start + _nphases - 1;
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
  //Compute space for nsrcranks, srcoffsets, srcranks, srcsubstasks +
  //ndstranks, dstoffsets, dstranks, dstsubstasks
  unsigned buf_size = 2 * (sizeof(PAMI::Topology *) + sizeof(pami_topology_t)) * nphases + (nsrc + ndst) * sizeof(size_t);

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

  _srcranks   =  (size_t *)(_cachebuf + offset);
  offset       += nsrc * sizeof(size_t);

  _dstranks   =  (size_t *)(_cachebuf + offset);
  offset       += ndst * sizeof(size_t);

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
