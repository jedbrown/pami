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

namespace PAMI
{
  namespace Memory
  {
    static const unsigned int PAMI_MM_PRIVATE   = 1;
    static const unsigned int PAMI_MM_SHARED    = 2;
    // how do platforms add more?
    static const unsigned int PAMI_MM_L2ATOMIC  = 4;
    static const unsigned int PAMI_MM_WACREGION = 8;

    class MemoryManager
    {
      private:
	/// \brief compute the padding needed to provide user with aligned addr
	///
	/// \param[in] base	Start address of raw chunk
	/// \param[in] off	Addition (meta) data needed
	/// \param[in] align	Alignment required (power of two)
	///
	static inline size_t padding(void *base, size_t off, size_t align) {
		return (((size_t)base + off + (align - 1)) & ~(align - 1)) -
			(size_t)base;
	}
      public:
	static const unsigned int MMKEYSIZE = 128;
	typedef void MM_INIT_FN(void *mem, size_t bytes, const char *key, unsigned attrs, void *cookie);

    static void memzero(void *mem, size_t bytes, const char *key, unsigned attrs, void *cookie)
    {
    	memset(mem, 0, bytes);
    }


        ///
        /// \brief Empty base memory manager constructor
        ///
        inline MemoryManager () :
	_base(NULL),
	_size(0),
	_offset(0),
	_enabled(false),
	_attrs(0),
	_meta(NULL),
	_alignment(0)
        {
          TRACE_ERR((stderr, "%s, this = %p\n", __PRETTY_FUNCTION__, this));
        }

        ///
        /// \brief Base memory manager constructor with initial memory buffer
        ///
        /// DEPRECATED?
        ///
        /// \param[in] addr  Address of the memory to be managed
        /// \param[in] bytes Number of bytes of memory to manage
        ///
        inline MemoryManager (void * addr, size_t bytes, unsigned attrs = 0)
        {
          TRACE_ERR((stderr, "%s(%p, %zu), this = %p\n", __PRETTY_FUNCTION__,addr,bytes, this));
		_base = addr;
		_size = bytes;
		_attrs = attrs;
		_alignment = 1;
	  	_meta = (MemoryManagerHeader *)_base;
	  	_offset = padding(_base, sizeof(*_meta), _alignment);
	  	new (_meta) MemoryManagerHeader();
          	_enabled = true;
        }

        ///
        /// \brief Intialize a memory manager from another memory manager
        ///
        /// \param[in] mm        MemoryManager providing buffer
        /// \param[in] bytes     Number of bytes of memory to manage
        /// \param[in] alignment (opt) Default/minimum alignment
        /// \param[in] attrs     (opt) Attributes for memory (addr,bytes)
        ///
        virtual inline pami_result_t init(MemoryManager *mm,
					size_t bytes, size_t alignment = 1,
					unsigned attrs = 0, const char *key = NULL,
					MM_INIT_FN *init_fn = NULL, void *cookie = NULL)
        {
	  	PAMI_assert_debugf(!(alignment & (alignment - 1)), "%zd: alignment must be power of two", alignment);
		if (mm) {
			// make new allocation...
			pami_result_t rc = mm->memalign((void **)&_base, alignment, bytes,
								key, init_fn, cookie);
			if (rc != PAMI_SUCCESS) {
				return rc;
			}
			_attrs = mm->attrs() | attrs;
          		_size = bytes;
	  		_alignment = alignment;
	  		_meta = (MemoryManagerHeader *)_base;
		} else {
			// "reset" - discard arena
			// this isn't really correct for multiple participants
			_meta->acquire();
			// subsequent new() will effectively release lock...
		}
	  	_offset = padding(_base, sizeof(*_meta), _alignment);
	  	new (_meta) MemoryManagerHeader();
          	_enabled = true;
		return PAMI_SUCCESS;
	}

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
	inline void attrs (unsigned attrs) { _attrs |= attrs; }

#ifdef SUPPORT_MM_FREE
	//
	// We keep all allocations on a 16-byte boundary (i.e. (_offset & 0x0f) == 0).
	// The user's buffer will also always be (at least) 16-byte aligned. This
	// allows tucking of meta data immediately before the user buffer:
	//
	// (previous _offset)>+---------------+
	//                    | header (opt)  | (variable length)
	//                    |  + alignment  |
	//                    +---------------+
	//                    |    meta       | (fixed length)
	// returned pointer ->+---------------+
	//                    |               |
	//                    |               |
	//                    |    user       |
	//                    |    buffer     |
	//                    |               |
	//                    |               |
	//                    |               |
	// (new) _offset ---->+---------------+
	//
	// So, by examining 'meta' one can tell where the 'previous _offset' was,
	// and, by proxy, where the 'header' is (if any). This does not get us to
	// a 'free' algorithm, though. Free space will still have to be managed.
	// Probably, some use of 'meta' and 'header' (and 'user buffer' after free)
	// to create a linked list (of sorts) for free space. The MemoryManager
	// header would contain some sort of offset to where the first chunk of
	// freespace is located, and from there the next chunk could be located
	// by reading data out of 'meta', and so on. Code in free() will have to
	// coallesce adjacent freec chunks, etc. Allocation will have to search
	// the free list, rather than just take memory directly off the end.
	//
	// The reason to force a specific, minimal, alignment is so that low
	// order bits in 'meta' could be used to encode extra information
	// (if needed). For example, whether the chunk is private or has a 'key'.
	// also, if 'meta' is something like size_t it may have to be aligned
	// such that loads/stores don't trigger exceptions.
	//
	/// \todo #warning Full support for freeing memory is not supported yet
#endif // SUPPORT_MM_FREE

