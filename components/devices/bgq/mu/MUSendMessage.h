/**
 * \file components/devices/bgq/mu/MUSendMessage.h
 * \brief ???
 */

#ifndef __components_devices_bgq_mu_MUSendMessage_h__
#define __components_devices_bgq_mu_MUSendMessage_h__
#error FOO!
#include "MUBaseMessage.h"
#include <spi/include/mu/DescriptorBaseXX.h>
#include <spi/include/mu/DescriptorWrapperXX.h>
#include <spi/include/mu/Pt2PtMemoryFIFODescriptorXX.h>
#include <spi/include/kernel/memory.h>

#define TRACE(x) //printf x

namespace DCMF
{
  namespace MU
  {
    namespace MUSendMessage
    {

      struct PacketHeader
      {
        uint64_t        reserved0;   //Bytes 0-7   Torus network header
        uint32_t        reserved1;   //Bytes 8-11  Torus network header
        uint16_t        reserved2;   //Bytes 12-13
        uint16_t        reserved3_0; //Bytes 14-17 Put offset
        uint16_t        reserved3_1; //Bytes 14-17 Put offset
        uint16_t        sw_bytes0;   //Bytes 18-19 2 bytes of software header
        uint32_t        sw_bytes1[3];//Bytes 20-31 12 byes of software header
      } __attribute__((__packed__));

      ///
      /// \brief The handler to be called when the packet arrives in
      /// the memory FIFO
      ///
      typedef int (*DispatchHandler) (void         * clientdata,
                                      PacketHeader * hdr,
                                      uint32_t       packet_bytes);

      ///
      ///  \brief Performance optimization to minimize the overhead
      ///  creation of descriptors
      ///
      class Model
      {
        protected:
          MUSPI_Pt2PtMemoryFIFODescriptor _descBase __attribute__((__aligned__(32)));
          MUSPI_DescriptorWrapper         _descWrapper;

        public:
          Model () : _descBase(), _descWrapper (NULL) {}

          void  buildModel (MUDevice *device, int funcid)
          {
            Mapping *mapping = device->getMapping ();
            unsigned char buf[14];
            memset (buf, 0, sizeof(buf));

            buf [0] = funcid;  //Setting the high byte to funcid
            _descBase.setSoftwareBytes (buf);
          }


          void clone (MUSPI_Pt2PtMemoryFIFODescriptor & descBase,
                      MUSPI_DescriptorWrapper         & descWrapper)
          {
            _descBase.clone (descBase);
            _descWrapper.clone    (descWrapper);
            descWrapper.setDescriptorPtr (&descBase);
          }
      };

      ///
      /// \brief Class to store protocol regisration info, which
      /// stores the function ID.
      ///
      class Registration
      {
        protected:
          Model            _model;
          int              _dispatchid;
          MUDevice       * _device;

        public:
          Registration () : _model(), _dispatchid (-1)  {}
          void setDispatchID (int fid) { _dispatchid =  fid; }
          int dispatchID () { return _dispatchid; }
          Model * getModel () { return &_model; }
          MUDevice * getDevice () { return _device; }

          void initialize (MUDevice         * dev,
                           DispatchHandler  function)
          {
            _device = dev;
            int funcid = dev->registerPacketHandler
                         ((MUDevice_DispatchFn_t) function,
                          this);
            setDispatchID (funcid);
            _model.buildModel (dev, funcid);
          }

          void initialize (MUDevice           * dev,
                           DispatchHandler    function,
                           void             * arg)
          {
            int funcid = dev->registerPacketHandler
                         ((MUDevice_DispatchFn_t) function, arg);
            setDispatchID (funcid);
            _model.buildModel (dev, funcid);
          }
      };


      ///
      /// \brief The send message class
      ///
      class Send : public MUBaseMessage
      {
        public:
          Send () :
              MUBaseMessage (),
              _device  (NULL),
              _descBase(),
              _descWrapper (NULL)
          {};

          Send (MUDevice       * device,
                Registration   * registration,
                unsigned         rank):
              MUBaseMessage (),
              _device (device),
              _destrank (rank),
              _descBase(),
              _descWrapper (NULL)
          {
            //Create a descriptor and descriptor wrapper
            registration->getModel()->clone (_descBase, _descWrapper);
            setDest (rank, device->getMapping());
          }

