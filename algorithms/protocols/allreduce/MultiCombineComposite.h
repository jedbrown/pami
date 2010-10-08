/**
 * \file algorithms/protocols/allreduce/MultiCombineComposite.h
 * \brief Simple composite based on multicombine
 */
#ifndef __algorithms_protocols_allreduce_MultiCombineComposite_h__
#define __algorithms_protocols_allreduce_MultiCombineComposite_h__

#include "algorithms/composite/Composite.h"
#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "algorithms/protocols/allreduce/ReduceFunctions.h"

#include "util/ccmi_debug.h"
#include "util/ccmi_util.h"


#ifndef CCMI_TRACE_ALL
#undef TRACE_ADAPTOR
#define TRACE_ADAPTOR(x) //fprintf x
#endif


#ifdef TRACE
#undef TRACE
#define TRACE(x) //fprintf x
#else
#define TRACE(x) //fprintf x
#endif


// Use a local done function for testing
#define LOCAL_TEST

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {
      class MultiCombineComposite : public CCMI::Executor::Composite
      {
        protected:
          Interfaces::NativeInterface        * _native;
          PAMI_GEOMETRY_CLASS                * _geometry;
          pami_allreduce_t                     _xfer_allreduce;
          PAMI::PipeWorkQueue                  _srcPwq;
          PAMI::PipeWorkQueue                  _dstPwq;
          pami_multicombine_t                  _minfo;
          void                               * _deviceInfo;

        public:
          MultiCombineComposite (Interfaces::NativeInterface          * mInterface,
                                 ConnectionManager::SimpleConnMgr     * cmgr,
                                 pami_geometry_t                        g,
                                 pami_xfer_t                          * cmd,
                                 pami_event_function                    fn,
                                 void                                 * cookie) :
              Composite(), _native(mInterface), _geometry((PAMI_GEOMETRY_CLASS*)g), _xfer_allreduce(cmd->cmd.xfer_allreduce)
          {
            TRACE_ADAPTOR((stderr, "%s, type %#zX/%#zX, count %zu/%zu, op %#X, dt %#X\n", __PRETTY_FUNCTION__,
                           (size_t)cmd->cmd.xfer_allreduce.stype, (size_t)cmd->cmd.xfer_allreduce.rtype,
                           cmd->cmd.xfer_allreduce.stypecount, cmd->cmd.xfer_allreduce.rtypecount, cmd->cmd.xfer_allreduce.op, cmd->cmd.xfer_allreduce.dt));


            /// \todo only supporting PAMI_BYTE right now
            PAMI_assertf((cmd->cmd.xfer_allreduce.stype == PAMI_BYTE) && (cmd->cmd.xfer_allreduce.rtype == PAMI_BYTE), "Not PAMI_BYTE? %#zX %#zX\n", (size_t)cmd->cmd.xfer_allreduce.stype, (size_t)cmd->cmd.xfer_allreduce.rtype);

//          PAMI_Type_sizeof(cmd->cmd.xfer_allreduce.stype); /// \todo PAMI_Type_sizeof() is PAMI_UNIMPL so use getReduceFunction for now?

            _deviceInfo                  = _geometry->getKey(PAMI::Geometry::GKEY_MCOMB_CLASSROUTEID);

            unsigned        sizeOfType;
            coremath        func;

            getReduceFunction(cmd->cmd.xfer_allreduce.dt,
                              cmd->cmd.xfer_allreduce.op,
                              cmd->cmd.xfer_allreduce.stypecount,// this parm is unused
                              sizeOfType,
                              func );
            size_t size = cmd->cmd.xfer_allreduce.stypecount * 1; /// \todo presumed size of PAMI_BYTE is 1?
            _srcPwq.configure(NULL, cmd->cmd.xfer_allreduce.sndbuf, size, size);
            _srcPwq.reset();

            size = cmd->cmd.xfer_allreduce.rtypecount * 1; /// \todo presumed size of PAMI_BYTE is 1?
            _dstPwq.configure(NULL, cmd->cmd.xfer_allreduce.rcvbuf, size, 0);
            _dstPwq.reset();

            DO_DEBUG(PAMI::Topology all);
            DO_DEBUG(all = *(PAMI::Topology*)_geometry->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX));

            DO_DEBUG(for (unsigned j = 0; j < all.size(); ++j) fprintf(stderr, "all[%u]=%zu, size %zu\n", j, (size_t)all.index2Rank(j), all.size()));

            _minfo.client               = 0;
            _minfo.context              = 0; /// \todo ?
            //_minfo.cb_done.function     = _cb_done;
            //_minfo.cb_done.clientdata   = _clientdata;
            _minfo.connection_id        = 0;
            _minfo.roles                = -1U;
            _minfo.results_participants = _geometry->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX);
            _minfo.data_participants    = _geometry->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX);
            _minfo.data                 = (pami_pipeworkqueue_t *) & _srcPwq;
            _minfo.results              = (pami_pipeworkqueue_t *) & _dstPwq;
            _minfo.optor                = cmd->cmd.xfer_allreduce.op;
            _minfo.dtype                = cmd->cmd.xfer_allreduce.dt;
            _minfo.count                = size / sizeOfType;
            TRACE_ADAPTOR((stderr, "%s, count %zu\n", __PRETTY_FUNCTION__, _minfo.count));
          }

          virtual void start()
          {
            TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));
            _minfo.cb_done.function     = _cb_done;
            _minfo.cb_done.clientdata   = _clientdata;
            _native->multicombine(&_minfo, _deviceInfo);
          }
      };

      /// \brief This is a factory for a 2 device MultiCombine
      /// The two device MultiCombine takes two native interfaces, the first is
      /// the local interface, and the second is the global interface
      template <class T_Composite, MetaDataFn get_metadata, class T_Connmgr>
      class MultiCombineComposite2DeviceFactoryT: public CollectiveProtocolFactory
      {
          class collObj
          {
            public:
              collObj(Interfaces::NativeInterface          *native0,
                      Interfaces::NativeInterface          *native1,
                      T_Connmgr                            *cmgr,
                      pami_geometry_t                       geometry,
                      pami_xfer_t                          *cmd,
                      pami_event_function                   fn,
                      void                                 *cookie,
                      MultiCombineComposite2DeviceFactoryT *factory):
                  _factory(factory),
                  _user_done_fn(cmd->cb_done),
                  _user_cookie(cmd->cookie),
                  _obj(native0, native1, cmgr, geometry, cmd, fn, cookie)
              {
              }
              void done_fn( pami_context_t   context,
                            pami_result_t    result )
              {
                _user_done_fn(context, _user_cookie, result);
              }

              MultiCombineComposite2DeviceFactoryT *_factory;
              pami_event_function                   _user_done_fn;
              void                                 *_user_cookie;
              T_Composite                           _obj;
              unsigned                              _connection_id;
          };
        public:
          MultiCombineComposite2DeviceFactoryT (T_Connmgr                   *cmgr,
                                                Interfaces::NativeInterface *native_l,
                                                Interfaces::NativeInterface *native_g):
              CollectiveProtocolFactory(),
              _cmgr(cmgr),
              _native_l(native_l),
              _native_g(native_g)
          {
          }

          virtual ~MultiCombineComposite2DeviceFactoryT()
          {
          }
          virtual void metadata(pami_metadata_t *mdata)
          {
            get_metadata(mdata);
          }
          static void done_fn(pami_context_t  context,
                              void           *clientdata,
                              pami_result_t   res)
          {
            collObj *cobj = (collObj *)clientdata;
            cobj->done_fn(context, res);
            cobj->_factory->_alloc.returnObject(cobj);
          }
          virtual Executor::Composite * generate(pami_geometry_t  geometry,
                                                 void            *cmd)
          {
            collObj *cobj = (collObj*) _alloc.allocateObject();
            TRACE_ADAPTOR((stderr, "<%p>CollectiveProtocolFactoryT::generate()\n", cobj));
            new(cobj) collObj(_native_l,          // Native interface local
                              _native_g,          // Native Interface global
                              _cmgr,              // Connection Manager
                              geometry,           // Geometry Object
                              (pami_xfer_t*)cmd,  // Parameters
                              done_fn,            // Intercept function
                              cobj,               // Intercept cookie
                              this);
            return (Executor::Composite *)&cobj->_obj;

          }
        protected:
          T_Connmgr                                       *_cmgr;
          Interfaces::NativeInterface                     *_native_l;
          Interfaces::NativeInterface                     *_native_g;
          PAMI::MemoryAllocator < sizeof(collObj), 16 >    _alloc;
      };

      /// \brief All sided allreduce over active message multicombines
      /// A local device will chain into a global multicombine
      template <int ReduceOnly>
      class MultiCombineComposite2Device : public CCMI::Executor::Composite
      {
        public:
          static void composite_done(pami_context_t  context,
                                     void           *cookie,
                                     pami_result_t   result)
          {
            MultiCombineComposite2Device *m = (MultiCombineComposite2Device*) cookie;
            m->_count--;
            TRACE((stderr, "MultiCombineComposite2Device:  composite done:  count=%ld\n", m->_count));

            if (m->_count == 0)
              m->_user_done.function(context, m->_user_done.clientdata, result);
          }

          MultiCombineComposite2Device (Interfaces::NativeInterface      *native_l,
                                        Interfaces::NativeInterface      *native_g,
                                        ConnectionManager::SimpleConnMgr *cmgr,
                                        pami_geometry_t                   g,
                                        pami_xfer_t                      *cmd,
                                        pami_event_function               fn,
                                        void                             *cookie) :
              Composite(),
              _native_l(native_l),
              _native_g(native_g),
              _geometry((PAMI_GEOMETRY_CLASS*)g)
          {
            PAMI::Topology  *t_master    = (PAMI::Topology*)_geometry->getTopology(PAMI::Geometry::MASTER_TOPOLOGY_INDEX);
            PAMI::Topology  *t_local     = (PAMI::Topology*)_geometry->getTopology(PAMI::Geometry::LOCAL_TOPOLOGY_INDEX);
            PAMI::Topology  *t_my_master = (PAMI::Topology*)_geometry->getTopology(PAMI::Geometry::LOCAL_MASTER_TOPOLOGY_INDEX);
            bool             amMaster    = _geometry->isLocalMasterParticipant();
            _deviceInfo                  = _geometry->getKey(PAMI::Geometry::GKEY_MCOMB_CLASSROUTEID);
            // todo:  shared mem may need its own devinfo
            unsigned        typesize;
            coremath        func;
            getReduceFunction(cmd->cmd.xfer_allreduce.dt,
                              cmd->cmd.xfer_allreduce.op,
                              cmd->cmd.xfer_allreduce.stypecount,
                              typesize,
                              func);
            size_t           sbytes      = cmd->cmd.xfer_allreduce.stypecount * 1; // todo add type
            size_t           scountDt    = sbytes / typesize;

            TRACE((stderr, "MultiCombineComposite2Device:  In Composite Constructor, setting up PWQ's %p %p, sbytes=%ld buf=%p\n",
                   &_pwq_src, &_pwq_dest, sbytes, cmd->cmd.xfer_allreduce.sndbuf));

            // Create a "flat pwq" for the send buffer
            _pwq_src.configure(NULL,                            // Memory manager
                               cmd->cmd.xfer_allreduce.sndbuf,  // buffer
                               sbytes,                          // buffer bytes
                               sbytes);                         // amount initially in buffer
            _pwq_dest.configure(NULL,                            // Memory manager
                                cmd->cmd.xfer_allreduce.rcvbuf,  // buffer
                                sbytes,                          // buffer bytes
                                0);                              // amount initially in buffer
            _user_done.clientdata = cmd->cookie;
            _user_done.function   = cmd->cb_done;
            _pwq_src.reset();
            _pwq_dest.reset();

            // The "Only Local" case
            // This means the geometry only contains local tasks
            // A single multicombine will suffice to handle the allreduce
            if (t_local->size() == _geometry->size() && amMaster)
              {
                _pwq_inter0.configure(NULL,                            // Memory manager
                                      cmd->cmd.xfer_allreduce.rcvbuf,  // buffer
                                      sbytes,                          // buffer bytes
                                      0);                              // amount initially in buffer
                _pwq_inter0.reset();
                _mcombine_l.client               = 0; /// \todo ?
                _mcombine_l.context              = 0; /// \todo ?
                _mcombine_l.cb_done.clientdata   = this;
                _mcombine_l.cb_done.function     = composite_done;
                _mcombine_l.connection_id        = _geometry->comm();
                _mcombine_l.roles                = -1U;
                _mcombine_l.data                 = (pami_pipeworkqueue_t*) & _pwq_src;
                _mcombine_l.data_participants    = (pami_topology_t*)t_local;
                _mcombine_l.results              = (pami_pipeworkqueue_t*) & _pwq_inter0;
                _mcombine_l.results_participants = (pami_topology_t*)t_my_master;
                _mcombine_l.optor                = cmd->cmd.xfer_allreduce.op;
                _mcombine_l.dtype                = cmd->cmd.xfer_allreduce.dt;
                _mcombine_l.count                = scountDt;

                // Also, prepare the local multicast, with the root as the local master
                // We are guaranteed to not have an early arrival with this multicast if we post
                // it first.  This protocol expects the local multicast to be 2 sided
                _mcast_l.client                  = 0; /// \todo ?
                _mcast_l.context                 = 0; /// \todo ?
                _mcast_l.cb_done.function        = composite_done;
                _mcast_l.cb_done.clientdata      = this;
                _mcast_l.connection_id           = _geometry->comm();
                _mcast_l.roles                   = -1U;
                _mcast_l.bytes                   = cmd->cmd.xfer_allreduce.rtypecount;
                _mcast_l.src                     = (pami_pipeworkqueue_t*) & _pwq_inter0;
                _mcast_l.src_participants        = (pami_topology_t*)t_my_master;
                _mcast_l.dst                     = (pami_pipeworkqueue_t*) & _pwq_dest;
                _mcast_l.dst_participants        = (pami_topology_t*)t_local;
                _mcast_l.msginfo                 = 0;
                _mcast_l.msgcount                = 0;
                _startFcn                        = &MultiCombineComposite2Device::start2;
                _count                           = 2;

                TRACE((stderr, "<%p>Local Only MASTER Setting up start2: local native() %p\n", this, _native_l));
                return;
              }

            // The "Only Global Master" case
            // My task only belongs to a global master, with no other local task
            // A single multicombine will suffice in this case as well
            // To detect this case, the local size will be only me
            if (t_local->size() == 1)
              {
                _mcombine_g.client               = 0; /// \todo ?
                _mcombine_g.context              = 0; /// \todo ?
                _mcombine_g.cb_done.clientdata   = this;
                _mcombine_g.cb_done.function     = composite_done;
                _mcombine_g.connection_id        = _geometry->comm();
                _mcombine_g.roles                = -1U;
                _mcombine_g.data                 = (pami_pipeworkqueue_t*) & _pwq_src;
                _mcombine_g.data_participants    = (pami_topology_t*)t_master;
                _mcombine_g.results              = (pami_pipeworkqueue_t*) & _pwq_dest;
                _mcombine_g.results_participants = (pami_topology_t*)t_master;
                _mcombine_g.optor                = cmd->cmd.xfer_allreduce.op;
                _mcombine_g.dtype                = cmd->cmd.xfer_allreduce.dt;
                _mcombine_g.count                = scountDt;
                _startFcn                        = &MultiCombineComposite2Device::start1;
                _count                           = 1;
                TRACE((stderr, "<%p>Global Only Setting up start1:\n", this));
                return;
              }

            // This is the non-master participant in the reduction
            // In this case, the task will participate in a local multicombine as a contributer(only)
            // followed by a local multicast(as a recipient).  To detect this case
            // we must not be a master task
            if (!amMaster)
              {
                // The local multicombine
                // Source is local topology
                // Destination is the global master, a reduction
                _mcombine_l.client               = 0; /// \todo ?
                _mcombine_l.context              = 0; /// \todo ?
                _mcombine_l.cb_done.clientdata   = this;
                _mcombine_l.cb_done.function     = composite_done;
                _mcombine_l.connection_id        = _geometry->comm();
                _mcombine_l.roles                = -1U;
                _mcombine_l.data                 = (pami_pipeworkqueue_t*) & _pwq_src;
                _mcombine_l.data_participants    = (pami_topology_t*)t_local;
                _mcombine_l.results              = NULL;
                _mcombine_l.results_participants = (pami_topology_t*)t_my_master;
                _mcombine_l.optor                = cmd->cmd.xfer_allreduce.op;
                _mcombine_l.dtype                = cmd->cmd.xfer_allreduce.dt;
                _mcombine_l.count                = scountDt;

                // Also, prepare the local multicast, with the root as the local master
                // We are guaranteed to not have an early arrival with this multicast if we post
                // it first.  This protocol expects the local multicast to be 2 sided
                _mcast_l.client                  = 0; /// \todo ?
                _mcast_l.context                 = 0; /// \todo ?
                _mcast_l.cb_done.function        = composite_done;
                _mcast_l.cb_done.clientdata      = this;
                _mcast_l.connection_id           = _geometry->comm();
                _mcast_l.roles                   = -1U;
                _mcast_l.bytes                   = cmd->cmd.xfer_allreduce.rtypecount;
                _mcast_l.src                     = NULL;
                _mcast_l.src_participants        = (pami_topology_t*)t_my_master;
                _mcast_l.dst                     = (pami_pipeworkqueue_t*) & _pwq_dest;
                _mcast_l.dst_participants        = (pami_topology_t*)t_local;
                _mcast_l.msginfo                 = 0;
                _mcast_l.msgcount                = 0;
                _startFcn                        = &MultiCombineComposite2Device::start2;
                _count                           = 2;
                TRACE((stderr, "<%p>Non Master Setting up start2:\n", this));
                return;
              }

            // The final case:  this task a global master, with local nodes as well
            // In this case, we will be the target of a local reduction, and participate
            // in this reduction.  The results will chain into a global reduction, which
            // will chain into a local mcast:
            // local_mc[local_topo,me]-->global_mc[master topo, master topo]-->local_mcast[me, local_topo]
            // These extra PWQ's are pointing at the reception buffer.  This means that the
            // reception buffers will be overwritten, maybe more than once
            // todo Do we need some scratch space if we want to do something like in place?
            _pwq_inter0.configure(NULL,                            // Memory manager
                                  cmd->cmd.xfer_allreduce.rcvbuf,  // buffer
                                  sbytes,                          // buffer bytes
                                  0);                              // amount initially in buffer
            _pwq_inter1.configure(NULL,                            // Memory manager
                                  cmd->cmd.xfer_allreduce.rcvbuf,  // buffer
                                  sbytes,                          // buffer bytes
                                  0);                              // amount initially in buffer
            _pwq_inter0.reset();
            _pwq_inter1.reset();

            _mcombine_l.client               = 0; /// \todo ?
            _mcombine_l.context              = 0; /// \todo ?
            _mcombine_l.cb_done.clientdata   = this;
            _mcombine_l.cb_done.function     = composite_done;
            _mcombine_l.connection_id        = _geometry->comm();
            _mcombine_l.roles                = -1U;
            _mcombine_l.data                 = (pami_pipeworkqueue_t*) & _pwq_src;
            _mcombine_l.data_participants    = (pami_topology_t*)t_local;
            _mcombine_l.results              = (pami_pipeworkqueue_t*) & _pwq_inter0; // results can go in dest buffer
            _mcombine_l.results_participants = (pami_topology_t*)t_my_master;       // me!
            _mcombine_l.optor                = cmd->cmd.xfer_allreduce.op;
            _mcombine_l.dtype                = cmd->cmd.xfer_allreduce.dt;
            _mcombine_l.count                = cmd->cmd.xfer_allreduce.stypecount; //todo!  get right count

            _mcombine_g.client               = 0; /// \todo ?
            _mcombine_g.context              = 0; /// \todo ?
            _mcombine_g.cb_done.clientdata   = this;
            _mcombine_g.cb_done.function     = composite_done;
            _mcombine_g.connection_id        = _geometry->comm();
            _mcombine_g.roles                = -1U;
            _mcombine_g.data                 = (pami_pipeworkqueue_t*) & _pwq_inter0;
            _mcombine_g.data_participants    = (pami_topology_t*)t_master;
            _mcombine_g.results              = (pami_pipeworkqueue_t*) & _pwq_inter1;
            _mcombine_g.results_participants = (pami_topology_t*)t_master;
            _mcombine_g.optor                = cmd->cmd.xfer_allreduce.op;
            _mcombine_g.dtype                = cmd->cmd.xfer_allreduce.dt;
            _mcombine_g.count                = cmd->cmd.xfer_allreduce.stypecount; //todo!  get right count

            _mcast_l.client                  = 0; /// \todo ?
            _mcast_l.context                 = 0; /// \todo ?
            _mcast_l.cb_done.function        = composite_done;
            _mcast_l.cb_done.clientdata      = this;
            _mcast_l.connection_id           = _geometry->comm();
            _mcast_l.roles                   = -1U;
            _mcast_l.bytes                   = cmd->cmd.xfer_allreduce.rtypecount;
            _mcast_l.src                     = (pami_pipeworkqueue_t*) & _pwq_inter1;
            _mcast_l.src_participants        = (pami_topology_t*)t_my_master;  // me!
            _mcast_l.dst                     = (pami_pipeworkqueue_t*) & _pwq_dest;
            _mcast_l.dst_participants        = (pami_topology_t*)t_local;
            _mcast_l.msginfo                 = 0;
            _mcast_l.msgcount                = 0;
            _count                           = 3;
            _startFcn                        = &MultiCombineComposite2Device::start3;
            TRACE((stderr, "<%p>Master(local and global) Setting up start3:\n", this));
          }
          void start0()
          {
            TRACE((stderr, "<%p>start0: local native() native=%p\n", this, _native_l));
            _native_l->multicombine(&_mcombine_l, _deviceInfo);
          }
          void start1()
          {
            TRACE((stderr, "<%p>start1(): global multicombine %p\n", this, _native_g));
            _native_g->multicombine(&_mcombine_g, _deviceInfo);
          }
          void start2()
          {
            TRACE((stderr, "<%p>start2(): local mcast+local multicombine %p\n", this, _native_l));
            _native_l->multicast(&_mcast_l, _deviceInfo);
            _native_l->multicombine(&_mcombine_l, _deviceInfo);
          }
          void start3()
          {
            TRACE((stderr, "<%p>start3(): local mcast+local multicombine+global multicombine l=%p g=%p\n"
                   , this, _native_l, _native_g));
            _native_l->multicast(&_mcast_l, _deviceInfo);
            _native_l->multicombine(&_mcombine_l, _deviceInfo);
            _native_g->multicombine(&_mcombine_g, _deviceInfo);
          }

          virtual void start()
          {
            TRACE((stderr, "<%p>Calling startFcn: multicombine() %p\n", this, this->_startFcn));
            (this->*_startFcn)();
          }
          void (MultiCombineComposite2Device::*_startFcn)();
          Interfaces::NativeInterface         *_native_l;
          Interfaces::NativeInterface         *_native_g;
          PAMI_GEOMETRY_CLASS                 *_geometry;
          void                                *_deviceInfo;
          PAMI::Topology                      *_l_topology;
          PAMI::Topology                      *_g_topology;
          pami_multicombine_t                  _mcombine_l;
          pami_multicombine_t                  _mcombine_g;
          pami_multicast_t                     _mcast_l;
          PAMI::PipeWorkQueue                  _pwq_src;
          PAMI::PipeWorkQueue                  _pwq_dest;
          PAMI::PipeWorkQueue                  _pwq_inter0;
          PAMI::PipeWorkQueue                  _pwq_inter1;
          pami_callback_t                      _user_done;
          size_t                               _count;
      };


      /// \brief All sided allreduce over all sided multicombines/multicasts
      /// A local device multicombine will chain into a global multicombine which
      /// will chain into a local multicast
      ///
      /// No real 'pipelining' is done - it waits until each step
      /// completes before starting the next step.

      class MultiCombineComposite2DeviceNP : public CCMI::Executor::Composite
      {

          static void local_master_done_fn(pami_context_t  context,
                                           void           *cookie,
                                           pami_result_t   result )
          {
            MultiCombineComposite2DeviceNP *m = (MultiCombineComposite2DeviceNP*) cookie;
            TRACE_ADAPTOR((stderr, "<%p>MultiCombineComposite2DeviceNP::local_master_done_fn() bytes produced:%zd buffer:%p\n", cookie, m->_pwq_temp.bytesAvailableToConsume(),
                           m->_pwq_temp.bufferToConsume()));
            DO_DEBUG(dumpDbuf((double*)m->_pwq_temp.bufferToConsume() , m->_pwq_temp.bytesAvailableToConsume() / sizeof(double)));
            m->_native_g->multicombine(&m->_mcomb_g, m->_deviceMcombInfo);
          }

          static void local_done_fn(pami_context_t  context,
                                    void           *cookie,
                                    pami_result_t   result )
          {
            TRACE_ADAPTOR((stderr, "<%p>MultiCombineComposite2DeviceNP::local_done_fn()\n", cookie));
            MultiCombineComposite2DeviceNP *m = (MultiCombineComposite2DeviceNP*) cookie;
            m->_native_l->multicast(&m->_mcast_l, m->_deviceMcastInfo);
          }

          static void global_done_fn(pami_context_t  context,
                                     void           *cookie,
                                     pami_result_t   result )
          {
            MultiCombineComposite2DeviceNP *m = (MultiCombineComposite2DeviceNP*) cookie;
            TRACE_ADAPTOR((stderr, "<%p>MultiCombineComposite2DeviceNP::global_master_done_fn() bytes produced:%zd buffer:%p\n", cookie, m->_pwq_dst.bytesAvailableToConsume(),
                           m->_pwq_dst.bufferToConsume()));
            DO_DEBUG(dumpDbuf((double*)m->_pwq_dst.bufferToConsume() , m->_pwq_dst.bytesAvailableToConsume() / sizeof(double)));
            m->_native_l->multicast(&m->_mcast_l, m->_deviceMcastInfo);
          }

#ifdef LOCAL_TEST
          static void dumpDbuf(double* dbuf, size_t count)
          {
            fprintf(stderr, "dbuf=%p, size %zu\n", dbuf, count);

            for (size_t i = 0; i < count; i++)
              fprintf(stderr, "dbuf[%zu]=%f\n", i, dbuf[i]);
          }

          static void test_local_done_fn(pami_context_t  context,
                                         void           *cookie,
                                         pami_result_t   result )
          {
            MultiCombineComposite2DeviceNP *m = (MultiCombineComposite2DeviceNP*) cookie;
            TRACE_ADAPTOR((stderr, "<%p>MultiCombineComposite2DeviceNP::test_local_done_fn() %p, %p\n", cookie, m->_buffer, m->_buffer1));
            DO_DEBUG(dumpDbuf((double*)m->_pwq_dst.bufferToConsume() , m->_pwq_dst.bytesAvailableToConsume() / sizeof(double)));

            if (m->_cb_done.function)
                   m->_cb_done.function(context, m->_cb_done.clientdata, result);
            if(m->_bytes > 2048)
            {
              __global.heap_mm->free(m->_buffer); 
              __global.heap_mm->free(m->_buffer1); 
            }
            m->_buffer = NULL;
            m->_buffer1 = NULL;
          }
#else
          static void dumpDbuf(double* dbuf,  unsigned count) {}
#endif

        public:
          ~MultiCombineComposite2DeviceNP ()
          {
            TRACE_ADAPTOR((stderr, "<%p>~MultiCombineComposite2DeviceNP() %p, %p\n",this,_buffer,_buffer1));
            if(_bytes > 2048)
            {
              __global.heap_mm->free(_buffer);
              __global.heap_mm->free(_buffer1);
            }
          }
          MultiCombineComposite2DeviceNP (Interfaces::NativeInterface      *mInterface,
                                          ConnectionManager::SimpleConnMgr *cmgr,
                                          pami_geometry_t                   g,
                                          pami_xfer_t                      *cmd,
                                          pami_event_function               fn,
                                          void                             *cookie) :
              Composite(),
              // This is a small hack to get around making a whole new set of factories
              // and classes for a 2 device NI.  We just treat the first parameter like a
              // 2 element pinter array.
              _native_l((Interfaces::NativeInterface*)((void **)mInterface)[0]),
              _native_g((Interfaces::NativeInterface*)((void **)mInterface)[1]),
              _geometry((PAMI_GEOMETRY_CLASS*)g),
              _amMaster(_geometry->isLocalMasterParticipant()),
              _pwq_src(),
              _pwq_dst(),
              _pwq_temp(),
              _topology_l((PAMI::Topology*)_geometry->getTopology(PAMI::Geometry::LOCAL_TOPOLOGY_INDEX)),
              _topology_lm(_geometry->localMasterParticipant()),
              _topology_g((PAMI::Topology*)_geometry->getTopology(PAMI::Geometry::MASTER_TOPOLOGY_INDEX)),
              _deviceMcombInfo(NULL),
              _deviceMcastInfo(NULL)
          {
            TRACE_ADAPTOR((stderr, "<%p>%s, type %#zX/%#zX, count %zu/%zu, op %#X, dt %#X\n", this, __PRETTY_FUNCTION__,
                           (size_t)cmd->cmd.xfer_allreduce.stype, (size_t)cmd->cmd.xfer_allreduce.rtype,
                           cmd->cmd.xfer_allreduce.stypecount, cmd->cmd.xfer_allreduce.rtypecount, cmd->cmd.xfer_allreduce.op, cmd->cmd.xfer_allreduce.dt));

            DO_DEBUG(for (unsigned j = 0; j < _topology_l->size(); ++j) fprintf(stderr, "_topology_l[%u]=%zu, size %zu\n", j, (size_t)_topology_l->index2Rank(j), _topology_l->size()));

            DO_DEBUG(for (unsigned j = 0; j < _topology_g->size(); ++j) fprintf(stderr, "_topology_g[%u]=%zu, size %zu\n", j, (size_t)_topology_g->index2Rank(j), _topology_g->size()));

            DO_DEBUG(for (unsigned j = 0; j < _topology_lm.size(); ++j) fprintf(stderr, "_topology_lm[%u]=%zu, size %zu\n", j, (size_t)_topology_lm.index2Rank(j), _topology_lm.size()));

            PAMI_assert(_topology_lm.index2Rank(0) != (unsigned) - 1); // no local master?

            /// \todo only supporting PAMI_BYTE right now
            PAMI_assertf((cmd->cmd.xfer_allreduce.stype == PAMI_BYTE) && (cmd->cmd.xfer_allreduce.rtype == PAMI_BYTE), "Not PAMI_BYTE? %#zX %#zX\n", (size_t)cmd->cmd.xfer_allreduce.stype, (size_t)cmd->cmd.xfer_allreduce.rtype);

            //            PAMI_Type_sizeof(cmd->cmd.xfer_allreduce.stype); /// \todo PAMI_Type_sizeof() is PAMI_UNIMPL so use getReduceFunction for now?

            unsigned        sizeOfType;
            coremath        func;

            getReduceFunction(cmd->cmd.xfer_allreduce.dt,
                              cmd->cmd.xfer_allreduce.op,
                              cmd->cmd.xfer_allreduce.stypecount,// this parm is unused
                              sizeOfType,
                              func );

            _bytes                       = cmd->cmd.xfer_allreduce.stypecount * 1; /// \todo presumed size of PAMI_BYTE?
            if(_bytes > 2048)
            {
	      pami_result_t rc;
	      /// \todo memory leaks?
	      rc = __global.heap_mm->memalign((void **)&_buffer, 0, _bytes + 128);
	      PAMI_assertf(rc == PAMI_SUCCESS, "alloc failed for _buffer %zd", _bytes + 128);
	      rc = __global.heap_mm->memalign((void **)&_buffer1, 0, _bytes + 128);
	      PAMI_assertf(rc == PAMI_SUCCESS, "alloc failed for _buffer1 %zd", _bytes + 128);
            }
            else
              {
                _buffer = _bufferb;
                _buffer1 = _buffer1b;
              }

            size_t countDt = _bytes / sizeOfType;

            // Discover the root node and intereesting topology information
            size_t           numMasters  = _topology_g->size();
            size_t           numLocals   = _topology_l->size();
            PAMI_assertf(numMasters > 1 && numLocals > 1, "numMasters %zu, numLocals %zu\n", numMasters, numLocals); /// \todo keep it simple for now.
            numLocals = numLocals;
            numMasters = numMasters; // silence warnings when ASSERTS=0

            _deviceMcombInfo = _geometry->getKey(PAMI::Geometry::GKEY_MCOMB_CLASSROUTEID);
            _deviceMcastInfo = _geometry->getKey(PAMI::Geometry::GKEY_MCAST_CLASSROUTEID);

            _pwq_src.configure(NULL,                            // Memory manager
                               cmd->cmd.xfer_allreduce.sndbuf,  // buffer
                               _bytes,                          // buffer bytes
                               _bytes);                         // amount initially in buffer
            _pwq_src.reset();
            TRACE_ADAPTOR((stderr, "<%p>MultiCombineComposite2DeviceNP() send buffer %p, bytes available to consume:%zd, produce:%zd\n", this, _pwq_src.bufferToConsume(),
                           _pwq_src.bytesAvailableToConsume(), _pwq_src.bytesAvailableToProduce()));
            DO_DEBUG(dumpDbuf((double*)_pwq_src.bufferToConsume() , _pwq_src.bytesAvailableToConsume() / sizeof(double)));

            _pwq_dst.configure(NULL,                            // Memory manager
                               cmd->cmd.xfer_allreduce.rcvbuf,  // buffer
                               _bytes,                          // buffer bytes
                               0);                              // amount initially in buffer
            _pwq_dst.reset();
            TRACE_ADAPTOR((stderr, "<%p>MultiCombineComposite2DeviceNP() receive buffer %p, bytes available to consume:%zd, produce:%zd\n", this, _pwq_dst.bufferToProduce(),
                           _pwq_dst.bytesAvailableToConsume(), _pwq_dst.bytesAvailableToProduce()));

            char* tmp = (char*)(((uint64_t)_buffer + 127) & ~(uint64_t)127); // align the buffer
            TRACE_ADAPTOR((stderr, "<%p>MultiCombineComposite2DeviceNP() _buffer %p/%p\n", this, _buffer, tmp));
            _pwq_temp.configure(NULL,                           // Memory manager
                                tmp,                            // buffer
                                _bytes,                         // buffer bytes
                                0);                             // amount initially in buffer
            _pwq_temp.reset();

            tmp = (char*)(((uint64_t)_buffer1 + 127) & ~(uint64_t)127); // align the buffer
            TRACE_ADAPTOR((stderr, "<%p>MultiCombineComposite2DeviceNP() _buffer1 %p/%p\n", this, _buffer1, tmp));
            _pwq_temp1.configure(NULL,                           // Memory manager
                                 tmp,                            // buffer
                                 _bytes,                         // buffer bytes
                                 0);                             // amount initially in buffer
            _pwq_temp1.reset();
            _mcomb_l.connection_id        = 0;
            _mcomb_l.roles                = -1U;
            _mcomb_l.data_participants    = (pami_topology_t*)_topology_l;
            _mcomb_l.results_participants = (pami_topology_t*) & _topology_lm;
            _mcomb_l.data                 = (pami_pipeworkqueue_t*) & _pwq_src;
            _mcomb_l.results              = (pami_pipeworkqueue_t*) & _pwq_temp;
            _mcomb_l.cb_done.function     =  _amMaster ? local_master_done_fn : local_done_fn;
            _mcomb_l.cb_done.clientdata   =  this;
            _mcomb_l.context              = 0; /// \todo ?
            _mcomb_l.client               = 0; /// \todo ?
            _mcomb_l.count                = countDt;
            _mcomb_l.dtype                = cmd->cmd.xfer_allreduce.dt;
            _mcomb_l.optor                = cmd->cmd.xfer_allreduce.op;

            _mcomb_g.connection_id        = 0;
            _mcomb_g.roles                = -1U;
            _mcomb_g.data_participants    = (pami_topology_t*)_topology_g;
            _mcomb_g.results_participants = (pami_topology_t*)_topology_g;
            _mcomb_g.data                 = (pami_pipeworkqueue_t*) & _pwq_temp;
            _mcomb_g.results              = (pami_pipeworkqueue_t*) & _pwq_dst;
            _mcomb_g.cb_done.function     = global_done_fn;
            _mcomb_g.cb_done.clientdata   = this;
            _mcomb_g.context              = 0; /// \todo ?
            _mcomb_g.client               = 0; /// \todo ?
            _mcomb_g.count                = countDt;
            _mcomb_g.dtype                = cmd->cmd.xfer_allreduce.dt;
            _mcomb_g.optor                = cmd->cmd.xfer_allreduce.op;

            _mcast_l.connection_id        = 0;
            _mcast_l.roles                = -1U;
            _mcast_l.src_participants     = (pami_topology_t*) & _topology_lm;
            _mcast_l.dst_participants     = (pami_topology_t*)_topology_l;
            _mcast_l.src                  = (pami_pipeworkqueue_t*) & _pwq_dst;
            _mcast_l.dst                  = _amMaster ? (pami_pipeworkqueue_t*) & _pwq_temp1 : (pami_pipeworkqueue_t*) & _pwq_dst; // masters use a temp for mcast destination
#ifdef LOCAL_TEST
            // Intercept the user's done for testing purposes.
            _cb_done.function   = fn;
            _cb_done.clientdata = cookie;
            _mcast_l.cb_done.function     =  test_local_done_fn;
            _mcast_l.cb_done.clientdata   =  this;
#else
            _mcast_l.cb_done.function     =  fn;
            _mcast_l.cb_done.clientdata   =  cookie;
#endif
            _mcast_l.context              = 0; /// \todo ?
            _mcast_l.client               = 0; /// \todo ?
            _mcast_l.bytes                = _bytes;
            _mcast_l.dispatch             = -1; /// \todo assuming all-sided?
            _mcast_l.msgcount             = 0;
            _mcast_l.msginfo              = NULL;

            TRACE_ADAPTOR((stderr, "MultiCombineComposite2DeviceNP() client data:%p\n", cookie));

          }
          virtual void start()
          {
            TRACE_ADAPTOR((stderr, "<%p>MultiCombineComposite2DeviceNP::start()\n", this));
            _native_l->multicombine(&_mcomb_l, _deviceMcombInfo);
          }
        protected:
          Interfaces::NativeInterface        *_native_l;
          Interfaces::NativeInterface        *_native_g;
          Interfaces::NativeInterface        *_active_native;
          PAMI_GEOMETRY_CLASS                *_geometry;
          bool                                _amMaster;
          PAMI::PipeWorkQueue                 _pwq_src;
          PAMI::PipeWorkQueue                 _pwq_dst;
          PAMI::PipeWorkQueue                 _pwq_temp;
          PAMI::PipeWorkQueue                 _pwq_temp1;
          PAMI::Topology                     *_topology_l;
          PAMI::Topology                      _topology_lm;
          PAMI::Topology                     *_topology_g;
          void                               *_deviceMcombInfo;
          void                               *_deviceMcastInfo;
          pami_multicombine_t                 _mcomb_l;
          pami_multicombine_t                 _mcomb_g;
          pami_multicast_t                    _mcast_l;
          pami_multicombine_t                *_active_mcomb;
          char*                               _buffer;
          char*                               _buffer1;
          char                                _bufferb[2048+128];
          char                                _buffer1b[2048+128];
          size_t                              _bytes;
#ifdef LOCAL_TEST
          pami_callback_t                     _cb_done;   // User's completion callback
#endif
      };












    };
  };
};

#ifndef CCMI_TRACE_ALL
#undef TRACE_ADAPTOR
#define TRACE_ADAPTOR(x)
#endif

#endif