	/// \brief Class to hold the meta-data portion of an allocation that is shared
	///
	/// Anonymous (private) chunks do not use this object
	///
	/// Can't use util/queue/*Queue.h since they all depend on MemoryManager.
	///
	class MemoryManagerChunk {
	public:
		MemoryManagerChunk(const char *key, size_t align, size_t size)
		{
			_next = NULL;
			strcpy(_key, key);
			_raw_data = staticSize(this, align, size);
			_raw_size = _raw_data + size;
		}

		/// \brief Static method to compute the meta size with alignment
		///
		/// \param[in] thus	Base address of raw chunk
		/// \param[in] align	Alignment (power of two)
		/// \param[in] size	Size of user request
		/// \return	padding needed to hold meta data and align user addr
		///
		static size_t staticSize(void *thus, size_t align, size_t size) {
			return MemoryManager::padding(thus, sizeof(MemoryManagerChunk), align);
		}

		/// \brief Return next chunk
		inline MemoryManagerChunk *next() { return _next; }

		/// \brief Return next chunk
		inline void next(MemoryManagerChunk *m) { _next = m; }

		/// \brief Return total length of raw chunk
		inline size_t size() { return _raw_size; }

		/// \brief Return padding length of raw chunk
		inline size_t padding() { return _raw_data; }

		/// \brief Return address of user buffer in chunk
		inline void *address() { return (uint8_t *)this + _raw_data; }

		/// \brief Determine if chunk is a match for key
		/// \param[in] key	Kay value searching for
		/// \return	boolean indicating match
		inline bool isMatch(const char *key) {
			return (key ? strncmp(key, _key, MMKEYSIZE) == 0 : false);
		}
	private:
		MemoryManagerChunk *_next;
		size_t _raw_data;
		size_t _raw_size;
		char _key[MMKEYSIZE];
	}; // class MemoryManagerChunk

	/// \brief Class to contain the root header of a MemoryManager
	///
	/// Can't use util/queue/*Queue.h or components/atomic/*... since they all
	/// depend on MemoryManager.
	///
	class MemoryManagerHeader {
	public:
		MemoryManagerHeader() :
		_pub_allocs(NULL),
		_priv_allocs(NULL),
		_mutex(0)
		{
		}

		/// \brief Search shared chunks for matching key
		/// \param[in] key	Key to look for
		/// \return Matching chunk object, or NULL if not found
		inline MemoryManagerChunk *find(const char *key) {
			MemoryManagerChunk *m = _pub_allocs;
			while (m) {
				if (m->isMatch(key)) { 
					return m;
				}
				m = m->next();
			}
			return NULL;
		}

		// caller holds lock!
		inline void push(MemoryManagerChunk *m) {
			m->next(_pub_allocs);
			_pub_allocs = m;
		}

		inline void acquire() {
			while (__sync_fetch_and_add(&_mutex, 1) != 0);
		}

		inline void release() {
			_mutex = 0;
		}
	private:
		MemoryManagerChunk *_pub_allocs;
		void *_priv_allocs;
		size_t _mutex;
	}; // class MemoryManagerHeader

        ///
        /// \brief Allocate an aligned buffer of the memory.
        ///
        /// The initializer function is called only once on a given chunk of memory,
	/// by the first caller to allocate with a given key.
        ///
        /// This does NOT support the freeing of memory
        ///
        /// \param[out] memptr    Pointer to the allocated memory.
        /// \param[in]  alignment Requested buffer alignment - must be a power of 2.
        /// \param[in]  bytes     Number of bytes to allocate.
        /// \param[in]  key       (opt) Shared identifier for allocation
        /// \param[in]  init_fn   (opt) Initializer
        /// \param[in]  cookie    (opt) Opaque data for initializer
        ///
        virtual inline pami_result_t memalign (void ** memptr, size_t alignment, size_t bytes,
			const char *key = NULL,
			MM_INIT_FN *init_fn = NULL, void *cookie = NULL)
	{
	  if (key && strlen(key) >= MMKEYSIZE) {
		return PAMI_INVAL;
	  }
	  _meta->acquire();
	  size_t len = bytes;
	  uint8_t *addr = (uint8_t *)_base + _offset;
	  size_t pad;
	  if (key) {
		// "public" (shared) allocation
		MemoryManagerChunk *m = _meta->find(key);
		if (m) {
			_meta->release();
			*memptr = m->address();
			return PAMI_SUCCESS;
		}
		pad = MemoryManagerChunk::staticSize(addr, alignment, bytes);
		len += pad;
	  	if (_offset + len > _size) {
			_meta->release();
			return PAMI_ERROR;
	  	}
	  	m = new (addr) MemoryManagerChunk(key, alignment, bytes);
		PAMI_assert_debugf(pad == m->padding(), "MemoryManagerChunk padding changed");
	  	_meta->push(m);
		addr = (uint8_t *)m->address();
	  } else {
		// private allocation
	  	pad = padding(addr, 0, alignment);
		len += pad;
	  	if (_offset + len > _size) {
			_meta->release();
			return PAMI_ERROR;
	  	}
	  	addr += pad;
	  }
	  if (init_fn) {
		init_fn(addr, bytes, key, _attrs, cookie);
	  }
	  _offset += len;
	  *memptr = addr;
	  _meta->release();
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
        }

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
        }

        ///
        /// \brief Return the base address of the managed memory buffer
        ///
        /// \return    base address of the memory buffer
        ///
        inline void *base ()
        {
          PAMI_assert_debug(_base != NULL);
          return _base;
        }

      protected:

        void * _base;
        size_t _size;
        size_t _offset;
        bool   _enabled;
	unsigned _attrs;
	MemoryManagerHeader *_meta;
	size_t _alignment;
    }; // class MemoryManager
  }; // namespace Memory
}; // namespace PAMI

#endif // __components_memory_MemoryManager_h__
