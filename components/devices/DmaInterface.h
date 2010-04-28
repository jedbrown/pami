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
#include <pami.h>

namespace PAMI
{
  namespace Device
  {
    namespace Interface
    {
      ///
      /// \todo Need A LOT MORE documentation on this interface and its use
      /// \param T_Model      Dma model template class
      /// \param T_Device     Dma device template class
      /// \param T_StateBytes Dma object template class
      ///
      template <class T_Model, class T_Device, unsigned T_StateBytes>
      class DmaModel
      {
        public:
          ///
          /// \brief Base dma model constructor
          ///
          /// \param[in] device  dma device
          ///
          inline DmaModel (T_Device & device, pami_result_t & status)
          {
            // This compile time assert verifies that the specific dma model
            // class, T_Model, has correctly specified the same value for the
            // 'transfer state bytes' template parameter and constant.
            COMPILE_TIME_ASSERT(T_Model::dma_model_state_bytes == T_StateBytes);

            // This compile time assert verifies that at least one of the two
            // types of dma post interfaces is supported.
            COMPILE_TIME_ASSERT(T_Model::dma_model_va_supported || T_Model::dma_model_mr_supported);

            status = PAMI_SUCCESS;
          };

          inline ~DmaModel () {};

          ///
          /// \brief Returns the virtual address dma attribute of this model.
          ///
          /// \attention All dma model interface derived classes \b must
          ///            contain a public static const data member named
          ///            'bool dma_model_va_supported'.
          ///
          /// C++ code using templates to specify the model may statically
          /// access the 'dma_model_va_supported' constant.
          ///
          static const bool getVirtualAddressSupported ();

          ///
          /// \brief Returns the memory region dma attribute of this model.
          ///
          /// \attention All dma model interface derived classes \b must
          ///            contain a public static const data member named
          ///            'bool dma_model_mr_supported'.
          ///
          /// C++ code using templates to specify the model may statically
          /// access the 'dma_model_mr_supported' constant.
          ///
          static const bool getMemoryRegionSupported ();

          ///
          /// \brief Returns the transfer state bytes attribute of this model.
          ///
          /// Typically a dma device will require some amount of temporary
          /// storage to be used during the dma operation. This attribute
          /// returns the number of bytes that must be provided to the various
          /// non-blocking dma post* methods.
          ///
          /// \attention All dma model interface derived classes \b must
          ///            contain a public static const data member named
          ///            'size_t dma_model_state_bytes'.
          ///
          /// C++ code using templates to specify the model may statically
          /// access the 'dma_model_state_bytes' constant.
          ///
          static const size_t getDmaTransferStateBytes ();

          ///
          /// \brief Post an immediate virtual address contiguous dma put operation
          ///
          ///
          /// \attention All dma model interface derived classes \b must
          ///            implement the postDmaPut_impl() method.
          ///
          /// \param[in] target_task      Global task identifier of the target
          /// \param[in] target_offset    Identifier of the destination context
          /// \param[in] bytes            Number of bytes to transfer
          /// \param[in] local            Virtual address of the local buffer to transfer
          /// \param[in] remote           Virtual address of the remote buffer
          ///
          /// \retval true  Put operation completed immediately
          /// \retval false Put operation was unable to be completed
          ///
          inline bool postDmaPut (size_t   target_task,
                                  size_t   target_offset,
                                  size_t   bytes,
                                  void   * local,
                                  void   * remote);

          ///
          /// \brief Post a non-blocking virtual address contiguous dma put operation
          ///
          /// The post dma put operation interface allows the dma model
          /// and dma device implementations to optimize for performance by
          /// avoiding the overhead to construct a dma message object. If
          /// the dma device has resources immediately available then the
          /// message may be directly posted, otherwise an internal dma message
          /// object may be constructed in the state array to maintain the
          /// transfer state until the callback is invoked.
          ///
          /// \attention All dma model interface derived classes \b must
          ///            implement the postDmaPut_impl() method.
          ///
          /// \param[in] state            Location to store the internal dma transfer state
          /// \param[in] local_fn         Callback to invoke when the operation is complete
          /// \param[in] cookie           Completion callback opaque application data
          /// \param[in] target_task      Global task identifier of the target
          /// \param[in] target_offset    Identifier of the destination context
          /// \param[in] bytes            Number of bytes to transfer
          /// \param[in] local            Virtual address of the local buffer to transfer
          /// \param[in] remote           Virtual address of the remote buffer
          ///
          /// \retval true  Operation completed immediately and the completion callback was invoked
          /// \retval false Operation was not completed immediately
          ///
          inline bool postDmaPut (uint8_t               (&state)[T_StateBytes],
                                  pami_event_function   local_fn,
                                  void                * cookie,
                                  size_t                target_task,
                                  size_t                target_offset,
                                  size_t                bytes,
                                  void                * local,
                                  void                * remote);


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
          /// \param[in] target_offset    Identifier of the destination context
          /// \param[in] bytes            Number of bytes to transfer
          /// \param[in] local_memregion  Local data memory region
          /// \param[in] local_offset     Offset of data buffer in the local memory region
          /// \param[in] remote_memregion Remote data memory region
          /// \param[in] remote_offset    Offset of data buffer in the remote memory region
          ///
          /// \retval true  Dma operation completed and the completion
          ///               callback was invoked
          ///
          /// \retval false Dma operation did not complete and the dma
          ///               device must be advanced until the completion
          ///               callback is invoked
          ///
          inline bool postDmaPut (uint8_t               (&state)[T_StateBytes],
                                  pami_event_function   local_fn,
                                  void                * cookie,
                                  size_t                target_task,
                                  size_t                target_offset,
                                  size_t                bytes,
                                  Memregion           * local_memregion,
                                  size_t                local_offset,
                                  Memregion           * remote_memregion,
                                  size_t                remote_offset);

