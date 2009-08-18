/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/AsyncCompositeT.h
 * \brief CCMI allreduce async composite template
 *  
 * \todo AsyncCompositeT and CompositeT are very similar and could be combined. 
 */

#ifndef __ccmi_adaptor_allreduce_asynccompositet_h__
#define __ccmi_adaptor_allreduce_asynccompositet_h__

#include "./AsyncComposite.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {
      // class AsyncCompositeT
      ///
      /// \brief Allreduce protocol composite
      ///
      /// 
      /// 
      template <class SCHEDULE, class EXECUTOR, class MAP> class AsyncCompositeT : public CCMI::Adaptor::Allreduce::AsyncComposite
      {
      protected:
        EXECUTOR  _executor;
        char  _schedule[sizeof(SCHEDULE)];
      public:
        static const char* name;
        /// Default Destructor
        virtual ~AsyncCompositeT()
        {
          TRACE_ALERT((stderr,"<%#.8X>Allreduce::%s::~AsyncCompositeT() ALERT\n",(int)this,name));
        }
        ///
        /// \brief Constructor
        ///
        AsyncCompositeT (XMI_CollectiveRequest_t  * req,
                         MAP             * map,
                         CCMI::ConnectionManager::ConnectionManager *cmgr,
                         XMI_Callback_t             cb_done,
                         CCMI_Consistency            consistency,
                         CCMI::MultiSend::OldMulticastInterface *mf,
                         Geometry                  * geometry,
                         char                      * srcbuf,
                         char                      * dstbuf,
                         unsigned                    offset,
                         unsigned                    count,
                         XMI_Dt                     dtype,
                         XMI_Op                     op,
                         ConfigFlags                 flags,
                         CollectiveProtocolFactory           * factory,
                         unsigned                    iteration,
                         int                         root = -1,
                         CCMI::Schedule::Color       color=CCMI::Schedule::XP_Y_Z) :
        CCMI::Adaptor::Allreduce::AsyncComposite( flags, factory, cb_done),
        _executor(map, cmgr, consistency, geometry->comm(), iteration)        
        {
          create_schedule(map, geometry, color);
          TRACE_ALERT((stderr,"<%#.8X>Allreduce::%s::AsyncCompositeT() ALERT\n",(int)this,name));
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
      }; // class AsyncCompositeT
    };
  };
};  //namespace CCMI::Adaptor::Allreduce

#endif
