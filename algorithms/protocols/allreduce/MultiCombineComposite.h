/**
 * \file algorithms/protocols/allreduce/MultiCombineComposite.h
 * \brief Simple composite based on multicombine
 */
#ifndef __algorithms_protocols_allreduce_MultiCombineComposite_h__
#define __algorithms_protocols_allreduce_MultiCombineComposite_h__

#include "algorithms/composite/Composite.h"
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
                            ConnectionManager::SimpleConnMgr<PAMI_SYSDEP_CLASS>     * cmgr,
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

          PAMI_Type_sizeof(cmd->cmd.xfer_allreduce.stype); /// \todo PAMI_Type_sizeof() is PAMI_UNIMPL so use getReduceFunction for now?

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




      /// \brief All sided allreduce over active message multicombines
      /// A local device will chain into a global multicombine
      class MultiCombineComposite2Device : public CCMI::Executor::Composite
      {
      public:
        MultiCombineComposite2Device (Interfaces::NativeInterface                         *mInterface,
                                      ConnectionManager::SimpleConnMgr<PAMI_SYSDEP_CLASS> *cmgr,
                                      pami_geometry_t                                      g,
                                      pami_xfer_t                                         *cmd,
                                      pami_event_function                                  fn,
                                      void                                                *cookie) :
          Composite()
          {
          }

        virtual void start()
          {
          }
      };






    };
  };
};

#endif
