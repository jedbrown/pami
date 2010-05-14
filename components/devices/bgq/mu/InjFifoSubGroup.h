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
/// \file components/devices/bgq/mu/InjFifoSubGroup.h
///
/// \brief Injection Fifo SubGroup definitions.
///
////////////////////////////////////////////////////////////////////////////////
#ifndef __components_devices_bgq_mu_InjFifoSubGroup_h__
#define __components_devices_bgq_mu_InjFifoSubGroup_h__

#include <errno.h>

#include <hwi/include/bqc/MU_Macros.h>
#include <hwi/include/bqc/MU_Descriptor.h>
#include <hwi/include/common/bgq_bitnumbers.h>
#include <spi/include/mu/InjFifo.h>
#include <spi/include/mu/DescriptorWrapperXX.h>
#include <spi/include/kernel/MU.h>

#include "components/devices/bgq/mu/MUDescriptorWrapper.h"

#include "util/queue/Queue.h"

#undef TRACE
#define TRACE(x) //fprintf x

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      typedef uint8_t torus_packet_payload_t[512];


      ////////////////////////////////////////////////////////////////////////////
      ///
      /// \brief Injection Fifo SubGroup Class
      ///
      /// This class provides interfaces for accessing an injection fifo subgroup.
      /// It assumes that MU initialization has previously occurred, including
      /// job initialization.
      ///
      /// The following interfaces are provided:
      ///
      /// Constructor              - Prepares the class object for use.
      /// init                     - Initializes the injection fifo subgroup.
      /// push(descriptor)         - Pushes a descriptor onto an injection fifo
      /// push(descriptor-wrapper) - Pushes a descriptor and sets-up/handles
      ///                            completion.
      /// dump                     - Dumps the injection fifo group.
      /// term                     - Terminates the injection fifo group.
      ///
      ////////////////////////////////////////////////////////////////////////////

      class InjFifoSubGroup
      {
        public:

          //////////////////////////////////////////////////////////////////////////
          ///
          /// \brief Injection Fifo SubGroup Constructor
          ///
          /// This constructor essentially does nothing but initialize
          /// member data.  The init() function must be called before
          /// this object can be usable.
          ///
          /// This is a default constructor.
          ///
          //////////////////////////////////////////////////////////////////////////

          InjFifoSubGroup ()
              :
              _numFifos             (0),
              _waitForDoneMask      (0),
              _waitToSendMask       (0)
          {
            for (int i = 0; i < BGQ_MU_NUM_INJ_FIFOS_PER_SUBGROUP; i++)
              {
                _waitForDoneQ[i].headPtr = NULL;
                _waitForDoneQ[i].tailPtr = NULL;
              }
          }


          //////////////////////////////////////////////////////////////////////////
          ///
          /// \brief Initialize the Injection Fifo Subgroup
          ///
          /// \param[in]  subGroupId  The subgroup ID to be managed by this
          ///                         Injection Fifo Subgroup object (0..67).
          /// \param[in]  numFifos    Number of injection fifos to be allocated.
          /// \param[in]  fifoPtrs    Array of virtual address pointers to the
          ///                         injection fifos, one pointer for each fifo.
          ///                         Each fifo must be 64-byte aligned.
          /// \param[in]  fifoSizes   Array of sizes for each fifo.
          /// \param[in]  fifoAttrs   Array of attributes for each fifo.
          ///
          /// \retval 0  Success.
          /// \retval -1 Unsuccessful.
          ///
          //////////////////////////////////////////////////////////////////////////

          int init ( uint32_t subGroupId,
                     uint32_t numFifos,
                     char    *fifoPtrs[],
                     uint32_t fifoSizes[],
                     Kernel_InjFifoAttributes_t fifoAttrs[] );


          //////////////////////////////////////////////////////////////////////////
          ///
          /// \brief Push a Descriptor onto the Specified Fifo
          ///
          /// \param[in]  fifoNum  Fifo number to push descriptor onto
          ///                      (0 through number of fifos being managed by this
          ///                      object).
          /// \param[in]  desc     Pointer to the Descriptor to be pushed onto the
          ///                      fifo.
          ///
          /// \retval  Not(-1)  Successful push.  The return value is the sequence
          ///                   number of the pushed descriptor in this fifo.
          /// \retval  -1 (0xFFF...FFF) Unsuccessful push...fifo is probably full.
          ///
          //////////////////////////////////////////////////////////////////////////

          inline uint64_t push ( int                  fifoNum,
                                 MUHWI_Descriptor_t * desc )
          {
            int MU_fifoid = _fifoNumbers[fifoNum]; // Map fifoNum to an MU fifo num
            TRACE((stderr, "InjFifoSubGroup::push() fifoNum %d, desc %p\n", fifoNum,  desc));
            MUSPI_InjFifo_t *fifoPtr = MUSPI_IdToInjFifo ( MU_fifoid,
                                                           &_fifoSubGroup );

            return MUSPI_InjFifoInject ( fifoPtr, desc );
          }


          //////////////////////////////////////////////////////////////////////////
          ///
          /// \brief Push a Descriptor onto the Specified Fifo
          ///
          /// This function pushes the descriptor onto the specified fifo.
          /// - If the push is successful,
          ///   - If a callback is desired, the descriptor is immediately checked
          ///     to see if it is complete.
          ///     - If it is not complete, the descriptor's sequence number is set
          ///       into the wrapper, and the wrapper is queued onto the
          ///       waitForDoneQueue.
          ///     - If it is complete, the wrapper callback is invoked.
          ///
          /// \param[in]  descWrapper  Pointer to a descriptor wrapper class.  The
          ///                          wrapper has the fifo number and descriptor
          ///                          pointer.
          ///
          /// \retval  Not(-1)  Successful push.  The return value is the sequence
          ///                   number of the pushed descriptor in this fifo.
          /// \retval  -1 (0xFFF...FFF) Unsuccessful push...fifo is probably full.
          ///
          //////////////////////////////////////////////////////////////////////////

          inline uint64_t push ( MUDescriptorWrapper &desc )
          {
            uint64_t sequenceNum;
            int fnum = desc.getFIFONum();

#if 0
            printf("push: wrapper=%p/%p\n", &desc,desc.getDescriptorPtr());
            desc.dump();
#endif

            sequenceNum = this->push ( fnum,
                                       desc.getDescriptorPtr() );

            if ( sequenceNum != 0xFFFFFFFFFFFFFFFFULL ) // Successful push?
              {
                //
                // If this descriptor requires "descriptor notification",
                // handle it.
                //
                if ( desc.requiresCallback() )
                  {
#ifndef OPTIMIZE_AGGREGATE_LATENCY
                    //
                    // Check if the descriptor is done.
                    //
                    int MU_fifoid =  _fifoNumbers[fnum];

                    TRACE((stderr, "InjFifoSubGroup::push() ifndef OPTIMIZE_AGGREGATE_LATENCY fifoNum %d\n", MU_fifoid));
                    MUSPI_InjFifo_t *fifoPtr = MUSPI_IdToInjFifo ( MU_fifoid,
                                                                   &_fifoSubGroup );

                    if ( MUSPI_CheckDescComplete ( fifoPtr,
                                                   sequenceNum ) )
                      {
                        desc.invokeCallback(); // Descriptor is done...notify.
                      }
                    else
#endif
                      {
                        //
                        // The descriptor is not done.
                        // Save the sequence number.
                        // Queue it.
                        // Set bit indicating this fifo has waiters.
                        //
                        desc.setSequenceNumber ( sequenceNum );

                        TRACE((stderr, "InjFifoSubGroup::push() not done fnum %d, desc %p/%p, desc fifonum %d\n", fnum,&desc,desc.getDescriptorPtr(), desc.getFIFONum()));

                        if ( _waitForDoneQ[fnum].headPtr == NULL )
                          _waitForDoneQ[fnum].headPtr = &desc;
                        else
                          _waitForDoneQ[fnum].tailPtr->setNextPtr(&desc);

                        _waitForDoneQ[fnum].tailPtr = &desc;

                        _waitForDoneMask |= _BN( fnum );
                      }

                  } // End: Callback is desired

              } // End: push successful

            return sequenceNum;
          }


          //////////////////////////////////////////////////////////////////////////
          ///
          /// \brief Advance an Injection Fifo SubGroup
          ///
          /// Fifos that have descriptors requiring "descriptor" notification
          /// of completion are processed.  All descriptors that are done
          /// will have their callback function invoked.
          ///
          /// \retval  NumProcessed  Number of descriptors processed
          ///
          //////////////////////////////////////////////////////////////////////////

          unsigned int advanceInternal ();


          //////////////////////////////////////////////////////////////////////////
          ///
          /// \brief Advance Injection Fifos
          ///
          /// This is an inline wrapper to quickly check if there is anything
          /// to advance.
          ///
          /// \retval  NumProcessed  The number of descriptors processed (that
          ///                        were done).
          ///
          //////////////////////////////////////////////////////////////////////////

          inline unsigned int advance()
          {
            //
            // If no fifos have descriptors queued, we are done.
            //
            if ( _waitForDoneMask == 0 ) return 0;

            //
            // There is something to process.  Call internal advance().
            //
            return ( this->advanceInternal() );

          } /// End: advance()


          //////////////////////////////////////////////////////////////////////////
          ///
          ///  \brief Check if the MU has finished processing this descriptor
          ///
          ///  \param[in]  desc  Pointer to the C++ descriptor class
          ///
          ///  \retval   true   DMA has processed the descriptor
          ///  \retval   false  The descriptor is still in the fifo
          ///
          //////////////////////////////////////////////////////////////////////////

          inline bool isDescriptorDone ( MUSPI_DescriptorWrapper *desc )
          {
            int fnum         =  desc->getFIFONum();
            int MU_fifoid    =  _fifoNumbers[fnum];

            TRACE((stderr, "InjFifoSubGroup::isDescriptorDone() fifoNum %d, desc %p\n", MU_fifoid,  desc));
            MUSPI_InjFifo_t *fifoPtr = MUSPI_IdToInjFifo ( MU_fifoid,
                                                           &_fifoSubGroup );

            return MUSPI_CheckDescComplete ( fifoPtr,
                                             desc->getSequenceNumber() );
          }


          void dump () {}

          void term () {}

          inline bool nextDescriptor (int                   subgroupFifoNumber,
                                      MUSPI_InjFifo_t    ** injfifo,
                                      MUHWI_Descriptor_t ** desc,
                                      void               ** payloadVa,
                                      void               ** payloadPa)
          {
            TRACE((stderr, ">> InjFifoSubGroup::nextDescriptor() .. _injectionfifo[%d] = %p, desc = %p\n", subgroupFifoNumber, _injectionfifo[subgroupFifoNumber], desc));
            TRACE((stderr, "   InjFifoSubGroup::nextDescriptor() .. *desc = %p\n", *desc));
            uint64_t sequenceNum =
              MUSPI_InjFifoNextDesc (_injectionfifo[subgroupFifoNumber], (void **) desc);

            TRACE((stderr, "   InjFifoSubGroup::nextDescriptor() .. sequenceNum = %ld, *desc = %p\n", sequenceNum, *desc));

            if (sequenceNum == 0xFFFFFFFFFFFFFFFFULL)
              {
                // Injection fifo is full. There is no "next" descriptor available
                // to initialize.
                TRACE((stderr, "<< InjFifoSubGroup::nextDescriptor() .. fifo full\n"));
                return false;
              }

            // Set the injection fifo output parameter.
            *injfifo = _injectionfifo[subgroupFifoNumber];

            TRACE((stderr, "   InjFifoSubGroup::nextDescriptor() .. _injectionfifo[%d] = %p, *injfifo = %p\n", subgroupFifoNumber, _injectionfifo[subgroupFifoNumber], *injfifo));

            // Locate the single packet payload buffer associated with this
            // descriptor and return the virtual and physical address.
            size_t index = sequenceNum % _singlePacketPayload[subgroupFifoNumber].count; // <---- this mod sucks.  Change it to a bitmask or something.
            *payloadVa = (void *) & _singlePacketPayload[subgroupFifoNumber].va[index];
            *payloadPa = (void *) & _singlePacketPayload[subgroupFifoNumber].pa[index];

            TRACE((stderr, "<< InjFifoSubGroup::nextDescriptor() .. return true\n"));
            return true;
          }

          inline void addToDoneQ (int                   subgroupFifoNumber,
                                  MUDescriptorWrapper * wrapper)
          {
            TRACE((stderr, "InjFifoSubGroup::addToDoneQ() fifonm %d/%d, desc %p/%p\n",subgroupFifoNumber, wrapper->getFIFONum(),  wrapper, wrapper->getDescriptorPtr()));
            wrapper->setFIFONum(subgroupFifoNumber); // not always set by caller, simple fix here...
            if ( _waitForDoneQ[subgroupFifoNumber].headPtr == NULL )
              _waitForDoneQ[subgroupFifoNumber].headPtr = wrapper;
            else
              _waitForDoneQ[subgroupFifoNumber].tailPtr->setNextPtr(wrapper);

            _waitForDoneQ[subgroupFifoNumber].tailPtr = wrapper;

            _waitForDoneMask |= _BN( subgroupFifoNumber );
          }

          inline void addToSendQ (int              subgroupFifoNumber,
                                  Queue::Element * msg)
          {
            _waitToSendQ[subgroupFifoNumber].pushTail (msg);
            _waitToSendMask |= _BN( subgroupFifoNumber );
          }

          //private:

          //////////////////////////////////////////////////////////////////////////
          ///
          /// Member data:
          ///
          /// _fifoSubGroup
          ///   The fifo subgroup structure.
          ///
          /// _numFifos
          ///   Number of fifos in this subgroup being used by this object.
          ///
          /// _fifoNumbers
          ///   The actual fifo numbers in this group, as known by the MU.
          ///   Users of this object use fifo numbers 0 through _numFifos-1.
          ///   This array maps from those fifo numbers to the actual MU
          ///   fifo numbers.
          ///
          /// _waitForDoneMask
          ///   A bit mask, one bit per injection fifo.
          ///     0 = Fifo does not have any descriptors on the _waitForDoneQ.
          ///     1 = Fifo has descriptors on the _waitForDoneQ.
          ///
          /// _waitForDoneQ
          ///   A queue of descriptors that are still in the fifo.  advance()
          ///   processes these descriptors when they have left the fifo.
          ///
          /// _injFifoMemRegion
          ///   An array of kernel memory regions used to register the memory for
          ///   each fifo before the memory is initialized for use by the MU. The
          ///   memory regions must not be destroyed, even though the destroy
          ///   operation is a noop on cnk, because the fifo memory must remain
          ///   pinned on other operating systems.
          ///
          /////////////////////////////////////////////////////////////////////////

          MUSPI_InjFifoSubGroup_t _fifoSubGroup;
          uint32_t                _numFifos;
          uint32_t                _fifoNumbers   [BGQ_MU_NUM_INJ_FIFOS_PER_SUBGROUP];
          unsigned long long      _waitForDoneMask;
          struct
          {
            MUDescriptorWrapper * headPtr;
            MUDescriptorWrapper * tailPtr;
          } _waitForDoneQ[BGQ_MU_NUM_INJ_FIFOS_PER_SUBGROUP];
          Kernel_MemoryRegion_t _injFifoMemRegion[BGQ_MU_NUM_INJ_FIFOS_PER_SUBGROUP];
          struct
          {
            torus_packet_payload_t * va;
            torus_packet_payload_t * pa;
            size_t                   count;
          } _singlePacketPayload[BGQ_MU_NUM_INJ_FIFOS_PER_SUBGROUP];

          unsigned long long _waitToSendMask;
          Queue              _waitToSendQ[BGQ_MU_NUM_INJ_FIFOS_PER_SUBGROUP];

          MUSPI_InjFifo_t  * _injectionfifo[BGQ_MU_NUM_INJ_FIFOS_PER_SUBGROUP];
      }; // PAMI::Device::MU::InjFifoSubGroup class
    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace
#undef TRACE
#endif   // __components_devices_bgq_mu_injfifosubgroup_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