          inline bool postDmaPut (size_t                target_task,
                                  size_t                target_offset,
                                  size_t                bytes,
                                  Memregion           * local_memregion,
                                  size_t                local_offset,
                                  Memregion           * remote_memregion,
                                  size_t                remote_offset);

          ///
          /// \brief Post an immediate virtual address contiguous dma get operation
          ///
          ///
          /// \attention All dma model interface derived classes \b must
          ///            implement the postDmaGet_impl() method.
          ///
          /// \param[in] target_task      Global task identifier of the target
          /// \param[in] target_offset    Identifier of the destination context
          /// \param[in] bytes            Number of bytes to transfer
          /// \param[in] local            Virtual address of the local buffer
          /// \param[in] remote           Virtual address of the remote buffer to transfer
          ///
          /// \retval true  Operation completed immediately
          /// \retval false Operation was unable to be completed
          ///
          inline bool postDmaGet (size_t   target_task,
                                  size_t   target_offset,
                                  size_t   bytes,
                                  void   * local,
                                  void   * remote);

          ///
          /// \brief Post a non-blocking virtual address contiguous dma get operation
          ///
          /// The post dma get operation interface allows the dma model
          /// and dma device implementations to optimize for performance by
          /// avoiding the overhead to construct a dma message object. If
          /// the dma device has resources immediately available then the
          /// message may be directly posted, otherwise an internal dma message
          /// object may be constructed in the state array to maintain the
          /// transfer state until the callback is invoked.
          ///
          /// \attention All dma model interface derived classes \b must
          ///            implement the postDmaGet_impl() method.
          ///
          /// \param[in] state            Location to store the internal dma transfer state
          /// \param[in] local_fn         Callback to invoke when the operation is complete
          /// \param[in] cookie           Completion callback opaque application data
          /// \param[in] target_task      Global task identifier of the target
          /// \param[in] target_offset    Identifier of the destination context
          /// \param[in] bytes            Number of bytes to transfer
          /// \param[in] local            Virtual address of the local buffer to transfer
          /// \param[in] remote           Virtual address of the remote buffer
          ///
          /// \retval true  Operation completed immediately and the completion callback was invoked
          /// \retval false Operation was not completed immediately
          ///
          inline bool postDmaGet (uint8_t               (&state)[T_StateBytes],
                                  pami_event_function   local_fn,
                                  void                * cookie,
                                  size_t                target_task,
                                  size_t                target_offset,
                                  size_t                bytes,
                                  void                * local,
                                  void                * remote);

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
          /// \param[in] cookie           Completion callback opaque application data
          /// \param[in] target_task      Global task identifier of the target
          /// \param[in] target_offset    Identifier of the destination context
          /// \param[in] bytes            Number of bytes to transfer
          /// \param[in] local_memregion  Local data memory region
          /// \param[in] local_offset     Offset of data buffer in the local memory region
          /// \param[in] remote_memregion Remote data memory region
          /// \param[in] remote_offset    Offset of data buffer in the remote memory region
          ///
          /// \retval true  Dma operation completed and the completion
          ///               callback was invoked
          ///
          /// \retval false Dma operation did not complete and the dma
          ///               device must be advanced until the completion
          ///               callback is invoked
          ///
          inline bool postDmaGet (uint8_t              (&state)[T_StateBytes],
                                  pami_event_function   local_fn,
                                  void                * cookie,
                                  size_t                target_task,
                                  size_t                target_offset,
                                  size_t                bytes,
                                  Memregion           * local_memregion,
                                  size_t                local_offset,
                                  Memregion           * remote_memregion,
                                  size_t                remote_offset);

