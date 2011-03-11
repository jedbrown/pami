/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file p2p/protocols/send/eager/Eager.h
/// \brief Eager send protocol for reliable devices.
///
/// The Eager class defined in this file uses C++ templates and
/// the device "message" interface - which also uses C++ templates.
///
/// C++ templates require all source code to be #include'd from a header file.
///
#ifndef __p2p_protocols_send_eager_Eager_h__
#define __p2p_protocols_send_eager_Eager_h__

#include "p2p/protocols/Send.h"
#include "p2p/protocols/send/eager/EagerSimple.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) // fprintf x
#endif

namespace PAMI
{
  namespace Protocol
  {
    namespace Send
    {
      ///
      /// \brief Eager send protocol factory for deterministic, reliable network devices.
      ///
      /// \tparam T_Model Packet model interface implementation class
      ///
      /// \see PAMI::Device::Interface::PacketModel
      ///
      template < class T_ModelPrimary, class T_ModelSecondary = T_ModelPrimary >
      class Eager
      {
        public:

          template <configuration_t T_Option, bool T_Composite >
          class EagerImpl : public PAMI::Protocol::Send::Send
          {
            public:

              ///
              /// \brief Eager send protocol constructor.
              ///
              /// \param [in]  dispatch    Dispatch identifier
              /// \param [in]  dispatch_fn Point-to-point dispatch callback function
              /// \param [in]  cookie      Opaque application dispatch data
              /// \param [in]  device      Device that implements the message interface
              /// \param [in]  origin      Origin endpoint
              /// \param [in]  context     Origin context
              /// \param [in]  hint        Dispatch 'hard' hints
              /// \param [out] status      Constructor status
              ///
              template <class T_DevicePrimary, class T_DeviceSecondary>
              inline EagerImpl (size_t                       dispatch,
                                pami_dispatch_p2p_function   dispatch_fn,
                                void                       * cookie,
                                T_DevicePrimary            & device0,
                                T_DeviceSecondary          & device1,
                                pami_endpoint_t              origin,
                                pami_context_t               context,
                                pami_dispatch_hint_t         hint,
                                pami_result_t              & status) :
                  PAMI::Protocol::Send::Send (),
                  _primary (device0),
                  _secondary (device1)
              {
                status = _primary.initialize (dispatch, dispatch_fn, cookie, origin, context, hint);

                if (T_Composite && status != PAMI_SUCCESS)
                  {
                    status = _secondary.initialize (dispatch, dispatch_fn, cookie, origin, context, hint);
                  }
              };

              virtual ~EagerImpl () {};

              /// \note This is required to make "C" programs link successfully
              ///       with virtual destructors
              inline void operator delete(void * p)
              {
                PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
              }

              ///
              /// \brief Query the value of one or more attributes
              ///
              /// \see PAMI::Protocol::Send::getAttributes
              ///
              virtual pami_result_t getAttributes (pami_configuration_t  configuration[],
                                                   size_t                num_configs)
              {
                size_t i;

                for (i = 0; i < num_configs; i++)
                  {
                    switch (configuration[i].name)
                      {
                        case PAMI_DISPATCH_RECV_IMMEDIATE_MAX:
                          configuration[i].value.intval = EagerSimple<T_ModelPrimary, T_Option>::recv_immediate_max;

                          if (T_Composite)
                            {
                              if (EagerSimple<T_ModelSecondary, T_Option>::recv_immediate_max < configuration[i].value.intval)
                                configuration[i].value.intval = EagerSimple<T_ModelSecondary, T_Option>::recv_immediate_max;
                            }

                          break;
                        case PAMI_DISPATCH_SEND_IMMEDIATE_MAX:
                          configuration[i].value.intval = T_ModelPrimary::packet_model_immediate_bytes;

                          if (T_Composite)
                            {
                              if (T_ModelSecondary::packet_model_immediate_bytes < configuration[i].value.intval)
                                configuration[i].value.intval = T_ModelSecondary::packet_model_immediate_bytes;
                            }

                          break;
                        default:
                          return PAMI_INVAL;
                          break;
                      };
                  };

                return PAMI_SUCCESS;
              };

              ///
              /// \brief Start a new immediate send operation.
              ///
              /// \see PAMI::Protocol::Send::immediate
              ///
              virtual pami_result_t immediate (pami_send_immediate_t * parameters)
              {
                TRACE_ERR((stderr, ">> EagerImpl::immediate()\n"));
                pami_result_t result = _primary.immediate_impl (parameters);

                if (T_Composite && result != PAMI_SUCCESS)
                  {
                    result = _secondary.immediate_impl (parameters);
                  }

                TRACE_ERR((stderr, "<< EagerImpl::immediate()\n"));
                return result;
              };

