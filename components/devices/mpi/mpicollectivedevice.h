/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/mpi/mpicollectivedevice.h
 * \brief Experimental MPI collective device on _communicator
 */

#ifndef __components_devices_mpi_mpicollectivedevice_h__
#define __components_devices_mpi_mpicollectivedevice_h__
#ifndef DISABLE_COLLDEVICE

#include "components/devices/BaseDevice.h"
#include "components/devices/mpi/mpicollectiveheader.h"
#include <list>
#include <sched.h>
#include "util/ccmi_debug.h"
#include "mpi.h"


#ifndef TRACE_DEVICE
  #define TRACE_DEVICE(x) //fprintf x
#endif

namespace XMI
{
  namespace Device
  {
#define SOME_ARBITRARY_LIMIT 100
    typedef void (*device_dispatch_fn)(void    *header,
                                       uint16_t headerLength,
                                       void    *cookie    );
    typedef struct
    {
      device_dispatch_fn    function;
      void                        *cookie;
    } dispatch_table_t;

    template <class T_Dispatch_Header>
    class MPICollectiveDevice //: public Interface::BaseDevice<MPICollectiveDevice<T_SysDep,T_Dispatch_Header>, T_SysDep>
    {

    public:
      // Default ctor creates an unusable device. Must ctor over it with a different ctor at some point.
      inline MPICollectiveDevice () :
      _communicator(MPI_COMM_NULL),
      _peers(0),
      _task_id(-1),
      _member(false),
      _dispatchQ(),
      _advanceQ(),
      _topology(&__global.topology_global)
      {
      };
      inline MPICollectiveDevice (size_t task_id) :
//      Interface::BaseDevice<MPICollectiveDevice<T_SysDep,T_Dispatch_Header>, T_SysDep> ()
      _task_id(task_id),
      _member(true),
      _dispatchQ(),
      _advanceQ(),
      _topology(&__global.topology_global)
      {
        MPI_Comm_dup(MPI_COMM_WORLD,&_communicator);
        MPI_Comm_size(_communicator, (int*)&_peers);
        TRACE_DEVICE((stderr, "<%#.8X>MPICollectiveDevice() _peers %zd\n", (int)this, _peers));
        // Initialize the registered receive function array to NULL.
        // The array is limited to 256 dispatch ids because of the size of the
        // dispatch id field in the packet header.
        for(int i=0; i<256; i++)
        {
          for(int j=0; j<256; j++)
          {
            _dispatch[i][j].function   = NULL;
            _dispatch[i][j].cookie     = NULL;
          }
        }
        _dispatch[0][0].function = (device_dispatch_fn)-1;
      };
      inline MPICollectiveDevice (XMI::Topology *topology, size_t task_id) :
//      Interface::BaseDevice<MPICollectiveDevice<T_SysDep,T_Dispatch_Header>, T_SysDep> (),
      _task_id(task_id),
      _dispatchQ(),
      _advanceQ(),
      _topology(topology)
      {
        int local_rank;
        _member = topology->isRankMember(_task_id);
        TRACE_DEVICE((stderr,"MPI_Comm_split(MPI_COMM_WORLD, %d, %d, &_communicator);",_member?(int)topology->index2Rank(0):-1, (int)topology->rank2Index(task_id)));
        MPI_Comm_split(MPI_COMM_WORLD,_member?(int)topology->index2Rank(0):-1, (int)topology->rank2Index(task_id), &_communicator);
        MPI_Comm_size(_communicator, (int*)&_peers);
        MPI_Comm_rank(_communicator,&local_rank);
        TRACE_DEVICE((stderr, "<%#.8X>MPICollectiveDevice(topology) _member %s, color %d, peers %zd, local_rank %d/%d task_id %d/%d\n", (int)this,_member?"true":"false",_member?topology->index2Rank(0):-1,_peers,local_rank,(int)topology->rank2Index(_task_id),_task_id,(int)topology->index2Rank(local_rank)));
        // Initialize the registered receive function array to NULL.
        // The array is limited to 256 dispatch ids because of the size of the
        // dispatch id field in the packet header.
        for(int i=0; i<256; i++)
        {
          for(int j=0; j<256; j++)
          {
            _dispatch[i][j].function   = NULL;
            _dispatch[i][j].cookie     = NULL;
          }
        }
        _dispatch[0][0].function = (device_dispatch_fn)-1;
      };