          inline bool postDmaGet (size_t                target_task,
                                  size_t                target_offset,
                                  size_t                bytes,
                                  Memregion           * local_memregion,
                                  size_t                local_offset,
                                  Memregion           * remote_memregion,
                                  size_t                remote_offset);
      };

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      const bool DmaModel<T_Model, T_Device, T_StateBytes>::getVirtualAddressSupported ()
      {
        return T_Model::dma_model_va_supported;
      }

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      const bool DmaModel<T_Model, T_Device, T_StateBytes>::getMemoryRegionSupported ()
      {
        return T_Model::dma_model_mr_supported;
      }

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      const size_t DmaModel<T_Model, T_Device, T_StateBytes>::getDmaTransferStateBytes ()
      {
        return T_Model::dma_model_state_bytes;
      }

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      inline bool DmaModel<T_Model, T_Device, T_StateBytes>::postDmaPut (
          size_t   target_task,
          size_t   target_offset,
          size_t   bytes,
          void   * local,
          void   * remote)
      {
        return static_cast<T_Model*>(this)->postDmaPut_impl (target_task,
                                                             target_offset,
                                                             bytes,
                                                             local,
                                                             remote);
      }

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      inline bool DmaModel<T_Model, T_Device, T_StateBytes>::postDmaPut (
          uint8_t               (&state)[T_StateBytes],
          pami_event_function   local_fn,
          void                * cookie,
          size_t                target_task,
          size_t                target_offset,
          size_t                bytes,
          void                * local,
          void                * remote)
      {
        return static_cast<T_Model*>(this)->postDmaPut_impl (state,
                                                             local_fn,
                                                             cookie,
                                                             target_task,
                                                             target_offset,
                                                             bytes,
                                                             local,
                                                             remote);
      }

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      inline bool DmaModel<T_Model, T_Device, T_StateBytes>::postDmaPut (
          size_t                target_task,
          size_t                target_offset,
          size_t                bytes,
          Memregion           * local_memregion,
          size_t                local_offset,
          Memregion           * remote_memregion,
          size_t                remote_offset)
      {
        return static_cast<T_Model*>(this)->postDmaPut_impl (target_task,
                                                             target_offset,
                                                             bytes,
                                                             local_memregion,
                                                             local_offset,
                                                             remote_memregion,
                                                             remote_offset);
      }

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      inline bool DmaModel<T_Model, T_Device, T_StateBytes>::postDmaPut (
          uint8_t               (&state)[T_StateBytes],
          pami_event_function   local_fn,
          void                * cookie,
          size_t                target_task,
          size_t                target_offset,
          size_t                bytes,
          Memregion           * local_memregion,
          size_t                local_offset,
          Memregion           * remote_memregion,
          size_t                remote_offset)
      {
        return static_cast<T_Model*>(this)->postDmaPut_impl (state,
                                                             local_fn,
                                                             cookie,
                                                             target_task,
                                                             target_offset,
                                                             bytes,
                                                             local_memregion,
                                                             local_offset,
                                                             remote_memregion,
                                                             remote_offset);
      }


      template <class T_Model, class T_Device, unsigned T_StateBytes>
      inline bool DmaModel<T_Model, T_Device, T_StateBytes>::postDmaGet (
          size_t   target_task,
          size_t   target_offset,
          size_t   bytes,
          void   * local,
          void   * remote)
      {
        return static_cast<T_Model*>(this)->postDmaGet_impl (target_task,
                                                             target_offset,
                                                             bytes,
                                                             local,
                                                             remote);
      }

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      inline bool DmaModel<T_Model, T_Device, T_StateBytes>::postDmaGet (
          uint8_t               (&state)[T_StateBytes],
          pami_event_function   local_fn,
          void                * cookie,
          size_t                target_task,
          size_t                target_offset,
          size_t                bytes,
          void                * local,
          void                * remote)
      {
        return static_cast<T_Model*>(this)->postDmaGet_impl (state,
                                                             local_fn,
                                                             cookie,
                                                             target_task,
                                                             target_offset,
                                                             bytes,
                                                             local,
                                                             remote);
      }

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      inline bool DmaModel<T_Model, T_Device, T_StateBytes>::postDmaGet (
          size_t                target_task,
          size_t                target_offset,
          size_t                bytes,
          Memregion           * local_memregion,
          size_t                local_offset,
          Memregion           * remote_memregion,
          size_t                remote_offset)
      {
        return static_cast<T_Model*>(this)->postDmaGet_impl (target_task,
                                                             target_offset,
                                                             bytes,
                                                             local_memregion,
                                                             local_offset,
                                                             remote_memregion,
                                                             remote_offset);
      }

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      inline bool DmaModel<T_Model, T_Device, T_StateBytes>::postDmaGet (
          uint8_t               (&state)[T_StateBytes],
          pami_event_function   local_fn,
          void                * cookie,
          size_t                target_task,
          size_t                target_offset,
          size_t                bytes,
          Memregion           * local_memregion,
          size_t                local_offset,
          Memregion           * remote_memregion,
          size_t                remote_offset)
      {
        return static_cast<T_Model*>(this)->postDmaGet_impl (state,
                                                             local_fn,
                                                             cookie,
                                                             target_task,
                                                             target_offset,
                                                             bytes,
                                                             local_memregion,
                                                             local_offset,
                                                             remote_memregion,
                                                             remote_offset);
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
