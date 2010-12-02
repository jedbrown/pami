/**
 * \file algorithms/protocols/broadcast/CSMultiCastComposite.h
 * \brief Broadcast composite based on collective shmem multicast
 */
#ifndef __algorithms_protocols_broadcast_CSMultiCastComposite_h__
#define __algorithms_protocols_broadcast_CSMultiCastComposite_h__

#include "algorithms/composite/Composite.h"
#include "util/ccmi_util.h"


namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {

      class CSMultiCastComposite : public CCMI::Executor::Composite
      {
        protected:
          Interfaces::NativeInterface        * _native;
          PAMI_GEOMETRY_CLASS                * _geometry;
          pami_broadcast_t                     _xfer_broadcast;
          PAMI::PipeWorkQueue                  _pwq;
          PAMI::Topology                       _root_topo;
          pami_multicast_t                     _minfo;

        public:
          CSMultiCastComposite ( pami_geometry_t                        g,
                                 pami_xfer_t                          * cmd,
                                 pami_event_function                    fn,
                                 void                                 * cookie) :
              Composite(), _geometry((PAMI_GEOMETRY_CLASS*)g),
              _xfer_broadcast(cmd->cmd.xfer_broadcast),
              _root_topo(cmd->cmd.xfer_broadcast.root)
          {
            TRACE_ADAPTOR((stderr, "%s type %#zX, count %zu, root %zu\n", __PRETTY_FUNCTION__, (size_t)cmd->cmd.xfer_broadcast.type, cmd->cmd.xfer_broadcast.typecount, cmd->cmd.xfer_broadcast.root));

            //setDoneCallback(fn, cookie);

            _native = (Interfaces::NativeInterface *)_geometry->getKey(PAMI::Geometry::GKEY_GEOMETRYCSNI);

            /// \todo only supporting PAMI_BYTE right now
            PAMI_assertf(cmd->cmd.xfer_broadcast.type == PAMI_BYTE, "Not PAMI_BYTE? %#zX\n", (size_t)cmd->cmd.xfer_broadcast.type);

            // PAMI_Type_sizeof(cmd->cmd.xfer_broadcast.type); /// \todo PAMI_Type_sizeof() is PAMI_UNIMPL so use getReduceFunction for now?

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
                _pwq.configure(cmd->cmd.xfer_broadcast.buf, bytes, bytes);
              }
            else
              {
                _pwq.configure(cmd->cmd.xfer_broadcast.buf, bytes, 0);
              }

            _pwq.reset();

            _minfo.client             = 0;
            _minfo.context            = 0; /// \todo ?
            _minfo.cb_done.function   = fn;
            _minfo.cb_done.clientdata = cookie;
            _minfo.connection_id      = 0;
            _minfo.roles              = -1U;
            _minfo.dst_participants   = (pami_topology_t *)_geometry->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX);
            _minfo.src_participants   = (pami_topology_t *) & _root_topo;
            _minfo.src                = (pami_pipeworkqueue_t *) & _pwq;
            _minfo.dst                = (pami_pipeworkqueue_t *) & _pwq;
            _minfo.msgcount           = 0;
            _minfo.bytes              = bytes;
          }

          virtual void start()
          {
            TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));
            //_minfo.cb_done.function   = _cb_done;
            //_minfo.cb_done.clientdata = _clientdata;
            _native->multicast(&_minfo);
          }
      };
    };
  };
};

#endif
