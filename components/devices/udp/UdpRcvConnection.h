/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/udp/UdpRcvConnection.h
 * \brief ???
 */

#ifndef __components_devices_udp_udprcvconnection_h__
#define __components_devices_udp_udprcvconnection_h__

#include "components/devices/udp/UdpMessage.h"
#include "util/ccmi_debug.h"
#define IOV_MAX 256  // TODO WHat is the right value for this?

#define DISPATCH_SET_SIZE 256
namespace XMI
{
  namespace Device
  {
  namespace UDP
  {
    class UdpRcvConnection
    {
    public:

      UdpRcvConnection():
       _msg()
      {
        __global.mapping.udpAddr( _rcvFd ); 
      }

      inline int advance()
      {
         struct sockaddr_storage _their_addr;
         socklen_t               _their_addr_len = sizeof(_their_addr); 
         int bytes_rcv = recvfrom(_rcvFd, &_msg, sizeof(_msg), MSG_DONTWAIT, (struct sockaddr*)&_their_addr, &_their_addr_len);
         if ( bytes_rcv == -1 && errno == EWOULDBLOCK ) return -1; 
         if ( bytes_rcv == -1 ) 
         {
           abort(); 
         }

         // All of the packet is available
         return 0;
      }

      inline uint32_t getDeviceDispatchId()
      {
         return _msg.getDeviceDispatchId(); 
      }

      int                     _rcvFd;     // UDP socket for this Connection

      UdpMsg                  _msg; 
    protected:
    private: 

     };
    };
  };
};
#endif // __components_devices_udp_udprcvconnection_h__
