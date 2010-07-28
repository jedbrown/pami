/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/geometry/CCMICSMultiRegistration.h
 * \brief Shared memory collectives over multi* interface
 */

#ifndef __algorithms_geometry_CCMICSMultiRegistration_h__
#define __algorithms_geometry_CCMICSMultiRegistration_h__

#undef TRACE_ERR
#define TRACE_ERR(x) //fprintf x

#include <map>
#include <vector>
#include "SysDep.h"
#include "TypeDefs.h"
#include "components/devices/cshmem/CollShmDevice.h"
#include "components/devices/NativeInterface.h"
#include "algorithms/interfaces/CollRegistrationInterface.h"
#include "algorithms/protocols/allreduce/CSMultiCombineComposite.h"
#include "algorithms/protocols/broadcast/CSMultiCastComposite.h"
#include "algorithms/protocols/barrier/CSMultiSyncComposite.h"
#include "algorithms/protocols/AllSidedCSProtocolFactoryT.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {

      void csmcomb_reduce_md(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        strcpy(&m->name[0],"CCMICSMcombReduce<");
      }

      typedef CCMI::Adaptor::AllSidedCSProtocolFactoryT<CSMultiCombineComposite, csmcomb_reduce_md> CSMultiCombineFactory;
    }; // Allreduce

    namespace Broadcast
    {

      void csmcast_broadcast_md(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        strcpy(&m->name[0],"CCMICSMcastBroadcast");
      }

      typedef CCMI::Adaptor::AllSidedCSProtocolFactoryT<CSMultiCastComposite, csmcast_broadcast_md> CSMultiCastFactory;
    }; // Broadcast

    namespace Barrier
    {

      void csmsync_barrier_md(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        strcpy(&m->name[0],"CCMICSMsyncBarrier");
      }

      typedef CCMI::Adaptor::AllSidedCSProtocolFactoryT<CSMultiSyncComposite, csmsync_barrier_md> CSMultiSyncFactory;
    }; // Barrier

  }; // Adaptor
}; // CCMI


namespace PAMI
{
  extern std::map<unsigned, pami_geometry_t> geometry_map;

  namespace CollRegistration
  {

    template <class T_Geometry, class T_CSNativeInterface, class T_CSMemoryManager, class T_CSModel>
    class CCMICSMultiRegistration :
    public CollRegistration<PAMI::CollRegistration::CCMICSMultiRegistration<T_Geometry,
                                      T_CSNativeInterface, T_CSMemoryManager, T_CSModel>, T_Geometry>
    {
    public:
      inline CCMICSMultiRegistration( pami_client_t       client,
                                      size_t              client_id,
                                      pami_context_t      context,
                                      size_t              context_id,
                                      // PAMI::Context       *contexts,
                                      PAMI::Device::Generic::Device &devs) :
      CollRegistration<PAMI::CollRegistration::CCMICSMultiRegistration<T_Geometry, T_CSNativeInterface,
                                                     T_CSMemoryManager, T_CSModel>, T_Geometry> (),
      _client(client),
      _client_id(client_id),
      _context(context),
      _context_id(context_id),
      // _contexts(contexts),
      _devs(devs)
      {
        TRACE_ERR((stderr, "<%p>%s\n", this, __PRETTY_FUNCTION__));
        //set the mapid functions
        _msync_reg.setMapIdToGeometry(mapidtogeometry);
        _csmm.init(__global.mapping.task(), __global.mapping.size());
      }

      inline pami_result_t analyze_impl(size_t context_id, T_Geometry *geometry, int phase)
      {

        if (phase != 0) return PAMI_SUCCESS;

        // only support single node for now
        PAMI_assert(context_id == 0);

        PAMI::Topology *cs_topo = (PAMI::Topology *) (geometry->getTopology(0));

        if (cs_topo->size() > 1) {
          // There is a hack to get shared memory region in shared memory manager
          // init method for context 0 and _world_geometry
          // Todo: need to re-enable this getCtrlStr and add allreduce before
          //       model construction
          // if (geometry->rank() == 0) _csmm.getCtrlStr(cs_topo->size());

          T_CSModel *cs_model = new T_CSModel(&_devs, geometry->comm(), cs_topo, &_csmm);

          T_CSNativeInterface *ni = new T_CSNativeInterface(*cs_model, _client, _client_id, _context, _context_id, 0, 0);
          geometry->setKey(PAMI::Geometry::PAMI_GKEY_GEOMETRYCSNI, ni);

          TRACE_ERR((stderr, "<%p>%s context_id %zu, geometry %p\n", this, __PRETTY_FUNCTION__, context_id, geometry));

          geometry->addCollective(PAMI_XFER_BARRIER,&_msync_reg,context_id);
          geometry->addCollective(PAMI_XFER_BROADCAST,&_mcast_reg,context_id);
          geometry->addCollective(PAMI_XFER_REDUCE,&_mcomb_reg,context_id);
        }

        return PAMI_SUCCESS;
      }

      static pami_geometry_t mapidtogeometry (int comm)
      {
        pami_geometry_t g = geometry_map[comm];
        TRACE_ERR((stderr, "<%p>%s\n", g, __PRETTY_FUNCTION__));
        return g;
      }

    public:
      pami_client_t                                            _client;
      size_t                                                   _client_id;
      pami_context_t                                           _context;
      size_t                                                   _context_id;
      // PAMI::Context                                            *_contexts;
      PAMI::Device::Generic::Device                            &_devs;

      CCMI::Adaptor::Barrier::CSMultiSyncFactory               _msync_reg;
      CCMI::Adaptor::Broadcast::CSMultiCastFactory             _mcast_reg;
      CCMI::Adaptor::Allreduce::CSMultiCombineFactory          _mcomb_reg;

      T_CSMemoryManager _csmm;
    }; // CCMICSMultiRegistration
  }; // CollRegistration
}; // PAMI

#endif
