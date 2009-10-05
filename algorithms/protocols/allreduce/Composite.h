/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/Composite.h
 * \brief CCMI composite adaptor for allreduce with barrier support
 */


#ifndef __ccmi_collectives_allreduce_composite_h__
#define __ccmi_collectives_allreduce_composite_h__

#include "algorithms/protocols/allreduce/BaseComposite.h"
#include "algorithms/executor/Barrier.h"
#include "algorithms/executor/AllreduceBase.h"
#include "math/math_coremath.h"

namespace CCMI
{
  namespace Adaptor
  {
    /// \brief Default for reuse_storage_limit is currently MAXINT/2G-1
#define CCMI_DEFAULT_REUSE_STORAGE_LIMIT ((unsigned)2*1024*1024*1024 - 1)
    /// \brief configuration flags/options for creating the factory
    class ConfigFlags
    {
    public:
      ConfigFlags(unsigned r, unsigned p)
        {
          reuse_storage_limit=r;
          pipeline_override=p;
        }
      
/*      unsigned reuse_storage:1;  // save allocated storage across calls
        unsigned reserved:31; */
      unsigned reuse_storage_limit; // save allocated storage up to this limit
      unsigned pipeline_override;
    };

    namespace Allreduce
    {
#if 0
      // Forward declare prototype
      extern void getReduceFunction(xmi_dt, xmi_op, unsigned,
                                    unsigned&, coremath&) __attribute__((noinline));
#endif
      //-- Composite
      /// \brief The Composite for the Allreduce (and reduce)
      /// kernel executor.
      ///
      /// It does common initialization for all subclasses (protocols)
      /// such as mapping the operator and datatype to a function and
      /// calling various setXXX() functions in the kernel executor.
      ///
      /// It also adds support for an optional barrier to synchronize
      /// the kernel executor.  It coordinates the barrier done
      /// callback and the [all]reduce done callback to call the
      /// client done callback.
      ///
      template <class T_Mcast, class T_Sysdep, class T_ConnectionManager>
      class Composite : public BaseComposite
      {
      protected:
        ///
        /// \brief The number of done callbacks to be called - both
        /// barrier and allreduce callbacks.  It will vary based on
        /// whether subclasses are using the barrier or not.
        ///
        int                             _doneCountdown;

        ///
        /// \brief Configuration flags
        ///
        ConfigFlags                     _flags;

        ///
        /// \brief Client's callback to call when the allreduce has
        /// finished
        ///
        void               (* _myClientFunction)(void*, void *, xmi_result_t );
        void                * _myClientData;
      public:
        Composite () :
        BaseComposite (NULL),
        _doneCountdown(1)  // default to just a composite done needed
        {
          CCMI_abort();
        }

        Composite ( ConfigFlags                       flags,
                    CCMI::Executor::Executor      * barrier,
                    CollectiveProtocolFactory                 * factory,
                    XMI_Callback_t                   cb_done):

        BaseComposite (factory),
        _doneCountdown(1),  // default to just a composite done needed
        _flags(flags),
        _myClientFunction (cb_done.function),
        _myClientData (cb_done.clientdata)
        {
          TRACE_ALERT((stderr,"<%#.8X>Allreduce::Composite::ctor() ALERT:\n",(int)this));
          TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::Composite::ctor() flags(%#X) barrier(%#X) factory(%#X)\n",(int)this,
                         *(unsigned*)&flags, (int) barrier, (int) factory));
          if(barrier)
            initializeBarrier ((CCMI::Executor::OldBarrier<T_Mcast> *) barrier);
        }

        /// Default Destructor
        virtual ~Composite()
        {
          TRACE_ALERT((stderr,"<%#.8X>Allreduce::Composite::dtor() ALERT:\n",(int)this));
          TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::Composite::dtor()\n",(int)this));
        }

        void operator delete (void *p)
        {
          CCMI_abort();
        }

