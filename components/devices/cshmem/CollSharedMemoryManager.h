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
#include "Memory.h"

#undef TRACE_ERR
#ifndef TRACE_ERR
#define TRACE_ERR(x)  // fprintf x
#endif

#undef TRACE_DBG
#ifndef TRACE_DBG
#define TRACE_DBG(x)  // fprintf x
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




    template <class T_Atomic, class T_Mutex, class T_Counter, unsigned T_SegSz, unsigned T_PageSz, unsigned T_WindowSz, unsigned T_ShmBufSz>
    class CollSharedMemoryManager
    {
      public:

        static const size_t _size     = T_SegSz;
        static const size_t _pgsize   = T_PageSz;
        static const size_t _windowsz = T_WindowSz;
        static const size_t _shmbufsz = T_ShmBufSz;

        inline size_t addr_to_offset(void *addr)
        {
          return (size_t)addr - (size_t)_collshm;
        }
        inline void *offset_to_addr(size_t offset)
        {
          return (void*)((char*)_collshm + offset);
        }
        union shm_ctrl_str_t
        {
          char     ctrl[_windowsz];
          size_t   next_offset;
        } __attribute__ ((__aligned__ (CACHEBLOCKSZ)));
        typedef shm_ctrl_str_t ctlstr_t;

        union shm_data_buf_t
        {
          char           data[_shmbufsz];
          size_t         next_offset;
        } __attribute__ ((__aligned__ (CACHEBLOCKSZ)));
        typedef shm_data_buf_t databuf_t;

        struct CollSharedMemory
        {
          T_Counter           ready_count;
          T_Counter           term_count;
          T_Mutex             ctlstr_pool_lock;
          T_Mutex             buffer_pool_lock;
          volatile uint64_t   clientdata_key;
          size_t              ctlstr_offset;
          size_t              buffer_offset;
          volatile size_t     free_ctlstr_list_offset;
          size_t              ctlstr_list_offset;
          size_t              ctlstr_pool_offset;
          volatile size_t     free_buffer_list_offset;
          size_t              buffer_list_offset;
          size_t              buffer_pool_offset;

        } __attribute__ ((__aligned__ (CACHEBLOCKSZ)));
        typedef CollSharedMemory collshm_t;

        CollSharedMemoryManager (Memory::MemoryManager *mm) :
          _nctrlstrs(0),
          _ndatabufs(0),
          _collshm (NULL),
          _localrank (__global.mapping.task()), // hacking for now, only support single node job
          _localsize (__global.mapping.size()), // hacking for now, only support single node job
          _key(0),
          _mm(mm)
        { }

        static void _collshminit(void *mem, size_t bytes, const char *key, unsigned attrs, void *cookie)
        {
          CollSharedMemoryManager *thus = (CollSharedMemoryManager *)cookie;
          thus->__collshminit(mem, bytes, key, attrs);
        }

        void __collshminit(void *mem, size_t bytes, const char *key, unsigned attrs)
        {

          // both shm_open and shmget should guarantee initial
          // content of the shared memory to be zero
          // _collshm->ready_count = 0;
          // _collshm->term_count  = 0;
          // _collshm->buffer_pool_lock = 0;
          // _collshm->ctlstr_pool_lock = 0;

          _collshm->ctlstr_pool_offset      = addr_to_offset(((char  *)_collshm + sizeof(collshm_t)));
          _collshm->buffer_pool_offset      = addr_to_offset(((char *)(offset_to_addr(_collshm->ctlstr_pool_offset))
                                                         + (_size - sizeof(collshm_t)) / 2));

          _collshm->clientdata_key          = _key;
          _collshm->ctlstr_offset           = (size_t)offset_to_addr(_collshm->ctlstr_pool_offset) - (size_t)_collshm;
          _collshm->buffer_offset           = (size_t)offset_to_addr(_collshm->buffer_pool_offset) - (size_t)_collshm;

          _collshm->free_ctlstr_list_offset = addr_to_offset(_get_ctrl_str_from_pool());
          _collshm->ctlstr_list_offset      = (size_t)addr_to_offset(new (offset_to_addr(_collshm->free_ctlstr_list_offset)) T_Atomic());
          _collshm->free_buffer_list_offset = addr_to_offset(_get_data_buf_from_pool());
          _collshm->buffer_list_offset      = addr_to_offset(new (offset_to_addr(_collshm->free_buffer_list_offset)) T_Atomic());

          TRACE_DBG((stderr, "_collshm                  %p,    "
                     "getSGCtrlStrVec() _localsize      %lu, "
                     "_collshm->ctlstr_pool_offset      %lu, "
                     "_collshm->buffer_pool_offset      %lu, "
                     "_collshm->ctlstr_offset           %lu, "
                     "_collshm->buffer_offset           %lu, "
                     "_collshm->free_ctlstr_list_offset %lu, "
                     "_collshm->ctlstr_list_offset      %lu, "
                     "_collshm->free_buffer_list_offset %lu, "
                     "_collshm->buffer_list_offset      %lu  "
                     "\n",
                     _collshm,
                     _localsize,
                     _collshm->ctlstr_pool_offset      ,
                     _collshm->buffer_pool_offset      ,
                     _collshm->ctlstr_offset    ,
                     _collshm->buffer_offset    ,
                     _collshm->free_ctlstr_list_offset ,
                     _collshm->ctlstr_list_offset      ,
                     _collshm->free_buffer_list_offset ,
                     _collshm->buffer_list_offset));
        }

        pami_result_t init(size_t rank, size_t size)
        {
          pami_result_t rc = PAMI_SUCCESS;
          _localrank = rank;
          _localsize = size;
          if(_mm)
          {
            PAMI_assertf(T_Counter::checkCtorMm(_mm), "Memory type incorrect for T_Counter class");
            PAMI_assertf(T_Mutex::checkCtorMm(_mm), "Memory type incorrect for T_Mutex class");            
            rc = _mm->memalign((void **)&_collshm, 16, _size, "/pami-collshmem",
                               _collshminit, (void *)this);
          }
          else
            rc = PAMI_ENOMEM;

          return rc;
        }

        pami_result_t init(size_t rank, size_t size, uint64_t key)
        {
          _key = key;
          return init(rank, size);
        }

        void * getCollShmAddr()
        {
          return (void*) _collshm;
        }

        ~CollSharedMemoryManager()
        {
          // placeholder for other cleanup work
          if (_collshm != NULL)
            {
              _mm->free(_collshm);
              /// \todo this should be a barrier, if needed at all.
              _collshm->term_count.fetch_and_inc();

              //COLLSHM_FETCH_AND_ADD((atomic_p)&_collshm->term_count, 1);
              while (_collshm->term_count.fetch() < _localsize);
            }

        }

        uint64_t getKey()
        {
          uint64_t returnKey;
          _collshm->buffer_pool_lock.acquire();
          returnKey = _collshm->clientdata_key;
          _collshm->buffer_pool_lock.release();
          return returnKey;
        }
        uint64_t getAndSetKey(uint64_t in)
        {
          uint64_t returnKey;
          _collshm->buffer_pool_lock.acquire();
          returnKey = _collshm->clientdata_key;
          _collshm->clientdata_key = in;
          _collshm->buffer_pool_lock.release();
          return returnKey;
        }

        void setKeyBit(int index)
        {
          _collshm->buffer_pool_lock.acquire();
          _collshm->clientdata_key |= (1ULL << (index));
          _collshm->buffer_pool_lock.release();
          return;
        }


        void setKey(uint64_t in)
        {
          _collshm->buffer_pool_lock.acquire();
          _collshm->clientdata_key = in;
          _collshm->buffer_pool_lock.release();
          return;
        }

        void setKeyOR(uint64_t in)
        {
          _collshm->buffer_pool_lock.acquire();
          _collshm->clientdata_key |= in;
          _collshm->buffer_pool_lock.release();
          return;
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
          _collshm->buffer_pool_lock.acquire();
          Memory::sync<Memory::instruction>();
          databuf_t *new_bufs = (databuf_t*) offset_to_addr(_collshm->buffer_pool_offset);
          databuf_t *bufs     = new_bufs;;

          if ((char *)(new_bufs + COLLSHM_INIT_BUFCNT) > ((char *)_collshm + _size))
            {
              TRACE_ERR((stderr, "Run out of shm data bufs, base=%p, buffer_offset=%lu, boundary=%p, end=%p\n",
                         _collshm, _collshm->buffer_offset, (char *)_collshm + _size,
                         (char *)(new_bufs + COLLSHM_INIT_BUFCNT)));
              PAMI_ASSERT(0);
              return NULL;
            }

          for (int i = 0; i < COLLSHM_INIT_BUFCNT - 1; ++i)
            {
              new_bufs->next_offset = addr_to_offset(new_bufs + 1);
              new_bufs              = (databuf_t*)offset_to_addr(new_bufs->next_offset);
            }

          new_bufs->next_offset = addr_to_offset(NULL);

          _collshm->buffer_pool_offset += (COLLSHM_INIT_BUFCNT*sizeof(databuf_t));
          //COLLSHM_CLEAR_LOCK((atomic_p)&(_collshm->buffer_pool_lock),0);
          Memory::sync();
          _collshm->buffer_pool_lock.release();

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

          T_Atomic *buffer_list = (T_Atomic*)offset_to_addr(_collshm->buffer_list_offset);
          while (buf_count < count)
            {
              cur = (databuf_t *)offset_to_addr(buffer_list->fetch());

              if (cur == NULL)
                {
                  tmp   = _get_data_buf_from_pool();   // Allocate a whold chunk of INIT_BUFCNT new buffers from the pool
                  cur   = tmp + count - buf_count - 1; // End of the list satisfying the requirement
                  next  = (databuf_t*)offset_to_addr(cur->next_offset);   // Extra buffers that should be put into free list

                  cur->next_offset = addr_to_offset(buffers);                 // Merge with buffers already allocated
                  buffers = tmp;

                  cur = tmp + COLLSHM_INIT_BUFCNT - 1;            // End of the newly allocated chunk
                  // Merge with free list
                  cur->next_offset = buffer_list->fetch();

                  while(!buffer_list->bool_compare_and_swap(cur->next_offset, addr_to_offset(next)))
                    {
                      cur->next_offset = buffer_list->fetch();
                    }

                  buf_count = count;
                  TRACE_DBG((stderr, "new buffer is %p\n", buffers));
                  continue;
                }

              next = (databuf_t*)offset_to_addr(cur->next_offset);
              TRACE_DBG((stderr, "start: cur = %p, cur->next = %p and _collshm->free_buffer_list_offset = %lu\n",
                         cur, next, _collshm->free_buffer_list_offset));

              while (!buffer_list->bool_compare_and_swap(addr_to_offset(cur), addr_to_offset(next)))
                {
                  TRACE_DBG((stderr, "entry cur = %p, cur->next = %p and _collshm->free_buffer_list_offset = %lu\n",
                             cur, next, _collshm->free_buffer_list_offset));

                  cur = (databuf_t*)offset_to_addr(buffer_list->fetch());
                  if (cur == NULL)
                    next = NULL;  // take care of the case in which free list becomes empty
                  else
                    next = (databuf_t*)offset_to_addr(cur->next_offset);
                  TRACE_DBG((stderr, "exit cur = %p, cur->next = %p and _collshm->free_buffer_list_offset = %lu\n",
                             cur, next, _collshm->free_buffer_list_offset));
                }

              if (cur == NULL) continue;  // may need to start over

              TRACE_DBG((stderr, "end cur = %p\n", cur));
              cur->next_offset = addr_to_offset(buffers);
              buffers = (databuf_t *)cur;
              buf_count ++;
            }

          _ndatabufs += count;
          TRACE_DBG((stderr, "_ndatabufs = %zu\n", _ndatabufs));
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
          T_Atomic  *buffer_list = (T_Atomic*)offset_to_addr(_collshm->buffer_list_offset);
          while (tmp->next_offset != addr_to_offset(NULL))
            {
              tmp = (databuf_t*)offset_to_addr(tmp->next_offset);
              --_ndatabufs;
            }
          tmp->next_offset = buffer_list->fetch();
          while(!buffer_list->bool_compare_and_swap(tmp->next_offset, addr_to_offset(tmp)))
            {
              tmp->next_offset = buffer_list->fetch();
            }

          TRACE_DBG((stderr, "_ndatabufs = %d\n", _ndatabufs));

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
          PAMI_ASSERT(&_collshm->ctlstr_pool_lock != NULL);
          _collshm->ctlstr_pool_lock.acquire();
          // is Memory::sync<Memory::instruction>() equivalent to isync() on PERCS ?
          Memory::sync<Memory::instruction>();
          ctlstr_t *ctlstr   = (ctlstr_t*)offset_to_addr(_collshm->ctlstr_pool_offset);
          ctlstr_t *tmp      = ctlstr;
          size_t    offset   = (size_t)offset_to_addr(_collshm->buffer_pool_offset) - (size_t)_collshm;
          if ((char *)(ctlstr + COLLSHM_INIT_CTLCNT) > ((char *)_collshm + offset))
            {
              TRACE_ERR((stderr, "Run out of shm ctrl structs: base=%p, ctrl_offset=%lu, boundary=%p, end=%p\n",
                         _collshm, _collshm->ctlstr_offset, (char *)_collshm + offset,
                         (char *)(ctlstr + COLLSHM_INIT_CTLCNT)));
              PAMI_ASSERT(0);
              return NULL;
            }


          for (int i = 0; i < COLLSHM_INIT_CTLCNT - 1; ++i)
            {
              tmp->next_offset = addr_to_offset(tmp + 1);
              tmp              = (ctlstr_t*)offset_to_addr(tmp->next_offset);
            }
          tmp->next_offset = addr_to_offset(NULL);

          _collshm->ctlstr_pool_offset += (COLLSHM_INIT_CTLCNT*sizeof(ctlstr_t));
          //COLLSHM_CLEAR_LOCK((atomic_p)&(_collshm->ctlstr_pool_lock),0);
          Memory::sync();
          _collshm->ctlstr_pool_lock.release();

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

          T_Atomic *ctlstr_list = (T_Atomic *) offset_to_addr(_collshm->ctlstr_list_offset);
          while (ctlstr_count < count)
            {
              PAMI_ASSERT(_collshm != NULL);
              PAMI_ASSERT(ctlstr_list != NULL);
              cur = (ctlstr_t *)offset_to_addr(ctlstr_list->fetch());

              if (cur == NULL)
                {
                  tmp   = _get_ctrl_str_from_pool();       // Allocate a whold chunk of INIT_CTLCNT new buffers from the pool
                  cur   = tmp + count - ctlstr_count - 1;  // End of the list satisfying the requirement
                  next  = (ctlstr_t*)offset_to_addr(cur->next_offset);// Extra buffers that should be put into free list

                  cur->next_offset = addr_to_offset(ctlstr);                    // Merge with ctrl structs already allocated
                  ctlstr   = tmp;

                  cur = tmp + COLLSHM_INIT_CTLCNT - 1;             // End of the newly allocated chunk
                  // Merge with free list
                  cur->next_offset = ctlstr_list->fetch();

                  while(!ctlstr_list->bool_compare_and_swap(cur->next_offset, addr_to_offset(next)))
                    {
                      cur->next_offset = ctlstr_list->fetch();
                    }

                  ctlstr_count = count;
                  continue;
                }

              next = (ctlstr_t*)offset_to_addr(cur->next_offset);

              while (!ctlstr_list->bool_compare_and_swap(addr_to_offset(cur), addr_to_offset(next)))
                {
                  cur = (ctlstr_t*)offset_to_addr(ctlstr_list->fetch());

                  if (cur == NULL)
                    next = NULL;  // take care of the case in which free list becomes empty
                  else
                    next = (ctlstr_t*)offset_to_addr(cur->next_offset);

                  TRACE_DBG((stderr, "cur = %p\n", cur));
                }

              if (cur == NULL) continue;  // may need to start over

              cur->next_offset = addr_to_offset(ctlstr);
              ctlstr = (ctlstr_t *)cur;
              ctlstr_count ++;
            }

          _nctrlstrs += count;
          TRACE_DBG((stderr, "_nctrlstrs = %zu, ctlstr %p\n", _nctrlstrs, ctlstr));
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

          ctlstr_t *tmp         = ctlstr;
          T_Atomic *ctlstr_list = (T_Atomic *) offset_to_addr(_collshm->ctlstr_list_offset);

          // Decrement the number of control strings
          // and count the number in the list
          int cnt = 0;
          while (tmp != NULL)
            { 
              tmp = (ctlstr_t*)offset_to_addr(tmp->next_offset);
              -- _nctrlstrs;
              cnt++;
            }
          ctlstr_t *cur=ctlstr;
          for(int i=0; i<cnt; i++)
          {
            ctlstr_t *next = (ctlstr_t*)offset_to_addr(cur->next_offset);            
            memset(((char*)cur)+sizeof(size_t), 0, sizeof(*cur)-sizeof(size_t));
            Memory::sync();
            cur->next_offset = ctlstr_list->fetch();
            while(!ctlstr_list->bool_compare_and_swap(cur->next_offset, addr_to_offset(cur)))
            {
              cur->next_offset = ctlstr_list->fetch();
            }
            cur=next;
          }
          TRACE_DBG((stderr, "_nctrlstrs = %d\n", _nctrlstrs));

        }

        // get coll shmem control struture address for world geometry
        ctlstr_t *getWGCtrlStr()
        {
          TRACE_DBG((stderr, "_collshm %p \n", _collshm));
          TRACE_DBG((stderr, "WGCtrlStr = %p\n", ((ctlstr_t *)((char*)_collshm + _collshm->ctlstr_offset) + (_localsize-1))));
          return ((ctlstr_t *)((char*)_collshm + _collshm->ctlstr_offset)) + (_localsize-1);
        }

        // fill in a vector of coll shmem control structure address offsets for sub-geometries
        // perform allreduce on the vector during geometry analyze()
        void getSGCtrlStrVec(pami_geometry_t geo,
                             uint64_t       *vec,
                             uint64_t       *ctlstr_offset)
        {

          PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *)geo;
          PAMI::Topology *local_topo    = (PAMI::Topology *)geometry->getTopology(PAMI::Geometry::LOCAL_TOPOLOGY_INDEX);
          PAMI::Topology *lm_topo       = (PAMI::Topology *)geometry->getTopology(PAMI::Geometry::MASTER_TOPOLOGY_INDEX);

          uint master_rank              =  local_topo->index2Rank(0);
          uint master_size              =  lm_topo->size();
          uint master_index             =  lm_topo->rank2Index(master_rank);
          uint local_size               =  local_topo->size();
          uint local_index              =  local_topo->rank2Index(__global.mapping.task());
          TRACE_DBG((stderr, "getSGCtrlStrVec() geometry %p, master_rank %u, master_size %u, master_index %u, local_size %u, local_index %u\n",
                     geo, master_rank, master_size, master_index, local_size, local_index));

          for (uint i = 0; i < master_size; ++i) vec[i] = 0xFFFFFFFFFFFFFFFFULL;

          if (local_index == 0)
            vec[master_index] = (uint64_t) addr_to_offset(getCtrlStr(local_size));

          *ctlstr_offset = vec[master_index];
        }
      
      void returnSGCtrlStr(uint64_t ctlstr_offset)
        {
          if(ctlstr_offset != 0xFFFFFFFFFFFFFFFFULL)
          {
            ctlstr_t *ctlstr = (ctlstr_t *)offset_to_addr(ctlstr_offset);
            returnCtrlStr(ctlstr);
          }
        }
      
      protected:
        size_t                    _nctrlstrs;
        size_t                    _ndatabufs;
        collshm_t                *_collshm;       // base pointer of the shared memory segment
        size_t                    _localrank;      // rank in the local topology
        size_t                    _localsize;      // size of the local topology
        uint64_t                  _key;
        Memory::MemoryManager    *_mm;

    };  // class CollSharedmemoryManager
  };  // namespace Memory
};   // namespace PAMI

#undef PAMI_ASSERT
#undef TRACE_ERR
#undef TRACE_DBG
#endif // __pami_components_devices_cshmem_collsharedmemorymanager_h__
