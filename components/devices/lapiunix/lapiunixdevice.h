/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/lapi/lapidevice.h
 * \brief ???
 */

#ifndef __components_devices_lapi_lapidevice_h__
#define __components_devices_lapi_lapidevice_h__

#include "components/devices/BaseDevice.h"
#include "components/devices/MessageDevice.h"
#include "components/devices/lapiunix/lapiunixmessage.h"
#include <map>
#include <list>
#include <sched.h>

namespace XMI
{
  namespace Device
  {
    typedef struct lapi_dispatch_info_t
    {
      Interface::RecvFunction_t  recv_func;
      void                      *recv_func_parm;
    }lapi_dispatch_info_t;

    typedef struct lapi_mcast_dispatch_info_t
    {
      xmi_olddispatch_multicast_fn  recv_func;
      void                         *async_arg;
    }lapi_mcast_dispatch_info_t;

    typedef struct lapi_m2m_dispatch_info_t
    {
      xmi_olddispatch_manytomany_fn  recv_func;
      void                          *async_arg;
    }lapi_m2m_dispatch_info_t;


    template <class T_SysDep>
    class LAPIDevice : public Interface::BaseDevice<LAPIDevice<T_SysDep>, T_SysDep>,
                      public Interface::MessageDevice<LAPIDevice<T_SysDep> >
    {
    public:
      static const size_t packet_payload_size = 224;
      inline LAPIDevice () :
        Interface::BaseDevice<LAPIDevice<T_SysDep>, T_SysDep> (),
        Interface::MessageDevice<LAPIDevice<T_SysDep> >(),
        _dispatch_id(0)
        {
//          LAPI_Comm_size(LAPI_COMM_WORLD, (int*)&_peers);
        };

      // Implement BaseDevice Routines

      inline ~LAPIDevice () {};


      int registerRecvFunction (Interface::RecvFunction_t  recv_func,
                                void                      *recv_func_parm)
        {
          _dispatch_table[_dispatch_id].recv_func=recv_func;
          _dispatch_table[_dispatch_id].recv_func_parm=recv_func_parm;
          _dispatch_lookup[_dispatch_id]=_dispatch_table[_dispatch_id];
          return _dispatch_id++;
        }

      int initMcast()
        {
          return _mcast_dispatch_id++;
        }

      int initM2M()
        {
          return _m2m_dispatch_id++;
        }
      
      void registerMcastRecvFunction (int                           dispatch_id,
                                      xmi_olddispatch_multicast_fn  recv_func,
                                      void                         *async_arg)
        {
          _mcast_dispatch_table[dispatch_id].recv_func=recv_func;
          _mcast_dispatch_table[dispatch_id].async_arg=async_arg;
          _mcast_dispatch_lookup[dispatch_id]=_mcast_dispatch_table[dispatch_id];
        }

      void registerM2MRecvFunction (int                           dispatch_id,
                                    xmi_olddispatch_multicast_fn  recv_func,
                                    void                         *async_arg)
        {
          _m2m_dispatch_table[dispatch_id].recv_func=recv_func;
          _m2m_dispatch_table[dispatch_id].async_arg=async_arg;
          _m2m_dispatch_lookup[dispatch_id]=_m2m_dispatch_table[dispatch_id];
        }

      inline xmi_result_t init_impl (T_SysDep * sysdep)
        {
          assert(0);
	  return XMI_UNIMPL;
        };

      inline bool isInit_impl ()
        {
          assert(0);
	  return false;
        };
      inline bool isReliableNetwork ()
        {
          return true;
        };

      inline int advance_impl ()
        {
        };

      // Implement MessageDevice Routines
      /// \see XMI::Device::Interface::MessageDevice::getMessageMetadataSize()
      static const size_t message_metadata_size = 128;

      inline void   setConnection_impl (int channel, size_t rank, void * arg)
        {
          assert(0);
        }
      inline void * getConnection_impl (int channel, size_t rank)
        {
          assert(0);
	  return NULL;
        }

      // Implement Packet Device Routines
      inline int    readData_impl(void * dst, size_t bytes)
        {
          assert(0);
	  return -1;
        }
      inline bool   requiresRead_impl()
        {
          assert(0);
	  return false;
        }
      inline size_t getPacketMetadataSize_impl()
        {
          return 128;
        }
      inline size_t getPacketPayloadSize_impl()
        {
          return 224;
        }

      inline size_t peers_impl ()
        {
          return _peers;
        }

      inline size_t task2peer_impl (size_t task)
        {
          assert(task < _peers);
          return task;
        }
      inline void enqueue(LAPIMessage* msg)
        {
          _sendQ.push_front(msg);
        }

      inline void enqueue(LAPIMcastMessage* msg)
        {
          _mcastsendQ.push_front(msg);
        }

      inline void enqueue(LAPIMcastRecvMessage *msg)
        {
          _mcastrecvQ.push_front(msg);
        }

      inline void enqueue(LAPIM2MRecvMessage<size_t> *msg)
        {
          _m2mrecvQ.push_front(msg);
        }

      inline void enqueue(LAPIM2MMessage *msg)
        {
          _m2msendQ.push_front(msg);
        }

      size_t                                    _peers;
      size_t                                    _dispatch_id;
      size_t                                    _mcast_dispatch_id;
      size_t                                    _m2m_dispatch_id;
      std::map<int, lapi_dispatch_info_t>        _dispatch_lookup;
      std::map<int, lapi_mcast_dispatch_info_t>  _mcast_dispatch_lookup;
      std::map<int, lapi_m2m_dispatch_info_t>    _m2m_dispatch_lookup;
      std::list<LAPIMessage*>                    _sendQ;
      std::list<LAPIMcastMessage*>               _mcastsendQ;
      std::list<LAPIM2MMessage*>                 _m2msendQ;
      std::list<LAPIMcastRecvMessage*>           _mcastrecvQ;
      std::list<LAPIM2MRecvMessage<size_t> *>    _m2mrecvQ;
      lapi_dispatch_info_t                       _dispatch_table[256];
      lapi_mcast_dispatch_info_t                 _mcast_dispatch_table[256];
      lapi_m2m_dispatch_info_t                   _m2m_dispatch_table[256];
    };
  };
};
#endif // __components_devices_lapi_lapipacketdevice_h__
