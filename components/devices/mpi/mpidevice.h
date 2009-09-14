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

      int registerMcastRecvFunction (xmi_olddispatch_multicast_fn  recv_func,
				     void                         *async_arg)
        {
          _mcast_dispatch_table[_mcast_dispatch_id].recv_func=recv_func;
          _mcast_dispatch_table[_mcast_dispatch_id].async_arg=async_arg;
          _mcast_dispatch_lookup[_mcast_dispatch_id]=_mcast_dispatch_table[_mcast_dispatch_id];
          return _mcast_dispatch_id++;
        }
      
      inline int init_impl (T_SysDep * sysdep)
        {
          assert(0);
        };

      inline bool isInit_impl ()
        {
          assert(0);
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
		    int rc = MPI_Recv(msg,nbytes,MPI_BYTE,sts.MPI_SOURCE,sts.MPI_TAG, MPI_COMM_WORLD,&sts);
		    XMI_assert (rc == MPI_SUCCESS);
		    unsigned         rcvlen;
		    char           * rcvbuf;
		    unsigned         pwidth;
		    xmi_callback_t   cb_done;
		    size_t dispatch_id      = msg->_dispatch_id;
		    mpi_mcast_dispatch_info_t mdi = _mcast_dispatch_lookup[dispatch_id];

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
		    if(rcvlen)
		      memcpy (rcvbuf, msg->buffer(), rcvlen);
		    
		    if(pwidth == 0 && rcvlen == 0)
		      if(cb_done.function)
			cb_done.function (&msg->_context, cb_done.clientdata, XMI_SUCCESS);
		    
		    for(unsigned count = 0; count < rcvlen; count += pwidth)
		      if(cb_done.function)
			cb_done.function (&msg->_context, cb_done.clientdata, XMI_SUCCESS);
		    
		    free (msg);
		  }
		  break;
		}
	    }
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
        }

      // Implement Packet Device Routines 
      inline int    readData_impl(void * dst, size_t bytes)
        {
          assert(0);
        }
      inline bool   requiresRead_impl()
        {
          assert(0);
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
      
      size_t                              _peers;
      size_t                              _dispatch_id;
      size_t                              _mcast_dispatch_id;
      std::map<int, mpi_dispatch_info_t>  _dispatch_lookup;
      std::map<int, mpi_mcast_dispatch_info_t>  _mcast_dispatch_lookup;
      std::list<MPIMessage*>              _sendQ;
      std::list<MPIMcastMessage*>         _mcastsendQ;
      mpi_dispatch_info_t                 _dispatch_table[256];
      mpi_mcast_dispatch_info_t           _mcast_dispatch_table[256];
    };
  };
};
#endif // __components_devices_mpi_mpipacketdevice_h__
