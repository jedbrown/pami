/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

////////////////////////////////////////////////////////////////////////////////
///
/// \file BaseAddressTableSubGroup.cc
///
/// \brief Base Address Table SubGroup implementations.
///
////////////////////////////////////////////////////////////////////////////////


#include "components/devices/bgq/mu/BaseAddressTableSubGroup.h"

#include <spi/include/kernel/MU.h>

#ifndef TRACE
#define TRACE(x) //printf x
#endif


////////////////////////////////////////////////////////////////////////////////
///
/// \brief Initialize the Base Address Table SubGroup
///
/// \param[in]  subGroupId  The subgroup ID to be managed by this
///                         Base Address Table Subgroup object (0..67).
/// \param[in]  numEntries  Number of base address table entries to be
///                         allocated.
///
/// \retval  0         Successful initialization
/// \retval  non-zero  Value indicates the error
///
////////////////////////////////////////////////////////////////////////////////

int32_t XMI::Device::MU::BaseAddressTableSubGroup::
init ( uint32_t subGroupId,
       uint32_t numEntries )
{
  int32_t rc;
  uint32_t numFreeEntries;

  XMI_assert_debug( subGroupId < BGQ_MU_NUM_FIFO_SUBGROUPS_PER_NODE );
  XMI_assert_debug( numEntries <= BGQ_MU_NUM_DATA_COUNTERS_PER_SUBGROUP );

  //////////////////////////////////////////////////////////////////////////////
  ///
  /// Ask the kernel which base address table entry numbers are free to use.
  ///
  //////////////////////////////////////////////////////////////////////////////

  rc = Kernel_QueryBaseAddressTable ( subGroupId,
                                      _entryNumbers,
                                      &numFreeEntries );

  TRACE(("BatSubGroup init(): Query Free returned rc=%d and %u free entries for subgroup %u\n",
         rc,
         numFreeEntries,
         subGroupId ));

  if ( rc != 0 ) return (rc); // Quit if this call fails.

  if ( numFreeEntries < numEntries )
    {
      TRACE(("BatSubGroup:init() Requesting %u entries, but only %u are free.\n",
             numEntries, numFreeEntries));
      return (-EBUSY); // Quit if not enough available.
    }

  //////////////////////////////////////////////////////////////////////////////
  ///
  /// Go reserve the requested entries from the kernel.
  ///
  //////////////////////////////////////////////////////////////////////////////

  rc = Kernel_AllocateBaseAddressTable ( subGroupId,
                                         &_baseAddressTableSubGroup,
                                         _entryNumbers,
                                         numEntries );

  TRACE(("BatSubGroup init(): Allocate returned rc=%d for subgroup %u\n",
         rc,
         subGroupId ));

  if ( rc != 0 ) return (rc); // Quit if this call fails.

  _numEntries = numEntries; // Remember this.

  return (0); // Success.

} // End: init()
#undef TRACE

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
