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
      class CompositeT : public CCMI::Adaptor::Allreduce::Composite<T_Sysdep, T_Mcast>
      {
      protected:
        T_Executor  _executor;
        char  _schedule[sizeof(T_Schedule)];
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
        CompositeT (XMI_CollectiveRequest_t   * req,
                    T_Sysdep                  * map,
                    T_ConnectionManager       * cmgr,
                    XMI_Callback_t              cb_done,
                    xmi_consistency_t           consistency,
                    T_Mcast                   * mf,
                    XMI_GEOMETRY_CLASS        * geometry,
                    char                      * srcbuf,
                    char                      * dstbuf,
                    unsigned                    offset,
                    unsigned                    count,
                    xmi_dt                      dtype,
                    xmi_op                      op,
                    ConfigFlags                 flags,
                    CollectiveProtocolFactory * factory,
                    int                         root = -1,
                    CCMI::Schedule::Color       color=CCMI::Schedule::XP_Y_Z) :
          CCMI::Adaptor::Allreduce::Composite<T_Sysdep, T_Mcast>(flags,
                                                                 geometry->getKey(XMI::Geometry::XMI_GKEY_BARRIEREXECUTOR),
                                                                 factory, cb_done),
        _executor(map, cmgr, consistency, geometry->comm(), geometry->getAllreduceIteration())
        {
          create_schedule(map, geometry, color);
          TRACE_ALERT((stderr,"<%#.8X>Allreduce::%s::CompositeT() ALERT\n",(int)this,name));
          addExecutor (&_executor);
          initialize (&_executor, req, srcbuf, dstbuf, count,
                      dtype, op, root);
          _executor.setMulticastInterface (mf);
          _executor.setSchedule ((T_Schedule*)&_schedule);
          _executor.reset ();
        }
        // Template implementation must specialize this function.
        void create_schedule(T_Sysdep        * map,
                             XMI_GEOMETRY_CLASS                  * geometry,
                             CCMI::Schedule::Color       color)
        {
          CCMI_abort();
        }
        // Template implementation must specialize this function.
        static bool analyze (XMI_GEOMETRY_CLASS *geometry)
        {
          CCMI_abort();
          return false;
        }
      }; // class CompositeT
    };
  };
};  //namespace CCMI::Adaptor::Allreduce

#endif
