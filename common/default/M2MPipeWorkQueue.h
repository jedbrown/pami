/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common/default/M2MPipeWorkQueue.h
 * \brief Implementation of M2MPipeWorkQueue
 */

#ifndef __common_default_M2MPipeWorkQueue_h__
#define __common_default_M2MPipeWorkQueue_h__

#include "Arch.h"
#include "common/PipeWorkQueueInterface.h"
#include "util/common.h"

#undef TRACE_ERR
#define TRACE_ERR(x) //fprintf x


namespace PAMI
{

  class M2MPipeWorkQueue : public Interface::PipeWorkQueue<PAMI::M2MPipeWorkQueue>
  {
///
/// \brief Work queue implementation of a flat many2manyshared indexed memory buffer.
///
/// \note May not both produce and consume on a single M2M PWQ because there is a
/// single "_bytes" member which can not track both operations.   Also, this means
/// that the M2M PWQ may NOT be initialized with partially full byte counts.
/// This was a result of intentionally trying to minimize the state data tracked down
/// to a single size_t per access point.


    public:
      M2MPipeWorkQueue() :
          Interface::PipeWorkQueue<PAMI::M2MPipeWorkQueue>(),
          _buffer(NULL),
          _offsets(NULL),
          _bytes(NULL),
          _dgspCounts(NULL),
          _dgsp(PAMI_BYTE), /// \assume PAMI_BYTE
          _indexCount(0),
          _sizeOfDgsp(1),    /// \assume PAMI_BYTE
          _dgspCount(0),
          _single(false)
      {
      }

