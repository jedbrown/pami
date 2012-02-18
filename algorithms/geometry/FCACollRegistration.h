/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/geometry/FCACollRegistration.h
 * \brief ???
 */

#ifndef __algorithms_geometry_FCACollRegistration_h__
#define __algorithms_geometry_FCACollRegistration_h__

#include "sys/pami.h"

#ifdef PAMI_USE_FCA

#include "algorithms/geometry/FCAWrapper.h"
namespace PAMI{namespace CollRegistration{namespace FCA{
template <class T_Geometry>
class FCARegistration : public CollRegistration <FCARegistration<T_Geometry>, T_Geometry >
{
public:
  class GeometryInfo
  {
    public:
    GeometryInfo(FCARegistration *reg):
      _registration(reg)
    {
    }
    FCARegistration *_registration;
  };
  typedef PAMI::MemoryAllocator<sizeof(GeometryInfo),16> GIAllocator;

  // Composite definitions
  typedef FCAReduceExec<T_Geometry>        ReduceExec;
  typedef FCAAllreduceExec<T_Geometry>     AllReduceExec;
  typedef FCABroadcastExec<T_Geometry>     BroadcastExec;
  typedef FCAAllgatherExec<T_Geometry>     AllgatherExec;
  typedef FCAAllgathervIntExec<T_Geometry> AllgathervIntExec;
  typedef FCABarrierExec<T_Geometry>       BarrierExec;

  // Factory definitions
  typedef FCAFactory<ReduceExec>        ReduceFactory;
  typedef FCAFactory<AllReduceExec>     AllReduceFactory;
  typedef FCAFactory<BroadcastExec>     BroadcastFactory;
  typedef FCAFactory<AllgatherExec>     AllgatherFactory;
  typedef FCAFactory<AllgathervIntExec> AllgathervIntFactory;
  typedef FCAFactory<BarrierExec>       BarrierFactory;

  inline FCARegistration(pami_client_t                        client,
                         pami_context_t                       context,
                         size_t                               context_id,
                         size_t                               client_id,
                         size_t                               num_contexts):
    CollRegistration <FCARegistration<T_Geometry>, T_Geometry > (),
    _client(client),
    _context(context),
    _context_id(context_id),
    _num_contexts(num_contexts),
    _client_id(client_id),
    _enabled(false),
    _reduce_f(context,context_id,mapidtogeometry),
    _allreduce_f(context,context_id,mapidtogeometry),
    _broadcast_f(context,context_id,mapidtogeometry),
    _allgather_f(context,context_id,mapidtogeometry),
    _allgatherv_int_f(context,context_id,mapidtogeometry),
    _barrier_f(context,context_id,mapidtogeometry)
  {
    if(num_contexts > 1) return;
    void * rc = FCA_Dlopen(0);
    if(!rc) return;
    else
      {
        // TODO:  print the FCA Banner and check FCA version
        
        // Fill in FCA Init Spec
        // use default config for now
        _fca_init_spec.element_type = FCA_ELEMENT_RANK;
        _fca_init_spec.job_id        = _Lapi_env.MP_partition;
        _fca_init_spec.rank_id       = _Lapi_env.MP_child;
        _fca_init_spec.progress.func = NULL;
        _fca_init_spec.progress.arg  = NULL;
        _fca_init_spec.dev_selector  = NULL;
        _fca_init_spec.config        = fca_default_config;

        FCA_Init(&_fca_init_spec, &_fca_context);
        _enabled=true;
      }
  }

  inline pami_result_t analyze_impl(size_t         context_id,
                                    T_Geometry    *geometry,
                                    uint64_t      *inout_val,
                                    int           *inout_nelem,
                                    int            phase)
  {
    if(!_enabled) return PAMI_SUCCESS;
    return PAMI_SUCCESS;

    // TODO:  Conditionally insert these algorithms into the list
    // TODO:  Implement metadata and strings
    GeometryInfo *gi = (GeometryInfo*)_geom_allocator.allocateObject();
    new(gi) GeometryInfo(this);
    geometry->setKey(_context_id, Geometry::CKEY_FCAGEOMETRYINFO, gi);
    geometry->setCleanupCallback(cleanupCallback, gi);

    geometry->addCollective(PAMI_XFER_REDUCE,
                            &_reduce_f,
                            _context,
                            _context_id);
    geometry->addCollective(PAMI_XFER_ALLREDUCE,
                            &_allreduce_f,
                            _context,
                            _context_id);
    geometry->addCollective(PAMI_XFER_BROADCAST,
                            &_broadcast_f,
                            _context,
                            _context_id);
    geometry->addCollective(PAMI_XFER_ALLGATHER,
                            &_allgather_f,
                            _context,
                            _context_id);
    geometry->addCollective(PAMI_XFER_ALLGATHERV_INT,
                            &_allgatherv_int_f,
                            _context,
                            _context_id);
    geometry->addCollective(PAMI_XFER_BARRIER,
                            &_barrier_f,
                            _context,
                            _context_id);
  }

  inline void freeGeomInfo(GeometryInfo *gi)
  {
    _geom_allocator.returnObject(gi);
  }
  static inline void cleanupCallback(pami_context_t ctxt,
                                     void          *data,
                                     pami_result_t  res)
  {
    GeometryInfo *gi = (GeometryInfo*) data;
    gi->_registration->freeGeomInfo(gi);
  }
private:
  // Client, Context, and Utility variables
  pami_client_t        _client;
  pami_context_t       _context;
  size_t               _context_id;
  size_t               _num_contexts;
  size_t               _client_id;
  bool                 _enabled;
  GIAllocator          _geom_allocator;
  fca_init_spec        _fca_init_spec;
  fca_t               *_fca_context;
  ReduceFactory        _reduce_f;
  AllReduceFactory     _allreduce_f;
  BroadcastFactory     _broadcast_f;
  AllgatherFactory     _allgather_f;
  AllgathervIntFactory _allgatherv_int_f;
  BarrierFactory       _barrier_f;



}; // FCARegistration
}; // FCA
}; // CollRegistration
}; // PAMI

#else //PAMI_USE_FCA

namespace PAMI{namespace CollRegistration{namespace FCA{
template <class T_Geometry>
class FCARegistration : public CollRegistration <FCARegistration<T_Geometry>, T_Geometry >
{
public:
  inline FCARegistration(pami_client_t                        client,
                         pami_context_t                       context,
                         size_t                               context_id,
                         size_t                               client_id,
                         size_t                               num_contexts):
    CollRegistration <FCARegistration<T_Geometry>, T_Geometry > ()
  {
    return;
  }

  inline pami_result_t analyze_impl(size_t         context_id,
                                    T_Geometry    *geometry,
                                    uint64_t      *inout_val,
                                    int           *inout_nelem,
                                    int            phase)
  {
    return PAMI_SUCCESS;
  }
}; // FCARegistration
}; // FCA
}; // CollRegistration
}; // PAMI

#endif //PAMI_USE_FCA
#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
