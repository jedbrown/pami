/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmemcoll/ShmemCollDevice.h
 * \brief ???
 */

#ifndef __components_devices_shmemcoll_ShmemCollDevice_h__
#define __components_devices_shmemcoll_ShmemCollDevice_h__

#include <sys/uio.h>

#include "sys/pami.h"

#include "Arch.h"
#include "Memregion.h"

#include "components/atomic/Counter.h"
#include "components/devices/BaseDevice.h"
#include "components/devices/FactoryInterface.h"
#include "components/devices/PacketInterface.h"
#include "components/devices/generic/Device.h"
#include "components/devices/shmem/ShmemMessage.h"
#include "components/memory/MemoryAllocator.h"

#include "components/devices/shmemcoll/ShmemCollDesc.h"
#include "components/devices/ShmemCollInterface.h"
#include "common/default/PipeWorkQueue.h"

#include "components/devices/MulticastModel.h"
#include "components/devices/MultisyncModel.h"
#include "components/devices/MulticombineModel.h"


#undef TRACE_ERR

#ifndef TRACE_ERR
#define TRACE_ERR(x)  fprintf x
#endif

#define MATCH_DISPATCH_SIZE	256
namespace PAMI
{
  namespace Device
  {

    typedef struct match_dispatch_t
    {
      Interface::MatchFunction_t   function;
      void                      * clientdata;
    } match_dispatch_t;



    template < class T_Desc >
    class ShmemCollDevice : public Interface::BaseDevice< ShmemCollDevice<T_Desc> >
    {
      protected:


      public:

