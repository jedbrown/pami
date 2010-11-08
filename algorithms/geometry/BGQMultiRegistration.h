/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/geometry/BGQMultiRegistration.h
 * \brief Simple BGQ collectives over shmem/mu multi* interface
 */

#ifndef __algorithms_geometry_BGQMultiRegistration_h__
#define __algorithms_geometry_BGQMultiRegistration_h__

#include <map>
#include <vector>
#include "algorithms/interfaces/CollRegistrationInterface.h"
#include "TypeDefs.h"
#include "algorithms/connmgr/SimpleConnMgr.h"
#include "algorithms/connmgr/CommSeqConnMgr.h"
#include "algorithms/protocols/broadcast/MultiCastComposite.h"
#include "algorithms/protocols/allreduce/MultiCombineComposite.h"
#include "algorithms/protocols/barrier/MultiSyncComposite.h"
#include "algorithms/protocols/AllSidedCollectiveProtocolFactoryT.h"
#include "algorithms/protocols/CollectiveProtocolFactoryT.h"

#include "algorithms/connmgr/ColorGeometryConnMgr.h"
#include "algorithms/connmgr/ColorConnMgr.h"
#include "algorithms/protocols/broadcast/BcastMultiColorCompositeT.h"
#include "algorithms/schedule/MCRect.h"
#include "algorithms/schedule/TorusRect.h"
#include "common/NativeInterface.h"

#include "util/ccmi_debug.h" // tracing
#include "util/ccmi_util.h"

#ifndef CCMI_TRACE_ALL
  #undef TRACE_INIT
  #define TRACE_INIT(x) //fprintf x
#endif

namespace PAMI
{
  namespace CollRegistration
  {

    //----------------------------------------------------------------------------
    /// Declare our protocol factory templates and their metadata templates
    ///
    /// 'Pure' protocols only work on the specified (Shmem or MU) device.
    ///
    /// 'Composite' protocols combine Shmem/MU devices.
    ///
    /// 'Sub' are pure protocols that work on a subtopology of the geometry, not
    /// the whole geometry.  They may be used to create composite protocols
    //----------------------------------------------------------------------------

