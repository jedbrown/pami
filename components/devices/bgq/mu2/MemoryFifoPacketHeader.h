/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/MemoryFifoPacketHeader.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu2_MemoryFifoPacketHeader_h__
#define __components_devices_bgq_mu2_MemoryFifoPacketHeader_h__

#include <hwi/include/bqc/MU_PacketHeader.h>


namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      class MemoryFifoPacketHeader : public MUHWI_PacketHeader_t
      {
        public:

          /// Number of unused software bytes available in a single packet
          /// transfer. The dispatch identifier and other control data are
          /// are set outside of this range.
          static const size_t packet_singlepacket_metadata_size = 17;

          /// Number of unused software bytes available in a multi-packet
          /// transfer. The dispatch identifier and other control data are
          /// are set outside of this range.
          static const size_t packet_multipacket_metadata_size  = 12;

          inline void setSinglePacket (bool value)
          {
            messageUnitHeader.Packet_Types.Memory_FIFO.Unused1 = value;
          };

          inline bool isSinglePacket ()
          {
            return messageUnitHeader.Packet_Types.Memory_FIFO.Unused1;
          };

          ///
          /// \brief Set the dispatch id in a mu packet header
          ///
          /// The dispatch id is written in a different location depending on
          /// the value of the "single packet" attribute in the header.
          ///
          /// \warning The single packet attribute must be set before this
          ///          method is used.
          ///
          /// \see setSinglePacket
          /// \see isSinglePacket
          ///
          /// \param[in]
          /// \param[in] id  The dispatch identifier to set
          ///
          inline void setDispatchId (uint16_t id)
          {
            if (likely(isSinglePacket ()))
              {
                uint32_t * raw32 = (uint32_t *) this;
                raw32[0] |= ((uint32_t) id) << 8;
                return;
              }

            uint16_t * metadata = (uint16_t *) messageUnitHeader.Packet_Types.Memory_FIFO.Unused2;
            metadata[0] = id;
            return;
          };

          inline uint16_t getDispatchId ()
          {
            if (likely(isSinglePacket ()))
              {
                uint32_t * raw32 = (uint32_t *) this;
                uint16_t id = (uint16_t) (raw32[0] >> 8);
                return id & 0x01fff;
              }

            uint16_t * metadata = (uint16_t *) messageUnitHeader.Packet_Types.Memory_FIFO.Unused2;
            return metadata[0];
          };
      };
    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

#endif   // __components_devices_bgq_mu2_MemoryFifoPacketHeader_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
