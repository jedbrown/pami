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
#define TRACE_ERR(x) fprintf x
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
      /// \tparam T_Device  Template packet device class
      ///
      /// \see PAMI::Device::Interface::PacketModel
      /// \see PAMI::Device::Interface::PacketDevice
      ///
      template < class T_Model, class T_Device, bool T_LongHeader = true, class T_Connection = ConnectionArray<T_Device> >
      class Eager : public PAMI::Protocol::Send::Send,
          public EagerImmediate<T_Model, T_Device>,
          public EagerSimple<T_Model, T_Device, T_LongHeader, T_Connection>
      {
        public:

          template <class T_Allocator>
          static Eager * generate (size_t                     dispatch,
                                   pami_dispatch_callback_fn   dispatch_fn,
                                   void                     * cookie,
                                   pami_endpoint_t             origin,
                                   T_Device                 & device,
                                   pami_context_t             context,
                                   T_Allocator              & allocator,
                                   pami_result_t             & result)
          {
            TRACE_ERR((stderr, ">> Eager::generate()\n"));
            COMPILE_TIME_ASSERT(sizeof(Eager) <= T_Allocator::objsize);

            Eager * eager = (Eager *) allocator.allocateObject ();
            new ((void *)eager) Eager (dispatch, dispatch_fn.p2p, cookie, origin, device, context, result);
            if (result != PAMI_SUCCESS)
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
                        pami_dispatch_p2p_fn   dispatch_fn,
                        void                     * cookie,
                        pami_endpoint_t             origin,
                        T_Device                 & device,
                        pami_context_t             context,
                        pami_result_t             & status) :
              PAMI::Protocol::Send::Send (),
              EagerImmediate<T_Model, T_Device> (dispatch,
                                                 dispatch_fn,
                                                 cookie,
                                                 device,
                                                 context,
                                                 status),
              EagerSimple<T_Model, T_Device,
                          T_LongHeader, T_Connection> (dispatch,
                                                       dispatch_fn,
                                                       cookie,
                                                       origin,
                                                       device,
                                                       context,
                                                       status)
          {
          };

          virtual ~Eager () {};

          /// \note This is required to make "C" programs link successfully with virtual destructors
          inline void operator delete(void * p)
          {
            PAMI_abortf("%s<%d>\n",__FILE__,__LINE__);
          }

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
