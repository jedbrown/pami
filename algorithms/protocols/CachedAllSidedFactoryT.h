/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing,  DO NOT MODIFY OR MOVE                       */
/*  --------------------------------------------------------------- */
/* Licensed Materials - Property of IBM                             */
/* Blue Gene/Q 5765-PER 5765-PRP                                    */
/*                                                                  */
/* (C) Copyright IBM Corp. 2011, 2012 All Rights Reserved           */
/* US Government Users Restricted Rights -                          */
/* Use, duplication, or disclosure restricted                       */
/* by GSA ADP Schedule Contract with IBM Corp.                      */
/*                                                                  */
/*  --------------------------------------------------------------- */
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
template < class T_Composite, MetaDataFn get_metadata, class T_Conn, PAMI::Geometry::ckeys_t T_Key, pami_xfer_type_t T_XFER_TYPE=PAMI_XFER_COUNT >
class CachedAllSidedFactoryT : public AllSidedCollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn, T_XFER_TYPE >
{
protected:
public:
  CachedAllSidedFactoryT(pami_context_t ctxt,
                         size_t         ctxt_id,
                         pami_mapidtogeometry_fn cb_geometry,
                         T_Conn                      *cmgr,
                           Interfaces::NativeInterface *native):
    AllSidedCollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn, T_XFER_TYPE >(ctxt,ctxt_id,cb_geometry,cmgr, native)
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
            composite = (T_Composite *) AllSidedCollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn, T_XFER_TYPE >::generate(geometry, cmd);
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
        TRACE_FN_EXIT();
        return composite;
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
