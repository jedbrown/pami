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

      template < bool T_inline=false, class T_Native=Interfaces::NativeInterface, PAMI::Geometry::topologyIndex_t T_Geometry_Index = PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX>

      class MultiSyncComposite : public CCMI::Executor::Composite
      {
        protected:
	Interfaces::NativeInterface        * _native;
	//PAMI_GEOMETRY_CLASS                * _geometry;
	pami_multisync_t                     _minfo;
	void                               * _deviceInfo;

        public:
          MultiSyncComposite (Interfaces::NativeInterface          * native,
                              ConnectionManager::SimpleConnMgr     * cmgr,
                              pami_geometry_t                         g,
                              void                                 * cmd,
                              pami_event_function                     fn,
                              void                                 * cookie) :
	  Composite(), _native(native) //, _geometry((PAMI_GEOMETRY_CLASS*)g)
          {
            TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));

	    PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *)g;
            _deviceInfo          = geometry->getKey(PAMI::Geometry::GKEY_MSYNC_CLASSROUTEID);
	    
            _minfo.cb_done.function   = NULL;
            _minfo.cb_done.clientdata = NULL;
            _minfo.connection_id      = 0;
            _minfo.roles              = -1U;
            _minfo.participants       = geometry->getTopology(T_Geometry_Index);	                
          }
	
	  ///Barrier composite is created and cached
	  virtual void start()
          {	    
            TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));
	    _minfo.cb_done.function   = _cb_done;
            _minfo.cb_done.clientdata = _clientdata;
	    if (T_inline) {
	      T_Native *t_native = (T_Native *)_native;
	      t_native->T_Native::multisync (&_minfo, _deviceInfo);
	    }
	    else 	      
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
            setup(_native_l,
                  _native_g,
                  cmgr,
                  g,
                  cmd,
                  fn,
                  cookie);
          }


          MultiSyncComposite2Device (Interfaces::NativeInterface      *mInterfaceL,
                                     Interfaces::NativeInterface      *mInterfaceG,
                                     ConnectionManager::SimpleConnMgr *cmgr,
                                     pami_geometry_t                   g,
                                     void                             *cmd,
                                     pami_event_function               fn,
                                     void                             *cookie) :
              Composite(),
              _native_l(mInterfaceL),
              _native_g(mInterfaceG),
              _geometry((PAMI_GEOMETRY_CLASS*)g),
              _deviceInfo(NULL)
          {
            setup(_native_l,
                  _native_g,
                  cmgr,
                  g,
                  cmd,
                  fn,
                  cookie);
          }

          void setup(Interfaces::NativeInterface      *mInterfaceL,
                Interfaces::NativeInterface      *mInterfaceG,
                ConnectionManager::SimpleConnMgr *cmgr,
                pami_geometry_t                   g,
                void                             *cmd,
                pami_event_function               fn,
                void                             *cookie) 
          {
            PAMI::Topology  *t_master    = (PAMI::Topology*)_geometry->getTopology(PAMI::Geometry::MASTER_TOPOLOGY_INDEX);
            PAMI::Topology  *t_local     = (PAMI::Topology*)_geometry->getTopology(PAMI::Geometry::LOCAL_TOPOLOGY_INDEX);

	    _g_reset_cb                   = false;
	    _l0_reset_cb                  = false;
	    _l1_reset_cb                  = false;

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
                _minfo_l0.cb_done.function   =  NULL; //fn;
                _minfo_l0.cb_done.clientdata =  NULL; //cookie;
                _active_native               =  _native_l;
                _active_minfo                = &_minfo_l0;
              }

            // If we have more than one master, but we are the only local process
            // we are guaranteed to be a "local master", so we will just
            // issue the collective on the global device
            _deviceInfo = _geometry->getKey(PAMI::Geometry::GKEY_MSYNC_CLASSROUTEID);

            if (t_master->size() > 1 && t_local->size() == 1)
              {
                _minfo_g.cb_done.function    =  NULL; //fn;
                _minfo_g.cb_done.clientdata  =  NULL; //cookie;
                _active_native               =  _native_g;
                _active_minfo                = &_minfo_g;
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

                _minfo_l1.cb_done.function   = NULL; //fn;
                _minfo_l1.cb_done.clientdata = NULL; //cookie;
                _active_native               =  _native_l;
                _active_minfo                = &_minfo_l0;
              }

	    if (_minfo_g.cb_done.function == NULL) 
	      _g_reset_cb = true;
	    
	    if (_minfo_l0.cb_done.function == NULL) 
	      _l0_reset_cb = true;

	    if (_minfo_l1.cb_done.function == NULL) 
	      _l1_reset_cb = true;
          }

          virtual void start()
          {
	    if (_g_reset_cb) {
	      _minfo_g.cb_done.function   = _cb_done;
	      _minfo_g.cb_done.clientdata = _clientdata;
	    }	    
	    if (_l0_reset_cb) {
	      _minfo_l0.cb_done.function   = _cb_done;
	      _minfo_l0.cb_done.clientdata = _clientdata;
	    }	    
	    if (_l1_reset_cb) {
	      _minfo_l1.cb_done.function   = _cb_done;
	      _minfo_l1.cb_done.clientdata = _clientdata;
	    }	    

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
	  bool                                _g_reset_cb;
	  bool                                _l0_reset_cb;
	  bool                                _l1_reset_cb;
          pami_multisync_t                   *_active_minfo;
      };


      class MultiSync2Composite : public CCMI::Executor::Composite
      {

          static void local_done_fn(pami_context_t  context,
                                    void           *cookie,
                                    pami_result_t   result )
          {
            TRACE_ADAPTOR((stderr, "<%p> MultiSync2Composite::local_done_fn()\n", cookie));
            MultiSync2Composite *m = (MultiSync2Composite*) cookie;
            m->_global_barrier->start();
          }

          static void global_done_fn(pami_context_t  context,
                                     void           *cookie,
                                     pami_result_t   result )
          {
            TRACE_ADAPTOR((stderr, "<%p> MultiSync2Composite::global_done_fn()\n", cookie));
            MultiSync2Composite *m = (MultiSync2Composite*) cookie;
            // next local barrier done is the user's done
            m->_local_barrier->setDoneCallback(m->_cb_done.function, m->_cb_done.clientdata);
            m->_local_barrier->start();
          }

        public:
          MultiSync2Composite (Interfaces::NativeInterface                         *mInterface,
                               ConnectionManager::SimpleConnMgr              *cmgr,
                               pami_geometry_t                                g,
                               void                                          *cmd,
                               pami_event_function                            fn,
                               void                                          *cookie) :
              Composite(),
              _local_barrier(NULL),
              _global_barrier(NULL),
              _active_barrier(NULL),
              _final_barrier(NULL),
              _geometry((PAMI_GEOMETRY_CLASS*)g)
          {
            _cb_done.function   = fn;
            _cb_done.clientdata = cookie;

            TRACE_ADAPTOR((stderr, "<%p> MultiSync2Composite (_cb_done.function %p, _cb_done.clientdata %p)\n", this, _cb_done.function, _cb_done.clientdata));
            PAMI::Topology  *t_master    = (PAMI::Topology*)_geometry->getTopology(PAMI::Geometry::MASTER_TOPOLOGY_INDEX);
            PAMI::Topology  *t_local     = (PAMI::Topology*)_geometry->getTopology(PAMI::Geometry::LOCAL_TOPOLOGY_INDEX);

            _local_barrier  = (CCMI::Executor::Composite *)_geometry->getKey((size_t)0, /// \todo does NOT support multicontext
                                                                             PAMI::Geometry::CKEY_LOCALBARRIERCOMPOSITE);
            _global_barrier = (CCMI::Executor::Composite *)_geometry->getKey((size_t)0, /// \todo does NOT support multicontext
                                                                             PAMI::Geometry::CKEY_GLOBALBARRIERCOMPOSITE);

            // If the global "master" topology has only one rank, the local barrier will
            // suffice to implement the barrier
            if (t_master->size() == 1 && t_local->size() != 1)
              {
                _final_barrier = _active_barrier  =  _local_barrier;
                return;
              }

            // If we have more than one master, but we are the only local process
            // we are guaranteed to be a "local master", so we will just
            // issue the collective on the global device

            if (t_master->size() > 1 && t_local->size() == 1)
              {
                _final_barrier = _active_barrier  =  _global_barrier;
                return;
              }

            // We have a mix of both local nodes and master nodes
            // We need to determine if we are the master.
            // If this node is the master, then it participates in
            // the standard local, global, local flow.  If the node
            // is not the master, it participates in only the
            // local, local flow.

            // _final_barrier will not be used - _global_done_fn will set
            // the final cb_done for the last local barrier.

            if (t_master->size() > 1 && t_local->size() > 1)
              {
                if (_geometry->isLocalMasterParticipant())
                  {
                    _local_barrier->setDoneCallback(local_done_fn, this);
                    _global_barrier->setDoneCallback(global_done_fn, this);
                  }
                else
                  {
                    _local_barrier->setDoneCallback(global_done_fn, this);
                  }

                _active_barrier               =  _local_barrier;
                return;
              }

          }
          virtual void start()
          {
            TRACE_ADAPTOR((stderr, "<%p> MultiSync2Composite::start() _cb_done.function %p, _cb_done.clientdata %p\n", this, _cb_done.function, _cb_done.clientdata));

            if (_final_barrier) _final_barrier->setDoneCallback(_cb_done.function, _cb_done.clientdata);

            return _active_barrier->start();
          }
        protected:
          CCMI::Executor::Composite          *_local_barrier;  // local shmem barrier
          CCMI::Executor::Composite          *_global_barrier; // glocal barrier
          CCMI::Executor::Composite          *_active_barrier; // active barrier to start
          CCMI::Executor::Composite          *_final_barrier;  // barrier which will call final cb_done
          PAMI_GEOMETRY_CLASS                *_geometry;
          pami_callback_t                     _cb_done;   /**< User's completion callback */
      };

    };
  };
};

#endif
