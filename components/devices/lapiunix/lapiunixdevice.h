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


    extern std::map<lapi_handle_t,void*> _g_context_to_device_table;


    template <class T_SysDep>
    class LAPIDevice : public Interface::BaseDevice<LAPIDevice<T_SysDep>, T_SysDep>,
                       public Interface::MessageDevice<LAPIDevice<T_SysDep> >
    {
    public:
      static const size_t packet_payload_size = DEV_PAYLOAD_SIZE;
      inline LAPIDevice () :
        Interface::BaseDevice<LAPIDevice<T_SysDep>, T_SysDep> (),
        Interface::MessageDevice<LAPIDevice<T_SysDep> >(),
        _dispatch_id(0)
        {
//          LAPI_Comm_size(LAPI_COMM_WORLD, (int*)&_peers);
        };

      // Implement BaseDevice Routines

      inline ~LAPIDevice ()
        {
          

        };


      void setLapiHandle(lapi_handle_t handle)
        {
          _lapi_handle=handle;
          _g_context_to_device_table[handle]=(void*) this;

          CALL_AND_CHECK_RC((LAPI_Addr_set (_lapi_handle,
                                            (void *)__xmi_lapi_mcast_fn,
                                            1)));
        }




      int registerRecvFunction (Interface::RecvFunction_t  recv_func,
                                void                      *recv_func_parm)
        {
          _dispatch_table[_dispatch_id].recv_func=recv_func;
          _dispatch_table[_dispatch_id].recv_func_parm=recv_func_parm;
          _dispatch_lookup[_dispatch_id]=_dispatch_table[_dispatch_id];
          return 0+_dispatch_id++;
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
          _sysdep = sysdep;
	  return XMI_SUCCESS;
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
          lapi_msg_info_t info;
          LAPI_Msgpoll (_lapi_handle, 5, &info);
        };

      // Implement MessageDevice Routines
      /// \see XMI::Device::Interface::MessageDevice::getMessageMetadataSize()
      static const size_t message_metadata_size = DEV_HEADER_SIZE;

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
          return DEV_HEADER_SIZE;
        }
      inline size_t getPacketPayloadSize_impl()
        {
          return DEV_PAYLOAD_SIZE;
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


      static void __xmi_lapi_mcast_done_fn(lapi_handle_t* handle, void *clientdata)
        {
          LAPIMcastRecvReq *req = (LAPIMcastRecvReq *)clientdata;
          if(req->_user_done.function)
            req->_user_done.function(NULL,
                                     req->_user_done.clientdata,
                                     XMI_SUCCESS);
          free(req);
        }
      
      static    void * __xmi_lapi_mcast_fn (lapi_handle_t   * hndl,
                                            void            * uhdr,
                                            uint            * uhdr_len,
                                            ulong           * retinfo,
                                            compl_hndlr_t  ** comp_h,
                                            void           ** uinfo)
        {
          lapi_return_info_t         *ri          = (lapi_return_info_t *) retinfo;
          LAPIMcastMessage           *msg         = (LAPIMcastMessage *)   uhdr;
          size_t                      dispatch_id = msg->_dispatch_id;
          void                       *r           = NULL;
          unsigned                    rcvlen;
          char                       *rcvbuf;
          unsigned                    pwidth;
          xmi_callback_t              cb_done;

          LAPIDevice *_dev = (LAPIDevice*) _g_context_to_device_table[*hndl];
          lapi_mcast_dispatch_info_t ldi = _dev->_mcast_dispatch_lookup[dispatch_id];

          LAPIMcastRecvMessage  m_store;
          LAPIMcastRecvMessage *mcast = &m_store;
          ldi.recv_func (&msg->_info[0],
                         msg->_info_count,
                         msg->_peer,
                         msg->_size,
                         msg->_conn,
                         ldi.async_arg,
                         &rcvlen,
                         &rcvbuf,
                         &pwidth,
                         &cb_done);
          assert(rcvlen <= (size_t)msg->_size);
          mcast->_conn        = msg->_conn;
          mcast->_done_fn     = cb_done.function;
          mcast->_cookie      = cb_done.clientdata;
          mcast->_buf         = rcvbuf;
          mcast->_size        = rcvlen;
          mcast->_pwidth      = pwidth;
          mcast->_hint        = XMI_PT_TO_PT_SUBTASK;
          mcast->_op          = XMI_UNDEFINED_OP;
          mcast->_dtype       = XMI_UNDEFINED_DT;
          mcast->_counter     = 0;
          mcast->_dispatch_id = dispatch_id;
          
          if (ri->udata_one_pkt_ptr)
              {
                if (r && ri->msg_len)
                  memcpy(rcvbuf,
                         (void *)ri->udata_one_pkt_ptr,
                         ri->msg_len);
                ri->ret_flags = LAPI_SEND_REPLY;                
                ri->ctl_flags = LAPI_BURY_MSG;
                if(cb_done.function)
                  cb_done.function(NULL, cb_done.clientdata, XMI_SUCCESS);
                return NULL;
              }
          else
              {
                LAPIMcastRecvReq *req;
                CHECK_NULL(req, (LAPIMcastRecvReq*)malloc(sizeof(LAPIMcastRecvReq)));
                req->_user_done         = cb_done;
                r                       = (void*)mcast->_buf;
                *comp_h                 = __xmi_lapi_mcast_done_fn;
                *uinfo                  = (void*)req;
                ri->ret_flags           = LAPI_SEND_REPLY;
                if (!r) ri->ctl_flags   = LAPI_BURY_MSG;
              }
          return r;
        }

      static    void * __xmi_lapi_amSendRequestHandler (lapi_handle_t   * hndl,
                                                        void            * uhdr,
                                                        uint            * uhdr_len,
                                                        ulong           * retinfo,
                                                        compl_hndlr_t  ** comp_h,
                                                        void           ** uinfo)
        {
          lapi_return_info_t        * ri = (lapi_return_info_t *) retinfo;
          LAPIP2PMessage            * hi = (LAPIP2PMessage *) uhdr;
          void                       * r = NULL;
          size_t                      hh =  hi->_dispatch_id;

          LAPIDevice *_dev = (LAPIDevice*) _g_context_to_device_table[*hndl];
          Interface::RecvFunction_t  recv_func;
          void                      *recv_func_parm;
          recv_func      = _dev->_dispatch_table[hh].recv_func;
          recv_func_parm = _dev->_dispatch_table[hh].recv_func_parm;

          if (!hh) XMI_abort();
          recv_func(hi->_metadata,
                    hi->_payload,
                    hi->_payloadsize0+hi->_payloadsize1,
                    recv_func_parm);

          if (ri->udata_one_pkt_ptr)
              {
                if (r && ri->msg_len)
                  memcpy(r,(void *)ri->udata_one_pkt_ptr,ri->msg_len);
                ri->ret_flags = LAPI_SEND_REPLY;
                ri->ctl_flags = LAPI_BURY_MSG;
                return NULL;
              }
          else
              {
                ri->ret_flags = LAPI_SEND_REPLY;
                if (!r) ri->ctl_flags = LAPI_BURY_MSG;
              }
          return r;
        }

      lapi_handle_t                              _lapi_handle;
      T_SysDep                                  *_sysdep;
      size_t                                     _peers;
      size_t                                     _dispatch_id;
      size_t                                     _mcast_dispatch_id;
      size_t                                     _m2m_dispatch_id;
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
