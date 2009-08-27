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

#ifndef TRACE_ERR
#define TRACE_ERR(x)  //fprintf x
#endif

namespace XMI
{
  namespace Device
  {
    template <class T_Fifo, class T_Packet>
    int ShmemBaseDevice<T_Fifo, T_Packet>::init_internal (SysDep & sysdep)
    {
      __sysdep = &sysdep;
      __mapping = &sysdep.mapping();

      unsigned i, j;
      _num_procs = __mapping->numActiveRanksLocal ();

      _global_rank = __mapping->rank ();
      size_t global;
      __mapping->rank2node (_global_rank, global, _local_rank);
      _fifo = (T_Fifo *) malloc (sizeof (T_Fifo) * _num_procs);

      for (i = 0; i < _num_procs; i++)
        {
          new (&_fifo[i]) T_Fifo ();
          _fifo[i].init (sysdep);

          if (_local_rank == i)
            {
              _rfifo = &_fifo[i];
            }
        }

      // Allocate memory for and construct the queue objects,
      // one for each local rank.
      __sendQ = (XMI::Queueing::Queue *) malloc ((sizeof (XMI::Queueing::Queue) * _num_procs));
      __doneQ = (XMI::Queueing::Queue *) malloc ((sizeof (XMI::Queueing::Queue) * _num_procs));

      for (i = 0; i < _num_procs; i++)
      {
        new (&__sendQ[i]) XMI::Queueing::Queue ();
        new (&__doneQ[i]) XMI::Queueing::Queue ();
      }

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

      for (i = 0; i < 1; i++)
        {
          _connection[i] = (void **) malloc (sizeof(void *) * _num_procs);

          for (j = 0; j < _num_procs; j++)
            _connection[i][j] = NULL;
        }

      return 0;
    }

    template <class T_Fifo, class T_Packet>
    bool ShmemBaseDevice<T_Fifo, T_Packet>::isInit_impl ()
    {
      return true;
    }

    /// \see XMI::Device::Interface::PacketDevice::requiresRead()
    template <class T_Fifo, class T_Packet>
    bool ShmemBaseDevice<T_Fifo, T_Packet>::requiresRead_impl ()
    {
      return false;
    };

    /// \see XMI::Device::Interface::PacketDevice::getPacketMetadataSize()
    template <class T_Fifo, class T_Packet>
    size_t ShmemBaseDevice<T_Fifo, T_Packet>::getPacketMetadataSize_impl ()
    {
      return T_Packet::headerSize_impl;
    };

    /// \see XMI::Device::Interface::PacketDevice::getPacketPayloadSize()
    template <class T_Fifo, class T_Packet>
    size_t ShmemBaseDevice<T_Fifo, T_Packet>::getPacketPayloadSize_impl ()
    {
      return T_Packet::payloadSize_impl;
    };

    /// \see XMI::Device::Interface::PacketDevice::getMessageMetadataSize()
    template <class T_Fifo, class T_Packet>
    size_t ShmemBaseDevice<T_Fifo, T_Packet>::getMessageMetadataSize_impl ()
    {
      return T_Packet::headerSize_impl - 1;
    };

    ///
    /// \brief Regieter the receive function to dispatch when a packet arrives.
    ///
    /// \param[in] recv_func       Receive function to dispatch
    /// \param[in] recv_func_parm  Receive function client data
    ///
    /// \return Dispatch id for this registration
    ///
    template <class T_Fifo, class T_Packet>
    int ShmemBaseDevice<T_Fifo, T_Packet>::registerRecvFunction (Packet::RecvFunction_t   recv_func,
                                                                 void                   * recv_func_parm)
    {
      if (_dispatch_count > 256) return -1;

      _dispatch[_dispatch_count].function   = recv_func;
      _dispatch[_dispatch_count].clientdata = recv_func_parm;

      return _dispatch_count++;
    };

    template <class T_Fifo, class T_Packet>
    CM_Result ShmemBaseDevice<T_Fifo, T_Packet>::post (size_t fnum, ShmemBaseMessage<T_Packet> * msg)
    {
      pushSendQueueTail (fnum, (Queueing::QueueElem *) msg);
      return CM_SUCCESS;
    };



    template <class T_Fifo, class T_Packet>
    int ShmemBaseDevice<T_Fifo, T_Packet>::noop (int      channel,
                                                 void   * metadata,
                                                 void   * payload,
                                                 size_t   bytes,
                                                 void   * recv_func_parm)
    {
      abort();
      return 0;
    }


    template <class T_Fifo, class T_Packet>
    void ShmemBaseDevice<T_Fifo, T_Packet>::advance_sendQ ()
    {
      unsigned peer;

      for (peer = 0; peer < _num_procs; peer++)
        advance_sendQ (peer);
    }

    template <class T_Fifo, class T_Packet>
    void ShmemBaseDevice<T_Fifo, T_Packet>::advance_sendQ (size_t peer)
    {
      ShmemBaseMessage<T_Packet> * msg;
      size_t sequence;

      while (!__sendQ[peer].isEmpty())
        {
          // There is a pending message on the send queue.
          msg = (ShmemBaseMessage<T_Packet> *) __sendQ[peer].peekHead ();

          CM_Result result = writeSinglePacket (peer, msg, sequence);

          if (result == CM_SUCCESS)
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
                      pushDoneQueueTail (peer, (Queueing::QueueElem *) msg);
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

    template <class T_Fifo, class T_Packet>
    void ShmemBaseDevice<T_Fifo, T_Packet>::advance_doneQ ()
    {
      size_t peer;

      for (peer = 0; peer < _num_procs; peer++)
        advance_doneQ (peer);
    };

    template <class T_Fifo, class T_Packet>
    void ShmemBaseDevice<T_Fifo, T_Packet>::advance_doneQ (size_t peer)
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
