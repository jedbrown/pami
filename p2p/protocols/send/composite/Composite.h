/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file p2p/protocols/send/composite/Composite.h
/// \brief Composite send protocol composed of multiple individual send protocols
///
/// The Composite class defined in this file uses C++ templates and
/// the device "message" interface - which also uses C++ templates.
///
/// C++ templates require all source code to be #include'd from a header file.
///
#ifndef __p2p_protocols_send_composite_Composite_h__
#define __p2p_protocols_send_composite_Composite_h__

#include "p2p/protocols/Send.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) // fprintf x
#endif

namespace XMI
{
  namespace Protocol
  {
    namespace Send
    {
      // Forward declaration
      template <class T_Primary, class T_Secondary> class Composite;

      namespace Factory
      {
        template <class T_Primary, class T_Secondary, class T_Allocator>
        static Composite<T_Primary, T_Secondary> * generate (T_Primary    * primary,
                                                             T_Secondary  * secondary,
                                                             T_Allocator  & allocator,
                                                             xmi_result_t & result)
        {
          TRACE_ERR((stderr, ">> Send::Factory::generate() [Composite]\n"));
          COMPILE_TIME_ASSERT(sizeof(Composite<T_Primary, T_Secondary>) <= T_Allocator::objsize);

          void * composite = allocator.allocateObject ();
          new (composite) Composite<T_Primary, T_Secondary> (primary, secondary, result);

          if (result != XMI_SUCCESS)
            {
              allocator.returnObject (composite);
              composite = NULL;
            }

          TRACE_ERR((stderr, "<< Send::Factory::generate() [Composite], composite = %p\n", composite));
          return (Composite<T_Primary, T_Secondary> *) composite;
        };
      };  // XMI::Protocol::Send::Factory namespace

      ///
      /// \brief Composite send protocol class
      ///
      /// The Composite send protocol internally invokes the send operations on
      /// the primary protocol and, if the primary protocol send operation does
      /// not succeed the secondary protocol is used for the send operations.
      ///
      /// \tparam T_Primary    Template send protocol class
      /// \tparam T_Secondary  Template send protocol class
      ///
      /// \see XMI::Protocol::Send::Send
      ///
      template <class T_Primary, class T_Secondary>
      class Composite : public XMI::Protocol::Send::Send
      {
        public:
          ///
          /// \brief Composite send protocol constructor.
          ///
          /// \param[in]  primary     Primary send protocol
          /// \param[in]  secondary   Secondary send protocol
          /// \param[out] status      Constructor status
          ///
          inline Composite (T_Primary    * primary,
                            T_Secondary  * secondary,
                            xmi_result_t & status) :
              XMI::Protocol::Send::Send (),
              _primary (primary),
              _secondary (secondary)
          {
            status = XMI_SUCCESS;
          };

          virtual ~Composite () {};

          /// \note This is required to make "C" programs link successfully
          ///       with virtual destructors
          inline void operator delete(void * p)
          {
            XMI_abortf("%s<%d>\n", __FILE__, __LINE__);
          }

          ///
          /// \brief Start a new immediate send operation.
          ///
          /// \see XMI::Protocol::Send::immediate
          ///
          virtual xmi_result_t immediate (xmi_send_immediate_t * parameters)
          {
            TRACE_ERR((stderr, ">> Composite::immediate()\n"));
            xmi_result_t result = _primary->immediate (parameters);
            TRACE_ERR((stderr, "   Composite::immediate(), primary result = %d\n", result));

            if (result != XMI_SUCCESS)
              {
                result = _secondary->immediate (parameters);
                TRACE_ERR((stderr, "   Composite::immediate(), secondary result = %d\n", result));
              }

            TRACE_ERR((stderr, "<< Composite::immediate()\n"));
            return result;
          };

          ///
          /// \brief Start a new simple send operation.
          ///
          /// \see XMI::Protocol::Send::simple
          ///
          virtual xmi_result_t simple (xmi_send_t * parameters)
          {
            TRACE_ERR((stderr, ">> Composite::simple()\n"));
            xmi_result_t result = _primary->simple (parameters);
            TRACE_ERR((stderr, "   Composite::simple(), primary result = %d\n", result));

            if (result != XMI_SUCCESS)
              {
                result = _secondary->simple (parameters);
                TRACE_ERR((stderr, "   Composite::simple(), secondary result = %d\n", result));
              }

            TRACE_ERR((stderr, "<< Composite::simple()\n"));
            return result;
          };

        protected:

          T_Primary   * _primary;
          T_Secondary * _secondary;

      };  // XMI::Protocol::Send::Composite class
    };    // XMI::Protocol::Send namespace
  };      // XMI::Protocol namespace
};        // XMI namespace
#undef TRACE_ERR
#endif // __p2p_protocols_send_composite_Composite_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
