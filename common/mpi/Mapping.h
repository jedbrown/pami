/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common/mpi/Mapping.h
 * \brief ???
 */

#ifndef __common_mpi_Mapping_h__
#define __common_mpi_Mapping_h__

#include "Platform.h"
#include "common/BaseMappingInterface.h"
#include "common/TorusMappingInterface.h"
#include "common/NodeMappingInterface.h"
#include "sys/xmi.h"
#include <mpi.h>
#include <errno.h>
#include <unistd.h>

#define XMI_MAPPING_CLASS XMI::Mapping

namespace XMI
{
#define MPI_TDIMS 1
#define MPI_LDIMS 1
    class Mapping : public Interface::Mapping::Base<Mapping>,
                       public Interface::Mapping::Torus<Mapping, MPI_TDIMS>,
                       public Interface::Mapping::Node<Mapping, MPI_LDIMS>
    {

    public:
      inline Mapping () :
        Interface::Mapping::Base<Mapping>(),
        Interface::Mapping::Torus<Mapping, MPI_TDIMS>(),
        Interface::Mapping::Node<Mapping, MPI_LDIMS>()
        {
        };
      inline ~Mapping () {};
    protected:
	size_t    _task;
	size_t    _size;
	size_t   *_peers;
	size_t    _npeers;
	uint32_t *_mapcache;
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
		int num_ranks;
		size_t r, q;
		int err;
		int nz, tz;
		char *host, *hosts, *s;
		int str_len = 128;

		// global process/rank info
		MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);
		_size = num_ranks;
		MPI_Comm_rank(MPI_COMM_WORLD, &num_ranks);
		_task = num_ranks;

		// local node process/rank info
#ifdef USE_MEMALIGN
		err = posix_memalign((void **)&_mapcache, sizeof(void *), sizeof(*_mapcache) * _size);
		XMI_assertf(err == 0, "memory alloc failed, err %d", err);
		err = posix_memalign((void **)&_peers, sizeof(void *), sizeof(*_peers) * _size);
		XMI_assertf(err == 0, "memory alloc failed, err %d", err);
		err = posix_memalign((void **)&host, sizeof(void *), str_len);
		XMI_assertf(err == 0, "memory alloc failed, err %d", err);
		err = posix_memalign((void **)&hosts, sizeof(void *), str_len * _size);
		XMI_assertf(err == 0, "memory alloc failed, err %d", err);
#else
                _mapcache=(uint32_t*)malloc(sizeof(*_mapcache) * _size);
                XMI_assertf(_mapcache != NULL, "memory alloc failed");
                _peers = (size_t*)malloc(sizeof(*_peers) * _size);
                XMI_assertf(_peers != NULL, "memory alloc failed");
                host=(char*)malloc(str_len);
                XMI_assertf(host != NULL, "memory alloc failed");
                hosts=(char*)malloc(str_len*_size);
                XMI_assertf(hosts != NULL, "memory alloc failed");
#endif
		err = gethostname(host, str_len);
		XMI_assertf(err == 0, "gethostname failed, errno %d", errno);

		err = MPI_Allgather(host, str_len, MPI_BYTE, hosts, str_len, MPI_BYTE, MPI_COMM_WORLD);
		XMI_assertf(err == 0, "allgather failed, err %d", err);

		_nSize = 0;
		_tSize = 1;
		_npeers = 0;
		for (r = 0; r < _size; ++r) {
			// search backwards for anyone with the same hostname...
			for (q = r - 1; (int)q >= 0 && strcmp(hosts + str_len * r, hosts + str_len * q) != 0; --q);
			if ((int)q >= 0) {
				// already saw this hostname... add new peer...
				uint32_t u = _mapcache[q];
				uint32_t t = (u & 0x0000ffff) + 1;
				_mapcache[r] = (u & 0xffff0000) | t;
				if (t >= _tSize) _tSize = t + 1;
			} else {
				// new hostname... first one for that host... give it T=0
				_mapcache[r] = (_nSize << 16) | 0;
				++_nSize;
			}
			if (strcmp(host, hosts + str_len * r) == 0) {
				_peers[_npeers++] = r;
			}
		}
		free(host);
		free(hosts);

		// if all ranks are local, then see if an ENV variable
		// gives us permission to spice things up.
		nz = tz = 0;
		s = getenv("XMI_MAPPING_TSIZE");
		if (s) {
			tz = strtol(s, NULL, 0);
		}
		s = getenv("XMI_MAPPING_NSIZE");
		if (s) {
			nz = strtol(s, NULL, 0);
		}
		if (_nSize == 1 && (nz > 0 || tz > 0)) {
			uint32_t t = 0;
			uint32_t n = 0;
			if (nz > 0) {
				tz = 0;
				// remap using N-first sequence
				_nSize = nz;
				for (r = 0; r < _size; ++r) {
					if (n >= _nSize) { ++t; n = 0; }
					_mapcache[r] = (n << 16) | t;
					++n;
				}
				_tSize = t + 1;
			} else if (tz > 0) {
				// remap using T-first sequence
				_tSize = tz;
				for (r = 0; r < _size; ++r) {
					if (t >= _tSize) { ++n; t = 0; }
					_mapcache[r] = (n << 16) | t;
					++t;
				}
				_nSize = n + 1;
			}
			// now, must recompute _peers, _npeers...
			_npeers = 0;
			n = _mapcache[_task] & 0xffff0000;
			for (r = 0; r < _size; ++r) {
				if ((_mapcache[r] & 0xffff0000) == n) {
					_peers[_npeers++] = r;
				}
			}
		}

		// local ranks could be represented as rectangle...
		// but, let Global.h use Topology analyze if it wants.
		*local_ranks = _peers;
		num_local = _npeers;
		// global ranks could be represented as rectangle...
		min_rank = 0;
		max_rank = _size-1;
		//
		// At this point, _mapcache[rank] -> [index1]|[index2], where:
		// (at target node)_peers[index2] -> rank
		// coordinates = (index1,index2)

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
#warning "These need to be added to a public interface"
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
        return XMI_ERROR;
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
      inline xmi_result_t task2network (size_t task, xmi_coord_t *addr, xmi_network type)
        {
		if (task >= _size ||
			(type != XMI_N_TORUS_NETWORK && type != XMI_DEFAULT_NETWORK)) {
			return XMI_ERROR;
		}
		uint32_t x = _mapcache[task];
		addr->u.n_torus.coords[0] = x >> 16;
		addr->u.n_torus.coords[1] = x & 0x0000ffff;
		addr->network = XMI_N_TORUS_NETWORK;
		return XMI_SUCCESS;
        }
      inline xmi_result_t network2task_impl(const xmi_coord_t *addr,
						size_t *task,
						xmi_network *type)
	{
		if (addr->network != XMI_N_TORUS_NETWORK) {
			return XMI_ERROR;
		}
		uint32_t x = (addr->u.n_torus.coords[0] << 16) | addr->u.n_torus.coords[1];
		// since we expect this to be small, searching is probably the easiest way
		size_t r;
		for (r = 0; r < _size && _mapcache[r] != x; ++r);
		if (r < _size) {
			*task = r;
			return XMI_SUCCESS;
		}
		*type = XMI_N_TORUS_NETWORK;
		// never happens?
		return XMI_ERROR;
	}
    }; // class Mapping
};	// namespace XMI
#endif // __components_mapping_mpi_mpimapping_h__
