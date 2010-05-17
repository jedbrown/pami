/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/msg/MessageQueue.h
 * \brief ???
 */

#ifndef __components_devices_bgq_mu2_msg_MessageQueue_h__
#define __components_devices_bgq_mu2_msg_MessageQueue_h__


#include "util/queue/Queue.h"

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      class MessageQueue : public PAMI::Queue
      {
        public:

          class Element : public PAMI::Queue::Element
          {
            public:

              inline Element () {};
              inline ~Element () {};

              ///
              /// \brief Message element advance method
              ///
              /// \retval true  Message complete
              /// \retval false Message incomplete and must remain on the queue
              ///
              virtual bool advance () = 0;

          };  // class     PAMI::Device::MU::MessageQueue::Element


          inline MessageQueue () {};
          virtual ~MessageQueue () {};

          ///
          /// \brief Post a message to the end of the queue
          ///
          /// \param[in] msg Message to post
          ///
          inline void post (MU::MessageQueue::Element * msg)
          {
            enqueue (msg);
          };

          ///
          /// \brief Message queue advance method
          ///
          /// \retval true  Queue contains active messages
          /// \retval false Queue is empty
          ///
          inline bool advance ()
          {
            MU::MessageQueue::Element * msg = NULL;
            while ((msg = (MU::MessageQueue::Element *) dequeue()) != NULL)
              {
                if (msg->advance())
                  {
                    push (msg);
                    return true;
                  }
              }

            return (size() > 0);
          }

      }; // class     PAMI::Device::MU::MessageQueue
    };   // namespace PAMI::Device::MU
  };     // namespace PAMI::Device
};       // namespace PAMI


#endif // __components_devices_bgq_mu2_msg_MessageQueue_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//

