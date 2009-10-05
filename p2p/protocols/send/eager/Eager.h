/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
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
#ifndef __xmi_p2p_protocol_send_eager_eager_h__
#define __xmi_p2p_protocol_send_eager_eager_h__

#include "p2p/protocols/Send.h"
#include "p2p/protocols/send/eager/EagerImmediate.h"
#include "p2p/protocols/send/eager/EagerSimple.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

namespace XMI
{
  namespace Protocol
  {
    namespace Send
    {

      ///
      /// \brief Eager simple send protocol class for reliable network devices.
      ///
      /// \tparam T_Model   Template packet model class
      /// \tparam T_Device  Template packet device class
      /// \tparam T_Message Template packet message class
      ///
      /// \see XMI::Device::Interface::PacketModel
      /// \see XMI::Device::Interface::PacketDevice
      ///
      template <class T_Model, class T_Device, class T_Message>
      class Eager : public XMI::Protocol::Send::Send,
                    public EagerImmediate<T_Model,T_Device,T_Message>,
                    public EagerSimple<T_Model,T_Device,T_Message>
      {
        public:

          ///
          /// \brief Eager send protocol constructor.
          ///
          /// \param[in]  dispatch     Dispatch identifier
          /// \param[in]  dispatch_fn  Dispatch callback function
          /// \param[in]  cookie       Opaque application dispatch data
          /// \param[in]  device       Device that implements the message interface
          /// \param[in]  origin_task  Origin task identifier
          /// \param[in]  context      Communication context
          /// \param[out] status       Constructor status
          ///
          inline Eager (size_t                     dispatch,
                        xmi_dispatch_callback_fn   dispatch_fn,
                        void                     * cookie,
                        T_Device                 & device,
                        size_t                     origin_task,
                        xmi_context_t              context,
                        size_t                     contextid,
                        xmi_result_t             & status) :
            XMI::Protocol::Send::Send (),
            EagerImmediate<T_Model,T_Device,T_Message> (dispatch,
                                                        dispatch_fn,
                                                        cookie,
                                                        device,
                                                        origin_task,
                                                        context,
                                                        contextid,
                                                        status),
            EagerSimple<T_Model,T_Device,T_Message> (dispatch,
                                                     dispatch_fn,
                                                     cookie,
                                                     device,
                                                     origin_task,
                                                     context,
                                                     contextid,
                                                     status)
            {
            };

            virtual ~Eager () {};

            ///
            /// \brief Start a new immediate send operation.
            ///
            /// \see XMI::Protocol::Send::immediate
            ///
            virtual xmi_result_t immediate (xmi_task_t   peer,
                                            void       * src,
                                            size_t       bytes,
                                            void       * msginfo,
                                            size_t       mbytes)
            {
              TRACE_ERR((stderr, ">> Eager::immediate()\n"));
              xmi_result_t result = this->immediate_impl (peer, src, bytes, msginfo, mbytes);
              TRACE_ERR((stderr, "<< Eager::immediate()\n"));
              return result;
            };

            ///
            /// \brief Start a new simple send operation.
            ///
            /// \see XMI::Protocol::Send::simple
            ///
            virtual xmi_result_t simple (xmi_event_function   local_fn,
                                         xmi_event_function   remote_fn,
                                         void               * cookie,
                                         xmi_task_t           peer,
                                         void               * src,
                                         size_t               bytes,
                                         void               * msginfo,
                                         size_t               mbytes)
            {
              TRACE_ERR((stderr, ">> Eager::simple()\n"));
              xmi_result_t result = this->simple_impl (local_fn, remote_fn, cookie, peer, src, bytes, msginfo, mbytes);
              TRACE_ERR((stderr, "<< Eager::simple()\n"));
              return result;
            };

        private:

          static inline void compile_time_assert ()
          {
            // This protocol only works with reliable networks.
            COMPILE_TIME_ASSERT(T_Device::reliable_network == true);

            // This protcol only works with deterministic models.
            COMPILE_TIME_ASSERT(T_Model::deterministic == true);
          };
      };  // XMI::Protocol::Send::Eager class
    };    // XMI::Protocol::Send namespace
  };      // XMI::Protocol namespace
};        // XMI namespace
#undef TRACE_ERR
#endif // __xmi_p2p_protocol_send_eager_eager_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
