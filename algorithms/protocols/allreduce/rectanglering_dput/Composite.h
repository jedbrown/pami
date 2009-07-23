/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/rectanglering_dput/Composite.h
 * \brief Composite adaptor for rectangle allreduce using
 * a ring sub-schedule and dput multisend.
 */

#ifndef __ccmi_adaptor_allreduce_rectanglering_dput_composite_h__
#define __ccmi_adaptor_allreduce_rectanglering_dput_composite_h__

#include "algorithms/executor/PipelinedAllreduce.h"
#include "algorithms/protocols/allreduce/Composite.h"
#include "algorithms/schedule/Rectangle.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {
      namespace RectangleRingDput
      {
#define MAX_DPUT_ARED_COLORS  3
        // class Composite
        ///
        /// \brief Rectangle allreduce protocol
        ///
        /// Specifies the OneColorRectAllredSched with the 
        ///  CCMI::Schedule::OneColorRectRedSched::Ring subschedule
        /// 
        class Composite : public CCMI::Adaptor::Allreduce::Composite
        {
        protected:
          CCMI::Schedule::Rectangle               * _rect;
          unsigned                                    _ideal;
          unsigned                                    _ncolors;
          CCMI_Executor_t                           * _executor_buf;    
          CCMI::TorusCollectiveMapping                              * _mapping;
          CCMI::Schedule::Color                     _colors[MAX_DPUT_ARED_COLORS];

          typedef struct _se
          {
            CCMI::Executor::PipelinedAllreduce       _executor;
            CCMI::Schedule::OneColorRectAllredSched  _schedule;
          } ScheduleExec;   

        public:
          /// Default Destructor
          virtual ~Composite()
          {
            if(_executor_buf != NULL)
            {
              for(unsigned idx = 0; idx < _ideal; idx++)
              {
                ScheduleExec *se = (ScheduleExec *) (_executor_buf + idx);
                se->_executor.~PipelinedAllreduce();
                se->_schedule.~OneColorRectAllredSched();
              }

              CCMI_Free (_executor_buf);

              TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::RectangleRingDput::Composite::dtor() ALERT, %X\n", 
                         (int)this, (unsigned)_executor_buf));
              TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::RectangleRingDput::Composite::dtor()\n",(int)this));

              _executor_buf = NULL;
              _numExecutors = 0;   //Reset all executors to be NULL
            }
          }

          ///
          /// \brief Constructor
          ///
          Composite (CM_CollectiveRequest_t  * req,
                     CCMI::TorusCollectiveMapping              * map,
                     CCMI::ConnectionManager::ConnectionManager *rcmgr,
                     CCMI::ConnectionManager::ConnectionManager *bcmgr,
                     CM_Callback_t             cb_done,
                     CCMI_Consistency            consistency,
                     CCMI::MultiSend::OldMulticastInterface *mf,
                     Geometry                  * geometry,
                     char                      * srcbuf,
                     char                      * dstbuf,
                     unsigned                    offset,
                     unsigned                    count,
                     CM_Dt                     dtype,
                     CM_Op                     op,
                     ConfigFlags                 flags,
                     CCMI::Adaptor::CollectiveProtocolFactory* factory,
                     unsigned                    ideal,
                     CCMI::Schedule::Color   * colors,
                     int                         root = -1) :
          CCMI::Adaptor::Allreduce::Composite( flags, geometry->getBarrierExecutor(), factory, cb_done),
          _rect(geometry->rectangle()), _ideal (ideal), _ncolors (ideal), _executor_buf(NULL), _mapping (map)
          {
            CCMI_assert(root == -1); // We should just remove this parm

            CCMI_assert (_ideal <= MAX_DPUT_ARED_COLORS);
            CCMI_assert (_ideal > 0);
            COMPILE_TIME_ASSERT (sizeof(ScheduleExec) < sizeof (CCMI_Executor_t));

            _executor_buf = (CCMI_Executor_t *) CCMI_Alloc (sizeof (CCMI_Executor_t) * _ideal);

            TRACE_ALERT ((stderr,"<%#.8X>Allreduce::RectangleRingDput::Composite::ctor() ALERT, %X\n",
                          (int)this, (unsigned)_executor_buf));
            TRACE_ADAPTOR((stderr, "<%#.8X>Allreduce::RectangleRingDput::Composite::ctor ideal colors %#X\n",(int)this,ideal));

            unsigned idx = 0;
            for(idx = 0; idx < _ideal; idx ++)
            {
              ScheduleExec *se = (ScheduleExec *) (_executor_buf + idx);

              new (& se->_executor)
              CCMI::Executor::PipelinedAllreduce (map, NULL, 
                                            consistency, 
                                            geometry->comm(), 
                                            geometry->getAllreduceIteration());
              new (& se->_schedule)
              CCMI::Schedule::OneColorRectAllredSched (map, colors[idx], 
                                                 *geometry->rectangle(), 
                                                 CCMI::Schedule::OneColorRectRedSched::Ring);

              addExecutor (& se->_executor);
              se->_executor.setSchedule (& se->_schedule, colors[idx]);
              se->_executor.setMulticastInterface (mf);
              se->_executor.setReduceConnectionManager (rcmgr);         
              se->_executor.setBroadcastConnectionManager (bcmgr);        

              _colors[idx] = colors[idx];
            }

            internal_restart (req, srcbuf, dstbuf, count, dtype, op);
            if(_barrier)
            {
              // set the barrier done callback to the multiclor done
              TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::RectangleRingDput::Composite::ctor initialize barrier(%#X,%#X)\n",(int)this,(int)_barrier,(int)this));
              _barrier->setDoneCallback (cb_mc_barrierDone, this);
            }
          }

          void internal_restart (CM_CollectiveRequest_t  * creq,
                                 char                      * srcbuf,
                                 char                      * dstbuf,
                                 unsigned                    count,
                                 CM_Dt                     dtype,
                                 CM_Op                     op)
          {
            coremath func;
            unsigned sizeOfType;
            CCMI::Adaptor::Allreduce::getReduceFunction(dtype, op, count,
                                                        sizeOfType, func);            
            unsigned bytes = count * sizeOfType;     
            unsigned pwidth = compute_pwidth(bytes / _ideal, CCMI::Schedule::XP_Y_Z);

            _ncolors = _ideal;
            if(bytes < pwidth)
              _ncolors = 1;

            if(_ncolors > _ideal)
              _ncolors = _ideal;

            char     * nexts[MAX_DPUT_ARED_COLORS]   = {srcbuf, 0, 0};
            char     * nextd[MAX_DPUT_ARED_COLORS]   = {dstbuf, 0, 0};
            unsigned   byteCnt[MAX_DPUT_ARED_COLORS] = {bytes,  0, 0};
            char     * rvec[MAX_DPUT_ARED_COLORS];
            rvec[0] = (char *) creq;

            unsigned idx = 0;
            if(_ncolors > 1)
            {
              TRACE_ADAPTOR((stderr, "<%#.8X>Allreduce::RectangleRingDput::Composite::internal_restart rvec[%#X] %#X,nexts[%#X] %#X,byteCnt[xx] xx,nextd[%#X] %#X\n",(int)this,
                             0,
                             (int)rvec[0],
                             0,
                             (int)nexts[0],
                             //idx-1,
                             //byteCnt[idx-1],
                             0,
                             (int)nextd[0]));
              for(idx = 1; idx < _ncolors; ++idx)
              {
                rvec[idx] = rvec[idx-1] + sizeof(CCMI::Executor::AllreduceBase::SendState) * CM_MAX_ACTIVE_SENDS;
                nexts[idx] = (char *)(((unsigned)nexts[idx-1] + (bytes/_ncolors)) & 0xFFFFFFF0);
                byteCnt[idx-1] = (unsigned)(nexts[idx] - nexts[idx-1]);
                nextd[idx] = nextd[idx-1] + byteCnt[idx-1]; 
                TRACE_ADAPTOR((stderr, "<%#.8X>Allreduce::RectangleRingDput::Composite::internal_restart rvec[%#X] %#X,nexts[%#X] %#X,byteCnt[%#X] %#X,nextd[%#X] %#X\n",(int)this,
                               idx,
                               (int)rvec[idx],
                               idx,
                               (int)nexts[idx],
                               idx-1,
                               byteCnt[idx-1],
                               idx,
                               (int)nextd[idx]));
              }
              byteCnt[_ncolors-1] = (srcbuf + bytes) - nexts[_ncolors-1];
              TRACE_ADAPTOR((stderr, "<%#.8X>Allreduce::RectangleRingDput::Composite::internal_restart byteCnt[%#X] %#X\n",(int)this,_ncolors-1,byteCnt[_ncolors-1]));
            }

            for(idx = 0; idx < _ncolors; idx ++)
            {
              pwidth = compute_pwidth(bytes / _ncolors, _colors[idx]);        
              TRACE_ADAPTOR((stderr, "<%#.8X>Allreduce::RectangleRingDput::Composite::internal_restart bytes %#X, _ncolors %#X, _colors[%#X] %#X, pwidth %#X, byteCnt[%#X] %#X\n",(int)this,
                             bytes,
                             _ncolors,
                             idx,
                             _colors[idx],
                             pwidth,
                             idx,
                             byteCnt[idx]/sizeOfType));
              CCMI::Executor::PipelinedAllreduce *allreduce = (CCMI::Executor::PipelinedAllreduce *) getExecutor(idx);
              initialize (allreduce, (CM_CollectiveRequest_t *)rvec[idx], 
                          nexts[idx], nextd[idx], byteCnt[idx]/sizeOfType, 
                          dtype, op, -1, pwidth, cb_mc_compositeDone, this);

              allreduce->reset ();
              allreduce->postReceives();          ///A post receive based algorithm
            }

            _doneCountdown = _ncolors; // default to just a composite done needed
          }

          virtual unsigned restart   ( CM_CollectiveRequest_t  * request,
                                       CM_Callback_t           & cb_done,
                                       CCMI_Consistency            consistency,
                                       char                      * srcbuf,
                                       char                      * dstbuf,
                                       unsigned                    count,
                                       CM_Dt                     dtype,
                                       CM_Op                     op,
                                       int                         root=-1) 
          {
            TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::RectangleRingDput::Composite::restart()\n",(int)this));
            _myClientFunction = cb_done.function;
            _myClientData     = cb_done.clientdata;

            internal_restart (request, srcbuf, dstbuf, count, dtype, op);

            if(_barrier)
            {
              // reset barrier since it may be been used between calls
              TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::RectangleRingDput::Composite::restart barrier(%#X,%#X)\n",(int)this,(int)_barrier,(int)this));
              _barrier->setDoneCallback (cb_mc_barrierDone, this);
              startBarrier (consistency);
            }

            return CM_SUCCESS;
          }

          void mc_done()
          {
            _doneCountdown --;
            TRACE_ADAPTOR((stderr, "<%#.8X>Allreduce::RectangleRingDput::Composite::mc_done _doneCountdown %#X\n",(int)this,_doneCountdown));

            if(!_doneCountdown)  //allreduce done and (maybe) barrier done
            {
              if(_myClientFunction) (*_myClientFunction) (_myClientData, NULL);

              for(unsigned idx = 0; idx < _ideal; idx++)
              {
                ScheduleExec *se = (ScheduleExec *) (_executor_buf + idx);
                se->_executor.getAllreduceState()->freeAllocations(_flags.reuse_storage_limit);
              }
            }
          }

          ///
          /// \brief The default done call back to be called when
          /// [all]reduce finishes
          ///
          /// It means this composite (and kernel executor) is done, but
          /// the client done isn't called until both the composite and
          /// (optional) barrier are done.
          /// 
          static void cb_mc_compositeDone(void *me, CM_Error_t *err)
          {
            Composite *composite = (Composite *) me;
            composite->mc_done();
          }

          ///
          /// \brief Multi-color done call back to be called when
          /// barrier finishes
          ///
          /// Start the [all]reduce now.
          /// 
          /// It means the is done, but the client done isn't called
          /// until both the composite and (optional) barrier are done.
          /// 
          static void cb_mc_barrierDone(void *me, CM_Error_t *err)
          {
            Composite *composite = (Composite *) me;

            TRACE_ADAPTOR((stderr, "<%#.8X>Allreduce::RectangleRingDput::Composite::cb_mc_barrierDone\n",(int)me));
            for(unsigned idx =0; idx < composite->_ncolors; idx ++)
            {
              CCMI::Executor::AllreduceBase *allreduce = 
              (CCMI::Executor::AllreduceBase *) composite->getExecutor(idx);
              allreduce->start();
            }
            //Barrier has completed
            composite->mc_done();
          }

          ///\brief Compute a pipelinewidth in the range of 960 bytes and 30720 bytes
          unsigned compute_pwidth (unsigned bytes, unsigned color)
          {
            static char query_env = 1;
            static unsigned min_pipeline_width = ALLREDUCE_MIN_PIPELINE_WIDTH_TORUS;
            unsigned hops = 0;

            if (query_env)
            {
              /// \todo this should be in the glue or some higher level - not DCMF specific
              char* envopts = getenv("DCMF_ALLREDUCE_MIN_PW");
              if(envopts) min_pipeline_width = atoi(envopts);
              query_env = 0;
            }

            if (color == CCMI::Schedule::XP_Y_Z)
              hops = 2*(_rect->xs * 2 + _rect->ys  + _rect->zs - 4); //XCLOR = (XP YN ZN XN) * 2 for reduce and bcast
            else if (color == CCMI::Schedule::YP_Z_X)
              hops = 2*(_rect->xs  + _rect->ys * 2 + _rect->zs - 4 ); //YCLOR = (YP ZN XN YN) * 2 for reduce and bcast
            else if (color == CCMI::Schedule::ZP_X_Y)
              hops = 2*(_rect->xs  + _rect->ys + 2 *_rect->zs  - 4 ); //ZCLOR = (ZP XN YN ZN) * 2 for reduce and bcast

            if (hops == 0) hops = 1;
            unsigned pwidth = (bytes / (hops * TORUS_PACKET_SIZE)) * TORUS_PACKET_SIZE;

            if (pwidth < min_pipeline_width)
              pwidth = min_pipeline_width;

            if (pwidth >  128 * min_pipeline_width)
              pwidth = 128 * min_pipeline_width;

            TRACE_ADAPTOR((stderr, "<%#.8X>Allreduce::RectangleRingDput::Composite::compute_pwidth bytes %#X, color %#X, hops %#X, pwidth %#X\n",(int)this,
                           bytes, color, hops, pwidth));
            return pwidth;
          }
        }; // class Composite
      };
    };
  };
};  //namespace CCMI::Adaptor::Allreduce::RectangleRingDput

#endif
