/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/Context.h
 * \brief ???
 */

#ifndef __components_devices_bgq_mu2_Context_h__
#define __components_devices_bgq_mu2_Context_h__

#include <hwi/include/bqc/MU_PacketHeader.h>


#include <spi/include/mu/InjFifo.h>
#include <spi/include/mu/RecFifo.h>
#include <spi/include/mu/DescriptorBaseXX.h>
#include <spi/include/mu/DescriptorWrapperXX.h>
#include <spi/include/mu/Pt2PtMemoryFIFODescriptorXX.h>

#include "Global.h"

#include "components/devices/BaseDevice.h"
#include "components/devices/PacketInterface.h"

#include "components/devices/generic/Device.h"

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      //
      /// \todo Eliminate the need for this class to implement
      ///       Interface::BaseDevice and Interface::PacketDevice
      ///
      class Context : public Interface::BaseDevice<Context>, public Interface::PacketDevice<Context>
      {
        protected:

          typedef struct
          {
            Interface::RecvFunction_t   f;
            void                      * p;
          } mu_dispatch_t;

          /// Total number of dispatch sets
          static const size_t dispatch_set_count = 256;

          /// Number of dispatch functions in a dispatch set
          static const size_t dispatch_set_size  = 16;

          /// \see PAMI::Device::Interface::RecvFunction_t
          static int noop (void   * metadata,
                           void   * payload,
                           size_t   bytes,
                           void   * recv_func_parm,
                           void   * cookie)
          {
            fprintf (stderr, "Error. Dispatch to unregistered id (%zu).\n", (size_t) recv_func_parm);
            abort();
            return 0;
          };

        public:

          /// Number of software bytes in a single packet transfer, less the
          /// number of bytes used internally by the MU::Context, available.
          /// \todo replace with a constant from SPIs somewhere
          static const size_t packet_metadata_size  = 17;

          /// Number of software bytes in a multi-packet transfer, less the
          /// number of bytes used internally by the MU::Context, available.
          /// \todo replace with a constant from SPIs somewhere
          static const size_t message_metadata_size = 13;

          /// Number of payload bytes available in a packet.
          /// \todo replace with a constant from SPIs somewhere
          static const size_t payload_size          = 512;

          ///
          /// \brief foo
          ///
          /// \param[in] global    The global component is used by the
          ///                      MU::Context to access the MU::Global
          ///                      component to allocate and initialize MU
          ///                      resources, as well as access the other
          ///                      global components such as the Mapping
          ///                      component
          /// \param[in] id_base   The base identifier of the MU::Context with
          ///                      offset zero
          /// \param[in] id_offset Offset from the base identifier for this
          ///                      MU::Context.
          /// \param[in] id_count  Number of MU::Context objects in the context
          ///                      set - all context objects in a set share a
          ///                      common base identifier
          ///
          inline Context (PAMI::Global    & global,
                          size_t            id_base,
                          size_t            id_offset,
                          size_t            id_count,
                          Generic::Device & progress) :
              Interface::BaseDevice<Context> (),
              Interface::PacketDevice<Context> (),
              _global (global),
              _progress (progress),
              _id_base (id_base),
              _id_offset (id_offset),
              _id_count (id_count)
          {
            // Initialize the dispatch table. This 'noop' function will be
            // replaced with an 'unexpected packet' function and queue.
            size_t i;

            for (i = 0; i < MU::Context::dispatch_set_count * MU::Context::dispatch_set_size; i++)
              {
                _dispatch[i].f = noop;
                _dispatch[i].p = (void *) i;
              }
          };

          ///
          /// \brief Initialize the mu context
          ///
          /// Functionally replaces the implementation of the
          /// Interface::BaseDevice::init interface and only specifies
          /// parameters that are necessary for MU operations.
          ///
          /// \param[in] id_client The client identifier
          // \param[in] client    PAMI communication client
          // \param[in] context   PAMI communication context
          // \param[in] mm        Memory manager for this mu context
          ///
          inline void init (size_t                  id_client//,
                            //pami_client_t           client,
                            //pami_context_t          context
                            //,Memory::MemoryManager * mm
                            )
          {
            _id_client = id_client;

            // Need to find a way to break this dependency...
            //_client = client;
            //_context = context;

            return;
          }

#if 1
          // ------------------------------------------------------------------
          // ------------------------------------------------------------------
          //
          // Begin device interface implementations.
          //
          // These implementations should be removed when the device interfaces
          // are eliminated, with the remaining device interfaces mirrored with
          // new model interfaces.
          //
          // ------------------------------------------------------------------
          // ------------------------------------------------------------------

          ///
          /// \copydoc Interface::BaseDevice::init
          ///
          inline int init_impl (Memory::MemoryManager * mm,
                                size_t                  clientid,
                                size_t                  num_ctx,
                                pami_context_t          context,
                                size_t                  contextid)
          {
            abort();
            return 0;
          }

          ///
          /// \copydoc Interface::BaseDevice::getContext
          ///
          inline pami_context_t getContext_impl ()
          {
            abort();
            return (pami_context_t) 0;
          }

          ///
          /// \copydoc Interface::BaseDevice::getContextOffset
          ///
          inline size_t getContextOffset_impl ()
          {
            abort();
            return 0;
          }

          ///
          /// \copydoc Interface::BaseDevice::isInit
          ///
          inline bool isInit_impl ()
          {
            abort();
            return false;
          }

          ///
          /// \copydoc Interface::BaseDevice::peers
          ///
          inline size_t peers_impl ()
          {
            abort();
            return 0;
          }

          ///
          /// \copydoc Interface::BaseDevice::task2peer
          ///
          inline size_t task2peer_impl (size_t task)
          {
            abort();
            return 0;
          }

          ///
          /// \copydoc Interface::BaseDevice::isPeer
          ///
          inline bool isPeer_impl (size_t task)
          {
            abort();
            return false;
          }

          ///
          /// \copydoc Interface::BaseDevice::advance
          ///
          inline int advance_impl ()
          {
            abort();
            return 0;
          }

          ///
          /// \copydoc Interface::PacketDevice::read
          ///
          inline int read_impl (void * dst, size_t bytes, void * cookie)
          {
            abort();
            return 0;
          }

          // ------------------------------------------------------------------
          // ------------------------------------------------------------------
          //
          // End device interface implementations.
          //
          // ------------------------------------------------------------------
          // ------------------------------------------------------------------
#endif

          ///
          /// \brief
          ///
          ///
          ///
          inline bool registerPacketHandler (size_t                      set,
                                             Interface::RecvFunction_t   fn,
                                             void                      * cookie,
                                             uint16_t                  & id)
          {
            // There are DISPATCH_SET_COUNT sets of dispatch functions.
            // There are DISPATCH_SET_SIZE  dispatch functions in each dispatch set.

            if (set >= MU::Context::dispatch_set_count) return false;

            unsigned i;

            for (i = 0; i < MU::Context::dispatch_set_size; i++)
              {
                id = set * MU::Context::dispatch_set_size + i;

                if (_dispatch[id].f == noop)
                  {
                    _dispatch[id].f = fn;
                    _dispatch[id].p = cookie;

                    return true;
                  }
              }

            return false;
          }


          ///
          /// \brief
          ///
          /// The pinFifo method is used for two purposes: to retrieve the
          /// context-relative injection fifo identification number of the
          /// injection fifo to which communication with the destination
          /// task+offset is pinned, and to provide MUSPI information needed
          /// to initialize and inject a descriptor.
          ///
          /// \note The reception fifo id field of the packet header is only
          ///       9 bits. Perhaps a reference to the descriptor to be pinned
          ///       should be passed in and initialized instead of returning
          ///       the reception fifo id and torus fifo map as output.
          ///
          /// \see MUHWI_MessageUnitHeader.Memory_FIFO.Rec_FIFO_Id
          /// \see MUHWI_Descriptor_t.Torus_FIFO_Map
          ///
          /// \param[in]  task   Destination task identifier
          /// \param[in]  offset Destination task context offset identifier
          /// \param[out] ififo  Pinned MUSPI injection fifo structure
          /// \param[out] rfifo  Reception fifo id to address the task+offset
          /// \param[out] map    Pinned MUSPI torus injection fifo map
          ///
          /// \return Context-relative injection fifo number pinned to the
          ///         task+offset destination
          ///
          inline size_t pinFifo (size_t             task,
                                 size_t             offset,
                                 MUSPI_InjFifo_t ** ififo,
                                 uint16_t         & rfifo,
                                 uint64_t         & map)
          {
            abort();
            return  0;
          }

          ///
          /// \brief
          ///
          /// This method obtains the next available descriptor slot in the
          /// injection fifo. MU model components may copy, or clone, a
          /// partially initialized descriptor into the injection fifo, then
          /// complete the descriptor initialization in the injection fifo, and
          /// finally update the injection fifo tail pointer.
          ///
          /// \todo Implement a version of this method that does not return
          ///       \c vaddr and \c paddr as some usage scenarios do not need
          ///       this information
          ///
          /// \param[in]  fnum  Context-relative injection fifo number
          /// \param[out] desc  Next available descriptor slot in the fifo
          /// \param[out] vaddr Virtual address of the look-aside payload
          ///                   buffer associated with the descriptor slot
          /// \param[out] paddr Physical address of \c vaddr
          ///
          /// \return   Number of available descriptor slots in the injection
          ///           fifo before a fifo wrap event will occur
          /// \retval 0 The injection fifo is full
          ///
          inline size_t nextInjectionDescriptor (size_t                fnum,
                                                 MUHWI_Descriptor_t ** desc,
                                                 void               ** vaddr,
                                                 uint64_t            * paddr)
          {
            abort();
            return 0;
          }

          ///
          /// \brief
          ///
          /// \param[in] fnum Context-relative injection fifo number
          /// \param[in] msg  Message object to be added to the send queue
          ///                 associated with the injection fifo number
          ///
          inline void post (size_t fnum, //MU::SendQueue::Message
                            void   * msg)
          {
            abort();
            return;
          }

        protected:

          PAMI::Global    & _global;
          Generic::Device & _progress;
          size_t            _id_base;
          size_t            _id_offset;
          size_t            _id_count;
          size_t            _id_client;


          mu_dispatch_t     _dispatch[dispatch_set_count * dispatch_set_size];

      }; // class     PAMI::Device::MU::Context
    };   // namespace PAMI::Device::MU
  };     // namespace PAMI::Device
};       // namespace PAMI


#endif // __components_devices_bgq_mu2_Context_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//