              ///
              /// \brief Start a new simple send operation.
              ///
              /// \see PAMI::Protocol::Send::simple
              ///
              virtual pami_result_t simple (pami_send_t * parameters)
              {
                TRACE_ERR((stderr, ">> EagerImpl::simple()\n"));
                pami_result_t result = _primary.simple_impl (parameters);

                if (T_Composite && result != PAMI_SUCCESS)
                  {
                    result = _secondary.simple_impl (parameters);
                  }

                TRACE_ERR((stderr, "<< EagerImpl::simple()\n"));
                return result;
              };

            protected:

              EagerSimple<T_ModelPrimary, T_Option>   _primary;
              EagerSimple<T_ModelSecondary, T_Option> _secondary;

          }; // PAMI::Protocol::Send::EagerImpl class

        protected:

          template <class T_Allocator, class T_DevicePrimary, class T_DeviceSecondary>
          static Send * generate (size_t                       dispatch,
                                  pami_dispatch_p2p_function   dispatch_fn,
                                  void                       * cookie,
                                  T_DevicePrimary            & device0,
                                  T_DeviceSecondary          & device1,
                                  pami_endpoint_t              origin,
                                  pami_context_t               context,
                                  pami_dispatch_hint_t         options,
                                  T_Allocator                & allocator,
                                  pami_result_t              & result,
                                  bool                         composite)
          {
            TRACE_ERR((stderr, ">> Eager::generate() dispatch %zu\n", dispatch));

            // Return an error for invalid / unimplemented 'hard' hints.
            if (
              options.remote_async_progress == PAMI_HINT_ENABLE  ||
              options.use_rdma              == PAMI_HINT_ENABLE  ||
              false)
              {
                result = PAMI_ERROR;
                return (Send *) NULL;
              }

            void * eager = allocator.allocateObject ();

            if (options.queue_immediate == PAMI_HINT_DISABLE)
              {
                const configuration_t queue_immediate = (configuration_t) (QUEUE_IMMEDIATE_DISABLE);

                if (options.long_header == PAMI_HINT_DISABLE)
                  {
                    const configuration_t long_header = (configuration_t) (queue_immediate | LONG_HEADER_DISABLE);

                    if (options.recv_immediate == PAMI_HINT_ENABLE)
                      {
                        const configuration_t hint = (configuration_t) (long_header | RECV_IMMEDIATE_FORCEON);
                        COMPILE_TIME_ASSERT(sizeof(EagerImpl<hint, true>) <= T_Allocator::objsize);
                        COMPILE_TIME_ASSERT(sizeof(EagerImpl<hint, false>) <= T_Allocator::objsize);

                        if (composite)
                          new (eager) EagerImpl<hint, true> (dispatch, dispatch_fn, cookie, device0, device1, origin, context, options, result);
                        else
                          new (eager) EagerImpl<hint, false>(dispatch, dispatch_fn, cookie, device0, device1, origin, context, options, result);
                      }
                    else if (options.recv_immediate == PAMI_HINT_DISABLE)
                      {
                        const configuration_t hint = (configuration_t) (long_header | RECV_IMMEDIATE_FORCEOFF);
                        COMPILE_TIME_ASSERT(sizeof(EagerImpl<hint, true>) <= T_Allocator::objsize);
                        COMPILE_TIME_ASSERT(sizeof(EagerImpl<hint, false>) <= T_Allocator::objsize);

                        if (composite)
                          new (eager) EagerImpl<hint, true> (dispatch, dispatch_fn, cookie, device0, device1, origin, context, options, result);
                        else
                          new (eager) EagerImpl<hint, false>(dispatch, dispatch_fn, cookie, device0, device1, origin, context, options, result);
                      }
                    else
                      {
                        const configuration_t hint = (configuration_t) (long_header);
                        COMPILE_TIME_ASSERT(sizeof(EagerImpl<hint, true>) <= T_Allocator::objsize);
                        COMPILE_TIME_ASSERT(sizeof(EagerImpl<hint, false>) <= T_Allocator::objsize);

                        if (composite)
                          new (eager) EagerImpl<hint, true> (dispatch, dispatch_fn, cookie, device0, device1, origin, context, options, result);
                        else
                          new (eager) EagerImpl<hint, false>(dispatch, dispatch_fn, cookie, device0, device1, origin, context, options, result);
                      }
                  }
                else
                  {
                    const configuration_t long_header = (configuration_t) (DEFAULT);

                    if (options.recv_immediate == PAMI_HINT_ENABLE)
                      {
                        const configuration_t hint = (configuration_t) (long_header | RECV_IMMEDIATE_FORCEON);
                        COMPILE_TIME_ASSERT(sizeof(EagerImpl<hint, true>) <= T_Allocator::objsize);
                        COMPILE_TIME_ASSERT(sizeof(EagerImpl<hint, false>) <= T_Allocator::objsize);

                        if (composite)
                          new (eager) EagerImpl<hint, true> (dispatch, dispatch_fn, cookie, device0, device1, origin, context, options, result);
                        else
                          new (eager) EagerImpl<hint, false>(dispatch, dispatch_fn, cookie, device0, device1, origin, context, options, result);
                      }
                    else if (options.recv_immediate == PAMI_HINT_DISABLE)
                      {
                        const configuration_t hint = (configuration_t) (long_header | RECV_IMMEDIATE_FORCEOFF);
                        COMPILE_TIME_ASSERT(sizeof(EagerImpl<hint, true>) <= T_Allocator::objsize);
                        COMPILE_TIME_ASSERT(sizeof(EagerImpl<hint, false>) <= T_Allocator::objsize);

                        if (composite)
                          new (eager) EagerImpl<hint, true> (dispatch, dispatch_fn, cookie, device0, device1, origin, context, options, result);
                        else
                          new (eager) EagerImpl<hint, false>(dispatch, dispatch_fn, cookie, device0, device1, origin, context, options, result);
                      }
                    else
                      {
                        const configuration_t hint = (configuration_t) (long_header);
                        COMPILE_TIME_ASSERT(sizeof(EagerImpl<hint, true>) <= T_Allocator::objsize);
                        COMPILE_TIME_ASSERT(sizeof(EagerImpl<hint, false>) <= T_Allocator::objsize);

                        if (composite)
                          new (eager) EagerImpl<hint, true> (dispatch, dispatch_fn, cookie, device0, device1, origin, context, options, result);
                        else
                          new (eager) EagerImpl<hint, false>(dispatch, dispatch_fn, cookie, device0, device1, origin, context, options, result);
                      }
                  }

              }
            else
              {
                const configuration_t queue_immediate = (configuration_t) (DEFAULT);

                if (options.long_header == PAMI_HINT_DISABLE)
                  {
                    const configuration_t long_header = (configuration_t) (queue_immediate | LONG_HEADER_DISABLE);

                    if (options.recv_immediate == PAMI_HINT_ENABLE)
                      {
                        const configuration_t hint = (configuration_t) (long_header | RECV_IMMEDIATE_FORCEON);
                        COMPILE_TIME_ASSERT(sizeof(EagerImpl<hint, true>) <= T_Allocator::objsize);
                        COMPILE_TIME_ASSERT(sizeof(EagerImpl<hint, false>) <= T_Allocator::objsize);

                        if (composite)
                          new (eager) EagerImpl<hint, true> (dispatch, dispatch_fn, cookie, device0, device1, origin, context, options, result);
                        else
                          new (eager) EagerImpl<hint, false>(dispatch, dispatch_fn, cookie, device0, device1, origin, context, options, result);
                      }
                    else if (options.recv_immediate == PAMI_HINT_DISABLE)
                      {
                        const configuration_t hint = (configuration_t) (long_header | RECV_IMMEDIATE_FORCEOFF);
                        COMPILE_TIME_ASSERT(sizeof(EagerImpl<hint, true>) <= T_Allocator::objsize);
                        COMPILE_TIME_ASSERT(sizeof(EagerImpl<hint, false>) <= T_Allocator::objsize);

                        if (composite)
                          new (eager) EagerImpl<hint, true> (dispatch, dispatch_fn, cookie, device0, device1, origin, context, options, result);
                        else
                          new (eager) EagerImpl<hint, false>(dispatch, dispatch_fn, cookie, device0, device1, origin, context, options, result);
                      }
                    else
                      {
                        const configuration_t hint = (configuration_t) (long_header);
                        COMPILE_TIME_ASSERT(sizeof(EagerImpl<hint, true>) <= T_Allocator::objsize);
                        COMPILE_TIME_ASSERT(sizeof(EagerImpl<hint, false>) <= T_Allocator::objsize);

                        if (composite)
                          new (eager) EagerImpl<hint, true> (dispatch, dispatch_fn, cookie, device0, device1, origin, context, options, result);
                        else
                          new (eager) EagerImpl<hint, false>(dispatch, dispatch_fn, cookie, device0, device1, origin, context, options, result);
                      }
                  }
                else
                  {
                    const configuration_t long_header = (configuration_t) (DEFAULT);

                    if (options.recv_immediate == PAMI_HINT_ENABLE)
                      {
                        const configuration_t hint = (configuration_t) (long_header | RECV_IMMEDIATE_FORCEON);
                        COMPILE_TIME_ASSERT(sizeof(EagerImpl<hint, true>) <= T_Allocator::objsize);
                        COMPILE_TIME_ASSERT(sizeof(EagerImpl<hint, false>) <= T_Allocator::objsize);

                        if (composite)
                          new (eager) EagerImpl<hint, true> (dispatch, dispatch_fn, cookie, device0, device1, origin, context, options, result);
                        else
                          new (eager) EagerImpl<hint, false>(dispatch, dispatch_fn, cookie, device0, device1, origin, context, options, result);
                      }
                    else if (options.recv_immediate == PAMI_HINT_DISABLE)
                      {
                        const configuration_t hint = (configuration_t) (long_header | RECV_IMMEDIATE_FORCEOFF);
                        COMPILE_TIME_ASSERT(sizeof(EagerImpl<hint, true>) <= T_Allocator::objsize);
                        COMPILE_TIME_ASSERT(sizeof(EagerImpl<hint, false>) <= T_Allocator::objsize);

                        if (composite)
                          new (eager) EagerImpl<hint, true> (dispatch, dispatch_fn, cookie, device0, device1, origin, context, options, result);
                        else
                          new (eager) EagerImpl<hint, false>(dispatch, dispatch_fn, cookie, device0, device1, origin, context, options, result);
                      }
                    else
                      {
                        const configuration_t hint = (configuration_t) (long_header);
                        COMPILE_TIME_ASSERT(sizeof(EagerImpl<hint, true>) <= T_Allocator::objsize);
                        COMPILE_TIME_ASSERT(sizeof(EagerImpl<hint, false>) <= T_Allocator::objsize);

                        if (composite)
                          new (eager) EagerImpl<hint, true> (dispatch, dispatch_fn, cookie, device0, device1, origin, context, options, result);
                        else
                          new (eager) EagerImpl<hint, false>(dispatch, dispatch_fn, cookie, device0, device1, origin, context, options, result);
                      }
                  }

              }

            if (result != PAMI_SUCCESS)
              {
                allocator.returnObject (eager);
                eager = NULL;
              }

            TRACE_ERR((stderr, "<< Eager::generate(), eager = %p, result = %d, dispatch = %zu\n", eager, result, dispatch));
            return (Send *) eager;
          };

