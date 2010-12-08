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

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

namespace PAMI
{
  namespace Device
  {
    template <class T_Fifo, class T_Shaddr>
    pami_result_t ShmemDevice<T_Fifo,T_Shaddr>::init (size_t clientid,
                                             size_t contextid,
                                             pami_client_t     client,
                                             pami_context_t    context,
                                             PAMI::Memory::MemoryManager *mm,
                                             PAMI::Device::Generic::Device * progress)
    {
      TRACE_ERR((stderr, "(%zu) ShmemDevice::init ()\n", __global.mapping.task()));
      _client   = client;
      _context  = context;
      _contextid  = contextid;
      _mm = mm;
      _progress = progress;
      _local_progress_device = &(Generic::Device::Factory::getDevice (progress, 0, contextid));

      unsigned i;
      __global.mapping.nodePeers (_num_procs);

      PAMI::Interface::Mapping::nodeaddr_t nodeaddr;
      __global.mapping.nodeAddr (nodeaddr);
      _global_task = __global.mapping.task();//nodeaddr.global;
      _local_task  = nodeaddr.local;
#ifdef __bgq__
      unsigned stride = 16 / _num_procs; //hack
      _local_task = _local_task / stride;//hack
#endif

      new (_rfifo) T_Fifo ();
      _rfifo->init (clientid);

      // Allocate memory for and construct the queue objects,
      // one for each context on the local node
      pami_result_t prc;
      prc = __global.heap_mm->memalign((void **)&__sendQ, 0,
						_total_fifos * sizeof(*__sendQ));
      PAMI_assertf(prc == PAMI_SUCCESS, "alloc of __sendQ failed");

      for (i = 0; i < _total_fifos; i++)
        {
          new (&__sendQ[i]) Shmem::SendQueue (Generic::Device::Factory::getDevice (progress, 0, contextid));
        }

      // Initialize the "last injection sequence id per fifo" array.
      prc = __global.heap_mm->memalign((void **)&_last_inj_sequence_id, 0,
					_total_fifos * sizeof(*_last_inj_sequence_id));
      PAMI_assertf(prc == PAMI_SUCCESS, "alloc of _last_inj_sequence_id failed");
      for (i=0; i<_total_fifos; i++) _last_inj_sequence_id[i] = (size_t) -1;

      // Initialize the registered receive function array to unexpected().
      // The array is limited to 256 dispatch ids because of the size of the
      // dispatch id field in the packet header.
      for (i = 0; i < DISPATCH_SET_COUNT*DISPATCH_SET_SIZE; i++)
        {
          _dispatch[i].function   = unexpected;
          _dispatch[i].clientdata = (void *)&__ueQ[i/DISPATCH_SET_SIZE];
        }

      // Register system dispatch functions
      registerSystemRecvFunction (system_shaddr_read, &shaddr, system_ro_put_dispatch);

      return PAMI_SUCCESS;
    }

    template <class T_Fifo, class T_Shaddr>
    bool ShmemDevice<T_Fifo,T_Shaddr>::isInit_impl ()
    {
      return true;
    }

    /// \see PAMI::Device::Interface::BaseDevice::peers()
    template <class T_Fifo, class T_Shaddr>
    size_t ShmemDevice<T_Fifo,T_Shaddr>::peers_impl ()
    {
      return _num_procs;
    }

    /// \see PAMI::Device::Interface::BaseDevice::task2peer()
    template <class T_Fifo, class T_Shaddr>
    size_t ShmemDevice<T_Fifo,T_Shaddr>::task2peer_impl (size_t task)
    {
      PAMI::Interface::Mapping::nodeaddr_t address;
      TRACE_ERR((stderr, ">> ShmemDevice::task2peer_impl(%zu)\n", task));
      __global.mapping.task2node (task, address);
      TRACE_ERR((stderr, "   ShmemDevice::task2peer_impl(%zu), address = {%zu, %zu}\n", task, address.global, address.local));

      size_t peer = 0;
      __global.mapping.node2peer (address, peer);
      TRACE_ERR((stderr, "<< ShmemDevice::task2peer_impl(%zu), peer = %zu\n", task, peer));

      return peer;
    }

    /// \see PAMI::Device::Interface::BaseDevice::isPeer()
    template <class T_Fifo, class T_Shaddr>
    bool ShmemDevice<T_Fifo,T_Shaddr>::isPeer_impl (size_t task)
    {
      return __global.mapping.isLocal(task);
    };

