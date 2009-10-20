/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common/lapiunix/PipeWorkQueue.h
 * \brief Implementation of PipeWorkQueue
 */

#ifndef __components_pipeworkqueue_lapiunix_pipeworkqueue_h__
#define __components_pipeworkqueue_lapiunix_pipeworkqueue_h__

#include "Arch.h"
#include "SysDep.h"
#include "util/common.h"
#include "common/PipeWorkQueueInterface.h"

#define XMI_PIPEWORKQUEUE_CLASS XMI::PipeWorkQueue

// undef/define to control workqueues that are optimized for the flat buffer case.
#define OPTIMIZE_FOR_FLAT_WORKQUEUE
#undef USE_FLAT_BUFFER	// (4*1024*1024)

#define ALLOC_SHMEM(memptr, align, size)	\
	((XMI_SYSDEP_CLASS *)_sysdep)->mm.memalign((void **)&memptr, align, size)

/// \todo Fix shmem free so that it doesn't assert
#define FREE_SHMEM(memptr)	\
	//((XMI_SYSDEP_CLASS *)_sysdep)->mm.free(memptr)

#define WAKEUP(vector)		\
	// ((XMI_SYSDEP_CLASS *)_sysdep)->wakeupManager().wakeup(vector)

namespace XMI {

class PipeWorkQueue : public Interface::PipeWorkQueue<XMI::PipeWorkQueue> {
///
/// \brief Work queue implementation of a fixed-size shared memory buffer.
///
/// Actually, _BaseWorkQueue's producers/consumers are totally unused.
/// Avoid any extra construction, and we should consider eliminating the storage.
///
/// NOTE! configure() is a one-time operation. cannot re-configure().
///

	///
	/// \brief Work queue structure in shared memory
	///
	typedef struct workqueue_t {
		union {
			struct {
				volatile size_t producedBytes;
				volatile size_t consumedBytes;
				volatile void *producerWakeVec;
				volatile void *consumerWakeVec;
			} _s;
			xmi_quad_t _pad;
		} _u;
		volatile char buffer[0]; ///< Producer-consumer buffer
	} workqueue_t __attribute__ ((__aligned__(16)));

	typedef struct export_t {
		uint64_t bufPaddr;	// memregion?
		uint64_t hdrPaddr;	// memregion?
		unsigned pmask;
	} export_t;

public:
	PipeWorkQueue() :
	Interface::PipeWorkQueue<XMI::PipeWorkQueue>(),
	_qsize(0),
	_isize(0),
	_pmask(0),
	_buffer(NULL),
	_sharedqueue(NULL) {
	}

#ifdef USE_FLAT_BUFFER
#warning USE_FLAT_BUFFER Requires MemoryManager.cc change to increase shmem pool, also BG_SHAREDMEMPOOLSIZE when run
        // shmem flat buffer... experimental
        inline void configure_impl(void *sysdep, size_t bufsize, size_t bufinit)
        {
          _sysdep = sysdep;
          _qsize = bufsize;
          _isize = bufinit;
          size_t size = sizeof(workqueue_t) + _qsize;
          ALLOC_SHMEM(_sharedqueue, 16, size);
          XMI_assert_debugf(_sharedqueue, "failed to allocate shared memory\n");
          _buffer = &_sharedqueue->buffer[0];
          XMI_assert_debugf((_qsize & (_qsize - 1)) == 0, "workqueue size is not power of two\n");
#ifdef OPTIMIZE_FOR_FLAT_WORKQUEUE
          _pmask = 0; // nil mask
#else /* !OPTIMIZE_FOR_FLAT_WORKQUEUE */
          _pmask = (size_t) - 1; // nil mask
#endif /* !OPTIMIZE_FOR_FLAT_WORKQUEUE */
        }
#endif /* USE_FLAT_BUFFER */
        ///
        /// \brief Configure for Shared Circular Buffer variety.
        ///
        /// Only one consumer and producer are allowed.
        /// Creates a circular buffer of specified size in shared memory.
        /// Buffer size must be power-of-two.
        ///
        /// \param[in] sysdep	System dependent methods
        /// \param[in] bufsize	Size of buffer to allocate
        ///
        inline void configure_impl(void *sysdep, size_t bufsize)
        {
          _sysdep = sysdep;
          _qsize = bufsize;
          size_t size = sizeof(workqueue_t) + _qsize;
          ALLOC_SHMEM(_sharedqueue, 16, size);
          XMI_assert_debugf(_sharedqueue, "failed to allocate shared memory\n");
          _buffer = &_sharedqueue->buffer[0];
          XMI_assert_debugf((_qsize & (_qsize - 1)) == 0, "workqueue size is not power of two\n");
          _pmask = _qsize - 1;
        }

