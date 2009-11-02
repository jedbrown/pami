/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/DmaModel.h
 * \brief ???
 */

#ifndef __componenets_devices_dmamodel_h__
#define __componenets_devices_dmamodel_h__

#include <errno.h>

#include "sys/xmi.h"


namespace XMI
{
  namespace Device
  {
    namespace myInterface
    {
      ///
      /// \todo Need A LOT MORE documentation on this interface and its use
      /// \param T_Model      Dma model template class
      /// \param T_Device     Dma device template class
      /// \param T_Memregion  Memory region template class
      /// \param T_StateBytes     Dma object template class
      ///
      template <class T_Model, class T_Device, class T_Memregion, unsigned T_StateBytes>
      class DmaModel
      {
        public:
          ///
          /// \brief Base dma model constructor
          ///
          /// \param[in] device  dma device
          /// \param[in] context Communication context
          ///
          inline DmaModel (T_Device & device, xmi_context_t context) {};
          inline ~DmaModel () {};

          // bool init (size_t origin_rank);

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
          /// \attention All CDI dma model derived classes \b must
          ///            implement the postDmaPut_impl() method.
          ///
          /// \param[in] obj              Location to store the dma transfer object
          /// \param[in] cb               Callback to invoke when the operation is complete
          /// \param[in] remote_rank      Global rank of the target
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
                                  void   	         * cookie,
                                  size_t            target_rank,
                                  T_Memregion     * local_memregion,
                                  size_t            local_offset,
                                  T_Memregion     * remote_memregion,
                                  size_t            remote_offset,
                                  size_t            bytes);

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
          /// \attention All CDI dma model derived classes \b must
          ///            implement the postDmaGet_impl() method.
          ///
          /// \param[in] obj              Location to store the dma transfer object
          /// \param[in] cb               Callback to invoke when the operation is complete
          /// \param[in] remote_rank      Global rank of the target
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
                                  void   	         * cookie,
                                  size_t            target_rank,
                                  T_Memregion     * local_memregion,
                                  size_t            local_offset,
                                  T_Memregion     * remote_memregion,
                                  size_t            remote_offset,
                                  size_t            bytes);
      };

//      template <class T_Model, class T_Device, class T_Memregion, unsigned T_StateBytes>
      //    inline bool DmaModel<T_Model,T_Device,T_Memregion,T_StateBytes>::init (size_t origin_rank)
      //  {
      //  return static_cast<T_Model*>(this)->init_impl (origin_rank);
      // }

      template <class T_Model, class T_Device, class T_Memregion, unsigned T_StateBytes>
      inline bool DmaModel<T_Model, T_Device, T_Memregion, T_StateBytes>::postDmaPut (uint8_t  (&state)[T_StateBytes],
          xmi_event_function   local_fn,
          void   	         * cookie,
          size_t            target_rank,
          T_Memregion     * local_memregion,
          size_t            local_offset,
          T_Memregion     * remote_memregion,
          size_t            remote_offset,
          size_t            bytes)
      {
        return static_cast<T_Model*>(this)->postDmaPut_impl (state,
                                                             local_fn,
                                                             cookie,
                                                             target_rank,
                                                             local_memregion,
                                                             local_offset,
                                                             remote_memregion,
                                                             remote_offset,
                                                             bytes);
      }


      template <class T_Model, class T_Device, class T_Memregion, unsigned T_StateBytes>
      inline bool DmaModel<T_Model, T_Device, T_Memregion, T_StateBytes>::postDmaGet (uint8_t (&state)[T_StateBytes],
          xmi_event_function   local_fn,
          void   	         * cookie,
          size_t            target_rank,
          T_Memregion     * local_memregion,
          size_t            local_offset,
          T_Memregion     * remote_memregion,
          size_t            remote_offset,
          size_t            bytes)
      {
        return static_cast<T_Model*>(this)->postDmaGet_impl (state,
                                                             local_fn,
                                                             cookie,
                                                             target_rank,
                                                             local_memregion,
                                                             local_offset,
                                                             remote_memregion,
                                                             remote_offset,
                                                             bytes);
      }
    };
  };
};
#endif // __componenets_devices_dmamodel_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
