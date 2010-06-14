/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/InjChannel.h
 * \brief ???
 */

#ifndef __components_devices_bgq_mu2_InjChannel_h__
#define __components_devices_bgq_mu2_InjChannel_h__

#include <spi/include/mu/InjFifo.h>

#include "components/devices/bgq/mu2/msg/MessageQueue.h"

#include "components/devices/bgq/mu2/trace.h"
#define DO_TRACE_ENTEREXIT 0
#define DO_TRACE_DEBUG     0

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {

      // Forward declaration of the InjGroup friend class
      class InjGroup;

      ///
      /// \brief The injection channel component encapsulates all resources
      ///        associated with an injection fifo.
      ///
      /// The InjChannel class may only be constructed by the InjGroup class.
      /// This allows the injection group to correctly specify the status
      /// variable references, which allows the injection group to quickley
      /// determine if any injection channels in the injection group need
      /// to be advanced/
      ///
      /// \see MU::InjGroup
      ///
      /// The injection channel public interfaces are used by the model and
      /// message classes to inject descriptors.
      ///
      class InjChannel
      {
        private:

          friend class InjGroup;

          /// \brief Immediate payload element for the lookaside array
          typedef MUHWI_Descriptor_t immediate_payload_t[2];

          /// \brief Number of bits to shift for a descriptor
          /// \todo Move this up ... MUSPI ?
          static const size_t BGQ_MU_DESCRIPTOR_SIZE_IN_POW2 = 6;

          inline InjChannel () :
              _sendq_status (_dummy_status),
              _completion_status (_dummy_status)
          {
          };

          ///
          /// \brief Injection channel constructor
          ///
          /// The injection channel is not active until it is initialized.
          /// An uninitialized injection channel will always result in a
          /// "empty advance" as the status bits for this channel identifier
          /// will always be off.
          ///
          /// \note The injection channel object does not allocate memory.
          ///
          /// \param[in] sendq_status      Reference to send queue status variable
          /// \param[in] completion_status Reference to completion status variable
          /// \param[in] id                Channel identifier used to update bit
          ///                              in status variables
          ///
          inline InjChannel (uint64_t & sendq_status,
                             uint64_t & completion_status,
                             size_t     id) :
              _sendq_status (sendq_status),
              _completion_status (completion_status),
              _channel_set_bit (1 << id),
              _channel_unset_bit (~_channel_set_bit),
              _channel_id (id),
              _ififo (NULL),
              _immediate_vaddr (NULL),
              _immediate_paddr (0),
              _completion_function (NULL),
              _completion_cookie (NULL),
              _completions_pending (0),
              _n (0)
          {
            TRACE_FN_ENTER();
            COMPILE_TIME_ASSERT(BGQ_MU_DESCRIPTOR_SIZE_IN_BYTES == (1 << BGQ_MU_DESCRIPTOR_SIZE_IN_POW2));
            TRACE_FN_EXIT();
          };

          ///
          /// \brief Injection channel destructor
          ///
          inline ~InjChannel () {};

          ///
          /// \brief Initialize the injection channel
          ///
          /// The injection channel class does not allocate memory for the MU
          /// injection fifo and other resources. An external entity allocates
          /// these resources and then provides them to the injection channel
          /// object with this initialization method.
          ///
          /// \param[in] f                   An initialized injection fifo
          /// \param[in] immediate_vaddr     Immediate payload lookaside array
          /// \param[in] immediate_paddr     Physical address of the immediate
          ///                                payload lookaside array
          /// \param[in] completion_function Completion function array
          /// \param[in] completion_cookie   Completion cookie array
          /// \param[in] n                   Number of element in \b all arrays
          /// \param[in] channel_cookie      Cookie delivered to all completion
          ///                                events invoked by this channel,
          ///                                a.k.a., "pami_context_t"
          ///
          inline void initialize (MUSPI_InjFifo_t      * f,
                                  immediate_payload_t  * immediate_vaddr,
                                  uint64_t               immediate_paddr,
                                  pami_event_function  * completion_function,
                                  void                ** completion_cookie,
                                  size_t                 n,
                                  void                 * channel_cookie)
          {
            TRACE_FN_ENTER();
            _channel_cookie = channel_cookie;

            _ififo = f;
            _immediate_vaddr = immediate_vaddr;
            _immediate_paddr = immediate_paddr;
            _completion_function = completion_function;
            _completion_cookie = completion_cookie;
            _n = n;
            TRACE_FN_EXIT();
          };

          ///
          /// \brief Advance the send queue and process the pending messages
          ///
          /// Send queue message processing completes when all messages
          /// complete and the queue is empty, or when a message at the head
          /// of the queue does not complete.
          ///
          /// The bit corresponding to this channel identifier in the
          /// "send queue status" variable is updated after processing the
          /// send queue. A bit value of \c 1 indicates that the send queue is
          /// not empty and requires additional processing, and a bit value of
          /// \c 0 indicates that the send queue is empty and does not require
          /// additional processing.
          ///
          /// The send queue status bit for this channel is set to \c 1 when
          /// a message is posted to the channel.
          ///
          /// \see PAMI::Device::MU::InjChannel::post()
          ///
          /// \return something
          ///
          inline size_t advanceSendQueue ()
          {
            TRACE_FN_ENTER();
            TRACE_FORMAT("_sendq_status = %016lx, _channel_set_bit = %016lx, _sendq_status & _channel_set_bit = %016lx", _sendq_status, _channel_set_bit, _sendq_status & _channel_set_bit);

            if (likely(_sendq.isEmpty()))
              {
                TRACE_FN_EXIT();
                return 0;
              }

            // Turn off send queue bit for this injection fifo channel
            _sendq_status &= _channel_unset_bit;

            // Advance the send queue and set/unset the bit for this injection
            // fifo channel
            _sendq_status |= (_sendq.advance() << _channel_id);

            TRACE_FORMAT("_sendq_status = %016lx", _sendq_status);
            TRACE_FN_EXIT();
            return 0; // huh?
          };

          ///
          /// \brief Process any pending injection descriptor completions
          ///
          /// The bit corresponding to this channel identifier in the injection
          /// group "completion status" variable is updated after processing
          /// the pending completion notifications. A bit value of \c 1
          /// indicates that there are additional pending completion notification
          /// to be processed due to injection descriptors that have not yet been
          /// processed by the MU hardware. A bit value of \c 0 indicates that
          /// there are no pending completion notifications at this time.
          ///
          /// The completion status bit for this channel is set to \c 1 when
          /// any completion notification is set for the channel.
          ///
          /// \todo Injection fifo head/tail updates to determine completion range
          ///
          /// \see PAMI::Device::MU::InjChannel::setInjectionDescriptorNotification()
          ///
          /// \return Number of completion functions invoked
          //
          inline size_t advanceCompletion ()
          {
            TRACE_FN_ENTER();
            size_t completion_count = 0;

            TRACE_FORMAT("_completion_status = %016lx, _channel_set_bit = %016lx, _completion_status & _channel_set_bit = %016lx", _completion_status, _channel_set_bit, _completion_status & _channel_set_bit);

            if (likely(_completions_pending != 0))
              {
                size_t start = 0; // get from mu spi somehow
                size_t count = 0; // get from mu spi somehow

                size_t i;

                for (i = 0; i < count; i++)
                  {
                    TRACE_FORMAT("_completion_function[%zu] = %p", start+i, _completion_function[start+i]);
                    if (likely(_completion_function[start+i] != NULL))
                      {
                        _completion_function[start+i] (_channel_cookie,
                                                       _completion_cookie[start+i],
                                                       PAMI_SUCCESS);
                        completion_count++;
                      }
                  }

                TRACE_FORMAT("_completions_pending %zu -> %zu", _completions_pending, _completions_pending - completion_count);

                // Update the count of active completion requests
                _completions_pending -= completion_count;

                // Turn off completion bit for this injection fifo channel
                _completion_status &= _channel_unset_bit;

                // Re-set the completion bit for this injection fifo channel
                _completion_status |= ((_completions_pending > 0) << _channel_id);
                TRACE_FORMAT("_completion_status = %016lx (%zu, %zu)", _completion_status, _completions_pending, _channel_id);
              }

            TRACE_FN_EXIT();
            return completion_count;
          };

          uint64_t               _dummy_status;        // Used only for array constructors
          uint64_t             & _sendq_status;        // The "send queue work" per channel status
          uint64_t             & _completion_status;   // The "completion work" per channel status
          uint64_t               _channel_set_bit;     // Bit setter mask for this channel
          uint64_t               _channel_unset_bit;   // Bit unsetter mask for this channel
          size_t                 _channel_id;          // a.k.a., MU::Context "fifo number"

          void                 * _channel_cookie;      // a.k.a., "pami_context_t"

          MessageQueue           _sendq;               // Pending send message queue
          MUSPI_InjFifo_t      * _ififo;               // MU injection fifo
          immediate_payload_t  * _immediate_vaddr;     // Virtual address of the payload array
          uint64_t               _immediate_paddr;     // Physical address of the payload array
          pami_event_function  * _completion_function; // Array of completion functions
          void                ** _completion_cookie;   // Array of completion cookies
          size_t                 _completions_pending; // Number of active completion requests
          size_t                 _n;                   // Number of elements in the arrays

        public:

          /// \brief The number of contiguous free descriptors after the tail of the injection fifo
          ///
          /// Reads the shadow value and, if zero, updates from the hardware
          ///
          /// \todo implement this!
          ///
          inline size_t getFreeDescriptorCount ()
          {
            TRACE_FN_ENTER();
            uint64_t freeSpace = MUSPI_getFreeSpaceFromShadow (_ififo);

            size_t head  = (size_t) MUSPI_getHeadVa ((MUSPI_Fifo_t *)_ififo);
            size_t tail  = (size_t) MUSPI_getTailVa ((MUSPI_Fifo_t *)_ififo);
            TRACE_FORMAT("head = %zu, tail = %zu, freeSpace = %ld, _n = %zu", head, tail, freeSpace, _n);

            if (((tail + freeSpace) % _n) == (head % _n))
              {
                // No free space wrap, empty fifo, or full fifo
                TRACE_FORMAT("free descriptor count = %ld", freeSpace);
                TRACE_FN_EXIT();
                return freeSpace;
              }

            size_t end   = (size_t) MUSPI_getEndVa ((MUSPI_Fifo_t *)_ififo);
            TRACE_FORMAT("free descriptor count = %zu (end = %zu)", end - tail, end);
            TRACE_FN_EXIT();
            return (end - tail) >> BGQ_MU_DESCRIPTOR_SIZE_IN_POW2;
          };

          ///
          /// \brief The number of contiguous free descriptors after the tail of the injection fifo
          ///
          /// Reads the free space shadow value and, if zero, updates from the hardware
          ///
          inline size_t getFreeDescriptorCountWithUpdate ()
          {
            TRACE_FN_ENTER();
            uint64_t freeSpace = MUSPI_getFreeSpaceFromShadow (_ififo);

            if (unlikely(freeSpace == 0))
              {
#ifdef ENABLE_MAMBO_WORKAROUNDS
                TRACE_STRING("mambo workaround!");
                _ififo->freeSpace = _n; // mambo doesn't implement the MUSPI correctly
#else
                _ififo->freeSpace = MUSPI_getHwFreeSpace (_ififo);
#endif
              }

            size_t n = getFreeDescriptorCount ();
            TRACE_FORMAT("free descriptor count = %zu", n);
            TRACE_FN_EXIT();
            return n;
          };

          ///
          /// \brief The next free descriptor in the injection fifo
          ///
          /// \warning Does not check if the fifo is full
          ///
          /// \see getFreeDescriptorCount
          /// \see getFreeDescriptorCountWithUpdate
          ///
          inline MUHWI_Descriptor_t * getNextDescriptor ()
          {
            TRACE_FN_ENTER();
            MUHWI_Descriptor_t * desc = (MUHWI_Descriptor_t *) MUSPI_getTailVa((MUSPI_Fifo_t *)_ififo);
            TRACE_FORMAT("desc = %p", desc);
            TRACE_FN_EXIT();
            return desc;
          };

          ///
          /// \brief Retreive the immediate payload information associated
          ///        with a descriptor in an injection fifo
          ///
          /// \param[in] desc Descriptor in an injection fifo
          /// \param[out] vaddr Immediate payload virtual address
          /// \param[out] paddr Immediate payload physical address
          ///
          inline void getDescriptorPayload (MUHWI_Descriptor_t * desc, void * & vaddr, uint64_t & paddr)
          {
            TRACE_FN_ENTER();
            size_t start = (size_t) MUSPI_getStartVa ((MUSPI_Fifo_t *)_ififo);
            size_t index = ((size_t) desc - start) >> BGQ_MU_DESCRIPTOR_SIZE_IN_POW2;
            size_t offset = index * sizeof(immediate_payload_t);

            vaddr = (void *) & _immediate_vaddr[index];
            paddr = (uint64_t) (_immediate_paddr + offset);

            TRACE_FORMAT("desc = %p, vaddr = %p, paddr = %p", desc, vaddr, (void *)paddr);
            TRACE_FN_EXIT();
            return;
          };

          ///
          /// \brief Advance the MU injection fifo tail pointer.
          ///
          /// This completes a descriptor injection operation.
          ///
          /// \see checkDescComplete
          /// \return sequence number of injected descriptor
          ///
          inline uint64_t injFifoAdvanceDesc ()
          {
            TRACE_FN_ENTER();
            uint64_t sequence = MUSPI_InjFifoAdvanceDesc (_ififo);
            TRACE_FORMAT("sequence = %zu", sequence);
            TRACE_FN_EXIT();
            return sequence;
          };

          ///
          /// \brief Check for descriptor completion
          ///
          /// \see injFifoAdvanceDesc
          /// \param[in] sequence Descriptor sequence number
          ///
          /// \retval true  Descriptor has completed
          /// \retval false Descriptor has not completed
          ///
          inline bool checkDescComplete (uint64_t sequence)
          {
            TRACE_FN_ENTER();
            uint32_t rc = MUSPI_CheckDescComplete (_ififo, sequence);
            TRACE_FORMAT("descriptor %ld complete = %d", sequence, (rc == 1));
            TRACE_FN_EXIT();
            return (rc == 1);
          }

          ///
          /// \brief Set an event for an injection fifo descriptor index
          ///
          /// Updates the injection group completion status bit for the channel
          ///
          /// \param[in] fn       Completion event function
          /// \param[in] cookie   Completion event cookie
          /// \param[in] desc     Descriptor to monitor for completion
          ///
          inline void setInjectionDescriptorNotification (pami_event_function    fn,
                                                          void                 * cookie,
                                                          MUSPI_DescriptorBase * desc)
          {
            TRACE_FN_ENTER();
            size_t offset = (size_t) desc - (size_t) MUSPI_getStartVa ((MUSPI_Fifo_t *)_ififo);
            size_t index = offset >> BGQ_MU_DESCRIPTOR_SIZE_IN_POW2;

            TRACE_FORMAT("desc = %p, MUSPI_getStartVa() = %p, offset = %zu, index = %zu", desc, MUSPI_getStartVa ((MUSPI_Fifo_t *)_ififo), offset, index);

            _completion_function[index] = fn;
            _completion_cookie[index]   = cookie;

            TRACE_FORMAT("_completions_pending = %zu, _completion_status = %016lx, _channel_set_bit = %016lx", _completions_pending, _completion_status, _channel_set_bit);
            _completions_pending++;

            // Turn on the completion status bit for this fifo
            _completion_status |= _channel_set_bit;
            TRACE_FORMAT("_completions_pending = %zu, _completion_status = %016lx", _completions_pending, _completion_status);
            TRACE_FN_EXIT();
          };


          ///
          /// \brief Send queue empty status
          ///
          /// \retval true  Send queue is empty
          /// \retval false Send queue is not empty
          ///
          inline bool isSendQueueEmpty ()
          {
            TRACE_FN_ENTER();
            bool empty = _sendq.isEmpty();
            TRACE_FORMAT("_sendq.isEmpty() = %d", empty);
            TRACE_FN_EXIT();
            return empty;
          }

          ///
          /// \brief Post a message to the channel to be processed later
          ///
          /// Updates the injection group send queue status bit for the channel
          ///
          /// \see advanceSendQueue
          ///
          /// \param[in] msg Message queue element
          ///
          inline void post (MessageQueue::Element * msg)
          {
            TRACE_FN_ENTER();
            TRACE_FORMAT("_sendq_status = 0x%016lx, _channel_set_bit = 0x%016lx", _sendq_status, _channel_set_bit);

            // Turn on the send queue status bit for this fifo
            _sendq_status |= _channel_set_bit;

            TRACE_FORMAT("_sendq_status = 0x%016lx", _sendq_status);

            // Post the message to the send queue .. does not advance
            _sendq.post (msg);
            TRACE_FN_EXIT();
          };
      }; // class     PAMI::Device::MU::InjChannel
    };   // namespace PAMI::Device::MU
  };     // namespace PAMI::Device
};       // namespace PAMI

#undef  DO_TRACE_ENTEREXIT
#undef  DO_TRACE_DEBUG

#endif // __components_devices_bgq_mu2_InjChannel_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//

