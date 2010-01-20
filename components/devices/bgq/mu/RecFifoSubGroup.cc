/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

////////////////////////////////////////////////////////////////////////////////
///
/// \file components/devices/bgq/mu/RecFifoSubGroup.cc
///
/// \brief Reception Fifo SubGroup implementations.
///
////////////////////////////////////////////////////////////////////////////////

#include <spi/include/kernel/MU.h>
#include <spi/include/kernel/memory.h>

#include "components/devices/bgq/mu/RecFifoSubGroup.h"
#include "util/common.h"

#ifndef TRACE
#define TRACE(x) printf x
#endif

////////////////////////////////////////////////////////////////////////////////
///
/// \brief Initialize the Reception Fifo Subgroup
///
/// \param[in]  subGroupId  The subgroup ID to be managed by this Reception
///                         Fifo Subgroup object (0..67).
/// \param[in]  numFifos    Number of reception fifos to be allocated.
/// \param[in]  fifoPtrs    Array of virtual address pointers to the reception
///                         fifos, one pointer for each fifo.  Each fifo must be
///                         64-byte aligned.
/// \param[in]  fifoSizes   Array of sizes for each fifo.
/// \param[in]  fifoAttrs   Array of attributes for each fifo.
/// \param[in]  dispatch    MU device dispatch function table
///
/// \retval 0        Success.
/// \retval non-zero Unsuccessful.
///
////////////////////////////////////////////////////////////////////////////////

int32_t XMI::Device::MU::RecFifoSubGroup::
init ( uint32_t subGroupId,
       uint32_t numFifos,
       char    *fifoPtrs[],
       uint32_t fifoSizes[],
       Kernel_RecFifoAttributes_t fifoAttrs[],
       dispatch_t *dispatch )
{
  int32_t rc;
  uint32_t numFreeFifos;
  uint32_t fifoNum;
  Kernel_MemoryRegion_t  memRegion;
  uint32_t groupId = subGroupId / BGQ_MU_NUM_REC_FIFOS_PER_SUBGROUP;
  uint64_t enableBits = 0;

  XMI_assert_debug( subGroupId < BGQ_MU_NUM_FIFO_SUBGROUPS_PER_NODE );
  XMI_assert_debug( numFifos <= BGQ_MU_NUM_REC_FIFOS_PER_GROUP );

  TRACE(("RecFifoSubGrou::init(), dispatch = %p\n", dispatch));
  _dispatch = dispatch;
  //////////////////////////////////////////////////////////////////////////////
  ///
  /// Ask the kernel which reception fifo numbers are free to use.
  ///
  //////////////////////////////////////////////////////////////////////////////

  rc = Kernel_QueryRecFifos ( subGroupId,
                              &numFreeFifos,
                              _fifoNumbers );

  TRACE(("RecFifoSubGroup init(): Query Free returned rc=%d and %d free Fifos for group %d\n",
         rc,
         numFreeFifos,
         subGroupId ));

  if ( rc != 0 ) return (rc); // Quit if this call fails.

  if ( numFreeFifos < numFifos )
    {
      printf("XMI::Device::MU::RecFifoSubGroup:init() Requesting %d fifos, but only %d are free.\n",
             numFifos, numFreeFifos);
      return (-EBUSY); // Quit if not enough available.
    }

  //////////////////////////////////////////////////////////////////////////////
  ///
  /// Go reserve the requested fifos from the kernel.
  ///
  //////////////////////////////////////////////////////////////////////////////

  rc = Kernel_AllocateRecFifos ( subGroupId,
                                 &_fifoSubGroup,
                                 numFifos,
                                 _fifoNumbers,
                                 fifoAttrs );


  TRACE(("RecFifoSubGroup init(): Allocate returned rc=%d for subgroup %d\n",
         rc,
         subGroupId ));

  if ( rc != 0 ) return (rc); // Quit if this call fails.

  //////////////////////////////////////////////////////////////////////////////
  ///
  /// Initialize the fifos in the group.
  ///
  //////////////////////////////////////////////////////////////////////////////

  for ( fifoNum = 0; fifoNum < numFifos; fifoNum++ )
    {
      XMI_assert_debug( ( fifoSizes[fifoNum] & 0x1F ) == 0 ); // 32B aligned

      rc = Kernel_CreateMemoryRegion( &memRegion,
                                      (void*) fifoPtrs[fifoNum],
                                      (size_t) fifoSizes[fifoNum] );

      if ( rc != 0 ) return (rc);

      rc = Kernel_RecFifoInit ( &_fifoSubGroup,
                                _fifoNumbers[fifoNum],
                                &memRegion,
                                (uint64_t)fifoPtrs[fifoNum] -
                                (uint64_t)memRegion.BaseVa, // startoffset of start,head,tail relative to memRegion
                                fifoSizes[fifoNum] - 1 );

      TRACE(("RecFifoSubGroup init() InitById for subgroup %d, logical fifo id=%d, MU fifo id=%d, start=%p, end=%p, size=%d, returned %d\n",
             subGroupId,
             fifoNum,
             _fifoNumbers[fifoNum],
             fifoPtrs[fifoNum],
             (fifoPtrs[fifoNum] + fifoSizes[fifoNum]),
             fifoSizes[fifoNum],
             rc));

      if ( rc != 0 ) return (rc); // Return if error.

      // Set the enable bit for this reception fifo.
      enableBits |= _BN(48 +
                        ((subGroupId % BGQ_MU_NUM_FIFO_SUBGROUPS_PER_NODE) *
                         BGQ_MU_NUM_REC_FIFOS_PER_GROUP) +
                        _fifoNumbers[fifoNum]);

      rc = Kernel_DestroyMemoryRegion( &memRegion );

      if ( rc != 0 ) return (rc); // Return if error.

    } // End: Initialize normal fifos.

  // Enable these reception fifos.
  rc = Kernel_RecFifoEnable ( groupId,
                              enableBits);

  if ( rc != 0 ) return (rc);

  _numFifos   = numFifos;   // Remember this.
  _subGroupId = subGroupId; // Remember this.

  return (0); // Success.

} // End: init()

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