        ///
        /// \brief Configure for User-supplied Circular Buffer variety.
        ///
        /// Only one consumer and producer are allowed.
        /// Uses the provided buffer as a circular buffer of specified size.
        /// Buffer size must be power-of-two.
        /// Assumes the caller has placed buffer and (this) in appropriate memory
        /// for desired use - i.e. all in shared memory if to be used beyond this process.
        ///
        /// \param[in] sysdep   System dependent methods
        /// \param[in] buffer	Buffer to use
        /// \param[in] bufsize	Size of buffer
        ///
        inline void configure_impl(void *sysdep, char *buffer, size_t bufsize)
        {
          _sysdep = sysdep;
          _qsize = bufsize;
          _buffer = buffer;
          _sharedqueue = &this->__sq;
          XMI_assert_debugf((_qsize & (_qsize - 1)) == 0, "workqueue size is not power of two\n");
          _pmask = _qsize - 1;
        }

        ///
        /// \brief Configure for Memeory (flat buffer) variety.
        ///
        /// Only one consumer and producer are allowed. Still supports pipelining.
        /// Sets up a flat buffer of specified maximum size with an arbitrary "initial fill".
        /// Buffer size must be power-of-two.
        /// Assumes the caller has placed buffer and (this) in appropriate memory
        /// for desired use - i.e. all in shared memory if to be used beyond this process.
        ///
        /// \param[in] sysdep   System dependent methods
        /// \param[in] buffer	Buffer to use
        /// \param[in] bufsize	Size of buffer
        /// \param[in] bufinit	Amount of data initially in buffer
        ///
        inline void configure_impl(void *sysdep, char *buffer, size_t bufsize, size_t bufinit)
        {
          _sysdep = sysdep;
          _qsize = bufsize;
          _isize = bufinit;
          _buffer = buffer;
          _sharedqueue = &this->__sq;
#ifdef OPTIMIZE_FOR_FLAT_WORKQUEUE
          _pmask = 0; // nil mask
#else /* !OPTIMIZE_FOR_FLAT_WORKQUEUE */
          _pmask = (size_t) - 1; // nil mask
#endif /* !OPTIMIZE_FOR_FLAT_WORKQUEUE */
        }

	///
	/// \brief PROPOSAL: Configure for Non-Contig Memory (flat buffer) variety.
	///
	/// Only one consumer and producer are allowed. Still supports pipelining.
	/// Sets up a flat buffer of specified maximum size with an arbitrary "initial fill".
	/// Assumes the caller has placed buffer and (this) in appropriate memory
	/// for desired use - i.e. all in shared memory if to be used beyond this process.
	///
	/// This is typically only used for the application buffer, either input or output,
	/// and so would not normally have both producer and consumer (only one or the other).
	/// The interface is the same as for contiguous data except that "bytesAvailable" will
	/// only return the number of *contiguous* bytes available. The user must consume those
	/// bytes before it can see the next contiguous chunk.
	///
	/// \param[out] wq            Opaque memory for PipeWorkQueue
	/// \param[in] buffer         Buffer to use
	/// \param[in] dgsp           Memory layout of a buffer unit
	/// \param[in] dgspcount      Number of repetitions of buffer units
	/// \param[in] dgspinit       Number of units initially in buffer
	///
	inline void configure_impl(void *sysdep, char *buffer, xmi_type_t *dgsp, size_t dgspcount, size_t dgspinit) {
		XMI_abortf("DGSP PipeWorkQueue not yet supported");
	}

