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
/// \file components/devices/bgq/mu/RecFifoSubGroup.h
///
/// \brief Reception Fifo SubGroup definitions.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef __components_devices_bgq_mu_recfifosubgroup_h__
#define __components_devices_bgq_mu_recfifosubgroup_h__

#include <errno.h>

#include <hwi/include/bqc/MU_Macros.h>
#include <spi/include/mu/RecFifo.h>
#include <spi/include/kernel/MU.h>

#include "components/devices/bgq/mu/Dispatch.h"

#ifdef TRACE
#undef TRACE
#endif
#define TRACE(x) //fprintf x

namespace XMI
{
  namespace Device
  {
    namespace MU
    {
      //////////////////////////////////////////////////////////////////////////
      ///
      /// \brief Reception Fifo SubGroup Class
      ///
      /// This class provides interfaces for accessing a reception fifo subgroup.
      /// It assumes that MU initialization has previously occurred, including
      /// job initialization.
      ///
      /// The following interfaces are provided:
      ///
      /// Constructor              - Prepares the class object for use.
      /// init                     - Initializes the reception fifo subgroup.
      /// registerDispatchFunction - Registers a function to be dispatched to
      ///                            handle packets having a specific
      ///                            registration ID.
      /// advance                  - Polls the fifos in the subgroup and
      ///                            dispatches functions to handle the packets
      ///                            received.
      /// term                     - Terminates the reception fifo group.
      ///
      //////////////////////////////////////////////////////////////////////////

      class RecFifoSubGroup
      {
        public:

          ////////////////////////////////////////////////////////////////////////
          ///
          /// \brief Construct a reception FIFO subgroup
          ///
          /// This constructor essentially does nothing but initialize
          /// member data.  The init() function must be called before
          /// this object can be usable.
          ///
          ////////////////////////////////////////////////////////////////////////

          RecFifoSubGroup ():
              _subGroupId ( 0 )
          { }

          ////////////////////////////////////////////////////////////////////////
          ///
          /// \brief Initialize the Reception Fifo SubGroup
          ///
          /// \param[in]  subGroupId  The subgroup ID to be managed by this
          ///                         Reception Fifo Subgroup object (0..67).
          /// \param[in]  numFifos    Number of reception fifos to be allocated.
          /// \param[in]  fifoPtrs    Array of virtual address pointers to the
          ///                         reception fifos, one pointer for each fifo.
          ///                         Each fifo must be 32-byte aligned.
          /// \param[in]  fifoSizes   Array of sizes for each fifo.
          /// \param[in]  fifoAttrs   Array of attributes for each fifo.
          /// \param[in]  dispatch    MU device dispatch function table
          ///
          /// \retval  0         Successful initialization
          /// \retval  non-zero  Value indicates the error
          ///
          ////////////////////////////////////////////////////////////////////////

          int init ( uint32_t    subGroupId,
                     uint32_t    numFifos,
                     char       *fifoPtrs[],
                     uint32_t    fifoSizes[],
                     Kernel_RecFifoAttributes_t fifoAttrs[],
                     dispatch_t *dispatch );


          ////////////////////////////////////////////////////////////////////////
          ///
          /// \brief Advance Reception Fifos
          ///
          /// This is an inline wrapper to quickly check if there is anything
          /// to advance.
          ///
          /// \retval  NumProcessed  The number of packets processed.
          ///
          ////////////////////////////////////////////////////////////////////////

          inline unsigned int advance ()
          {
            unsigned int numProcessed = 0;


            TRACE((stderr, ">> RecFifoSubGroup::advance() .. _numFifos = %d\n", _numFifos));

            for ( uint32_t fifoNum = 0; fifoNum < _numFifos; fifoNum++ )
              {
                MUSPI_RecFifo_t *fifoPtr = MUSPI_IdToRecFifo ( _fifoNumbers[fifoNum],
                                                               &_fifoSubGroup );
                TRACE((stderr, "   RecFifoSubGroup::advance() .. fifoPtr = %p, _fifoNumbers[%d] = %d, &_fifoSubGroup = %p\n", fifoPtr, fifoNum, _fifoNumbers[fifoNum], &_fifoSubGroup));
                numProcessed += recFifoPoll ( fifoPtr );
              }

            TRACE((stderr, "<< RecFifoSubGroup::advance() .. numProcessed = %d\n", numProcessed));
            return numProcessed;
          }