        // Inner factory class
        class Factory : public Interface::FactoryInterface<Factory, ShmemCollDevice, PAMI::Device::Generic::Device>
        {
          public:
            static inline ShmemCollDevice * generate_impl (size_t clientid, size_t n, Memory::MemoryManager & mm, PAMI::Device::Generic::Device *gds)
            {
              size_t i;
              TRACE_ERR((stderr, ">> ShmemCollDevice::Factory::generate_impl() n = %zu\n", n));

              // Determine the number of contexts on the entire node, then
              // allocate shared memory for all contexts.

              // Get the number of peer tasks on the node
              size_t npeers = 0;
              __global.mapping.nodePeers (npeers);
              TRACE_ERR((stderr, "ShmemCollDevice::Factory::generate_impl() npeers = %zu\n", npeers));

#if 0
              // Allocate a "context count" array and a peer fifo pointer
              // array from shared memory
              volatile size_t * ncontexts = NULL;
              size_t size = sizeof(size_t) * 2 * npeers;
              TRACE_ERR((stderr, "ShmemCollDevice::Factory::generate_impl() size = %zu\n", size));
              mm.memalign ((void **)&ncontexts, 16, size);
              TRACE_ERR((stderr, "ShmemCollDevice::Factory::generate_impl() ncontexts = %p\n", ncontexts));

              //size_t * peer_fnum = (size_t *)ncontexts + npeers;

              // Get the peer id for this task
              size_t me = 0;
              PAMI::Interface::Mapping::nodeaddr_t address;

              __global.mapping.nodeAddr (address);
              TRACE_ERR((stderr, "ShmemCollDevice::Factory::generate_impl() after nodeAddr() global %zu, local %zu\n", address.global, address.local));
              __global.mapping.node2peer (address, me);
              TRACE_ERR((stderr, "ShmemCollDevice::Factory::generate_impl() me = %zu\n", me));

              // Set the number of contexts in this peer
              ncontexts[me] = n;
              TRACE_ERR((stderr, "ShmemCollDevice::Factory::generate_impl() ncontexts = %p, ncontexts[%zu] = %zu\n", ncontexts, me, ncontexts[me]));

              // Determine the total number of contexts on the node and the
              // number of contexts in each peer
              size_t done = 0;
              size_t total_desc_fifos = 0;
#if 1
              int countdown = 10000;
#endif

              while (done != npeers)
                {
                  total_desc_fifos = 0;
                  done = 0;

                  for (i = 0; i < npeers; i++)
                    {
                      total_desc_fifos += ncontexts[i];

                      if (ncontexts[i] > 0) done++;

#if 1

                      if (countdown == 1) fprintf(stderr, "ShmemCollDevice::Factory::generate_impl() ncontexts[%zu] = %zu, %p, %zu\n", i, ncontexts[i], ncontexts, mm.available());

#endif
                    }

#if 1
                  PAMI_assertf(countdown--, "I give up\n");
#endif
                }

              TRACE_ERR((stderr, "ShmemCollDevice::Factory::generate_impl() ncontexts = %p total_desc_fifos:%zu sync'd\n", ncontexts, total_desc_fifos));
#endif
              // Allocate a "context count" array and a peer fifo pointer
              // array from shared memory
              volatile size_t * ncontexts = NULL;
              size_t size = sizeof(size_t) * (2 * npeers + 1);
              TRACE_ERR((stderr, "ShmemDevice::Factory::generate_impl() size = %zu\n", size));
              mm.memalign ((void **)&ncontexts, 16, size);
              TRACE_ERR((stderr, "ShmemDevice::Factory::generate_impl() ncontexts = %p\n", ncontexts));

              size_t * peer_fnum = (size_t *)ncontexts + npeers + 1;

              // Get the peer id for this task
              size_t me = 0;
              PAMI::Interface::Mapping::nodeaddr_t address;

              __global.mapping.nodeAddr (address);
              TRACE_ERR((stderr, "ShmemDevice::Factory::generate_impl() after nodeAddr() global %zu, local %zu\n", address.global, address.local));
              __global.mapping.node2peer (address, me);
              TRACE_ERR((stderr, "ShmemDevice::Factory::generate_impl() me = %zu\n", me));

              // will there always be a "0"?
              TRACE_ERR((stderr, "ShmemDevice::Factory::generate_impl() before barriered shmemzero \n"));
              local_barriered_shmemzero((void *)ncontexts, size, npeers, me == 0);
              TRACE_ERR((stderr, "ShmemDevice::Factory::generate_impl() after barriered shmemzero \n"));
#if 1

              ncontexts[me] = n;
              __sync_fetch_and_add(&ncontexts[npeers], 1);
              mem_sync(); // paranoia?

              while (ncontexts[npeers] < npeers);

              TRACE_ERR((stderr, "ShmemDevice::Factory::generate_impl() after getting the total contexts \n"));
              mem_sync(); // paranoia?
              //size_t total_fifos_on_node = 0;
              size_t total_desc_fifos = 0;

              // Assign fifo indexes to the peer fnum cache while computing total
              for (i = 0; i < npeers; i++)
                {
                  // This should only be done by one peer, but all write the same data
                  // and this way no extra synchronization is needed.
                  peer_fnum[i] = total_desc_fifos;
                  total_desc_fifos += ncontexts[i];
                }

#endif
#if 0
              //hack for now..remove later
              size_t total_desc_fifos = 0;

              for (i = 0; i < npeers; i++)
                {
                  // This should only be done by one peer, but all write the same data
                  // and this way no extra synchronization is needed.
                  peer_fnum[i] = total_desc_fifos;
                  total_desc_fifos += 1;
                }

#endif

              //T_Desc * all_desc = NULL;
              Shmem::ShmemCollDescFifo<T_Desc>* all_desc_fifos = NULL;
              //size = (sizeof(Shmem::ShmemCollDescFifo<T_Desc>)) * total_desc_fifos + 128;
              size = ((sizeof(Shmem::ShmemCollDescFifo<T_Desc>) + 128) & 0xffffff80 ) * total_desc_fifos ;
              TRACE_ERR((stderr, "ShmemCollDevice::Factory::allocating %zu bytes\n", size));
              mm.memalign ((void **)&all_desc_fifos, 128, size);
              assert(all_desc_fifos != NULL);

              Shmem::ShmemCollDescFifo<T_Desc>* all_world_desc_fifos = NULL;
              //size = (sizeof(Shmem::ShmemCollDescFifo<T_Desc>)) * total_desc_fifos + 128;
              size = ((sizeof(Shmem::ShmemCollDescFifo<T_Desc>) + 128) & 0xffffff80 ) * total_desc_fifos ;
              mm.memalign ((void **)&all_world_desc_fifos, 128, size);
              assert(all_world_desc_fifos != NULL);

              size_t *peer_desc_fnum;
              pami_result_t rc;	// avoid warning when ASSERTS=0
              rc = __global.heap_mm->memalign((void **) & peer_desc_fnum, 0, sizeof(*peer_desc_fnum) * npeers);
              PAMI_assertf(rc == PAMI_SUCCESS, "alloc failed for peer_desc_fnum");
              // Assign fifo indexes to the peer fnum cache
              peer_desc_fnum[0] = 0;


              for (i = 1; i < npeers; i++)
                {
                  peer_desc_fnum[i] = peer_desc_fnum[i-1] + ncontexts[i-1];
                  TRACE_ERR((stderr, "ShmemCollDevice::Factory::generate_impl() i=%zu peer_desc_fnum[i]:%zu \n", i, peer_desc_fnum[i]));
                }

              ShmemCollDevice * devices;
              rc = __global.heap_mm->memalign((void **) & devices, 16, sizeof(*devices) * n);
              PAMI_assertf(rc == PAMI_SUCCESS, "alloc failed for ShmemCollDevice[%zu], errno=%d\n", n, errno);

              for (i = 0; i < n; ++i)
                {
                  new (&devices[i]) ShmemCollDevice (i, total_desc_fifos, all_desc_fifos, all_world_desc_fifos, peer_desc_fnum);
                }

              TRACE_ERR((stderr, "<< ShmemCollDevice::Factory::generate_impl()\n"));
              return devices;
            };