	///
	/// \brief Clone constructor.
	///
	/// Used to create a second local memory wrapper object of the same
	/// shared memory resource.
	///
	/// \see WorkQueue(WorkQueue &)
	///
	/// \param[in] obj     Shared work queue object
	///
	PipeWorkQueue(PipeWorkQueue &obj) :
	Interface::PipeWorkQueue<XMI::PipeWorkQueue>(),
	_sysdep(obj._sysdep),
	_qsize(obj._qsize),
	_pmask(obj._pmask),
	_buffer(obj._buffer),
	_sharedqueue(obj._sharedqueue) {
		// need ref count so we know when to free...
		reset();
	}
	///
	/// \brief Virtual destructors make compilers happy.
	///
	inline void operator delete(void * p) { }
	~PipeWorkQueue() {
		// need ref count so we know when to free...
		if (_sharedqueue != &this->__sq) {
			FREE_SHMEM(_sharedqueue);
		}
	}

	///
	/// \brief Reset this shared memory work queue.
	///
	/// Sets the number of bytes produced and the number of bytes
	/// consumed by each consumer to zero.
	///
	inline void reset_impl() {
		_sharedqueue->_u._s.producedBytes = _isize;
		_sharedqueue->_u._s.consumedBytes = 0;
		_sharedqueue->_u._s.producerWakeVec = NULL;
		_sharedqueue->_u._s.consumerWakeVec = NULL;
		mem_sync();
	}

	///
	/// \brief Dump shared memory work queue statistics to stderr.
	///
	/// \param[in] prefix Optional character string to prefix.
	///
	inline void dump_impl(const char *prefix = NULL) {
		mem_sync();
		size_t pbytes0 = _sharedqueue->_u._s.producedBytes;
		size_t cbytes0 = _sharedqueue->_u._s.consumedBytes;

		if (prefix == NULL) {
			prefix = "";
		}

		fprintf(stderr, "%s dump(%p) _sharedqueue = %p, "
			"size = %u, init size = %u, mask = 0x%08x, "
			"wake p=%p c=%p, "
			"produced bytes = %zd (%zd), "
			"consumed bytes = %zd (%zd)\n",
			prefix, this, _sharedqueue,
			_qsize, _isize, _pmask,
			_sharedqueue->_u._s.producerWakeVec, _sharedqueue->_u._s.consumerWakeVec,
			pbytes0, bytesAvailableToProduce(),
			cbytes0, bytesAvailableToConsume());
	}

	///
	/// \brief Export
	///
	/// Produces information about the PipeWorkQueue into the opaque buffer "export".
	/// This info is suitable for sharing with other processes such that those processes
	/// can then construct a PipeWorkQueue which accesses the same data stream.
	///
	/// \param[in] wq             Opaque memory for PipeWorkQueue
	/// \param[out] export        Opaque memory to export into
	/// \return   success of the export operation
	///
	inline xmi_result_t exp_impl(xmi_pipeworkqueue_ext_t *exp) {
		if (unlikely(_pmask)) {
			return XMI_ERROR;
		}
		export_t *e = (export_t *)exp;
		//e->bufPaddr = vtop(_buffer);
		//e->hdrPaddr = vtop(_sharedqueue);
		e->pmask = _pmask;
		return XMI_SUCCESS;
	}

	///
	/// \brief Import
	///
	/// Takes the results of an export of a PipeWorkQueue on a different process and
	/// constructs a new PipeWorkQueue which the local process may use to access the
	/// data stream.
	///
	/// The resulting PipeWorkQueue may consume data, but that is a local-only operation.
	/// The producer has no knowledge of data consumed. There can be only one producer.
	/// There may be multiple consumers, but the producer does not know about them.
	///
	/// TODO: can this work for circular buffers? does it need to, since those are
	/// normally shared memory and thus already permit inter-process communication.
	///
	/// \param[in] import        Opaque memory into which an export was done.
	/// \param[out] wq           Opaque memory for new PipeWorkQueue
	/// \return   success of the import operation
	///
	inline xmi_result_t import_impl(xmi_pipeworkqueue_ext_t *import) {
		// import is not supported for this class
		return XMI_ERROR;
	}

