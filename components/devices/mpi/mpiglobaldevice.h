/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/mpi/MPIGlobalDevice.h
 * \brief Blocking, symmetric global device on MPI_COMM_WORLD
 */

#ifndef __components_devices_mpi_mpiglobaldevice_h__
#define __components_devices_mpi_mpiglobaldevice_h__

#include "components/devices/BaseDevice.h"
#include "components/devices/mpi/mpimessage.h"
#include "common/mpi/PipeWorkQueue.h"
#include "common/mpi/Topology.h"
#include "common/mpi/Global.h"
#include <map>
#include <list>
#include <sched.h>
#include "util/ccmi_debug.h"

#define DISPATCH_SET_SIZE 256

#ifndef TRACE_ADAPTOR
#define TRACE_ADAPTOR(x) //fprintf x
#endif

namespace XMI
{
  namespace Device
  {

    typedef struct mpi_global_mcast_dispatch_info_t
    {
      xmi_dispatch_multicast_fn    recv_func;
      void                         *async_arg;
    }mpi_global_mcast_dispatch_info_t;

      ///
      /// \brief Device dispatch receive function signature
      ///
      /// \param[in] metadata       Pointer to network header metadata that is
      ///                           not part of the normal packet payload.
      /// \param[in] payload        Pointer to the raw packet payload.
      /// \param[in] bytes          Number of valid bytes of packet payload.
      /// \param[in] recv_func_parm Registered dispatch clientdata
      /// \param[in] cookie         Device cookie
      ///
      /// \todo Define return value(s)
      ///
      /// \return ???
      ///
      typedef int (*RecvFunction_t) (void   * metadata,
                                     void   * payload,
                                     size_t   bytes,
                                     void   * recv_func_parm,
                                     void   * cookie);

    template <class T_SysDep>
    class MPIGlobalDevice : public Interface::BaseDevice<MPIGlobalDevice<T_SysDep>, T_SysDep>
    {

    public:
      inline MPIGlobalDevice () :
      Interface::BaseDevice<MPIGlobalDevice<T_SysDep>, T_SysDep> (),
      _mcast_dispatch_id(0)
      {
        MPI_Comm_size(MPI_COMM_WORLD, (int*)&_peers);
        TRACE_ADAPTOR((stderr, "<%#.8X>MPIGlobalDevice()\n", (int)this));
      };

      // Implement BaseDevice Routines

      inline ~MPIGlobalDevice () {};

      int initMcast()
      {
        return _mcast_dispatch_id++;
      }

      int registerRecvFunction (size_t                      dispatch_id,
                                RecvFunction_t              recv_func,
                                void                      * recv_func_parm)
      {
      _mcast_dispatch_table[dispatch_id].recv_func = NULL;//recv_func;
      _mcast_dispatch_table[dispatch_id].async_arg = recv_func_parm;
      TRACE_ADAPTOR((stderr, "<%#.8X>MPIGlobalDevice::registerGlobalMcastRecvFunction %d\n", (int)this, _mcast_dispatch_id));
        return dispatch_id;
      };

//    void registerGlobalMcastRecvFunction (int                           dispatch_id,
//                                          xmi_olddispatch_multicast_fn  recv_func,
//                                          void                         *async_arg)
//    {
//      _mcast_dispatch_table[dispatch_id].recv_func = recv_func;
//      _mcast_dispatch_table[dispatch_id].async_arg = async_arg;
//      TRACE_ADAPTOR((stderr, "<%#.8X>MPIGlobalDevice::registerGlobalMcastRecvFunction %d\n", (int)this, _mcast_dispatch_id));
//    }

      // Implement Base Device Routines
      inline size_t peers_impl ()
      {
        return _peers;
      }

