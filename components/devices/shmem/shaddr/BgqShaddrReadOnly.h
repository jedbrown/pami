/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/shaddr/BgqShaddrReadOnly.h
 * \brief ???
 */

#ifndef __components_devices_shmem_shaddr_BgqShaddrReadOnly_h__
#define __components_devices_shmem_shaddr_BgqShaddrReadOnly_h__

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
      class BgqShaddrReadOnly : public BgqShaddr, public ShaddrInterface<BgqShaddrReadOnly>
      {
        public:

          static const bool shaddr_write_supported = false;

          inline BgqShaddrReadOnly () :
              BgqShaddr (),
              ShaddrInterface<BgqShaddrReadOnly> ()
          {
          };

          inline ~BgqShaddrReadOnly () {};

          ///
          /// \brief Shared address write operation using virtual addresses
          ///
          /// \see ShaddrInterface::write
          ///
          inline size_t write_impl (void   * remote,
                                    void   * local,
                                    size_t   bytes,
                                    size_t   task)
          {
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
            return 0;
          }

          ///
          /// \brief Shared address write operation using memory regions
          ///
          /// \see ShaddrInterface::write
          ///
          inline size_t write_impl (Memregion * remote,
                                    size_t      remote_offset,
                                    Memregion * local,
                                    size_t      local_offset,
                                    size_t      bytes)
          {
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
            return 0;
          }
      };  // PAMI::Device::Shmem::BgqShaddrReadOnly class
    };    // PAMI::Device::Shmem namespace
  };      // PAMI::Device namespace
};        // PAMI namespace

#undef TRACE_ERR
#endif /* __components_devices_shmem_bgq_BgqShaddrReadOnly_h__ */

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