        public:

          template <class T_Allocator, class T_DevicePrimary, class T_DeviceSecondary>
          static Send * generate (size_t                       dispatch,
                                  pami_dispatch_p2p_function   dispatch_fn,
                                  void                       * cookie,
                                  T_DevicePrimary            & device0,
                                  T_DeviceSecondary          & device1,
                                  pami_endpoint_t              origin,
                                  pami_context_t               context,
                                  pami_dispatch_hint_t         options,
                                  T_Allocator                & allocator,
                                  pami_result_t              & result)
          {
            return generate (dispatch, dispatch_fn, cookie, device0, device1, origin, context, options, allocator, result, true);
          };

          template <class T_Allocator, class T_Device>
          static Send * generate (size_t                       dispatch,
                                  pami_dispatch_p2p_function   dispatch_fn,
                                  void                       * cookie,
                                  T_Device                   & device,
                                  pami_endpoint_t              origin,
                                  pami_context_t               context,
                                  pami_dispatch_hint_t         options,
                                  T_Allocator                & allocator,
                                  pami_result_t              & result)
          {
            return generate (dispatch, dispatch_fn, cookie, device, device, origin, context, options, allocator, result, false);
          };
      };     // PAMI::Protocol::Send::Eager class
    };       // PAMI::Protocol::Send namespace
  };         // PAMI::Protocol namespace
};           // PAMI namespace
#undef TRACE_ERR
#endif // __pami_p2p_protocol_send_eager_eager_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
