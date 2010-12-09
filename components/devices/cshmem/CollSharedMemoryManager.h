/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/devices/cshmem/CollSharedMemoryManager.h
/// \brief Shared memory buffer manager class for CollShm device
///
/// The class defined in this file uses C++ templates. C++ templates
/// require all source code to be #include'd from a header file.
///
#ifndef __components_devices_cshmem_CollSharedMemoryManager_h__
#define __components_devices_cshmem_CollSharedMemoryManager_h__

/// \todo #warning This should move into the Coll/CCMI area - it is not PAMI::Memory!

#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include "Platform.h"

#undef TRACE_ERR
#ifndef TRACE_ERR
#define TRACE_ERR(x)   //fprintf x
#endif

#undef TRACE_DBG
#ifndef TRACE_DBG
#define TRACE_DBG(x)   //fprintf x
#endif

#ifndef PAMI_ASSERT
#define PAMI_ASSERT(x) PAMI_assert(x)
#endif

namespace PAMI
{
  namespace Memory
  {

    ///
    /// \brief Memory manager class for collective shared memory optimization
    ///        The shared memory region is composed of a contorl region, a region for
    ///        control structures of collective operations, and a region for data
    ///        buffers of collective operations.
    ///        T_SegSz is the size of the entire region managered by the memory manager
    ///        T_PageSz is the page size backing the shared memory region
    ///        T_WindowSz is the size of allocation unit for collective operation control structures
    ///        T_ShmBufSz is the size of shared memory data buffers

    template <class T_Atomic, unsigned T_SegSz, unsigned T_PageSz, unsigned T_WindowSz, unsigned T_ShmBufSz>
    class CollSharedMemoryManager
    {
      public:

      static const size_t _size     = T_SegSz;
      static const size_t _pgsize   = T_PageSz;
      static const size_t _windowsz = T_WindowSz;
      static const size_t _shmbufsz = T_ShmBufSz;

      union shm_ctrl_str_t {
        char    ctrl[_windowsz];
        shm_ctrl_str_t *next;
      } __attribute__ ((__aligned__ (CACHEBLOCKSZ)));
      typedef shm_ctrl_str_t ctlstr_t;

      union shm_data_buf_t {
        char           data[_shmbufsz];
        shm_data_buf_t *next;
      } __attribute__ ((__aligned__ (CACHEBLOCKSZ)));
      typedef shm_data_buf_t databuf_t;

      struct CollSharedMemory {
        T_Atomic       ready_count;
        T_Atomic       term_count;
        T_Atomic       ctlstr_pool_lock;
        T_Atomic       buffer_pool_lock;
        void           *ctlstr_memory;
        void           *buffer_memory;
        volatile ctlstr_t       *free_ctlstr_list;
        T_Atomic       *ctlstr_list;
        ctlstr_t       *ctlstr_pool;
        volatile databuf_t      *free_buffer_list;
        T_Atomic       *buffer_list;
        databuf_t      *buffer_pool;
      } __attribute__ ((__aligned__ (CACHEBLOCKSZ)));
      typedef CollSharedMemory collshm_t;

        CollSharedMemoryManager () :
          _nctrlstrs(0),
          _ndatabufs(0),
          _collshm (NULL),
          _localrank (__global.mapping.task()), // hacking for now, only support single node job
          _localsize (__global.mapping.size())  // hacking for now, only support single node job
        { }

	static void _collshminit(void *mem, size_t bytes, const char *key, unsigned attrs, void *cookie) {
	    CollSharedMemoryManager *thus = (CollSharedMemoryManager *)cookie;
	    thus->__collshminit(mem, bytes, key, attrs);
	}

