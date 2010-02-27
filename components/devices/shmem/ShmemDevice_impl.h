/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
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

#include "Global.h"
#include "SysDep.h"
#include "Progress.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) //  fprintf x
#endif

namespace XMI
{
  namespace Device
  {
    template <class T_Fifo>
    xmi_result_t ShmemDevice<T_Fifo>::init (xmi_client_t     client,
                                            xmi_context_t    context,
                                            SysDep         * sysdep,
                                            ProgressDevice * progress)
    {
      TRACE_ERR((stderr, "(%zd) ShmemDevice::init ()\n", __global.mapping.task()));
      _client   = client;
      _context  = context;
      _sysdep   = sysdep;
      _progress = progress;

      unsigned i;
      __global.mapping.nodePeers (_num_procs);

      XMI::Interface::Mapping::nodeaddr_t nodeaddr;
      __global.mapping.nodeAddr (nodeaddr);
      _global_task = nodeaddr.global;
      _local_task  = nodeaddr.local;
#ifdef __bgq__
	  unsigned stride = 16/_num_procs; //hack
	  _local_task = _local_task/stride;//hack
#endif

      new (_rfifo) T_Fifo ();
      _rfifo->init (*sysdep);

      // barrier ?

      // Allocate memory for and construct the queue objects,
      // one for each context on the local node
      __sendQ = (MessageQueue *) malloc ((sizeof (MessageQueue) * _total_fifos));

      for (i = 0; i < _total_fifos; i++)
        {
          new (&__sendQ[i]) MessageQueue (_progress);
        }


      // Initialize the send queue mask to zero (empty).
//      __sendQMask = 0;

      // Initialize the registered receive function array to noop().
      // The array is limited to 256 dispatch ids because of the size of the
      // dispatch id field in the packet header.
      for (i = 0; i < DISPATCH_SET_COUNT*DISPATCH_SET_SIZE; i++)
        {
          _dispatch[i].function   = noop;
          _dispatch[i].clientdata = NULL;
        }


      return XMI_SUCCESS;
    }

    template <class T_Fifo>
    bool ShmemDevice<T_Fifo>::isInit_impl ()
    {
      return true;
    }

    /// \see XMI::Device::Interface::BaseDevice::peers()
    template <class T_Fifo>
    size_t ShmemDevice<T_Fifo>::peers_impl ()
    {
      return _num_procs;
    }

    /// \see XMI::Device::Interface::BaseDevice::task2peer()
    template <class T_Fifo>
    size_t ShmemDevice<T_Fifo>::task2peer_impl (size_t task)
    {
      XMI::Interface::Mapping::nodeaddr_t address;
      TRACE_ERR((stderr,">> ShmemDevice::task2peer_impl(%zu)\n",task));
      __global.mapping.task2node (task, address);
      TRACE_ERR((stderr,"   ShmemDevice::task2peer_impl(%zu), address = {%zu, %zu}\n",task, address.global, address.local));

      size_t peer = 0;
      __global.mapping.node2peer (address, peer);
      TRACE_ERR((stderr,"<< ShmemDevice::task2peer_impl(%zu), peer = %zu\n",task, peer));

#ifdef __bgq__
      return task; //hack
#else
      return peer;
#endif
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
    template <class T_Fifo>
    xmi_result_t ShmemDevice<T_Fifo>::registerRecvFunction (size_t                      set,
                                                            Interface::RecvFunction_t   recv_func,
                                                            void                      * recv_func_parm,
                                                            uint16_t                  & id)
    {
      TRACE_ERR((stderr, ">> (%zd) ShmemDevice::registerRecvFunction(%d,%p,%p) .. DISPATCH_SET_COUNT = %d\n", __global.mapping.task(), set, recv_func, recv_func_parm, DISPATCH_SET_COUNT));

      // This device only supports up to 256 dispatch sets.
      if (set >= DISPATCH_SET_COUNT) return XMI_ERROR;

      // Find the next available id for this dispatch set.
      bool found_free_slot = false;
      size_t n = set * DISPATCH_SET_SIZE + DISPATCH_SET_COUNT;
      for (id = set * DISPATCH_SET_SIZE; id < n; id++)
        {
          TRACE_ERR((stderr, "   (%zd) ShmemDevice::registerRecvFunction(), _dispatch[%d].function= %p\n", __global.mapping.task(), id, _dispatch[id].function));
          if (_dispatch[id].function == (Interface::RecvFunction_t) noop)
            {
              found_free_slot = true;
              break;
            }
        }

      if (!found_free_slot) return XMI_ERROR;

      _dispatch[id].function   = recv_func;
      _dispatch[id].clientdata = recv_func_parm;

      TRACE_ERR((stderr, "<< (%zd) ShmemDevice::registerRecvFunction() => %d\n", __global.mapping.task(), id));
      return XMI_SUCCESS;
    };

    template <class T_Fifo>
    xmi_result_t ShmemDevice<T_Fifo>::post (size_t fnum, ShmemMessage * msg)
    {
      __sendQ[fnum].post (msg);
//      pushSendQueueTail (fnum, (XMI::Queue::Element *) msg);
      return XMI_SUCCESS;
    };



    template <class T_Fifo>
    int ShmemDevice<T_Fifo>::noop (void   * metadata,
                                   void   * payload,
                                   size_t   bytes,
                                   void   * recv_func_parm,
                                   void   * cookie)
    {
      XMI_abort();
      return 0;
    }

#if 0
    template <class T_Fifo, class T_Progress>
    int ShmemDevice<T_Fifo,T_Progress>::advance_sendQ ()
    {
      TRACE_ERR((stderr, "(%zd) ShmemDevice::advance_sendQ () >> _num_procs = %zd\n", __global.mapping.task(), _num_procs));
      unsigned peer;

      int events = 0;

      for (peer = 0; peer < _num_procs; peer++)
        events += advance_sendQ (peer);

      TRACE_ERR((stderr, "(%zd) ShmemDevice::advance_sendQ () <<\n", __global.mapping.task()));
      return events;
    }

    template <class T_Fifo, class T_Progress>
    int ShmemDevice<T_Fifo,T_Progress>::advance_sendQ (size_t peer)
    {
      ShmemMessage * msg;
      size_t sequence;
      int events = 0;

      TRACE_ERR((stderr, "(%zd) ShmemDevice::advance_sendQ (%zd) >>\n", __global.mapping.task(), peer));

      while (!__sendQ[peer].isEmpty())
        {
          // There is a pending message on the send queue.
          msg = (ShmemMessage *) __sendQ[peer].peekHead ();

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
#endif
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