      /// \brief Configure for Many to Many (indexed flat buffer) access.
      ///
      /// Only one consumer OR producer at a time is allowed.
      ///
      /// Sets up a flat buffer for indexed access by many to many.
      ///
      /// Each index (offset) in the buffer may be consumed or produced independently.
      ///
      /// Each index specifies maximum size with an arbitrary "initial fill".   An initial
      /// fill of 0 implies a producer-only PWQ.   A FULL initial fill implies a consumer-only
      /// PWQ.  Its the users responsibility to only call consumer or producer
      /// interfaces or unpredictable results may occur.
      ///
      /// The interface is similar to PAMI::PipwWorkQueue's except that an index" will be
      /// required for all consume/produce calls.
      ///
      /// Warning! The PWQ retains and uses the input arrays, it does NOT copy them.  It
      /// may change the contents of these arrays.
      ///
      /// \param[out] unused        REMOVE? Consistent with PAMI::PipeWorkQueue?
      /// \param[in]  buffer        Buffer to use
      /// \param[in]  indexcount    Number of indexed access points to the pwq
      /// \param[in]  dgsp          Memory layout of each buffer unit
      /// \param[in]  offsets       Array[indexcount] of byte offsets for each indexed access point
      /// \param[in]  dgspcounts    Array[indexcount] of data type (dgsp) counts
      /// \param[in]  bufinit       Array[indexcount] of bytes initially in buffer (storage may be modified)
      ///
      /// \note bufinit must be empty (0) for producer PWQ or full (size of dgsp type * dsgpcounts[index]) for consumer PWQ.
      ///
      inline void configure_impl(PAMI::Memory::MemoryManager *unused, char *buffer, size_t indexcount,
                                 pami_type_t *dgsp, size_t *offsets, size_t *dgspcounts, size_t *bufinit)
      {
        /// \todo 'real' dgsp is unimplemented now, so assume PAMI_BYTE
        /// \todo why pami_type_t*? Copied from PAMI::PipeWorkQueue
        PAMI_assert(*dgsp == PAMI_BYTE);
        _sizeOfDgsp = 1;

        _indexCount = indexcount;
        _buffer     = buffer;
        _offsets    = offsets;
        _bytes      = bufinit;
        _dgspCounts = dgspcounts;
        _dgsp       = *dgsp;
        _indexCount = indexcount;

      }
      /// \brief Configure for Many to Many (indexed flat buffer) access.
      ///
      /// Only one consumer OR producer at a time is allowed.
      ///
      /// Sets up a flat buffer for indexed access by many to many.
      ///
      /// Each index (offset) in the buffer may be consumed or produced independently.
      ///
      /// Each index specifies maximum size with an arbitrary "initial fill".   An initial
      /// fill of 0 implies a producer-only PWQ.   A FULL initial fill implies a consumer-only
      /// PWQ.  Its the users responsibility to only call consumer or producer
      /// interfaces or unpredictable results may occur.
      ///
      /// The interface is similar to PAMI::PipwWorkQueue's except that an index" will be
      /// required for all consume/produce calls.
      ///
      /// Warning! The PWQ retains and uses the input arrays, it does NOT copy them.  It
      /// may change the contents of these arrays.
      ///
      /// \param[out] unused        REMOVE? Consistent with PAMI::PipeWorkQueue?
      /// \param[in]  buffer        Buffer to use
      /// \param[in]  indexcount    Number of indexed access points to the pwq (bufinit array only)
      /// \param[in]  dgsp          Memory layout of each buffer unit
      /// \param[in]  offset        Byte offset for each indexed access point
      /// \param[in]  dgspcount     Data type (dgsp) countfor each indexed access point
      /// \param[in]  bufinit       Array[indexcount] of bytes initially in buffer (storage may be modified)
      ///
      /// \note bufinit must be empty (0) for producer PWQ or full (size of dgsp type * dsgpcounts[index]) for consumer PWQ.
      ///
      inline void configure_impl(PAMI::Memory::MemoryManager *unused, char *buffer, size_t indexcount,
                                 pami_type_t *dgsp, size_t offset, size_t dgspcount, size_t *bufinit)
      {
        /// \todo 'real' dgsp is unimplemented now, so assume PAMI_BYTE
        /// \todo why pami_type_t*? Copied from PAMI::PipeWorkQueue
        PAMI_assert(*dgsp == PAMI_BYTE);
        _sizeOfDgsp = 1;

        _indexCount = indexcount;
        _buffer     = buffer;
        _bytes      = bufinit;
        _dgsp       = *dgsp;
        _indexCount = indexcount;
        // Only a single offset and count, point our normal array at this single value.
        _single     = true;
        _offsets    = NULL;
        _dgspCount  = dgspcount;
        _dgspCounts = &_dgspCount;
        TRACE_ERR((stderr,  "<%p>M2MPipeWorkQueue::configure()indexcount %zu, offset %zu, dgspcount %zu, bufinit %zu\n",this, indexcount,  offset,  dgspcount,  bufinit[0]));
        #if 0
        size_t size = _sizeOfDgsp * _dgspCount * _indexCount;
        for(size_t i=0; i < size; ++i)
          {
          TRACE_ERR((stderr,  "<%p>M2MPipeWorkQueue::configure() buffer[%zu]=%u\n",this, i, buffer[i]));
          }
        #endif
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
      M2MPipeWorkQueue(M2MPipeWorkQueue &obj) :
          Interface::PipeWorkQueue<PAMI::M2MPipeWorkQueue>(),
          _buffer(obj._buffer),
          _offsets(obj._offsets),
          _bytes(obj._bytes),
          _dgspCounts(obj._dgspCounts),
          _dgsp(obj._dgsp),
          _indexCount(obj._indexCount),
          _sizeOfDgsp(obj._sizeOfDgsp),
          _dgspCount(obj._dgspCount),
          _single(obj._single)
      {
      }

      ///
      /// \brief Virtual destructors make compilers happy.
      ///
      inline void operator delete(void * p)
      {
      }
      ~M2MPipeWorkQueue()
      {
      }

      ///
      /// \brief Reset this shared memory work queue.
      ///
      /// Sets the number of bytes produced and the number of bytes
      /// consumed by each consumer to zero.
      ///
      inline void reset_impl()
      {
      }

      ///
      /// \brief Dump shared memory work queue statistics to stderr.
      ///
      /// \param[in] prefix Optional character string to prefix.
      ///
      inline void dump_impl(const char *prefix = NULL)
      {

        if (prefix == NULL)
          {
            prefix = "";
          }

        fprintf(stderr, "%s dump(%p)\n",
                prefix, this);
      }

      ///
      /// \brief Return the maximum number of bytes that can be produced into this work queue.
      ///
      /// Bytes must be produced into the memory location returned by bufferToProduce() and then
      /// this work queue \b must be updated with produceBytes().
      ///
      /// \see bufferToProduce
      /// \see produceBytes
      ///
      /// \param[in] index of access point
      ///
      /// \return Number of bytes that may be produced.
      ///
      inline size_t bytesAvailableToProduce_impl(size_t index)
      {
        PAMI_assert(index < _indexCount);
        TRACE_ERR((stderr,  "<%p>M2MPipeWorkQueue::bytesAvailableToProduce()=%zu index %zu, _indexcount %zu, offset %zu, dgspcount %zu, bufinit %zu/%zu\n",this, _dgspCounts[_single?0:index]*_sizeOfDgsp - _bytes[index], index, _indexCount,  _offsets?_offsets[0]:-1U,  _dgspCounts[0],  _bytes[0], _bytes[index]));
        return _dgspCounts[_single?0:index]*_sizeOfDgsp - _bytes[index];
      }

      ///
      /// \brief Return the maximum number of bytes that can be consumed from this work queue.
      ///
      /// Bytes must be consumed into the memory location returned by bufferToConsume() and then
      /// this work queue \b must be updated with consumeBytes().
      ///
      /// \see bufferToConsume
      /// \see consumeBytes
      ///
      /// \param[in] index of access point
      ///
      /// \return Number of bytes that may be consumed.
      ///
      inline size_t bytesAvailableToConsume_impl(size_t index)
      {
        PAMI_assert(index < _indexCount);
        TRACE_ERR((stderr,  "<%p>M2MPipeWorkQueue::bytesAvailableToConsume()=%zu index %zu, _indexcount %zu, offset %zu, dgspcount %zu, bufinit %zu/%zu\n",this, _bytes[index], index, _indexCount,  _offsets?_offsets[0]:-1U,  _dgspCounts[0],  _bytes[0], _bytes[index]));
        return _bytes[index];
      }

      /// \brief raw accessor for total number of bytes produced since reset()
      ///
      /// \param[in] index of access point
      ///
      /// \return number of bytes produced
      ///
      inline size_t getBytesProduced_impl(size_t index)
      {
        PAMI_assert(index < _indexCount);
        TRACE_ERR((stderr,  "<%p>M2MPipeWorkQueue::bytesProduce()=%zu index %zu, _indexcount %zu, offset %zu, dgspcount %zu, bufinit %zu/%zu\n",this, _bytes[index], index, _indexCount,  _offsets?_offsets[0]:-1U,  _dgspCounts[0],  _bytes[0], _bytes[index]));
        return _bytes[index];
      }

      /// \brief raw accessor for total number of bytes consumed since reset()
      ///
      /// \param[in] index of access point
      ///
      /// \return number of bytes consumed
      ///
      inline size_t getBytesConsumed_impl(size_t index)
      {
        PAMI_assert(index < _indexCount);
        TRACE_ERR((stderr,  "<%p>M2MPipeWorkQueue::bytesConsumeed()=%zu index %zu, _indexcount %zu, offset %zu, dgspcount %zu, bufinit %zu/%zu\n",this, _dgspCounts[_single?0:index]*_sizeOfDgsp - _bytes[index], index, _indexCount,  _offsets?_offsets[0]:-1U,  _dgspCounts[0],  _bytes[0], _bytes[index]));
        return _dgspCounts[_single?0:index]*_sizeOfDgsp - _bytes[index];
      }

      /// \brief current position for producing into buffer
      ///
      /// \param[in] index of access point
      ///
      /// \return location in buffer to produce into
      ///
      inline char *bufferToProduce_impl(size_t index)
      {
        PAMI_assert(index < _indexCount);
        char *b;
        /// \todo Need 'real' dgsp support to find the byte offset in the dgsp type.
        /// Assuming PAMI_BYTE now.
        size_t offset = _single?_dgspCounts[0]*_sizeOfDgsp*index:_offsets[index];
        b = (char *) & _buffer[offset+_bytes[index]];
        TRACE_ERR((stderr,  "<%p>M2MPipeWorkQueue::bufferToProduce() <%p>buffer\n",this, b));
        return b;
      }

      /// \brief notify workqueue that bytes have been produced
      ///
      /// \param[in] index of access point
      ///
      /// \return number of bytes that were produced
      ///
      inline void produceBytes_impl(size_t index, size_t bytes)
      {
        PAMI_assert(index < _indexCount);
        _bytes[index] += bytes;
        TRACE_ERR((stderr,  "<%p>M2MPipeWorkQueue::produceBytes()=%zu index %zu, _indexcount %zu, offset %zu, dgspcount %zu, bufinit %zu/%zu\n",this, _bytes[index], index, _indexCount,  _offsets?_offsets[0]:-1U,  _dgspCounts[0],  _bytes[0], _bytes[index]));
      }

      /// \brief current position for consuming from buffer
      ///
      /// \param[in] index of access point
      ///
      /// \return location in buffer to consume from
      ///
      inline char *bufferToConsume_impl(size_t index)
      {
        PAMI_assert(index < _indexCount);
        char *b;
        /// \todo Need 'real' dgsp support to find the byte offset in the dgsp type.
        /// Assuming PAMI_BYTE now.
        size_t offset = _single?_dgspCounts[0]*_sizeOfDgsp*index:_offsets[index];
        TRACE_ERR((stderr,  "<%p>M2MPipeWorkQueue::bufferToConsume() index %zu, single %u, _indexcount %zu, offset %zu, dgspcount %zu, _sizeOfDgsp %zu, _bytes[index] %zu\n",this, index, (unsigned)_single, _indexCount,  offset,  _dgspCounts[_single?0:index],_sizeOfDgsp, _bytes[index]));
        b = (char *) & _buffer[offset+(_dgspCounts[_single?0:index] * _sizeOfDgsp - _bytes[index])];
        TRACE_ERR((stderr,  "<%p>M2MPipeWorkQueue::bufferToConsume() <%p/%p>buffer\n",this, _buffer, b));
        #if 0
        size_t size = bytesAvailableToConsume_impl(index);
        for(size_t i=0; i < size; ++i)
          {
          TRACE_ERR((stderr,  "<%p>M2MPipeWorkQueue::bufferToConsume() index %zu <%p>buffer[%zu]=%u\n",this, index, b+i, i, b[i]));
          }
        #endif
        return b;
      }

      /// \brief notify workqueue that bytes have been consumed
      ///
      /// \param[in] index of access point
      ///
      /// \return number of bytes that were consumed
      ///
      inline void consumeBytes_impl(size_t index, size_t bytes)
      {
        PAMI_assert(index < _indexCount);
        _bytes[index] -= bytes;
        TRACE_ERR((stderr,  "<%p>M2MPipeWorkQueue::consumeBytes()=%zu index %zu, _indexcount %zu, offset %zu, dgspcount %zu, bufinit %zu/%zu\n",this, _bytes[index], index, _indexCount,  _offsets?_offsets[0]:-1U,  _dgspCounts[0],  _bytes[0], _bytes[index]));
      }

      static inline void compile_time_assert ()
      {
//    COMPILE_TIME_ASSERT(sizeof(export_t) <= sizeof(pami_pipeworkqueue_ext_t));
        COMPILE_TIME_ASSERT(sizeof(M2MPipeWorkQueue) <= sizeof(pami_pipeworkqueue_t));
      }

    private:
//      PAMI::Memory::MemoryManager *_mm;
//      unsigned _qsize;
//      unsigned _isize;
//      unsigned _pmask;
//      workqueue_t *_sharedqueue;
//      workqueue_t __sq;

      volatile char *_buffer;     /**< flat buffer */
      size_t        *_offsets;    /**< array of byte offsets to start of each access point */
      size_t        *_bytes;      /**< array of byte counts (available to consume) */
      size_t        *_dgspCounts; /**< array of dgsp count size */
      pami_type_t    _dgsp;       /**< dgsp data type */
      size_t         _indexCount; /**< number of indexed access points to the buffer(size of arrays) */
      size_t         _sizeOfDgsp; /**< byte size of the dgsp data type */
      size_t         _dgspCount;  /**< single dgsp count size */
      bool           _single;       /**< single byte/count pwq */
  }; // class M2MPipeWorkQueue

}; /* namespace PAMI */

#endif // __components_pipeworkqueue_default_pipeworkqueue_h__
