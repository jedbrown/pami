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
/// \file components/devices/bgq/mu/InjFifoSubGroup.cc
///
/// \brief Injection Fifo SubGroup implementations.
///
////////////////////////////////////////////////////////////////////////////////

#include "components/devices/bgq/mu/InjFifoSubGroup.h"
#include "components/devices/bgq/mu/MUInjFifoMessage.h"

#include <spi/include/kernel/MU.h>
#include <spi/include/kernel/memory.h>

#ifdef TRACE
#undef TRACE
#endif
#define TRACE(x) fprintf x

////////////////////////////////////////////////////////////////////////////////
///
/// \brief Initialize the Injection Fifo Subgroup
///
/// \param[in]  subGroupId  The subgroup ID to be managed by this Injection
///                         Fifo Subgroup object (0..67).
/// \param[in]  numFifos    Number of injection fifos to be allocated.
/// \param[in]  fifoPtrs    Array of virtual address pointers to the injection
///                         fifos, one pointer for each fifo.  Each fifo must be
///                         64-byte aligned.
/// \param[in]  fifoSizes   Array of sizes for each fifo.
/// \param[in]  fifoAttrs   Array of attributes for each fifo.
///
/// \retval 0        Success.
/// \retval non-zero Unsuccessful.
///
////////////////////////////////////////////////////////////////////////////////

