/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
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

#include "Platform.h"
#include "components/memory/heap/HeapMemoryManager.h"
#include "common/BaseMappingInterface.h"
#include "common/TorusMappingInterface.h"
#include "common/NodeMappingInterface.h"
#include "sys/xmi.h"
#include <mpi.h>
#include <pmi.h>
#include <errno.h>

#define XMI_MAPPING_CLASS XMI::Mapping

namespace XMI
{
#define MPI_TDIMS 1
#define MPI_LDIMS 1
    class Mapping : public Interface::Mapping::Base<Mapping, XMI::Memory::HeapMemoryManager>,
                       public Interface::Mapping::Torus<Mapping, MPI_TDIMS>,
                       public Interface::Mapping::Node<Mapping, MPI_LDIMS>
    {

    public:
      inline Mapping () :
        Interface::Mapping::Base<Mapping,XMI::Memory::HeapMemoryManager >(),
        Interface::Mapping::Torus<Mapping, MPI_TDIMS>(),
        Interface::Mapping::Node<Mapping, MPI_LDIMS>()
        {
        };
      inline ~Mapping () {};
    protected:
	size_t    _task;
	size_t    _size;
	size_t    *_peers;
	size_t    _npeers;
	uint32_t *_mapcache;
	size_t   *_nodecache;
	size_t    _nSize;
	size_t    _tSize;

	static int rank_compare(const void *a, const void *b) {
		size_t *aa = (size_t *)a, *bb = (size_t *)b;
		size_t r1 = *aa;
		size_t r2 = *bb;
		int x = r1 - r2;
		return x;
	}
    public:
      inline xmi_result_t init(size_t &min_rank, size_t &max_rank,
				size_t &num_local, size_t **local_ranks)
        {
		int num_ranks, mine;
		size_t *ranks, r, q;
		int err;
		int x;
		char *kvsname, *id;

		// global process/rank info
		err = PMI_Get_size(&num_ranks);
		XMI_assertf(err == PMI_SUCCESS, "PMI_Get_size failed %d", err);
		_size = num_ranks;
		err = PMI_Get_rank(&num_ranks);
		XMI_assertf(err == PMI_SUCCESS, "PMI_Get_rank failed %d", err);
		_task = num_ranks;

		// local node process/rank info
		// this requires 'smpd' as the mpich2 process manager!
		err = PMI_Get_clique_size(&num_ranks);
		XMI_assertf(err == PMI_SUCCESS, "PMI_Get_clique_size failed %d", err);
		err = posix_memalign((void **)&ranks, sizeof(*ranks), sizeof(*ranks) * num_ranks);
		XMI_assertf(err == 0, "memory alloc failed, errno %d", err);
		err = PMI_Get_clique_ranks((int *)ranks, num_ranks);
		XMI_assertf(err == PMI_SUCCESS, "PMI_Get_clique_ranks failed %d", err);
		// now, need to convert int to size_t...
		if (sizeof(*ranks) != sizeof(int)) {
			for (x = num_ranks - 1; x >= 0; --x) {
				ranks[x] = ((int *)ranks)[x];
			}
		}
		// needed?
		// qsort(ranks, num_ranks, sizeof(*ranks), rank_compare);
		mine = 0;
		for (x = 0; x < num_ranks; ++x) {
			if (ranks[x] == _task) {
				mine = x;
				break;
			}
		}
		*local_ranks = ranks;
		num_local = num_ranks;
		_peers = ranks;
		_npeers = num_ranks;

		// is this a valid assumption?
		min_rank = 0;
		max_rank = _size-1;

		int name_max;
		err = PMI_KVS_Get_name_length_max(&name_max);
		XMI_assertf(err == PMI_SUCCESS, "PMI_KVS_Get_name_length_max failed %d", err);
		err = posix_memalign((void **)&kvsname, sizeof(void *), name_max);
		XMI_assertf(err == 0, "memory alloc failed, errno %d", err);
		err = PMI_KVS_Get_my_name(kvsname, name_max);
		XMI_assertf(err == PMI_SUCCESS, "PMI_KVS_Get_my_name failed %d", err);
		err = PMI_Get_id_length_max(&name_max);
		XMI_assertf(err == PMI_SUCCESS, "PMI_Get_id_length_max failed %d", err);
		err = posix_memalign((void **)&id, sizeof(void *), name_max);
		XMI_assertf(err == 0, "memory alloc failed, errno %d", err);
		static char buf[4096];

		// now post our local "map" for others to see
		char *s = buf;
		sprintf(buf, "%zdx%zd", mine, _peers[0]);
		sprintf(id, "PEERS_%zd", _task);
		err = PMI_KVS_Put(kvsname, id, buf);
		XMI_assertf(err == 0, "Failed (%d/%d) to put KVS value for \"%s\"", err, errno, id);
		err = PMI_KVS_Commit(kvsname);
		XMI_assertf(err == 0, "Failed (%d) to commit KVS value", err);
		PMI_Barrier();

		err = posix_memalign((void **)&_mapcache, sizeof(*_mapcache), sizeof(*_mapcache) * _size);
		XMI_assertf(err == 0, "memory alloc failed, errno %d", err);
		err = posix_memalign((void **)&_nodecache, sizeof(*_nodecache), sizeof(*_nodecache) * _size);
		XMI_assertf(err == 0, "memory alloc failed, errno %d", err);

		// now go through all ranks and get each one's "map" entry
		int max = 0;
		_tSize = 0;
		_nSize = 0;
		for (r = 0; r < _size; ++r) {
			size_t ix, master;
			sprintf(id, "PEERS_%zd", r);
			err = PMI_KVS_Get(kvsname, id, buf, sizeof(buf));
			XMI_assertf(err == 0, "Failed (%d) to get KVS value for \"%s\"", err, id);
			x = sscanf(buf, "%zdx%zd", &ix, &master);
			XMI_assertf(x == 2, "Failed to parse (%d) KVS value for \"%s\"", x, id);
			if (ix >= _tSize) _tSize = ix + 1;
			// TODO! all nodes must build this list in the same order!
			for (q = 0; q < _nSize && master != _nodecache[q]; ++q);
			if (!(q < _nSize)) {
				_nodecache[_nSize++] = master;
			}
			_mapcache[r] = (q << 16) | ix;
		}
		// At this point, _mapcache[rank] -> [index1]|[index2], where:
		// _nodecache[index1] -> "first" rank on that node
		// (at target node)_peers[index2] -> rank

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
          return XMI_UNIMPL;
        }
      inline xmi_result_t nodePeers_impl (size_t & peers)
        {
		peers = _npeers;
		return XMI_SUCCESS;
        }
      inline bool isPeer_impl (size_t task1, size_t task2)
        {
		// this really needs to be global, but if we only check locally it
		// will at least work for Topology.
		Interface::Mapping::nodeaddr_t addr;
		size_t peer;

		task2node_impl(task1, addr);
		xmi_result_t err1 = node2peer_impl(addr, peer);
		task2node_impl(task2, addr);
		xmi_result_t err2 = node2peer_impl(addr, peer);
		return (err1 == XMI_SUCCESS && err2 == XMI_SUCCESS);
        }
      inline void nodeAddr_impl (Interface::Mapping::nodeaddr_t & address)
        {
		xmi_result_t err = task2node_impl(_task, address);
		XMI_assertf(err == XMI_SUCCESS, "Internal error, my task does not exist");
        }
      inline xmi_result_t task2node_impl (size_t task, Interface::Mapping::nodeaddr_t & address)
        {
		uint32_t x = _mapcache[task];
		address.global = x >> 16;
		address.local = x & 0x0000ffff;
		return XMI_SUCCESS;
        }
      inline xmi_result_t node2task_impl (Interface::Mapping::nodeaddr_t & address, size_t & task)
        {
		uint32_t x = (address.global << 16) | address.local;
		// since we expect this to be small, searching is probably the easiest way
		size_t r;
		for (r = 0; r < _size && _mapcache[r] != x; ++r);
		if (r < _size) {
			task = r;
			return XMI_SUCCESS;
		}
		// never happens?
		return XMI_ERROR;
        }
      inline xmi_result_t node2peer_impl (Interface::Mapping::nodeaddr_t & address, size_t & peer)
        {
		size_t x, r = address.global;
		for (x = 0; x < _npeers && r != _peers[x]; ++x);
		if (x < _npeers) {
			peer = x;
			return XMI_SUCCESS;
		}
		return XMI_ERROR;
        }
      inline void torusAddr_impl (size_t (&addr)[MPI_TDIMS])
        {
		uint32_t x = _mapcache[_task];
		addr[0] = x >> 16;
		addr[1] = x & 0x0000ffff;
        }
      inline xmi_result_t task2torus_impl (size_t task, size_t (&addr)[MPI_TDIMS])
        {
		if (task >= _size) {
			return XMI_ERROR;
		}
		uint32_t x = _mapcache[task];
		addr[0] = x >> 16;
		return XMI_SUCCESS;
        }
      inline xmi_result_t task2global (size_t task, size_t (&addr)[MPI_TDIMS + MPI_LDIMS])
        {
		if (task >= _size) {
			return XMI_ERROR;
		}
		uint32_t x = _mapcache[task];
		addr[0] = x >> 16;
		addr[1] = x & 0x0000ffff;
		return XMI_SUCCESS;
        }

      inline xmi_result_t torus2task_impl (size_t (&addr)[MPI_TDIMS], size_t & task)
        {
		XMI_abort();
	}
      inline xmi_result_t global2task(size_t (&addr)[MPI_TDIMS + MPI_LDIMS], size_t & task)
        {
		uint32_t x = (addr[0] << 16) | addr[1];
		// since we expect this to be small, searching is probably the easiest way
		size_t r;
		for (r = 0; r < _size && _mapcache[r] != x; ++r);
		if (r < _size) {
			task = r;
			return XMI_SUCCESS;
		}
		// never happens?
		return XMI_ERROR;
        }
      inline size_t       torusgetcoord_impl (size_t dimension)
        {
          if(dimension >= MPI_TDIMS + MPI_LDIMS)
            abort();
          return _task;
        }
      inline size_t globalDims()
        {
          return MPI_TDIMS + MPI_LDIMS;
        }
      inline xmi_result_t network2task_impl(const xmi_coord_t *addr,
						size_t *task,
						xmi_network *type)
	{
		*task = addr->u.n_torus.coords[0];
		return XMI_SUCCESS;
	}
    }; // class Mapping
};	// namespace XMI
#endif // __components_mapping_mpi_mpimapping_h__
