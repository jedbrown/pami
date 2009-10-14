/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/workqueue/SharedWorkQueue.h
 * \brief ???
 */

#ifndef __components_devices_workqueue_sharedworkqueue_h__
#define __components_devices_workqueue_sharedworkqueue_h__

#include "components/devices/workqueue/WorkQueue.h"
#include "SysDep.h"

#warning This platform-specific code needs to move somewhere else.
#if defined(__bgp__) and !defined(__bgq__)
#include <bpcore/ppc450_inlines.h>
#define mem_sync()	_bgp_msync()
#define mem_barrier()	_bgp_mbar()
#define LQU(x, ptr, incr) \
asm volatile ("lfpdux %0,%1,%2" : "=f"(x), "+Ob"(ptr) : "r"(incr) : "memory")
#define SQU(x, ptr, incr) \
asm volatile ("stfpdux %2,%0,%1": "+Ob" (ptr) : "r" (incr), "f" (x) : "memory")

#elif defined(__bgq__)

#include <hwi/include/bqc/A2_inlines.h>
#ifdef mem_sync
#warning mem_sync already defined
#else
#define mem_sync ppc_msync
#endif
#define LQU(x, y, z)
#define SQU(x, y, z)

#else

#define mem_sync()
#define mem_barrier()

#endif

//#define WORKSIZE (8*1024)
//#define WORKSIZE (1024)
//#define WORKSIZE (16*1024)
//#define QSIZE (WORKSIZE*32)
//#define QSIZE (WORKSIZE*4)

namespace XMI
{
  namespace Device
  {
    namespace WorkQueue
    {
      ///
      /// \brief Work queue implementation of a fixed-size shared memory buffer.
      ///
      class SharedWorkQueue : public WorkQueue
      {
        public:

          ///
          /// \brief Work queue structure in shared memory
          ///
          typedef struct workqueue_t
          {
            //volatile char buffer[QSIZE]; ///< Producer-consumer buffer
            struct
            {
              volatile size_t bytes;     ///< Number of bytes produced - Only written by each producer!
            } producer[4];	// must preserve 16-byte alignment
            struct
            {
              volatile size_t bytes;     ///< Number of bytes consumed - Only written by each consumer!
            } consumer[4];	// must preserve 16-byte alignment
            volatile char buffer[0]; ///< Producer-consumer buffer
          } workqueue_t __attribute__ ((__aligned__ (16)));

