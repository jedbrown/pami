/**
 * \file algorithms/protocols/broadcast/MultiCastComposite.h
 * \brief Simple composite based on multicast
 */
#ifndef __algorithms_protocols_broadcast_MultiCastComposite_h__
#define __algorithms_protocols_broadcast_MultiCastComposite_h__

#include "algorithms/composite/Composite.h"
#include "util/ccmi_util.h"


namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {

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
          TRACE_ADAPTOR((stderr,"%s type %#zX, count %zu, root %zu\n", __PRETTY_FUNCTION__,(size_t)cmd->cmd.xfer_broadcast.type,cmd->cmd.xfer_broadcast.typecount,cmd->cmd.xfer_broadcast.root));

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
          if(cmd->cmd.xfer_broadcast.root == __global.mapping.task())
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
          _minfo.connection_id      = 0;
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
          TRACE_ADAPTOR((stderr,"%s\n", __PRETTY_FUNCTION__));
          _minfo.cb_done.function   = _cb_done;
          _minfo.cb_done.clientdata = _clientdata;
          _native->multicast(&_minfo);
        }
      };
    };
  };
};

#endif
