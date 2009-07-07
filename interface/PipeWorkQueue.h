/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file PipeWorkQueue.h
 * \brief ???
 */

#ifndef __ll_cpp_pipeworkqueue_h__
#define __ll_cpp_pipeworkqueue_h__

#if TARGET==genericmpi
#include "collectives/interface/genericmpi/PipeWorkQueueImpl.h"
#elif TARGET==lapiunix
#include "collectives/interface/lapiunix/PipeWorkQueueImpl.h"
#else
#error "Incorrect Target Specified"
#endif

namespace LL {

///
/// \brief Work queue implementation of a fixed-size shared memory buffer.
///
/// Actually, _BaseWorkQueue's producers/consumers are totally unused.
/// Avoid any extra construction, and we should consider eliminating the storage.
///
/// NOTE! configure() is a one-time operation. cannot re-configure().
///
class PipeWorkQueue : public _PipeWorkQueueImpl {
private:
public:

	PipeWorkQueue() : _PipeWorkQueueImpl() {}

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
	inline void configure(void *sysdep, size_t bufsize);

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
	inline void configure(void *sysdep, char *buffer, size_t bufsize);

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
	inline void configure(void *sysdep, char *buffer, size_t bufsize, size_t bufinit);

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
	PipeWorkQueue(PipeWorkQueue &obj);

	///
	/// \brief Virtual destructors make compilers happy.
	///
	inline void operator delete(void * p);
	~PipeWorkQueue() { }

	///
	/// \brief Reset this shared memory work queue.
	///
	/// Sets the number of bytes produced and the number of bytes
	/// consumed by each consumer to zero.
	///
	inline void reset();

	///
	/// \brief Dump shared memory work queue statistics to stderr.
	///
	/// \param[in] prefix Optional character string to prefix.
	///
	inline void dump(const char *prefix = NULL);

	/// \brief register a wakeup for the consumer side of the PipeWorkQueue
	///
	/// \param[in] vec	Opaque wakeup vector parameter
	///
	inline void setConsumerWakeup(void *vec);

	/// \brief register a wakeup for the producer side of the PipeWorkQueue
	///
	/// \param[in] vec	Opaque wakeup vector parameter
	///
	inline void setProducerWakeup(void *vec);

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
	inline size_t bytesAvailableToProduce();

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
	inline size_t bytesAvailableToConsume();

	/// \brief raw accessor for total number of bytes produced since reset()
	///
	/// \return	number of bytes produced
	///
	inline size_t getBytesProduced();

	/// \brief raw accessor for total number of bytes consumed since reset()
	///
	/// \return	number of bytes consumed
	///
	inline size_t getBytesConsumed();

	/// \brief current position for producing into buffer
	///
	/// \return	location in buffer to produce into
	///
	inline char *bufferToProduce();

	/// \brief notify workqueue that bytes have been produced
	///
	/// \return	number of bytes that were produced
	///
	inline void produceBytes(size_t bytes);

	/// \brief current position for consuming from buffer
	///
	/// \return	location in buffer to consume from
	///
	inline char *bufferToConsume();

	/// \brief notify workqueue that bytes have been consumed
	///
	/// \return	number of bytes that were consumed
	///
	inline void consumeBytes(size_t bytes);

	/// \brief is workqueue ready for action
	///
	/// \return	boolean indicate workqueue readiness
	///
	inline bool available();

	/// \brief is workqueue buffer 16-byte aligned
	///
	/// \return	boolean indicate workqueue buffer alignment
	///
	inline bool aligned();

}; /* class PipeWorkQueue */

}; /* namespace LL */

// This is ugly - but it never has to change...
inline void LL::PipeWorkQueue::configure(void *sysdep, size_t bufsize) { _PipeWorkQueueImpl::configure(sysdep, bufsize); }
inline void LL::PipeWorkQueue::configure(void *sysdep, char *buffer, size_t bufsize) { _PipeWorkQueueImpl::configure(sysdep, buffer, bufsize); }
inline void LL::PipeWorkQueue::configure(void *sysdep, char *buffer, size_t bufsize, size_t bufinit) { _PipeWorkQueueImpl::configure(sysdep, buffer, bufsize, bufinit); }
inline LL::PipeWorkQueue::PipeWorkQueue(LL::PipeWorkQueue &obj) {_PipeWorkQueueImpl::_PipeWorkQueueImpl((_PipeWorkQueueImpl&)obj); }
inline void LL::PipeWorkQueue::reset() { _PipeWorkQueueImpl::reset(); }
inline void LL::PipeWorkQueue::dump(const char *prefix) { _PipeWorkQueueImpl::dump(prefix); }
inline void LL::PipeWorkQueue::setConsumerWakeup(void *vec) { _PipeWorkQueueImpl::setConsumerWakeup(vec); }
inline void LL::PipeWorkQueue::setProducerWakeup(void *vec) { _PipeWorkQueueImpl::setProducerWakeup(vec); }
inline size_t LL::PipeWorkQueue::bytesAvailableToProduce() { return _PipeWorkQueueImpl::bytesAvailableToProduce(); }
inline size_t LL::PipeWorkQueue::bytesAvailableToConsume() { return _PipeWorkQueueImpl::bytesAvailableToConsume(); }
inline size_t LL::PipeWorkQueue::getBytesProduced() { return _PipeWorkQueueImpl::getBytesProduced(); }
inline size_t LL::PipeWorkQueue::getBytesConsumed() { return _PipeWorkQueueImpl::getBytesConsumed(); }
inline char *LL::PipeWorkQueue::bufferToProduce() { return _PipeWorkQueueImpl::bufferToProduce(); }
inline void LL::PipeWorkQueue::produceBytes(size_t bytes) { _PipeWorkQueueImpl::produceBytes(bytes); }
inline char *LL::PipeWorkQueue::bufferToConsume() { return _PipeWorkQueueImpl::bufferToConsume(); }
inline void LL::PipeWorkQueue::consumeBytes(size_t bytes) { _PipeWorkQueueImpl::consumeBytes(bytes); }
inline bool LL::PipeWorkQueue::available() { return _PipeWorkQueueImpl::available(); }
inline bool LL::PipeWorkQueue::aligned() { return _PipeWorkQueueImpl::aligned(); }

#endif /* __ll_cpp_pipeworkqueue_h__ */
