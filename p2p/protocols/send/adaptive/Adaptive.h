/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file p2p/protocols/send/adaptive/Adaptive.h
/// \brief Adaptive send protocol for reliable devices.
///
/// The Adaptive class defined in this file uses C++ templates and
/// the device "message" interface - which also uses C++ templates.
///
/// C++ templates require all source code to be #include'd from a header file.
///
#ifndef __p2p_protocols_send_adaptive_Eager_h__
#define __p2p_protocols_send_adaptive_Eager_h__

#include "p2p/protocols/Send.h"
#include "p2p/protocols/send/adaptive/AdaptiveImmediate.h"
#include "p2p/protocols/send/adaptive/AdaptiveSimple.h"

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
      /// \brief Adaptive simple send protocol class for reliable network devices.
      ///
      /// \tparam T_Model   Template packet model class
      /// \tparam T_Device  Template packet device class
      ///
      /// \see XMI::Device::Interface::PacketModel
      /// \see XMI::Device::Interface::PacketDevice
      ///
      template < class T_Model, class T_Device, bool T_LongHeader = true >
      class Adaptive : public XMI::Protocol::Send::Send,
          public AdaptiveImmediate<T_Model, T_Device>,
          public AdaptiveSimple<T_Model, T_Device, T_LongHeader>
      {
        public:

          ///
          /// \brief Adaptive send protocol constructor.
          ///
          /// \param[in]  dispatch     Dispatch identifier
          /// \param[in]  dispatch_fn  Dispatch callback function
          /// \param[in]  cookie       Opaque application dispatch data
          /// \param[in]  device       Device that implements the message interface
          /// \param[in]  origin_task  Origin task identifier
          /// \param[in]  context      Communication context
          /// \param[out] status       Constructor status
          ///
          inline Adaptive (size_t                     dispatch,
                        xmi_dispatch_callback_fn   dispatch_fn,
                        void                     * cookie,
                        T_Device                 & device,
                        size_t                     origin_task,
                        xmi_context_t              context,
                        size_t                     contextid,
                        xmi_result_t             & status) :
              XMI::Protocol::Send::Send (),
              AdaptiveImmediate<T_Model, T_Device> (dispatch,
                                                 dispatch_fn,
                                                 cookie,
                                                 device,
                                                 origin_task,
                                                 context,
                                                 contextid,
                                                 status),
              AdaptiveSimple<T_Model, T_Device, T_LongHeader> (dispatch,
                                                            dispatch_fn,
                                                            cookie,
                                                            device,
                                                            origin_task,
                                                            context,
                                                            contextid,
                                                            status)
          {
          };

          virtual ~Adaptive () {};

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
            TRACE_ERR((stderr, ">> Adaptive::immediate()\n"));
            xmi_result_t result = this->immediate_impl (peer, src, bytes, msginfo, mbytes);
            TRACE_ERR((stderr, "<< Adaptive::immediate()\n"));
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
            TRACE_ERR((stderr, ">> Adaptive::simple()\n"));
            xmi_result_t result = this->simple_impl (local_fn, remote_fn, cookie, peer, src, bytes, msginfo, mbytes);
            TRACE_ERR((stderr, "<< Adaptive::simple()\n"));
            return result;
          };

      };  // XMI::Protocol::Send::Adaptive class
    };    // XMI::Protocol::Send namespace
  };      // XMI::Protocol namespace
};        // XMI namespace
#undef TRACE_ERR
#endif // __xmi_p2p_protocol_send_adaptive_adaptive_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
