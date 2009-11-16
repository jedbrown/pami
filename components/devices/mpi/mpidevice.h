/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/mpi/mpidevice.h
 * \brief ???
 */

#ifndef __components_devices_mpi_mpidevice_h__
#define __components_devices_mpi_mpidevice_h__

#include "components/devices/BaseDevice.h"
#include "components/devices/PacketInterface.h"
#include "components/devices/mpi/mpimessage.h"
#include <map>
#include <list>
#include "util/ccmi_debug.h"

#define DISPATCH_SET_SIZE 256
namespace XMI
{
  namespace Device
  {
    typedef struct mpi_dispatch_info_t
    {
      Interface::RecvFunction_t  recv_func;
      void                      *recv_func_parm;
    }mpi_dispatch_info_t;

    typedef struct mpi_mcast_dispatch_info_t
    {
      xmi_olddispatch_multicast_fn  recv_func;
      void                         *async_arg;
    }mpi_mcast_dispatch_info_t;

    typedef struct mpi_m2m_dispatch_info_t
    {
      xmi_olddispatch_manytomany_fn  recv_func;
      void                          *async_arg;
    }mpi_m2m_dispatch_info_t;


    template <class T_SysDep>
    class MPIDevice : public Interface::BaseDevice<MPIDevice<T_SysDep>, T_SysDep>,
                      public Interface::PacketDevice<MPIDevice<T_SysDep> >
    {
    public:
      static const size_t packet_payload_size = 224;
      inline MPIDevice () :
      Interface::BaseDevice<MPIDevice<T_SysDep>, T_SysDep> (),
      Interface::PacketDevice<MPIDevice<T_SysDep> >(),
      _dispatch_id(0)
      {
        MPI_Comm_size(MPI_COMM_WORLD, (int*)&_peers);
        TRACE_ADAPTOR((stderr,"<%#.8X>MPIDevice()\n",(int)this));
      };

      // Implement BaseDevice Routines

      inline ~MPIDevice () {};


      int registerRecvFunction (size_t                     dispatch,
                                Interface::RecvFunction_t  recv_func,
                                void                      *recv_func_parm)
        {
          unsigned i;
          for (i=0; i<DISPATCH_SET_SIZE; i++)
          {
            unsigned id = dispatch * DISPATCH_SET_SIZE + i;
            if (_dispatch_table[id].recv_func == NULL)
            {
              _dispatch_table[id].recv_func=recv_func;
              _dispatch_table[id].recv_func_parm=recv_func_parm;
              _dispatch_lookup[id] = _dispatch_table[id];

              return id;
            }
          }
          return -1;
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
        TRACE_ADAPTOR((stderr,"<%#.8X>MPIDevice::registerMcastRecvFunction %d\n",(int)this,_dispatch_id));
      }

      void registerM2MRecvFunction (int                           dispatch_id,
                                    xmi_olddispatch_multicast_fn  recv_func,
                                    void                         *async_arg)
      {
        _m2m_dispatch_table[dispatch_id].recv_func=recv_func;
        _m2m_dispatch_table[dispatch_id].async_arg=async_arg;
        _m2m_dispatch_lookup[dispatch_id]=_m2m_dispatch_table[dispatch_id];
        TRACE_ADAPTOR((stderr,"<%#.8X>MPIDevice::registerM2MRecvFunction %d\n",(int)this,_dispatch_id));
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
        static int dbg = 1;
        int flag = 0;
        MPI_Status sts;
        int events=0;

        if(dbg) {
          TRACE_ADAPTOR((stderr,"<%#.8X>MPIDevice::advance_impl\n",(int)this));
          dbg = 0;
        }
#ifdef EMULATE_NONDETERMINISTIC_DEVICE
        // Check the P2P *pending* send queue
        while (!_pendingQ.empty())
        {
          MPIMessage * msg = _pendingQ.front();
          _pendingQ.pop_front();
          MPI_Isend (&msg->_p2p_msg,
                     sizeof(msg->_p2p_msg),
                     MPI_CHAR,
                     msg->_target_task,
                     0,
                     MPI_COMM_WORLD,
                     &msg->_request);
          enqueue(msg);
        }
#endif
        // Check the P2P send queue
        std::list<MPIMessage*>::iterator it_p2p;
        for(it_p2p=_sendQ.begin();it_p2p != _sendQ.end(); it_p2p++)
        {
          flag            = 0;
          MPI_Testall(1,&((*it_p2p)->_request),&flag,MPI_STATUSES_IGNORE);
          if(flag)
          {
            events++;
            TRACE_ADAPTOR((stderr,"<%#.8X>MPIDevice::advance_impl() p2p\n",(int)this)); dbg = 1;
            xmi_event_function  done_fn = (*it_p2p)->_done_fn;
            void               *cookie  = (*it_p2p)->_cookie;
            xmi_client_t       client = (*it_p2p)->_client;
            size_t       context = (*it_p2p)->_context;
            _sendQ.remove((*it_p2p));
            if((*it_p2p)->_freeme)
              free(*it_p2p);

            if(done_fn)
              done_fn(client,context,cookie,XMI_SUCCESS);
            break;
          }
        }
        // Check the Multicast send queue
        std::list<MPIMcastMessage*>::iterator it_mcast;
        for(it_mcast=_mcastsendQ.begin();it_mcast != _mcastsendQ.end(); it_mcast++)
        {
          int numStatuses = (*it_mcast)->_num;
          flag            = 0;
          MPI_Testall(numStatuses,(*it_mcast)->_req,&flag,MPI_STATUSES_IGNORE);
          if(flag)
          {
            events++;
            TRACE_ADAPTOR((stderr,"<%#.8X>MPIDevice::advance_impl mc\n",(int)this)); dbg = 1;
            if((*it_mcast)->_cb_done.function )
              (*(*it_mcast)->_cb_done.function)((*it_mcast)->_client,(*it_mcast)->_context, (*it_mcast)->_cb_done.clientdata, XMI_SUCCESS);
            free ((*it_mcast)->_req);
            free (*it_mcast);
            _mcastsendQ.remove((*it_mcast));
            break;
          }
        }
        // Check the M2M send Queue
        std::list<MPIM2MMessage*>::iterator it;
        for(it=_m2msendQ.begin();it != _m2msendQ.end(); it++)
        {
          int numStatuses = (*it)->_num;
          flag            = 0;
          MPI_Testall(numStatuses,(*it)->_reqs,&flag,MPI_STATUSES_IGNORE);
          if(flag)
          {
            events++;
            TRACE_ADAPTOR((stderr,"<%#.8X>MPIDevice::advance_impl m2m\n",(int)this)); dbg = 1;
            if((*it)->_done_fn )
              ((*it)->_done_fn)(NULL, 0, (*it)->_cookie, XMI_SUCCESS);

            free ((*it)->_reqs);
            free ((*it)->_bufs);
            _m2msendQ.remove((*it));
            free (*it);
            break;
          }
        }



        flag = 0;
        int rc = MPI_Iprobe (MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &sts);
        assert (rc == MPI_SUCCESS);
        if(flag)
        {
          events++;
          TRACE_ADAPTOR((stderr,"<%#.8X>MPIDevice::advance_impl MPI_Iprobe %d\n",(int)this,sts.MPI_TAG)); dbg = 1;
          //p2p messages
          switch(sts.MPI_TAG)
          {
          case 0:
            {
              int nbytes = 0;
              MPI_Get_count(&sts, MPI_BYTE, &nbytes);
              MPIMessage *msg = (MPIMessage *) malloc (sizeof(*msg));
              int rc = MPI_Recv(&msg->_p2p_msg,nbytes,MPI_BYTE,sts.
                                MPI_SOURCE,sts.MPI_TAG,
                                MPI_COMM_WORLD,&sts);
              assert(rc == MPI_SUCCESS);
              size_t dispatch_id      = msg->_p2p_msg._dispatch_id;
              TRACE_ADAPTOR((stderr,"<%#.8X>MPIDevice::advance_impl MPI_Recv nbytes %d, dispatch_id %zd\n",
                             (int)this, nbytes,dispatch_id));
              _currentBuf = msg->_p2p_msg._payload;
              mpi_dispatch_info_t mdi = _dispatch_lookup[dispatch_id];
              if(mdi.recv_func)
                mdi.recv_func(msg->_p2p_msg._metadata,
                              msg->_p2p_msg._payload,
                              msg->_p2p_msg._payloadsize0+msg->_p2p_msg._payloadsize1,
                              mdi.recv_func_parm,
                              NULL);
              free(msg);
            }
            break;
          case 1:
            {
              int nbytes = 0;
              MPI_Get_count(&sts, MPI_BYTE, &nbytes);
              MPIMessage *msg = (MPIMessage *) malloc (sizeof(*msg)+nbytes);
              int rc = MPI_Recv(&msg->_p2p_msg,nbytes,MPI_BYTE,sts.
                                MPI_SOURCE,sts.MPI_TAG,
                                MPI_COMM_WORLD,&sts);
              assert(rc == MPI_SUCCESS);
              size_t dispatch_id      = msg->_p2p_msg._dispatch_id;
              TRACE_ADAPTOR((stderr,"<%#.8X>MPIDevice::advance_impl MPI_Recv nbytes %d, dispatch_id %zd\n",
                             (int)this, nbytes,dispatch_id));
              _currentBuf = (char*)msg->_p2p_msg._metadata+msg->_p2p_msg._metadatasize;
              mpi_dispatch_info_t mdi = _dispatch_lookup[dispatch_id];
              if(mdi.recv_func)
                mdi.recv_func(msg->_p2p_msg._metadata,
                              (char*)msg->_p2p_msg._metadata+msg->_p2p_msg._metadatasize,
                              msg->_p2p_msg._payloadsize0+msg->_p2p_msg._payloadsize1,
                              mdi.recv_func_parm,
                              NULL);
              free(msg);
            }
            break;
          case 2:
            {
              int nbytes = 0;
              MPI_Get_count(&sts, MPI_BYTE, &nbytes);
              MPIMcastMessage *msg = (MPIMcastMessage *) malloc (nbytes);
              assert(msg != NULL);
              int rc = MPI_Recv(msg,nbytes,MPI_BYTE,sts.MPI_SOURCE,sts.MPI_TAG, MPI_COMM_WORLD,&sts);
              XMI_assert (rc == MPI_SUCCESS);
              unsigned         rcvlen;
              char           * rcvbuf;
              unsigned         pwidth;
              xmi_callback_t   cb_done;
              size_t dispatch_id      = msg->_dispatch_id;
              TRACE_ADAPTOR((stderr,"<%#.8X>MPIDevice::advance_impl MPI_Recv nbytes %d, dispatch_id %zd\n",
                             (int)this, nbytes,dispatch_id));
              mpi_mcast_dispatch_info_t mdi = _mcast_dispatch_lookup[dispatch_id];

              MPIMcastRecvMessage *mcast;
              std::list<MPIMcastRecvMessage*>::iterator it;
              int found=0;
              for(it=_mcastrecvQ.begin();it != _mcastrecvQ.end(); it++)
              {
                if( (*it)->_conn == msg->_conn &&
                    (*it)->_dispatch_id == msg->_dispatch_id)
                {
                  found = 1;
                  break;
                }
              }
              MPIMcastRecvMessage _m_store;
              if( !found )
              {
                XMI_assert (mdi.recv_func != NULL);

                mdi.recv_func (&msg->_info[0],
                               msg->_info_count,
                               sts.MPI_SOURCE,
                               msg->_size,
                               msg->_conn,
                               mdi.async_arg,
                               &rcvlen,
                               &rcvbuf,
                               &pwidth,
                               &cb_done);
                assert(rcvlen <= (size_t)msg->_size);
//                          mcast = (MPIMcastRecvMessage*)malloc(sizeof(*mcast));
                mcast = &_m_store;
                assert(mcast != NULL);
                mcast->_conn     = msg->_conn;
                mcast->_done_fn  = cb_done.function;
                mcast->_cookie   = cb_done.clientdata;
                mcast->_buf      = rcvbuf;
                mcast->_size     = rcvlen;
                mcast->_pwidth   = pwidth;
                mcast->_hint     = XMI_PT_TO_PT_SUBTASK;
                mcast->_op       = XMI_UNDEFINED_OP;
                mcast->_dtype    = XMI_UNDEFINED_DT;
                mcast->_counter = 0;
                mcast->_dispatch_id = dispatch_id;
                enqueue(mcast);
              }
              else
              {
                mcast = (*it);
              }

              if(mcast->_pwidth == 0 && (mcast->_size == 0||mcast->_buf == 0))
              {
                if(mcast->_done_fn)
                  mcast->_done_fn (msg->_client, msg->_context, mcast->_cookie, XMI_SUCCESS);

                _mcastrecvQ.remove(mcast);
                free (msg);
                if(found)
                  free (mcast);

                break;
              }

              int bytes = mcast->_size - mcast->_counter;
              if(bytes > msg->_size) bytes = msg->_size;
              if(mcast->_size)
                memcpy (mcast->_buf + mcast->_counter, msg->buffer(), bytes);

              //printf ("dispatch %d matched posted receive %d %d %d %d\n",
              //	    dispatch_id,
              //	    nbytes, mcast->_pwidth, mcast->_counter,
              //	    mcast->_size);

              //for(unsigned count = 0; count < mcast->_size; count += mcast->_pwidth)
              //if(mcast->_done_fn)
              //  mcast->_done_fn(msg->_client, msg->context, mcast->_cookie, XMI_SUCCESS);

              // XMI_assert (nbytes <= mcast->_pwidth);

              for(; bytes > 0; bytes -= mcast->_pwidth)
              {
                TRACE_ADAPTOR((stderr,"<%#.8X>MPIDevice::calling done counter %zd, pwidth %zd, bytes %zd, size %zd\n",
                               (int)this, mcast->_counter,mcast->_pwidth, bytes, mcast->_size));
                mcast->_counter += mcast->_pwidth;
                if(mcast->_done_fn)
                  mcast->_done_fn(msg->_client, msg->_context, mcast->_cookie, XMI_SUCCESS);
              }

              if(mcast->_counter >= mcast->_size)
              {
                _mcastrecvQ.remove(mcast);
                if(found)
                  free (mcast);
              }

              free (msg);
            }
            break;
          case 3:
            {
              int nbytes = 0;
              MPI_Get_count(&sts, MPI_BYTE, &nbytes);
              MPIM2MHeader *msg = (MPIM2MHeader *) malloc (nbytes);
              int rc            = MPI_Recv(msg,nbytes,MPI_BYTE,sts.MPI_SOURCE,sts.MPI_TAG, MPI_COMM_WORLD,&sts);
              XMI_assert (rc == MPI_SUCCESS);
              TRACE_ADAPTOR((stderr,"<%#.8X>MPIDevice::advance_impl MPI_Recv nbytes %d\n",
                             (int)this, nbytes));

              std::list<MPIM2MRecvMessage<size_t>*>::iterator it;
              for(it=_m2mrecvQ.begin();it != _m2mrecvQ.end(); it++)
              {
                if( (*it)->_conn == msg->_conn ) break;
              }

              mpi_m2m_dispatch_info_t mdi = _m2m_dispatch_lookup[msg->_dispatch_id];
              MPIM2MRecvMessage<size_t> * m2m;
              if( it == _m2mrecvQ.end() )
              {
                xmi_callback_t    cb_done;
                char            * buf;
                size_t        * sizes;
                size_t        * offsets;
                size_t        * rcvcounters;
                size_t          nranks;
                mdi.recv_func(msg->_conn,
                              mdi.async_arg,
                              &buf,
                              &offsets,
                              &sizes,
                              &rcvcounters,
                              &nranks,
                              &cb_done );
                m2m = (MPIM2MRecvMessage<size_t> *)malloc(sizeof(MPIM2MRecvMessage<size_t>) );
                XMI_assert ( m2m != NULL );
                m2m->_conn = msg->_conn;
                m2m->_done_fn = cb_done.function;
                m2m->_cookie  = cb_done.clientdata;
                m2m->_num = 0;
                for( unsigned i = 0; i < nranks; i++)
                {
                  if( sizes[i] == 0 ) continue;
                  m2m->_num++;
                }
                if( m2m->_num == 0 )
                {
                  if( m2m->_done_fn )
                    (m2m->_done_fn)(NULL, 0, m2m->_cookie,XMI_SUCCESS);
                  free ( m2m );
                  return NULL;
                }
                m2m->_buf     = buf;
                m2m->_sizes   = sizes;
                m2m->_offsets = offsets;
                m2m->_nranks  = nranks;
                enqueue(m2m);
              }
              else
              {
                m2m = (*it);
              }
              unsigned src = sts.MPI_SOURCE;
              if( m2m )
              {
                unsigned size = msg->_size < m2m->_sizes[src] ? msg->_size : m2m->_sizes[src];
                XMI_assert( size > 0 );
                memcpy( m2m->_buf + m2m->_offsets[src], msg->buffer(), size );
                m2m->_num--;
                if( m2m->_num == 0 )
                {
                  if( m2m->_done_fn )
                  {
                    m2m->_done_fn(NULL, 0, m2m->_cookie,XMI_SUCCESS);
                  }
                  _m2mrecvQ.remove(m2m);
                  free ( m2m );
                }
              }
              free ( msg );
            }
            break;
          }
        }
        return events;
      };

      // Implement MessageDevice Routines
      static const size_t metadata_size = 128;
      static const size_t payload_size  = 224;

      // Implement Packet Device Routines
      inline int    read_impl(void * dst, size_t bytes, void * cookie)
      {
        memcpy(dst, _currentBuf, bytes);
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
      inline void enqueue(MPIMessage* msg)
      {
        TRACE_ADAPTOR((stderr,"<%#.8X>MPIDevice::enqueue message size 0 %zd, size 1 %zd, msize %zd\n",(int)this, (size_t)msg->_p2p_msg._payloadsize0,(size_t)msg->_p2p_msg._payloadsize1,(size_t)msg->_p2p_msg._metadatasize));
        _sendQ.push_front(msg);
      }

      inline void enqueue(MPIMcastMessage* msg)
      {
        TRACE_ADAPTOR((stderr,"<%#.8X>MPIDevice::enqueue mcast message size %zd\n",(int)this, (size_t)msg->_size));
        _mcastsendQ.push_front(msg);
      }

      inline void enqueue(MPIMcastRecvMessage *msg)
      {
        TRACE_ADAPTOR((stderr,"<%#.8X>MPIDevice::enqueue mcast recv message pwidth %zd size %zd\n",(int)this, (size_t)msg->_pwidth, (size_t)msg->_size));
        _mcastrecvQ.push_front(msg);
      }

      inline void enqueue(MPIM2MRecvMessage<size_t> *msg)
      {
        TRACE_ADAPTOR((stderr,"<%#.8X>MPIDevice::enqueue m2m recv message size %zd\n",(int)this, (size_t)msg->_sizes[0]));
        _m2mrecvQ.push_front(msg);
      }

      inline void enqueue(MPIM2MMessage *msg)
      {
        TRACE_ADAPTOR((stderr,"<%#.8X>MPIDevice::enqueue m2m message total size %zd\n",(int)this, (size_t)msg->_totalsize));
        _m2msendQ.push_front(msg);
      }

      inline void addToNonDeterministicQueue(MPIMessage* msg)
      {
        size_t index, insert = __global.time.timebase() % _pendingQ.size();
        std::list<MPIMessage*>::iterator it;
        for (index = 0; index < insert; index++) it++;
        _pendingQ.insert(it,msg);
      }

      char                                     *_currentBuf;
      size_t                                    _peers;
      size_t                                    _dispatch_id;
      size_t                                    _mcast_dispatch_id;
      size_t                                    _m2m_dispatch_id;
      std::map<int, mpi_dispatch_info_t>        _dispatch_lookup;
      std::map<int, mpi_mcast_dispatch_info_t>  _mcast_dispatch_lookup;
      std::map<int, mpi_m2m_dispatch_info_t>    _m2m_dispatch_lookup;
      std::list<MPIMessage*>                    _sendQ;
      std::list<MPIMcastMessage*>               _mcastsendQ;
      std::list<MPIM2MMessage*>                 _m2msendQ;
      std::list<MPIMcastRecvMessage*>           _mcastrecvQ;
      std::list<MPIM2MRecvMessage<size_t> *>    _m2mrecvQ;
      std::list<MPIMessage*>                    _pendingQ;
      mpi_dispatch_info_t                       _dispatch_table[256*DISPATCH_SET_SIZE];
      mpi_mcast_dispatch_info_t                 _mcast_dispatch_table[256];
      mpi_m2m_dispatch_info_t                   _m2m_dispatch_table[256];
    };
  };
};
#endif // __components_devices_mpi_mpipacketdevice_h__