    template <class T_Fifo, class T_Shaddr>
    pami_result_t ShmemDevice<T_Fifo,T_Shaddr>::registerRecvFunction (size_t                      set,
                                                            Interface::RecvFunction_t   recv_func,
                                                            void                      * recv_func_parm,
                                                            uint16_t                  & id)
    {
      TRACE_ERR((stderr, ">> (%zu) ShmemDevice::registerRecvFunction(%zu,%p,%p) .. DISPATCH_SET_COUNT = %d\n", __global.mapping.task(), set, recv_func, recv_func_parm, DISPATCH_SET_COUNT));

      // This device only supports up to 256 dispatch sets.
      if (set >= DISPATCH_SET_COUNT) return PAMI_ERROR;

      // Find the next available id for this dispatch set.
      bool found_free_slot = false;
      size_t n = set * DISPATCH_SET_SIZE + DISPATCH_SET_COUNT;

      for (id = set * DISPATCH_SET_SIZE; id < n; id++)
        {
         // TRACE_ERR((stderr, "   (%zu) ShmemDevice::registerRecvFunction(), _dispatch[%d].function= %p\n", __global.mapping.task(), id, _dispatch[id].function));

          if (_dispatch[id].function == (Interface::RecvFunction_t) unexpected)
            {
              found_free_slot = true;
              break;
            }
        }

      if (!found_free_slot) return PAMI_ERROR;

      _dispatch[id].function   = recv_func;
      _dispatch[id].clientdata = recv_func_parm;

      // Deliver any unexpected packets for registered dispatch ids. Stop at
      // the first unexpected packet for an un-registered dispatch id.
      UnexpectedPacket * uepkt = NULL;
      while ((uepkt = (UnexpectedPacket *) __ueQ[set].peek()) != NULL)
      {
        if (_dispatch[uepkt->id].function != unexpected)
        {
          // Invoke the registered dispatch function
          TRACE_ERR((stderr, "   (%zu) ShmemDevice::registerRecvFunction() uepkt = %p, uepkt->id = %u\n", __global.mapping.task(), uepkt, uepkt->id));
          _dispatch[uepkt->id].function (uepkt->meta,
                                         uepkt->data,
                                         uepkt->bytes,
                                         _dispatch[uepkt->id].clientdata,
                                         uepkt->data);

          // Remove the unexpected packet from the queue and free
          __ueQ[set].dequeue();
          __global.heap_mm->free (uepkt);
        }
        else
        {
          // Stop unexpected queue processing.  This maintains packet order
          // which is required for protocols such as eager.
          break;
        }
      }

      TRACE_ERR((stderr, "<< (%zu) ShmemDevice::registerRecvFunction() => %d\n", __global.mapping.task(), id));
      return PAMI_SUCCESS;
    };


    template <class T_Fifo, class T_Shaddr>
    pami_result_t ShmemDevice<T_Fifo,T_Shaddr>::registerSystemRecvFunction (Interface::RecvFunction_t   recv_func,
                                                                            void                      * recv_func_parm,
                                                                            uint16_t                    id)
    {
      TRACE_ERR((stderr, ">> (%zu) ShmemDevice::registerSystemRecvFunction(%p,%p,%d) .. DISPATCH_SET_COUNT = %d, _dispatch[%d].function = %p (== %p ?)\n", __global.mapping.task(), recv_func, recv_func_parm, id, DISPATCH_SET_COUNT, id, _dispatch[id].function, unexpected));

      if (_dispatch[id].function != (Interface::RecvFunction_t) unexpected)
      {
        // Error .. this system dispatch id has already been registered.
        PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
        return PAMI_ERROR;
      }

      _dispatch[id].function   = recv_func;
      _dispatch[id].clientdata = recv_func_parm;

      TRACE_ERR((stderr, "   (%zu) ShmemDevice::registerSystemRecvFunction(), _dispatch[%d].function = %p, _dispatch[%d].clientdata = %p, this = %p\n", __global.mapping.task(), id, _dispatch[id].function, id, _dispatch[id].clientdata, this));


      // Deliver any unexpected packets for registered dispatch ids. Stop at
      // the first unexpected packet for an un-registered dispatch id.
      UnexpectedPacket * uepkt = NULL;
      size_t set = id / DISPATCH_SET_SIZE;
      while ((uepkt = (UnexpectedPacket *) __ueQ[set].peek()) != NULL)
      {
        if (_dispatch[uepkt->id].function != unexpected)
        {
          // Invoke the registered dispatch function
          TRACE_ERR((stderr, "   (%zu) ShmemDevice::registerSystemRecvFunction() uepkt = %p, uepkt->id = %d\n", __global.mapping.task(), uepkt, uepkt->id));
          _dispatch[uepkt->id].function (uepkt->meta,
                                         uepkt->data,
                                         uepkt->bytes,
                                         _dispatch[uepkt->id].clientdata,
                                         uepkt->data);

          // Remove the unexpected packet from the queue and free
          __ueQ[set].dequeue();
          __global.heap_mm->free (uepkt);
        }
        else
        {
          // Stop unexpected queue processing.  This maintains packet order
          // which is required for protocols such as eager.
          break;
        }
      }

      TRACE_ERR((stderr, "<< (%zu) ShmemDevice::registerSystemRecvFunction() => %d\n", __global.mapping.task(), id));
      return PAMI_SUCCESS;
    };


