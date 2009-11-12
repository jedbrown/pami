/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/lapiunix/lapiunixdevice.h
 * \brief ???
 */

#ifndef __components_devices_lapiunix_lapiunixdevice_h__
#define __components_devices_lapiunix_lapiunixdevice_h__

#include "components/devices/BaseDevice.h"
#include "components/devices/PacketModel.h"
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
                       public Interface::PacketDevice<LAPIDevice<T_SysDep> >
    {
    public:
      inline LAPIDevice () :
        Interface::BaseDevice<LAPIDevice<T_SysDep>, T_SysDep> (),
        Interface::PacketDevice<LAPIDevice<T_SysDep> >(),
        _dispatch_id(0)
        {
        };

      // Implement BaseDevice Routines

      inline ~LAPIDevice ()
        {


        };


      inline void setLapiHandle(lapi_handle_t handle)
        {
          _lapi_handle=handle;
          _g_context_to_device_table[handle]=(void*) this;

          _tf.Util_type = LAPI_GET_THREAD_FUNC;
          CALL_AND_CHECK_RC((LAPI_Util(_lapi_handle, (lapi_util_t *)&_tf)));

          CALL_AND_CHECK_RC((LAPI_Addr_set (_lapi_handle,
                                            (void *)__xmi_lapi_mcast_fn,
                                            1)));

          CALL_AND_CHECK_RC((LAPI_Addr_set (_lapi_handle,
                                            (void *)__xmi_lapi_m2m_fn,
                                            2)));
}

      void lock()
        {
          //MUTEX_LOCK(&_adaptor_lock);
          _tf.mutex_lock(_lapi_handle);
        }
      void unlock()
        {
          //MUTEX_UNLOCK(&_adaptor_lock);
          _tf.mutex_unlock(_lapi_handle);
        }


      int registerRecvFunction (Interface::RecvFunction_t  recv_func,
                                void                      *recv_func_parm)
        {
          assert(recv_func != NULL);
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
          lock();
          LAPI_Msgpoll (_lapi_handle, 5, &info);
          unlock();
        };

      // Implement MessageDevice Routines
      /// \see XMI::Device::Interface::MessageDevice::getMessageMetadataSize()
      static const size_t metadata_size = DEV_HEADER_SIZE;
      static const size_t payload_size  = DEV_PAYLOAD_SIZE;

      // Implement Packet Device Routines
      inline int    read_impl(void * dst, size_t bytes, void * cookie)
        {
          assert(0);
          return -1;
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

      static void __xmi_lapi_m2m_done_fn(lapi_handle_t* handle, void *clientdata)
        {
          LAPIM2MRecvMessage<size_t> *m2m = (LAPIM2MRecvMessage<size_t> *)clientdata;
          m2m->_num--;
          if(m2m->_num==0)
              {
                if( m2m->_done_fn )
                  m2m->_done_fn(NULL, m2m->_cookie,XMI_SUCCESS);
                m2m->_m2mrecvQ->remove(m2m);
                free ( m2m );
              }
        }

      static    void * __xmi_lapi_m2m_fn (lapi_handle_t   * hndl,
                                          void            * uhdr,
                                          uint            * uhdr_len,
                                          ulong           * retinfo,
                                          compl_hndlr_t  ** comp_h,
                                          void           ** uinfo)
        {
          void               *r   = NULL;
          lapi_return_info_t *ri  = (lapi_return_info_t *) retinfo;
          LAPIM2MHeader      *hdr = (LAPIM2MHeader*) uhdr;
          LAPIDevice         *dev = (LAPIDevice*) _g_context_to_device_table[*hndl];

          std::list<LAPIM2MRecvMessage<size_t>*>::iterator it;
          for(it=dev->_m2mrecvQ.begin();it != dev->_m2mrecvQ.end(); it++)
              {
                if((*it)->_conn == hdr->_conn) break;
              }

          lapi_m2m_dispatch_info_t mdi = dev->_m2m_dispatch_lookup[hdr->_dispatch_id];
          LAPIM2MRecvMessage<size_t> * m2m;
          if(it == dev->_m2mrecvQ.end())
              {
                xmi_callback_t    cb_done;
                char            * buf;
                size_t          * sizes;
                size_t          * offsets;
                size_t          * rcvcounters;
                size_t            nranks;
                mdi.recv_func(hdr->_conn,
                              mdi.async_arg,
                              &buf,
                              &offsets,
                              &sizes,
                              &rcvcounters,
                              &nranks,
                              &cb_done);
                m2m = (LAPIM2MRecvMessage<size_t> *)malloc(sizeof(LAPIM2MRecvMessage<size_t>) );
                XMI_assert ( m2m != NULL );
                m2m->_conn    = hdr->_conn;
                m2m->_done_fn = cb_done.function;
                m2m->_cookie  = cb_done.clientdata;
                m2m->_num     = 0;
                for( unsigned i = 0; i < nranks; i++)
                    {
                      if( sizes[i] == 0 ) continue;
                      m2m->_num++;
                    }
                if( m2m->_num == 0 )
                    {
                      if( m2m->_done_fn )
                        (m2m->_done_fn)(NULL, m2m->_cookie,XMI_SUCCESS);
                      free ( m2m );
                      return NULL;
                    }
                m2m->_buf      = buf;
                m2m->_sizes    = sizes;
                m2m->_offsets  = offsets;
                m2m->_nranks   = nranks;
                m2m->_m2mrecvQ = &dev->_m2mrecvQ;
                dev->enqueue(m2m);
              }
          else
              {
                m2m = (*it);
              }
          XMI_assert(m2m != NULL);

          size_t src = hdr->_peer;
          if (ri->udata_one_pkt_ptr)
              {
                unsigned size = hdr->_size < m2m->_sizes[src] ? hdr->_size : m2m->_sizes[src];
                XMI_assert(size>0);
                memcpy(m2m->_buf+m2m->_offsets[src],
                       (void *)ri->udata_one_pkt_ptr,
                       size);
                m2m->_num--;
                if(m2m->_num==0)
                    {
                      if( m2m->_done_fn )
                        m2m->_done_fn(NULL, m2m->_cookie,XMI_SUCCESS);
                      dev->_m2mrecvQ.remove(m2m);
                      free ( m2m );
                    }
                r             = NULL;
                *comp_h       = NULL;
                ri->ret_flags = LAPI_SEND_REPLY;
                ri->ctl_flags = LAPI_BURY_MSG;
              }
          else
              {
                r             = (void*)(m2m->_buf+m2m->_offsets[src]);
                *comp_h       = __xmi_lapi_m2m_done_fn;
                *uinfo        = m2m;
                ri->ret_flags = LAPI_SEND_REPLY;
              }
          return r;
        }

      static void __xmi_lapi_mcast_done_fn(lapi_handle_t* handle, void *clientdata)
        {
          LAPIMcastRecvReq *req = (LAPIMcastRecvReq *)clientdata;
          int bytes = req->_mcast._size - req->_mcast._counter;
          for(; bytes > 0; bytes -= req->_mcast._pwidth)
              {
                req->_mcast._counter += req->_mcast._pwidth;
                if(req->_mcast._done_fn)
                  req->_mcast._done_fn(NULL, req->_mcast._cookie, XMI_SUCCESS);
              }
          if(req->_mcast._counter >= req->_mcast._size)
            req->_mcastrecvQ->remove(&req->_mcast);
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
          _dev->lock();
          std::list<LAPIMcastRecvMessage*>::iterator it;
          int found=0;
          for(it=_dev->_mcastrecvQ.begin();it != _dev->_mcastrecvQ.end(); it++)
              {
                if( (*it)->_conn == msg->_conn &&
                    (*it)->_dispatch_id == msg->_dispatch_id)
                    {
                      found = 1;
                      break;
                    }
              }
          XMI_assert(!(ldi.recv_func==NULL && found!=1));
          LAPIMcastRecvMessage  m_store;
          LAPIMcastRecvMessage *mcast = &m_store;
          if(!found)
              {
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
                it = _dev->_mcastrecvQ.insert(it,mcast);
              }
          else
              {
                mcast = (*it);
              }


          if(mcast->_pwidth == 0 && (mcast->_size == 0||mcast->_buf == 0))
              {
                if(mcast->_done_fn)
                  mcast->_done_fn (&msg->_context, mcast->_cookie, XMI_SUCCESS);
                _dev->_mcastrecvQ.remove(mcast);
                if(found)
                  free (mcast);

                r             = NULL;
                *comp_h       = NULL;
                ri->ret_flags = LAPI_SEND_REPLY;
                ri->ctl_flags = LAPI_BURY_MSG;
                _dev->unlock();
                return NULL;
              }



          if (ri->udata_one_pkt_ptr)
              {
                int remain_bytes   = mcast->_size - mcast->_counter;
                int incoming_bytes = msg->_size;
                memcpy(mcast->_buf + mcast->_counter,
                       (void *)ri->udata_one_pkt_ptr,
                       incoming_bytes);
                mcast->_counter+=incoming_bytes;

                for(; incoming_bytes > 0; incoming_bytes -= mcast->_pwidth)
                    {
                      if(mcast->_done_fn)
                        mcast->_done_fn(&msg->_context, mcast->_cookie, XMI_SUCCESS);
                    }
                if(mcast->_counter >= mcast->_size)
                    {
                      _dev->_mcastrecvQ.remove(mcast);
                      if(found)
                        free (mcast);
                    }
                r             = NULL;
                *comp_h       = NULL;
                ri->ret_flags = LAPI_SEND_REPLY;
                ri->ctl_flags = LAPI_BURY_MSG;
              }
          else
              {
                LAPIMcastRecvReq *req;
                CHECK_NULL(req, (LAPIMcastRecvReq*)malloc(sizeof(LAPIMcastRecvReq)));
                req->_mcastrecvQ        = &_dev->_mcastrecvQ;
                req->_mcast             = *mcast;
                req->_found             = found;
                *it                     = &req->_mcast;
                if(found)
                  free (mcast);

                r                       = (void*)((size_t)mcast->_buf + mcast->_counter);
                *comp_h                 = __xmi_lapi_mcast_done_fn;
                *uinfo                  = (void*)req;
                ri->ret_flags           = LAPI_SEND_REPLY;
                assert(r != NULL);
                if (!r) ri->ctl_flags   = LAPI_BURY_MSG;
              }
          _dev->unlock();
          return r;
        }

      lapi_handle_t                              _lapi_handle;
      lapi_thread_func_t                         _tf;
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
