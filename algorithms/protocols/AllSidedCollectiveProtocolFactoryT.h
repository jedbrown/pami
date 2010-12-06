/**
 * \file algorithms/protocols/AllSidedCollectiveProtocolFactoryT.h
 * \brief ???
 */
#ifndef __algorithms_protocols_AllSidedCollectiveProtocolFactoryT_h__
#define __algorithms_protocols_AllSidedCollectiveProtocolFactoryT_h__

#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "algorithms/interfaces/NativeInterface.h"
#include "components/memory/MemoryAllocator.h"
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
    ///
    /// \brief choose if this protocol is supports the input geometry
    ///
    typedef void      (*MetaDataFn)   (pami_metadata_t *m);

    template <class T, MetaDataFn get_metadata, class C>
    class AllSidedCollectiveProtocolFactoryT: public CollectiveProtocolFactory
    {
      class collObj
      {
      public:
        collObj(Interfaces::NativeInterface             * native,
                C                                       * cmgr,
                pami_geometry_t                           geometry,
                pami_xfer_t                             * cmd,
                pami_event_function                       fn,
                void                                    * cookie,
                AllSidedCollectiveProtocolFactoryT      * factory):
          _obj(native,cmgr,geometry,cmd,fn,cookie),
        _factory(factory),
        _user_done_fn(cmd->cb_done),
        _user_cookie(cmd->cookie)
        {
          TRACE_FN_ENTER();
          TRACE_FORMAT("<%p>",this);
          DO_DEBUG((templateName<T>()));
          TRACE_FN_EXIT();
        }
        T                                    _obj;
        AllSidedCollectiveProtocolFactoryT * _factory;
        pami_event_function                  _user_done_fn;
        void                               * _user_cookie;
      };


    public:
      AllSidedCollectiveProtocolFactoryT (C                   * cmgr,
                                  Interfaces::NativeInterface * native):
      CollectiveProtocolFactory(),
      _cmgr(cmgr),
      _native(native)
      {
        TRACE_FN_ENTER();
        TRACE_FORMAT("<%p>",this);
        TRACE_FN_EXIT();
      }

      virtual ~AllSidedCollectiveProtocolFactoryT ()
      {
      }

      /// NOTE: This is required to make "C" programs link successfully with virtual destructors
      void operator delete(void * p)
      {
        CCMI_abort();
      }

      static void done_fn(pami_context_t  context,
                          void          * clientdata,
                          pami_result_t   res)
      {
        TRACE_FN_ENTER();
        collObj *cobj = (collObj *)clientdata;
        TRACE_FORMAT("<%p> cobj %p",cobj->_factory, cobj);
        cobj->_user_done_fn(context, cobj->_user_cookie, res);
        cobj->_factory->_alloc.returnObject(cobj);
        TRACE_FN_EXIT();
      }


      virtual Executor::Composite * generate(pami_geometry_t             geometry,
                                             void                      * cmd)
      {
        TRACE_FN_ENTER();
        collObj *cobj = (collObj*)  _alloc.allocateObject();
        TRACE_FORMAT("<%p> cobj %p",this, cobj);
        new(cobj) collObj(_native,          // Native interface
                          _cmgr,            // Connection Manager
                          geometry,         // Geometry Object
                          (pami_xfer_t*)cmd,// Parameters
                          done_fn,          // Intercept function
                          cobj,             // Intercept cookie
                          this);            // Factory
        TRACE_FN_EXIT();
        return(Executor::Composite *)&cobj->_obj;
      }

      virtual void metadata(pami_metadata_t *mdata)
      {
        TRACE_FN_ENTER();
        TRACE_FORMAT("mdata=%p",mdata);
        get_metadata(mdata);
        TRACE_FN_EXIT();
      }
    private:
      C                                          * _cmgr;
      Interfaces::NativeInterface                * _native;
      PAMI::MemoryAllocator<sizeof(collObj), 16>   _alloc;
    };//AllSidedCollectiveProtocolFactoryT

  };//Adaptor
};//CCMI

#endif
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