          Send (MUDevice                        * device,
                MUSPI_Pt2PtMemoryFIFODescriptor * model,
                unsigned                          rank,
                void                            * metadata):
              MUBaseMessage (),
              _device (device),
              _destrank (rank),
              _descBase (),
              _descWrapper (NULL)
          {
            // Clone the model descriptor and then initialize
            model->clone (_descBase);

            // Copy the metadata into the network header.
            MemoryFifoPacketHeader_t * hdr =
              (MemoryFifoPacketHeader_t *) & desc.PacketHeader;
            DCMF_assert(metasize <= 17);
            memcpy((void *) &hdr->dev.singlepkt.metadata, metadata, metasize);

            _descWrapper.setDescriptorPtr ((MUSPI_DescriptorBase *) & _descBase);

            setDest (rank, device->getMapping());
          }

          void setDynamicRouting()
          {
            //Update to BGQ call
            //_desc.setVirtualChannel(DMA_PACKET_VC_D0);
          }

          void setSoftwareBytes (uint8_t *swbytes, uint32_t nsw)
          {
            DCMF_assert (nsw <= 12);

            for (uint32_t b = 0; b < nsw; b++)
              //Set from byte 2 to 12
              _descBase.setSoftwareByte (b + 2, swbytes[b]);
          }

#if 0
          //This param isnt aligned and hence will be slow to set
          void setSinglePacketParameter (unsigned parameter)
          {
            _descBase.setSinglePacketParameter (parameter);
          }
#endif

          void post () { _device->post (*this, _destrank); }

          ///
          /// \brief Set the sender side info
          /// \param[in] buf  Application data buffer to send
          /// \param[in] size Number of bytes to send
          ///
          void setSendData (const char *buf, unsigned size);


          ///
          /// \brief Set the destination of the message
          /// \param[in] rank of the destination
          ///
          void setDest (unsigned rank, Mapping *map);

          ///
          /// \brief Set the callback to be called when DMA has processed
          ///        the descriptor
          /// \param[in] cb_done    function pointer
          /// \param[in] clientdata opaque client data
          ///
          void setDoneCallback (void (*cb_done)(void *, DCMF_Error_t *),
                                void *clientdata)
          {
            _descWrapper.setDoneCallback
            ((void (*)(void *)) cb_done, clientdata);
          }

          ///
          /// \brief Advance This Message
          ///
          /// This function posts the descriptor for this message to the
          /// MU fifo(s).
          ///
          /// \param[in]  fifo  Pointer to a list of MU fifo number to be
          ///                   used by the advance function.
          /// \param[in]  nfifos  Number of fifo numbers in the fifo list.
          ///
          virtual bool advance(unsigned *fnum, unsigned nfifos);

        protected:
          int postDescriptor (unsigned fnum)
          {
            InjFifoSubGroup *injFifoSubGroup = NULL;
            uint32_t         relativeFnum = 0;

            _device->getSubGroupAndRelativeFifoNum ( fnum,
                                                     &injFifoSubGroup,
                                                     &relativeFnum );
            _descWrapper.setFIFONum ( relativeFnum );

            TRACE((stderr,"MUSendMessage.h postDescriptor() subgrpPtr:%llx fnum:%d\n",
                   (unsigned long long)injFifoSubGroup,relativeFnum));

            int rc  = injFifoSubGroup->push ( _descWrapper );
            return rc;
          }

          MUDevice                 * _device;
          uint32_t                   _destrank;
          MUSPI_Pt2PtMemoryFIFODescriptor _descBase __attribute__((__aligned__(16)));
          MUSPI_DescriptorWrapper         _descWrapper    __attribute__((__aligned__(16)));
      }; // class Send


      class Recv
      {
        public:

          inline Recv () :
              _sndlen (0),
              _rcvlen (0),
              _bytes (0),
              _rcvbuf (NULL),
              _cb_done (NULL),
              _clientdata (NULL)
          { }

          ///
          /// \brief Set the callback to be called when DMA has processed
          ///        the descriptor
          /// \param[in] cb_done    function pointer
          /// \param[in] clientdata opaque client data
          ///
          void setDoneCallback (void (*cb_done)(void *, DCMF_Error_t *), void *clientdata)
          {
            _cb_done    = cb_done;
            _clientdata = clientdata;
          }

          ///
          /// \brief set the receiver side parameters
          ///
          void setData (unsigned sndlen, unsigned rcvlen, char *buf)
          {
            _sndlen = sndlen;
            _rcvlen = (rcvlen <= sndlen) ? rcvlen : sndlen;
            _rcvbuf = buf;
          }

          bool processPacket (char *packet, int size);
          void executeCallback () { if (_cb_done) _cb_done (_clientdata, NULL); }

          unsigned getRcvlen () const { return _rcvlen; }
          unsigned getSndlen () const { return _sndlen; }

        protected:

          unsigned _sndlen;
          unsigned _rcvlen;
          unsigned _bytes;
          char * _rcvbuf;
          void (* _cb_done)(void *, DCMF_Error_t *);
          void * _clientdata;

      }; // End: Recv class

    };
  };
};


////////////////////////////////////////////////////////////////////////////////
///
/// \brief Set Send Data
///
/// Set information about the send buffer (its address and size) into the
/// descriptor.
///
/// \param[in]  buf   Pointer to the buffer.
/// \param[in]  size  Number of bytes in the buffer.
///
/// \todo Replace getting the memory region, which is a syscall, with something
///       that is faster.
///
////////////////////////////////////////////////////////////////////////////////

inline void DCMF::MU::MUSendMessage::Send::setSendData
(const char   * buf,
 unsigned       size)
{
  uint64_t pa; // Physical address of buffer
  uint32_t rc; // Return code
  Kernel_MemoryRegion_t memRegion; // Memory region associated with the buffer.

#if 0
  _device->getOffsetFromBase
  ((void*)buf, size);  // Use local memory region
#endif

  rc = Kernel_CreateMemoryRegion( &memRegion,
                                  (void*) buf,
                                  (size_t) size );
  DCMF_assert ( rc == 0 );

  pa = (uint64_t)memRegion.BasePa + ( (uint64_t)buf - (uint64_t)memRegion.BaseVa );

  _descBase.setPayload (pa, size);
}


inline bool DCMF::MU::MUSendMessage::Send::advance (unsigned * fifo,
                                                    unsigned   nfifos)
{
  int rc = postDescriptor (*fifo);
  return (rc > 0);
}


inline void DCMF::MU::MUSendMessage::Send::setDest (unsigned rank,
                                                    Mapping *mapping)
{
  MUHWI_Destination dst;

  DCMF_NetworkCoord_t  network;

  //assuming t is the
  //recv grp id
  mapping->rank2Network (rank, &network, DCMF_5DTORUS_NETWORK);

  dst.Destination.A_Destination = network.u.n_torus.coords[0];
  dst.Destination.B_Destination = network.u.n_torus.coords[1];
  dst.Destination.C_Destination = network.u.n_torus.coords[2];
  dst.Destination.D_Destination = network.u.n_torus.coords[3];
  dst.Destination.E_Destination = network.u.n_torus.coords[4];

  _descBase.setDestination (dst);
  _descBase.setRecFIFOId (_device->getRecFifoIdForDescriptor(network.u.n_torus.coords[5]));

  /// \todo Calculate the best torusInjectionFifoMap.
  /// For now, hard code to A-minus direction.
  _descBase.setTorusInjectionFIFOMap (MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM);

  /// \todo Calculate the best torus hints.
  /// For now, hard code to A-minus direction.
  _descBase.setHints ( MUHWI_PACKET_HINT_AM |
                       MUHWI_PACKET_HINT_B_NONE |
                       MUHWI_PACKET_HINT_C_NONE |
                       MUHWI_PACKET_HINT_D_NONE,
                       MUHWI_PACKET_HINT_E_NONE );

  // Initialize a local or remote descriptor
  // Equivalent bgq call
  //if ( mapping->isvnpeer (rank) ) _desc.setLocalMemcopy ();
}



///
/// \brief Process an incoming packet by copying its data into
///        application buffer
/// \param payload : pointer to packet buffer
/// \param size    : approx size of the packet
///
inline bool DCMF::MU::MUSendMessage::Recv::processPacket (char *payload, int asize)
{
  //copy (payload, size);
  int    to_copy  = _rcvlen - _bytes;
  char * dst_addr = _rcvbuf + _bytes;


  TRACE((stderr,"MUSendMessage.h processPacket rcvlen:%d bytes:%d to_copy:%d asize:%d\n",
         _rcvlen, _bytes, to_copy, asize));

  // All packets are full except for possibly the last packet.
#warning "Need optimize copy of full packets"
#if 0

  if ((((unsigned) _rcvbuf & 0x0f) == 0) && (to_copy >= 512))
    {
      _QuadCpy512 (dst_addr, payload);
      _bytes += 512;
    }
  else
#endif
    {
      //
      // Determine the amount to copy to be the MIN(size,to_copy)
      //
      int copySize = (to_copy > asize ? asize : to_copy);

      if (to_copy > 0)
        MEMCPY (dst_addr, payload, copySize);

      _bytes += asize; // This is a sender-side counter.  Must reflect
      // number of bytes sent, so we handle message
      // completion properly.
    }


  // This function only returns true
  // when all of the sender's data
  // has been handled, even if all of
  // it may not have been copied into
  // the receive buffer.


  return (_bytes >= _sndlen);
}

#undef TRACE

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