      inline size_t task2peer_impl (size_t task)
      {
        assert(task < _peers);
        return task;
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
        static unsigned idle = 0; unsigned reset_idle=0xfffff;
        int events = 0;

        if(!idle || (idle > reset_idle))
        {
          TRACE_ADAPTOR((stderr, "<%#.8X>MPIGlobalDevice::advance_impl idle(%#X)\n", (int)this, idle));
          idle = 0;
        }

        // Check the Multicast send queue
        std::list<MPIGlobalMcastMessage*>::iterator it_mcast;

        for(it_mcast = _mcast_global_sendQ.begin();it_mcast != _mcast_global_sendQ.end(); )
        {
          MPIGlobalMcastMessage* msg = *it_mcast;
          if(!idle) TRACE_ADAPTOR((stderr, "<%#.8X>MPIGlobalDevice::advance_impl msg %p, queue size %zd\n", (int)this, msg,(size_t)_mcast_global_sendQ.size()));
          XMI::PipeWorkQueue* src =(XMI::PipeWorkQueue*)msg->src;
          XMI::PipeWorkQueue* dst =(XMI::PipeWorkQueue*)msg->dst;
          size_t bytesAvailable;
          XMI::Topology* src_participants =(XMI::Topology*)msg->src_participants;
          XMI::Topology* dst_participants =(XMI::Topology*)msg->dst_participants;
          size_t root = src_participants->index2Rank(0);
          if(root == __global.mapping.task()) bytesAvailable = src->bytesAvailableToConsume();
          MPI_Bcast(&bytesAvailable, sizeof(size_t), MPI_BYTE, root, MPI_COMM_WORLD);
          if(!idle || bytesAvailable) TRACE_ADAPTOR((stderr, "<%#.8X>MPIGlobalDevice::advance_impl root %d bytesAvailable(%zd)\n", (int)this, root, bytesAvailable));
          if(bytesAvailable)
          {
            char* buffer;
            idle=0;
            if(root == __global.mapping.task()) 
            { 
              buffer = src->bufferToConsume();
               if(dst_participants->isRankMember(__global.mapping.task())) //src & dst? do a local copy
               {
                 XMI_assert(dst->bytesAvailableToProduce() >= bytesAvailable);
                 memcpy(dst->bufferToProduce(), buffer, bytesAvailable);
                 dst->produceBytes(bytesAvailable);
               }
            }
            else 
            {
              XMI_assert(dst->bytesAvailableToProduce() >= bytesAvailable); /// \todo need better pwq support, fewer assumptions
              buffer = dst->bufferToProduce();
            }
            MPI_Bcast(buffer, bytesAvailable, MPI_BYTE, root, MPI_COMM_WORLD);
           if(root == __global.mapping.task()) src->consumeBytes(bytesAvailable);
           else dst->produceBytes(bytesAvailable);
            events++;
          }
          size_t bytesComplete;
          if(root == __global.mapping.task()) bytesComplete = src->getBytesConsumed();
          else bytesComplete = dst->getBytesProduced();
          if(!idle) TRACE_ADAPTOR((stderr, "<%#.8X>MPIGlobalDevice::advance_impl root %d bytesComplete(%zd) bytes(%zd)\n", (int)this, root, bytesComplete, msg->bytes));
          if(bytesComplete == msg->bytes)
          {
            if(msg->cb_done.function )
              (msg->cb_done.function)(msg->context, msg->cb_done.clientdata, XMI_SUCCESS);
            //free (msg);
            std::list<MPIGlobalMcastMessage*>::iterator it_oldmcast = it_mcast;
            it_mcast++;
            _mcast_global_sendQ.remove((*it_oldmcast));
            TRACE_ADAPTOR((stderr, "<%#.8X>MPIGlobalDevice::advance_impl remove msg %p, queue size %zd, next msg %p\n", (int)this, msg,(size_t)_mcast_global_sendQ.size(),*it_mcast));
          }
          else it_mcast++;
        }
        if(!events) idle++;
        // This isn't performance sensitive because this device is just for testing
        // but we want to play nice with other
        // processes, so let's be nice and yield to them.
        sched_yield();

        return events;
      };

      inline void enqueue(MPIGlobalMcastMessage* msg)
      {
        _mcast_global_sendQ.push_front(msg);
        TRACE_ADAPTOR((stderr, "<%#.8X>MPIGlobalDevice::enqueue global mcast message %p, msg size %zd, queue size %zd\n", (int)this, msg, (size_t)msg->bytes,(size_t)_mcast_global_sendQ.size()));
      }

      char                                     *_currentBuf;
      size_t                                    _peers;
      size_t                                    _mcast_dispatch_id;
      std::list<MPIGlobalMcastMessage*>         _mcast_global_sendQ;
      mpi_global_mcast_dispatch_info_t          _mcast_dispatch_table[256];
    };
  };
};

#endif // __components_devices_mpi_mpiglobaldevice_h__
