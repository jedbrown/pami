/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/memory/shmem/CollSharedMemoryManager.h
/// \brief Shared memory manager class
///
/// The class defined in this file uses C++ templates. C++ templates
/// require all source code to be #include'd from a header file.
///
#ifndef __components_memory_shmem_CollSharedMemoryManager_h__
#define __components_memory_shmem_CollSharedMemoryManager_h__

#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include "Platform.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x)  // printf x
#endif

#ifndef TRACE_DBG
#define TRACE_DBG(x)  // printf x
#endif

#ifndef PAMI_ASSERT
#define PAMI_ASSERT(x) assert(x)
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
          _shm_id(-1),
          _collshm (NULL),
          _localrank (__global.mapping.task()), // hacking for now, only support single node job
          _localsize (__global.mapping.size())  // hacking for now, only support single node job
        { }

        int init(size_t rank, size_t size) {

          _localrank = rank;
          _localsize = size;
          int lrank;

#ifdef _POSIX_SHM_OPEN
          _shm_id = shm_open(COLLSHM_KEY, O_CREAT|O_RDWR, 0600);
          if( _shm_id != -1) {
            if ( ftruncate(_shm_id, _size) != -1 )
            {
              _collshm = (collshm_t *) mmap(NULL, _size, PROT_READ|PROT_WRITE, MAP_SHARED, _shm_id, 0);
              if ( _collshm == MAP_FAILED ) {
                TRACE_ERR(("mmap failed, %d\n", errno));
                return PAMI_ERROR;
              }
            }
          } else {
            TRACE_ERR(("shm_open failed, %d\n",errno));
            return PAMI_ERROR;
          }
#else
          unsigned req_segs = 1000; // for xmem attach

          _shm_id = shmget(COLLSHM_KEY, _size, IPC_CREAT | IPC_EXCL | 0600);
          if (_shm_id != -1) {
            _collshm = (collshm_t *)shmat(_shm_id, 0, 0);
            if (_collshm == NULL || _collshm == (collshm_t *)-1) {
               TRACE_ERR(("master shmat failed: %d\n", errno));
               shmctl(_shm_id, IPC_RMID, NULL);
               return PAMI_ERROR;
            }
          } else {
            _shm_id = shmget(COLLSHM_KEY, 0, 0);
            if (_shm_id == -1) {
              TRACE_ERR(("slave shmget failed\n"));
              return PAMI_ERROR;
            }
            _collshm = (collshm_t *)shmat(_shm_id, 0, 0);
            if (_collshm == NULL || _collshm == (collshm_t *)-1) {
              TRACE_ERR(("slave shmat failed: %d\n", errno));
              shmctl(_shm_id, IPC_RMID, NULL);
              return PAMI_ERROR;
            }
          }
#endif /* _POSIX_SHM_OPEN */
          if (!_localrank) {

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

#ifdef _POSIX_SHM_OPEN
            shm_unlink("COLLSHM_KEY");
#else
            shmctl(_shm_id, IPC_RMID, NULL);
#endif
          }

          lrank = _collshm->ready_count.fetch_and_inc();
          //lrank = COLLSHM_FETCH_AND_ADD((atomic_p)&_collshm->ready_count, 1);
          TRACE_DBG(("task %d joined, _collshm=%p\n", _localrank, _collshm));
          while (_collshm->ready_count.fetch() < _localsize);

          return PAMI_SUCCESS;
        }


        ~CollSharedMemoryManager()
        {
            // placeholder for other cleanup work

            if (_collshm != NULL)
            {
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
            TRACE_ERR(("Run out of shm data bufs, base=%lp, buffer_memory=%lp, boundary=%lp, end=%lp\n",
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

          PAMI_ASSERT(count < COLLSHM_INIT_BUFCNT);

          unsigned buf_count = 0;
          databuf_t * volatile cur;
          databuf_t *next, *tmp, *buffers = NULL;

          while (buf_count < count)
          {
            cur = (databuf_t * volatile)_collshm->free_buffer_list;
            if (cur == NULL)
            {
              tmp   = _get_data_buf_from_pool();   // Allocate a whold chunk of INIT_BUFCNT new buffers from the pool
              cur   = tmp + count - buf_count - 1; // End of the list satisfying the requirement
              next  = cur->next;                   // Extra buffers that should be put into free list

              cur->next = buffers;                 // Merge with buffers already allocated
              buffers = tmp;

              cur = tmp + COLLSHM_INIT_BUFCNT - 1;            // End of the newly allocated chunk
              cur->next = (databuf_t *)_collshm->free_buffer_list; // Merge with free list
              // while(!COLLSHM_COMPARE_AND_SWAPLP((atomic_l)&(_collshm->free_buffer_list), (long *)&(cur->next), (long)next));
              while(!_collshm->buffer_list->compare_and_swap((long *)(&(cur->next)), (long)next)) {
                 cur->next = (databuf_t *)_collshm->free_buffer_list;
              }

              buf_count = count;
              TRACE_DBG(("new buffer is %p\n",buffers));
              continue;
            }

            next = cur->next;
            TRACE_DBG(("start: cur = %p, cur->next = %p and _collshm->free_buffer_list = %p\n",
                        cur, next, _collshm->free_buffer_list));

            //while (!COLLSHM_COMPARE_AND_SWAPLP((atomic_l)&(_collshm->free_buffer_list),(long *)&cur, (long)next))
            while (!_collshm->buffer_list->compare_and_swap((long *)(&cur), (long)next))
            {
               TRACE_DBG(("entry cur = %p, cur->next = %p and _collshm->free_buffer_list = %p\n",
                     cur, next, _collshm->free_buffer_list));

               cur = (databuf_t * volatile)_collshm->free_buffer_list;

               if (cur == NULL)
                 next = NULL;  // take care of the case in which free list becomes empty
               else
                 next = cur->next;

               TRACE_DBG(("exit cur = %p, cur->next = %p and _collshm->free_buffer_list = %p\n",
                         cur, next, _collshm->free_buffer_list));
            }
            if (cur == NULL) continue;  // may need to start over

            TRACE_DBG(("end cur = %p\n", cur));
            cur->next = buffers;
            buffers = cur;
            buf_count ++;
          }

          _ndatabufs += count;
          TRACE_DBG(("_ndatabufs = %d\n", _ndatabufs));
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
          tmp->next = _collshm->free_buffer_list;

          //while(!COLLSHM_COMPARE_AND_SWAPLP((atomic_l)&(_collshm->free_buffer_list),(long *)&(tmp->next), (long)tmp));
          while(!_collshm->buffer_list->compare_and_swap((long *)(&(tmp->next)), (long)tmp)) {
            tmp->next = _collshm->free_buffer_list;
          }

          TRACE_DBG(("_ndatabufs = %d\n", _ndatabufs));

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
            TRACE_ERR(("Run out of shm ctrl structs, base=%lp, ctrl_memory=%lp, boundary=%lp, end=%lp\n",
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

          PAMI_ASSERT(count < COLLSHM_INIT_CTLCNT);

          unsigned ctlstr_count = 0;
          ctlstr_t * volatile cur;
          ctlstr_t *next, *tmp, *ctlstr = NULL;

          // TRACE_DBG(("_collshm->free_ctlstr_list = %lx\n", _collshm->free_ctlstr_list));
          while (ctlstr_count < count)
          {
            cur = (ctlstr_t * volatile)_collshm->free_ctlstr_list;
            if (cur == NULL)
            {
              tmp   = _get_ctrl_str_from_pool();       // Allocate a whold chunk of INIT_CTLCNT new buffers from the pool
              cur   = tmp + count - ctlstr_count - 1;  // End of the list satisfying the requirement
              next  = cur->next;                       // Extra buffers that should be put into free list

              cur->next = (ctlstr_t *)ctlstr;                    // Merge with ctrl structs already allocated
              ctlstr   = tmp;

              cur = tmp + COLLSHM_INIT_CTLCNT - 1;             // End of the newly allocated chunk
              cur->next = (ctlstr_t *)_collshm->free_ctlstr_list; // Merge with free list
              // while(!COLLSHM_COMPARE_AND_SWAPLP((atomic_l)&(_collshm->free_ctlstr_list),(long *)&(cur->next), (long)next));
              while(!_collshm->ctlstr_list->compare_and_swap((long *)(&(cur->next)), (long)next)) {
                cur->next = (ctlstr_t *)_collshm->free_ctlstr_list;
              }

              ctlstr_count = count;
              continue;
            }

            next = cur->next;
            //while (!COLLSHM_COMPARE_AND_SWAPLP((atomic_l)&(_collshm->free_ctlstr_list),(long *)&cur, (long)next))
            while (!_collshm->ctlstr_list->compare_and_swap((long *)(&cur), (long)next))
            {
               cur = (ctlstr_t * volatile)_collshm->free_ctlstr_list;
               if (cur == NULL)
                 next = NULL;  // take care of the case in which free list becomes empty
               else
                 next = cur->next;
               TRACE_DBG(("cur = %lx\n", cur));
            }
            if (cur == NULL) continue;  // may need to start over

            cur->next = (ctlstr_t *)ctlstr;
            ctlstr = cur;
            ctlstr_count ++;
          }

          _nctrlstrs += count;
          TRACE_DBG(("_nctrlstrs = %d\n", _nctrlstrs));
          return (ctlstr_t *)ctlstr;
        }

        // hack for getting per geometry shared memory region, need to determine this information
        // geometry creation allreduce
        ctlstr_t *getWGCtrlStr()
        {
          TRACE_DBG(("WGCtrlStr = %x\n", ((ctlstr_t *)_collshm->ctlstr_memory + (_localsize -1))));
          return ((ctlstr_t *)_collshm->ctlstr_memory + (_localsize -1));
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
          tmp->next = _collshm->free_ctlstr_list;

          // while(!COLLSHM_COMPARE_AND_SWAPLP((atomic_l)&(_collshm->free_ctlstr_list),(long *)&(tmp->next), (long)tmp));
          while(!_collshm->ctlstr_list->compare_and_swap((long *)(&(tmp->next)), (long)tmp)) {
            tmp->next =  _collshm->free_ctlstr_list;
          }

          TRACE_DBG(("_nctrlstrs = %d\n", _nctrlstrs));

        }

      protected:

        size_t     _nctrlstrs;
        size_t     _ndatabufs;
        int        _shm_id;
        collshm_t *_collshm;       // base pointer of the shared memory segment
        size_t     _localrank;      // rank in the local topology
        size_t     _localsize;      // size of the local topology


    };  // class CollSharedmemoryManager
  };  // namespace Memory
};   // namespace PAMI

#undef PAMI_ASSERT
#undef TRACE_ERR
#undef TRACE_DBG
#endif // __pami_components_memory_shmem_collsharedmemorymanager_h__
