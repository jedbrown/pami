/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu/MUChannel.cc
 * \brief ???
 */

#include "components/devices/bgq/mu/MUChannel.h"

XMI::Device::MU::Channel::Channel() :
    _resMgr(),
    _pollQueues(false)
{
  for (int i = 0; i < MAX_INJ_FIFOS; i++)
    _msgQ [i] = Queue();
}



uint32_t XMI::Device::MU::Channel::pollQueuedMessages ()
{
  // Count the events processed.
  unsigned events = 0;

  _pollQueues = false;

  MUBaseMessage *msg;

  //Allow message to use inj fifos
  for (unsigned count = 0; count < _resMgr.getNumInjFifos(); count ++)
    {
      //
      // Dequeue the message, as opposed to leaving it in the queue, because
      // if the message is advanced below, it could be done and as such we cannot
      // touch it again.  Enqueue it later if the advance() fails.
      //
      msg = (MUBaseMessage *) _msgQ[count].popHead();

      if (!msg)  continue;

      while (msg && msg->advance(&count, 1) )
        {

          events++; // Count that a message was advanced.

          // Move to next message on the message queue.
          msg = (MUBaseMessage *) _msgQ[count].popHead();
        }

      // If there is still a message that was not advanced,
      // put it back on the queue and indicate that the queue
      // is not empty.
      if (msg)
        {
          _msgQ[count].pushHead(msg);
          _pollQueues = true;
        }
    }

  return events;
}

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
