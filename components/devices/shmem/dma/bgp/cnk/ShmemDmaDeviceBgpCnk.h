/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2008                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file devices/prod/cdi/shmem/dma/bgp/cnk/ShmemDmaDeviceBgpCnk.h
 * \brief ???
 */

#ifndef __dcmf_cdi_device_shmem_dma_bgp_cnk_h__
#define __dcmf_cdi_device_shmem_dma_bgp_cnk_h__

#include "../../../ShmemBaseDevice.h"
#include "../../../../BaseDevice.h"
#include "../../../../MessageDevice.h"

#define MAX_BGP_PROCS 4
#define WINDOW_SIZE (1<<28)

#ifndef TRACE_ERR
#define TRACE_ERR(x)  // fprintf x
#endif

namespace DCMF
{
  namespace CDI
  {
    template <class T_Fifo, class T_Packet>
    class ShmemDmaDeviceBgpCnk : public ShmemBaseDevice<T_Fifo, T_Packet>, public Base::Device<ShmemDmaDeviceBgpCnk<T_Fifo, T_Packet> >, public Message::Device<ShmemDmaDeviceBgpCnk<T_Fifo, T_Packet> >
    {
      public:
        inline ShmemDmaDeviceBgpCnk () :
            ShmemBaseDevice<T_Fifo, T_Packet> (),
            Message::Device<ShmemDmaDeviceBgpCnk<T_Fifo, T_Packet> > (),
            _dma_available (false)
        {

        };

        inline ~ShmemDmaDeviceBgpCnk () {};

        int init_impl (SysDep & sysdep)
        {
          ShmemBaseDevice<T_Fifo, T_Packet>::init_internal (sysdep);

          // Set up the tlb slots for each of the peer and also cache the last address mapping
          int minslot = 0; // Lowest TLB in the range of ProcessWindow TLBs for the current core
          int maxslot = 0; // Highest TLB in the range of ProcessWindow TLBs for the current core

          // call interface to see the range of tlb slots allocated for process window
          int rc = 0;
          rc = Kernel_GetProcessWindowSlotRange (&minslot, &maxslot);

          if (rc != 0 || minslot == -1 || maxslot == -1)
            {
              _dma_available = false;
              return true;
            }

          size_t global, peer;
          size_t rank = ShmemBaseDevice<T_Fifo, T_Packet>::__mapping->rank ();
          ShmemBaseDevice<T_Fifo, T_Packet>::__mapping->rank2node (rank, global, peer);
          unsigned j = 0;

          size_t num_procs = ShmemBaseDevice<T_Fifo, T_Packet>::__mapping->numActiveRanksLocal ();

          for (size_t i = 0; i < num_procs; i++)
            {
              if (peer != i)
                {
                  _tlbslot[i] = minslot + j++;
                }
            }

          _dma_available = true;
          return true;
        }

        /// \see DCMF::CDI::Base::Device::advance()
        inline int advance_impl ()
        {
          return ShmemBaseDevice<T_Fifo, T_Packet>::advance_internal ();
        }

        inline bool isDmaAvailable () { return _dma_available; }

        inline int mmap_copy (size_t peer, void * local_vaddr, void * remote_paddr, uint32_t size)
        {

          int rc = 0;                  // Testcase return code
          uint32_t actualvaddr;        // virtual address returned by the setProcessWindow indexed by target core
          uint64_t actualpaddr;        // physical address returned by setProcessWindow indexed by target core
          uint32_t actualsize;         // window size returned by setProcessWindow indexec by target core
          uint32_t paddr_src;          // physical address of the src buffer
          uint32_t varOffset;          // offset from the virtual address base
          uint32_t TestCasewindowSize; // Size of the window to be reqested in the call to setProcessWindow
          unsigned mytlb;              // TLB slot that we are going to modify

          TestCasewindowSize = WINDOW_SIZE;   // use a 1M window for the test size


          paddr_src = (uint32_t)remote_paddr;

          // setup three process windows in each core to every other core
          // initialize the first tlbslot to use
          //fprintf (stderr, "ShmemDmaDeviceBgpCnk::mmap_copy() .. _tlbslot[%d] = %d\n", peer, _tlbslot[peer]);
          mytlb = _tlbslot[peer];
          int rounds;

          uint32_t paddrRoundDown = paddr_src & ~(WINDOW_SIZE - 1);

          uint32_t bytes_copied_sofar = 0, bytes_to_copy = 0;
          uint32_t start_address;

          rounds = 0;

          while (bytes_copied_sofar < size)
            {
              //All the code for caching is disabled for now..
              //if (paddrRoundDown != last_phyaddr[peer]){
              //fprintf(stderr, "mmap_copy() .. mytlb = %d, paddrRoundDown = 0x%08x\n", mytlb, paddrRoundDown);
              rc = Kernel_SetProcessWindow(mytlb, paddrRoundDown, WINDOW_SIZE, PROT_READ | PROT_WRITE,
                                           &actualvaddr, &actualpaddr, &actualsize);

              if (rc)
                {
                  printf("Bad return code from Kernel_SetProcessWindow: %d\n", rc);
                  DCMF_abort();
                  return(-1);
                }

              DCMF_assert(actualsize == WINDOW_SIZE);

              //       last_phyaddr[peer] = paddrRoundDown;
              //       last_vaddr[peer] = actualvaddr;
              //}
              //else{
              //        actualvaddr = last_vaddr[peer];
              //}

              paddrRoundDown = (uint32_t)((char*)paddrRoundDown + (WINDOW_SIZE));

              if (rounds == 0)
                {
                  varOffset = (uint32_t)paddr_src & ((WINDOW_SIZE) - 1);
                  bytes_to_copy = (size < (WINDOW_SIZE - varOffset)) ? size : (WINDOW_SIZE - varOffset);
                  start_address = (uint32_t)((char*)actualvaddr + varOffset);
                }
              else
                {
                  varOffset = 0;
                  bytes_to_copy =
                    ((size - bytes_copied_sofar) < WINDOW_SIZE) ? (size - bytes_copied_sofar) : WINDOW_SIZE;
                  start_address = (uint32_t)actualvaddr;
                }

              //printf("copying to %8x from %8x for %d bytes\n", ((char*)src+bytes_copied_sofar), start_address, bytes_to_copy);
              memcpy((void*)((char*)local_vaddr + bytes_copied_sofar), (void*)start_address, bytes_to_copy);

              bytes_copied_sofar += bytes_to_copy;
              ++rounds;
            }

          return 0;
        }

      private:
        uint32_t _tlbslot[MAX_BGP_PROCS];
        bool     _dma_available;
    };
  };
};
#undef TRACE_ERR
#undef WINDOW_SIZE
#undef MAX_BGP_PROCS

#endif /* __dcmf_cdi_device_shmem_dma_bgp_cnk_h__ */

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
