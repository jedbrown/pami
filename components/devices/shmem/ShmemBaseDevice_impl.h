/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2008                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/ShmemBaseDevice_impl.h
 * \brief ???
 */

#include "SysDep.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

namespace XMI
{
  namespace Device
  {
    template <class T_Fifo, class T_Packet>
    int ShmemBaseDevice<T_Fifo, T_Packet>::init_internal (XMI::SysDep * sysdep)
    {
      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::init_internal ()\n", __global.mapping.task()));
      _sysdep = sysdep;
      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::init_internal () _sysdep = %p\n", __global.mapping.task(), _sysdep));

      unsigned i, j;
      __global.mapping.nodePeers (_num_procs);
      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::init_internal () .. 0 _num_procs = %zd\n", __global.mapping.task(), _num_procs));

      //_global_task = __global.mapping.task ();


      //size_t global;
      XMI::Interface::Mapping::nodeaddr_t nodeaddr;
      __global.mapping.nodeAddr (nodeaddr);
      _global_task = nodeaddr.global;
      _local_task  = nodeaddr.local;

      //__global.mapping.task2node (_global_task, &nodeaddr);

      //_local_task = _peercache[nodeaddr.local];

      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::init_internal () .. 1\n", __global.mapping.task()));

      // Allocate a shared memory segment for the fifos
      size_t size = ((sizeof(T_Fifo) + 15) & 0xfff0) * _num_procs;
      _sysdep->mm.memalign ((void **)&_fifo, 16, size);
      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::init_internal () .. 2\n", __global.mapping.task()));

      // Initialize the fifo acting as the reception fifo for this local task
      _rfifo = &_fifo[_local_task];
      new (_rfifo) T_Fifo ();
      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::init_internal () .. 3, _local_task = %zd, _fifo = %p, _rfifo = %p\n", __global.mapping.task(), _local_task, _fifo, _rfifo));
      _rfifo->init (*sysdep);
      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::init_internal () .. 4\n", __global.mapping.task()));

      // barrier ?

      // Allocate memory for and construct the queue objects,
      // one for each local rank.
      __sendQ = (Queue *) malloc ((sizeof (Queue) * _num_procs));
      __doneQ = (Queue *) malloc ((sizeof (Queue) * _num_procs));

      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::init_internal () .. 5\n", __global.mapping.task()));
      for (i = 0; i < _num_procs; i++)
      {
        new (&__sendQ[i]) Queue ();
        new (&__doneQ[i]) Queue ();
      }
      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::init_internal () .. 6\n", __global.mapping.task()));

      // Initialize the send and done queue masks to zero (empty).
      // There should be a queue mask associated with each channel.
      __sendQMask = 0;
      __doneQMask = 0;

      // Initialize the registered receive function array to noop().
      // The array is limited to 256 dispatch ids because of the size of the
      // dispatch id field in the packet header.
      for (i=0; i<256; i++)
      {
//        _dispatch_count[i] = 0;
        for (j=0; j<256; j++)
        {
          _dispatch[i][j].function   = noop;
          _dispatch[i][j].clientdata = NULL;
        }
      }
      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::init_internal () .. 7\n", __global.mapping.task()));

      return 0;
    }

    template <class T_Fifo, class T_Packet>
    bool ShmemBaseDevice<T_Fifo, T_Packet>::isInit_impl ()
    {
      return true;
    }

    /// \see XMI::Device::Interface::BaseDevice::peers()
    template <class T_Fifo, class T_Packet>
    size_t ShmemBaseDevice<T_Fifo, T_Packet>::peers_impl ()
    {
      return _num_procs;
    }

    /// \see XMI::Device::Interface::BaseDevice::task2peer()
    template <class T_Fifo, class T_Packet>
    size_t ShmemBaseDevice<T_Fifo, T_Packet>::task2peer_impl (size_t task)
    {
      XMI::Interface::Mapping::nodeaddr_t address;
      __global.mapping.task2node (task, address);

      size_t peer = 0;
      __global.mapping.node2peer (address, peer);

      return peer;
    }

    ///
    /// \brief Regieter the receive function to dispatch when a packet arrives.
    ///
    /// \param[in] id              Dispatch set identifier
    /// \param[in] recv_func       Receive function to dispatch
    /// \param[in] recv_func_parm  Receive function client data
    ///
    /// \return Dispatch id for this registration
    ///
    template <class T_Fifo, class T_Packet>
    int ShmemBaseDevice<T_Fifo, T_Packet>::registerRecvFunction (size_t                      id,
                                                                 Interface::RecvFunction_t   recv_func,
                                                                 void                      * recv_func_parm)
    {
      TRACE_ERR((stderr, ">> (%zd) ShmemBaseDevice::registerRecvFunction (%d,%p,%p)\n", __global.mapping.task(), id, recv_func, recv_func_parm));

      // This device only supports up to 256 dispatch sets.
      if (id >= 256) return -1;

      // Find the next available slot for this dispatch set.
      size_t slot;
      for (slot=0; slot<256; slot++)
      {
        if (_dispatch[id][slot].function == (Interface::RecvFunction_t) noop) break;
      }

      if (slot == 256) return -1;

      _dispatch[id][slot].function  = recv_func;
      _dispatch[id][slot].clientdata = recv_func_parm;

      size_t f = id*256+slot;

      TRACE_ERR((stderr, "<< (%zd) ShmemBaseDevice::registerRecvFunction (%d,%p,%p) => %d => %d\n", __global.mapping.task(), id, recv_func, recv_func_parm, slot, f));
      return f;
    };