    template <class T_Fifo, class T_Shaddr>
    pami_result_t ShmemDevice<T_Fifo,T_Shaddr>::post (size_t fnum, Shmem::SendQueue::Message * msg)
    {
      TRACE_ERR((stderr, ">> (%zu) ShmemDevice::post(%zu, %p)\n", __global.mapping.task(), fnum, msg));
      __sendQ[fnum].post(msg);
      TRACE_ERR((stderr, "<< (%zu) ShmemDevice::post(%zu, %p)\n", __global.mapping.task(), fnum, msg));
      return PAMI_SUCCESS;
    };

    template <class T_Fifo, class T_Shaddr>
    pami_result_t ShmemDevice<T_Fifo,T_Shaddr>::post (PAMI::Device::Generic::GenericThread * work)
    {
      TRACE_ERR((stderr, ">> (%zu) ShmemDevice::post(%p)\n", __global.mapping.task(), work));
      _local_progress_device->postThread (work);
      TRACE_ERR((stderr, "<< (%zu) ShmemDevice::post(%p)\n", __global.mapping.task(), work));
      return PAMI_SUCCESS;
    };

    template <class T_Fifo, class T_Shaddr>
    int ShmemDevice<T_Fifo,T_Shaddr>::unexpected (void   * metadata,
                                         void   * payload,
                                         size_t   bytes,
                                         void   * recv_func_parm,
                                         void   * cookie)
    {
      //TRACE_ERR((stderr, ">> (%zu) ShmemDevice::unexpected()\n", __global.mapping.task()));

      // The metadata is at the front of the packet.
      PacketImpl * pkt = (PacketImpl *) metadata;

      UnexpectedPacket * uepkt;
      pami_result_t prc;
      prc = __global.heap_mm->memalign((void **)&uepkt, 0, sizeof(*uepkt));
      PAMI_assertf(prc == PAMI_SUCCESS, "alloc of uepkt failed");
      new ((void *)uepkt) UnexpectedPacket (pkt);

      TRACE_ERR((stderr, "   (%zu) ShmemDevice::unexpected(), uepkt = %p, uepkt->id = %u\n", __global.mapping.task(), uepkt, uepkt->id));

      CircularQueue * q = (CircularQueue *) recv_func_parm;
      q->enqueue ((CircularQueue::Element *) uepkt);

      //TRACE_ERR((stderr, "<< (%zu) ShmemDevice::unexpected(), q = %p\n", __global.mapping.task(), q));
      return 0;
    }

    template <class T_Fifo, class T_Shaddr>
    int ShmemDevice<T_Fifo,T_Shaddr>::system_shaddr_read (void   * metadata,
                                                          void   * payload,
                                                          size_t   bytes,
                                                          void   * recv_func_parm,
                                                          void   * cookie)
    {
      TRACE_ERR((stderr, ">> ShmemDevice::system_shaddr_read():%d .. recv_func_parm = %p\n", __LINE__, recv_func_parm));

      T_Shaddr * shaddr = (T_Shaddr *) recv_func_parm;

      typename ShmemDevice<T_Fifo,T_Shaddr>::SystemShaddrInfo * info =
        (typename ShmemDevice<T_Fifo,T_Shaddr>::SystemShaddrInfo *) payload;

      shaddr->read (&(info->_target_mr),
                    info->_target_offset,
                    &(info->_origin_mr),
                    info->_origin_offset,
                    info->_bytes);

      TRACE_ERR((stderr, "<< ShmemDevice::system_shaddr_read():%d\n", __LINE__));
      return 0;
    }

    template <class T_Fifo, class T_Shaddr>
    int ShmemDevice<T_Fifo,T_Shaddr>::system_shaddr_write (void   * metadata,
                                                           void   * payload,
                                                           size_t   bytes,
                                                           void   * recv_func_parm,
                                                           void   * cookie)
    {
      TRACE_ERR((stderr, ">> ShmemDevice::system_shaddr_write():%d\n", __LINE__));

      T_Shaddr * shaddr = (T_Shaddr *) recv_func_parm;

      typename ShmemDevice<T_Fifo,T_Shaddr>::SystemShaddrInfo * info =
        (typename ShmemDevice<T_Fifo,T_Shaddr>::SystemShaddrInfo *) payload;

      shaddr->write (&(info->_origin_mr),
                     info->_origin_offset,
                     &(info->_target_mr),
                     info->_target_offset,
                     info->_bytes);

      TRACE_ERR((stderr, "<< ShmemDevice::system_shaddr_write():%d\n", __LINE__));
      return 0;
    }


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
