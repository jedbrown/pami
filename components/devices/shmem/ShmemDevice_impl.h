/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2008                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/ShmemDevice_impl.h
 * \brief ???
 */

#ifndef __components_devices_shmem_ShmemDevice_impl_h__
#define __components_devices_shmem_ShmemDevice_impl_h__

#include "SysDep.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

namespace XMI
{
  namespace Device
  {
    template <class T_Fifo, class T_Packet>
    int ShmemDevice<T_Fifo, T_Packet>::init_impl (XMI::SysDep * sysdep)
    {
      TRACE_ERR((stderr, "(%zd) ShmemDevice::init_impl ()\n", __global.mapping.task()));
      _sysdep = sysdep;
      TRACE_ERR((stderr, "(%zd) ShmemDevice::init_impl () _sysdep = %p\n", __global.mapping.task(), _sysdep));

      unsigned i, j;
      __global.mapping.nodePeers (_num_procs);
      TRACE_ERR((stderr, "(%zd) ShmemDevice::init_impl () .. 0 _num_procs = %zd\n", __global.mapping.task(), _num_procs));

      XMI::Interface::Mapping::nodeaddr_t nodeaddr;
      __global.mapping.nodeAddr (nodeaddr);
      _global_task = nodeaddr.global;
      _local_task  = nodeaddr.local;

      TRACE_ERR((stderr, "(%zd) ShmemDevice::init_impl () .. 1\n", __global.mapping.task()));

      // Allocate a shared memory segment for the fifos
      size_t size = ((sizeof(T_Fifo) + 15) & 0xfff0) * _num_procs;
      _sysdep->mm.memalign ((void **)&_fifo, 16, size);
      TRACE_ERR((stderr, "(%zd) ShmemDevice::init_impl () .. 2\n", __global.mapping.task()));

      // Initialize the fifo acting as the reception fifo for this local task
      _rfifo = &_fifo[_local_task];
      new (_rfifo) T_Fifo ();
      TRACE_ERR((stderr, "(%zd) ShmemDevice::init_impl () .. 3, _local_task = %zd, _fifo = %p, _rfifo = %p\n", __global.mapping.task(), _local_task, _fifo, _rfifo));
      _rfifo->init (*sysdep);
      TRACE_ERR((stderr, "(%zd) ShmemDevice::init_impl () .. 4\n", __global.mapping.task()));

      // barrier ?

      // Allocate memory for and construct the queue objects,
      // one for each local rank.
      __sendQ = (Queue *) malloc ((sizeof (Queue) * _num_procs));

      TRACE_ERR((stderr, "(%zd) ShmemDevice::init_impl () .. 5\n", __global.mapping.task()));

      for (i = 0; i < _num_procs; i++)
        {
          new (&__sendQ[i]) Queue ();
        }

      TRACE_ERR((stderr, "(%zd) ShmemDevice::init_impl () .. 6\n", __global.mapping.task()));

      // Initialize the send queue mask to zero (empty).
      __sendQMask = 0;

      // Initialize the registered receive function array to noop().
      // The array is limited to 256 dispatch ids because of the size of the
      // dispatch id field in the packet header.
      for (i = 0; i < 256*256; i++)
        {
          _dispatch[i].function   = noop;
          _dispatch[i].clientdata = NULL;
        }

      TRACE_ERR((stderr, "(%zd) ShmemDevice::init_impl () .. 7\n", __global.mapping.task()));

      return 0;
    }

    template <class T_Fifo, class T_Packet>
    bool ShmemDevice<T_Fifo, T_Packet>::isInit_impl ()
    {
      return true;
    }

    /// \see XMI::Device::Interface::BaseDevice::peers()
    template <class T_Fifo, class T_Packet>
    size_t ShmemDevice<T_Fifo, T_Packet>::peers_impl ()
    {
      return _num_procs;
    }

    /// \see XMI::Device::Interface::BaseDevice::task2peer()
    template <class T_Fifo, class T_Packet>
    size_t ShmemDevice<T_Fifo, T_Packet>::task2peer_impl (size_t task)
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
    xmi_result_t ShmemDevice<T_Fifo, T_Packet>::registerRecvFunction (size_t                      set,
                                                                      Interface::RecvFunction_t   recv_func,
                                                                      void                      * recv_func_parm,
                                                                      uint16_t                  & id)
    {
      TRACE_ERR((stderr, ">> (%zd) ShmemDevice::registerRecvFunction (%d,%p,%p)\n", __global.mapping.task(), id, recv_func, recv_func_parm));

      // This device only supports up to 256 dispatch sets.
      if (set >= 256) return XMI_ERROR;

      // Find the next available id for this dispatch set.
      bool found_free_slot = false;
      for (id = set*256; id < ((set+1)*256); id++)
        {
          if (_dispatch[id].function == (Interface::RecvFunction_t) noop)
          {
            found_free_slot = true;
            break;
          }
        }

      if (!found_free_slot) return XMI_ERROR;

      _dispatch[id].function   = recv_func;
      _dispatch[id].clientdata = recv_func_parm;

      TRACE_ERR((stderr, "<< (%zd) ShmemDevice::registerRecvFunction (%d,%p,%p) => %d\n", __global.mapping.task(), set, recv_func, recv_func_parm, id));
      return XMI_SUCCESS;
    };

