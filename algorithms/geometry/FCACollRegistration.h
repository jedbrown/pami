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
#include "algorithms/interfaces/GeometryInterface.h"
namespace PAMI{namespace CollRegistration{namespace FCA{
namespace G = PAMI::Geometry;
template <class T_Geometry>
class FCARegistration : public CollRegistration <FCARegistration<T_Geometry>, T_Geometry >
{
public:
  class GeometryInfo
  {
    public:
    GeometryInfo(FCARegistration *reg):
      _registration(reg),
      _fca_comm(NULL),
      _amRoot(false)
    {
    }
    FCARegistration *_registration;
    fca_comm_t      *_fca_comm;
    fca_comm_desc_t  _fca_comm_desc;
    bool             _amRoot;
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

  typedef PAMI::Topology                Topology;


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
        _fca_init_spec.element_type  = FCA_ELEMENT_RANK;
        _fca_init_spec.job_id        = _Lapi_env.MP_partition;
        _fca_init_spec.rank_id       = _Lapi_env.MP_child;
        _fca_init_spec.progress.func = NULL;
        _fca_init_spec.progress.arg  = NULL;
        _fca_init_spec.dev_selector  = NULL;
        _fca_init_spec.config        = FCA_Default_config;

        int ret = FCA_Init(&_fca_init_spec, &_fca_context);
        if (ret < 0)
          {
            printf("fca_init failed with rc %d [%s]\n", ret, FCA_Strerror(ret));
            exit(0);
          }
        else
          {
            // tmp: for debugging purpose
            printf("FCA Init succeeded\n");
          }

        _fca_rank_info = FCA_Get_rank_info(_fca_context, &_fca_rank_info_sz);
        if (_fca_rank_info == NULL)
          {
            printf("fca_get_rank_info failed with ptr %p\n", ret);
            exit(0);
          }
        else
          {
          }
        _enabled=true;
      }
  }