            static inline pami_result_t init_impl (ShmemCollDevice    * devices,
                                                   size_t           clientid,
                                                   size_t           contextid,
                                                   pami_client_t     client,
                                                   pami_context_t    context,
                                                   Memory::MemoryManager *mm,
                                                   PAMI::Device::Generic::Device * progress)
            {
              return getDevice_impl(devices, clientid, contextid).init (clientid, contextid, client, context, mm, progress);
            };

            static inline size_t advance_impl (ShmemCollDevice * devices,
                                               size_t        clientid,
                                               size_t        contextid)
            {
              return getDevice_impl(devices, clientid, contextid).advance ();
            };

            static inline ShmemCollDevice & getDevice_impl (ShmemCollDevice * devices,
                                                            size_t        clientid,
                                                            size_t        contextid)
            {
              return devices[contextid];
            };
        };

        inline ShmemCollDevice (size_t contextid, size_t ndescfifos, Shmem::ShmemCollDescFifo<T_Desc>* all_fifos,
                                Shmem::ShmemCollDescFifo<T_Desc>* all_world_fifos, size_t * fnum_hash) :
            Interface::BaseDevice< ShmemCollDevice<T_Desc> > (),
            _total_fifos (ndescfifos),
            _fnum_hash (fnum_hash),
            _all_desc_fifos(all_fifos),
            _all_world_desc_fifos(all_world_fifos),
            _contextid (contextid),
            __collectiveQ (NULL),
            _progress (NULL)
        {
          TRACE_ERR((stderr, "ShmemCollDevice() constructor\n"));

          // Get the peer id for this task
          _me = 0;
          PAMI::Interface::Mapping::nodeaddr_t address;
          __global.mapping.nodeAddr (address);
          __global.mapping.node2peer (address, _me);

          size_t f = fnum (_me, contextid);
          _my_desc_fifo = & _all_desc_fifos[f];
          _my_world_desc_fifo = & _all_world_desc_fifos[f];
        };

        //inline ~ShmemCollDevice () {};
        inline ~ShmemCollDevice ();

        inline size_t getLocalRank ();

        // ------------------------------------------

        /// \see PAMI::Device::Interface::BaseDevice::isInit()
        bool isInit_impl ();

        /// \see PAMI::Device::Interface::BaseDevice::peers()
        inline size_t peers_impl ();

        /// \see PAMI::Device::Interface::BaseDevice::task2peer()
        inline size_t task2peer_impl (size_t task);

        /// \see PAMI::Device::Interface::BaseDevice::isPeer()
        inline bool isPeer_impl (size_t task);

        inline pami_context_t getContext_impl ();

        inline size_t getContextId_impl ();
        inline size_t getContextOffset_impl ();

        // ------------------------------------------

        /// \see PAMI::Device::Interface::PacketDevice::read()
        inline int read_impl (void * buf, size_t length, void * cookie);

        static const size_t metadata_size = T_Desc::packet_header_size - sizeof(uint16_t);
        static const size_t payload_size  = T_Desc::packet_payload_size;

        pami_result_t post (Shmem::SendQueue::Message * msg);

        pami_result_t postCollective (PAMI::PipeWorkQueue *src, PAMI::PipeWorkQueue *dst, size_t bytes, pami_callback_t cb_done,
                                      unsigned conn_id, unsigned master, uint16_t dispatch_id, void* state);

        pami_result_t postMulticastShmem (pami_multicast_t* mcast, unsigned master, uint16_t dispatch_id, void* state);

        pami_result_t postMulticastShaddr (pami_multicast_t* mcast, unsigned master, uint16_t dispatch_id, void* state);
        pami_result_t postDescriptor (T_Desc & desc);

        pami_result_t postPendingDescriptor (Shmem::SendQueue::Message * msg);

        pami_result_t getShmemWorldDesc(T_Desc** my_desc, T_Desc** master_desc, unsigned master);
        ///
        /// \brief Check if the send queue to an injection fifo is empty
        ///
        ///
        inline bool isCollectiveQueueEmpty ();

        inline bool isPendingDescQueueEmpty();

        inline Shmem::SendQueue *getQS ();

        inline pami_result_t init (size_t clientid, size_t contextid, pami_client_t client, pami_context_t context, Memory::MemoryManager *mm, PAMI::Device::Generic::Device * progress);

        inline size_t advance ();

        inline size_t fnum (size_t peer, size_t offset);

