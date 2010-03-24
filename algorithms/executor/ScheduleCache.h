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

      ///A vector of source topologies with one for each phase
      PAMI::Topology          ** _srctopologies;

      ///A vector of destination topologies with one of each phase
      PAMI::Topology          ** _dsttopologies;

      size_t                 * _srcranks;  //Cache buffer of source ranks
      size_t                 * _dstranks;  //Cache buffer of destination ranks

      ///Buffer to cache the schedule
      char                   * _cachebuf;
      ///Size of the cache buffer
      unsigned            _cachesize;

      static const size_t SC_MAXPHASES = 32;
      static const size_t SC_MAXRANKS  = 128;

      ///Allocate routing that allocates memory for src phases and dst
      ///phases
      void allocate (unsigned nphases, unsigned nsrc, unsigned ndst);

    public:
      ScheduleCache () : _cachebuf(NULL), _cachesize(0)
        {
        }

      virtual ~ScheduleCache() { if (_cachebuf) CCMI_Free (_cachebuf); }

      void init(Interfaces::Schedule *schedule)
      {
	int start, nph;
	schedule->init (-1, BARRIER_OP, start, nph);
	_start = start;
	_nphases = nph;

	TRACE_ERR((stderr,"<%X>Executor::Barrier::ScheduleCache::init _start %d, nph %d\n",
		   (int) this,_start, _nphases));

	CCMI_assert(_start + _nphases  <=  SC_MAXPHASES);

	unsigned ntotal_src = 0, ntotal_dst = 0, count = 0;
	for(count = _start; count < (_start + _nphases); count ++)
        {
	  size_t srcranks[SC_MAXRANKS], dstranks[SC_MAXRANKS];
	  PAMI::Topology src_topology((pami_task_t *)&srcranks, SC_MAXRANKS);
	  PAMI::Topology dst_topology((pami_task_t *)&dstranks, SC_MAXRANKS);

	  schedule->getSrcTopology(count, &src_topology);
	  ntotal_src += src_topology.size();

	  schedule->getDstTopology(count, &dst_topology);
	  ntotal_dst += dst_topology.size();

	  //fprintf (stderr, "Schedule Cache take_1 phase %d ndst %d dstrank %d\n", count, dst_topology.size(), dstranks[0]);
	}

	allocate (_start + _nphases, ntotal_src, ntotal_dst);

	unsigned srcindex = 0, dstindex = 0;
	for(count = _start; count < (_start + _nphases); count ++)
        {
	  //fprintf (stderr, "Schedule Cache : construct topology of size src %d dst %d\n", ntotal_src - srcindex, ntotal_dst - dstindex);

	  new (_srctopologies[count]) PAMI::Topology ((pami_task_t*)_srcranks + srcindex, ntotal_src - srcindex);
	  new (_dsttopologies[count]) PAMI::Topology ((pami_task_t*)_dstranks + dstindex, ntotal_dst - dstindex);

	  CCMI_assert (_srctopologies[count]->type() == PAMI_LIST_TOPOLOGY);
	  CCMI_assert (_dsttopologies[count]->type() == PAMI_LIST_TOPOLOGY);

	  schedule->getSrcTopology(count, _srctopologies[count]);
	  schedule->getDstTopology(count, _dsttopologies[count]);
	  srcindex += _srctopologies[count]->size();
	  dstindex += _dsttopologies[count]->size();
	}

#if 0
	for( count = _start; count < (_start + _nphases); count ++)
	  if (getDstTopology(count)->size() > 0) {
	    size_t *dstranks;
	    getDstTopology(count)->rankList(&dstranks);
	    TRACE_ERR ((stderr, "Schedule Cache take_2 phase %d ndst %d dstrank %d\n", count, _dsttopologies[count]->size(), dstranks[0]));
	  }
#endif

      }

      PAMI::Topology  *getSrcTopology (unsigned phase)
      {
	if ((phase < _start) || (phase >= _start + _nphases))
	  fprintf(stderr, "<%p>phase not in range %d, %d, %d\n", this, phase, _start, _start+_nphases);

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
  unsigned buf_size = 2 * (sizeof(PAMI::Topology *) + sizeof(pami_topology_t)) * nphases + (nsrc + ndst)*sizeof(size_t);

  if (_cachesize < buf_size) {
    if (_cachebuf != NULL)
      CCMI_Free (_cachebuf);

    _cachebuf = (char *) CCMI_Alloc (buf_size);
    _cachesize = buf_size;

    memset (_cachebuf, 0, buf_size);
  }

  unsigned offset = 0, count = 0;
  _srctopologies =  (PAMI::Topology **)(_cachebuf + offset);
  offset += nphases * sizeof(PAMI::Topology *);
  for (count = 0; count < nphases; count ++) {
    _srctopologies[count] = (PAMI::Topology *)(_cachebuf + offset);
    offset += sizeof(pami_topology_t);
  }

  _dsttopologies =  (PAMI::Topology **)(_cachebuf + offset);
  offset += nphases * sizeof(PAMI::Topology *);
  for (count = 0; count < nphases; count ++) {
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