	void __collshminit(void *mem, size_t bytes, const char *key, unsigned attrs) {

            // both shm_open and shmget should guarantee initial
            // content of the shared memory to be zero
            // _collshm->ready_count = 0;
            // _collshm->term_count  = 0;
            // _collshm->buffer_pool_lock = 0;
            // _collshm->ctlstr_pool_lock = 0;

            _collshm->ctlstr_pool = (ctlstr_t *)((char  *)_collshm + sizeof(collshm_t));
            _collshm->buffer_pool = (databuf_t *)((char *)(_collshm->ctlstr_pool) + (_size - sizeof(collshm_t))/2);

            _collshm->ctlstr_memory  = _collshm->ctlstr_pool;
            _collshm->buffer_memory  = _collshm->buffer_pool;

            _collshm->free_ctlstr_list   = _get_ctrl_str_from_pool();
            _collshm->ctlstr_list        = new (&(_collshm->free_ctlstr_list)) T_Atomic();
            _collshm->free_buffer_list   = _get_data_buf_from_pool();
            _collshm->buffer_list        = new (&(_collshm->free_buffer_list)) T_Atomic();

            // Shared memory setup hack for _world_geometry and context 0
            // Todo:  remove the following getCtrlStr()
            // Todo:  add the per geometry shared memory information to allreduce in analyze().
            getCtrlStr(_localsize);
            TRACE_DBG((stderr,"getSGCtrlStrVec() _localsize %zu, "
                              "_collshm->ctlstr_pool      %p, "
                              "_collshm->buffer_pool      %p, "
                              "_collshm->ctlstr_memory    %p, "
                              "_collshm->buffer_memory    %p, "
                              "_collshm->free_ctlstr_list %p, "
                              "_collshm->ctlstr_list      %p, "
                              "_collshm->free_buffer_list %p, "
                              "_collshm->buffer_list      %p  "
                                                           "\n",
                              _localsize,
                              _collshm->ctlstr_pool      ,
                              _collshm->buffer_pool      ,
                              _collshm->ctlstr_memory    ,
                              _collshm->buffer_memory    ,
                              _collshm->free_ctlstr_list ,
                              _collshm->ctlstr_list      ,
                              _collshm->free_buffer_list ,
                              _collshm->buffer_list      ));
	}

        int init(size_t rank, size_t size) {

          _localrank = rank;
          _localsize = size;

	  __global.mm.memalign((void **)&_collshm, 16, _size, "/pami-collshmem",
					_collshminit, (void *)this);
          return PAMI_SUCCESS;
        }


        ~CollSharedMemoryManager()
        {
            // placeholder for other cleanup work

            if (_collshm != NULL)
            {
              __global.mm.free(_collshm);

		/// \todo this should be a barrier, if needed at all.
               _collshm->term_count.fetch_and_inc();
               //COLLSHM_FETCH_AND_ADD((atomic_p)&_collshm->term_count, 1);
               while (_collshm->term_count.fetch() < _localsize);
            }

        }

        ///
        /// \brief Get a whole chunk of INIT_BUFCNT new data buffers from the pool
        ///        Hold buffer pool lock.
        ///
        /// \return A chunk of INIT_BUFCNT new data buffers chained together
        ///

        databuf_t * _get_data_buf_from_pool ()
        {

          // require a implementation of check_lock and clear_lock in atomic class
          //while(COLLSHM_CHECK_LOCK((atomic_p)&(_collshm->buffer_pool_lock), 0, 1));
          while(!_collshm->buffer_pool_lock.compare_and_swap((size_t)0, (size_t)1)) yield();
          mem_isync();
          databuf_t *new_bufs = _collshm->buffer_pool;
          databuf_t *bufs     = _collshm->buffer_pool;

          if ((char *)(new_bufs + COLLSHM_INIT_BUFCNT) > ((char *)_collshm + _size)) {
            TRACE_ERR((stderr,"Run out of shm data bufs, base=%p, buffer_memory=%p, boundary=%p, end=%p\n",
                        _collshm, _collshm->buffer_memory, (char *)_collshm+_size,
                        (char *)(new_bufs+COLLSHM_INIT_BUFCNT)));
            PAMI_ASSERT(0);
            return NULL;
          }

          for (int i = 0; i < COLLSHM_INIT_BUFCNT-1; ++i)
          {
             new_bufs->next = new_bufs + 1;
             new_bufs       = new_bufs->next;
          }
          new_bufs->next = NULL;

          _collshm->buffer_pool += COLLSHM_INIT_BUFCNT;
          //COLLSHM_CLEAR_LOCK((atomic_p)&(_collshm->buffer_pool_lock),0);
          mem_barrier();
          _collshm->buffer_pool_lock.compare_and_swap((size_t)1,(size_t)0);

          return bufs;
        }


        ///
        /// \brief get shm data buffers either from the free list or from the pool
        ///        when free list is empty. No lock is held, rely on atomic operation
        ///        to guarantee integrity
        ///
        /// \param count Number of buffers requested
        ///
        /// \return point to list of shmem data buffers
        ///

