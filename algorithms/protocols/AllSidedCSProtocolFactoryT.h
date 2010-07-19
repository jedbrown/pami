/**
 * \file algorithms/protocols/AllSidedCSProtocolFactoryT.h
 * \brief ???
 */
#ifndef __algorithms_protocols_AllSidedCSProtocolFactoryT_h__
#define __algorithms_protocols_AllSidedCSProtocolFactoryT_h__

#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "algorithms/interfaces/NativeInterface.h"
#include "components/memory/MemoryAllocator.h"
#include "util/ccmi_util.h"

namespace CCMI
{
  namespace Adaptor
  {
    ///
    /// \brief choose if this protocol supports the input geometry
    ///
    typedef void      (*MetaDataFn)   (pami_metadata_t *m);

    template <class T, MetaDataFn get_metadata>
    class AllSidedCSProtocolFactoryT: public CollectiveProtocolFactory
    {
      class collObj
      {
      public:
        collObj(pami_geometry_t                           geometry,
                pami_xfer_t                             * cmd,
                pami_event_function                       fn,
                void                                    * cookie,
                AllSidedCSProtocolFactoryT      * factory):
        _obj(geometry,cmd,fn,cookie),
        _factory(factory),
        _user_done_fn(cmd->cb_done),
        _user_cookie(cmd->cookie)
        {
          TRACE_ADAPTOR((stderr,"%s\n", __PRETTY_FUNCTION__));
          DO_DEBUG((templateName<T>()));
        }
        T                                    _obj;
        AllSidedCSProtocolFactoryT         * _factory;
        pami_event_function                  _user_done_fn;
        void                               * _user_cookie;
      };


    public:
      AllSidedCSProtocolFactoryT ():
      CollectiveProtocolFactory()
      {
        TRACE_ADAPTOR((stderr,"%s\n", __PRETTY_FUNCTION__));
      }

      virtual ~AllSidedCSProtocolFactoryT ()
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
        TRACE_ADAPTOR((stderr,"%s\n", __PRETTY_FUNCTION__));
        // printf("factory done function gets activated\n");
        collObj *cobj = (collObj *)clientdata;
        AllSidedCSProtocolFactoryT  * factory = cobj->_factory;
        cobj->_user_done_fn(context, cobj->_user_cookie, res);
        cobj->~collObj();
        factory->_alloc.returnObject(cobj);
      }


      virtual Executor::Composite * generate(pami_geometry_t             geometry,
                                             void                      * cmd)
      {
        TRACE_ADAPTOR((stderr,"%s\n", __PRETTY_FUNCTION__));
        collObj *cobj = (collObj*)  _alloc.allocateObject();
        new(cobj) collObj(geometry,         // Geometry Object
                          (pami_xfer_t*)cmd,// Parameters
                          done_fn,          // Intercept function
                          cobj,             // Intercept cookie
                          this);            // Factory
        return(Executor::Composite *)&cobj->_obj;
      }

      virtual void metadata(pami_metadata_t *mdata)
      {
        get_metadata(mdata);
      }
    private:
      PAMI::MemoryAllocator<sizeof(collObj), 16>   _alloc;
    };//AllSidedCSProtocolFactoryT

  };//Adaptor
};//CCMI

#endif
