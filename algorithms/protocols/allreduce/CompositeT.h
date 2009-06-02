/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file collectives/bgp/protocols/allreduce/CompositeT.h
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
      template <class SCHEDULE, class EXECUTOR> class CompositeT : public CCMI::Adaptor::Allreduce::Composite
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
                    CCMI::Mapping        * map,
                    CCMI::ConnectionManager::ConnectionManager *cmgr,
                    CCMI_Callback_t             cb_done,
                    CCMI_Consistency            consistency,
                    CCMI::MultiSend::MulticastInterface *mf,
                    Geometry                  * geometry,
                    char                      * srcbuf,
                    char                      * dstbuf,
                    unsigned                    offset,
                    unsigned                    count,
                    CCMI_Dt                     dtype,
                    CCMI_Op                     op,
                    ConfigFlags                 flags,
                    ProtocolFactory           * factory,
                    int                         root = -1) :
        CCMI::Adaptor::Allreduce::Composite( flags, geometry->getBarrierExecutor(), factory, cb_done),
        _executor(map, cmgr, consistency, geometry->comm(), geometry->getAllreduceIteration())        
        {
          create_schedule(map, geometry);
          TRACE_ALERT((stderr,"<%#.8X>Allreduce::%s::CompositeT() ALERT\n",(int)this,name));
          addExecutor (&_executor);
          initialize (&_executor, req, srcbuf, dstbuf, count, 
                      dtype, op, root);
          _executor.setMulticastInterface (mf);
          _executor.setSchedule ((SCHEDULE*)&_schedule);
          _executor.reset ();
        }
        // Template implementation must specialize this function.
        void create_schedule(CCMI::Mapping        * map,
                             Geometry                  * geometry)
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
