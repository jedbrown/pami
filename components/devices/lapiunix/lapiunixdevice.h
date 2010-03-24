/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
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
#include "components/devices/PacketInterface.h"
#include "components/devices/lapiunix/lapiunixmessage.h"
#include <map>
#include <list>
#include <sched.h>

namespace PAMI
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
      pami_olddispatch_multicast_fn  recv_func;
      void                         *async_arg;
    }lapi_mcast_dispatch_info_t;

    typedef struct lapi_m2m_dispatch_info_t
    {
      pami_olddispatch_manytomany_fn  recv_func;
      void                          *async_arg;
    }lapi_m2m_dispatch_info_t;

    extern std::map<lapi_handle_t,void*> _g_context_to_device_table;

    class LAPIDevice : public Interface::BaseDevice<LAPIDevice>,
                       public Interface::PacketDevice<LAPIDevice>
    {
    public:
      inline LAPIDevice () :
        Interface::BaseDevice<LAPIDevice> (),
        Interface::PacketDevice<LAPIDevice>(),
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
          CheckLapiRC(lapi_util(_lapi_handle, (lapi_util_t *)&_tf));

          CheckLapiRC(lapi_addr_set (_lapi_handle,
                                            (void *)__pami_lapi_mcast_fn,
                                            1));

          CheckLapiRC(lapi_addr_set (_lapi_handle,
                                            (void *)__pami_lapi_m2m_fn,
                                            2));

          CheckLapiRC(lapi_addr_set (_lapi_handle,
                                            (void *)__pami_lapi_msync_fn,
                                            3));
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
                                      pami_olddispatch_multicast_fn  recv_func,
                                      void                         *async_arg)
        {
          _mcast_dispatch_table[dispatch_id].recv_func=recv_func;
          _mcast_dispatch_table[dispatch_id].async_arg=async_arg;
          _mcast_dispatch_lookup[dispatch_id]=_mcast_dispatch_table[dispatch_id];
        }

      void registerM2MRecvFunction (int                           dispatch_id,
                                    pami_olddispatch_manytomany_fn  recv_func,
                                    void                         *async_arg)
        {
          _m2m_dispatch_table[dispatch_id].recv_func=recv_func;
          _m2m_dispatch_table[dispatch_id].async_arg=async_arg;
          _m2m_dispatch_lookup[dispatch_id]=_m2m_dispatch_table[dispatch_id];
        }

      inline pami_result_t init_impl (PAMI::Memory::MemoryManager         *mm,
                                     size_t          clientid,
                                     size_t          num_ctx,
                                     pami_context_t   context,
                                     size_t          contextid)
        {
          _mm      = mm;
          _context = context;
          _offset  = contextid;
          return PAMI_SUCCESS;
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
          lapi_msgpoll (_lapi_handle, 5, &info);
          unlock();
          return 0;
        };

      // Implement MessageDevice Routines
      /// \see PAMI::Device::Interface::MessageDevice::getMessageMetadataSize()
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

      inline bool isPeer_impl (size_t task)
      {
#if 0
        PAMI::Interface::Mapping::nodeaddr_t node;
        size_t peer;

        __global.mapping.task2node(task,node);
        pami_result_t result = __global.mapping.node2peer(node,peer);

        return result == PAMI_SUCCESS;
#else
        return false;
#endif
      }

      inline void enqueue(LAPIMessage* msg)
        {
          _sendQ.push_front(msg);
        }

      inline void enqueue(OldLAPIMcastMessage* msg)
        {
          _mcastsendQ.push_front(msg);
        }

      inline void enqueue(OldLAPIMcastRecvMessage *msg)
        {
          _mcastrecvQ.push_front(msg);
        }

      inline void enqueue(OldLAPIM2MRecvMessage<size_t> *msg)
        {
          _m2mrecvQ.push_front(msg);
        }

      inline void enqueue(OldLAPIM2MMessage *msg)
        {
          _m2msendQ.push_front(msg);
        }

      inline void enqueue(LAPIMsyncMessage *msg)
        {
          // Check for UE Message
          // If message is there, mark this phase complete
          if(_msyncsendQ[msg->_p2p_msg._connection_id])
              {
                msg->_r_flag = _msyncsendQ[msg->_p2p_msg._connection_id]->_r_flag;
                free(_msyncsendQ[msg->_p2p_msg._connection_id]);
              }
          _msyncsendQ[msg->_p2p_msg._connection_id] = msg;

          if(msg->_r_flag == msg->_total)
              {
                if(msg->_cb_done.function)
                  msg->_cb_done.function(NULL,
                                       msg->_cb_done.clientdata,
                                       PAMI_SUCCESS);
                _msyncsendQ.erase(msg->_p2p_msg._connection_id);
              }
        }

      static void __pami_lapi_m2m_done_fn(lapi_handle_t* handle, void *clientdata)
        {
          OldLAPIM2MRecvMessage<size_t> *m2m = (OldLAPIM2MRecvMessage<size_t> *)clientdata;
          m2m->_num--;
          if(m2m->_num==0)
              {
                if( m2m->_done_fn )
                  m2m->_done_fn(NULL, m2m->_cookie,PAMI_SUCCESS);
                m2m->_m2mrecvQ->remove(m2m);
                free ( m2m );
              }
        }

      static    void * __pami_lapi_m2m_fn (lapi_handle_t   * hndl,
                                          void            * uhdr,
                                          uint            * uhdr_len,
                                          ulong           * retinfo,
                                          compl_hndlr_t  ** comp_h,
                                          void           ** uinfo)
        {
          void               *r   = NULL;
          lapi_return_info_t *ri  = (lapi_return_info_t *) retinfo;
          OldLAPIM2MHeader      *hdr = (OldLAPIM2MHeader*) uhdr;
          LAPIDevice         *dev = (LAPIDevice*) _g_context_to_device_table[*hndl];

          std::list<OldLAPIM2MRecvMessage<size_t>*>::iterator it;
          for(it=dev->_m2mrecvQ.begin();it != dev->_m2mrecvQ.end(); it++)
              {
                if((*it)->_conn == hdr->_conn) break;
              }

          lapi_m2m_dispatch_info_t mdi = dev->_m2m_dispatch_lookup[hdr->_dispatch_id];
          OldLAPIM2MRecvMessage<size_t> * m2m;
          if(it == dev->_m2mrecvQ.end())
              {
                pami_callback_t    cb_done;
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
                m2m = (OldLAPIM2MRecvMessage<size_t> *)malloc(sizeof(OldLAPIM2MRecvMessage<size_t>) );
                PAMI_assert ( m2m != NULL );
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
                        (m2m->_done_fn)(NULL, m2m->_cookie,PAMI_SUCCESS);
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
          PAMI_assert(m2m != NULL);

          size_t src = hdr->_peer;
          if (ri->udata_one_pkt_ptr)
              {
                unsigned size = hdr->_size < m2m->_sizes[src] ? hdr->_size : m2m->_sizes[src];
                PAMI_assert(size>0);
                memcpy(m2m->_buf+m2m->_offsets[src],
                       (void *)ri->udata_one_pkt_ptr,
                       size);
                m2m->_num--;
                if(m2m->_num==0)
                    {
                      if( m2m->_done_fn )
                        m2m->_done_fn(NULL, m2m->_cookie,PAMI_SUCCESS);
                      dev->_m2mrecvQ.remove(m2m);
                      free ( m2m );
                    }
                r             = NULL;
                *comp_h       = NULL;
                ri->ret_flags = LAPI_LOCAL_STATE;
                ri->ctl_flags = LAPI_BURY_MSG;
              }
          else
              {
                r             = (void*)(m2m->_buf+m2m->_offsets[src]);
                *comp_h       = __pami_lapi_m2m_done_fn;
                *uinfo        = m2m;
                ri->ret_flags = LAPI_SEND_REPLY;
              }
          return r;
        }



      static    void * __pami_lapi_msync_fn (lapi_handle_t   * hndl,
                                            void            * uhdr,
                                            uint            * uhdr_len,
                                            ulong           * retinfo,
                                            compl_hndlr_t  ** comp_h,
                                            void           ** uinfo)
        {
          void               *r   = NULL;
          lapi_return_info_t *ri  = (lapi_return_info_t *) retinfo;
          OldLAPIM2MHeader      *hdr = (OldLAPIM2MHeader*) uhdr;
          LAPIDevice         *dev = (LAPIDevice*) _g_context_to_device_table[*hndl];
          unsigned        conn_id = *((unsigned*)uhdr);
          LAPIMsyncMessage     *m = dev->_msyncsendQ[conn_id];

          if(m==NULL)
              {
                m = (LAPIMsyncMessage*)malloc(sizeof(*m));
                m->_r_flag=1;
                m->_total=2147483647;
                dev->_msyncsendQ[conn_id] = m;
              }
          else
              {
                m->_r_flag++;
              }
#if 0
          fprintf(stderr, "conn_id=%d m->_r_flag=%d, m->_total=%d\n",
                  conn_id,
                  m->_r_flag,
                  m->_total);
#endif
          if(m->_r_flag == m->_total)
              {
                if(m->_cb_done.function)
                  m->_cb_done.function(NULL,
                                       m->_cb_done.clientdata,
                                       PAMI_SUCCESS);
                dev->_msyncsendQ.erase(m->_p2p_msg._connection_id);
              }
          r             = NULL;
          *comp_h       = NULL;
          ri->ret_flags = LAPI_LOCAL_STATE;
          ri->ctl_flags = LAPI_BURY_MSG;
          return r;
        }

      static void __pami_lapi_mcast_done_fn(lapi_handle_t* handle, void *clientdata)
        {
          OldLAPIMcastRecvReq *req = (OldLAPIMcastRecvReq *)clientdata;
          int bytes = req->_mcast._size - req->_mcast._counter;
          for(; bytes > 0; bytes -= req->_mcast._pwidth)
              {
                req->_mcast._counter += req->_mcast._pwidth;
                if(req->_mcast._done_fn)
                  req->_mcast._done_fn(NULL, req->_mcast._cookie, PAMI_SUCCESS);
              }
          if(req->_mcast._counter >= req->_mcast._size)
            req->_mcastrecvQ->remove(&req->_mcast);
          free(req);
        }

      static    void * __pami_lapi_mcast_fn (lapi_handle_t   * hndl,
                                            void            * uhdr,
                                            uint            * uhdr_len,
                                            ulong           * retinfo,
                                            compl_hndlr_t  ** comp_h,
                                            void           ** uinfo)
        {
          lapi_return_info_t         *ri          = (lapi_return_info_t *) retinfo;
          OldLAPIMcastMessage           *msg         = (OldLAPIMcastMessage *)   uhdr;
          size_t                      dispatch_id = msg->_dispatch_id;
          void                       *r           = NULL;
          unsigned                    rcvlen;
          char                       *rcvbuf;
          unsigned                    pwidth;
          pami_callback_t              cb_done;

          LAPIDevice *_dev = (LAPIDevice*) _g_context_to_device_table[*hndl];
          lapi_mcast_dispatch_info_t ldi = _dev->_mcast_dispatch_lookup[dispatch_id];
          _dev->lock();
          std::list<OldLAPIMcastRecvMessage*>::iterator it;
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
          PAMI_assert(!(ldi.recv_func==NULL && found!=1));
          OldLAPIMcastRecvMessage  m_store;
          OldLAPIMcastRecvMessage *mcast = &m_store;
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
                mcast->_hint        = PAMI_PT_TO_PT_SUBTASK;
                mcast->_op          = PAMI_UNDEFINED_OP;
                mcast->_dtype       = PAMI_UNDEFINED_DT;
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
                  mcast->_done_fn (&msg->_context, mcast->_cookie, PAMI_SUCCESS);
                _dev->_mcastrecvQ.remove(mcast);
                if(found)
                  free (mcast);

                r             = NULL;
                *comp_h       = NULL;
                ri->ret_flags = LAPI_LOCAL_STATE;
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
                        mcast->_done_fn(&msg->_context, mcast->_cookie, PAMI_SUCCESS);
                    }
                if(mcast->_counter >= mcast->_size)
                    {
                      _dev->_mcastrecvQ.remove(mcast);
                      if(found)
                        free (mcast);
                    }
                r             = NULL;
                *comp_h       = NULL;
                ri->ret_flags = LAPI_LOCAL_STATE;
                ri->ctl_flags = LAPI_BURY_MSG;
              }
          else
              {
                OldLAPIMcastRecvReq *req;
                CheckNULL(req, (OldLAPIMcastRecvReq*)malloc(sizeof(OldLAPIMcastRecvReq)));
                req->_mcastrecvQ        = &_dev->_mcastrecvQ;
                req->_mcast             = *mcast;
                req->_found             = found;
                *it                     = &req->_mcast;
                if(found)
                  free (mcast);

                r                       = (void*)((size_t)mcast->_buf + mcast->_counter);
                *comp_h                 = __pami_lapi_mcast_done_fn;
                *uinfo                  = (void*)req;
                ri->ret_flags           = LAPI_LOCAL_STATE;
                assert(r != NULL);
                if (!r) ri->ctl_flags   = LAPI_BURY_MSG;
              }
          _dev->unlock();
          return r;
        }

      inline pami_context_t getContext_impl ()
        {
          //  \todo Implement this
          return NULL;
        }

      inline size_t getContextOffset_impl ()
        {

          //  \todo Implement this
          return 0;
        }

      PAMI::Memory::MemoryManager                *_mm;
      pami_context_t                              _context;
      size_t                                     _offset;
      lapi_handle_t                              _lapi_handle;
      lapi_thread_func_t                         _tf;
      size_t                                     _peers;
      size_t                                     _dispatch_id;
      size_t                                     _mcast_dispatch_id;
      size_t                                     _m2m_dispatch_id;
      std::map<int, lapi_dispatch_info_t>        _dispatch_lookup;
      std::map<int, lapi_mcast_dispatch_info_t>  _mcast_dispatch_lookup;
      std::map<int, lapi_m2m_dispatch_info_t>    _m2m_dispatch_lookup;
      std::list<LAPIMessage*>                    _sendQ;
      std::list<OldLAPIMcastMessage*>            _mcastsendQ;
      std::list<OldLAPIM2MMessage*>                 _m2msendQ;
      std::list<OldLAPIMcastRecvMessage*>        _mcastrecvQ;
      std::list<OldLAPIM2MRecvMessage<size_t> *>    _m2mrecvQ;
      std::map<int,LAPIMsyncMessage*>            _msyncsendQ;
      lapi_dispatch_info_t                       _dispatch_table[256];
      lapi_mcast_dispatch_info_t                 _mcast_dispatch_table[256];
      lapi_m2m_dispatch_info_t                   _m2m_dispatch_table[256];
    };




  };
};
#endif // __components_devices_lapi_lapipacketdevice_h__
