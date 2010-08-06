/**
 * \file algorithms/protocols/broadcast/MultiCastComposite.h
 * \brief Simple composite based on multicast
 */
#ifndef __algorithms_protocols_broadcast_MultiCastComposite_h__
#define __algorithms_protocols_broadcast_MultiCastComposite_h__

#include "algorithms/composite/Composite.h"

#include "algorithms/connmgr/SimpleConnMgr.h"
#include "algorithms/connmgr/CommSeqConnMgr.h"

#include "util/ccmi_util.h"


namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {

      /// \brief An all-sided multicast composite built on an all-sided
      /// multicast.
      ///
      class MultiCastComposite : public CCMI::Executor::Composite
      {
      protected:
        Interfaces::NativeInterface        * _native;
        PAMI_GEOMETRY_CLASS                * _geometry;
        pami_broadcast_t                     _xfer_broadcast;
        PAMI::Topology                       _root;
        PAMI::Topology                       _destinations;
        PAMI::PipeWorkQueue                  _pwq;
        pami_multicast_t                     _minfo;

      public:
        MultiCastComposite (Interfaces::NativeInterface          * mInterface,
                            ConnectionManager::SimpleConnMgr<PAMI_SYSDEP_CLASS>     * cmgr,
                            pami_geometry_t                        g,
                            pami_xfer_t                          * cmd,
                            pami_event_function                    fn,
                            void                                 * cookie) :
        Composite(), _native(mInterface), _geometry((PAMI_GEOMETRY_CLASS*)g),
        _xfer_broadcast(cmd->cmd.xfer_broadcast), _root(cmd->cmd.xfer_broadcast.root)
        {
          TRACE_ADAPTOR((stderr,"<%p>%s type %#zX, count %zu, root %zu\n", this,__PRETTY_FUNCTION__,(size_t)cmd->cmd.xfer_broadcast.type,cmd->cmd.xfer_broadcast.typecount,cmd->cmd.xfer_broadcast.root));

          PAMI::Topology all;
          all = *(PAMI::Topology*)_geometry->getTopology(0);
          all.subtractTopology(&_destinations,  &_root);
          DO_DEBUG(for(unsigned j=0; j< _root.size(); ++j) fprintf(stderr,"root[%u]=%zu, size %zu\n",j,(size_t)_root.index2Rank(j),_root.size()));
          DO_DEBUG(for(unsigned j=0; j< _destinations.size(); ++j) fprintf(stderr,"destinations[%u]=%zu, size %zu\n",j,(size_t)_destinations.index2Rank(j),_destinations.size()));
          DO_DEBUG(for(unsigned j=0; j< all.size(); ++j) fprintf(stderr,"all[%u]=%zu, size %zu\n",j,(size_t)all.index2Rank(j),all.size()));

          /// \todo only supporting PAMI_BYTE right now
          PAMI_assertf(cmd->cmd.xfer_broadcast.type == PAMI_BYTE,"Not PAMI_BYTE? %#zX\n",(size_t)cmd->cmd.xfer_broadcast.type);

          PAMI_Type_sizeof(cmd->cmd.xfer_broadcast.type); /// \todo PAMI_Type_sizeof() is PAMI_UNIMPL so use getReduceFunction for now?

//        unsigned        sizeOfType;
//        coremath        func;
//        pami_op         bogusOp = PAMI_NOOP;
//
//        getReduceFunction(PAMI_UNSIGNED_CHAR, //cmd->cmd.xfer_broadcast.type,/// \todo pami_type_t is not == pami_dt so this doesn't work either
//                          bogusOp,
//                          cmd->cmd.xfer_broadcast.typecount,
//                          sizeOfType,
//                          func );
//        size_t bytes = cmd->cmd.xfer_broadcast.typecount * sizeOfType;

          size_t bytes = cmd->cmd.xfer_broadcast.typecount * 1; /// \todo presumed size of PAMI_BYTE?
          if (cmd->cmd.xfer_broadcast.root == __global.mapping.task())
          {
            _pwq.configure(NULL, cmd->cmd.xfer_broadcast.buf, bytes, bytes);
          }
          else
          {
            _pwq.configure(NULL, cmd->cmd.xfer_broadcast.buf, bytes, 0);
          }
          _pwq.reset();

          _minfo.client             = 0;
          _minfo.context            = 0; /// \todo ?
          //_minfo.cb_done.function   = _cb_done;
          //_minfo.cb_done.clientdata = _clientdata;
          _minfo.connection_id      = 0; /// \todo ?
          _minfo.roles              = -1U;
          _minfo.dst_participants   = (pami_topology_t *)&_destinations;
          _minfo.src_participants   = (pami_topology_t *)&_root;
          _minfo.src                = (pami_pipeworkqueue_t *)&_pwq;
          _minfo.dst                = (pami_pipeworkqueue_t *)&_pwq;
          _minfo.msgcount           = 0;
          _minfo.bytes              = bytes;
        }

        virtual void start()
        {
          TRACE_ADAPTOR((stderr,"<%p>%s\n", this,__PRETTY_FUNCTION__));
          _minfo.cb_done.function   = _cb_done;
          _minfo.cb_done.clientdata = _clientdata;
          _native->multicast(&_minfo);
        }
      };

      /// \brief An all-sided multicast composite built on an active message
      /// multicast.
      ///
      /// It multisyncs before doing the active message multicast.
      /// When the multisync is complete, the root will multicast and the
      /// non-roots will have buffers ready to receive the data.
      ///
      template <class T_Geometry>
      class MultiCastComposite2 : public CCMI::Executor::Composite
      {
      protected:
        Interfaces::NativeInterface        * _native;
        PAMI_GEOMETRY_CLASS                * _geometry;
        pami_broadcast_t                     _xfer_broadcast;
        PAMI::Topology                       _all;
        PAMI::Topology                       _root;
        PAMI::Topology                       _destinations;
        PAMI::PipeWorkQueue                  _src;
        PAMI::PipeWorkQueue                  _dst;
        pami_multicast_t                     _minfo;
        pami_multisync_t                     _msync;
        size_t                               _bytes;
        char                               * _buffer;

      public:
        ~MultiCastComposite2()
        {
          TRACE_ADAPTOR((stderr,"<%p>%s\n", this,__PRETTY_FUNCTION__));
          free(_buffer);
        }
        MultiCastComposite2 (Interfaces::NativeInterface          * mInterface,
                             ConnectionManager::CommSeqConnMgr    * cmgr,
                             pami_geometry_t                        g,
                             pami_xfer_t                          * cmd,
                             pami_event_function                    fn,
                             void                                 * cookie) :
        Composite(), _native(mInterface), _geometry((PAMI_GEOMETRY_CLASS*)g),
        _xfer_broadcast(cmd->cmd.xfer_broadcast), _root(cmd->cmd.xfer_broadcast.root),
        _bytes(cmd->cmd.xfer_broadcast.typecount * 1), /// \todo presumed size of PAMI_BYTE?
        _buffer(NULL)
        {
          TRACE_ADAPTOR((stderr,"<%p>%s type %#zX, count %zu, root %zu\n", this,__PRETTY_FUNCTION__,(size_t)cmd->cmd.xfer_broadcast.type,cmd->cmd.xfer_broadcast.typecount,cmd->cmd.xfer_broadcast.root));

          _all = *(PAMI::Topology*)_geometry->getTopology(0);
          _all.subtractTopology(&_destinations,  &_root);
          DO_DEBUG(for(unsigned j=0; j< _root.size(); ++j) fprintf(stderr,"root[%u]=%zu, size %zu\n",j,(size_t)_root.index2Rank(j),_root.size()));
          DO_DEBUG(for(unsigned j=0; j< _destinations.size(); ++j) fprintf(stderr,"destinations[%u]=%zu, size %zu\n",j,(size_t)_destinations.index2Rank(j),_destinations.size()));
          DO_DEBUG(for(unsigned j=0; j< _all.size(); ++j) fprintf(stderr,"all[%u]=%zu, size %zu\n",j,(size_t)_all.index2Rank(j),_all.size()));

          /// \todo only supporting PAMI_BYTE right now
          PAMI_assertf(cmd->cmd.xfer_broadcast.type == PAMI_BYTE,"Not PAMI_BYTE? %#zX\n",(size_t)cmd->cmd.xfer_broadcast.type);

          PAMI_Type_sizeof(cmd->cmd.xfer_broadcast.type); /// \todo PAMI_Type_sizeof() is PAMI_UNIMPL so use getReduceFunction for now?

//        unsigned        sizeOfType;
//        coremath        func;
//        pami_op         bogusOp = PAMI_NOOP;
//
//        getReduceFunction(PAMI_UNSIGNED_CHAR, //cmd->cmd.xfer_broadcast.type,/// \todo pami_type_t is not == pami_dt so this doesn't work either
//                          bogusOp,
//                          cmd->cmd.xfer_broadcast.typecount,
//                          sizeOfType,
//                          func );
//        size_t bytes = cmd->cmd.xfer_broadcast.typecount * sizeOfType;

          if (cmd->cmd.xfer_broadcast.root == __global.mapping.task())
          {
            _src.configure(NULL, cmd->cmd.xfer_broadcast.buf, _bytes, _bytes);
            /// \todo unless the device lets me toss unwanted data, we need a dummy buffer to receive.
            _buffer = (char*) malloc(_bytes);
            _dst.configure(NULL, _buffer, _bytes, 0);
          }
          else
          {
            _buffer = (char*)& _bytes; // dummy buffer - unused
            _src.configure(NULL, _buffer, 0, 0);
            _dst.configure(NULL, cmd->cmd.xfer_broadcast.buf, _bytes, 0);
          }
          _src.reset();
          _dst.reset();

          // Initialize the mcast
          _minfo.client             = 0;
          _minfo.context            = 0; /// \todo ?
          //_minfo.cb_done.function   = _cb_done;
          //_minfo.cb_done.clientdata = _clientdata;
          T_Geometry *bgqGeometry = (T_Geometry *)g;
          unsigned comm = bgqGeometry->comm();
          _minfo.connection_id      =  cmgr->updateConnectionId(comm);
          _minfo.roles              = -1U;
          _minfo.dst_participants   = (pami_topology_t *)&_destinations;
          _minfo.src_participants   = (pami_topology_t *)&_root;
          _minfo.src                = (pami_pipeworkqueue_t *)&_src;
          _minfo.dst                = (pami_pipeworkqueue_t *)&_dst;
          _minfo.msgcount           = 0;
          _minfo.bytes              = _bytes;

          // Initialize the msync
          _msync.client             = 0;
          _msync.context            = 0; /// \todo ?
          _msync.cb_done.function   = cb_msync_done;
          _msync.cb_done.clientdata = this;
          _msync.connection_id      = 0; /// \todo ?
          _msync.roles              = -1U;
          _msync.participants       = (pami_topology_t *)&_all;

//        pami_dispatch_callback_fn lfn;
//        lfn.multicast = dispatch_multicast_fn;
//        _native->setDispatch(lfn, this);

        }
        virtual void start()
        {
          TRACE_ADAPTOR((stderr,"<%p>%s\n", this,__PRETTY_FUNCTION__));
          _minfo.cb_done.function   = _cb_done;
          _minfo.cb_done.clientdata = _clientdata;

//          pami_dispatch_callback_fn fn;
//          fn.multicast = dispatch_multicast_fn;
//          _native->setDispatch(fn, this);

          // Start the msync. When it completes, it will start the mcast.
          _native->multisync(&_msync);
        }
        void startMcast()
        {
          TRACE_ADAPTOR((stderr,"<%p>%s\n", this,__PRETTY_FUNCTION__));
          if (_xfer_broadcast.root == __global.mapping.task())
            _native->multicast(&_minfo);
        }
        static void cb_msync_done(pami_context_t context, void *me, pami_result_t err)
        {
          TRACE_ADAPTOR((stderr,"<%p>%s\n", me,__PRETTY_FUNCTION__));
          MultiCastComposite2 * composite = (MultiCastComposite2 *) me;
          CCMI_assert (composite != NULL);

          // Msync is done, start the active message mcast on the root
          composite->startMcast();
        }

//      static void dispatch_multicast_fn(const pami_quad_t     * msginfo,       // \param[in] msginfo    Metadata
//                                        unsigned                msgcount,      // \param[in] msgcount Count of metadata
//                                        unsigned                connection_id, // \param[in] connection_id  Stream ID of data
//                                        size_t                  root,          // \param[in] root        Sending task
//                                        size_t                  sndlen,        // \param[in] sndlen      Length of data sent
//                                        void                  * clientdata,    // \param[in] clientdata  Opaque arg
//                                        size_t                * rcvlen,        // \param[out] rcvlen     Length of data to receive
//                                        pami_pipeworkqueue_t ** rcvpwq,        // \param[out] rcvpwq     Where to put recv data
//                                        pami_callback_t       * cb_done)       // \param[out] cb_done    Completion callback to invoke when data received
//      {
//        TRACE_ADAPTOR((stderr,"<%p>%s\n", clientdata,__PRETTY_FUNCTION__));
//        MultiCastComposite2 * composite = (MultiCastComposite2 *)clientdata;
//        composite->dispatch_multicast(rcvlen,  rcvpwq,  cb_done);
//      }
//
        void notifyRecv(size_t                  root,          // \param[in]  root       Sending task
                        const pami_quad_t      *msginfo,       // \param[in]  msginfo    Metadata
                        pami_pipeworkqueue_t ** rcvpwq,        // \param[out] rcvpwq     Where to put recv data
                        pami_callback_t       * cb_done)       // \param[out] cb_done    Completion callback to invoke when data received
        {
          TRACE_ADAPTOR((stderr,"<%p>%s root %zu\n", this,__PRETTY_FUNCTION__, root));
          *rcvpwq  = (pami_pipeworkqueue_t *)&_dst;
          cb_done->function   = _cb_done;
          cb_done->clientdata = _clientdata;
        }

      };



#if 0 /// old test protocol - not needed.
      ///
      /// \brief An all-sided multicast composite built on an all-sided
      /// multicombine ('binary or' operation).
      ///
      class MultiCastComposite3 : public CCMI::Executor::Composite
      {
      protected:
        Interfaces::NativeInterface        * _native;
        PAMI_GEOMETRY_CLASS                * _geometry;
        pami_broadcast_t                     _xfer_broadcast;
        PAMI::Topology                       _all;
        PAMI::PipeWorkQueue                  _data;
        PAMI::PipeWorkQueue                  _results;
        pami_multicombine_t                  _minfo;
        size_t                               _bytes;
        size_t                               _buffer_size;
        char                               * _buffer;

      public:
        ~MultiCastComposite3()
        {
          TRACE_ADAPTOR((stderr,"<%p>%s\n", this,__PRETTY_FUNCTION__));
          _buffer_size = 0;
          free(_buffer);
        }
        MultiCastComposite3 (Interfaces::NativeInterface          * mInterface,
                             ConnectionManager::SimpleConnMgr<PAMI_SYSDEP_CLASS>     * cmgr,
                             pami_geometry_t                        g,
                             pami_xfer_t                          * cmd,
                             pami_event_function                    fn,
                             void                                 * cookie) :
        Composite(), _native(mInterface), _geometry((PAMI_GEOMETRY_CLASS*)g),
        _xfer_broadcast(cmd->cmd.xfer_broadcast),
        _bytes(cmd->cmd.xfer_broadcast.typecount * 1), /// \todo presumed size of PAMI_BYTE?
        _buffer_size(0),
        _buffer(NULL)
        {
          TRACE_ADAPTOR((stderr,"<%p>%s type %#zX, count %zu, root %zu\n", this,__PRETTY_FUNCTION__,(size_t)cmd->cmd.xfer_broadcast.type,cmd->cmd.xfer_broadcast.typecount,cmd->cmd.xfer_broadcast.root));

          _all = *(PAMI::Topology*)_geometry->getTopology(0);
          DO_DEBUG(for(unsigned j=0; j< _all.size(); ++j) fprintf(stderr,"all[%u]=%zu, size %zu\n",j,(size_t)_all.index2Rank(j),_all.size()));

          /// \todo only supporting PAMI_BYTE right now
          PAMI_assertf(cmd->cmd.xfer_broadcast.type == PAMI_BYTE,"Not PAMI_BYTE? %#zX\n",(size_t)cmd->cmd.xfer_broadcast.type);

          PAMI_Type_sizeof(cmd->cmd.xfer_broadcast.type); /// \todo PAMI_Type_sizeof() is PAMI_UNIMPL so use getReduceFunction for now?

//        unsigned        sizeOfType;
//        coremath        func;
//        pami_op         bogusOp = PAMI_NOOP;
//
//        getReduceFunction(PAMI_UNSIGNED_CHAR, //cmd->cmd.xfer_broadcast.type,/// \todo pami_type_t is not == pami_dt so this doesn't work either
//                          bogusOp,
//                          cmd->cmd.xfer_broadcast.typecount,
//                          sizeOfType,
//                          func );
//        size_t bytes = cmd->cmd.xfer_broadcast.typecount * sizeOfType;


          _buffer_size = _bytes;
          _buffer = (char*) malloc(_buffer_size);

          if (cmd->cmd.xfer_broadcast.root == __global.mapping.task())
          {
            _data.configure(NULL, cmd->cmd.xfer_broadcast.buf, _bytes, _bytes);
            _results.configure(NULL, _buffer, _bytes, 0);
          }
          else
          {
            /// \todo would be nice to consume/produce from one buffer and avoid the temp _buffer
            memset(_buffer,  0x00,  _bytes);
            _data.configure(NULL, _buffer, _bytes, _bytes);
            _results.configure(NULL, cmd->cmd.xfer_broadcast.buf, _bytes, 0);
          }
          _data.reset();
          _results.reset();

          _minfo.client             = 0;
          _minfo.context            = 0; /// \todo ?
          //_minfo.cb_done.function   = _cb_done;
          //_minfo.cb_done.clientdata = _clientdata;
          _minfo.connection_id      = 0; /// \todo ?
          _minfo.roles              = -1U;
          _minfo.results_participants   = (pami_topology_t *)&_all;  /// \todo not the root?
          _minfo.data_participants  = (pami_topology_t *)&_all;
          _minfo.data               = (pami_pipeworkqueue_t *)&_data;
          _minfo.results            = (pami_pipeworkqueue_t *)&_results;
          _minfo.optor              = PAMI_BOR;
          _minfo.dtype              = PAMI_UNSIGNED_CHAR;
          _minfo.count              = _bytes;
#ifdef __pami_target_bgq__  // doesn't support chars on MU
          /// \todo this isn't 100%
          if (!((PAMI::Topology*)_geometry->getTopology(0))->isLocal())
          {
            _minfo.dtype              = PAMI_UNSIGNED_INT;
            _minfo.count              = _bytes/4;
          }
#endif
        }

        virtual void start()
        {
          TRACE_ADAPTOR((stderr,"<%p>%s\n", this,__PRETTY_FUNCTION__));
          _minfo.cb_done.function   = _cb_done;
          _minfo.cb_done.clientdata = _clientdata;
          _native->multicombine(&_minfo);
        }

        virtual unsigned restart (void *pcmd)
        {
          pami_xfer_t *cmd = (pami_xfer_t *)pcmd;

          _xfer_broadcast = cmd->cmd.xfer_broadcast;
          TRACE_ADAPTOR((stderr,"<%p>%s type %#zX, count %zu, root %zu\n", this,__PRETTY_FUNCTION__,(size_t)cmd->cmd.xfer_broadcast.type,cmd->cmd.xfer_broadcast.typecount,cmd->cmd.xfer_broadcast.root));

          /// \todo only supporting PAMI_BYTE right now
          PAMI_assertf(cmd->cmd.xfer_broadcast.type == PAMI_BYTE,"Not PAMI_BYTE? %#zX\n",(size_t)cmd->cmd.xfer_broadcast.type);

          PAMI_Type_sizeof(cmd->cmd.xfer_broadcast.type); /// \todo PAMI_Type_sizeof() is PAMI_UNIMPL so use getReduceFunction for now?

//        unsigned        sizeOfType;
//        coremath        func;
//        pami_op         bogusOp = PAMI_NOOP;
//
//        getReduceFunction(PAMI_UNSIGNED_CHAR, //cmd->cmd.xfer_broadcast.type,/// \todo pami_type_t is not == pami_dt so this doesn't work either
//                          bogusOp,
//                          cmd->cmd.xfer_broadcast.typecount,
//                          sizeOfType,
//                          func );
//        size_t bytes = cmd->cmd.xfer_broadcast.typecount * sizeOfType;

          _bytes = cmd->cmd.xfer_broadcast.typecount * 1; /// \todo presumed size of PAMI_BYTE?

          if (_buffer_size > _bytes)
          {
            free(_buffer);
            _buffer_size = _bytes;
            _buffer = (char*) malloc(_buffer_size);
          }

          if (cmd->cmd.xfer_broadcast.root == __global.mapping.task())
          {
            _data.configure(NULL, cmd->cmd.xfer_broadcast.buf, _bytes, _bytes);
            _results.configure(NULL, _buffer, _bytes, 0);
          }
          else
          {
            /// \todo would be nice to consume/produce from one buffer and avoid the temp _buffer
            memset(_buffer,  0x00,  _bytes);
            _data.configure(NULL, _buffer, _bytes, _bytes);
            _results.configure(NULL, cmd->cmd.xfer_broadcast.buf, _bytes, 0);
          }
          _data.reset();
          _results.reset();

          _minfo.client             = 0;
          _minfo.context            = 0; /// \todo ?
          //_minfo.cb_done.function   = _cb_done;
          //_minfo.cb_done.clientdata = _clientdata;
          _minfo.connection_id      = 0; /// \todo ?
          _minfo.roles              = -1U;
          _minfo.results_participants   = (pami_topology_t *)&_all; /// \todo not the root?
          _minfo.data_participants  = (pami_topology_t *)&_all;
          _minfo.data               = (pami_pipeworkqueue_t *)&_data;
          _minfo.results            = (pami_pipeworkqueue_t *)&_results;
          _minfo.optor              = PAMI_BOR;
          _minfo.dtype              = PAMI_UNSIGNED_CHAR;
          _minfo.count              = _bytes;
#ifdef __pami_target_bgq__  // doesn't support chars on MU
          /// \todo this isn't 100%
          if (!((PAMI::Topology*)_geometry->getTopology(0))->isLocal())
          {
            _minfo.dtype              = PAMI_UNSIGNED_INT;
            _minfo.count              = _bytes/4;
          }
#endif
          return 0;
        };

      };
#endif


    };
  };
};

#endif
