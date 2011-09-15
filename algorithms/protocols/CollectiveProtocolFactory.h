/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/CollectiveProtocolFactory.h
 * \brief ???
 */

#ifndef __algorithms_protocols_CollectiveProtocolFactory_h__
#define __algorithms_protocols_CollectiveProtocolFactory_h__

//#include "TypeDefs.h"
#include  "algorithms/interfaces/Executor.h"
#include  "algorithms/composite/Composite.h"

#include "algorithms/interfaces/NativeInterface.h"


#include "util/ccmi_debug.h"
#include "util/ccmi_util.h"

#include "util/trace.h"

#ifdef CCMI_TRACE_ALL
  #define DO_TRACE_ENTEREXIT 1
  #define DO_TRACE_DEBUG     1
#else
  #define DO_TRACE_ENTEREXIT 0
  #define DO_TRACE_DEBUG     0
#endif


namespace CCMI
{
  namespace Adaptor
  {

    class CollectiveProtocolFactory
    {
      public:
        CollectiveProtocolFactory ():
          _cb_geometry(NULL),
          _context(NULL),
          _native(NULL)
        {
          TRACE_FN_ENTER();
          TRACE_FORMAT("<%p>",this);
          TRACE_FN_EXIT();
        }
        CollectiveProtocolFactory (Interfaces::NativeInterface  * native):
          _cb_geometry(NULL),
          _context(NULL),
          _native(native)
        {
          TRACE_FN_ENTER();
          TRACE_FORMAT("<%p> native %p",this,native);
          TRACE_FN_EXIT();
        }
        static void cleanup_done_fn(pami_context_t  context,
                                  void           *clientdata,
                                  pami_result_t   res)
        {
          PAMI_abort();
        }

        void setMapIdToGeometry(pami_mapidtogeometry_fn     cb_geometry)
        {
          TRACE_FN_ENTER();
          TRACE_FORMAT("<%p> cb_geometry %p",this,cb_geometry);
          _cb_geometry = cb_geometry;
          TRACE_FN_EXIT();
        }


        pami_geometry_t getGeometry(pami_context_t ctxt, unsigned id)
        {
          TRACE_FN_ENTER();
          TRACE_FORMAT("<%p>",this);
          CCMI_assert (_cb_geometry != NULL);
          TRACE_FN_EXIT();
          return _cb_geometry (ctxt, id);  // -1, the function is scoped to the geometry
        }

        virtual ~CollectiveProtocolFactory ()
        {
          TRACE_FN_ENTER();
          TRACE_FORMAT("<%p>",this);
          TRACE_FN_EXIT();
        }

        /// NOTE: This is required to make "C" programs link successfully with virtual destructors
        void operator delete(void * p)
        {
          CCMI_abort();
        }

        virtual Executor::Composite * generate(pami_geometry_t              geometry,
                                               void                      * cmd) = 0;

        virtual void metadata(pami_metadata_t *mdata)=0;
        inline void metadata(pami_metadata_t *m, pami_xfer_type_t t)
        {
          TRACE_FN_ENTER();
          // trace input
          TRACE_FORMAT("<%p:%p> %s  metatadata %p, type %u",this,_native,m->name,m,t);
          TRACE_HEXDATA(m, sizeof(*m));
          if(_native) _native->metadata(m,t);
          // trace overrides
          TRACE_FORMAT("<%p:%p> %s  metatadata %p, type %u",this,_native,m->name,m,t);
          TRACE_HEXDATA(m, sizeof(*m));
          TRACE_FN_EXIT();
        }

        virtual void setAsyncInfo (bool                          is_buffered,
                                   pami_dispatch_callback_function cb_async,
                                   pami_mapidtogeometry_fn        cb_geometry)
        {
          TRACE_FN_ENTER();
          TRACE_FORMAT("<%p>",this);
          TRACE_FN_EXIT();
          PAMI_abort();
        };

        pami_mapidtogeometry_fn getMapIdToGeometry()
        {
          TRACE_FN_ENTER();
          TRACE_FORMAT("<%p> _cb_geometry %p",this,_cb_geometry);
          TRACE_FN_EXIT();
          return _cb_geometry;
        }

        inline void setContext(pami_context_t ctxt) 
        {
          TRACE_FN_ENTER();
          _context=ctxt;
          TRACE_FORMAT("<%p> _context %p",this,_context);
          TRACE_FN_EXIT();
        }
        inline pami_context_t getContext() 
        {
          TRACE_FN_ENTER();
          TRACE_FORMAT("<%p> _context %p",this,_context);
          TRACE_FN_EXIT();
          return _context;
        }
      protected:
        pami_mapidtogeometry_fn              _cb_geometry;
        pami_context_t                       _context;
        Interfaces::NativeInterface        * _native;
    };
  };
};


#undef  DO_TRACE_ENTEREXIT
#undef  DO_TRACE_DEBUG

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