        void initializeBarrier (CCMI::Executor::OldBarrier<T_Mcast> *barrier)
        {
          TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::Composite::initializeBarrier barrier(%#X,%#X)\n",(int)this,(int)barrier,(int)this));
          addBarrier (barrier);
          //Setup barrier
          _barrier->setDoneCallback (cb_barrierDone, this);
        }

        ///
        /// \brief initialize should be called after the executors
        /// have been added to the composite
        ///
        void initialize ( CCMI::Executor::AllreduceBase<T_Mcast, T_Sysdep, T_ConnectionManager> * allreduce,
                          XMI_CollectiveRequest_t        * request,
                          char                            * srcbuf,
                          char                            * dstbuf,
                          unsigned                          count,
                          xmi_dt                           dtype,
                          xmi_op                           op,
                          int                               root,
                          unsigned                          pipelineWidth = 0,// none specified, calculate it
                          void                           (* cb_done)(void *, void *, xmi_result_t ) = cb_compositeDone,
                          void                            * cd = NULL
                        )
        {
          TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::Composite::initialize()\n",(int)this));
          allreduce->setSendState(request);
          allreduce->setRoot( root );
          allreduce->setDataInfo(srcbuf, dstbuf);

          allreduce->setDoneCallback( cb_done, cd == NULL? this: cd );

          if((op != allreduce->getOp()) || (dtype != allreduce->getDt()) ||
             (count != allreduce->getCount()))
          {
            coremath func;
            unsigned sizeOfType;
            CCMI::Adaptor::Allreduce::getReduceFunction(dtype, op, count,
                                                        sizeOfType, func);

            unsigned min_pwidth = MIN_PIPELINE_WIDTH;
            if(dtype == XMI_DOUBLE && op == XMI_SUM)
              min_pwidth = MIN_PIPELINE_WIDTH_SUM2P;

            /* Select pipeline width.
                Zero which means calculate it.
                -1 means no pipelining so use count*sizeOfType.
            */

            /*
               First, the function parameter overrides the config value.
            */
            unsigned pwidth = pipelineWidth ? pipelineWidth : _flags.pipeline_override;
            /*
               If -1, disable pipelining or use specified value
            */
            pwidth = (pwidth == (unsigned)-1)? count*sizeOfType : pwidth;
            /*
               Use specified (non-zero) value or calculate (if zero is specified)
            */
            pwidth = pwidth ? pwidth : computePipelineWidth (count, sizeOfType, min_pwidth);

            allreduce->setReduceInfo ( count, pwidth, sizeOfType,
                                       func, op, dtype );
          }
        }

        unsigned computePipelineWidth (unsigned count, unsigned sizeOfType, unsigned min_pwidth)
        {
          TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::Composite::computePipelineWidth() count %#X, size %#X, min %#X\n",(int)this,
                         count, sizeOfType, min_pwidth));
          unsigned pwidth = min_pwidth;

          if(count * sizeOfType > 1024 * pwidth)
            pwidth *= 32;
          else if(count * sizeOfType > 256  * pwidth)
            pwidth *= 16;
          else if(count * sizeOfType > 64 * pwidth)
            pwidth *= 8;
          else if(count * sizeOfType > 16 * pwidth)
            pwidth *= 4;

          TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::Composite::computePipelineWidth() pwidth %#X\n",(int)this,
                         pwidth));
          return pwidth;
        }

        ///
        /// \brief At this level we only support single color
        /// collectives
        ///
        virtual unsigned restart   ( XMI_CollectiveRequest_t  * request,
                                     XMI_Callback_t           & cb_done,
                                     xmi_consistency_t            consistency,
                                     char                      * srcbuf,
                                     char                      * dstbuf,
                                     size_t                      count,
                                     xmi_dt                     dtype,
                                     xmi_op                     op,
                                     size_t                      root = (size_t)-1)
        {
          TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::Composite::restart()\n",(int)this));
          _myClientFunction = cb_done.function;
          _myClientData     = cb_done.clientdata;

          CCMI_assert_debug (getNumExecutors() == 1);
          CCMI::Executor::AllreduceBase<T_Mcast, T_Sysdep, T_ConnectionManager> * allreduce =
          (CCMI::Executor::AllreduceBase<T_Mcast, T_Sysdep, T_ConnectionManager> *) getExecutor(0);

          initialize (allreduce, request, srcbuf, dstbuf,
                      count, dtype, op, root);
          allreduce->setConsistency(consistency);
          allreduce->reset();
          //allreduce->resetReceives ();

          _doneCountdown = 1; // default to just a composite done needed

          if(_barrier)
          {
            // reset barrier since it may be been used between calls
            TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::Composite::reset barrier(%#X,%#X)\n",(int)this,(int)_barrier,(int)this));
            _barrier->setDoneCallback (cb_barrierDone, this);
            startBarrier (consistency);
          }

          return XMI_SUCCESS;
        }

        virtual void start()
        {
          TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::Composite::start()\n",(int)this));
          getExecutor(0)->start();
        }

        void done()
        {
          _doneCountdown --;
          TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::Composite::done() "
                         "_doneCountdown:%#X %#X/%#X \n",(int)this,
                         _doneCountdown,(int)_myClientFunction,
                         (int)_myClientData));
          if(!_doneCountdown)  //allreduce done and (maybe) barrier done
          {
            if(_myClientFunction) (*_myClientFunction) (NULL, _myClientData, XMI_SUCCESS);
            ((CCMI::Executor::AllreduceBase<T_Mcast, T_Sysdep,T_ConnectionManager> *) getExecutor(0))->getAllreduceState()->freeAllocations(_flags.reuse_storage_limit);
            TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::Composite::DONE() \n",
                           (int)this));
          }
        }

        ///
        /// \brief The default done call back to be called when
        /// barrier finishes
        ///
        /// Start the [all]reduce now.
        ///
        /// It means the is done, but the client done isn't called
        /// until both the composite and (optional) barrier are done.
        ///
        static void cb_barrierDone(void *ctxt, void *me, xmi_result_t err)
        {

          TRACE_ADAPTOR((stderr,
                         "<%#.8X>Allreduce::Composite::cb_barrierDone()\n",
                         (int)me));

          Composite *composite = (Composite *) me;
          CCMI::Executor::AllreduceBase<T_Mcast, T_Sysdep,T_ConnectionManager> *allreduce =
          (CCMI::Executor::AllreduceBase<T_Mcast, T_Sysdep,T_ConnectionManager> *) composite->getExecutor(0);
          allreduce->start();
          composite->done();
          TRACE_ADAPTOR((stderr,
                         "<%#.8X>Allreduce::Composite::cb_barrierDone() 2\n",
                         (int)me));
        }

        ///
        /// \brief The default done call back to be called when
        /// [all]reduce finishes
        ///
        /// It means this composite (and kernel executor) is done, but
        /// the client done isn't called until both the composite and
        /// (optional) barrier are done.
        ///
        static void cb_compositeDone(void *ctxt, void *me, xmi_result_t err)
        {
          TRACE_ADAPTOR((stderr,
                         "<%#.8X>Allreduce::Composite::cb_compositeDone()\n",
                         (int)me));
          Composite *composite = (Composite *) me;
          composite->done();
          TRACE_ADAPTOR((stderr,
                         "<%#.8X>Allreduce::Composite::cb_compositeDone() 2\n",
                         (int)me));
        }

        ///
        /// \brief Setup and start the (optional) barrier.  It's done callback
        /// will start the [all]reduce.
        ///
        void startBarrier(xmi_consistency_t             consistency)
        {
          TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::Composite::startBarrier() barrier(%#X)\n",
                         (int)this,(int)_barrier));
          CCMI_assert_debug (_barrier != NULL);

          // We need to account for this barrier calling done in
          // addition to the allreduce calling done, so increment
          // countdown
          _doneCountdown++;

          _barrier->setConsistency (consistency);
          _barrier->start();
        }

      };  //-- Composite
    };
  };
}; // namespace CCMI::Adaptor::Allreduce

#endif
