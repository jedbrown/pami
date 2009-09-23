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

#include "components/mapping/NodeMapping.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x)  //fprintf x
#endif

namespace XMI
{
  namespace Device
  {
    template <class T_SysDep, class T_Fifo, class T_Packet>
    int ShmemBaseDevice<T_SysDep, T_Fifo, T_Packet>::init_internal (T_SysDep * sysdep)
    {
      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::init_internal ()\n", sysdep->mapping.task()));
      _sysdep = sysdep;
      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::init_internal () _sysdep = %p\n", sysdep->mapping.task(), _sysdep));

      unsigned i, j;
      _sysdep->mapping.nodePeers (_num_procs);
      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::init_internal () .. 0\n", sysdep->mapping.task()));

      //_global_task = _sysdep->mapping.task ();
      
      
      //size_t global;
      XMI::Mapping::Interface::nodeaddr_t nodeaddr;
      _sysdep->mapping.nodeAddr (nodeaddr);
      _global_task = nodeaddr.global;
      _local_task  = nodeaddr.local;
      
      //_sysdep->mapping.task2node (_global_task, &nodeaddr);
      
      //_local_task = _peercache[nodeaddr.local];
      
      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::init_internal () .. 1\n", sysdep->mapping.task()));

      // Allocate a shared memory segment for the fifos
      size_t size = ((sizeof(T_Fifo) + 15) & 0xfff0) * _num_procs;
      sysdep->mm.memalign ((void **)&_fifo, 16, size);
      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::init_internal () .. 2\n", sysdep->mapping.task()));

      // Initialize the fifo acting as the reception fifo for this local task
      _rfifo = &_fifo[_local_task];
      new (_rfifo) T_Fifo ();
      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::init_internal () .. 3, _local_task = %zd, _fifo = %p, _rfifo = %p\n", sysdep->mapping.task(), _local_task, _fifo, _rfifo));
      _rfifo->init ();
      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::init_internal () .. 4\n", sysdep->mapping.task()));

      // barrier ?

      // Allocate memory for and construct the queue objects,
      // one for each local rank.
      __sendQ = (Queue *) malloc ((sizeof (Queue) * _num_procs));
      __doneQ = (Queue *) malloc ((sizeof (Queue) * _num_procs));

      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::init_internal () .. 5\n", sysdep->mapping.task()));
      for (i = 0; i < _num_procs; i++)
      {
        new (&__sendQ[i]) Queue ();
        new (&__doneQ[i]) Queue ();
      }
      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::init_internal () .. 6\n", sysdep->mapping.task()));

      // Initialize the send and done queue masks to zero (empty).
      // There should be a queue mask associated with each channel.
      __sendQMask = 0;
      __doneQMask = 0;

      // Initialize the registered receive function array to noop().
      // The array is limited to 256 dispatch ids because of the size of the
      // dispatch id field in the packet header.
      _dispatch_count = 0;

      for (i = 0; i < 256; i++)
        {
          _dispatch[i].function   = noop;
          _dispatch[i].clientdata = NULL;
        }
      TRACE_ERR((stderr, "(%zd) ShmemBaseDevice::init_internal () .. 7\n", sysdep->mapping.task()));

      return 0;
    }

    template <class T_SysDep, class T_Fifo, class T_Packet>
    bool ShmemBaseDevice<T_SysDep, T_Fifo, T_Packet>::isInit_impl ()
    {
      return true;
    }

    /// \see XMI::Device::Interface::BaseDevice::peers()
    template <class T_SysDep, class T_Fifo, class T_Packet>
    size_t ShmemBaseDevice<T_SysDep, T_Fifo, T_Packet>::peers_impl ()
    {
      return _num_procs;
    }

    /// \see XMI::Device::Interface::BaseDevice::task2peer()
    template <class T_SysDep, class T_Fifo, class T_Packet>
    size_t ShmemBaseDevice<T_SysDep, T_Fifo, T_Packet>::task2peer_impl (size_t task)
    {
      XMI::Mapping::Interface::nodeaddr_t address;
      _sysdep->mapping.task2node (task, address);

      size_t peer = 0;
      _sysdep->mapping.node2peer (address, peer);

      return peer;
    }

    /// \see XMI::Device::Interface::PacketDevice::requiresRead()
    template <class T_SysDep, class T_Fifo, class T_Packet>
    bool ShmemBaseDevice<T_SysDep, T_Fifo, T_Packet>::requiresRead_impl ()
    {
      return false;
    };

    ///
    /// \brief Regieter the receive function to dispatch when a packet arrives.
    ///
    /// \param[in] recv_func       Receive function to dispatch
    /// \param[in] recv_func_parm  Receive function client data
    ///
    /// \return Dispatch id for this registration
    ///
    template <class T_SysDep, class T_Fifo, class T_Packet>
    int ShmemBaseDevice<T_SysDep, T_Fifo, T_Packet>::registerRecvFunction (Interface::RecvFunction_t   recv_func,
                                                                 void                   * recv_func_parm)
    {
      if (_dispatch_count > 256) return -1;

      _dispatch[_dispatch_count].function   = recv_func;
      _dispatch[_dispatch_count].clientdata = recv_func_parm;

      return _dispatch_count++;
    };

    template <class T_SysDep, class T_Fifo, class T_Packet>
    xmi_result_t ShmemBaseDevice<T_SysDep, T_Fifo, T_Packet>::post (size_t fnum, ShmemBaseMessage<T_Packet> * msg)
    {
      pushSendQueueTail (fnum, (QueueElem *) msg);
      return XMI_SUCCESS;
    };



    template <class T_SysDep, class T_Fifo, class T_Packet>
    int ShmemBaseDevice<T_SysDep, T_Fifo, T_Packet>::noop (void   * metadata,
                                                 void   * payload,
                                                 size_t   bytes,
                                                 void   * recv_func_parm)
    {
      abort();
      return 0;
    }


    template <class T_SysDep, class T_Fifo, class T_Packet>
    void ShmemBaseDevice<T_SysDep, T_Fifo, T_Packet>::advance_sendQ ()
    {
      unsigned peer;

      for (peer = 0; peer < _num_procs; peer++)
        advance_sendQ (peer);
    }

    template <class T_SysDep, class T_Fifo, class T_Packet>
    void ShmemBaseDevice<T_SysDep, T_Fifo, T_Packet>::advance_sendQ (size_t peer)
    {
      ShmemBaseMessage<T_Packet> * msg;
      size_t sequence;

      while (!__sendQ[peer].isEmpty())
        {
          // There is a pending message on the send queue.
          msg = (ShmemBaseMessage<T_Packet> *) __sendQ[peer].peekHead ();

          xmi_result_t result = writeSinglePacket (peer, msg, sequence);

          if (result == XMI_SUCCESS)
            {
              if (msg->done())
                {
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
    };

    template <class T_SysDep, class T_Fifo, class T_Packet>
    void ShmemBaseDevice<T_SysDep, T_Fifo, T_Packet>::advance_doneQ ()
    {
      size_t peer;

      for (peer = 0; peer < _num_procs; peer++)
        advance_doneQ (peer);
    };

    template <class T_SysDep, class T_Fifo, class T_Packet>
    void ShmemBaseDevice<T_SysDep, T_Fifo, T_Packet>::advance_doneQ (size_t peer)
    {
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
            popDoneQueueHead (peer);
            msg->executeCallback ();
          }
          else
          {
            return;
          }
        }
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
