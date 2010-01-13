/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/udp/UdpDevice.h
 * \brief ???
 */

#ifndef __components_devices_udp_UdpDevice_h__
#define __components_devices_udp_UdpDevice_h__

#include "components/devices/BaseDevice.h"
#include "components/devices/PacketInterface.h"
#include "components/devices/udp/UdpMessage.h"
#include "components/devices/udp/UdpSndConnection.h"
#include "components/devices/udp/UdpRcvConnection.h"
#include <map>
#include <list>
#include "util/ccmi_debug.h"
#include "trace.h"

#define DISPATCH_SET_SIZE 256
namespace XMI
{
  namespace Device
  {
    namespace UDP
    {
    typedef struct udp_dispatch_info_t
    {
      Interface::RecvFunction_t  direct_recv_func;
      void                      *direct_recv_func_parm;
    }udp_dispatch_info_t;

    template <class T_SysDep>
    class UdpDevice : public Interface::BaseDevice<UdpDevice<T_SysDep>, T_SysDep>,
                      public Interface::PacketDevice<UdpDevice<T_SysDep> >
    {
    public:
      static const size_t packet_payload_size = 224;
      inline UdpDevice () :
      Interface::BaseDevice<UdpDevice<T_SysDep>, T_SysDep> (),
      Interface::PacketDevice<UdpDevice<T_SysDep> >()
      {
        TRACE_ADAPTOR((stderr,"<%#.8X>UdpDevice()\n",(int)this));
      };

      // Implement BaseDevice Routines

      inline ~UdpDevice () {
        if ( __global.mapping.isUdpActive() )
        {
         __global.mapping.deactivateUdp();
        }
      }

      inline xmi_result_t setDispatchFunc (size_t                      dispatch,
                                  Interface::RecvFunction_t   direct_recv_func,
                                  void                      * direct_recv_func_parm,
                                  size_t & device_dispatch_id )
      {
         unsigned i;
          for (i=0; i<DISPATCH_SET_SIZE; i++)
          {
            unsigned id = dispatch * DISPATCH_SET_SIZE + i;
            if (_dispatch_table[id].direct_recv_func == NULL)
            {
              TRACE_COUT( "dispatch table ["<<id<<"].direct_recv_func being set to " << (void*)direct_recv_func )
              _dispatch_table[id].direct_recv_func=direct_recv_func;
              _dispatch_table[id].direct_recv_func_parm=direct_recv_func_parm;
              _dispatch_lookup[id] = _dispatch_table[id];
              device_dispatch_id = id;
              return XMI_SUCCESS;
            }
          }
          return XMI_NERROR;
      }

      inline int init_impl (T_SysDep * sysdep)
      {
        if ( __global.mapping.activateUdp() != XMI_SUCCESS ) abort();

        _sndConnections = (UdpSndConnection**)malloc(__global.mapping.size()*sizeof(UdpSndConnection*));
        // setup the connections
        size_t i;
        // Advance all the send UDP sockets
        for (i=0; i<__global.mapping.size(); i++ )
        {
           _sndConnections[i] = new UdpSndConnection( i );
        }

        _rcvConnection = new UdpRcvConnection( );

        return XMI_SUCCESS;
      };

      inline bool isInit_impl ()
      {
        return __global.mapping.isUdpActive();
      };

      inline int advance_impl ()
      {
        static int dbg = 1;

        if(dbg) {
          TRACE_ADAPTOR((stderr,"<%#.8X>UdpDevice::advance_impl\n",(int)this));
          dbg = 0;
        }

        size_t i;
        // Advance all the send UDP sockets
        for (i=0; i<__global.mapping.size(); i++ )
        {
           _sndConnections[i]->advance();
        }

        if ( _rcvConnection->advance() == 0 )
        {
          // Packet received, so dispatch it
          Interface::RecvFunction_t rcvFun = _dispatch_table[_rcvConnection->getDeviceDispatchId()].direct_recv_func;
          if ( rcvFun != NULL )   // Ignoring stuff we don't expect, since we are UDP
          {
             TRACE_COUT( "Dispatching to _dispatch_table["<<_rcvConnection->getDeviceDispatchId()<<"] = "<< (void*)rcvFun )

             rcvFun( _rcvConnection->_msg.getMetadataAddr(),
                     _rcvConnection->_msg.getPayloadAddr(),
                     _rcvConnection->_msg.getPayloadSize(),
                     _dispatch_table[_rcvConnection->getDeviceDispatchId()].direct_recv_func_parm,
                     NULL );
          } else {
            TRACE_COUT( "Receive function NULL!  _dispatch_table["<<_rcvConnection->getDeviceDispatchId()<<"]" )
          }
        }

        return XMI_SUCCESS;

      };

      // Implement MessageDevice Routines
      static const size_t metadata_size = 10;  // TODO need to tune these   WAS 128  or 16
      static const size_t payload_size  = 224;

      // Implement Packet Device Routines
      inline int    read_impl(void * dst, size_t bytes, void * cookie)
      {
        return XMI_UNIMPL;
      }

      inline size_t peers_impl ()
      {
        return __global.mapping.size();
      }

      inline size_t task2peer_impl (size_t task)
      {
        assert(task < __global.mapping.size());
        return task;
      }
      inline void post(size_t task, UdpSendMessage* msg)
      {
        _sndConnections[task]->enqueueMsg(msg);
      }

      size_t                                    _dispatch_id;
      UdpSndConnection                       ** _sndConnections;
      UdpRcvConnection			      * _rcvConnection;
      std::map<int, udp_dispatch_info_t>        _dispatch_lookup;
      udp_dispatch_info_t                       _dispatch_table[256*DISPATCH_SET_SIZE];

    };
  };
   };
};
#endif // __components_devices_upd_upddevice_h__