        databuf_t *getDataBuffer (unsigned count)
        {

          PAMI_ASSERT(count <= COLLSHM_INIT_BUFCNT);

          unsigned buf_count = 0;
          databuf_t * cur;
          databuf_t *next, *tmp, *buffers = NULL;

          while (buf_count < count)
          {
            // cur = (databuf_t *)_collshm->free_buffer_list;
            cur = (databuf_t *)_collshm->buffer_list->fetch();
            if (cur == NULL)
            {
              tmp   = _get_data_buf_from_pool();   // Allocate a whold chunk of INIT_BUFCNT new buffers from the pool
              cur   = tmp + count - buf_count - 1; // End of the list satisfying the requirement
              next  = cur->next;                   // Extra buffers that should be put into free list

              cur->next = buffers;                 // Merge with buffers already allocated
              buffers = tmp;

              cur = tmp + COLLSHM_INIT_BUFCNT - 1;            // End of the newly allocated chunk
              // Merge with free list
              // cur->next = (databuf_t *)_collshm->free_buffer_list;
              cur->next = (databuf_t *)_collshm->buffer_list->fetch();
              // while(!COLLSHM_COMPARE_AND_SWAPLP((atomic_l)&(_collshm->free_buffer_list), (long *)&(cur->next), (long)next));
              while(!_collshm->buffer_list->compare_and_swap((size_t)cur->next, (size_t)next)) {
                 //cur->next = (databuf_t *)_collshm->free_buffer_list;
                 cur->next = (databuf_t *)_collshm->buffer_list->fetch();
              }

              buf_count = count;
              TRACE_DBG((stderr,"new buffer is %p\n",buffers));
              continue;
            }

            next = cur->next;
            TRACE_DBG((stderr,"start: cur = %p, cur->next = %p and _collshm->free_buffer_list = %p\n",
                        cur, next, _collshm->free_buffer_list));

            //while (!COLLSHM_COMPARE_AND_SWAPLP((atomic_l)&(_collshm->free_buffer_list),(long *)&cur, (long)next))
            while (!_collshm->buffer_list->compare_and_swap((size_t)cur, (size_t)next))
            {
               TRACE_DBG((stderr,"entry cur = %p, cur->next = %p and _collshm->free_buffer_list = %p\n",
                     cur, next, _collshm->free_buffer_list));

               // cur = (databuf_t *)_collshm->free_buffer_list;
               cur = (databuf_t *)_collshm->buffer_list->fetch();

               if (cur == NULL)
                 next = NULL;  // take care of the case in which free list becomes empty
               else
                 next = cur->next;

               TRACE_DBG((stderr,"exit cur = %p, cur->next = %p and _collshm->free_buffer_list = %p\n",
                         cur, next, _collshm->free_buffer_list));
            }
            if (cur == NULL) continue;  // may need to start over

            TRACE_DBG((stderr,"end cur = %p\n", cur));
            cur->next = buffers;
            buffers = (databuf_t *)cur;
            buf_count ++;
          }

          _ndatabufs += count;
          TRACE_DBG((stderr,"_ndatabufs = %zu\n", _ndatabufs));
          return buffers;
        }


        ///
        /// \brief Return a list of shm data buf to the free list
        ///
        /// \param data_buf pointer to data bufs returned
        ///

        void returnDataBuffer (databuf_t *data_buf)
        {

          PAMI_ASSERT(data_buf != NULL);

          databuf_t *tmp = data_buf;

          while (tmp->next != NULL) {
            tmp = tmp->next;
            --_ndatabufs;
          }
          // tmp->next = _collshm->free_buffer_list;
          tmp->next = (databuf_t *)_collshm->buffer_list->fetch();

          //while(!COLLSHM_COMPARE_AND_SWAPLP((atomic_l)&(_collshm->free_buffer_list),(long *)&(tmp->next), (long)tmp));
          while(!_collshm->buffer_list->compare_and_swap((size_t)tmp->next, (size_t)tmp)) {
            //tmp->next = _collshm->free_buffer_list;
            tmp->next = (databuf_t *)_collshm->buffer_list->fetch();
          }

          TRACE_DBG((stderr,"_ndatabufs = %d\n", _ndatabufs));

        }

