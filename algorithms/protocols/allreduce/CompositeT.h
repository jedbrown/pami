/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/CompositeT.h
 * \brief CCMI allreduce sync composite template
 */

#ifndef __ccmi_collectives_allreduce_compositet_h__
#define __ccmi_collectives_allreduce_compositet_h__

#include "./Composite.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {
      // class CompositeT
      ///
      /// \brief Allreduce protocol composite
      ///
      /// 
      /// 
      template <class SCHEDULE, class EXECUTOR, class MAP> class CompositeT : public CCMI::Adaptor::Allreduce::Composite
      {
      protected:
        EXECUTOR  _executor;
        char  _schedule[sizeof(SCHEDULE)];
      public:
        static const char* name;
        /// Default Destructor
        virtual ~CompositeT()
        {
          TRACE_ALERT((stderr,"<%#.8X>Allreduce::%s::~CompositeT() ALERT\n",(int)this,name));
        }
        ///
        /// \brief Constructor
        ///
        CompositeT (CCMI_CollectiveRequest_t  * req,
                    MAP        * map,
                    CCMI::ConnectionManager::ConnectionManager *cmgr,
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
                    CollectiveProtocolFactory           * factory,
                    int                         root = -1,
                    CCMI::Schedule::Color       color=CCMI::Schedule::XP_Y_Z) :
        CCMI::Adaptor::Allreduce::Composite( flags, geometry->getBarrierExecutor(), factory, cb_done),
        _executor(map, cmgr, consistency, geometry->comm(), geometry->getAllreduceIteration())        
        {
          create_schedule(map, geometry, color);
          TRACE_ALERT((stderr,"<%#.8X>Allreduce::%s::CompositeT() ALERT\n",(int)this,name));
          addExecutor (&_executor);
          initialize (&_executor, req, srcbuf, dstbuf, count, 
                      dtype, op, root);
          _executor.setMulticastInterface (mf);
          _executor.setSchedule ((SCHEDULE*)&_schedule);
          _executor.reset ();
        }
        // Template implementation must specialize this function.
        void create_schedule(MAP        * map,
                             Geometry                  * geometry,
                             CCMI::Schedule::Color       color)
        {
          CCMI_abort();
        }
        // Template implementation must specialize this function.
        static bool analyze (Geometry *geometry)
        {
          CCMI_abort();
          return false;
        }
      }; // class CompositeT
    };
  };
};  //namespace CCMI::Adaptor::Allreduce

#endif
