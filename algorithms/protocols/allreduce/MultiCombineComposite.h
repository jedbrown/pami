/**
 * \file algorithms/protocols/allreduce/MultiCombineComposite.h
 * \brief Simple composite based on multicombine
 */
#ifndef __algorithms_protocols_allreduce_MultiCombineComposite_h__
#define __algorithms_protocols_allreduce_MultiCombineComposite_h__

#include "algorithms/composite/Composite.h"
#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "util/ccmi_util.h"


namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {
      // Forward declare prototype
      extern void getReduceFunction(pami_dt, pami_op, unsigned,
                                    unsigned&, coremath&) __attribute__((noinline));

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
          TRACE_ADAPTOR((stderr,"%s, type %#zX/%#zX, count %zu/%zu, op %#X, dt %#X\n", __PRETTY_FUNCTION__,
                         (size_t)cmd->cmd.xfer_allreduce.stype,(size_t)cmd->cmd.xfer_allreduce.rtype,
                         cmd->cmd.xfer_allreduce.stypecount,cmd->cmd.xfer_allreduce.rtypecount,cmd->cmd.xfer_allreduce.op,cmd->cmd.xfer_allreduce.dt));


          /// \todo only supporting PAMI_BYTE right now
          PAMI_assertf((cmd->cmd.xfer_allreduce.stype == PAMI_BYTE)&&(cmd->cmd.xfer_allreduce.rtype == PAMI_BYTE),"Not PAMI_BYTE? %#zX %#zX\n",(size_t)cmd->cmd.xfer_allreduce.stype,(size_t)cmd->cmd.xfer_allreduce.rtype);

//          PAMI_Type_sizeof(cmd->cmd.xfer_allreduce.stype); /// \todo PAMI_Type_sizeof() is PAMI_UNIMPL so use getReduceFunction for now?

          _deviceInfo                  = _geometry->getKey(PAMI::Geometry::PAMI_GKEY_MCOMB_CLASSROUTEID);

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
          DO_DEBUG(all = *(PAMI::Topology*)_geometry->getTopology(0));
          DO_DEBUG(for(unsigned j=0; j< all.size(); ++j) fprintf(stderr,"all[%u]=%zu, size %zu\n",j,(size_t)all.index2Rank(j),all.size()));

         _minfo.client               = 0;
         _minfo.context              = 0; /// \todo ?
         //_minfo.cb_done.function     = _cb_done;
         //_minfo.cb_done.clientdata   = _clientdata;
         _minfo.connection_id        = 0;
         _minfo.roles                = -1U;
         _minfo.results_participants = _geometry->getTopology(0);
         _minfo.data_participants    = _geometry->getTopology(0);
         _minfo.data                 = (pami_pipeworkqueue_t *)&_srcPwq;
         _minfo.results              = (pami_pipeworkqueue_t *)&_dstPwq;
         _minfo.optor                = cmd->cmd.xfer_allreduce.op;
         _minfo.dtype                = cmd->cmd.xfer_allreduce.dt;
         _minfo.count                = size/sizeOfType;
         TRACE_ADAPTOR((stderr,"%s, count %zu\n", __PRETTY_FUNCTION__,_minfo.count));
        }

        virtual void start()
        {
          TRACE_ADAPTOR((stderr,"%s\n", __PRETTY_FUNCTION__));
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
            _obj(native0,native1, cmgr, geometry, cmd, fn, cookie)
            {
            }
          void done_fn( pami_context_t   context,
                        pami_result_t    result )
            {
              _user_done_fn(context, _user_cookie, result);
            }

          MultiCombineComposite2DeviceFactoryT *_factory;
          pami_event_function                _user_done_fn;
          void                              *_user_cookie;
          T_Composite                        _obj;
          unsigned                           _connection_id;
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
      class MultiCombineComposite2Device : public CCMI::Executor::Composite
      {
      public:
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
          _geometry((PAMI_GEOMETRY_CLASS*)g),
          _deviceInfo(NULL),
          _root_topo(cmd->cmd.xfer_broadcast.root),
          _justme_topo(_geometry->rank())
          {
          }

        virtual void start()
          {
          }

        Interfaces::NativeInterface        *_native_l;
        Interfaces::NativeInterface        *_native_g;
        Interfaces::NativeInterface        *_active_native[2];
        pami_multicombine_t                *_active_minfo[2];
        PAMI_GEOMETRY_CLASS                *_geometry;
        void                               *_deviceInfo;
        PAMI::Topology                     *_l_topology;
        PAMI::Topology                     *_g_topology;
        pami_multicast_t                    _minfo_l;
        pami_multicast_t                    _minfo_g;
        PAMI::PipeWorkQueue                 _pwq0;
        PAMI::PipeWorkQueue                 _pwq1;
        PAMI::Topology                      _root_topo;
        PAMI::Topology                      _justme_topo;
        pami_callback_t                     _master_done;
        size_t                              _count;
      };






    };
  };
};

#endif