        ///
        /// \brief Get a whole chunk of INIT_CTLCNT new ctrl struct from the pool
        ///        Hold buffer pool lock.
        ///
        /// \return A chunk of INIT_CTLCNT new ctrl struct chained together
        ///

        ctlstr_t * _get_ctrl_str_from_pool ()
        {

          // require implementation of check_lock and clear_lock in atomic class
          // while(COLLSHM_CHECK_LOCK((atomic_p)&(_collshm->ctlstr_pool_lock), 0, 1));
          while(!_collshm->ctlstr_pool_lock.compare_and_swap((size_t)0, (size_t)1)) yield();
          // is mem_isync() equivalent to isync() on PERCS ?
          mem_isync();
          ctlstr_t *ctlstr   = _collshm->ctlstr_pool;
          ctlstr_t *tmp      = _collshm->ctlstr_pool;

          if ((char *)(ctlstr + COLLSHM_INIT_CTLCNT) > ((char *)_collshm->buffer_memory)) {
            TRACE_ERR((stderr,"Run out of shm ctrl structs, base=%p, ctrl_memory=%p, boundary=%p, end=%p\n",
                        _collshm, _collshm->ctlstr_memory, (char *)_collshm->buffer_memory,
                        (char *)(ctlstr+COLLSHM_INIT_CTLCNT)));
            PAMI_ASSERT(0);
            return NULL;
          }


          for (int i = 0; i < COLLSHM_INIT_CTLCNT-1; ++i)
          {
             tmp->next = tmp + 1;
             tmp       = tmp->next;
          }
          tmp->next = NULL;

          _collshm->ctlstr_pool += COLLSHM_INIT_CTLCNT;
          //COLLSHM_CLEAR_LOCK((atomic_p)&(_collshm->ctlstr_pool_lock),0);
          mem_barrier();
          _collshm->ctlstr_pool_lock.compare_and_swap((size_t)1,(size_t)0);

          return ctlstr;
        }

        ///
        /// \brief get shm ctrl structs either from the free list or from the pool
        ///        when free list is empty. No lock is held, rely on atomic operation
        ///        to guarantee integrity
        ///
        /// \param count Number of ctrl structs requested
        ///
        /// \return point to list of shmem ctrl structs
        ///

        ctlstr_t *getCtrlStr (unsigned count)
        {

          PAMI_ASSERT(count <= COLLSHM_INIT_CTLCNT);

          unsigned ctlstr_count = 0;
          ctlstr_t * cur;
          ctlstr_t *next, *tmp, *ctlstr = NULL;

          // TRACE_DBG((stderr,"_collshm->free_ctlstr_list = %lx\n", _collshm->free_ctlstr_list));
          while (ctlstr_count < count)
          {
            // cur = (ctlstr_t * )_collshm->free_ctlstr_list;
            cur = (ctlstr_t *)_collshm->ctlstr_list->fetch();
            if (cur == NULL)
            {
              tmp   = _get_ctrl_str_from_pool();       // Allocate a whold chunk of INIT_CTLCNT new buffers from the pool
              cur   = tmp + count - ctlstr_count - 1;  // End of the list satisfying the requirement
              next  = cur->next;                       // Extra buffers that should be put into free list

              cur->next = (ctlstr_t *)ctlstr;                    // Merge with ctrl structs already allocated
              ctlstr   = tmp;

              cur = tmp + COLLSHM_INIT_CTLCNT - 1;             // End of the newly allocated chunk
              // Merge with free list
              // cur->next = (ctlstr_t *)_collshm->free_ctlstr_list;
              cur->next = (ctlstr_t *)_collshm->ctlstr_list->fetch();

              // while(!COLLSHM_COMPARE_AND_SWAPLP((atomic_l)&(_collshm->free_ctlstr_list),(long *)&(cur->next), (long)next));
              while(!_collshm->ctlstr_list->compare_and_swap((size_t)cur->next, (size_t)next)) {
                // cur->next = (ctlstr_t *)_collshm->free_ctlstr_list;
                cur->next = (ctlstr_t *)_collshm->ctlstr_list->fetch();
              }

              ctlstr_count = count;
              continue;
            }

            next = cur->next;
            //while (!COLLSHM_COMPARE_AND_SWAPLP((atomic_l)&(_collshm->free_ctlstr_list),(long *)&cur, (long)next))
            while (!_collshm->ctlstr_list->compare_and_swap((size_t)cur, (size_t)next))
            {
               // cur = (ctlstr_t * )_collshm->free_ctlstr_list;
               cur = (ctlstr_t *)_collshm->ctlstr_list->fetch();
               if (cur == NULL)
                 next = NULL;  // take care of the case in which free list becomes empty
               else
                 next = cur->next;
               TRACE_DBG((stderr,"cur = %p\n", cur));
            }
            if (cur == NULL) continue;  // may need to start over

            cur->next = (ctlstr_t *)ctlstr;
            ctlstr = (ctlstr_t *)cur;
            ctlstr_count ++;
          }

          _nctrlstrs += count;
          TRACE_DBG((stderr,"_nctrlstrs = %zu, ctlstr %p\n", _nctrlstrs,ctlstr));
          return (ctlstr_t *)ctlstr;
        }