    template <class T_Fifo, class T_Packet>
    xmi_result_t ShmemBaseDevice<T_Fifo, T_Packet>::post (size_t fnum, ShmemBaseMessage<T_Packet> * msg)
    {
      pushSendQueueTail (fnum, (QueueElem *) msg);
      return XMI_SUCCESS;
    };



    template <class T_Fifo, class T_Packet>
    int ShmemBaseDevice<T_Fifo, T_Packet>::noop (void   * metadata,
                                                 void   * payload,
                                                 size_t   bytes,
                                                 void   * recv_func_parm,
                                                 void   * cookie)
    {
      XMI_abort();
      return 0;
    }


    template <class T_Fifo, class T_Packet>
    int ShmemBaseDevice<T_Fifo, T_Packet>::advance_sendQ ()
    {
      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::advance_sendQ () >> _num_procs = %zd\n", __global.mapping.task(), _num_procs));
      unsigned peer;

      int events = 0;
      for (peer = 0; peer < _num_procs; peer++)
        events += advance_sendQ (peer);
      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::advance_sendQ () <<\n", __global.mapping.task()));
      return events;
    }

    template <class T_Fifo, class T_Packet>
    int ShmemBaseDevice<T_Fifo, T_Packet>::advance_sendQ (size_t peer)
    {
      ShmemBaseMessage<T_Packet> * msg;
      size_t sequence;
      int events = 0;

      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::advance_sendQ (%zd) >>\n", __global.mapping.task(), peer));
      while (!__sendQ[peer].isEmpty())
        {
          // There is a pending message on the send queue.
          msg = (ShmemBaseMessage<T_Packet> *) __sendQ[peer].peekHead ();

		  if (msg->isRMAType() == true){
				  sequence = _fifo[peer].nextInjSequenceId();
				  xmi_result_t res = processRMAMessage(peer, msg, sequence);
				  if (res == XMI_SUCCESS)
				  {	
				  	popSendQueueHead (peer);
                    msg->executeCallback ();
				    continue;
				  }
				  else
				  {		
					break; //dont process any further elements until the RMA message is processed
				  }	
		  }

          TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::advance_sendQ (%zd) .. before writeSinglePacket()\n", __global.mapping.task(), peer));
          xmi_result_t result = writeSinglePacket (peer, msg, sequence);
          TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::advance_sendQ (%zd) ..  after writeSinglePacket(), result = %zd\n", __global.mapping.task(), peer, result));

          if (result == XMI_SUCCESS)
            {
              if (msg->done())
                {
                  events++;

                  // The message has completed processing the source data.
                  // Remove from the send queue and advance next message
                  // before invoking the message's done callback.
                  popSendQueueHead (peer);

                  if (msg->isRemoteCompletionRequired ())
                  {
                    // Do not invoke the completion callback until the packets
                    // have been processed by the remote rank.
                    size_t last_rec_seq_id = _fifo[peer].lastRecSequenceId ();
                    if (sequence <= last_rec_seq_id)
                    {
                      // The last packet in this message has been processed by
                      // the remote node. Invoke the send completion callback
                      // here.. may post another message!
                      msg->executeCallback ();
                    }
                    else
                    {
                      // The remote node has not yet processed the last packet
                      // of this message. Add the message to the done queue
                      // and check for completion again later.
                      msg->setSequenceId (sequence);
                      pushDoneQueueTail (peer, (QueueElem *) msg);
                    }
                  }
                  else
                  {
                    // Invoke the send completion callback here.. may post
                    // another message!
                    msg->executeCallback ();
                    break;
                  }
                }
            }
          else
            {
              // Unable to write a packet .. ififo is full.
              // Leave this message on the queue and break out of the loop.
              break;
            }
        }
        return events;
    };

    template <class T_Fifo, class T_Packet>
    int ShmemBaseDevice<T_Fifo, T_Packet>::advance_doneQ ()
    {
      size_t peer;
      int events = 0;

      for (peer = 0; peer < _num_procs; peer++)
        events += advance_doneQ (peer);

      return events;
    };

    template <class T_Fifo, class T_Packet>
    int ShmemBaseDevice<T_Fifo, T_Packet>::advance_doneQ (size_t peer)
    {
      int events = 0;
      size_t last_rec_seq_id = _fifo[peer].lastRecSequenceId ();
      ShmemBaseMessage<T_Packet> * msg;

      while (!__doneQ[peer].isEmpty())
        {
          // There is a pending message on the done queue.
          msg = (ShmemBaseMessage<T_Packet> *) __doneQ[peer].peekHead ();

          if (msg->getSequenceId() <= last_rec_seq_id)
          {
            // The remote rank has completed processing the packets
            // associated with this message. Remove from the done queue
            // and invoke the message's done callback.
            events++;
            popDoneQueueHead (peer);
            msg->executeCallback ();
          }
          else
          {
            return events;
          }
        }
      return events;
    };
  };
};
#undef TRACE_ERR

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
