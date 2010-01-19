/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/DmaInterface.h
 * \brief ???
 */

#ifndef __components_devices_DmaInterface_h__
#define __components_devices_DmaInterface_h__

#include <errno.h>
#include "Memregion.h"
#include "sys/xmi.h"

namespace XMI
{
  namespace Device
  {
    namespace Interface
    {
      ///
      /// \todo Need A LOT MORE documentation on this interface and its use
      /// \param T_Model      Dma model template class
      /// \param T_Device     Dma device template class
      /// \param T_StateBytes     Dma object template class
      ///
      template <class T_Model, class T_Device, unsigned T_StateBytes>
      class DmaModel
      {
        public:
          ///
          /// \brief Base dma model constructor
          ///
          /// \param[in] device  dma device
          /// \param[in] context Communication context
          ///
          inline DmaModel (T_Device & device) {};
          inline ~DmaModel () {};

          ///
          /// \brief Post a contiguous dma put transfer operation
          ///
          /// The post dma put operation interface allows the dma model
          /// and dma device implementations to optimize for performance by
          /// avoiding the overhead to construct a dma message object. If
          /// the dma device has resources immediately available then the
          /// message may be directly posted, otherwise a dma message object
          /// may be constructed to maintain the send state until the callback
          /// is invoked.
          ///
          /// \attention All dma model interface derived classes \b must
          ///            implement the postDmaPut_impl() method.
          ///
          /// \param[in] state            Location to store the internal dma transfer state
          /// \param[in] local_fn         Callback to invoke when the operation is complete
          /// \param[in] cookie           Completion callback opaque application data
          /// \param[in] target_task      Global task identifier of the target
          /// \param[in] local_memregion  Local data memory region
          /// \param[in] local_offset     Offset of data buffer in the local memory region
          /// \param[in] remote_memregion Remote data memory region
          /// \param[in] remote_offset    Offset of data buffer in the remote memory region
          /// \param[in] bytes            Number of bytes to transfer
          ///
          /// \retval true  Dma operation completed and the completion
          ///               callback was invoked
          ///
          /// \retval false Dma operation did not complete and the dma
          ///               device must be advanced until the completion
          ///               callback is invoked
          ///
          inline bool postDmaPut (uint8_t              (&state)[T_StateBytes],
                                  xmi_event_function   local_fn,
                                  void               * cookie,
                                  size_t               target_task,
                                  Memregion          * local_memregion,
                                  size_t               local_offset,
                                  Memregion          * remote_memregion,
                                  size_t               remote_offset,
                                  size_t               bytes);

          ///
          /// \brief Post a contiguous dma get transfer operation
          ///
          /// The post dma get operation interface allows the dma model
          /// and dma device implementations to optimize for performance by
          /// avoiding the overhead to construct a dma message object. If
          /// the dma device has resources immediately available then the
          /// message may be directly posted, otherwise a dma message object
          /// may be constructed to maintain the send state until the callback
          /// is invoked.
          ///
          /// \attention All dma model interface derived classes \b must
          ///            implement the postDmaGet_impl() method.
          ///
          /// \param[in] state            Location to store the internal dma transfer state
          /// \param[in] local_fn         Callback to invoke when the operation is complete
          /// \param[in] target_task      Global task identifier of the target
          /// \param[in] cookie           Completion callback opaque application data
          /// \param[in] local_memregion  Local data memory region
          /// \param[in] local_offset     Offset of data buffer in the local memory region
          /// \param[in] remote_memregion Remote data memory region
          /// \param[in] remote_offset    Offset of data buffer in the remote memory region
          /// \param[in] bytes            Number of bytes to transfer
          ///
          /// \retval true  Dma operation completed and the completion
          ///               callback was invoked
          ///
          /// \retval false Dma operation did not complete and the dma
          ///               device must be advanced until the completion
          ///               callback is invoked
          ///
          inline bool postDmaGet (uint8_t              (&state)[T_StateBytes],
                                  xmi_event_function   local_fn,
                                  void               * cookie,
                                  size_t               target_rank,
                                  Memregion          * local_memregion,
                                  size_t               local_offset,
                                  Memregion          * remote_memregion,
                                  size_t               remote_offset,
                                  size_t               bytes);
      };

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      inline bool DmaModel<T_Model, T_Device, T_StateBytes>::postDmaPut (
          uint8_t              (&state)[T_StateBytes],
          xmi_event_function   local_fn,
          void               * cookie,
          size_t               target_task,
          Memregion          * local_memregion,
          size_t               local_offset,
          Memregion          * remote_memregion,
          size_t               remote_offset,
          size_t               bytes)
      {
        return static_cast<T_Model*>(this)->postDmaPut_impl (state,
                                                             local_fn,
                                                             cookie,
                                                             target_task,
                                                             local_memregion,
                                                             local_offset,
                                                             remote_memregion,
                                                             remote_offset,
                                                             bytes);
      }


      template <class T_Model, class T_Device, unsigned T_StateBytes>
      inline bool DmaModel<T_Model, T_Device, T_StateBytes>::postDmaGet (
          uint8_t              (&state)[T_StateBytes],
          xmi_event_function   local_fn,
          void               * cookie,
          size_t               target_task,
          Memregion          * local_memregion,
          size_t               local_offset,
          Memregion          * remote_memregion,
          size_t               remote_offset,
          size_t               bytes)
      {
        return static_cast<T_Model*>(this)->postDmaGet_impl (state,
                                                             local_fn,
                                                             cookie,
                                                             target_task,
                                                             local_memregion,
                                                             local_offset,
                                                             remote_memregion,
                                                             remote_offset,
                                                             bytes);
      }
    };
  };
};
#endif // __componenets_devices_DmaInterface_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