    template <class T_Fifo, class T_Packet>
    xmi_result_t ShmemDevice<T_Fifo, T_Packet>::post (size_t fnum, ShmemMessage<T_Packet> * msg)
    {
      pushSendQueueTail (fnum, (QueueElem *) msg);
      return XMI_SUCCESS;
    };



    template <class T_Fifo, class T_Packet>
    int ShmemDevice<T_Fifo, T_Packet>::noop (void   * metadata,
                                                          void   * payload,
                                                          size_t   bytes,
                                                          void   * recv_func_parm,
                                                          void   * cookie)
    {
      XMI_abort();
      return 0;
    }


    template <class T_Fifo, class T_Packet>
    int ShmemDevice<T_Fifo, T_Packet>::advance_sendQ ()
    {
      TRACE_ERR((stderr, "(%zd) ShmemDevice::advance_sendQ () >> _num_procs = %zd\n", __global.mapping.task(), _num_procs));
      unsigned peer;

      int events = 0;

      for (peer = 0; peer < _num_procs; peer++)
        events += advance_sendQ (peer);

      TRACE_ERR((stderr, "(%zd) ShmemDevice::advance_sendQ () <<\n", __global.mapping.task()));
      return events;
    }

    template <class T_Fifo, class T_Packet>
    int ShmemDevice<T_Fifo, T_Packet>::advance_sendQ (size_t peer)
    {
      ShmemMessage<T_Packet> * msg;
      size_t sequence;
      int events = 0;

      TRACE_ERR((stderr, "(%zd) ShmemDevice::advance_sendQ (%zd) >>\n", __global.mapping.task(), peer));

      while (!__sendQ[peer].isEmpty())
        {
          // There is a pending message on the send queue.
          msg = (ShmemMessage<T_Packet> *) __sendQ[peer].peekHead ();

          if (Memregion::shared_address_read_supported ||
              Memregion::shared_address_write_supported)
            {
              if (msg->isRMAType() == true)
                {
                  sequence = _fifo[peer].nextInjSequenceId();

                  size_t last_rec_seq_id = _fifo[peer].lastRecSequenceId ();

                  if (sequence - 1 <= last_rec_seq_id) //sequence id is carried by a pt-to-pt message before me
                    {
                      Memregion * local_memregion  = (Memregion *) NULL;
                      Memregion * remote_memregion = (Memregion *) NULL;
                      size_t local_offset, remote_offset, bytes;

                      if (msg->getRMA (&local_memregion,
                                       local_offset,
                                       &remote_memregion,
                                       remote_offset,
                                       bytes))
                        {
                          local_memregion->write (local_offset, remote_memregion, remote_offset, bytes);
                        }
                      else
                        {
                          local_memregion->read (local_offset, remote_memregion, remote_offset, bytes);
                        }

                      popSendQueueHead (peer);
                      msg->executeCallback ();
                      continue;
                    }
                  else
                    {
                      break; //dont process any further elements until the RMA message is processed
                    }
                }
            }

          TRACE_ERR((stderr, "(%zd) ShmemDevice::advance_sendQ (%zd) .. before writeSinglePacket()\n", __global.mapping.task(), peer));
          xmi_result_t result = writeSinglePacket (peer, msg, sequence);
          TRACE_ERR((stderr, "(%zd) ShmemDevice::advance_sendQ (%zd) ..  after writeSinglePacket(), result = %zd\n", __global.mapping.task(), peer, result));

          if (result == XMI_SUCCESS)
            {
              if (msg->done())
                {
                  events++;

                  // The message has completed processing the source data.
                  // Remove from the send queue and advance next message
                  // before invoking the message's done callback.
                  popSendQueueHead (peer);

                  // Invoke the send completion callback here.. may post
                  // another message!
                  msg->executeCallback ();
                  break;
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
  };
};
#undef TRACE_ERR

#endif

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