      // Implement BaseDevice Routines

      inline ~MPICollectiveDevice ()
      {
      };

      int registerRecvFunction (size_t                      dispatch_id,
                                device_dispatch_fn          function,
                                void                      * cookie)
      {
        TRACE_DEVICE((stderr, "<%#.8X>MPICollectiveDevice::registerRecvFunction(%zd, %p, %p)\n", (int)this, dispatch_id, function, cookie));

        // This device only supports up to 256 dispatch sets.
        if(dispatch_id >= 256) XMI_abortf("<%#.8X>MPICollectiveDevice::registerRecvFunction(%zd, %p, %p)\n", (int)this, dispatch_id, function, cookie);

        // Find the next available slot for this dispatch set.
        size_t slot;
        for(slot=0; slot<256; slot++)
        {
          if(_dispatch[dispatch_id][slot].function == NULL) break;

        }

        if(slot == 256) XMI_abortf("<%#.8X>MPICollectiveDevice::registerRecvFunction(%zd, %p, %p) slot %d\n", (int)this, dispatch_id, function, cookie, slot);

        _dispatch[dispatch_id][slot].function  = function;
        _dispatch[dispatch_id][slot].cookie     = cookie;

        size_t id = dispatch_id*256+slot;

        TRACE_DEVICE((stderr, "<%#.8X>MPICollectiveDevice::registerRecvFunction dispatch id %zd, slot %d, id %d\n", (int)this, dispatch_id, slot, id));
        return id;
      }

