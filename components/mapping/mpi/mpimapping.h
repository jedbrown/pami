/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/mapping/mpi/mpimapping.h
 * \brief ???
 */

#ifndef __components_mapping_mpi_mpimapping_h__
#define __components_mapping_mpi_mpimapping_h__

#include "sys/xmi.h"
#include "components/mapping/BaseMapping.h"
#include "components/mapping/TorusMapping.h"
#include "components/mapping/NodeMapping.h"
#include "components/memory/heap/HeapMemoryManager.h"
#include <mpi.h>

#define XMI_MAPPING_CLASS XMI::Mapping::MPIMapping

namespace XMI
{
  namespace Mapping
  {
#define MPI_DIMS 1
    class MPIMapping : public Interface::Base<MPIMapping, XMI::Memory::HeapMemoryManager>,
                       public Interface::Torus<MPIMapping, MPI_DIMS>,
                       public Interface::Node<MPIMapping, MPI_DIMS>
    {
	
    public:
      inline MPIMapping () : 
        Interface::Base<MPIMapping,XMI::Memory::HeapMemoryManager >(),
        Interface::Torus<MPIMapping, MPI_DIMS>(),
        Interface::Node<MPIMapping, MPI_DIMS>()
        {
          MPI_Comm_rank(MPI_COMM_WORLD, (int*)&_task);
          MPI_Comm_size(MPI_COMM_WORLD, (int*)&_size);
        };
      inline ~MPIMapping () {};
    protected:
      size_t    _task;
      size_t    _size;
    public:
      inline xmi_result_t init_impl(xmi_coord_t                    &ll,
                                    xmi_coord_t                    &ur,
				    size_t                          min_rank,
                                    size_t                          max_rank,
				    XMI::Memory::HeapMemoryManager &mm)
        {
          ll.n_torus.coords[0] = 0;
          ur.n_torus.coords[0] = _size-1;
          
          return XMI_SUCCESS;
        }
      inline size_t task_impl()
        {
          return _task;
        }
      inline size_t size_impl()
        {
          return _size;
        }
      inline xmi_result_t nodeTasks_impl (size_t global, size_t & tasks)
        {
          assert(0);
          return XMI_UNIMPL;
        }
      inline xmi_result_t nodePeers_impl (size_t & peers)
        {
          assert(0);
          return XMI_UNIMPL;
        }
      inline bool isPeer_impl (size_t task1, size_t task2)
        {
          assert(0);
          return XMI_UNIMPL;
        }
      inline void nodeAddr_impl (Interface::nodeaddr_t & address)
        {
          assert(0);
        }
      inline xmi_result_t task2node_impl (size_t task, Interface::nodeaddr_t & address)
        {
          address.global=task;
          address.local =0;
          return XMI_SUCCESS;
        }
      inline xmi_result_t node2task_impl (Interface::nodeaddr_t & address, size_t & task)
        {
          task = address.global;
          return XMI_SUCCESS;
        }
      inline xmi_result_t node2peer_impl (Interface::nodeaddr_t & address, size_t & peer)
        {
          assert(0);
          return XMI_UNIMPL;
        }
      inline void torusAddr_impl (size_t (&addr)[MPI_DIMS])
        {
          addr[0]=_task;
        }
      inline xmi_result_t task2torus_impl (size_t task, size_t (&addr)[MPI_DIMS])
        {
          addr[0] = task;
          return XMI_SUCCESS;
        }

      inline xmi_result_t torus2task_impl (size_t (&addr)[MPI_DIMS], size_t & task)
        {
          task = addr[0];
          return XMI_SUCCESS;
        }
      inline size_t       torusgetcoord_impl (size_t dimension)
        {
          if(dimension >= MPI_DIMS)
            abort();
          return _task;
        }
      inline size_t globalDims()
        {
          return MPI_DIMS;
        }
    };
  };
};
#endif // __components_mapping_mpi_mpimapping_h__