          ///
          /// \brief Default constructor.
          ///
          /// Only one consumer of this shared memory work queue is allowed.
          ///
          /// \param[in] queue Location of the workqueue structure in shared memory.
          ///
          SharedWorkQueue (XMI_SYSDEP_CLASS *sysdep, unsigned workunits = 32, unsigned worksize = 8192) :
            WorkQueue (),
            _qsize (workunits * worksize),
            _worksize (worksize),
            _sharedqueue (NULL)
          {
                size_t size = sizeof(workqueue_t) + _qsize;
                sysdep->mm.memalign((void **)&_sharedqueue, 16, size);
		XMI_assert_debug(_sharedqueue);
		XMI_assert_debug((_qsize & (_qsize - 1)) == 0);
		_qmask = _qsize - 1;
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
          SharedWorkQueue (SharedWorkQueue & obj) :
            WorkQueue (obj),
            _qsize (obj._qsize),
            _qmask (obj._qmask),
            _worksize (obj._worksize),
            _sharedqueue (obj._sharedqueue)
          {}

          ///
          /// \brief Virtual destructors make compilers happy.
          ///
          virtual ~SharedWorkQueue ();

          ///
          /// \brief Reset this shared memory work queue.
          ///
          /// Sets the number of bytes produced and the number of bytes
          /// consumed by each consumer to zero.
          ///
          inline void reset ()
          {
            unsigned i;
            for (i=0; i<_producers; i++)
              _sharedqueue->producer[i].bytes = 0;
            for (i=0; i<_consumers; i++)
              _sharedqueue->consumer[i].bytes = 0;

            mem_sync();
          }

          ///
          /// \brief Dump shared memory work queue statistics to stderr.
          ///
          /// \param[in] prefix Optional character string to prefix.
          ///
          inline void dump (const char * prefix = NULL)
          {
            mem_sync();
            unsigned pbytes0 = _sharedqueue->producer[0].bytes;
            unsigned pbytes1 = _sharedqueue->producer[1].bytes;
            unsigned pbytes2 = _sharedqueue->producer[2].bytes;
            unsigned pbytes3 = _sharedqueue->producer[3].bytes;
            unsigned cbytes0 = _sharedqueue->consumer[0].bytes;
            unsigned cbytes1 = _sharedqueue->consumer[1].bytes;
            unsigned cbytes2 = _sharedqueue->consumer[2].bytes;
            unsigned cbytes3 = _sharedqueue->consumer[3].bytes;

            if (prefix == NULL)
              prefix = "";

            fprintf (stderr, "%s SharedWorkQueue::dump() _sharedqueue = %p, produced bytes = %d %d %d %d (%zd %zd %zd %zd), consumed bytes = %d %d %d %d (%zd %zd %zd %zd)\n", prefix, _sharedqueue, pbytes0, pbytes1, pbytes2, pbytes3, bytesAvailableToProduce(0), bytesAvailableToProduce(1), bytesAvailableToProduce(2), bytesAvailableToProduce(3), cbytes0, cbytes1, cbytes2, cbytes3, bytesAvailableToConsume(0), bytesAvailableToConsume(1), bytesAvailableToConsume(2), bytesAvailableToConsume(3));
          }

          ///
          /// \brief Optimized shared memory copy.
          ///
          /// Best performance is achieved if both \c dst and \c src buffers
          /// are 16-byte aligned and the number of bytes is equal to a complete
          /// work unit.
          ///
          /// \see Q2Q
          ///
          /// \param[in] dst Destination memory buffer
          /// \param[in] src Source memory buffer
          /// \param[in] n   Number of bytes to copy from the source buffer to the destination buffer.
          ///
          static inline void shmemcpy (void * dst, void * src, size_t n)
          {
            bool isaligned = (((((unsigned long) dst) | ((unsigned long) src)) & 0x0f) == 0);
            //fprintf (stderr, "SharedWorkQueue::shmemcpy() dst = %p, src = %p, n = %d, aligned = %d\n", dst, src, n, isaligned);
            //if ((((((unsigned) dst) | ((unsigned) src)) & 0x0f) == 0) && (n == _worksize))
            bool ismultiple256 = !(n & 0x000000ff);
#if defined(__bgp__) and !defined(__bgq__) and !defined(__xlC__) and !defined(__xlc__) 
/// \todo xlC doesn't like LQU/SQU so don't do it for now

            if (isaligned && ismultiple256)
            {
              //fprintf (stderr, "SharedWorkQueue::shmemcpy() do dhummer memcpy\n");
              // src and dst are 16-byte aligned and the length is an entire work unit
              register unsigned i16 = sizeof(xmi_quad_t);
              register xmi_quad_t *d = ((xmi_quad_t *)dst) - 1;
              register xmi_quad_t *s = ((xmi_quad_t *)src) - 1;
              register double r0, r1, r2, r3, r4, r5, r6, r7, r8;

              unsigned i, loop = n >> 8; // --> n/256
              for (i=0; i<loop; i++)
              {
                LQU(r0,s,i16); LQU(r1,s,i16); LQU(r2,s,i16);
                LQU(r3,s,i16); LQU(r4,s,i16); LQU(r5,s,i16);
                SQU(r0,d,i16); SQU(r1,d,i16); SQU(r2,d,i16);
                LQU(r6,s,i16); LQU(r7,s,i16); LQU(r8,s,i16);
                SQU(r3,d,i16); SQU(r4,d,i16); SQU(r5,d,i16);
                LQU(r0,s,i16); LQU(r1,s,i16); LQU(r2,s,i16);
                SQU(r6,d,i16); SQU(r7,d,i16); SQU(r8,d,i16);
                LQU(r3,s,i16); LQU(r4,s,i16); LQU(r5,s,i16);
                SQU(r0,d,i16); SQU(r1,d,i16); SQU(r2,d,i16);
                LQU(r6,s,i16);
                SQU(r3,d,i16); SQU(r4,d,i16); SQU(r5,d,i16);
                SQU(r6,d,i16);
              }
            }
            else
#else
#warning need some sort of optimized bgq memcopy someday
		isaligned = isaligned;
		ismultiple256 = ismultiple256;
#endif
            {
              memcpy (dst, src, n);
            }
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
          /// \see bufferToProduce
          /// \see produceBytes
          ///
          /// \return Number of bytes that may be produced.
          ///
          inline size_t bytesAvailableToProduce (unsigned producer)
          {
            size_t bytes_available = 0;

            if (producer == 0)
            {
              unsigned produced_bytes = _sharedqueue->producer[0].bytes;
              unsigned head_offset = produced_bytes & _qmask;

              unsigned min_consumed_bytes = _sharedqueue->consumer[0].bytes;
              unsigned i;
              for (i=1; i<_consumers; i++)
                min_consumed_bytes = MIN(min_consumed_bytes,_sharedqueue->consumer[i].bytes);
              unsigned tail_offset = min_consumed_bytes & _qmask;

              if (head_offset > tail_offset)
              {
                bytes_available = MIN(_qsize-head_offset,_worksize);
              }
              else if (head_offset < tail_offset)
              {
                bytes_available = MIN(tail_offset-head_offset,_worksize);
              }
              else if (produced_bytes == min_consumed_bytes)
              {
                bytes_available = MIN(_qsize-head_offset,_worksize);
              }
            }
            else
            {
              // producer != 0 implies _producers > 0

              // Find the difference in the number of bytes the "previous"
              // producer has produced and cap at a _worksize.
              unsigned bytes = _sharedqueue->producer[producer-1].bytes -
                               _sharedqueue->producer[producer].bytes;
              bytes_available = MIN(bytes,_worksize);
            }

            return bytes_available;
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
          /// \see bufferToProduce
          /// \see produceBytes
          ///
          /// \return Number of bytes that may be produced.
          ///
          inline size_t bytesAvailableToProduce ()
          {
            return bytesAvailableToProduce (_producer);
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
          inline size_t bytesAvailableToConsume (unsigned consumer)
          {
            unsigned bytes_available = 0;

            unsigned pbytes = _sharedqueue->producer[_producers-1].bytes;
            unsigned cbytes = _sharedqueue->consumer[consumer].bytes;
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
            if (pbytes < cbytes && (pbytes + _qsize) > (cbytes + _qsize))
	    {
	      pbytes += _qsize;
	      cbytes += _qsize;
	    }

            unsigned head_offset = (pbytes) & _qmask;
            unsigned tail_offset = (cbytes) & _qmask;

            if (head_offset > tail_offset)
            {
              bytes_available = MIN(head_offset-tail_offset,_worksize);
            }
            else if (head_offset < tail_offset)
            {
              bytes_available = MIN(_qsize-tail_offset,_worksize);
            }
            else if (pbytes > cbytes)
            {
              bytes_available = MIN(_qsize-tail_offset,_worksize);
            }

            return bytes_available;
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
          inline size_t bytesAvailableToConsume ()
          {
            return bytesAvailableToConsume (_consumer);
          }

          inline size_t getBytesProduced (unsigned producer)
          {
            return _sharedqueue->producer[producer].bytes;
          }

          inline size_t getBytesProduced ()
          {
            return getBytesProduced (_producer);
          }

          inline size_t getBytesConsumed (unsigned consumer)
          {
            return _sharedqueue->consumer[consumer].bytes;
          }

          inline size_t getBytesConsumed ()
          {
            return getBytesConsumed (_consumer);
          }

          inline char * bufferToProduce (unsigned producer)
          {
            unsigned offset = _sharedqueue->producer[producer].bytes & _qmask;
            return (char *) &_sharedqueue->buffer[offset];
          }

          inline char * bufferToProduce ()
          {
            return bufferToProduce (_producer);
          }

          inline void produceBytes (size_t bytes, unsigned producer)
          {
            _sharedqueue->producer[producer].bytes += bytes;
          }

          inline void produceBytes (size_t bytes)
          {
            produceBytes (bytes, _producer);
          }

          inline char * bufferToConsume (unsigned consumer)
          {
            unsigned tail_offset = _sharedqueue->consumer[consumer].bytes & _qmask;
            return (char *) &_sharedqueue->buffer[tail_offset];
          }

          inline char * bufferToConsume ()
          {
            return bufferToConsume (_consumer);
          }

          inline void consumeBytes (size_t bytes, unsigned consumer)
          {
            _sharedqueue->consumer[consumer].bytes += bytes;
          }

          inline void consumeBytes (size_t bytes)
          {
            consumeBytes (bytes, _consumer);
          }

          inline bool available ()
          {
            return (_sharedqueue != NULL);
          }

        protected:

          unsigned      _qsize;
          unsigned      _qmask;
          unsigned      _worksize;
          workqueue_t * _sharedqueue;
      };
    };
  };
};
#endif // __components_devices_workqueue_sharedworkqueue_h__
