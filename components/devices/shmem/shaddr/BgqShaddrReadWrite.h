/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/bgq/BgqShaddrReadWrite.h
 * \brief ???
 */

#ifndef __components_devices_shmem_bgq_BgqShaddrReadWrite_h__
#define __components_devices_shmem_bgq_BgqShaddrReadWrite_h__

#include "components/devices/shmem/shaddr/BgqShaddr.h"
#include "components/devices/shmem/shaddr/ShaddrInterface.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) // fprintf x
#endif

namespace PAMI
{
  namespace Device
  {
    namespace Shmem
    {
      class BgqShaddrReadWrite : public BgqShaddr, public ShaddrInterface<BgqShaddrReadWrite>
      {
        public:

          static const bool shaddr_write_supported = true;

          inline BgqShaddrReadWrite () :
              BgqShaddr (),
              ShaddrInterface<BgqShaddrReadWrite> ()
          {
          };

          inline ~BgqShaddrReadWrite () {};

          ///
          /// \brief Shared address write operation using virtual addresses
          ///
          /// \see ShaddrInterface::write
          ///
          inline void write_impl (void   * remote,
                                  void   * local,
                                  size_t   bytes,
                                  size_t   task);

          ///
          /// \brief Shared address write operation using memory regions
          ///
          /// \see ShaddrInterface::write
          ///
          inline void write_impl (Memregion * remote,
                                  size_t      remote_offset,
                                  Memregion * local,
                                  size_t      local_offset,
                                  size_t      bytes);
      };  // PAMI::Device::Shmem::BgqShaddrReadWrite class
    };    // PAMI::Device::Shmem namespace
  };      // PAMI::Device namespace
};        // PAMI namespace


void PAMI::Device::Shmem::BgqShaddrReadWrite::write_impl (void   * remote,
                                                 void   * local,
                                                 size_t   bytes,
                                                 size_t   task)
{
  PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
};


void PAMI::Device::Shmem::BgqShaddrReadWrite::write_impl (Memregion * remote,
                                                 size_t      remote_offset,
                                                 Memregion * local,
                                                 size_t      local_offset,
                                                 size_t      bytes)
{
  TRACE_ERR((stderr, ">> Shmem::BgqShaddrReadWrite::write_impl()\n"));
  uint32_t rc = 0;

  void * local_paddr = (void *) (local->getBasePhysicalAddress() + local_offset);
  void * local_vaddr = NULL;
  rc = Kernel_Physical2Virtual (local_paddr, &local_vaddr);
  PAMI_assert_debugf(rc==0, "%s<%d> .. Kernel_Physical2Virtual(), rc = %d\n", __FILE__, __LINE__, rc);

  void * remote_paddr = (void *) (remote->getBasePhysicalAddress() + remote_offset);
  void * remote_vaddr = NULL;
  rc = Kernel_Physical2GlobalVirtual (remote_paddr, &remote_vaddr);
  PAMI_assert_debugf(rc==0, "%s<%d> .. Kernel_Physical2GlobalVirtual(), rc = %d\n", __FILE__, __LINE__, rc);


  TRACE_ERR((stderr, "   Shmem::BgqShaddrReadWrite::write_impl(), local_vaddr = %p, remote_vaddr = %p\n", local_vaddr, remote_vaddr));

  memcpy (remote_vaddr, local_vaddr, bytes);

  TRACE_ERR((stderr, "<< Shmem::BgqShaddrReadWrite::write_impl()\n"));
};

#undef TRACE_ERR
#endif /* __components_devices_shmem_bgq_BgqShaddrReadWrite_h__ */

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
