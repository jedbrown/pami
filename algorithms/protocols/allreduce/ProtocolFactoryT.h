/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/ProtocolFactoryT.h
 * \brief ???
 */
#ifndef __algorithms_protocols_allreduce_ProtocolFactoryT_h__
#define __algorithms_protocols_allreduce_ProtocolFactoryT_h__

#include "algorithms/protocols/CollectiveProtocolFactoryT.h"
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
namespace Allreduce
{
template <class T_Composite, MetaDataFn get_metadata, class T_Conn>
class ProtocolFactoryT: public CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>
{
public:
    ProtocolFactoryT (T_Conn                      *cmgr,
                      Interfaces::NativeInterface *native,
                      pami_dispatch_multicast_function cb_head = NULL):
        CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>(cmgr, native, cb_head)
    {
        TRACE_FN_ENTER();
        TRACE_FORMAT("%p", this);
        TRACE_FN_EXIT();
    }

    virtual ~ProtocolFactoryT ()
    {
        TRACE_FN_ENTER();
        TRACE_FORMAT("%p", this);
        TRACE_FN_EXIT();
    }

    /// NOTE: This is required to make "C" programs link successfully with virtual destructors
    void operator delete(void * p)
    {
        CCMI_abort();
    }

    virtual Executor::Composite * generate(pami_geometry_t              g,
                                           void                      * cmd)
    {
        TRACE_FN_ENTER();
        PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *)g;
        T_Composite * arcomposite = (T_Composite *)geometry->getAllreduceComposite();
        TRACE_FORMAT("%p composite %p", this,arcomposite);

        pami_xfer_t *allreduce = (pami_xfer_t *)cmd;

        ///If the allreduce algorithm was created by this factory before, just restart it
        if (arcomposite != NULL  &&  arcomposite->getAlgorithmFactory() == this)
        {
            pami_result_t status =  (pami_result_t)arcomposite->restart(allreduce);

            if (status == PAMI_SUCCESS)
            {
                geometry->setAllreduceComposite(arcomposite);
                TRACE_FN_EXIT();
                return NULL;
            }
        }

        if (arcomposite != NULL) // Different factory?  Cleanup old executor.
        {
            geometry->setAllreduceComposite(NULL);
            arcomposite->~T_Composite(); //Call destructor
            CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>::_alloc.returnObject(arcomposite);
        }

        T_Composite* obj = (T_Composite*)CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>::_alloc.allocateObject();
        TRACE_FORMAT("%p composite %p", this,arcomposite);
        obj->setContext(this->_context);
        geometry->setAllreduceComposite(obj);
        new (obj) T_Composite(CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>::_native,  // Native interface
                              CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>::_cmgr,    // Connection Manager
                              geometry,          // Geometry Object
                              (pami_xfer_t*) cmd, // Parameters
                              allreduce->cb_done,
                              allreduce->cookie);

        TRACE_FN_EXIT();
        return NULL;
    }
};
};
};
}; //CCMI

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
