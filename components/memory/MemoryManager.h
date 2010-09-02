/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/memory/MemoryManager.h
/// \brief Base memory manager class
///
#ifndef __components_memory_MemoryManager_h__
#define __components_memory_MemoryManager_h__

#include <pami.h>
#include "util/common.h"
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif
	    _offset = ((((size_t)_base + sizeof(_mm_header)) +
				(_alignment - 1)) & ~(_alignment - 1)) -
			(size_t)_base;

namespace PAMI
{
  namespace Memory
  {
    class MemoryManager
    {
      public:
	static const int MMKEYSIZE = 128;
	typedef void _mm_init_fn(void *mem, size_t bytes, char *key, unsigned attrs, void *cookie);
	class MemoryManagerHeader :	public PAMI::Queue,
					public PAMI::Mutex::CounterMutex<PAMI::Counter::GccProcCounter>
	{
	public:
		MemoryManagerHeader() :
		PAMI::Queue(),
		PAMI::Mutex::CounterMutex<PAMI::Counter::GccProcCounter>()
		{
	  		// _meta->_mutex.init(NULL); // should be no-op.
	  		// _meta->_allocations.init(NULL); // should be no-op/redundant
		}
	}; // class MemoryManagerHeader

	class MemoryManagerAlloc : public PAMI::Queue::Element {
	public:
		MemoryManagerAlloc(char *key, size_t align, size_t size) :
		PAMI::Queue::Element(),
		{
			if (key) {
				strcpy(_key, key);
			} else {
				_key[0] = 0;
			}
			_raw_data = (((size_t)this + sizeof(MemoryManagerAlloc) + (align - 1)) & ~(align - 1)) - (size_t)this;
			_raw_size = _raw_data + size;
		}
		inline size_t size() { return _raw_size; }
		inline void *address() { return this + _raw_data; }
		inline bool isMatch(char *key) {
			return (key ? strncmp(key, _key, MMKEYSIZE) == 0 : false);
		}
	private:
		size_t _raw_data;
		size_t _raw_size;
		char _key[MMKEYSIZE];
	}; // class MemoryManagerAlloc

        ///
        /// \brief Empty base memory manager constructor
        ///
        inline MemoryManager ()
        {
          TRACE_ERR((stderr, "%s, this = %p\n", __PRETTY_FUNCTION__, this));
          init (NULL, 0);
        };

        ///
        /// \brief Base memory manager constructor with initial memory buffer
        ///
        /// \param[in] addr  Address of the memory to be managed
        /// \param[in] bytes Number of bytes of memory to manage
        ///
        inline MemoryManager (void * addr, size_t bytes, unsigned attrs = 0)
        {
          TRACE_ERR((stderr, "%s(%p, %zu), this = %p\n", __PRETTY_FUNCTION__,addr,bytes, this));
          init (addr, bytes, attrs);
        };

        ///
        /// \brief Intialize a memory manager with a memory buffer
        ///
        /// \param[in] addr      Address of the memory to be managed
        /// \param[in] bytes     Number of bytes of memory to manage
        /// \param[in] alignment (opt) Default/minimum alignment
        /// \param[in] attrs     (opt) Attributes for memory (addr,bytes)
        ///
        inline void init (void * addr, size_t bytes, size_t alignment = 1, unsigned attrs = 0)
        {
          TRACE_ERR((stderr, "%s(%p, %zu), this = %p\n", __PRETTY_FUNCTION__,addr,bytes, this));
          _base   = (uint8_t *) addr;
          _size   = bytes;
	  _attrs  = attrs;
	  PAMI_assert_debugf(!(alignment & (alignment - 1)), "%zd: alignment must be power of two", alignment);
	  _alignment = alignment;
	  _meta = (MemoryManagerHeader *)_base;
	  _offset = ((((size_t)_base + sizeof(*_meta)) +
				(_alignment - 1)) & ~(_alignment - 1)) -
			(size_t)_base;
	  new (&_meta) MemoryManagerHeader();
          _enabled = true;
        };

        ///
        /// \brief Memory syncronization
        ///
        /// \todo Remove? Why is this needed? The \c msync macros defined in
        ///       Arch.h should be sufficient.
        ///
        void sync()
        {
          static bool perr = false;
          int rc = msync((void*)_base, _size, MS_SYNC);
          if(!perr && rc) {
            perr=true;
            fprintf(stderr,  "MemoryManager::msync failed with %d, errno %d: %s\n", rc, errno, strerror(errno));
          }
        }