        pami_result_t registerMatchDispatch (Interface::MatchFunction_t   match_func, void * recv_func_parm, uint16_t &id);


        T_Desc * _desc;
        size_t _total_fifos;
        size_t * _fnum_hash;    //< Fifo number lookup table
        Shmem::ShmemCollDescFifo<T_Desc> *_all_desc_fifos;
        Shmem::ShmemCollDescFifo<T_Desc>  * _my_desc_fifo;

        Shmem::ShmemCollDescFifo<T_Desc> *_all_world_desc_fifos;
        Shmem::ShmemCollDescFifo<T_Desc>  * _my_world_desc_fifo;

        Memory::MemoryManager *_mm;
        pami_client_t       _client;
        pami_context_t      _context;
        size_t             _contextid;

        match_dispatch_t  _dispatch[MATCH_DISPATCH_SIZE];

        Shmem::SendQueue    *__collectiveQ;
        Shmem::SendQueue    *__pending_descriptorQ;
        PAMI::Device::Generic::Device * _progress;

        size_t            _num_procs;
        size_t            _global_task;
        size_t            _local_task;
        size_t            _me;

    };

    template <class T_Desc>
    inline ShmemCollDevice<T_Desc>::~ShmemCollDevice()
    {
    }

    template <class T_Desc>
    inline size_t ShmemCollDevice<T_Desc>::getLocalRank()
    {
      return _local_task;
    }

    template <class T_Desc>
    inline pami_context_t ShmemCollDevice<T_Desc>::getContext_impl()
    {
      return _context;
    }

    template <class T_Desc>
    inline size_t ShmemCollDevice<T_Desc>::getContextId_impl()
    {
      return _contextid;
    }
    template <class T_Desc>
    inline size_t ShmemCollDevice<T_Desc>::getContextOffset_impl()
    {
      return getContextId_impl();
    }

    template <class T_Desc>
    inline bool ShmemCollDevice<T_Desc>::isCollectiveQueueEmpty ()
    {
      return (__collectiveQ->size() == 0);
    }

    template <class T_Desc>
    inline bool ShmemCollDevice<T_Desc>::isPendingDescQueueEmpty ()
    {
      return (__pending_descriptorQ->size() == 0);
    }

    template <class T_Desc>
    inline Shmem::SendQueue * ShmemCollDevice<T_Desc>::getQS ()
    {
      return __collectiveQ;
    }


    template <class T_Desc>
    size_t ShmemCollDevice<T_Desc>::advance ()
    {
      size_t events = 0;

      /* Releasing done descriptors for comm world communicators */

      if (!_my_world_desc_fifo->is_empty())
        {
          //printf("Calling shmem colldevice advance\n");
          _my_world_desc_fifo->release_done_descriptors();
          events++;
        }


      return events;

      /* Advance logic for arbitrary communicator collective */

      if (!_my_desc_fifo->is_empty())
        {

          /*  Release all the "Done" descriptors */

          _my_desc_fifo->release_done_descriptors();

          /* Get the next descriptor to match..return if there no descriptor to be matched */

          T_Desc* desc = _my_desc_fifo->next_desc_pending_match();

          if (desc != NULL)
            {

              /* Find the descriptor matching the same connection id at the master */
              unsigned master = desc->get_master();
              T_Desc* matched_desc = _all_desc_fifos[master].match_descriptor(desc->get_conn_id());

              if (matched_desc != NULL)
                {
                  TRACE_ERR((stderr, "(%zu) ShmemCollDevice::found the matching descriptor at the master:%u and conn_id:%u \n",
                             __global.mapping.task(), master, desc->get_conn_id()));

                  uint16_t dispatch_id = desc->get_dispatch_id();
                  _dispatch[dispatch_id].function (desc, matched_desc, _dispatch[dispatch_id].clientdata);
                  desc->set_state(Shmem::ACTIVE);
                  _my_desc_fifo->advance_next_match();
                }
            }
        }
      else
        {
//			TRACE_ERR((stderr,"my_desc_fifo is empty\n"));
        }

    }


    template <class T_Desc>
    inline size_t ShmemCollDevice<T_Desc>::fnum (size_t peer, size_t offset)
    {
      TRACE_ERR((stderr, ">> ShmemCollDevice::fnum(%zu, %zu), _fnum_hash = %p\n", peer, offset, _fnum_hash));
      TRACE_ERR((stderr, "<< ShmemCollDevice::fnum(%zu, %zu), _fnum_hash[%zu] = %zu\n", peer, offset, peer, _fnum_hash[peer]));
      return _fnum_hash[peer] + offset;
    }


  };
};
#undef TRACE_ERR

// Include the non-inline method definitions
#include "components/devices/shmemcoll/ShmemCollDevice_impl.h"

#endif // __components_devices_shmem_shmembasedevice_h__
