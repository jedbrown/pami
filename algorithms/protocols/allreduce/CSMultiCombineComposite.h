/**
 * \file algorithms/protocols/allreduce/CSMultiCombineComposite.h
 * \brief Collective shmem composite based on csmulticombine
 */
#ifndef __algorithms_protocols_allreduce_CSMultiCombineComposite_h__
#define __algorithms_protocols_allreduce_CSMultiCombineComposite_h__

#include "algorithms/composite/Composite.h"
#include "algorithms/protocols/allreduce/ReduceFunctions.h"
#include "util/ccmi_util.h"


namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {
      class CSMultiCombineComposite : public CCMI::Executor::Composite
      {
      protected:
        Interfaces::NativeInterface        * _native;
        PAMI_GEOMETRY_CLASS                * _geometry;
        pami_reduce_t                      _xfer_reduce;
        PAMI::Topology                     _root_topo;
        PAMI::PipeWorkQueue                _srcPwq;
        PAMI::PipeWorkQueue                _dstPwq;
        pami_multicombine_t                _minfo;

      public:
        CSMultiCombineComposite ( pami_geometry_t                        g,
                                  pami_xfer_t                          * cmd,
                                  pami_event_function                    fn,
                                  void                                 * cookie) :
        Composite(), _geometry((PAMI_GEOMETRY_CLASS*)g), _xfer_reduce(cmd->cmd.xfer_reduce), _root_topo(cmd->cmd.xfer_reduce.root)
        {
          TRACE_ADAPTOR((stderr,"%s, type %#zX/%#zX, count %zu/%zu, op %#X, dt %#X\n", __PRETTY_FUNCTION__,
                         (size_t)cmd->cmd.xfer_reduce.stype,(size_t)cmd->cmd.xfer_reduce.rtype,
                         cmd->cmd.xfer_reduce.stypecount,cmd->cmd.xfer_reduce.rtypecount,_op,_dt));

          // setDoneCallback(fn, cookie);

          _native = (Interfaces::NativeInterface  *)_geometry->getKey(PAMI::Geometry::GKEY_GEOMETRYCSNI);

          /// \todo only supporting PAMI_BYTE right now
          PAMI_assertf((cmd->cmd.xfer_reduce.stype == PAMI_BYTE)&&(cmd->cmd.xfer_reduce.rtype == PAMI_BYTE),"Not PAMI_BYTE? %#zX %#zX\n",(size_t)cmd->cmd.xfer_reduce.stype,(size_t)cmd->cmd.xfer_reduce.rtype);

          // PAMI_Type_sizeof(cmd->cmd.xfer_reduce.stype); /// \todo PAMI_Type_sizeof() is PAMI_UNIMPL so use getReduceFunction for now?

          unsigned        sizeOfType;
          coremath        func;

          getReduceFunction(cmd->cmd.xfer_reduce.dt,
                            cmd->cmd.xfer_reduce.op,
                            cmd->cmd.xfer_reduce.stypecount,// this parm is unused
                            sizeOfType,
                            func );
          size_t size = cmd->cmd.xfer_reduce.stypecount * 1; /// \todo presumed size of PAMI_BYTE is 1?
          _srcPwq.configure(NULL, cmd->cmd.xfer_reduce.sndbuf, size, size);
          _srcPwq.reset();

          if(cmd->cmd.xfer_reduce.root == __global.mapping.task()) {
            size = cmd->cmd.xfer_reduce.rtypecount * 1; /// \todo presumed size of PAMI_BYTE is 1?
            _dstPwq.configure(NULL, cmd->cmd.xfer_reduce.rcvbuf, size, 0);
            _dstPwq.reset();
          }

          DO_DEBUG(PAMI::Topology all);
          DO_DEBUG(all = *(PAMI::Topology*)_geometry->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX));
          DO_DEBUG(for(unsigned j=0; j< all.size(); ++j) fprintf(stderr,"all[%u]=%zu, size %zu\n",j,(size_t)all.index2Rank(j),all.size()));

         _minfo.client               = 0;
         _minfo.context              = 0; /// \todo ?
         _minfo.cb_done.function     = fn;
         _minfo.cb_done.clientdata   = cookie;
         _minfo.connection_id        = 0;
         _minfo.roles                = -1U;
         _minfo.results_participants = (pami_topology_t *) &_root_topo;
         _minfo.data_participants    = (pami_topology_t *)_geometry->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX);
         _minfo.data                 = (pami_pipeworkqueue_t *)&_srcPwq;
         _minfo.results              = (pami_pipeworkqueue_t *)&_dstPwq;
         _minfo.optor                = cmd->cmd.xfer_reduce.op;
         _minfo.dtype                = cmd->cmd.xfer_reduce.dt;
         _minfo.count                = size/sizeOfType;
         TRACE_ADAPTOR((stderr,"%s, count %zu\n", __PRETTY_FUNCTION__,_minfo.count));
        }

        virtual void start()
        {
          TRACE_ADAPTOR((stderr,"%s\n", __PRETTY_FUNCTION__));
          // _minfo.cb_done.function     = _cb_done;
          // _minfo.cb_done.clientdata   = _clientdata;
          _native->multicombine(&_minfo);
        }
      };
    };
  };
};

#endif
