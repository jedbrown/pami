/**
 * \file algorithms/protocols/barrier/MultiSyncComposite.h
 * \brief Simple composite based on multisync
 */
#ifndef __algorithms_protocols_barrier_MultiSyncComposite_h__
#define __algorithms_protocols_barrier_MultiSyncComposite_h__

#include "algorithms/executor/Barrier.h"
#include "algorithms/composite/Composite.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Barrier
    {

      class MultiSyncComposite : public CCMI::Executor::Composite
      {
        protected:
          Interfaces::NativeInterface        * _native;
          PAMI_GEOMETRY_CLASS                * _geometry;
          pami_multisync_t                     _minfo;
          void                               * _deviceInfo;

        public:
          MultiSyncComposite (Interfaces::NativeInterface          * mInterface,
                              ConnectionManager::SimpleConnMgr     * cmgr,
                              pami_geometry_t                         g,
                              void                                 * cmd,
                              pami_event_function                     fn,
                              void                                 * cookie) :
              Composite(), _native(mInterface), _geometry((PAMI_GEOMETRY_CLASS*)g)
          {
            TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));

            _deviceInfo                  = _geometry->getKey(PAMI::Geometry::PAMI_GKEY_MSYNC_CLASSROUTEID);
            //_minfo.cb_done.function   = _cb_done;
            //_minfo.cb_done.clientdata = _clientdata;
            _minfo.connection_id      = 0;
            _minfo.roles              = -1U;
            _minfo.participants       = _geometry->getTopology(0);

          }

          virtual void start()
          {
            TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));
            _minfo.cb_done.function   = _cb_done;
            _minfo.cb_done.clientdata = _clientdata;
            _native->multisync(&_minfo, _deviceInfo);
          }
      };

      class MultiSyncComposite2Device : public CCMI::Executor::Composite
      {

          static void local_done_fn(pami_context_t  context,
                                    void           *cookie,
                                    pami_result_t   result )
          {
            MultiSyncComposite2Device *m = (MultiSyncComposite2Device*) cookie;
            m->_native_g->multisync(&m->_minfo_g, m->_deviceInfo);
          }

          static void global_done_fn(pami_context_t  context,
                                     void           *cookie,
                                     pami_result_t   result )
          {
            MultiSyncComposite2Device *m = (MultiSyncComposite2Device*) cookie;
            m->_native_l->multisync(&m->_minfo_l1, m->_deviceInfo);
          }

        public:
          MultiSyncComposite2Device (Interfaces::NativeInterface      *mInterface,
                                     ConnectionManager::SimpleConnMgr *cmgr,
                                     pami_geometry_t                   g,
                                     void                             *cmd,
                                     pami_event_function               fn,
                                     void                             *cookie) :
              Composite(),
              // This is a small hack to get around making a whole new set of factories
              // and classes for a 2 device NI.  We just treat the first parameter like a
              // 2 element pointer array.
              _native_l((Interfaces::NativeInterface*)((void **)mInterface)[0]),
              _native_g((Interfaces::NativeInterface*)((void **)mInterface)[1]),
              _geometry((PAMI_GEOMETRY_CLASS*)g),
              _deviceInfo(NULL)
          {
            PAMI::Topology  *t_master    = (PAMI::Topology*)_geometry->getLocalMasterTopology();
            PAMI::Topology  *t_local     = (PAMI::Topology*)_geometry->getLocalTopology();

            _minfo_l0.connection_id      = 0;
            _minfo_l0.roles              = -1U;
            _minfo_l0.participants       = (pami_topology_t*)t_local;

            _minfo_g.connection_id       = 0;
            _minfo_g.roles               = -1U;
            _minfo_g.participants        = (pami_topology_t*)t_master;

            _minfo_l1.connection_id      = 0;
            _minfo_l1.roles              = -1U;
            _minfo_l1.participants       = (pami_topology_t*)t_local;

            // If the global "master" topology has only one rank, the local barrier will
            // suffice to implement the barrier
            if (t_master->size() == 1 && t_local->size() != 1)
              {
                _minfo_l0.cb_done.function   =  fn;
                _minfo_l0.cb_done.clientdata =  cookie;
                _active_native               =  _native_l;
                _active_minfo                = &_minfo_l0;
                return;
              }

            // If we have more than one master, but we are the only local process
            // we are guaranteed to be a "local master", so we will just
            // issue the collective on the global device
            _deviceInfo = _geometry->getKey(PAMI::Geometry::PAMI_GKEY_MSYNC_CLASSROUTEID);

            if (t_master->size() > 1 && t_local->size() == 1)
              {
                _minfo_g.cb_done.function    =  fn;
                _minfo_g.cb_done.clientdata  =  cookie;
                _active_native               =  _native_g;
                _active_minfo                = &_minfo_g;
                return;
              }

            // We have a mix of both local nodes and master nodes
            // We need to determine if we are the master.
            // If this node is the master, then it participates in
            // the standard local, global, local flow.  If the node
            // is not the master, it participates in only the
            // local, local flow.
            if (t_master->size() > 1 && t_local->size() > 1)
              {
                if (_geometry->isLocalMasterParticipant())
                  {
                    _minfo_l0.cb_done.function   = local_done_fn;
                    _minfo_l0.cb_done.clientdata = this;
                    _minfo_g.cb_done.function    = global_done_fn;
                    _minfo_g.cb_done.clientdata  = this;
                  }
                else
                  {
                    _minfo_l0.cb_done.function   = global_done_fn;
                    _minfo_l0.cb_done.clientdata = this;
                  }

                _minfo_l1.cb_done.function   = fn;
                _minfo_l1.cb_done.clientdata = cookie;
                _active_native               =  _native_l;
                _active_minfo                = &_minfo_l0;
                return;
              }

            PAMI_assert(0);
          }
          virtual void start()
          {
            _active_native->multisync(_active_minfo, _deviceInfo);
          }
        protected:
          Interfaces::NativeInterface        *_native_l;
          Interfaces::NativeInterface        *_native_g;
          Interfaces::NativeInterface        *_active_native;
          PAMI_GEOMETRY_CLASS                *_geometry;
          void                               *_deviceInfo;
          pami_multisync_t                    _minfo_l0;
          pami_multisync_t                    _minfo_g;
          pami_multisync_t                    _minfo_l1;
          pami_multisync_t                   *_active_minfo;
      };
    };
  };
};

#endif