    //----------------------------------------------------------------------------
    // 'Pure' Shmem allsided multisync
    //----------------------------------------------------------------------------
    void ShmemMsyncMetaData(pami_metadata_t *m)
    {
      strncpy(&m->name[0], "ShmemMultiSync", 32);
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Barrier::MultiSyncComposite<>,
    ShmemMsyncMetaData,
    CCMI::ConnectionManager::SimpleConnMgr > ShmemMultiSyncFactory;

    //----------------------------------------------------------------------------
    // 'Pure' Shmem allsided multicombine
    //----------------------------------------------------------------------------
    void ShmemMcombMetaData(pami_metadata_t *m)
    {
      strncpy(&m->name[0], "ShmemMultiComb", 32);
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite,
    ShmemMcombMetaData,
    CCMI::ConnectionManager::SimpleConnMgr > ShmemMultiCombineFactory;

    //----------------------------------------------------------------------------
    // 'Pure' Shmem allsided multicast
    //----------------------------------------------------------------------------
    void ShmemMcastMetaData(pami_metadata_t *m)
    {
      strncpy(&m->name[0], "ShmemMultiCast", 32);
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Broadcast::MultiCastComposite,
    ShmemMcastMetaData,
    CCMI::ConnectionManager::SimpleConnMgr > ShmemMultiCastFactory;


    //----------------------------------------------------------------------------
    // 'Pure' MU allsided multisync
    //----------------------------------------------------------------------------
    void MUMsyncMetaData(pami_metadata_t *m)
    {
      strncpy(&m->name[0], "MUMultiSync", 32);
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Barrier::MultiSyncComposite<>,
    MUMsyncMetaData,
    CCMI::ConnectionManager::SimpleConnMgr > MUMultiSyncFactory;

    //----------------------------------------------------------------------------
    // 'Pure' MU allsided multicast
    //----------------------------------------------------------------------------
    void MUMcombMetaData(pami_metadata_t *m)
    {
      strncpy(&m->name[0], "MUMultiComb", 32);
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite,
    MUMcombMetaData,
    CCMI::ConnectionManager::SimpleConnMgr > MUMultiCombineFactory;

    //----------------------------------------------------------------------------
    // 'Pure' MU allsided multicast built on active message multicast with an
    // synchronizing multisync
    //
    // The necessary factory is defined here to implement the appropriate
    // dispatch/notifyRecv.  We simply us a map<> to associate connection id's
    // with composites.  Since it's all-sided, this should be fine.
    //
    // Connection id's are based on incrementing CommSeqConnMgr id's.
    //----------------------------------------------------------------------------
    void MUMcast2MetaData(pami_metadata_t *m)
    {
      strncpy(&m->name[0], "MUMultiCast_Msync", 32);
    }

    // Define our base factory
    typedef CCMI::Adaptor::CollectiveProtocolFactoryT < CCMI::Adaptor::Broadcast::MultiCastComposite2<BGQGeometry>,
    MUMcast2MetaData,
    CCMI::ConnectionManager::CommSeqConnMgr > MUMultiCast2FactoryBase;

    // Extend the base factory to handle our multicast dispatch head
    class MUMultiCast2Factory : public MUMultiCast2FactoryBase
    {
    public:
      MUMultiCast2Factory (CCMI::ConnectionManager::CommSeqConnMgr *cmgr,
                           CCMI::Interfaces::NativeInterface *native,
                           pami_dispatch_multicast_function cb_head = NULL):
      // pass our multicast dispatch (mu2_cb_async) to the parent
      MUMultiCast2FactoryBase(cmgr, native, (pami_dispatch_multicast_function)&mu2_cb_async)
      {
        TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::MUMultiCast2Factory()\n", this));
      };
      virtual CCMI::Executor::Composite * generate(pami_geometry_t             geometry,
                                                   void                      * cmd)
      {
        TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::MUMultiCast2Factory::generate()\n", this));

        // The composite will ctor the connection manager and generate a unique connection id.
        collObj *cobj = (collObj*) _alloc.allocateObject();
        TRACE_ADAPTOR((stderr, "<%p>CollectiveProtocolFactoryT::generate()\n", cobj));
        new(cobj) collObj(_native,          // Native interface
                          _cmgr,            // Connection Manager
                          geometry,         // Geometry Object
                          (pami_xfer_t*)cmd, // Parameters
                          done_fn,          // Intercept function
                          cobj,             // Intercept cookie
                          this);            // Factory

        CCMI::Adaptor::Broadcast::MultiCastComposite2<BGQGeometry> *composite = &cobj->_obj;

        // Get the (updated and unique) connection id and store this new composite in the map[connection_id].
        BGQGeometry *bgqGeometry = (BGQGeometry *)geometry;
        unsigned comm = bgqGeometry->comm();
        cobj->_connection_id = _cmgr->getConnectionId (comm, 0, 0, 0, 0);
        TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::MUMultiCast2Factory::generate() map comm %u, connection_id %u, composite %p\n", this, comm, cobj->_connection_id, composite));
        _composite_map[cobj->_connection_id] = composite;
        return composite;
      }
      static void done_fn(pami_context_t  context,
                          void           *clientdata,
                          pami_result_t   res)
      {
        collObj *cobj = (collObj *)clientdata;
        TRACE_ADAPTOR((stderr, "<%p>PAMI::CollRegistration::MUMultiCast2Factory::done_fn()\n", cobj));
        cobj->done_fn(context, res);
        MUMultiCast2Factory* factory = (MUMultiCast2Factory*) cobj->_factory;
        factory->_composite_map.erase(cobj->_connection_id);
        cobj->_factory->_alloc.returnObject(cobj);
      }


      ///
      /// \brief multicast dispatch - call the factory notifyRecvHead
      ///
      static void mu2_cb_async(const pami_quad_t      *msginfo,       // \param[in] msginfo    Metadata
                               unsigned                msgcount,      // \param[in] msgcount Count of metadata
                               unsigned                connection_id, // \param[in] connection_id  Stream ID of data
                               size_t                  root,          // \param[in] root        Sending task
                               size_t                  sndlen,        // \param[in] sndlen      Length of data sent
                               void                  * clientdata,    // \param[in] clientdata  Opaque arg
                               size_t                * rcvlen,        // \param[out] rcvlen     Length of data to receive
                               pami_pipeworkqueue_t ** rcvpwq,        // \param[out] rcvpwq     Where to put recv data
                               pami_callback_t       * cb_done)       // \param[out] cb_done    Completion callback to invoke when data received
      {
        TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::MUMultiCast2Factory::mu2_cb_async()\n", clientdata));
        MUMultiCast2Factory* factory = (MUMultiCast2Factory*) clientdata;
        return factory->notifyRecvHead(msginfo, msgcount, connection_id, root, sndlen, rcvlen, rcvpwq, cb_done);
      }

      ///
      /// \brief Find the composite by connection_id in the map and notifyRecv()
      ///
      void notifyRecvHead(const pami_quad_t      *msginfo,       // \param[in] msginfo    Metadata
                          unsigned                msgcount,      // \param[in] msgcount Count of metadata
                          unsigned                connection_id, // \param[in] connection_id  Stream ID of data
                          size_t                  root,          // \param[in] root        Sending task
                          size_t                  sndlen,        // \param[in] sndlen      Length of data sent
                          size_t                * rcvlen,        // \param[out] rcvlen     Length of data to receive
                          pami_pipeworkqueue_t ** rcvpwq,        // \param[out] rcvpwq     Where to put recv data
                          pami_callback_t       * cb_done)       // \param[out] cb_done    Completion callback to invoke when data received
      {
        TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::MUMultiCast2Factory::notifyRecvHead() msgcount %u, connection_id %u, root %zu, sndlen %zu\n", this, msgcount, connection_id, root, sndlen));
        CCMI::Adaptor::Broadcast::MultiCastComposite2<BGQGeometry> *composite = _composite_map[connection_id];
        PAMI_assertf(composite, "connection_id %u, root %zu\n", connection_id, root);
        *rcvlen = sndlen;  // Not passed or set by notifyRecv? Just assume everything sent will be received
        return composite->notifyRecv (root,
                                      msginfo,
                                      rcvpwq,
                                      cb_done);
      };
      std::map<unsigned, CCMI::Adaptor::Broadcast::MultiCastComposite2<BGQGeometry> * > _composite_map;
    };

    //----------------------------------------------------------------------------
    // 'Pure' MU Axial/line allsided multicast built on active message multicast with an
    // synchronizing multisync
    //
    // The necessary factory is defined here to implement the appropriate
    // dispatch/notifyRecv.  We simply us a map<> to associate connection id's
    // with composites.  Since it's all-sided, this should be fine.
    //
    // Connection id's are based on incrementing CommSeqConnMgr id's.
    //----------------------------------------------------------------------------
    void LineMcast2MetaData(pami_metadata_t *m)
    {
      strncpy(&m->name[0], "LineMultiCast_Msync", 32);
    }

    // Define our base factory
    typedef CCMI::Adaptor::CollectiveProtocolFactoryT < CCMI::Adaptor::Broadcast::MultiCastComposite2<BGQGeometry>,
    LineMcast2MetaData,
    CCMI::ConnectionManager::CommSeqConnMgr > LineMultiCast2FactoryBase;

    // Extend the base factory to handle our multicast dispatch head
    class LineMultiCast2Factory : public LineMultiCast2FactoryBase
    {
    public:
      LineMultiCast2Factory (CCMI::ConnectionManager::CommSeqConnMgr *cmgr,
                              CCMI::Interfaces::NativeInterface *native,
                              pami_dispatch_multicast_function cb_head = NULL):
      // pass our multicast dispatch (line2_cb_async) to the parent
      LineMultiCast2FactoryBase(cmgr, native, (pami_dispatch_multicast_function)&line2_cb_async)
      {
        TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::LineMultiCast2Factory()\n", this));
      };
      virtual CCMI::Executor::Composite * generate(pami_geometry_t             geometry,
                                                   void                      * cmd)
      {
        TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::LineMultiCast2Factory::generate()\n", this));

        // The composite will ctor the connection manager and generate a unique connection id.
        collObj *cobj = (collObj*) _alloc.allocateObject();
        TRACE_ADAPTOR((stderr, "<%p>CollectiveProtocolFactoryT::generate()\n", cobj));
        new(cobj) collObj(_native,          // Native interface
                          _cmgr,            // Connection Manager
                          geometry,         // Geometry Object
                          (pami_xfer_t*)cmd, // Parameters
                          done_fn,          // Intercept function
                          cobj,             // Intercept cookie
                          this);            // Factory

        CCMI::Adaptor::Broadcast::MultiCastComposite2<BGQGeometry> *composite = &cobj->_obj;

        // Get the (updated and unique) connection id and store this new composite in the map[connection_id].
        BGQGeometry *bgqGeometry = (BGQGeometry *)geometry;
        unsigned comm = bgqGeometry->comm();
        cobj->_connection_id = _cmgr->getConnectionId (comm, 0, 0, 0, 0);
        TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::LineMultiCast2Factory::generate() map comm %u, connection_id %u, composite %p\n", this, comm, cobj->_connection_id, composite));
        _composite_map[cobj->_connection_id] = composite;
        return composite;
      }
      static void done_fn(pami_context_t  context,
                          void           *clientdata,
                          pami_result_t   res)
      {
        collObj *cobj = (collObj *)clientdata;
        TRACE_ADAPTOR((stderr, "<%p>PAMI::CollRegistration::LineMultiCast2Factory::done_fn()\n", cobj));
        cobj->done_fn(context, res);
        LineMultiCast2Factory* factory = (LineMultiCast2Factory*) cobj->_factory;
        factory->_composite_map.erase(cobj->_connection_id);
        cobj->_factory->_alloc.returnObject(cobj);
      }


      ///
      /// \brief multicast dispatch - call the factory notifyRecvHead
      ///
      static void line2_cb_async(const pami_quad_t      *msginfo,       // \param[in] msginfo    Metadata
                                  unsigned                msgcount,      // \param[in] msgcount Count of metadata
                                  unsigned                connection_id, // \param[in] connection_id  Stream ID of data
                                  size_t                  root,          // \param[in] root        Sending task
                                  size_t                  sndlen,        // \param[in] sndlen      Length of data sent
                                  void                  * clientdata,    // \param[in] clientdata  Opaque arg
                                  size_t                * rcvlen,        // \param[out] rcvlen     Length of data to receive
                                  pami_pipeworkqueue_t ** rcvpwq,        // \param[out] rcvpwq     Where to put recv data
                                  pami_callback_t       * cb_done)       // \param[out] cb_done    Completion callback to invoke when data received
      {
        TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::LineMultiCast2Factory::line2_cb_async()\n", clientdata));
        LineMultiCast2Factory* factory = (LineMultiCast2Factory*) clientdata;
        return factory->notifyRecvHead(msginfo, msgcount, connection_id, root, sndlen, rcvlen, rcvpwq, cb_done);
      }

      ///
      /// \brief Find the composite by connection_id in the map and notifyRecv()
      ///
      void notifyRecvHead(const pami_quad_t      *msginfo,       // \param[in] msginfo    Metadata
                          unsigned                msgcount,      // \param[in] msgcount Count of metadata
                          unsigned                connection_id, // \param[in] connection_id  Stream ID of data
                          size_t                  root,          // \param[in] root        Sending task
                          size_t                  sndlen,        // \param[in] sndlen      Length of data sent
                          size_t                * rcvlen,        // \param[out] rcvlen     Length of data to receive
                          pami_pipeworkqueue_t ** rcvpwq,        // \param[out] rcvpwq     Where to put recv data
                          pami_callback_t       * cb_done)       // \param[out] cb_done    Completion callback to invoke when data received
      {
        TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::LineMultiCast2Factory::notifyRecvHead() msgcount %u, connection_id %u, root %zu, sndlen %zu\n", this, msgcount, connection_id, root, sndlen));
        CCMI::Adaptor::Broadcast::MultiCastComposite2<BGQGeometry> *composite = _composite_map[connection_id];
        PAMI_assertf(composite, "connection_id %u, root %zu\n", connection_id, root);
        *rcvlen = sndlen;  // Not passed or set by notifyRecv? Just assume everything sent will be received
        return composite->notifyRecv (root,
                                      msginfo,
                                      rcvpwq,
                                      cb_done);
      };
      std::map<unsigned, CCMI::Adaptor::Broadcast::MultiCastComposite2<BGQGeometry> * > _composite_map;
    };

    //----------------------------------------------------------------------------
    // 'Sub' Shmem allsided multisync - works on geometry->getLocalTopology()
    // (LOCAL_TOPOLOGY_INDEX)
    //----------------------------------------------------------------------------
    void SubShmemMsyncMetaData(pami_metadata_t *m)
    {
      strncpy(&m->name[0], "ShmemMultiSyncSubComposite", 32);
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Barrier::MultiSyncComposite<LOCAL_TOPOLOGY_INDEX>,
    SubShmemMsyncMetaData,
    CCMI::ConnectionManager::SimpleConnMgr > SubShmemMultiSyncFactory;

    //----------------------------------------------------------------------------
    // 'Sub' MU allsided multisync - works on geometry->getLocalMasterTopology()
    // (LOCAL_MASTER_TOPOLOGY_INDEX)
    //----------------------------------------------------------------------------
    void SubMUMsyncMetaData(pami_metadata_t *m)
    {
      strncpy(&m->name[0], "MUMultiSyncSubComposite", 32);
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Barrier::MultiSyncComposite<LOCAL_MASTER_TOPOLOGY_INDEX>,
    SubMUMsyncMetaData,
    CCMI::ConnectionManager::SimpleConnMgr > SubMUMultiSyncFactory;

    //----------------------------------------------------------------------------
    // 'Composite' Shmem/MU allsided multisync
    //----------------------------------------------------------------------------
    void Msync2MetaData(pami_metadata_t *m)
    {
      strncpy(&m->name[0], "MultiSyncComposite", 32);
    }

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Barrier::MultiSync2Composite,
    Msync2MetaData,
    CCMI::ConnectionManager::SimpleConnMgr > MultiSync2Factory;

    //----------------------------------------------------------------------------
    // 'Composite' Shmem/MU allsided 2 device multisync
    //----------------------------------------------------------------------------
    void Msync2DMetaData(pami_metadata_t *m)
    {
      strncpy(&m->name[0], "MultiSync2Device", 32);
    }
    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Barrier::MultiSyncComposite2Device,
    Msync2DMetaData, CCMI::ConnectionManager::SimpleConnMgr >  MultiSync2DeviceFactory;

    //----------------------------------------------------------------------------
    // 'Composite' Shmem/MU allsided 2 device multicast
    //----------------------------------------------------------------------------
    void Mcast2DMetaData(pami_metadata_t *m)
    {
      strncpy(&m->name[0], "MultiCast2Device", 32);
    }
    typedef CCMI::Adaptor::Broadcast::MultiCastComposite2DeviceFactoryT < CCMI::Adaptor::Broadcast::MultiCastComposite2Device<PAMI_GEOMETRY_CLASS,true>,
    Mcast2DMetaData, CCMI::ConnectionManager::SimpleConnMgr > MultiCast2DeviceFactory;

    //----------------------------------------------------------------------------
    // 'Composite' Shmem/MU allsided 2 device multicombine
    //----------------------------------------------------------------------------
    void Mcomb2DMetaData(pami_metadata_t *m)
    {
      strncpy(&m->name[0], "MultiCombine2Device", 32);
    }
    typedef CCMI::Adaptor::Allreduce::MultiCombineComposite2DeviceFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite2Device<0>,
    Mcomb2DMetaData, CCMI::ConnectionManager::SimpleConnMgr >    MultiCombine2DeviceFactory;


    //----------------------------------------------------------------------------
    // 'Composite' Shmem/MU allsided 2 device multicombine with no pipelining
    //----------------------------------------------------------------------------
    void Mcomb2DMetaDataNP(pami_metadata_t *m)
    {
      strncpy(&m->name[0], "MultiCombine2DeviceNP", 32);
    }
    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT < CCMI::Adaptor::Allreduce::MultiCombineComposite2DeviceNP,
    Mcomb2DMetaDataNP, CCMI::ConnectionManager::SimpleConnMgr > MultiCombine2DeviceFactoryNP;

    //----------------------------------------------------------------------------
    // Rectangle broadcast
    //----------------------------------------------------------------------------
    void get_colors (PAMI::Topology             * t,
                     unsigned                    bytes,
                     unsigned                  * colors,
                     unsigned                  & ncolors)
    {
      TRACE_INIT((stderr, "get_colors\n"));
      ncolors = 1;
      colors[0] = CCMI::Schedule::NO_COLOR;
    }

    void get_rect_colors (PAMI::Topology             * t,
                          unsigned                    bytes,
                          unsigned                  * colors,
                          unsigned                  & ncolors)
    {

      unsigned max = 0, ideal = 0;
      unsigned _colors[10];
      CCMI::Schedule::TorusRect::getColors (t, ideal, max, _colors);
      TRACE_INIT((stderr, "get_rect_colors() bytes %u, ncolors %u, ideal %u, max %u\n", bytes, ncolors, ideal, max));
      
      if (bytes < 2048) //4 packets
        ideal = 1;
      else if (bytes < 8192 && ideal >= 2) //4 packets
        ideal = 2;
      
      if (ideal < ncolors)
        ncolors = ideal;  //Reduce the number of colors to the relavant colors
      
      TRACE_INIT((stderr, "get_rect_colors() ncolors %u, ideal %u\n", ncolors, ideal));
      memcpy (colors, _colors, ncolors * sizeof(int));
    }

    void rectangle_dput_1color_broadcast_metadata(pami_metadata_t *m)
    {
      // \todo:  fill in other metadata
      TRACE_INIT((stderr, "%s\n", __PRETTY_FUNCTION__));
      strcpy(&m->name[0], "Rectangle1ColorDputBroadcast");
    }

    typedef CCMI::Adaptor::Broadcast::BcastMultiColorCompositeT
    <1,
    CCMI::Schedule::TorusRect,
    CCMI::ConnectionManager::ColorConnMgr,
    get_rect_colors>
    RectangleDput1ColorBroadcastComposite;

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT
    < RectangleDput1ColorBroadcastComposite,
    rectangle_dput_1color_broadcast_metadata,
    CCMI::ConnectionManager::ColorConnMgr>
    RectangleDput1ColorBroadcastFactory;

    void rectangle_dput_broadcast_metadata(pami_metadata_t *m)
    {
      // \todo:  fill in other metadata
      TRACE_INIT((stderr, "%s\n", __PRETTY_FUNCTION__));
      strcpy(&m->name[0], "RectangleDputBroadcast");
    }

    typedef CCMI::Adaptor::Broadcast::BcastMultiColorCompositeT
    <10,
    CCMI::Schedule::TorusRect,
    CCMI::ConnectionManager::ColorConnMgr,
    get_rect_colors>
    RectangleDputBroadcastComposite;

    typedef CCMI::Adaptor::AllSidedCollectiveProtocolFactoryT
    < RectangleDputBroadcastComposite,
    rectangle_dput_broadcast_metadata,
    CCMI::ConnectionManager::ColorConnMgr>
    RectangleDputBroadcastFactory;

    //----------------------------------------------------------------------------
    /// \brief The BGQ Multi* registration class for Shmem and MU.
    //----------------------------------------------------------------------------
    template <class T_Geometry, class T_ShmemNativeInterface, class T_MUDevice, class T_MUNativeInterface, class T_AxialNativeInterface, class T_AxialDputNativeInterface>
    class BGQMultiRegistration :
    public CollRegistration<PAMI::CollRegistration::BGQMultiRegistration<T_Geometry, T_ShmemNativeInterface, T_MUDevice, T_MUNativeInterface, T_AxialNativeInterface, T_AxialDputNativeInterface>, T_Geometry>
    {

    public:
      inline BGQMultiRegistration(T_ShmemNativeInterface              *shmem_ni,
                                  T_MUDevice                          &mu_device,
                                  pami_client_t                        client,
                                  pami_context_t                       context,
                                  size_t                               context_id,
                                  size_t                               client_id,
                                  int                                 *dispatch_id,
                                  std::map<unsigned, pami_geometry_t> *geometry_map):
      CollRegistration<PAMI::CollRegistration::BGQMultiRegistration<T_Geometry, T_ShmemNativeInterface, T_MUDevice, T_MUNativeInterface, T_AxialNativeInterface, T_AxialDputNativeInterface>, T_Geometry> (),
      _client(client),
      _context(context),
      _context_id(context_id),
      _dispatch_id(dispatch_id),
      _geometry_map(geometry_map),
      _sconnmgr(65535),
      _csconnmgr(),
      _cg_connmgr(65535),
      _color_connmgr(),
      _shmem_barrier_composite(NULL),
      _sub_shmem_barrier_composite(NULL),
      _mu_barrier_composite(NULL),
      _sub_mu_barrier_composite(NULL),
      _msync_composite(NULL),
      _msync2d_composite(NULL),
      _shmem_ni(shmem_ni),
      _shmem_msync_factory(&_sconnmgr, _shmem_ni),
      _sub_shmem_msync_factory(&_sconnmgr, _shmem_ni),
      _shmem_mcast_factory(&_sconnmgr, _shmem_ni),
      _shmem_mcomb_factory(&_sconnmgr, _shmem_ni),
      _mu_device(mu_device),
      _mu_ni_msync(NULL),
      _mu_ni_sub_msync(NULL),
      _mu_ni_mcomb(NULL),
      _mu_ni_mcast2(NULL),
      _mu_ni_msync2d(NULL),
      _mu_ni_mcast2d(NULL),
      _mu_ni_mcomb2d(NULL),
      _mu_ni_mcomb2dNP(NULL),
      _axial_mu_ni(NULL),
      _axial_mu_1_ni(NULL),
      _axial_dput_mu_ni(NULL),
	//_axial_dput_mu_1_ni(NULL),
      _mu_msync_factory(NULL),
      _sub_mu_msync_factory(NULL),
      _mu_mcast2_factory(NULL),
      _line_mcast2_factory(NULL),
	//_line_dput_mcast2_factory(NULL),
      _mu_mcomb_factory(NULL),
      _msync_composite_factory(&_sconnmgr, NULL),
      _msync2d_composite_factory(NULL),
      _mcast2d_composite_factory(NULL),
      _mcomb2d_composite_factory(NULL),
      _mcomb2dNP_composite_factory(NULL),
      _rectangle_1color_dput_broadcast_factory(NULL),
      _rectangle_dput_broadcast_factory(NULL)
      {
        TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration()\n", this));
        //DO_DEBUG((templateName<T_Geometry>()));
        //DO_DEBUG((templateName<T_ShmemNativeInterface>()));
        //DO_DEBUG((templateName<T_MUDevice>()));
        //DO_DEBUG((templateName<T_MUNativeInterface>()));
        //DO_DEBUG((templateName<T_AxialNativeInterface>()));
        //DO_DEBUG((templateName<T_AxialDputNativeInterface>()));

        //set the mapid functions
        if (__global.useshmem())// && (__global.topology_local.size() > 1))
        {
          TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration() useshmem\n", this));
          _shmem_msync_factory.setMapIdToGeometry(mapidtogeometry);
          _sub_shmem_msync_factory.setMapIdToGeometry(mapidtogeometry);
        }

        if (__global.useMU())
        {
          TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration() usemu\n", this));

          _mu_ni_msync          = new (_mu_ni_msync_storage         ) T_MUNativeInterface(_mu_device, client, context, context_id, client_id,_dispatch_id);
          _mu_ni_sub_msync      = new (_mu_ni_sub_msync_storage     ) T_MUNativeInterface(_mu_device, client, context, context_id, client_id,_dispatch_id);
          _mu_ni_mcomb          = new (_mu_ni_mcomb_storage         ) T_MUNativeInterface(_mu_device, client, context, context_id, client_id,_dispatch_id);
          _mu_ni_mcast2         = new (_mu_ni_mcast2_storage        ) T_MUNativeInterface(_mu_device, client, context, context_id, client_id,_dispatch_id);
          _mu_ni_msync2d        = new (_mu_ni_msync2d_storage       ) T_MUNativeInterface(_mu_device, client, context, context_id, client_id,_dispatch_id);
          _mu_ni_mcast2d        = new (_mu_ni_mcast2d_storage       ) T_MUNativeInterface(_mu_device, client, context, context_id, client_id,_dispatch_id);
          _mu_ni_mcomb2d        = new (_mu_ni_mcomb2d_storage       ) T_MUNativeInterface(_mu_device, client, context, context_id, client_id,_dispatch_id);
          _mu_ni_mcomb2dNP      = new (_mu_ni_mcomb2dNP_storage     ) T_MUNativeInterface(_mu_device, client, context, context_id, client_id,_dispatch_id);
          _axial_mu_ni          = new (_axial_mu_ni_storage         ) T_AxialNativeInterface(_mu_device, client, context, context_id, client_id,_dispatch_id);

          _axial_dput_mu_ni     = new (_axial_dput_mu_ni_storage    ) T_AxialDputNativeInterface(_mu_device, client, context, context_id, client_id,_dispatch_id);
          if (_axial_dput_mu_ni->status() != PAMI_SUCCESS) _axial_dput_mu_ni = NULL; // Not enough resources?

          _mu_msync_factory     = new (_mu_msync_factory_storage    ) MUMultiSyncFactory(&_sconnmgr, _mu_ni_msync);
          _sub_mu_msync_factory = new (_sub_mu_msync_factory_storage) SubMUMultiSyncFactory(&_sconnmgr, _mu_ni_sub_msync);
          _mu_mcomb_factory     = new (_mu_mcomb_factory_storage    ) MUMultiCombineFactory(&_sconnmgr, _mu_ni_mcomb);

          TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration()  RectangleDput1ColorBroadcastFactory\n", this));
          if (_axial_dput_mu_ni)
            _rectangle_1color_dput_broadcast_factory = new (_rectangle_1color_dput_broadcast_factory_storage) RectangleDput1ColorBroadcastFactory(&_color_connmgr, _axial_dput_mu_ni);

          TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration()  RectangleDputBroadcastFactory\n", this));
          if (_axial_dput_mu_ni)
            _rectangle_dput_broadcast_factory = new (_rectangle_dput_broadcast_factory_storage) RectangleDputBroadcastFactory(&_color_connmgr, _axial_dput_mu_ni);


          _mu_msync_factory->setMapIdToGeometry(mapidtogeometry);
          _sub_mu_msync_factory->setMapIdToGeometry(mapidtogeometry);
          _msync_composite_factory.setMapIdToGeometry(mapidtogeometry);

          // Can't be ctor'd unless the NI was created
          _mu_mcast2_factory = new (_mu_mcast2_factory_storage) MUMultiCast2Factory(&_csconnmgr, _mu_ni_mcast2);
          _line_mcast2_factory = new (_line_mcast2_factory_storage) LineMultiCast2Factory(&_csconnmgr, _axial_mu_ni);
          //_line_dput_mcast2_factory = new (_line_dput_mcast2_factory_storage) LineMultiCast2Factory(&_csconnmgr, _axial_dput_mu_ni);

        }

//          if ((__global.useMU()) && (__global.useshmem()))
        {
          _ni_array[0] = _shmem_ni;
          _ni_array[1] = _mu_ni_msync2d;
          _ni_array[2] = _shmem_ni;
          _ni_array[3] = _mu_ni_mcomb2dNP;
          _msync2d_composite_factory = new (_msync2d_composite_factory_storage) MultiSync2DeviceFactory(&_sconnmgr, (CCMI::Interfaces::NativeInterface*)_ni_array);
          _msync2d_composite_factory->setMapIdToGeometry(mapidtogeometry);
          _mcast2d_composite_factory = new (_mcast2d_composite_factory_storage) MultiCast2DeviceFactory(&_sconnmgr, _shmem_ni, false, _mu_ni_mcast2d,  _mu_ni_mcast2d ? true : false);
          _mcomb2d_composite_factory = new (_mcomb2d_composite_factory_storage) MultiCombine2DeviceFactory(&_sconnmgr, _shmem_ni, _mu_ni_mcomb2d);
          _mcomb2dNP_composite_factory = new (_mcomb2dNP_composite_factory_storage) MultiCombine2DeviceFactoryNP(&_sconnmgr,  (CCMI::Interfaces::NativeInterface*)_ni_array[2]);
        }

      }

      inline pami_result_t analyze_impl(size_t context_id, T_Geometry *geometry, int phase)
      {
        /// \todo These are really 'must query' protocols and should not be added to the regular protocol list
        TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() phase %d, context_id %zu, geometry %p, msync %p, mcast %p, mcomb %p\n", this, phase, context_id, geometry, &_shmem_msync_factory, &_shmem_mcast_factory, &_shmem_mcomb_factory));
        pami_xfer_t xfer = {0};
        PAMI::Topology * topology = (PAMI::Topology*) geometry->getTopology(0);
        PAMI::Topology * local_sub_topology = (PAMI::Topology*) geometry->getLocalTopology();
        PAMI::Topology * master_sub_topology = (PAMI::Topology*) geometry->getLocalMasterTopology();
        TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() topology: size() %zu, isLocal() %u/%zu, isGlobal #u/%zu\n", this, topology->size(),  topology->isLocalToMe(), local_sub_topology->size(), master_sub_topology->size()));//,  topology->isGlobal()));

        //DO_DEBUG(for (unsigned i = 0; i < topology->size(); ++i) fprintf(stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() topology[%u] = %u\n", this, i, topology->index2Rank(i)););

        if (phase == 0)
        {

          if ((__global.useshmem()) && (__global.topology_local.size() > 1)
              && (__global.topology_local.size() == local_sub_topology->size())) /// \todo shmem doesn't seem to work on subnode topologies?
          {
            TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Register Shmem local barrier\n", this));

            _sub_shmem_barrier_composite = _sub_shmem_msync_factory.generate(geometry, &xfer);
            geometry->setKey(_context_id, PAMI::Geometry::PAMI_CKEY_LOCALBARRIERCOMPOSITE,
                             (void*)_sub_shmem_barrier_composite);


            // If the geometry is all local nodes, we can use pure shmem composites.
            if (topology->isLocalToMe())
            {
              TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Register Local Shmem factories\n", this));
              _shmem_barrier_composite = _shmem_msync_factory.generate(geometry, &xfer);


              // Add Barriers
              geometry->addCollective(PAMI_XFER_BARRIER, &_shmem_msync_factory, _context_id);

              // Add Broadcasts
              geometry->addCollective(PAMI_XFER_BROADCAST, &_shmem_mcast_factory, _context_id);

              // Add Allreduces
              geometry->addCollective(PAMI_XFER_ALLREDUCE, &_shmem_mcomb_factory, _context_id);
            }
          }

          // (Maybe) Add rectangle broadcasts
          TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Analyze Rectangle factories %p/%p, isLocal? %u\n", this, _rectangle_1color_dput_broadcast_factory,_rectangle_dput_broadcast_factory,topology->isLocal()));
//        if ((_rectangle_1color_dput_broadcast_factory) && (!topology->isLocal()))
//          do
//          {
//            // make a local copy of the topology
//            PAMI::Topology coord_topology = *(PAMI::Topology*) geometry->getTopology(0);
//            coord_topology.convertTopology(PAMI_COORD_TOPOLOGY);
//            TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Analyze Rectangle type %u\n", this, coord_topology.type()));
//            if (coord_topology.type() != PAMI_COORD_TOPOLOGY) break; //not a coord? then not a line
//
//            PAMI::Topology tmp;
//            topology->subTopologyLocalToMe(&tmp);
//            TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Analyze Rectangle local size %zu\n", this, tmp.size()));
//            if (tmp.size()!=1) break; // local ranks not yet supported
//
//            TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Register Rectangle\n", this));
//            //Add a rectangle broadcasts
//
//          } while (0);

            if (_rectangle_dput_broadcast_factory)
            do 
            {
              // make a local copy of the topology
              PAMI::Topology coord_topology = *(PAMI::Topology*) geometry->getTopology(0);
              coord_topology.convertTopology(PAMI_COORD_TOPOLOGY);
              TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Analyze Rectangle type %u\n", this, coord_topology.type()));
              if (coord_topology.type() != PAMI_COORD_TOPOLOGY) break; //not a coord? then not a line

              TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Register Rectangle\n", this));
              //Add a rectangle broadcasts
              if (_rectangle_1color_dput_broadcast_factory)
                geometry->addCollective(PAMI_XFER_BROADCAST,  _rectangle_1color_dput_broadcast_factory, _context_id);

              geometry->addCollective(PAMI_XFER_BROADCAST,  _rectangle_dput_broadcast_factory, _context_id);
            } while (0);

        }
        else if (phase == 1)
        {

          // If we have > 1 node, check MU
          if (__global.useMU())
          {
            /// Remember, this is 'pure' MU - we won't do any shmem - so only one process per node
            /// on the same T dimension.

            /// A simple check (of sizes) to see if this subgeometry is all global,
            /// then the geometry topology is usable by MU.
            bool usePureMu = topology->size() == master_sub_topology->size() ? true : false;
            TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() usePureMu = %u (size %zu/%zu)\n", this, usePureMu, topology->size(), master_sub_topology->size()));

            void *val;
            val = geometry->getKey(PAMI::Geometry::PAMI_GKEY_MSYNC_CLASSROUTEID);
            TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() PAMI_GKEY_MSYNC_CLASSROUTEID %p\n", this, val));

            if (val && val != PAMI_CR_GKEY_FAIL)
            {
              // Only register protocols if we got a classroute
              _sub_mu_barrier_composite = _sub_mu_msync_factory->generate(geometry, &xfer);

              geometry->setKey(_context_id, PAMI::Geometry::PAMI_CKEY_GLOBALBARRIERCOMPOSITE,
                               (void*)_sub_mu_barrier_composite);

              // If we can use pure MU composites, add them
              if (usePureMu  && !topology->isLocalToMe())
              {
                TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Register MU barrier\n", this));
                _mu_barrier_composite = _mu_msync_factory->generate(geometry, &xfer);


                // Add Barriers
                geometry->addCollective(PAMI_XFER_BARRIER, _mu_msync_factory, _context_id);
              }

              // Add 2 device composite protocols
              if ((local_sub_topology->size() > 1) && (master_sub_topology->size() > 1) && (__global.useshmem()))
              {
                if (_msync2d_composite_factory)
                {
                  TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Register msync 2D\n", this));
                  _msync2d_composite = _msync2d_composite_factory->generate(geometry, &xfer);
                  geometry->addCollective(PAMI_XFER_BARRIER, _msync2d_composite_factory, _context_id);
                }
              }
            }

            val = geometry->getKey(PAMI::Geometry::PAMI_GKEY_MCAST_CLASSROUTEID);
            TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() PAMI_GKEY_MCAST_CLASSROUTEID %p\n", this, val));

            if (val && val != PAMI_CR_GKEY_FAIL)
            {
              // Only register protocols if we got a classroute

              // If we can use pure MU composites, add them
              if (usePureMu && !topology->isLocalToMe())
              {
                TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Register MU bcast\n", this));
                // Add Broadcasts
                //geometry->addCollective(PAMI_XFER_BROADCAST,  _mu_mcast2_factory, _context_id);

                // 'Pure' Axial only makes sense on a line... useless but enabled here for testing
                // Note, while axial doesn't use classroutes it does use msync and expects a classroute for that,
                // should check for that but we'll assume if we got mcast then we got msync too
                /// \todo This could change to geometry->default_barrier() or some other barrier instead of msync to not require classroutes...
                do
                {
                  // make a local copy of the topology
                  PAMI::Topology coord_topology = *(PAMI::Topology*) geometry->getTopology(0);
                  coord_topology.convertTopology(PAMI_COORD_TOPOLOGY);
                  TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Register Axial (line) type %u\n", this, coord_topology.type()));
                  if (coord_topology.type() != PAMI_COORD_TOPOLOGY) break; //not a coord? then not a line

                  pami_coord_t rll;
                  pami_coord_t rur;
                  unsigned char risTorus[PAMI_MAX_DIMS];
                  coord_topology.rectSeg(&rll, &rur, risTorus);
                  TRACE_INIT((stderr,"<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Register Axial (line) "
                              "ll[%zu,%zu,%zu,%zu,%zu], ur[%zu,%zu,%zu,%zu,%zu]\n", this,                             
                              rll.u.n_torus.coords[0],                                 
                              rll.u.n_torus.coords[1],                                 
                              rll.u.n_torus.coords[2],                                 
                              rll.u.n_torus.coords[3],                                 
                              rll.u.n_torus.coords[4],                                
                              rur.u.n_torus.coords[0],                                 
                              rur.u.n_torus.coords[1],                                 
                              rur.u.n_torus.coords[2],                                 
                              rur.u.n_torus.coords[3],                                 
                              rur.u.n_torus.coords[4]));                                
                  unsigned ndimsDifferent=0;
                  for (size_t i=0; i < __global.mapping.globalDims(); ++i)
                    if (rll.u.n_torus.coords[i] != rur.u.n_torus.coords[i]) ++ndimsDifferent;

                  if (ndimsDifferent != 1) break; // only 1 dim different on a line

                  TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Register Axial (line) bcast\n", this));
                  //geometry->addCollective(PAMI_XFER_BROADCAST,  _line_mcast2_factory, _context_id);
                  //geometry->addCollective(PAMI_XFER_BROADCAST,  _line_dput_mcast2_factory, _context_id);
                } while (0);
              }

              // Add 2 device composite protocols
              if ((local_sub_topology->size() > 1) && (master_sub_topology->size() > 1) && (__global.useshmem()))
              {
                if (_mcast2d_composite_factory)
                {
                  TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Register mcast 2D\n", this));
// Doesn't work well enough to be a default protocol
#ifdef ENABLE_NEW_SHMEM
                  geometry->addCollective(PAMI_XFER_BROADCAST, _mcast2d_composite_factory, _context_id);
#endif
                }
              }
            }

            val = geometry->getKey(PAMI::Geometry::PAMI_GKEY_MCOMB_CLASSROUTEID);
            TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() PAMI_GKEY_MCOMB_CLASSROUTEID %p\n", this, val));

            if (val && val != PAMI_CR_GKEY_FAIL)
            {
              // If we can use pure MU composites, add them
              if (usePureMu && !topology->isLocalToMe())
              {
                // Direct MU allreduce only on one context per node (lowest T, context 0)
                if ((__global.mapping.isLowestT()) && (_context_id == 0))
                {
                  // Add Allreduces
                  TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Register MU allreduce\n", this));

                  geometry->addCollective(PAMI_XFER_ALLREDUCE, _mu_mcomb_factory, _context_id);
                }
              }

              // Add 2 device composite protocols
              if ((local_sub_topology->size() > 1) && (master_sub_topology->size() > 1) && (__global.useshmem()))
              {
                if (_mcomb2dNP_composite_factory)
                {
                  TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Register mcomb 2DNP\n", this));
                  geometry->addCollective(PAMI_XFER_ALLREDUCE, _mcomb2dNP_composite_factory, _context_id);
                }

                if (_mcomb2d_composite_factory)
                {
                  TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() Register mcomb 2D\n", this));
// Doesn't work well enough to be a default protocol
#ifdef ENABLE_NEW_SHMEM
                  geometry->addCollective(PAMI_XFER_ALLREDUCE, _mcomb2d_composite_factory, _context_id);
#endif
                }

              }
            }

          }


          // Check if *someone* registered local/global protocols for our geometry
          // before generating any composite protocol...

          if ((local_sub_topology->size() > 1) && (geometry->getKey(_context_id, PAMI::Geometry::PAMI_CKEY_LOCALBARRIERCOMPOSITE) == NULL))
            return PAMI_SUCCESS; // done - we can't do a protocol composite

          if ((master_sub_topology->size() > 1) && (geometry->getKey(_context_id, PAMI::Geometry::PAMI_CKEY_GLOBALBARRIERCOMPOSITE) == NULL))
            return PAMI_SUCCESS; // done - we can't do a protocol composite

          // Add Composite Barrier
          TRACE_INIT((stderr, "<%p>PAMI::CollRegistration::BGQMultiregistration::analyze_impl() register a composite barrier\n", this));
          _msync_composite = _msync_composite_factory.generate(geometry, &xfer);

          geometry->addCollective(PAMI_XFER_BARRIER, &_msync_composite_factory, _context_id);


        }
        else if (phase == -1)
        {

          /// \todo remove MU collectives algorithms... TBD
          geometry->rmCollective(PAMI_XFER_BROADCAST, _mu_mcast2_factory, _context_id);
          geometry->rmCollective(PAMI_XFER_BROADCAST, _line_mcast2_factory, _context_id);
          //geometry->rmCollective(PAMI_XFER_BROADCAST, _line_dput_mcast2_factory, _context_id);
          geometry->rmCollective(PAMI_XFER_ALLREDUCE, _mu_mcomb_factory, _context_id);
          geometry->rmCollective(PAMI_XFER_BARRIER, _mu_msync_factory, _context_id);
        }

        return PAMI_SUCCESS;
      }
    public:
      pami_client_t                                   _client;
      pami_context_t                                  _context;
      size_t                                          _context_id;
      // This is a pointer to the current dispatch id of the context
      // This will be decremented by the ConstructNativeInterface routines
      int                                            *_dispatch_id;
      std::map<unsigned, pami_geometry_t>            *_geometry_map;

      // CCMI Connection Manager Class
      CCMI::ConnectionManager::SimpleConnMgr          _sconnmgr;
      CCMI::ConnectionManager::CommSeqConnMgr         _csconnmgr;
      CCMI::ConnectionManager::ColorGeometryConnMgr   _cg_connmgr;
      CCMI::ConnectionManager::ColorConnMgr           _color_connmgr;

      // Barrier Storage
      CCMI::Executor::Composite                      *_shmem_barrier_composite;
      CCMI::Executor::Composite                      *_sub_shmem_barrier_composite;
      CCMI::Executor::Composite                      *_mu_barrier_composite;
      CCMI::Executor::Composite                      *_sub_mu_barrier_composite;
      CCMI::Executor::Composite                      *_msync_composite;
      CCMI::Executor::Composite                      *_msync2d_composite;

      //* SHMEM interfaces:
      // Native Interface
      T_ShmemNativeInterface                         *_shmem_ni;

      // CCMI Barrier Interface
      ShmemMultiSyncFactory                           _shmem_msync_factory;
      SubShmemMultiSyncFactory                        _sub_shmem_msync_factory;

      // CCMI Broadcast Interfaces
      ShmemMultiCastFactory                           _shmem_mcast_factory;

      // CCMI Allreduce Interface
      ShmemMultiCombineFactory                        _shmem_mcomb_factory;

      // MU Device
      T_MUDevice                                     &_mu_device;

      // MU Native Interface

      T_MUNativeInterface                            *_mu_ni_msync;
      uint8_t                                         _mu_ni_msync_storage[sizeof(T_MUNativeInterface)];
      T_MUNativeInterface                            *_mu_ni_sub_msync;
      uint8_t                                         _mu_ni_sub_msync_storage[sizeof(T_MUNativeInterface)];
      T_MUNativeInterface                            *_mu_ni_mcomb;
      uint8_t                                         _mu_ni_mcomb_storage[sizeof(T_MUNativeInterface)];
      T_MUNativeInterface                            *_mu_ni_mcast2;
      uint8_t                                         _mu_ni_mcast2_storage[sizeof(T_MUNativeInterface)];
      T_MUNativeInterface                            *_mu_ni_msync2d;
      uint8_t                                         _mu_ni_msync2d_storage[sizeof(T_MUNativeInterface)];
      T_MUNativeInterface                            *_mu_ni_mcast2d;
      uint8_t                                         _mu_ni_mcast2d_storage[sizeof(T_MUNativeInterface)];
      T_MUNativeInterface                            *_mu_ni_mcomb2d;
      uint8_t                                         _mu_ni_mcomb2d_storage[sizeof(T_MUNativeInterface)];
      T_MUNativeInterface                            *_mu_ni_mcomb2dNP;
      uint8_t                                         _mu_ni_mcomb2dNP_storage[sizeof(T_MUNativeInterface)];
      T_AxialNativeInterface                         *_axial_mu_ni;
      uint8_t                                         _axial_mu_ni_storage[sizeof(T_AxialNativeInterface)];
      T_AxialNativeInterface                         *_axial_mu_1_ni;
      uint8_t                                         _axial_mu_1_ni_storage[sizeof(T_AxialNativeInterface)];

      T_AxialDputNativeInterface                     *_axial_dput_mu_ni;
      uint8_t                                         _axial_dput_mu_ni_storage[sizeof(T_AxialDputNativeInterface)];
      //T_AxialDputNativeInterface                     *_axial_dput_mu_1_ni;
      //uint8_t                                         _axial_dput_mu_1_ni_storage[sizeof(T_AxialDputNativeInterface)];

      // Barrier factories
      MUMultiSyncFactory                             *_mu_msync_factory;
      uint8_t                                         _mu_msync_factory_storage[sizeof(MUMultiSyncFactory)];
      SubMUMultiSyncFactory                          *_sub_mu_msync_factory;
      uint8_t                                         _sub_mu_msync_factory_storage[sizeof(SubMUMultiSyncFactory)];

      // Broadcast factories
      MUMultiCast2Factory                            *_mu_mcast2_factory;
      uint8_t                                         _mu_mcast2_factory_storage[sizeof(MUMultiCast2Factory)];
      LineMultiCast2Factory                          *_line_mcast2_factory;
      uint8_t                                         _line_mcast2_factory_storage[sizeof(LineMultiCast2Factory)];
      //LineMultiCast2Factory                          *_line_dput_mcast2_factory;
      //uint8_t                                         _line_dput_mcast2_factory_storage[sizeof(LineMultiCast2Factory)];

      // Allreduce factories
      MUMultiCombineFactory                          *_mu_mcomb_factory;
      uint8_t                                         _mu_mcomb_factory_storage[sizeof(MUMultiCombineFactory)];

      // Barrier factories
      MultiSync2Factory                               _msync_composite_factory;

      // 2 device composite factories
      CCMI::Interfaces::NativeInterface              *_ni_array[4];
      MultiSync2DeviceFactory                        *_msync2d_composite_factory;
      uint8_t                                         _msync2d_composite_factory_storage[sizeof(MultiSync2DeviceFactory)];

      MultiCast2DeviceFactory                        *_mcast2d_composite_factory;
      uint8_t                                         _mcast2d_composite_factory_storage[sizeof(MultiCast2DeviceFactory)];

      MultiCombine2DeviceFactory                     *_mcomb2d_composite_factory;
      uint8_t                                         _mcomb2d_composite_factory_storage[sizeof(MultiCombine2DeviceFactory)];

      MultiCombine2DeviceFactoryNP                   *_mcomb2dNP_composite_factory;
      uint8_t                                         _mcomb2dNP_composite_factory_storage[sizeof(MultiCombine2DeviceFactoryNP)];

      // 2 device/NI composite
      RectangleDput1ColorBroadcastFactory            *_rectangle_1color_dput_broadcast_factory;
      uint8_t                                         _rectangle_1color_dput_broadcast_factory_storage[sizeof(RectangleDput1ColorBroadcastFactory)];

      RectangleDputBroadcastFactory                  *_rectangle_dput_broadcast_factory;
      uint8_t                                         _rectangle_dput_broadcast_factory_storage[sizeof(RectangleDputBroadcastFactory)];
    };



  };
};

#ifndef CCMI_TRACE_ALL
  #undef TRACE_INIT
  #define TRACE_INIT(x)
#endif

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
