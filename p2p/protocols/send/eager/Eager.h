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
#include "p2p/protocols/send/eager/EagerImmediate.h"
#include "p2p/protocols/send/eager/EagerSimple.h"
#include "p2p/protocols/send/eager/ConnectionArray.h"

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
      ///
      /// \see XMI::Device::Interface::PacketModel
      /// \see XMI::Device::Interface::PacketDevice
      ///
      template < class T_Model, class T_Device, bool T_LongHeader = true, class T_Connection = ConnectionArray<T_Device> >
      class Eager : public XMI::Protocol::Send::Send,
          public EagerImmediate<T_Model, T_Device>,
          public EagerSimple<T_Model, T_Device, T_LongHeader, T_Connection>
      {
        public:

          template <class T_Allocator>
          static Eager * generate (size_t                     dispatch,
                                   xmi_dispatch_callback_fn   dispatch_fn,
                                   void                     * cookie,
                                   T_Device                 & device,
                                   T_Allocator              & allocator,
                                   xmi_result_t             & result)
          {
            TRACE_ERR((stderr, ">> Eager::generate()\n"));
            COMPILE_TIME_ASSERT(sizeof(Eager) <= T_Allocator::objsize);

            Eager * eager = (Eager *) allocator.allocateObject ();
            new ((void *)eager) Eager (dispatch, dispatch_fn, cookie, device, result);
            if (result != XMI_SUCCESS)
            {
              allocator.returnObject (eager);
              eager = NULL;
            }

            TRACE_ERR((stderr, "<< Eager::generate(), eager = %p, result = %d\n", eager, result));
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
          /// \param[in]  dispatch_fn Dispatch callback function
          /// \param[in]  cookie      Opaque application dispatch data
          /// \param[in]  device      Device that implements the message interface
          /// \param[in]  context     Communication context
          /// \param[in]  task        Origin task identifier
          /// \param[in]  offset      Origin context offset identifier
          /// \param[out] status      Constructor status
          ///
          inline Eager (size_t                     dispatch,
                        xmi_dispatch_callback_fn   dispatch_fn,
                        void                     * cookie,
                        T_Device                 & device,
                        xmi_result_t             & status) :
              XMI::Protocol::Send::Send (),
              EagerImmediate<T_Model, T_Device> (dispatch,
                                                 dispatch_fn,
                                                 cookie,
                                                 device,
                                                 status),
              EagerSimple<T_Model, T_Device,
                          T_LongHeader, T_Connection> (dispatch,
                                                       dispatch_fn,
                                                       cookie,
                                                       device,
                                                       status)
          {
          };

          virtual ~Eager () {};

          /// \note This is required to make "C" programs link successfully with virtual destructors
          inline void operator delete(void * p)
          {
            XMI_abortf("%s<%d>\n",__FILE__,__LINE__);
          }

          ///
          /// \brief Start a new immediate send operation.
          ///
          /// \see XMI::Protocol::Send::immediate
          ///
          virtual xmi_result_t immediate (xmi_send_immediate_t * parameters)
          {
            TRACE_ERR((stderr, ">> Eager::immediate()\n"));
            xmi_result_t result = this->immediate_impl (parameters);
            TRACE_ERR((stderr, "<< Eager::immediate()\n"));
            return result;
          };

          ///
          /// \brief Start a new simple send operation.
          ///
          /// \see XMI::Protocol::Send::simple
          ///
          virtual xmi_result_t simple (xmi_send_t * parameters)
          {
            TRACE_ERR((stderr, ">> Eager::simple()\n"));
            xmi_result_t result = this->simple_impl (parameters);
            TRACE_ERR((stderr, "<< Eager::simple()\n"));
            return result;
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
