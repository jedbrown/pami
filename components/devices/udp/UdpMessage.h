/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/udp/udpmessage.h
 * \brief ???
 */

#ifndef __components_devices_udp_udpmessage_h__
#define __components_devices_udp_udpmessage_h__

#include "sys/xmi.h"
#include "util/common.h"
#include "util/queue/Queue.h"
#include <stdint.h>
//#include <netinet/in.h>
//#include <sys/socket.h>

namespace XMI
{
  namespace Device
  {
   namespace UDP
   {
    struct UdpMsg
    {
       uint32_t            _device_dispatch_id;
       uint32_t            _metadata_size;
       uint32_t            _payload_size;
       char                _var_data[352];  // Metadata and payload

       UdpMsg( uint32_t device_dispatch_id, void * metadata, uint32_t metadatasize )
       {
          _device_dispatch_id = htonl(device_dispatch_id);
          _metadata_size = htonl(metadatasize);
          _payload_size = 0; 
         memcpy( _var_data, metadata, metadatasize ); 
       }

       UdpMsg()
       {
       }

       inline void setPayloadSize( size_t payload_size) 
        {
          _payload_size = htonl(payload_size); 
        }

        inline uint32_t getDeviceDispatchId()
        {
          return ntohl(_device_dispatch_id); 
        }

        inline size_t getMetadataSize()
        {
          return ntohl(_metadata_size); 
        }
 
        inline size_t getPayloadSize()
        {
          return ntohl(_payload_size); 
        }

        inline void * getMetadataAddr()
        {
          return _var_data;
        }
  
        inline void * getPayloadAddr()
        {
          return (void *)( (uintptr_t)_var_data+getMetadataSize() ); 
        }

    };

    class UdpSendMessage
    {
      public:
        inline UdpSendMessage ( xmi_client_t         client, 
                            size_t               context,
                            xmi_event_function   fn,
                            void               * cookie,
                            uint32_t             device_dispatch_id,
                            void               * metadata,
                            size_t               metasize,
                            struct iovec       * iov,
                            size_t               niov,
                            bool                 single_pkt) :
          _msg( device_dispatch_id, metadata, metasize ),
          _context (context),
          _client (client),
          _done_fn (fn),
          _cookie (cookie),
          _iov(iov),
          _niov(niov),
          _single_pkt(single_pkt),
          _offset(0),
          _on_idx(0), 
          _complete(false)
        {
          size_t left = 352 - metasize;    // TODO need to make size variable
          void * insert_pt = (void *)((uintptr_t)_msg._var_data+metasize);
          size_t i, amount;          
          for ( i=0; i<niov; i++ )
          {
             amount = iov[i].iov_len;
             if (left - amount < 0 ) {
               // Can't fit the whole thing       
               _offset = left; 
               amount = left; 
               _on_idx = i; 
             } 
             memcpy( insert_pt, iov[i].iov_base, amount ); 
             insert_pt = (void *)((uintptr_t)insert_pt + amount);
             left -= amount;   
             if ( left == 0 ) break;               
          }

          _msg.setPayloadSize( (352 - metasize) - left ); 

          if ( _single_pkt == true || ( _offset == 0 && _on_idx == 0 ) ) 
          {
            _complete = true; 
          } 
        }

        inline void nextPacket()
        {
          // if no more to do return true; 
          if (_complete == true ) return;  // should I abort? 
          size_t metasize = _msg.getMetadataSize(); 
          size_t left = 352 - metasize;    // TODO need to make size variable
          void * insert_pt = (void *)((uintptr_t)_msg._var_data+metasize);
          void * take_from; 
          size_t i, amount;          
          for ( i=_on_idx; i<_niov; i++ )
          {
             amount = _iov[i].iov_len-_offset;
             take_from = (void*)((uintptr_t)_iov[i].iov_base+_offset);
             if (left - amount < 0 ) {
               // Can't fit the whole thing       
               _offset = left; 
               amount = left; 
               _on_idx = i; 
             } else {
               _offset = 0; 
               _on_idx = 0; 
             }
             memcpy( insert_pt, take_from, amount ); 
             insert_pt = (void *)((uintptr_t)insert_pt + amount);
             left -= amount;   
             if ( left == 0 ) break;               
          }

         _msg.setPayloadSize( (352 - metasize) - left ); 

          if ( _offset == 0 && _on_idx == 0 ) 
          {
            _complete = true; 
          }          
        }   
          

        inline void done()
        {
           if (_done_fn != NULL ) 
           {
              _done_fn((void*)_context,_cookie,XMI_SUCCESS);
           }
        }

       inline void setPayloadSize( size_t payload_size) 
        {
          _msg._payload_size = htonl(payload_size); 
        }

        inline void * getAddr()
        {
          return &_msg;
        }

        inline size_t getSize()
        {
          return sizeof(_msg); 
        }

        inline uint32_t getDeviceDispatchId()
        {
          return ntohl(_msg._device_dispatch_id); 
        }

        inline size_t getMetadataSize()
        {
          return ntohl(_msg._metadata_size); 
        }
 
        inline size_t getPayloadSize()
        {
          return ntohl(_msg._payload_size); 
        }

        UdpMsg              _msg; 
        size_t              _context; 
        xmi_client_t        _client; 
        xmi_event_function  _done_fn;
        void              * _cookie;
        void              * _metadata;
        struct iovec      * _iov;
        size_t              _niov;
        bool                _single_pkt;
        size_t              _offset;
        size_t              _on_idx; 
        bool                _complete; 

      protected:
      private:
    };
   };
  };
};

#endif // __components_devices_udp_udpbasemessage_h__
