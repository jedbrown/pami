/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/udp/UdpSndConnection.h
 * \brief Manages the send connection for Udp
 */

#ifndef __components_devices_udp_udpsendconnection_h__
#define __components_devices_udp_udpsendconnection_h__

#include "components/devices/udp/UdpMessage.h"
#include <list>
#include "util/ccmi_debug.h"

#define DISPATCH_SET_SIZE 256
namespace XMI
{
  namespace Device
  {
  namespace UDP
  {
    class UdpSndConnection
    {
    public:

      UdpSndConnection( size_t target )
      {
        __global.mapping.task2udp( target, _sendFd, (sockaddr*)&_sendAddr, _sendAddrLen ); 
      }
  
      inline void enqueueMsg( UdpSendMessage * msg ) 
      {
         _sendQ.push_back( msg );
      } 

      inline int advance()
      {
        if (_sendQ.empty() == true ) return 0; 
        UdpSendMessage* msg = *(_sendQ.begin());
        // Use blocking, since will overwrite the data
        int numbytes = sendto(_sendFd, &(msg->_msg), msg->getSize(), 0, (struct sockaddr *)&_sendAddr, _sendAddrLen); 
        if ( msg->_complete == true )
        {
          msg->done(); 
         _sendQ.remove( msg ); 
        } else {
          msg->nextPacket(); 
        }

        // ??? Use return value to indicate if advance is still needed?
        return 0; 
      }

    protected:
    private: 
      int                         _sendFd;     // UDP socket for this Connection
      sockaddr_storage            _sendAddr;
      int                         _sendAddrLen; 
      std::list<UdpSendMessage*>  _sendQ;      // Messages to send over this connection
    };
   };
  };
};
#endif // __components_devices_udp_udpsendconnection_h__