  // Return the number of integers to reduce
  inline size_t analyze_count(size_t      context_id,
                              T_Geometry *geometry)
  {
    if(!_enabled)
      {
        return 0;
      }
    Topology *master_topo = (Topology *) (geometry->getTopology(G::MASTER_TOPOLOGY_INDEX));
    size_t    bufsz       = (master_topo->size()*_fca_rank_info_sz)+sizeof(fca_comm_desc_t);
    size_t    extrapad    = (bufsz%sizeof(uint64_t))?1:0;
    return (bufsz/sizeof(uint64_t))+extrapad;
  }
  inline pami_result_t analyze_impl(size_t      context_id,
                                    T_Geometry *geometry,
                                    uint64_t   *inout_val,
                                    int        *inout_nelem,
                                    int         phase)
  {
    PAMI_assert(context_id == _context_id);
    if(!_enabled) return PAMI_SUCCESS;

    // TODO:  Conditionally insert these algorithms into the list
    // TODO:  Implement metadata and strings
    Topology        *topo             = (Topology *) (geometry->getTopology(G::DEFAULT_TOPOLOGY_INDEX));
    Topology        *master_topo      = (Topology *) (geometry->getTopology(G::MASTER_TOPOLOGY_INDEX));
    Topology        *local_topo       = (Topology *) (geometry->getTopology(G::LOCAL_TOPOLOGY_INDEX));
    Topology        *my_master_topo   = (Topology *) (geometry->getTopology(G::MASTER_TOPOLOGY_INDEX));

    pami_endpoint_t  my_endpoint = PAMI_ENDPOINT_INIT(_client_id,
                                                     __global.mapping.task(),
                                                     _context_id);

    pami_endpoint_t  master_ep        = local_topo->index2Endpoint(0);
    pami_endpoint_t  root_ep          = master_topo->index2Endpoint(0);
    uint             master_index     = master_topo->endpoint2Index(master_ep);
    uint             numtasks         = topo->size();
    uint             num_local_tasks  = local_topo->size();
    uint             num_master_tasks = master_topo->size();
    bool             amLeader         = master_topo->isEndpointMember(my_endpoint);
    bool             amRoot           = (root_ep == my_endpoint);

    switch (phase)
      {
        case 0:
        {
          // Allocate Per Geometry Information
          GeometryInfo *gi = (GeometryInfo *)_geom_allocator.allocateObject();
          new(gi) GeometryInfo(this);
          geometry->setKey(_context_id, G::CKEY_FCAGEOMETRYINFO, gi);
          geometry->setCleanupCallback(cleanupCallback, gi);
          // Set up the first phase reduction buffer
          uint64_t *uptr      = (uint64_t*)inout_val;
          size_t    count     = analyze_count(context_id, geometry);
          for(int i=0; i<count; i++)
            uptr[i] = 0xFFFFFFFFFFFFFFFFULL;
          if(amLeader)
            {
              // master_index is my index in the master topology
              // for a leader (leader is master)
              char *ptr       = ((char*)inout_val)+(master_index*_fca_rank_info_sz);
              memcpy(ptr, _fca_rank_info, _fca_rank_info_sz);
            }
          else
            {

            }
          return PAMI_SUCCESS;
        }
        break;
        case 1:
        {
          uint64_t *uptr      = (uint64_t*)inout_val;
          size_t    count     = analyze_count(context_id, geometry);
          for(int i=0; i<count; i++)
            uptr[i] = 0xFFFFFFFFFFFFFFFFULL;
          char *ptr       = ((char*)inout_val)+(master_topo->size()*_fca_rank_info_sz);
          memset(ptr, 0xFF,sizeof(fca_comm_desc_t));
          if(amRoot)
            {
              GeometryInfo   *gi = (GeometryInfo*)geometry->getKey(_context_id,
                                                                   G::CKEY_FCAGEOMETRYINFO);
              fca_comm_new_spec_t cs;
              cs.rank_info     = (void*)inout_val[0];
              cs.rank_count    = master_topo->size();
              cs.is_comm_world = 0;
              fca_comm_desc_t *comm_desc = (fca_comm_desc_t*) ptr;
              FCA_Comm_new(_fca_context,
                           &cs,
                           comm_desc);
              gi->_amRoot = true;
            }
        }
        return PAMI_SUCCESS;
        break;
        case 2:
        {
          fca_comm_init_spec is;
          char            *ptr       = ((char*)inout_val)+(master_topo->size()*_fca_rank_info_sz);
          fca_comm_desc_t *d         = (fca_comm_desc_t*)ptr;
          GeometryInfo    *gi        = (GeometryInfo*)geometry->getKey(_context_id,
                                                                       G::CKEY_FCAGEOMETRYINFO);
          gi->_fca_comm_desc         = *d;
          is.desc                    = *d;
          is.rank                    = topo->endpoint2Index(my_endpoint);
          is.size                    = topo->size();
          is.proc_idx                = local_topo->endpoint2Index(my_endpoint);
          is.num_procs               = local_topo->size();

          FCA_Comm_init(_fca_context,
                        &is,
                        &gi->_fca_comm);

        }
        return PAMI_SUCCESS;
        break;
        default:
          return PAMI_SUCCESS;
          break;
      }


    geometry->addCollectiveCheck(PAMI_XFER_REDUCE,
                                 &_reduce_f,
                                 _context,
                                 _context_id);
    geometry->addCollectiveCheck(PAMI_XFER_ALLREDUCE,
                                 &_allreduce_f,
                                 _context,
                                 _context_id);
    geometry->addCollectiveCheck(PAMI_XFER_BROADCAST,
                                 &_broadcast_f,
                                 _context,
                                 _context_id);
    geometry->addCollectiveCheck(PAMI_XFER_ALLGATHER,
                                 &_allgather_f,
                                 _context,
                                 _context_id);
    geometry->addCollectiveCheck(PAMI_XFER_ALLGATHERV_INT,
                                 &_allgatherv_int_f,
                                 _context,
                                 _context_id);
    geometry->addCollectiveCheck(PAMI_XFER_BARRIER,
                                 &_barrier_f,
                                 _context,
                                 _context_id);
  }
  inline fca_t *getFCAContext()
    {
      return _fca_context;
    }

  inline void freeGeomInfo(GeometryInfo *gi)
  {
    FCA_Comm_destroy(gi->_fca_comm);
    if(gi->_amRoot == true)
      {
        FCA_Comm_end(gi->_registration->getFCAContext(),
                     gi->_fca_comm_desc.comm_id);
      }
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
  void                *_fca_rank_info;
  int                  _fca_rank_info_sz;
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
