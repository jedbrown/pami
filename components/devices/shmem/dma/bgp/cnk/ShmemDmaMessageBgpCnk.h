/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2008                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file devices/prod/cdi/shmem/dma/bgp/cnk/ShmemDmaMessageBgpCnk.h
 * \brief ???
 */

#ifndef __dcmf_cdi_shmem_dma_message_bgp_cnk_h__
#define __dcmf_cdi_shmem_dma_message_bgp_cnk_h__

#include "../../ShmemDmaMessage.h"
#include "../../../../bgp/cnk/DmaMemregionBgpCnk.h"
#include "sysdep/atomic/bgp/LockboxAtomicObject.h"

namespace DCMF
{
  namespace CDI
  {
    class ShmemDmaMessageBgpCnk : public ShmemDmaMessage<DmaMemregionBgpCnk, DCMF::LockboxAtomicObject<DCMF::LockManager::SHMEM_FIFO_0> >
    {
      public:
        inline ShmemDmaMessageBgpCnk (DCMF_Callback_t       & cb,
                                      ShmemFifoPacketHeader * model,
                                      size_t                  peer,
                                      size_t                  src_rank,
                                      DmaMemregionBgpCnk    * src_mr,
                                      DMA::offset_vec_t     * src_vec,
                                      unsigned                src_vec_count,
                                      size_t                  dst_rank,
                                      DmaMemregionBgpCnk    * dst_mr,
                                      DMA::offset_vec_t     * dst_vec,
                                      unsigned                dst_vec_count,
                                      DCMF_Callback           hook_cb,
                                      void                  * device,
                                      DCMF_Result           & status) :
            ShmemDmaMessage<DmaMemregionBgpCnk, DCMF::LockboxAtomicObject<DCMF::LockManager::SHMEM_FIFO_0> > (cb, model, peer, src_rank,
                                                 src_mr, src_vec, src_vec_count,
                                                 dst_rank, dst_mr, dst_vec,
                                                 dst_vec_count, hook_cb, device,
                                                 status)
        {
        };

        inline ~ShmemDmaMessageBgpCnk () {};


      protected:
        inline bool initializeDmaXferArray (dma_chunk_info_t * info, unsigned max, unsigned * num)
        {
          bool advance_again = false;
          unsigned chunk = 0;
          size_t src_offset, dst_offset;

          do
            {
              advance_again = next (src_offset, dst_offset, info[chunk].bytes);
              info[chunk].dst_addr = (void *)((char *)_src_mr->getBasePhysicalAddress() + src_offset);
              info[chunk].src_addr = (void *)((char *)_dst_mr->getBaseVirtualAddress() + dst_offset);
              printf("dst_addr(_phy_):%p, src_add(_vir_):%p\n", info[chunk].dst_addr, info[chunk].src_addr );
              chunk++;
            }
          while (advance_again && (chunk < max));

          *num = chunk;

          return advance_again;
        }
    };
  };
};
#endif /* __dcmf_cdi_shmem_dma_message_bgp_cnk_h__ */

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
