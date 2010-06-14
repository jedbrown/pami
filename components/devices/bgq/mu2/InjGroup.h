/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/InjGroup.h
 * \brief ???
 */

#ifndef __components_devices_bgq_mu2_InjGroup_h__
#define __components_devices_bgq_mu2_InjGroup_h__

#include <malloc.h>

#include <spi/include/mu/InjFifo.h>

#include "components/devices/bgq/mu2/InjChannel.h"

#include "components/devices/bgq/mu2/trace.h"
#define DO_TRACE_ENTEREXIT 0
#define DO_TRACE_DEBUG     0

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      ///
      /// \brief The injection group component contains several injection channels
      ///
      /// The InjGroup class constructs and provides an initialization method
      /// for each injection channel. This allows the injection group to
      /// correctly specify the status variable references, which allows
      /// the injection group to quickly determine if any injection channels
      /// in the injection group need to be advanced.
      ///
      /// \see MU::InjChannel
      ///
      /// The injection channel public interfaces are used by the MU::Context
      ///
      /// \see MU::Context
      ///
      class InjGroup
      {
        public:

          /// Immediate payload lookaside buffer type
          typedef InjChannel::immediate_payload_t immediate_payload_t;

          ///
          /// \brief Injection group constructor
          ///
          inline InjGroup () :
              _sendqueue_status (0),
              _completion_status (0)
          {
            TRACE_FN_ENTER();

            size_t i;
            for (i=0; i<10; i++)
              new (&channel[i]) InjChannel (_sendqueue_status, _completion_status, i);

            TRACE_FN_EXIT();
          };

          ///
          /// \brief Injection group destructor
          ///
          inline ~InjGroup () {};

          ///
          /// \brief Initialize an injection channel in this injection group
          ///
          /// \param[in] fnum                Injection channel "fifo number"
          /// \param[in] f                   Injection fifo for this fifo number
          /// \param[in] immediate_vaddr     Array of payload lookaside buffers
          /// \param[in] immediate_paddr     Physical address of the payload lookaside array
          /// \param[in] completion_function Array of completion functions
          /// \param[in] completion_cookie   Array of completion cookies
          /// \param[in] n                   Size of all input parameter arrays
          /// \param[in] channel_cookie      Injection channel cookie
          ///
          inline void initialize (size_t                 fnum,
                                  MUSPI_InjFifo_t      * f,
                                  immediate_payload_t  * immediate_vaddr,
                                  uint64_t               immediate_paddr,
                                  pami_event_function  * completion_function,
                                  void                ** completion_cookie,
                                  size_t                 n,
                                  void                 * channel_cookie)
          {
            TRACE_FN_ENTER();

            PAMI_assert_debugf(fnum < 10, "%s<%d>\n", __FILE__, __LINE__);

            channel[fnum].initialize (f, immediate_vaddr, immediate_paddr,
                                      completion_function, completion_cookie,
                                      n, channel_cookie);

            TRACE_FN_EXIT();
          }

          ///
          /// \brief Advance the injection channels in this injection group
          ///
          /// All injection channels are advanced, regardles of initialization,
          /// because the common case run modes will always initialize all of
          /// the injection channels. The uncommon run modes, such as 64- and
          /// 32-tasks/node, will attempt to advance the unitialized channels
          /// and will return immediately because the status bit for these
          /// channels will always be zero.
          ///
          /// This allows the common case run modes to eliminate a for-loop.
          ///
          inline size_t advance ()
          {
            TRACE_FN_ENTER();
            TRACE_FORMAT("_completion_status = %016lx, _sendqueue_status = %016lx", _completion_status, _sendqueue_status);

            size_t events = 0;

            if (likely(_completion_status != 0))
              {
                events += channel[0].advanceCompletion ();
                events += channel[1].advanceCompletion ();
                events += channel[2].advanceCompletion ();
                events += channel[3].advanceCompletion ();
                events += channel[4].advanceCompletion ();
                events += channel[5].advanceCompletion ();
                events += channel[6].advanceCompletion ();
                events += channel[7].advanceCompletion ();
                events += channel[8].advanceCompletion ();
                events += channel[9].advanceCompletion ();
              }

            if (unlikely(_sendqueue_status != 0))
              {
                events += channel[0].advanceSendQueue ();
                events += channel[1].advanceSendQueue ();
                events += channel[2].advanceSendQueue ();
                events += channel[3].advanceSendQueue ();
                events += channel[4].advanceSendQueue ();
                events += channel[5].advanceSendQueue ();
                events += channel[6].advanceSendQueue ();
                events += channel[7].advanceSendQueue ();
                events += channel[8].advanceSendQueue ();
                events += channel[9].advanceSendQueue ();
              }

            TRACE_FN_EXIT();
            return events;
          }

          /// Injection channel array
          InjChannel channel[10];

        protected:

          uint64_t   _sendqueue_status;  // Send queue status, one bit for each channel
          uint64_t   _completion_status; // Completion status, one bit for each channel

      }; // class     PAMI::Device::MU::InjGroup
    };   // namespace PAMI::Device::MU
  };     // namespace PAMI::Device
};       // namespace PAMI

#undef  DO_TRACE_ENTEREXIT
#undef  DO_TRACE_DEBUG

#endif // __components_devices_bgq_mu2_InjGroup_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//

