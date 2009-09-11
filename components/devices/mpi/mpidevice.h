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
          fprintf(stderr, "registering dispatch id=%d\n", _dispatch_id);
          _dispatch_table[_dispatch_id].recv_func=recv_func;
          _dispatch_table[_dispatch_id].recv_func_parm=recv_func_parm;
          _dispatch_lookup[_dispatch_id]=_dispatch_table[_dispatch_id];
          return _dispatch_id++;
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
          std::list<MPIMessage*>::iterator it;
          for(it=_sendQ.begin();it != _sendQ.end(); it++)
              {
                flag            = 0;
                MPI_Testall(1,&((*it)->_request),&flag,MPI_STATUSES_IGNORE);
                if(flag)
                    {
                      if((*it)->_done_fn )
                        (*(*it)->_done_fn)((*it)->_context,(*it)->_cookie, XMI_SUCCESS);
                      _sendQ.remove((*it));
                      break;
                    }
              }
          flag = 0;
          int rc = MPI_Iprobe (MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, &sts);
          assert (rc == MPI_SUCCESS);
          if(flag)
              {
                int nbytes = 0;
                MPI_Get_count(&sts, MPI_BYTE, &nbytes);
                MPIMessage *msg = (MPIMessage *) malloc (nbytes);
                int rc = MPI_Recv(msg,nbytes,MPI_BYTE,sts.
                                  MPI_SOURCE,sts.MPI_TAG,
                                  MPI_COMM_WORLD,&sts);
                assert(rc == MPI_SUCCESS);
                size_t dispatch_id      = msg->_msg._dispatch_id;
                mpi_dispatch_info_t mdi = _dispatch_lookup[dispatch_id];

                fprintf(stderr, "RECEIVED MSG=%d\n", dispatch_id);
                
                if(mdi.recv_func)
                  mdi.recv_func(msg->_msg._metadata,
                                msg->_msg._payload,
                                msg->_msg._payloadsize,
                                mdi.recv_func_parm);
                free(msg);
                
#if 0
                unsigned         rcvlen;
                char           * rcvbuf;
                unsigned         pwidth;
                XMI_Callback_t   cb_done;

                _cb_async_head (&msg->_info[0], msg->_info_count, sts.MPI_SOURCE, msg->_size, msg->_conn,
                                _async_arg, &rcvlen, &rcvbuf, &pwidth, &cb_done);

                if(rcvlen)
                  memcpy (rcvbuf, msg->buffer(), rcvlen);

                if(pwidth == 0 && rcvlen == 0)
                  if(cb_done.function)
                    cb_done.function (cb_done.clientdata, NULL);

                for(unsigned count = 0; count < rcvlen; count += pwidth)
                  if(cb_done.function)
                    cb_done.function (cb_done.clientdata, NULL);

                CCMI_Free (msg);
#endif                
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
      
      size_t                              _peers;
      size_t                              _dispatch_id;
      std::map<int, mpi_dispatch_info_t>  _dispatch_lookup;
      std::list<MPIMessage*>              _sendQ;
      mpi_dispatch_info_t                 _dispatch_table[256];
    };
  };
};
#endif // __components_devices_mpi_mpipacketdevice_h__