      inline void getDispatchHeader(T_Dispatch_Header * header,
                                    uint16_t            length)
      {
        TRACE_DEVICE((stderr, "<%#.8X>MPICollectiveDevice::getDispatchHeader header %p, length %d, root %d/%d\n", (int)this, header->header(), length, header->root(),_topology->rank2Index(header->root())));
        MPI_Bcast(header->header(), length*sizeof(xmi_quad_t), MPI_BYTE, _topology->rank2Index(header->root()), _communicator);
        for(int i = 0; i < length; i++)
        {
          TRACE_DEVICE((stderr, "<%#.8X>MPICollectiveDevice::getDispatchHeader %#.8X %#.8X %#.8X %#.8X\n", (int)this, ((unsigned*)header->header())[i],((unsigned*)header->header())[i+1],((unsigned*)header->header())[i+2],((unsigned*)header->header())[i+3]));
        }
        TRACE_DEVICE((stderr, "<%#.8X>MPICollectiveDevice::getDispatchHeader length %d, root %d, id %d, connection id %d, sndlen %d, msgcount %d \n",
                      (int)this, length, header->root(), header->id(), header->connection_id(), header->sndlen(), header->msgcount()));
      }
      inline bool getBytesAvailable(T_Dispatch_Header * header)
      {
        unsigned bytesAvailable=header->available();  // bytes I have available
        TRACE_DEVICE((stderr, "<%#.8X>MPICollectiveDevice::getBytesAvailable %d, root %d, connection id %d\n", (int)this, bytesAvailable, header->root(), header->connection_id()));
        MPI_Allreduce(MPI_IN_PLACE, &bytesAvailable, 1, MPI_UNSIGNED, MPI_MIN, _communicator); // collectively decide how many bytes available
        TRACE_DEVICE((stderr, "<%#.8X>MPICollectiveDevice::getBytesAvailable %d\n", (int)this, bytesAvailable));
        header->collectiveBytesAvailable(bytesAvailable); // set collective bytes available in the header and prep data buffer
        return bytesAvailable;
      }
      inline void multicast(T_Dispatch_Header * header)
      {
        TRACE_DEVICE((stderr, "<%#.8X>MPICollectiveDevice::multicast buffer %p, bytesAvailable %d, root %d/%d\n", (int)this, header->dataBuffer(), header->collectiveBytesAvailable(), header->root(),_topology->rank2Index(header->root())));
        MPI_Bcast(header->dataBuffer(), header->collectiveBytesAvailable(), MPI_BYTE, _topology->rank2Index(header->root()), _communicator);

      }
      inline void multisync(T_Dispatch_Header * header)
      {
        TRACE_DEVICE((stderr, "<%#.8X>MPICollectiveDevice::multisync\n", (int)this));
        MPI_Barrier(_communicator);
      }
      inline bool tryHeaderPost(T_Dispatch_Header *dispatch_header,T_Dispatch_Header *selected_header)
      {
        TRACE_DEVICE((stderr, "<%#.8X>MPICollectiveDevice::tryHeaderPost< id %zd\n", (int)this,dispatch_header->id()));
        TRACE_DEVICE((stderr, "<%#.8X>MPICollectiveDevice::tryHeaderPost< %#.8X %#.8X\n", (int)this, ((unsigned*)dispatch_header->header())[0],((unsigned*)dispatch_header->header())[1]));
        MPI_Allreduce(dispatch_header->header(), selected_header->header(), 1 , MPI_LONG_LONG, MPI_MAX, _communicator);//MPI_UNSIGNED_LONG_LONG didn't work in ppc linux mpi
        TRACE_DEVICE((stderr, "<%#.8X>MPICollectiveDevice::tryHeaderPost> %#.8X %#.8X\n", (int)this, ((unsigned*)selected_header->header())[0],((unsigned*)selected_header->header())[1]));
        TRACE_DEVICE((stderr, "<%#.8X>MPICollectiveDevice::tryHeaderPost> id %zd\n", (int)this,selected_header->id()));
        if(selected_header->id()) return true; // some header was selected
        return false;      // some protocol header was received, not mine
      }
      inline int advance_impl ()
      {
        static unsigned idle = 0; unsigned reset_idle=0xfffff;
        int events = 0;

        if(!idle || (idle > reset_idle))
        {
          TRACE_DEVICE((stderr, "<%#.8X>MPICollectiveDevice::advance_impl _member %s\n", (int)this, _member?"true":"false"));
          idle = 0;
        }
        if(!_member) return 0;

        while(events < SOME_ARBITRARY_LIMIT)
        {
          int events_this_loop = 0;

          // Try to dispatch any new messages
          T_Dispatch_Header* dispatch_header = (T_Dispatch_Header*)_dispatchQ.peekHead();
          while(dispatch_header != NULL)
          {
            if(!idle) TRACE_DEVICE((stderr, "<%#.8X>MPICollectiveDevice::advance_impl dispatch_header %p id %d\n", (int)this,dispatch_header, dispatch_header->id()));
            if(!dispatch_header->ready())
            {
              dispatch_header = (T_Dispatch_Header*)dispatch_header->next();
              continue;
            }
/*            if(dispatch_header->done())
            {
              T_Dispatch_Header* temp = dispatch_header;
              dispatch_header = (T_Dispatch_Header*)dispatch_header->next();
              _dispatchQ.deleteElem(temp);
              free(temp);
              continue;
            } */
            T_Dispatch_Header selected_header;
            if(tryHeaderPost(dispatch_header,&selected_header)) // something was dispatched, process it
            {
              events_this_loop++;
              uint16_t id = selected_header.id();
              uint8_t * id_slot = (uint8_t *) &id;
              dispatch_table_t dispatch = _dispatch[id_slot[0]][id_slot[1]];
              if(!idle) TRACE_DEVICE((stderr, "<%#.8X>MPICollectiveDevice::advance_impl events %d, header id %d/%d, id %d, slot %d\n", (int)this,events_this_loop, selected_header.id(), id, id_slot[0],id_slot[1]));
              TRACE_DEVICE((stderr, "<%#.8X>MPICollectiveDevice::advance_impl events %d, header id %d/%d, id %d, slot %d\n", (int)this,events_this_loop, selected_header.id(), id, id_slot[0],id_slot[1]));
              XMI_assert(dispatch.function);
              uint16_t length = selected_header.length();
              if(selected_header==(*dispatch_header))
              {
                dispatch_header->resetFullHeader();
                // Remove our header if it was dispatched
                _dispatchQ.deleteElem(dispatch_header);
                dispatch.function(dispatch_header, length, dispatch.cookie);
              }
              else
                dispatch.function(&selected_header, length, dispatch.cookie);

            }
            break;
          }

          // Advance any dispatched messages
          T_Dispatch_Header* data_header = (T_Dispatch_Header*)_advanceQ.peekHead();
          while(data_header != NULL)
          {
            if(!idle) TRACE_DEVICE((stderr, "<%#.8X>MPICollectiveDevice::advance_impl data_header %p id %d\n", (int)this, data_header, data_header->id()));
            if(!data_header->ready())
            {
              data_header = (T_Dispatch_Header*)data_header->next();
              continue;
            }
            if(data_header->done())
            {
              if(!idle) TRACE_DEVICE((stderr, "<%#.8X>MPICollectiveDevice::advance_impl done %p id %d\n", (int)this, data_header, data_header->id()));
              T_Dispatch_Header* temp = data_header;
              data_header = (T_Dispatch_Header*)data_header->next();
              if(!idle) TRACE_DEVICE((stderr, "<%#.8X>MPICollectiveDevice::advance_impl delete %p id %d, next header %p id %d\n", (int)this,temp,temp==NULL?-1:temp->id(), data_header,data_header==NULL?-1:data_header->id()));
              _advanceQ.deleteElem(temp);
              free(temp);
              continue;
            }
            T_Dispatch_Header selected_header;
            if(tryHeaderPost(data_header,&selected_header))
            {
              events_this_loop++;
              uint16_t id = selected_header.id();
              uint8_t * id_slot = (uint8_t *) &id;
              dispatch_table_t dispatch = _dispatch[id_slot[0]][id_slot[1]];
              if(!idle) TRACE_DEVICE((stderr, "<%#.8X>MPICollectiveDevice::advance_impl events %d, header id %d/%d, id %d, slot %d\n", (int)this,events_this_loop, selected_header.id(), id, id_slot[0],id_slot[1]));
              XMI_assert(dispatch.function);
              dispatch.function(&selected_header, -1, dispatch.cookie);
            }
            break;
          }
          events += events_this_loop;
          if(!events_this_loop) break;
        }

        // This isn't performance sensitive because this device is just for testing
        // but we want to play nice with other
        // processes, so let's be nice and yield to them.
        sched_yield();
        if(!idle) TRACE_DEVICE((stderr, "<%#.8X>MPICollectiveDevice::advance_impl events %d\n", (int)this,events));
        if(events)
          ;//idle = 0;
        else
          idle++;
        return events;
      };

      inline void enqueueDispatch(T_Dispatch_Header* header)
      {
        _dispatchQ.pushHead(header);
        TRACE_DEVICE((stderr, "<%#.8X>MPICollectiveDevice::enqueue dispatch message %p, id %d\n", (int)this, header, header->id()));
      }
      inline void enqueueData(T_Dispatch_Header* header)
      {
        _advanceQ.pushHead(header);
        TRACE_DEVICE((stderr, "<%#.8X>MPICollectiveDevice::enqueue data message %p, id %d\n", (int)this, header, header->id()));
      }

      MPI_Comm                                  _communicator;
      size_t                                    _peers;
      size_t                                    _task_id;
      bool                                      _member;
      Queue                                     _dispatchQ;
      Queue                                     _advanceQ;
      dispatch_table_t                          _dispatch[256][256];
      XMI::Topology                            *_topology;
    };
#undef SOME_ARBITRARY_LIMIT
  };
};

#endif // DISABLE_COLLDEVICE
#endif // __components_devices_mpi_mpicollectivedevice_h__
