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
              _completion_status (0),
              _channel0 (_sendqueue_status, _completion_status, 0),
              _channel1 (_sendqueue_status, _completion_status, 1),
              _channel2 (_sendqueue_status, _completion_status, 2),
              _channel3 (_sendqueue_status, _completion_status, 3),
              _channel4 (_sendqueue_status, _completion_status, 4),
              _channel5 (_sendqueue_status, _completion_status, 5),
              _channel6 (_sendqueue_status, _completion_status, 6),
              _channel7 (_sendqueue_status, _completion_status, 7),
              _channel8 (_sendqueue_status, _completion_status, 8),
              _channel9 (_sendqueue_status, _completion_status, 9)
          {
            channel[0] = &_channel0;
            channel[1] = &_channel1;
            channel[2] = &_channel2;
            channel[3] = &_channel3;
            channel[4] = &_channel4;
            channel[5] = &_channel5;
            channel[6] = &_channel6;
            channel[7] = &_channel7;
            channel[8] = &_channel8;
            channel[9] = &_channel9;
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
            PAMI_assert_debugf(fnum < 10, "%s<%d>\n", __FILE__, __LINE__);

            channel[fnum]->initialize (f, immediate_vaddr, immediate_paddr,
                                       completion_function, completion_cookie, n, channel_cookie);
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
            size_t events = 0;

            if (likely(_completion_status != 0))
              {
                events += _channel0.advanceCompletion ();
                events += _channel1.advanceCompletion ();
                events += _channel2.advanceCompletion ();
                events += _channel3.advanceCompletion ();
                events += _channel4.advanceCompletion ();
                events += _channel5.advanceCompletion ();
                events += _channel6.advanceCompletion ();
                events += _channel7.advanceCompletion ();
                events += _channel8.advanceCompletion ();
                events += _channel9.advanceCompletion ();
              }

            if (unlikely(_sendqueue_status != 0))
              {
                events += _channel0.advanceSendQueue ();
                events += _channel1.advanceSendQueue ();
                events += _channel2.advanceSendQueue ();
                events += _channel3.advanceSendQueue ();
                events += _channel4.advanceSendQueue ();
                events += _channel5.advanceSendQueue ();
                events += _channel6.advanceSendQueue ();
                events += _channel7.advanceSendQueue ();
                events += _channel8.advanceSendQueue ();
                events += _channel9.advanceSendQueue ();
              }

            return events;
          }

          /// Injection channel \b pointer array, \b not an array of injection channels
          InjChannel * channel[10];

        protected:

          uint64_t     _sendqueue_status;  // Send queue status, one bit for each channel
          uint64_t     _completion_status; // Completion status, one bit for each channel

          InjChannel   _channel0;
          InjChannel   _channel1;
          InjChannel   _channel2;
          InjChannel   _channel3;
          InjChannel   _channel4;
          InjChannel   _channel5;
          InjChannel   _channel6;
          InjChannel   _channel7;
          InjChannel   _channel8;
          InjChannel   _channel9;

      }; // class     PAMI::Device::MU::InjGroup
    };   // namespace PAMI::Device::MU
  };     // namespace PAMI::Device
};       // namespace PAMI

#endif // __components_devices_bgq_mu2_InjGroup_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//

