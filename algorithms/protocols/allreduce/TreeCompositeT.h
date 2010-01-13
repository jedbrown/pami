/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/TreeCompositeT.h
 * \brief ???
 */

#ifndef __algorithms_protocols_allreduce_TreeCompositeT_h__
#define __algorithms_protocols_allreduce_TreeCompositeT_h__

#include "./Composite.h"


namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {
      namespace Tree
      {

        extern int checkOp(XMI_Dt dt, XMI_Op op);

        ///
        /// \brief Tree allreduce protocol
        ///
        /// Optimizes allreduce for large messages
        ///
        template <class SCHEDULE, class EXECUTOR> class CompositeT : public CCMI::Adaptor::Allreduce::Composite
        {
        protected:
          EXECUTOR  _executor;
          SCHEDULE  _schedule;
        public:
          static const char* name;
          /// Default Destructor
          virtual ~CompositeT()
          {
            TRACE_ALERT((stderr,"<%#.8X>Allreduce::%s::~CompositeT() ALERT\n",(int)this,name));
          }
          CompositeT(XMI_CollectiveRequest_t                      * req,
                     CCMI::TorusCollectiveMapping                  * map,
                     CCMI::ConnectionManager::ConnectionManager    * cmgr,
                     XMI_Callback_t                                 cb_done,
                     CCMI_Consistency                                consistency,
                     CCMI::MultiSend::OldMulticastInterface           * mf,
                     Geometry                                      * geometry,
                     char                                          * srcbuf,
                     char                                          * dstbuf,
                     unsigned                                        count,
                     XMI_Dt                                           dtype,
                     XMI_Op                                           op,
                     ConfigFlags                                     flags,
                     CCMI::Adaptor::CollectiveProtocolFactory*                 factory,
                     int                                             root = -1 ) :
          CCMI::Adaptor::Allreduce::Composite (flags, NULL, factory, cb_done),
          _executor(map, cmgr, consistency, geometry->comm(), geometry->getAllreduceIteration()),
          _schedule(map, geometry->nranks(), geometry->ranks())
          {
            TRACE_ALERT((stderr,"<%#.8X>Allreduce::%s::CompositeT() ALERT\n",(int)this,name));

            if(map->GetDimLength(CCMI_T_DIM) > 1)
            {
              CCMI::Executor::Executor *exe =
              geometry->getLocalBarrierExecutor();
              if(exe == NULL)
                exe = geometry->getBarrierExecutor();

              initializeBarrier ((CCMI::Executor::Barrier *)exe);
            }

            addExecutor (&_executor);
            initialize (&_executor, req, srcbuf, dstbuf,
                        count, dtype, op, root, 0,
                        root == -1?cb_compositeDone:cb_reduceCompositeDone);

            _executor.setMulticastInterface (mf);
            _executor.setSchedule (&_schedule);
            _executor.reset();
            //_executor.setDoneCallback (cb_done.function, cb_done.clientdata);
          }

          void internal_restart (CCMI_Consistency   consistency,
                                 XMI_Callback_t    & cb_done)
          {
            TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::%s::CompositeT::internal_restart\n", (int)this,name));
            _doneCountdown = 1;
            _executor.postReceives ();
            if(_barrier)
            {
              // reset barrier since it may be been used between calls
              TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::%s::CompositeT::internal_restart barrier(%#X,%#X)\n",(int)this,name,(int)_barrier,(int)this));
              if(_executor.getRoot() != -1)  // Reduce needs a double barrier
                _barrier->setDoneCallback (Allreduce::Tree::CompositeT<SCHEDULE,EXECUTOR>::cb_barrierDone, this);
              else
                _barrier->setDoneCallback (Allreduce::Composite::cb_barrierDone, this);
              startBarrier(consistency);
            }
            else
            {
              _executor.setDoneCallback (cb_done.function, cb_done.clientdata);
              _executor.start();
            }
          }


          ///
          /// \brief The default done call back to be called when
          /// reduce finishes
          ///
          /// It means this composite (and kernel executor) is done, but
          /// the client done isn't called until both the composite and
          /// both barriers are done.
          ///
          static void cb_reduceCompositeDone(void *me, XMI_Error_t *err)
          {
            TRACE_ADAPTOR((stderr,
                           "<%#.8X>Allreduce::%s::CompositeT::cb_reduceCompositeDone()\n",
                           (int)me,name));
            ((CCMI::Adaptor::Allreduce::Tree::CompositeT<SCHEDULE,EXECUTOR> *)me)->reduceCompositeDone();
            TRACE_ADAPTOR((stderr,
                           "<%#.8X>Allreduce::%s::CompositeT::cb_reduceCompositeDone() 2\n",
                           (int)me,name));
          }
          void reduceCompositeDone()
          {
            TRACE_ADAPTOR((stderr,
                           "<%#.8X>Allreduce::%s::CompositeT::reduceCompositeDone() _doneCountdown %d\n",
                           (int)this,name,_doneCountdown));
            if(_doneCountdown == 1)
            {
              _barrier->setDoneCallback(Allreduce::Tree::CompositeT<SCHEDULE,EXECUTOR>::cb_endBarrierDone, this);
              startBarrier(_executor.getConsistency());
            }
            done();
          }

          ///
          /// \brief The default done call back to be called when
          /// the first barrier finishes (on reduce only)
          ///
          /// Start the [all]reduce now.  Start the ending/second barrier.
          ///
          static void cb_endBarrierDone(void *me, XMI_Error_t *err)
          {
            TRACE_ADAPTOR((stderr,
                           "<%#.8X>Allreduce::%s::CompositeT::cb_endBarrierDone()\n",
                           (int)me,name));
            ((CCMI::Adaptor::Allreduce::Tree::CompositeT<SCHEDULE,EXECUTOR> *)me)->done();
            TRACE_ADAPTOR((stderr,
                           "<%#.8X>Allreduce::%s::CompositeT::cb_endBarrierDone() 2\n",
                           (int)me,name));
          }
          ///
          /// \brief The default done call back to be called when
          /// the first barrier finishes (on reduce only)
          ///
          /// Start the [all]reduce now.  Start the ending/second barrier.
          ///
          static void cb_barrierDone(void *me, XMI_Error_t *err)
          {
            TRACE_ADAPTOR((stderr,
                           "<%#.8X>Allreduce::%s::CompositeT::cb_barrierDone()\n",
                           (int)me,name));
            ((CCMI::Adaptor::Allreduce::Tree::CompositeT<SCHEDULE,EXECUTOR> *)me)->barrierDone();
            TRACE_ADAPTOR((stderr,
                           "<%#.8X>Allreduce::%s::CompositeT::cb_barrierDone() 2\n",
                           (int)me,name));
          }
          void barrierDone()
          {
            TRACE_ADAPTOR((stderr,
                           "<%#.8X>Allreduce::%s::CompositeT::barrierDone() _doneCountdown %d\n",
                           (int)this,name, _doneCountdown));

            _executor.start();
            if(_doneCountdown == 1)
            {
              _barrier->setDoneCallback(Allreduce::Tree::CompositeT<SCHEDULE,EXECUTOR>::cb_endBarrierDone, this);
              startBarrier(_executor.getConsistency());
            }
            done();
          }

          virtual unsigned restart ( XMI_CollectiveRequest_t  * request,
                                     XMI_Callback_t           & cb_done,
                                     CCMI_Consistency            consistency,
                                     char                      * srcbuf,
                                     char                      * dstbuf,
                                     size_t                      count,
                                     XMI_Dt                     dtype,
                                     XMI_Op                     op,
                                     size_t                      root = (size_t)-1)
          {
            TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::%s::CompositeT::restart _executor %#X\n", (int)this,name,
                           (int)&_executor));
            _myClientFunction = cb_done.function;
            _myClientData     = cb_done.clientdata;

            //If the op and datatype have changed we need to check if
            //it is supportable on the tree If any of the three change
            //we should call generate again.
            if(op != _executor.getOp() || dtype != _executor.getDt()
               || count != _executor.getCount())
            {
              TRACE_ALERT((stderr, "<%#.8X>Allreduce::%s::CompositeT::restart() ALERT: "
                           "XMI_ERROR op %#X, type %#X, count %#X!\n", (int)this,name, op, dtype, count));
              TRACE_ADAPTOR((stderr, "<%#.8X>Allreduce::%s::CompositeT::restart():"
                             "XMI_ERROR op %#X, type %#X, count %#X!\n",(int)this,name, op, dtype, count));
              _executor.getAllreduceState()->freeAllocations();
              return XMI_ERROR;
            }

            initialize (&_executor, request, srcbuf, dstbuf,
                        count, dtype, op, root, 0,
                        root == (size_t)-1 ? cb_compositeDone : cb_reduceCompositeDone);
            _executor.reset();
            internal_restart (consistency, cb_done);

            return XMI_SUCCESS;
          }
        }; // class Composite

      };
    };
  };
};  //namespace CCMI::Adaptor::Allreduce::Tree


#endif
