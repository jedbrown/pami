/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file p2p/protocols/send/datagram/Datagram.h
/// \brief Datagram send protocol for reliable devices.
///
/// The Datagram class defined in this file uses C++ templates and
/// the device "message" interface - which also uses C++ templates.
///
/// C++ templates require all source code to be #include'd from a header file.
///
#ifndef __p2p_protocols_send_datagram_h__
#define __p2p_protocols_send_datagram_h__

#include "p2p/protocols/Send.h"
#include "p2p/protocols/send/datagram/DatagramImmediate.h"
#include "p2p/protocols/send/datagram/DatagramSimple.h"

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
      /// \brief Datagram simple send protocol class for reliable network devices.
      ///
      /// \tparam T_Model   Template packet model class
      /// \tparam T_Device  Template packet device class
      ///
      /// \see XMI::Device::Interface::PacketModel
      /// \see XMI::Device::Interface::PacketDevice
      ///
      template < class T_Model, class T_Device, bool T_LongHeader = true >
      class Datagram : public XMI::Protocol::Send::Send,
          public DatagramImmediate<T_Model, T_Device>,
          public DatagramSimple<T_Model, T_Device, T_LongHeader>
      {
        public:

          ///
          /// \brief Datagram send protocol constructor.
          ///
          /// \param[in]  dispatch     Dispatch identifier
          /// \param[in]  dispatch_fn  Dispatch callback function
          /// \param[in]  cookie       Opaque application dispatch data
          /// \param[in]  device       Device that implements the message interface
          /// \param[in]  origin_task  Origin task identifier
          /// \param[in]  context      Communication context
          /// \param[out] status       Constructor status
          ///
          inline Datagram (size_t                     dispatch,
                        xmi_dispatch_callback_fn   dispatch_fn,
                        void                     * cookie,
                        T_Device                 & device,
                        size_t                     origin_task,
                        xmi_context_t              context,
                        size_t                     contextid,
                        xmi_result_t             & status) :
              XMI::Protocol::Send::Send (),
              DatagramImmediate<T_Model, T_Device> (dispatch,
                                                 dispatch_fn,
                                                 cookie,
                                                 device,
                                                 origin_task,
                                                 context,
                                                 contextid,
                                                 status),
              DatagramSimple<T_Model, T_Device, T_LongHeader> (dispatch,
                                                            dispatch_fn,
                                                            cookie,
                                                            device,
                                                            origin_task,
                                                            context,
                                                            contextid,
                                                            status)
          {
          };

          virtual ~Datagram () {};

          ///
          /// \brief Start a new immediate send operation.
          ///
          /// \see XMI::Protocol::Send::immediate
          ///
		  
		  virtual xmi_result_t immediate (xmi_send_immediate_t * parameters)
          {
            TRACE_ERR((stderr, ">> Datagram::immediate()\n"));
            xmi_result_t result = this->immediate_impl (parameters);
            TRACE_ERR((stderr, "<< Datagram::immediate()\n"));
            return result;
          };
		  
		  
		  
                    ///
          /// \brief Start a new simple send operation.
          ///
          /// \see XMI::Protocol::Send::simple
          ///
		  virtual xmi_result_t simple (xmi_send_t * parameters)
          {
            TRACE_ERR((stderr, ">> Datagram::simple()\n"));
            xmi_result_t result = this->simple_impl (parameters);
            TRACE_ERR((stderr, "<< Datagram::simple()\n"));
            return result;
          };
		  
          

      };  // XMI::Protocol::Send::Datagram class
    };    // XMI::Protocol::Send namespace
  };      // XMI::Protocol namespace
};        // XMI namespace
#undef TRACE_ERR
#endif // __xmi_p2p_protocol_send_datagram_datagram_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
