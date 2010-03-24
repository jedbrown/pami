/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/CompositeT.h
 * \brief CCMI allreduce sync composite template
 */

#ifndef __algorithms_protocols_allreduce_CompositeT_h__
#define __algorithms_protocols_allreduce_CompositeT_h__

#include "algorithms/protocols/allreduce/Composite.h"
#include "algorithms/schedule/Rectangle.h"

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
      template <class T_Schedule, class T_Executor, class T_Sysdep, class T_Mcast, class T_ConnectionManager>
      class CompositeT : public CCMI::Adaptor::Allreduce::Composite<T_Mcast, T_Sysdep, T_ConnectionManager>
      {
      protected:
        T_Executor  _executor;
        char  _schedule[sizeof(T_Schedule)];
      public:
        static const char* name;
        /// Default Destructor
        virtual ~CompositeT()
        {
          TRACE_ALERT((stderr,"<%p>Allreduce::%s::~CompositeT() ALERT\n",this,name));
        }
        ///
        /// \brief Constructor
        ///
        CompositeT (PAMI_CollectiveRequest_t   * req,
                    T_Sysdep                  * map,
                    T_ConnectionManager       * cmgr,
                    PAMI_Callback_t              cb_done,
                    pami_consistency_t           consistency,
                    T_Mcast                   * mf,
                    PAMI_GEOMETRY_CLASS        * geometry,
                    char                      * srcbuf,
                    char                      * dstbuf,
                    unsigned                    offset,
                    unsigned                    count,
                    pami_dt                      dtype,
                    pami_op                      op,
                    ConfigFlags                 flags,
                    CollectiveProtocolFactory * factory,
                    int                         root = -1,
                    CCMI::Schedule::Color       color=CCMI::Schedule::XP_Y_Z) :
          CCMI::Adaptor::Allreduce::Composite<T_Mcast, T_Sysdep,T_ConnectionManager>(flags,
										     (CCMI::Executor::Composite*)geometry->getKey(PAMI::Geometry::PAMI_GKEY_BARRIERCOMPOSITE0),
                                                                 factory, cb_done),
        _executor(map, cmgr, consistency, geometry->comm(), geometry->getAllreduceIteration())
        {
          create_schedule(map, geometry, color);
          TRACE_ALERT((stderr,"<%p>Allreduce::%s::CompositeT() ALERT\n",this,name));
          addExecutor (&_executor);
          initialize (&_executor, req, srcbuf, dstbuf, count,
                      dtype, op, root);
          _executor.setMulticastInterface (mf);
          _executor.setSchedule ((T_Schedule*)&_schedule);
          _executor.reset ();
        }
        // Template implementation must specialize this function.
        void create_schedule(T_Sysdep        * map,
                             PAMI_GEOMETRY_CLASS                  * geometry,
                             CCMI::Schedule::Color       color)
        {
          CCMI_abort();
        }
        // Template implementation must specialize this function.
        static bool analyze (PAMI_GEOMETRY_CLASS *geometry)
        {
          CCMI_abort();
          return false;
        }
      }; // class CompositeT
    };
  };
};  //namespace CCMI::Adaptor::Allreduce

#endif
