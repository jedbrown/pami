/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
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
#include "components/devices/MessageDevice.h"
#include "components/devices/mpi/mpimessage.h"
#include <map>
#include <list>

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


    template <class T_SysDep>
    class MPIDevice : public Interface::BaseDevice<MPIDevice<T_SysDep>, T_SysDep>,
                      public Interface::MessageDevice<MPIDevice<T_SysDep> >
    {
    public:
      static const size_t packet_payload_size = 224;
      inline MPIDevice () :
        Interface::BaseDevice<MPIDevice<T_SysDep>, T_SysDep> (),
        Interface::MessageDevice<MPIDevice<T_SysDep> >(),
        _dispatch_id(0)
        {
          MPI_Comm_size(MPI_COMM_WORLD, (int*)&_peers);
        };

      // Implement BaseDevice Routines

      inline ~MPIDevice () {};


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
      
      void registerMcastRecvFunction (int                           dispatch_id,
                                     xmi_olddispatch_multicast_fn  recv_func,
                                     void                         *async_arg)
        {
          _mcast_dispatch_table[dispatch_id].recv_func=recv_func;
          _mcast_dispatch_table[dispatch_id].async_arg=async_arg;
          _mcast_dispatch_lookup[dispatch_id]=_mcast_dispatch_table[dispatch_id];
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
          int flag = 0;
          MPI_Status sts;
          std::list<MPIMessage*>::iterator it_p2p;
          for(it_p2p=_sendQ.begin();it_p2p != _sendQ.end(); it_p2p++)
              {
                flag            = 0;
                MPI_Testall(1,&((*it_p2p)->_request),&flag,MPI_STATUSES_IGNORE);
                if(flag)
                    {
                      if((*it_p2p)->_done_fn )
                        (*(*it_p2p)->_done_fn)((*it_p2p)->_context,(*it_p2p)->_cookie, XMI_SUCCESS);
                      _sendQ.remove((*it_p2p));
                      break;
                    }
              }
          std::list<MPIMcastMessage*>::iterator it_mcast;
          for(it_mcast=_mcastsendQ.begin();it_mcast != _mcastsendQ.end(); it_mcast++)
	    {
	      int numStatuses = (*it_mcast)->_num;
	      flag            = 0;
	      MPI_Testall(numStatuses,(*it_mcast)->_req,&flag,MPI_STATUSES_IGNORE);
	      if(flag)
		{
		  if((*it_mcast)->_cb_done.function )
		    (*(*it_mcast)->_cb_done.function)((*it_p2p)->_context, (*it_mcast)->_cb_done.clientdata, XMI_SUCCESS);
		  free ((*it_mcast)->_req);
		  free (*it_mcast);
		  _mcastsendQ.remove((*it_mcast));
		  break;
		}
	    }

          flag = 0;
          int rc = MPI_Iprobe (MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &sts);
          assert (rc == MPI_SUCCESS);
          if(flag)
	    {
	      //p2p messages
	      switch(sts.MPI_TAG)
		{
		case 0:
		  {
		    int nbytes = 0;
		    MPI_Get_count(&sts, MPI_BYTE, &nbytes);
		    MPIMessage *msg = (MPIMessage *) malloc (nbytes);
		    int rc = MPI_Recv(&msg->_p2p_msg,nbytes,MPI_BYTE,sts.
				      MPI_SOURCE,sts.MPI_TAG,
				      MPI_COMM_WORLD,&sts);
		    assert(rc == MPI_SUCCESS);
		    size_t dispatch_id      = msg->_p2p_msg._dispatch_id;
		    mpi_dispatch_info_t mdi = _dispatch_lookup[dispatch_id];
		    if(mdi.recv_func)
		      mdi.recv_func(msg->_p2p_msg._metadata,
				    msg->_p2p_msg._payload,
				    msg->_p2p_msg._payloadsize,
				    mdi.recv_func_parm);
		    free(msg);
		  }
		  break;
		case 1:
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
			  XMI_assert (mpi.recv_func != NULL);
			  
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

		    if(mcast->_pwidth == 0 && mcast->_buf == 0) {
		      if(mcast->_done_fn)
                        mcast->_done_fn (&msg->_context, mcast->_cookie, XMI_SUCCESS);
		      
		      _mcastrecvQ.remove(mcast);
		      free (msg);
		      if(found)
			free (mcast);

		      break;
		    }

		    int bytes = mcast->_size - mcast->_counter;
		    if (bytes > msg->_size) bytes = msg->_size;
		    if(mcast->_size)
		      memcpy (mcast->_buf + mcast->_counter, msg->buffer(), bytes);
		    
		    //printf ("dispatch %d matched posted receive %d %d %d %d\n", 
		    //	    dispatch_id,
		    //	    nbytes, mcast->_pwidth, mcast->_counter,
		    //	    mcast->_size);
		    
		    //for(unsigned count = 0; count < mcast->_size; count += mcast->_pwidth)
		    //if(mcast->_done_fn)
		    //  mcast->_done_fn(&msg->_context, mcast->_cookie, XMI_SUCCESS);
		    
		    XMI_assert (nbytes <= mcast->_pwidth);
		    
		    mcast->_counter += mcast->_pwidth;
		    if(mcast->_done_fn)
		      mcast->_done_fn(&msg->_context, mcast->_cookie, XMI_SUCCESS);
		    
		    if (mcast->_counter >= mcast->_size) {
		      _mcastrecvQ.remove(mcast);
		      if(found)
			free (mcast);
		    }

		    free (msg);
		  }	      
		  break;
		}
	    }
          // This isn't performance sensitive because this device is just for testing
          // but we want to play nice with other
          // processes, so let's be nice and yield to them.
          sched_yield();
	  return 0;
        };

      // Implement MessageDevice Routines
      inline size_t getMessageMetadataSize_impl ()
        {
          return 128;
        }
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
      inline void enqueue(MPIMessage* msg)
        {
          _sendQ.push_front(msg);
        }

      inline void enqueue(MPIMcastMessage* msg)
        {
          _mcastsendQ.push_front(msg);
        }

      inline void enqueue(MPIMcastRecvMessage *msg)
        {
          _mcastrecvQ.push_front(msg);
        }

      inline void create_recvreq(unsigned                 connid,
                                 const xmi_callback_t   * cb_done,
                                 char                   * buf,
                                 unsigned                 size,
                                 unsigned                 nranks )
        {
          assert(0);
        }
      

      
      
      size_t                              _peers;
      size_t                              _dispatch_id;
      size_t                              _mcast_dispatch_id;
      std::map<int, mpi_dispatch_info_t>  _dispatch_lookup;
      std::map<int, mpi_mcast_dispatch_info_t>  _mcast_dispatch_lookup;
      std::list<MPIMessage*>              _sendQ;
      std::list<MPIMcastMessage*>         _mcastsendQ;
      std::list<MPIMcastRecvMessage*>     _mcastrecvQ;
      mpi_dispatch_info_t                 _dispatch_table[256];
      mpi_mcast_dispatch_info_t           _mcast_dispatch_table[256];
    };
  };
};
#endif // __components_devices_mpi_mpipacketdevice_h__
