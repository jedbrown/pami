
#ifndef __common_bgq_NativeInterfaceFactory__
#define __common_bgq_NativeInterfaceFactory__

#include  "util/trace.h"
#include  "algorithms/interfaces/NativeInterface.h"
#include  "algorithms/interfaces/NativeInterfaceFactory.h"

#undef DO_TRACE_ENTEREXIT
#undef DO_TRACE_DEBUG

#ifdef CCMI_TRACE_ALL
#define DO_TRACE_ENTEREXIT 1
#define DO_TRACE_DEBUG     1
#else
#define DO_TRACE_ENTEREXIT 0
#define DO_TRACE_DEBUG     0
#endif

namespace PAMI {

  template <class T_Allocator, class T_NI, class T_Device, CCMI::Interfaces::NativeInterfaceFactory::NISelect T_Sel, CCMI::Interfaces::NativeInterfaceFactory::NIType T_Type>
    class BGQNativeInterfaceFactory : public CCMI::Interfaces::NativeInterfaceFactory {
  protected:
    pami_client_t       _client;
    pami_context_t      _context;
    size_t              _client_id;	
    size_t              _context_id;
    T_Device          & _device;
    T_Allocator       & _allocator;
    
  public:
    
    BGQNativeInterfaceFactory ( pami_client_t       client,
				pami_context_t      context,
				size_t              clientid,
				size_t              contextid,
				T_Device          & device,
				T_Allocator       & allocator) : 
    CCMI::Interfaces::NativeInterfaceFactory(),
      _client (client),
      _context (context),
      _client_id  (clientid),
      _context_id (contextid),
      _device(device),
      _allocator(allocator)
      {	
        TRACE_FN_ENTER();
        COMPILE_TIME_ASSERT(sizeof(T_NI) <= T_Allocator::objsize);
        TRACE_FORMAT("Allocator:  sizeof(T_NI) %zu, T_Allocator::objsize %zu",sizeof(T_NI),T_Allocator::objsize);
        TRACE_FN_EXIT();
      }
    

    /// \brief Construct a P2p Native Interface
    /// \details
    ///
    ///  A native interface is constructed.
    ///
    ///  Then a P2P protocol is constructed from the device and using the
    ///  same dispatch id and using the native interface's dispatch function
    ///  and the native interface as a cookie.
    ///
    ///  Finally, the P2P protocol is set into the native interface.
    ///
    virtual pami_result_t generate (int                                                        *   dispatch_id,
                                    CCMI::Interfaces::NativeInterfaceFactory::NISelect             ni_select,
                                    CCMI::Interfaces::NativeInterfaceFactory::NIType               ni_type,
                                    CCMI::Interfaces::NativeInterface                          *&  ni)
    {
      TRACE_FN_ENTER();
      pami_result_t result = PAMI_ERROR;
      ni = NULL;           
      if (T_Sel != ni_select)
      {  
        TRACE_FN_EXIT();
        return result;
      }

      if (T_Type != ni_type)
      {  
        TRACE_FN_EXIT();
        return result;
      }
      
      result = PAMI_SUCCESS;
      // Construct the protocol(s) using the NI dispatch function and cookie
      ni = (CCMI::Interfaces::NativeInterface *) _allocator.allocateObject ();
      TRACE_FORMAT("<%p> ni %p", this,  ni);
      new ((void *)ni) T_NI (_device, _client, _context, _context_id, _client_id, dispatch_id);
     
      TRACE_FN_EXIT();
      return result;
    }
    virtual pami_result_t analyze(size_t context_id, void *geometry, int phase, int* flag){return PAMI_SUCCESS;};
  
  };


#if 0
  template <class T_Allocator, class T_NI, class T_Device1, class T_Device2, CCMI::Interfaces::NativeInterface::NISelect T_Sel, CCMI::Interfaces::NativeInterface::NIType T_Type>
    class BGQNativeInterfaceFactory2Device : public CCMI::Interfaces::NativeInterfaceFactory {
  protected:
    pami_client_t       _client;
    pami_context_t      _context;
    size_t              _client_id;	
    size_t              _context_id;
    T_Device1         & _device1;
    T_Device2         & _device2;
    T_Allocator       & _allocator;
    
  public:
    
    NativeInterfaceFactory ( pami_client_t       client,
			     pami_context_t      context,
			     size_t              clientid,
			     size_t              contextid,
			     T_Device1         & device1,
			     T_Device2         & device2,
			     T_Allocator       & allocator) : 
    CCMI::Interfaces::NativeInterfaceFactory(),
      _client (client),
      _context (context),
      _client_id  (clientid),
      _context_id (contextid),
      _device(device),
      _allocator(allocator)
      {	
        COMPILE_TIME_ASSERT(sizeof(T_NI) <= T_Allocator::objsize);
        TRACE_FN_ENTER();
        TRACE_FORMAT("Allocator:  sizeof(T_NI) %zu, T_Allocator::objsize %zu",sizeof(T_NI),T_Allocator::objsize);
        TRACE_FN_EXIT();
      }
    

    /// \brief Construct a P2p Native Interface
    /// \details
    ///
    ///  A native interface is constructed.
    ///
    ///  Then a P2P protocol is constructed from the device and using the
    ///  same dispatch id and using the native interface's dispatch function
    ///  and the native interface as a cookie.
    ///
    ///  Finally, the P2P protocol is set into the native interface.
    ///
    virtual pami_result_t generate (int                                                        *   dispatch_id,
				    CCMI::Interfaces::NativeInterfaceFactory::NISelect             ni_select,
				    CCMI::Interfaces::NativeInterfaceFactory::NIType               ni_type,
				    CCMI::Interfaces::NativeInterface                          *&  ni)
    {
      TRACE_FN_ENTER();
      pami_result_t result = PAMI_ERROR;
      ni = NULL;           
      if (T_Sel != ni_select)
      {
        TRACE_FN_EXIT();
            return result;
      }

      if (T_Type != ni_type)
      {
        TRACE_FN_EXIT();
        return result;
      }
      
      // Construct the protocol(s) using the NI dispatch function and cookie
      ni = (CCMI::Interfaces::NativeInterface *) _allocator.allocateObject ();
      TRACE_FORMAT("<%p> ni %p", this,  ni);
      new ((void *)ni) T_NI (_client, _context, _context_id, _client_id, dispatch_id, _device, result);
      
      TRACE_FN_EXIT();
      return result;
    }
  
  };
#endif

};

#undef DO_TRACE_ENTEREXIT
#undef DO_TRACE_DEBUG

#endif      
