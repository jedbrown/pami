/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/bgq/BgqShaddr.h
 * \brief ???
 */

#ifndef __components_devices_shmem_bgq_BgqShaddr_h__
#define __components_devices_shmem_bgq_BgqShaddr_h__

#include <pami.h>

#ifndef TRACE_ERR
#define TRACE_ERR(x) // fprintf x
#endif

namespace PAMI
{
  namespace Device
  {
    namespace Shmem
    {
      class BgqShaddr
      {
        protected:

          inline  BgqShaddr () {};
          inline ~BgqShaddr () {};

        public:

          static const bool shaddr_va_supported    = false;
          static const bool shaddr_mr_supported    = true;

          static const bool shaddr_read_supported  = true;

          inline bool isEnabled_impl () { return true; };

          ///
          /// \brief Shared address read operation using virtual addresses
          ///
          /// \see ShaddrInterface::read
          ///
          inline void read_impl (void   * local,
                                 void   * remote,
                                 size_t   bytes,
                                 size_t   task);

          ///
          /// \brief Shared address read operation using memory regions
          ///
          /// \see ShaddrInterface::read
          ///
          inline void read_impl (Memregion * local,
                                 size_t      local_offset,
                                 Memregion * remote,
                                 size_t      remote_offset,
                                 size_t      bytes);

      };  // PAMI::Device::Shmem::BgqShaddr class
    };    // PAMI::Device::Shmem namespace
  };      // PAMI::Device namespace
};        // PAMI namespace

void PAMI::Device::Shmem::BgqShaddr::read_impl (void   * local,
                                                void   * remote,
                                                size_t   bytes,
                                                size_t   task)
{
  PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
};


void PAMI::Device::Shmem::BgqShaddr::read_impl (Memregion * local,
                                                size_t      local_offset,
                                                Memregion * remote,
                                                size_t      remote_offset,
                                                size_t      bytes)
{
  TRACE_ERR((stderr, ">> Shmem::BgqShaddr::read_impl()\n"));
  uint32_t rc = 0;

  void * local_paddr = (void *) (local->getBasePhysicalAddress() + local_offset);
  void * local_vaddr = NULL;
  rc = Kernel_Physical2Virtual (local_paddr, &local_vaddr);
  PAMI_assert_debugf(rc==0, "%s<%d> .. Kernel_Physical2Virtual(), rc = %d\n", __FILE__, __LINE__, rc);

  void * remote_paddr = (void *) (remote->getBasePhysicalAddress() + remote_offset);
  void * remote_vaddr = NULL;
  rc = Kernel_Physical2GlobalVirtual (remote_paddr, &remote_vaddr);
  PAMI_assert_debugf(rc==0, "%s<%d> .. Kernel_Physical2GlobalVirtual(), rc = %d\n", __FILE__, __LINE__, rc);


  TRACE_ERR((stderr, "   Shmem::BgqShaddr::read_impl(), local_vaddr = %p, remote_vaddr = %p\n", local_vaddr, remote_vaddr));

  memcpy (local_vaddr, remote_vaddr, bytes);

  TRACE_ERR((stderr, "<< Shmem::BgqShaddr::read_impl()\n"));
};

#undef TRACE_ERR
#endif /* __components_devices_shmem_bgq_BgqShaddr_h__ */

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