          inline unsigned int recFifoPoll (MUSPI_RecFifo_t       * rfifo)
          {
            uint32_t wrap = 0;
            uint32_t cur_bytes = 0;
            uint32_t total_bytes = 0;
            uint32_t cumulative_bytes = 0;
            //MUHWI_PacketHeader_t *hdr = 0;
            MemoryFifoPacketHeader_t *hdr = NULL;
            unsigned packets = 0;

            TRACE((stderr, ">> RecFifoSubGroup::recFifoPoll(%p)\n", rfifo));
            while ((total_bytes = MUSPI_getAvailableBytes (rfifo, &wrap)) != 0)
              {
                TRACE((stderr, "   RecFifoSubGroup::recFifoPoll(%p) .. wrap = %d\n", rfifo, wrap));
                if (wrap)   //Extra branch over older packet loop
                  {
                    hdr = (MemoryFifoPacketHeader_t *) MUSPI_getNextPacketWrap (rfifo, &cur_bytes);

                    void * metadata = hdr->dev.singlepkt.metadata;

                    if (!hdr->dev.issingle)
                      metadata = (void *) hdr->dev.multipkt.metadata;

                    uint8_t id = hdr->dev.dispatch_id;
                    //fprintf (stderr, "recFifoPoll(wrap)    packet = %p, id = %d, cur_bytes = %d\n", hdr, id, cur_bytes);
                    _dispatch[id].f(metadata, hdr + 1, cur_bytes - 32, _dispatch[id].p);
                    packets++;

                    MUSPI_syncRecFifoHwHead (rfifo);
                  }
                else
                  {
                    cumulative_bytes = 0;

                    while (cumulative_bytes < total_bytes )
                      {
                        TRACE((stderr, "recFifoPoll(no-wrap) .. before MUSPI_getNextPacketOptimized() .. va_start = %p, va_head = %p, va_tail = %p, va_end = %p\n", rfifo->_fifo.va_start, rfifo->_fifo.va_head, rfifo->_fifo.va_tail, rfifo->_fifo.va_end));
                        hdr = (MemoryFifoPacketHeader_t *) MUSPI_getNextPacketOptimized (rfifo, &cur_bytes);
                        TRACE((stderr, "recFifoPoll(no-wrap) ..  after MUSPI_getNextPacketOptimized() .. va_start = %p, va_head = %p, va_tail = %p, va_end = %p\n", rfifo->_fifo.va_start, rfifo->_fifo.va_head, rfifo->_fifo.va_tail, rfifo->_fifo.va_end));
#ifdef TRACE
                        uint32_t * p = (uint32_t *) hdr;
                        TRACE((stderr, "recFifoPoll(no-wrap) ..  after MUSPI_getNextPacketOptimized() .. packet = %p, cur_bytes = %d, dump header: 0x%08x 0x%08x 0x%08x 0x%08x  0x%08x 0x%08x 0x%08x 0x%08x\n", hdr, cur_bytes, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]));
#endif
                        void * metadata = hdr->dev.singlepkt.metadata;

                        if (!hdr->dev.issingle)
                          metadata = (void *) hdr->dev.multipkt.metadata;

                        uint8_t id = hdr->dev.dispatch_id;

                        TRACE((stderr, "recFifoPoll(no-wrap) packet = %p, id = %d, cur_bytes = %d\n", hdr, id, cur_bytes));

                        TRACE((stderr, "recFifoPoll(no-wrap) _dispatch = %p, _dispatch[%d].f = %p\n", _dispatch, id, _dispatch[id].f));
                        _dispatch[id].f(metadata, hdr + 1, cur_bytes - 32, _dispatch[id].p);

                        cumulative_bytes += cur_bytes;

                        MUSPI_syncRecFifoHwHead (rfifo);
                        packets++;

                        // Touch head for next packet
                      }
                  }
              }

            TRACE((stderr, "<< RecFifoSubGroup::recFifoPoll(%p) .. packets = %d\n", rfifo, packets));
            return packets;
          }


          ////////////////////////////////////////////////////////////////////////
          ///
          ///  \brief  Get the Real Fifo Number
          ///
          ///  \param  fnum    The fifo number whose real fifo number
          ///                  is to be returned.
          ///
          ///  \retval realFnum  The real fifo number corresponding
          ///                    to the logical fifo number passed
          ///                    in.
          ///
          ////////////////////////////////////////////////////////////////////////

          inline int getRealFifoNumber (int fnum)
          {
            return ( _fifoNumbers[fnum] );
          }


          void term () {}


        private:

          ////////////////////////////////////////////////////////////////////////
          ///
          /// Private member data:
          ///
          /// _fifoSubGroup
          ///   The fifo subgroup structure.
          ///
          /// _subGroupId
          ///   The Id of the subgroup (0..67).
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
          ////////////////////////////////////////////////////////////////////////

          MUSPI_RecFifoSubGroup_t _fifoSubGroup;
          uint32_t                _subGroupId;
          uint32_t                _numFifos;
          uint32_t                _fifoNumbers[BGQ_MU_NUM_REC_FIFOS_PER_SUBGROUP];

          dispatch_t              * _dispatch; ///< MU device dispatch function table

        public:
          /// Set the default number of packets per advance/fifo to be large
          /// enough to process a full fifo, but quit after a reasonable
          /// number of packets to avoid deadlock.
          const static unsigned DEFAULT_NUM_PACKETS_PER_ADVANCE = 32768;

      }; // XMI::Device::MU::RecFifoSubGroup class
    };   // XMI::Device::MU namespace
  };     // XMI::Device namespace
};       // XMI namespace
#undef TRACE
#endif   // __components_devices_bgq_mu_recfifosubgroup_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
