/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/CachedAllSidedFactoryT.h
 * \brief ???
 */
#ifndef __algorithms_protocols_CachedAllSidedFactoryT_h__
#define __algorithms_protocols_CachedAllSidedFactoryT_h__

#include "algorithms/protocols/AllSidedCollectiveProtocolFactoryT.h"
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
///
/// Cached All Sided Factory that caches the collective in the
/// geometry and later retrieves the object. The reset method upates
/// composite with new parameters
///
template < class T_Composite, MetaDataFn get_metadata, class T_Conn, PAMI::Geometry::ckeys_t T_Key >
class CachedAllSidedFactoryT : public AllSidedCollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>
{
public:
    CachedAllSidedFactoryT(T_Conn                      *cmgr,
                           Interfaces::NativeInterface *native):
        AllSidedCollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>(cmgr, native)
    {
        TRACE_FN_ENTER();
        TRACE_FORMAT( "%p",this);
        TRACE_FN_EXIT();
    }

    virtual Executor::Composite * generate(pami_geometry_t              geometry,
                                           void                       * cmd)
    {
        TRACE_FN_ENTER();
        PAMI_GEOMETRY_CLASS  *g = ( PAMI_GEOMETRY_CLASS *)geometry;
        /// \todo does NOT support multicontext
        T_Composite *composite = (T_Composite *) g->getKey((size_t)0, T_Key);
        TRACE_FORMAT( "<%p>composite %p",this,composite);

        if (!composite)
        {
            composite = (T_Composite *) AllSidedCollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>::generate(geometry, cmd);
            TRACE_FORMAT( "<%p>composite %p",this,composite);
            g->setKey((size_t)0, /// \todo does NOT support multicontext
                      T_Key,
                      (void*)composite);
        }
        else
            //Reset composite with new collective inputs
            composite->reset (geometry, cmd);

        pami_xfer_t *xfer = (pami_xfer_t *)cmd;
        composite->setDoneCallback(xfer->cb_done, xfer->cookie);
        return composite;
        TRACE_FN_EXIT();
    }
};
};
};

#undef  DO_TRACE_ENTEREXIT
#undef  DO_TRACE_DEBUG

#endif /* __algorithms_protocols_CachedAllSidedFactoryT_H */
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
