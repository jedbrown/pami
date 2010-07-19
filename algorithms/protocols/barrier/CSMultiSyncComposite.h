/**
 * \file algorithms/protocols/barrier/MultiSyncComposite.h
 * \brief Simple composite based on multisync
 */
#ifndef __algorithms_protocols_barrier_CSMultiSyncComposite_h__
#define __algorithms_protocols_barrier_CSMultiSyncComposite_h__

#include "algorithms/executor/Barrier.h"
#include "algorithms/composite/Composite.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Barrier
    {

      class CSMultiSyncComposite : public CCMI::Executor::Composite {
      protected:
        PAMI_GEOMETRY_CLASS                * _geometry;
        pami_multisync_t                     _minfo;
        Interfaces::NativeInterface        * _native;

      public:
        CSMultiSyncComposite (pami_geometry_t                       g,
                            void                                 * cmd,
                            pami_event_function                     fn,
                            void                                 * cookie) :
        Composite(), _geometry((PAMI_GEOMETRY_CLASS*)g)
        {
          TRACE_ADAPTOR((stderr,"%s\n", __PRETTY_FUNCTION__));

          // setDoneCallback(fn, cookie);
 
          _native = (Interfaces::NativeInterface *)_geometry->getKey(PAMI::Geometry::PAMI_GKEY_GEOMETRYCSNI);

         _minfo.cb_done.function   = fn;
         _minfo.cb_done.clientdata = cookie;
         _minfo.connection_id      = 0;
         _minfo.roles              = -1U;
         _minfo.participants       = _geometry->getTopology(0);

        }

        virtual void start() {
          TRACE_ADAPTOR((stderr,"%s\n", __PRETTY_FUNCTION__));
          // _minfo.cb_done.function   = _cb_done;
          // _minfo.cb_done.clientdata = _clientdata;
          _native->multisync(&_minfo);
        }
      };
    };
  };
};

#endif