        inline void enable () { _enabled = true; }
        inline void disable () { _enabled = false; }
	inline unsigned attrs () { return _attrs; }

        ///
        /// \brief Allocate an aligned buffer of the memory.
        ///
        /// Provides backward compatability
        ///
        /// \param[out] memptr    Pointer to the allocated memory.
        /// \param[in]  alignment Requested buffer alignment - must be a power of 2.
        /// \param[in]  bytes     Number of bytes to allocate.
        ///
        inline pami_result_t memalign (void ** memptr, size_t alignment, size_t bytes)
        {
	  return key_memalign(memptr, alignment, bytes);
        };

        ///
        /// \brief Allocate an aligned buffer of the memory.
        ///
        /// The initializer function is called only once on a given chunk of memory,
	/// by the first caller to allocate with a given key.
        ///
        /// \param[out] memptr    Pointer to the allocated memory.
        /// \param[in]  alignment Requested buffer alignment - must be a power of 2.
        /// \param[in]  bytes     Number of bytes to allocate.
        /// \param[in]  key       (opt) Shared identifier for allocation
        /// \param[in]  init_fn   (opt) Initializer
        /// \param[in]  cookie    (opt) Opaque data for initializer
        ///
        inline pami_result_t key_memalign (void ** memptr, size_t alignment, size_t bytes,
			char *key = NULL,
			_mm_init_fn *init_fn = NULL, void *cookie = NULL)
	{
	  if (key && strlen(key) >= MMKEYSIZE) {
		return PAMI_INVAL;
	  }
	  _meta->acquire();
	  if (key) {
		MemoryManagerAlloc *m = (MemoryManagerAlloc *)_meta->head();
		while (m) {
			if (m->isMatch(key)) { 
				_meta->release();
				*memptr = m->address();
				return PAMI_SUCCESS;
			}
			m = (MemoryManagerAlloc *)m->next();
		}
	  }
	  // need at least enough space for alloc header
	  if (_offset + sizeof(MemoryManagerAlloc) > _size) {
		_meta->release();
		return PAMI_ERROR;
	  }
	  m = new (_base + _offset) MemoryManagerAlloc(key, alignment, bytes);
	  if (_offset + m->size() > _size) {
		_meta->release();
		return PAMI_ERROR;
	  }
	  if (init_fn) {
		init_fn(m->address(), bytes, key, _attrs, cookie);
	  }
	  _meta->push(m);
	  _meta->release();
	  *memptr = m->address();
	  return PAMI_SUCCESS;
	}

        ///
        /// \brief Return the current maximum number of bytes that may be allocated
        ///
        /// \param[in] alignment Optional alignment parameter
        ///
        /// \return    Number of bytes available
        ///
        inline size_t available (size_t alignment = 1)
        {
          TRACE_ERR((stderr, "%s(%zu) _size %zu, _offset %zu, this = %p\n", __PRETTY_FUNCTION__,alignment, _size, _offset, this));
          PAMI_assert(_enabled==true);
          PAMI_assert_debug((alignment & (alignment - 1)) == 0);

          size_t pad = 0;
          if (alignment > 0)
          {
            pad = ((size_t)_base + _offset) & (alignment - 1);
            if (pad > 0)
              pad = (alignment - pad);
          }

          return _size - _offset - pad;
        };

        ///
        /// \brief Return the size of the managed memory buffer
        ///
        /// \return    Number of bytes in the memory buffer
        ///
        inline size_t size ()
        {
          TRACE_ERR((stderr, "%s %zu\n", __PRETTY_FUNCTION__,_size));
          PAMI_assert_debug(_base != NULL);
          return _size;
        };

        ///
        /// \brief Return the base address of the managed memory buffer
        ///
        /// \return    base address of the memory buffer
        ///
        inline void *base ()
        {
          PAMI_assert_debug(_base != NULL);
          return _base;
        };

      protected:

        void * _base;
        size_t _size;
        size_t _offset;
        bool   _enabled;
	unsigned _attrs;
	MemoryManagerHeader *_meta;
	size_t _alignment;
    };
  };
};

#endif // __components_memory_MemoryManager_h__
