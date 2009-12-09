
#ifndef __ccmi_executor_schedulecache_h__
#define __ccmi_executor_schedulecache_h__

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
      XMI::Topology          * _srctopologies;

      ///A vector of destination topologies with one of each phase
      XMI::Topology          * _dsttopologies;

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
	int start, nph, nmessages = 0;
	schedule->init (-1, BARRIER_OP, start, nph); 
	_start = start;
	_nphases = nph;
	
	TRACE_ERR((stderr,"<%X>Executor::Barrier::ScheduleCache::init _start %d, nph %d, nmessages %d\n",
		   (int) this,_start, _nphases, nmessages));
	
	CCMI_assert(_start + _nphases  <=  SC_MAXPHASES);
	CCMI_assert(nmessages <=  (int)SC_MAXRANKS);

	unsigned ntotal_src = 0, ntotal_dst = 0;
	for(unsigned count = _start; count < (_start + _nphases); count ++)
        {
	  size_t srcranks[SC_MAXRANKS], dstranks[SC_MAXRANKS];
	  XMI::Topology src_topology((size_t *)&srcranks, SC_MAXRANKS);
	  XMI::Topology dst_topology((size_t *)&dstranks, SC_MAXRANKS);

	  schedule->getSrcTopology(count, &src_topology);	  
	  ntotal_src += src_topology.size();
	  
	  schedule->getDstTopology(count, &dst_topology);        
	  ntotal_dst += dst_topology.size();
	} 
	
	allocate (_start + _nphases, ntotal_src, ntotal_dst);
	
	unsigned srcindex = 0, dstindex = 0, count = 0;
	for(count = _start; count < (_start + _nphases); count ++)
        {
	  new (&_srctopologies[count]) XMI::Topology (_srcranks + srcindex, ntotal_src - srcindex);
	  new (&_dsttopologies[count]) XMI::Topology (_dstranks + dstindex, ntotal_dst - dstindex);

	  schedule->getSrcTopology(count, &_srctopologies[count]);        
	  schedule->getDstTopology(count, &_dsttopologies[count]);        	  
	  srcindex += _srctopologies[count].size();
	  dstindex += _dsttopologies[count].size();
	}

	for( count = _start; count < (_start + _nphases); count ++)
	  if (getDstTopology(count)->size() > 0) {
	    size_t *dstranks;
	    getDstTopology(count)->rankList(&dstranks);
	    fprintf (stderr, "Schedule Cache phase %d ndst %d dstrank %d\n", count, _dsttopologies[count].size(), dstranks[0]);
	  }
	
      }

      XMI::Topology  *getSrcTopology (unsigned phase)
      {
	CCMI_assert ((phase >= _start) && (phase < _start + _nphases)); 
	return &_srctopologies[phase]; 
      }
      
      XMI::Topology  *getDstTopology (unsigned phase)
      {
	CCMI_assert ((phase >= _start) && (phase < _start + _nphases)); 
	return &_dsttopologies[phase]; 
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
  unsigned buf_size = 2 * sizeof(xmi_topology_t) * nphases + (nsrc + ndst)*sizeof(size_t);

  if (_cachesize < buf_size) {
    if (_cachebuf != NULL) 
      CCMI_Free (_cachebuf);
    
    _cachebuf = (char *) CCMI_Alloc (buf_size);
    _cachesize = buf_size;
    
    memset (_cachebuf, 0, buf_size);
  }

  unsigned offset = 0;
  _srctopologies =  (XMI::Topology *)(_cachebuf + offset);
  offset       += nphases * sizeof(xmi_topology_t);

  _dsttopologies =  (XMI::Topology *)(_cachebuf + offset);
  offset       += nphases * sizeof(xmi_topology_t);

  _srcranks   =  (size_t *)(_cachebuf + offset);
  offset       += nsrc * sizeof(size_t);

  _dstranks   =  (size_t *)(_cachebuf + offset);
  offset       += ndst * sizeof(size_t);
  
  CCMI_assert (offset == buf_size);  
}

#endif
