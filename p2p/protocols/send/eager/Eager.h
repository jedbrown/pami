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
      /// \brief Eager simple send protocol class for reliable network devices.
      ///
      /// \tparam T_Model   Template packet model class
      ///
      /// \see PAMI::Device::Interface::PacketModel
      ///
      template < class T_Model, configuration_t T_Option = UNSET >
      class Eager : public PAMI::Protocol::Send::Send,
          public EagerSimple<T_Model, T_Option>
      {
        public:

          template <class T_Allocator, class T_Device>
          static Eager * generate (size_t                 dispatch,
                                   pami_dispatch_p2p_function dispatch_fn,
                                   void                 * cookie,
                                   T_Device             & device,
                                   pami_endpoint_t        origin,
                                   pami_context_t         context,
                                   pami_dispatch_hint_t   hint,
                                   T_Allocator          & allocator,
                                   pami_result_t        & result)
          {
            TRACE_ERR((stderr, ">> Eager::generate() dispatch %zu\n", dispatch));
            COMPILE_TIME_ASSERT(sizeof(Eager) <= T_Allocator::objsize);

            Eager * eager = (Eager *) allocator.allocateObject ();
            new ((void *)eager) Eager (dispatch, dispatch_fn, cookie, device, origin, context, hint, result);

            if (result != PAMI_SUCCESS)
              {
                allocator.returnObject (eager);
                eager = NULL;
              }

            TRACE_ERR((stderr, "<< Eager::generate(), eager = %p, result = %d, dispatch = %zu\n", eager, result, dispatch));
            return eager;
          }

          ///
          /// \brief Eager send protocol constructor.
          ///
          /// \note The {task, offset] pair may be replaced by a single origin
          ///       endpoint parameter if doing so would result in fewer
          ///       conversions.
          ///
          /// \param[in]  dispatch    Dispatch identifier
          /// \param[in]  dispatch_fn Point-to-point dispatch callback function
          /// \param[in]  cookie      Opaque application dispatch data
          /// \param[in]  device      Device that implements the message interface
          /// \param[in]  origin      Origin endpoint
          /// \param[out] status      Constructor status
          ///
          template <class T_Device>
          inline Eager (size_t                 dispatch,
                        pami_dispatch_p2p_function dispatch_fn,
                        void                 * cookie,
                        T_Device             & device,
                        pami_endpoint_t        origin,
                        pami_context_t         context,
                        pami_dispatch_hint_t   hint,
                        pami_result_t        & status) :
              PAMI::Protocol::Send::Send (),
              EagerSimple < T_Model,
              T_Option > (dispatch,
                                            dispatch_fn,
                                            cookie,
                                            device,
                                            origin,
                                            context,
                                            hint,
                                            status)
          {
          };

          virtual ~Eager () {};

          /// \note This is required to make "C" programs link successfully with virtual destructors
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
                      configuration[i].value.intval = Eager::recv_immediate_max;
                      break;
                    case PAMI_DISPATCH_SEND_IMMEDIATE_MAX:
                      configuration[i].value.intval = T_Model::packet_model_immediate_bytes;
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
            TRACE_ERR((stderr, ">> Eager::immediate()\n"));
            pami_result_t result = this->immediate_impl (parameters);
            TRACE_ERR((stderr, "<< Eager::immediate()\n"));
            return result;
          };

          ///
          /// \brief Start a new simple send operation.
          ///
          /// \see PAMI::Protocol::Send::simple
          ///
          virtual pami_result_t simple (pami_send_t * parameters)
          {
            TRACE_ERR((stderr, ">> Eager::simple()\n"));
            pami_result_t result = this->simple_impl (parameters);
            TRACE_ERR((stderr, "<< Eager::simple()\n"));
            return result;
          };
      };  // PAMI::Protocol::Send::Eager class
    };    // PAMI::Protocol::Send namespace
  };      // PAMI::Protocol namespace
};        // PAMI namespace
#undef TRACE_ERR
#endif // __pami_p2p_protocol_send_eager_eager_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
