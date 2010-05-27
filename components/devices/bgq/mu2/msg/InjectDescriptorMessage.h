/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/msg/InjectDescriptorMessage.h
 * \brief ???
 */

#ifndef __components_devices_bgq_mu2_msg_InjectDescriptorMessage_h__
#define __components_devices_bgq_mu2_msg_InjectDescriptorMessage_h__

#include "components/devices/bgq/mu2/msg/MessageQueue.h"

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
      /// \brief Inject one or more descriptors into an inject fifo
      ///
      /// \tparam T_Num Number of descriptors that will be injected
      ///
      template <unsigned T_Num>
      class InjectDescriptorMessage : public MessageQueue::Element
      {
        public:

          ///
          /// \brief Inject descriptor(s) into a specific injection fifo
          ///
          /// \param[in] injfifo Specific injection fifo for the descriptor(s)
          ///
          inline InjectDescriptorMessage (MUSPI_InjFifo_t * injfifo) :
              _injfifo (injfifo),
              _next (0)
          {
            TRACE_FN_ENTER();

            TRACE_FN_EXIT();
          };

          inline ~InjectDescriptorMessage () {};

          ///
          /// \brief Inject descriptor message virtual advance implementation
          ///
          /// The inject descriptor message is complete only after all
          /// descriptors have been injected into the injection fifo.
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

            //TRACE_FORMAT("success = %d, _next = %zu", success, next);
            TRACE_FN_EXIT();
            return success;
          }

          ///
          /// \brief Set the injection fifo for the message
          ///
          /// \param[in] injfifo Injection Fifo
          ///
          inline void setInjectionFifo (MUSPI_InjFifo_t * injfifo)
          {
            _injfifo = injfifo;
          }

          ///
          /// \brief Reset the internal state of the message
          ///
          /// \note Only used for message reuse.
          ///
          inline void reset () { _next = 0; };

          MUSPI_DescriptorBase   desc[T_Num];

        protected:

          MUSPI_InjFifo_t      * _injfifo;
          size_t                 _next;

      }; // class     PAMI::Device::MU::InjectDescriptorMessage

      ///
      /// \brief Single descriptor advance template specialization
      ///
      template <>
      bool InjectDescriptorMessage<1>::advance ()
      {
        TRACE_FN_ENTER();

        uint64_t sequence =  MUSPI_InjFifoInject (_injfifo, (void *) & desc[0]);

        TRACE_FORMAT("success = %d", (sequence != ((uint64_t) - 1)));
        TRACE_FN_EXIT();
        return (sequence != ((uint64_t) - 1));
      };
    };   // namespace PAMI::Device::MU
  };     // namespace PAMI::Device
};       // namespace PAMI

#undef  DO_TRACE_ENTEREXIT
#undef  DO_TRACE_DEBUG

#endif // __components_devices_bgq_mu2_msg_InjectDescriptorMessage_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//

