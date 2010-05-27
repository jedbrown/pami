/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/msg/InjectDescriptorWithCompletionMessage.h
 * \brief ???
 */

#ifndef __components_devices_bgq_mu2_msg_InjectDescriptorWithCompletionMessage_h__
#define __components_devices_bgq_mu2_msg_InjectDescriptorWithCompletionMessage_h__

#include "components/devices/bgq/mu2/msg/InjectDescriptorMessage.h"

#include "components/devices/bgq/mu2/trace.h"
#define DO_TRACE_ENTEREXIT 1
#define DO_TRACE_DEBUG 1

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      ///
      /// \brief Inject one or more descriptors into an inject fifo and track completion of the descriptors
      ///
      /// \tparam T_Num Number of descriptors that will be injected
      ///
      template <unsigned T_Num>
      class InjectDescriptorWithCompletionMessage : public InjectDescriptorMessage<T_Num>
      {
        public:

          ///
          /// \brief Inject descriptor(s) into a specific injection fifo
          ///
          /// The callback function will be invoked when the MU hardware has
          /// finished processing the descriptor(s).
          ///
          /// \param[in] injfifo Specific injection fifo for the descriptor(s)
          /// \param[in] fn      Completion callback function
          /// \param[in] cookie  Completion callback cookie
          ///
          inline InjectDescriptorWithCompletionMessage (MUSPI_InjFifo_t     * injfifo,
                                                        pami_event_function   fn,
                                                        void                * cookie) :
              InjectDescriptorMessage<T_Num> (injfifo),
              _fn (fn),
              _cookie (cookie)
          {
            TRACE_FN_ENTER();

            TRACE_FN_EXIT();
          };

          inline ~InjectDescriptorWithCompletionMessage () {};

          ///
          /// \brief Inject descriptor message virtual advance implementation
          ///
          /// The inject descriptor message is complete only after all
          /// descriptors have been injected into the injection fifo. The
          /// completion callback function will be invoked after the last
          /// descriptor injected by this message has been processed by the
          /// MU hardware.
          ///
          /// The number of descriptors to inject is specified as a template
          /// parameter for the class.
          ///
          /// \see MU::MessageQueue::Element::advance
          ///
          /// \retval true  Message complete
          /// \retval false Message incomplete and must remain on the queue
          ///
          virtual bool advance ()
          {
            TRACE_FN_ENTER();

            uint64_t sequence = 0;
            bool success = false;

            do
              {
                sequence = MUSPI_InjFifoInject (_injfifo, (void *) & desc[_next]);
                success = (sequence != ((uint64_t) - 1));

              }
            while (success && (_next++ != (T_Num - 1)));

            if (likely(success == true))
              {
                // Do completion processing. 'sequence' contains the sequence id
                // of the last descriptor that was injected.
                PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
              }

            TRACE_FORMAT("success = %zu, _next = %zu", success, next);
            TRACE_FN_EXIT();
            return success;
          }

          ///
          /// \brief Set the completion callback for the message
          ///
          /// \param[in] fn      Completion callback function
          /// \param[in] cookie  Completion callback cookie
          ///
          inline void setCompletionCallback (pami_event_function   fn,
                                             void                * cookie)
          {
            _fn = fn;
            _cookie = cookie;
          }

        protected:

          pami_event_function   _fn;
          void                * _cookie;

      }; // class     PAMI::Device::MU::InjectDescriptorWithCompletionMessage

      ///
      /// \brief Single descriptor advance template specialization
      ///
      template <>
      bool InjectDescriptorWithCompletionMessage<1>::advance ()
      {
        TRACE_FN_ENTER();

        uint64_t sequence =  MUSPI_InjFifoInject (_injfifo, (void *) & desc[0]);
        bool success = (sequence != ((uint64_t) - 1));

        if (likely(success == true))
          {
            // Do completion processing. 'sequence' contains the sequence id
            // of the last descriptor that was injected.
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
          }

        TRACE_FORMAT("success = %zu", success);
        TRACE_FN_EXIT();
        return success;
      };
    };   // namespace PAMI::Device::MU
  };     // namespace PAMI::Device
};       // namespace PAMI

#undef  DO_TRACE_ENTEREXIT
#undef  DO_TRACE_DEBUG

#endif // __components_devices_bgq_mu2_msg_InjectDescriptorWithCompletionMessage_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//