	/// \brief register a wakeup for the consumer side of the PipeWorkQueue
	///
	/// \param[in] vec	Opaque wakeup vector parameter
	///
	inline void setConsumerWakeup_impl(void *vec) {
		_sharedqueue->_u._s.consumerWakeVec = vec;
		// It's possible that bytes have already been produced.
		// So we may need to do a wakeup manually.
		// This also ensures there is one wakeup on a flat buffer WQ.
		if (vec && _sharedqueue->_u._s.producedBytes != 0) {
			WAKEUP(vec);
		}
	}

	/// \brief register a wakeup for the producer side of the PipeWorkQueue
	///
	/// \param[in] vec	Opaque wakeup vector parameter
	///
	inline void setProducerWakeup_impl(void *vec) {
		_sharedqueue->_u._s.producerWakeVec = vec;
	}

	///
	/// \brief Return the maximum number of bytes that can be produced into this work queue.
	///
	/// Bytes must be produced into the memory location returned by bufferToProduce() and then
	/// this work queue \b must be updated with produceBytes().
	///
	/// The number of bytes that may be produced is calculated by determining
	/// the difference between the \b minimum number of bytes consumed by all
	/// consumers and the number of bytes produced.
	///
	/// This must only be called with serious intent to produce! If this
	/// routine returns non-zero, the caller MUST produceBytes() in a timely
	/// manner. Never returns more than one packet.
	///
	/// \see bufferToProduce
	/// \see produceBytes
	///
	/// \return Number of bytes that may be produced.
	///
	inline size_t bytesAvailableToProduce_impl() {
		size_t a = 0;
		size_t p = _sharedqueue->_u._s.producedBytes;
#ifdef OPTIMIZE_FOR_FLAT_WORKQUEUE

          if (likely(!_pmask))
            {
              a = _qsize - p;
            }
          else
            {
#else /* !OPTIMIZE_FOR_FLAT_WORKQUEUE */
          {
#endif /* !OPTIMIZE_FOR_FLAT_WORKQUEUE */
              size_t c = _sharedqueue->_u._s.consumedBytes;
              unsigned po = p & _pmask;
              unsigned co = c & _pmask;

              if (po > co)
                {
                  a = _qsize - po;
                }
              else if (po < co)   // po wrapped... not flat buffer case...
                {
                  a = co - po;
                }
              else if (p == c)
                {
                  a = _qsize - po;
                }
            }
          return a;
        }

        ///
        /// \brief Return the maximum number of bytes that can be consumed from this work queue.
        ///
        /// Consuming from work queues with multiple consumers must specify the consumer id.
        ///
        /// Bytes must be consumed into the memory location returned by bufferToConsume() and then
        /// this work queue \b must be updated with consumeBytes().
        ///
        /// \see bufferToConsume
        /// \see consumeBytes
        ///
        /// \param[in] consumer Consumer id for work queues with multiple consumers
        ///
        /// \return Number of bytes that may be consumed.
        ///
        inline size_t bytesAvailableToConsume_impl()
        {
          size_t a = 0;
          size_t p = _sharedqueue->_u._s.producedBytes;
          size_t c = _sharedqueue->_u._s.consumedBytes;
#ifdef OPTIMIZE_FOR_FLAT_WORKQUEUE

          if (likely(!_pmask))
            {
              a = p - c;
            }
          else
            {
#else /* !OPTIMIZE_FOR_FLAT_WORKQUEUE */
          {
#endif /* !OPTIMIZE_FOR_FLAT_WORKQUEUE */

              //
              // Detect counter wrap around by checking if pbytes is less than
              // cbytes, and then if bumping both by the same value changes that
              // relationship. The condition we're worried about is where the pbytes
              // has wrapped but cbytes has not. If both counters are on the same
              // side of the wrap point, everything works fine. It is an error if
              // consumed bytes passes produced bytes, so we don't look for that
              // condition. If we are spanning the wrap point, we bump both values
              // by some amount in order to get them both on the same side of the wrap.
              // We choose _qsize as the bump amount because the difference between
              // the counters should never exceed this value (in fact, it should never
              // exceed _worksize) and so we can depend on this value always causing
              // cbytes to wrap if pbytes has already wrapped.
              //
              if (p < c && p + _qsize > c + _qsize)
                {
                  p += _qsize;
                  c += _qsize;
                }

              unsigned po = p & _pmask;
              unsigned co = c & _pmask;

              if (po > co)
                {
                  a = po - co;
                }
              else if (po < co)
                {
                  a = _qsize - co;
                }
              else if (p > c)
                {
                  a = _qsize - co;
                }
            }
          return a;
        }

        /// \brief raw accessor for total number of bytes produced since reset()
        ///
        /// \return	number of bytes produced
        ///
        inline size_t getBytesProduced_impl()
        {
          return _sharedqueue->_u._s.producedBytes;
        }

        /// \brief raw accessor for total number of bytes consumed since reset()
        ///
        /// \return	number of bytes consumed
        ///
        inline size_t getBytesConsumed_impl()
        {
          return _sharedqueue->_u._s.consumedBytes;
        }

        /// \brief current position for producing into buffer
        ///
        /// \return	location in buffer to produce into
        ///
        inline char *bufferToProduce_impl()
        {
          char *b;
          size_t p = _sharedqueue->_u._s.producedBytes;
#ifdef OPTIMIZE_FOR_FLAT_WORKQUEUE

          if (likely(!_pmask))
            {
              b = (char *) & _buffer[p];
            }
          else
            {
#else /* !OPTIMIZE_FOR_FLAT_WORKQUEUE */
          {
#endif /* !OPTIMIZE_FOR_FLAT_WORKQUEUE */
              unsigned po = p & _pmask;
              b = (char *) & _buffer[po];
            }
          return b;
        }

        /// \brief notify workqueue that bytes have been produced
        ///
        /// \return	number of bytes that were produced
        ///
        inline void produceBytes_impl(size_t bytes)
        {
          _sharedqueue->_u._s.producedBytes += bytes;
          // cast undoes "volatile"...
          void *v = (void *)_sharedqueue->_u._s.consumerWakeVec;

          if (unlikely((long)v != 0))
            {
              WAKEUP(v);
            }
        }

        /// \brief current position for consuming from buffer
        ///
        /// \return	location in buffer to consume from
        ///
        inline char *bufferToConsume_impl()
        {
          char *b;
          size_t c = _sharedqueue->_u._s.consumedBytes;
#ifdef OPTIMIZE_FOR_FLAT_WORKQUEUE

          if (likely(!_pmask))
            {
              b = (char *) & _buffer[c];
            }
          else
            {
#else /* !OPTIMIZE_FOR_FLAT_WORKQUEUE */
          {
#endif /* !OPTIMIZE_FOR_FLAT_WORKQUEUE */
			unsigned co = c & _pmask;
			b = (char *)&_buffer[co];
		}
		return b;
	}

	/// \brief notify workqueue that bytes have been consumed
	///
	/// \return	number of bytes that were consumed
	///
	inline void consumeBytes_impl(size_t bytes) {
		_sharedqueue->_u._s.consumedBytes += bytes;
		// cast undoes "volatile"...
		void *v = (void *)_sharedqueue->_u._s.producerWakeVec;
		if (unlikely((long)v != 0)) {
			WAKEUP(v);
		}
	}

	/// \brief is workqueue ready for action
	///
	/// \return	boolean indicate workqueue readiness
	///
	inline bool available_impl() {
		return (_sharedqueue != NULL);
	}

	static inline void compile_time_assert () {
		COMPILE_TIME_ASSERT(sizeof(export_t) <= sizeof(xmi_pipeworkqueue_ext_t));
	}

private:
	void *_sysdep;	// possible pointer to system-dependencies (platform specifics)
	unsigned _qsize;
	unsigned _isize;
	unsigned _pmask;
	volatile char *_buffer;
	workqueue_t *_sharedqueue;
	workqueue_t __sq;
}; // class PipeWorkQueue

}; /* namespace XMI */

#endif // __components_pipeworkqueue_lapiunix_pipeworkqueue_h__