int32_t XMI::Device::MU::InjFifoSubGroup::
init ( uint32_t subGroupId,
       uint32_t numFifos,
       char    *fifoPtrs[],
       uint32_t fifoSizes[],
       Kernel_InjFifoAttributes_t fifoAttrs[] )
{
  int32_t rc;
  uint32_t numFreeFifos;
  uint32_t fifoNum;

  XMI_assert_debug( subGroupId < BGQ_MU_NUM_FIFO_SUBGROUPS_PER_NODE );
  XMI_assert_debug( numFifos <= BGQ_MU_NUM_INJ_FIFOS_PER_GROUP );

  //////////////////////////////////////////////////////////////////////////////
  ///
  /// Ask the kernel which injection fifo numbers are free to use.
  ///
  //////////////////////////////////////////////////////////////////////////////

  rc = Kernel_QueryInjFifos ( subGroupId,
                              &numFreeFifos,
                              _fifoNumbers );

  TRACE((stderr, "InjFifoSubGroup init(): Query Free returned rc=%d and %d free Fifos for group %d\n",
         rc,
         numFreeFifos,
         subGroupId ));

  XMI_assertf(rc == 0, "Kernel_QueryInjFifos failed (rc=%d)\n", rc);

  if ( numFreeFifos < numFifos )
    {
      TRACE((stderr, "XMI::Device::MU::InjFifoSubGroup:init() Requesting %d fifos, but only %d are free.\n",
             numFifos, numFreeFifos));
      return (-EBUSY); // Quit if not enough available.
    }

  //////////////////////////////////////////////////////////////////////////////
  //
  // Go reserve the requested fifos from the kernel.
  //
  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  // TODO fix parameters to Kernel_AllocateInjFifos call!
  rc = Kernel_AllocateInjFifos ( subGroupId,
                                 &_fifoSubGroup,
                                 numFifos,
                                 _fifoNumbers,
                                 fifoAttrs );
  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////


  TRACE((stderr, "InjFifoSubGroup init(): Allocate returned rc=%d for subgroup %d\n",
         rc,
         subGroupId ));

  XMI_assertf(rc == 0, "Kernel_AllocateInjFifos failed (rc=%d)\n", rc);

  //////////////////////////////////////////////////////////////////////////////
  //
  // Initialize the fifos in the group.
  //
  //////////////////////////////////////////////////////////////////////////////

  for ( fifoNum = 0; fifoNum < numFifos; fifoNum++ )
    {
      XMI_assert_debug( ( fifoSizes[fifoNum] & 0x3F ) == 0 ); // 64B aligned

      rc = Kernel_CreateMemoryRegion( &_injFifoMemRegion[fifoNum],
                                      (void*) fifoPtrs[fifoNum],
                                      (size_t) fifoSizes[fifoNum] );

      XMI_assertf(rc == 0, "Kernel_CreateMemoryRegion failed (rc=%d)\n", rc);

      TRACE((stderr, "   InjFifoSubGroup::init() .. before Kernel_InjFifoInit(%p, %d, %p, %ld, %d)\n",
                                &_fifoSubGroup,
                                _fifoNumbers[fifoNum],
                                &_injFifoMemRegion[fifoNum],
                                (uint64_t)fifoPtrs[fifoNum] -
                                (uint64_t)_injFifoMemRegion[fifoNum].BaseVa, // startoffset of start,head,tail relative to memRegion
                                fifoSizes[fifoNum] - 1));
      rc = Kernel_InjFifoInit ( &_fifoSubGroup,
                                _fifoNumbers[fifoNum],
                                &_injFifoMemRegion[fifoNum],
                                (uint64_t)fifoPtrs[fifoNum] -
                                (uint64_t)_injFifoMemRegion[fifoNum].BaseVa, // startoffset of start,head,tail relative to memRegion
                                fifoSizes[fifoNum] - 1 );

      TRACE((stderr, "   InjFifoSubGroup::init() InitById for subgroupPtr:%llx subgroup %d, logical fifo id=%d, MU fifo id=%d, start=%p, end=%p, size=%d, (startVA:%llx startPA:%llx) fifoPtr:%llx headPA:%llx tailPA:%llx headVA:%llx tailVA:%llx returned %d\n",
             (unsigned long long)(&_fifoSubGroup),
             /*subGroupId*/_fifoSubGroup.subgroupid,
             fifoNum,
             _fifoNumbers[fifoNum],
             fifoPtrs[fifoNum],
             (fifoPtrs[fifoNum] + fifoSizes[fifoNum]),
             fifoSizes[fifoNum],
             (unsigned long long)_injFifoMemRegion[fifoNum].BaseVa,
             (unsigned long long)_injFifoMemRegion[fifoNum].BasePa,
             (unsigned long long)(&_fifoSubGroup._injfifos[_fifoNumbers[fifoNum]]),
             (unsigned long long)_fifoSubGroup._injfifos[_fifoNumbers[fifoNum]]._fifo.hwfifo->pa_head,
             (unsigned long long)_fifoSubGroup._injfifos[_fifoNumbers[fifoNum]]._fifo.hwfifo->pa_tail,
             (unsigned long long)_fifoSubGroup._injfifos[_fifoNumbers[fifoNum]]._fifo.va_head,
             (unsigned long long)_fifoSubGroup._injfifos[_fifoNumbers[fifoNum]]._fifo.va_tail,
             rc));

      XMI_assertf(rc == 0, "Kernel_InjFifoInit failed (rc=%d)\n", rc);

      // ------------------------------------------------------------------------
      // Cache a pointer to the MUSPI_InjFifo_t structure associate with each
      // subgroup fifo number.
      TRACE((stderr, "   InjFifoSubGroup::init() .. before MUSPI_IdToInjFifo(_fifoNumbers[%d] = %d, %p)\n", fifoNum, _fifoNumbers[fifoNum], &_fifoSubGroup));
      _injectionfifo[fifoNum] = MUSPI_IdToInjFifo (_fifoNumbers[fifoNum], &_fifoSubGroup);
      TRACE((stderr, "   InjFifoSubGroup::init() ..  after MUSPI_IdToInjFifo(), _injectionfifo[%d] = %p\n", fifoNum, _injectionfifo[fifoNum]));

      // ------------------------------------------------------------------------
      // Save the virtual address of the single packet payload array.
      _singlePacketPayload[fifoNum].va =
        (torus_packet_payload_t *) (fifoPtrs[fifoNum] + fifoSizes[fifoNum]);

      // Calculate the offset of the single packet payload array from the base
      // virtual address of the memory region.
      uint64_t offset = (uint64_t)_singlePacketPayload[fifoNum].va -
                        (uint64_t)_injFifoMemRegion[fifoNum].BaseVa;

      // Calculate and save the physical address of the single packet payload array.
      _singlePacketPayload[fifoNum].pa =
        (torus_packet_payload_t *)((uint64_t)_injFifoMemRegion[fifoNum].BasePa + offset);

      // Save the length of the single packet payload array.
      // blocksom - This can probably be deduced from information elsewhere, but
      //            I don't know how to do that right now.
      _singlePacketPayload[fifoNum].count = fifoSizes[fifoNum] / sizeof(MUHWI_Descriptor_t);
      // ------------------------------------------------------------------------

      rc = Kernel_InjFifoActivate ( &_fifoSubGroup,
				    1,
                                    &_fifoNumbers[fifoNum],
				    KERNEL_INJ_FIFO_ACTIVATE );

      if ( rc != 0 ) return (rc); // Return if error.

    } // End: Initialize normal fifos.

  _numFifos = numFifos; // Remember this.

  return (0); // Success.

} // End: init()