        ///
        /// \brief Return a list of shm ctrl struct to the free list
        ///
        /// \param ctlstr pointer to a list of ctrl structs to be returned
        ///

        void returnCtrlStr (ctlstr_t *ctlstr)
        {

          PAMI_ASSERT(ctlstr != NULL);

          ctlstr_t *tmp = ctlstr;

          while (tmp->next != NULL) {
            tmp = tmp->next;
            -- _nctrlstrs;
          }
          //tmp->next = _collshm->free_ctlstr_list;
          tmp->next = (ctlstr_t *)_collshm->ctlstr_list->fetch();

          // while(!COLLSHM_COMPARE_AND_SWAPLP((atomic_l)&(_collshm->free_ctlstr_list),(long *)&(tmp->next), (long)tmp));
          while(!_collshm->ctlstr_list->compare_and_swap((size_t)tmp->next, (size_t)tmp)) {
            // tmp->next =  _collshm->free_ctlstr_list;
            tmp->next = (ctlstr_t *)_collshm->ctlstr_list->fetch();
          }

          TRACE_DBG((stderr,"_nctrlstrs = %d\n", _nctrlstrs));

        }

        // get coll shmem control struture address for world geometry
        ctlstr_t *getWGCtrlStr()
        {
          TRACE_DBG((stderr,"_collshm %p \n",_collshm));
          TRACE_DBG((stderr,"WGCtrlStr = %p\n", ((ctlstr_t *)_collshm->ctlstr_memory + (_localsize -1))));
          return ((ctlstr_t *)_collshm->ctlstr_memory + (_localsize -1));
        }

        // fill in a vector of coll shmem control structure addresses for sub-geometries
        // perform allreduce on the vector during geometry analyze()
        void getSGCtrlStrVec(pami_geometry_t geo, uint64_t *vec)
        {

          PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *)geo;
          PAMI::Topology *local_topo    = (PAMI::Topology *)geometry->getTopology(PAMI::Geometry::LOCAL_TOPOLOGY_INDEX);
          PAMI::Topology *lm_topo       = (PAMI::Topology *)geometry->getTopology(PAMI::Geometry::MASTER_TOPOLOGY_INDEX);

          uint master_rank              =  local_topo->index2Rank(0);
          uint master_size              =  lm_topo->size();
          uint master_index             =  lm_topo->rank2Index(master_rank);
          uint local_size               =  local_topo->size();
          uint local_index              =  local_topo->rank2Index(__global.mapping.task());
          TRACE_DBG((stderr,"getSGCtrlStrVec() geometry %p, master_rank %u, master_size %u, master_index %u, local_size %u, local_index %u\n",
                     geo, master_rank, master_size, master_index, local_size, local_index));

          for (uint i = 0; i< master_size; ++i) vec[i] = 0xFFFFFFFFFFFFFFFFULL;
          if (local_index == 0)
            vec[master_index] = (uint64_t) getCtrlStr(local_size);
        }

      protected:

        size_t     _nctrlstrs;
        size_t     _ndatabufs;
        collshm_t *_collshm;       // base pointer of the shared memory segment
        size_t     _localrank;      // rank in the local topology
        size_t     _localsize;      // size of the local topology


    };  // class CollSharedmemoryManager
  };  // namespace Memory
};   // namespace PAMI

#undef PAMI_ASSERT
#undef TRACE_ERR
#undef TRACE_DBG
#endif // __pami_components_devices_cshmem_collsharedmemorymanager_h__