////////////////////////////////////////////////////////////////////////////////
///
/// \brief Advance an Injection Fifo SubGroup
///
/// Fifos that have descriptors requiring "descriptor" notification
/// of completion are processed.  All descriptors that are done
/// will have their callback function invoked.
///
/// \retval  NumProcessed  Number of descriptors processed
///
////////////////////////////////////////////////////////////////////////////////

unsigned int XMI::Device::MU::InjFifoSubGroup::
advanceInternal ()
{
  unsigned long long waitForDoneMaskCopy = _waitForDoneMask; // Copy of the
  // _waitForDoneMask
  unsigned int numProcessed = 0; // Number of descriptors processed.
  int          fnum;             // Fifo number being processed.
  unsigned long long mask;       // The bit corresponding to the fifo we are
                                 // processing.
  MUDescriptorWrapper * desc;    // Descriptor wrapper being processed.

  //
  // Process all bits that are set in _waitForDoneMask...
  //
  for ( fnum = 0; waitForDoneMaskCopy != 0; fnum++ )
    {
      mask = _BN( fnum );     // Obtain a mask with only that bit set.

      if ( waitForDoneMaskCopy & mask ) // Waiting for descriptors in this fifo?
        {
          waitForDoneMaskCopy &= ~mask;   // Turn off the bit we are processing.

          TRACE((stderr, "InjFifoSubGroup::advance()  Processing fnum %d, Bit 0x%08llx, Bits left=0x%08llx\n",
                 fnum, mask, waitForDoneMaskCopy));

          // Check waitingForDone descriptors on this fifo's queue until one is not
          // done.

          while (1)
            {
              desc = _waitForDoneQ[fnum].headPtr; // Get first descriptor in Q.
              TRACE((stderr, "InjFifoSubGroup::advance()  desc = %p\n", desc));

              if ( desc == NULL ) // No more descriptors for this fifo?
                {
                  _waitForDoneMask &= ~mask;    // Indicate no more for this fifo.
                  break;                        // Go to next fifo.
                }

              if ( isDescriptorDone ( desc ) ) // Is this descriptor done?
                {
                  TRACE((stderr, "InjFifoSubGroup::advance()  desc %p is done (isCallbackDesired = %d)\n", desc, desc->isCallbackDesired()));
                  _waitForDoneQ[fnum].headPtr = (MUDescriptorWrapper *) desc->getNextPtr(); // Remove from Q

                  if (desc->requiresCallback()) desc->invokeCallback(); // Notify user.

                  // DON'T TOUCH THE DESCRIPTOR AFTER THIS.  The user may
                  // reuse its storage.

                  numProcessed++;
                }
              else
                {
                  TRACE((stderr, "InjFifoSubGroup::advance()  desc %p is NOT done\n", desc));
                  break; // This descriptor is not done.  No more to process
                  // for this fifo.
                }

            } // End: while(1) loop for this fifo.

        } // End: Process this fifo.

    } // End: Loop through fifos that have descriptors waiting.

  //
  // Process all bits that are set in _waitToSendMask...
  //
  MUInjFifoMessage * msg = NULL;
  unsigned long long waitToSendMaskCopy = _waitToSendMask;

  for ( fnum = 0; waitToSendMaskCopy != 0; fnum++ )
    {
      mask = _BN( fnum );     // Obtain a mask with only that bit set.

      if ( waitToSendMaskCopy & mask ) // Waiting to send on this fifo?
        {
          waitToSendMaskCopy &= ~mask;   // Turn off the bit we are processing.

          while (1)
            {
              msg = (MUInjFifoMessage *) _waitToSendQ[fnum].peekHead(); // Get first message

              if (msg == NULL) // No more messages waiting to send on this fifo?
                {
                  _waitToSendMask &= ~mask;    // Indicate no more for this fifo.
                  break;                       // Go to next fifo.
                }

              MUSPI_InjFifo_t    * injfifo;
              MUHWI_Descriptor_t * desc;
              void               * payloadVa;
              void               * payloadPa;

              if (nextDescriptor (fnum, &injfifo, &desc, &payloadVa, &payloadPa))
                {
                  // There is space available in this injection fifo.
                  if (msg->advance (injfifo, desc, payloadVa, payloadPa))
                    {
                      // This message has completed injecting descriptors.
                      _waitToSendQ[fnum].popHead();

                      if (msg->isCallbackDesired ())
                        {
                          addToDoneQ (fnum, msg->getWrapper());
                        }
                    }
                }
              else
                {
                  // There is no spacae available in this injection fifo. Therefore,
                  // the message is not done and there is nothing left to process for
                  // this injection fifo.
                  break;
                }
            }
        }
    }



  return numProcessed;

} // End: advanceInternal()
#undef TRACE
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
