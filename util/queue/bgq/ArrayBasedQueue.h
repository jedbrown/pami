/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
///  \file util/queue/ArrayBasedQueue.h
///  \brief Fast Aarray Based Queue Classes
///
///  These classes implement a base queue element and queues constructed
///  from the queue element.  This can be used to implement
///  - Message Queues
///  - Shared Memory Queues
///  - Circular or linear Queues
///
///  Definitions:
///  - QueueElement:  An item to be inserted into a queue
///  - Queue:         A queue of elements
///
///
#ifndef __util_queue_ArrayBasedQueue_h__
#define __util_queue_ArrayBasedQueue_h__

#include <execinfo.h>
#include <stdio.h>
#include <malloc.h>

#include "components/memory/MemoryManager.h"

#include "util/common.h"
#include "util/queue/QueueInterface.h"
#include "util/queue/QueueIteratorInterface.h"
#include "util/queue/Queue.h"
#include "spi/include/l2/atomic.h"
#include "Global.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x)
#endif

#define DEFAULT_SIZE         2048

#ifndef L1D_CACHE_LINE_SIZE 
#define L1D_CACHE_LINE_SIZE  64
#endif

#define L2_ATOMIC_FULL        0x8000000000000000UL
#define L2_ATOMIC_EMPTY       0x8000000000000000UL

namespace PAMI {
  ///
  /// \brief A fast array based queue based on L2 atomics. All calls
  /// except enqueue are not thread safe and must be called from one
  /// thread. In this algorithm each thread tries to enqueue to the
  /// array based queue via bounded-load-increment and if that fails
  /// it enqueues to the overflow queue. A clear disadvandage of this
  /// method is that once the overflow queue has elements, the
  /// algorithm will not use fast queue till the producers stop
  /// enqueueing.
  ///
  ///  
  template <class T_Mutex>
    class ArrayBasedQueue : public PAMI::Interface::DequeInterface<ArrayBasedQueue<T_Mutex>,
    Queue::Element >,
    public PAMI::Interface::QueueInfoInterface<ArrayBasedQueue<T_Mutex>,
    Queue::Element >,
    public PAMI::Interface::QueueIterator<ArrayBasedQueue<T_Mutex>,
    Queue::Element,
    BasicQueueIterator<ArrayBasedQueue<T_Mutex>, Queue::Element> > {

  protected:

    ///
    ///  \brief Move queue elments from array/overflow queues to
    ///  private queue. This routine must be called from only one
    ///  thread.
    ///
    inline bool  advance() {
      bool newwork = false;
      uint64_t index = 0; 
      
      if ( (index =  L2_AtomicLoadIncrementBounded (&_atomicCounters[0])) != L2_ATOMIC_EMPTY ) {
	mem_sync();
	//fprintf(stderr, "Dequeueing index %ld, counter address %lx\n", index, (uint64_t)&_atomicCounters[0]);
	uint64_t qindex = index & (DEFAULT_SIZE - 1);
	
	volatile Element *element = _queueArray[qindex].element; //Wait till producer updates this
	while (element == NULL) 	  //Wait till producer updates this
	  element = _queueArray[qindex].element; 
	
	//fprintf(stderr, "Found element %ld\n", index);
	
	_privateq.enqueue((Element *)element);
	_queueArray[qindex].element = NULL; //Mark the element as unused
	newwork = true;
	
	//Increment the queue size to permit another enqueue
	L2_AtomicLoadIncrement(&_atomicCounters[2]); 
      }
      
      if (!_overflowq.isEmpty()) {
	_mutex.acquire();
	Queue::Element *head;
	Queue::Element *tail;
	size_t size;
	_overflowq.removeAll(head, tail, size);
	if (head) {
	  _privateq.appendAll (head,  tail,  size);
	  newwork = true;
	}
	_mutex.release();
      }      

      return newwork;
    }

  public:    
    const static bool removeAll_can_race = false;
    typedef Queue::Element  Element;
    typedef BasicQueueIterator<ArrayBasedQueue<T_Mutex>, Element > Iterator;
    
    //Structure designed to force write combining
    struct ArrayBasedQueueElement {
      volatile Element   *element;
    } __attribute__((__aligned__(8))); //__attribute__((__aligned__(L1D_CACHE_LINE_SIZE)));


  public:
    inline ArrayBasedQueue() :  _overflowq(), _privateq(), _mutex()
    {
      _wakeup             = 0;
      _atomicCounters     = NULL;
      _counterAddress[0]  = NULL;
      _counterAddress[1]  = NULL;
      _counterAddress[2]  = NULL;
      _counterAddress[3]  = NULL;
      _queueArray         = NULL;
    }
      
    inline void init(PAMI::Memory::MemoryManager *mm)
      {    
	_mutex.init(mm);
	_overflowq.init(mm);
	_privateq.init(mm);
	
	uint64_t *buffer;
	int rc = 0;
	rc = __global.l2atomicFactory.__procscoped_mm.memalign((void **)&buffer, 
							       L1D_CACHE_LINE_SIZE, L1D_CACHE_LINE_SIZE);
	PAMI_assertf(rc == PAMI_SUCCESS, "Failed to allocate L2 Atomic Counter");
	//Verify counter array is 32-byte aligned

	//printf ("Array Based Queue Initialized, atomics at address %lx\n", (uint64_t)buffer);
	
	_atomicCounters = buffer;
	PAMI_assert( (((uint64_t)_atomicCounters) & 0x3f) == 0 );	

	L2_AtomicStore(&_atomicCounters[0], 0);   //Consumer
	L2_AtomicStore(&_atomicCounters[1], 0);   //Producer
	L2_AtomicStore(&_atomicCounters[2], DEFAULT_SIZE); //Upper bound
	
	uint64_t tid = 0;
	for (tid = 0; tid < 4UL; tid ++)
	  _counterAddress[tid] =  (volatile uint64_t *)
	    (((Kernel_L2AtomicsBaseAddress() +
	       ((((uint64_t) &_atomicCounters[1]) << 5) & ~0xfful)) |
	      (tid << 6)) +
	     (4UL << 3)); //__l2_op_ptr (_atomicCounters, 4 /*bounded increment*/);
	
	for (tid = 0; tid < 4UL; tid ++)
	  _flushAddress[tid] =  (volatile uint64_t *)
	    (((Kernel_L2AtomicsBaseAddress() +
	       ((((uint64_t) &_atomicCounters[3]) << 5) & ~0xfful)) |
	      (tid << 6)) +
	     (4UL << 3)); //__l2_op_ptr (_atomicCounters, 4 /*bounded increment*/);
	
	//PAMI_assertf(sizeof(ArrayBasedQueueElement)==L1D_CACHE_LINE_SIZE, "Error: QueueElement size");

	_queueArray = (ArrayBasedQueueElement *) memalign (L1D_CACHE_LINE_SIZE, 
							       sizeof(ArrayBasedQueueElement) * DEFAULT_SIZE);
	memset (_queueArray, 0, sizeof(ArrayBasedQueueElement) * DEFAULT_SIZE);
	PAMI_assert (_queueArray != NULL);
      }
      
      /// \copydoc PAMI::Interface::QueueInterface::enqueue
    inline void enqueue_impl(Element *element)
      {
	//printf("Calling enqueue\n");
	int tid = Kernel_ProcessorThreadID();
	*_flushAddress[tid] = 0; //Store 0 to the flush address to
	                         //flush all stores. Low overhead
	                         //non-blocking write fence
	//mbar();
	uint64_t index = 0;
	if ( likely (_overflowq.isEmpty() && 
		     ((index = *(_counterAddress[tid])) != L2_ATOMIC_FULL)) )
	  { 
	    uint64_t qindex = index & (DEFAULT_SIZE - 1);
            //printf("Atomic increment of counter %lx returned index %lu\n", (uint64_t)&_atomicCounters[1], index);
	    _queueArray[qindex].element = element;
	    //printf("After enqueue\n");
	    _wakeup = 1;
	    return;
	  }
	
	_mutex.acquire();
	_overflowq.enqueue((Queue::Element *)element);
	_mutex.release();
      }
      
      /// \copydoc PAMI::Interface::QueueInterface::dequeue
    inline Element *dequeue_impl()
      {
	advance();
	return 	_privateq.dequeue();
      }
    
    /// \copydoc PAMI::Interface::QueueInterface::push (implemented as an enqueue)
    inline void push_impl(Element *element)
      {
	enqueue_impl (element);
      }
    
    /// \copydoc PAMI::Interface::QueueInterface::peek
    inline Element *peek_impl()
      {
	advance();
	return _privateq.peek();
      }
    
    /// \copydoc PAMI::Interface::QueueInterface::isEmpty
    inline bool isEmpty_impl()
      {
	advance();
	
	return _privateq.isEmpty();
      }
    
    /// \copydoc PAMI::Interface::QueueInterface::next
    inline Element *next_impl(Element *reference)
      {
	PAMI_abort();
	return NULL;
      }
    
    /// \copydoc PAMI::Interface::QueueInterface::removeAll
    inline void removeAll_impl(Element *&head, Element *&tail, size_t &size)
      {
	PAMI_assertf(0, "removeAll not implemented\n");
      }
    
    /// \copydoc PAMI::Interface::QueueInterface::appendAll
    inline void appendAll_impl(Element *head, Element *tail, size_t size)
      {
	PAMI_assertf(0, "appendAll not implemented\n");
      }
    
#ifdef COMPILE_DEPRECATED_QUEUE_INTERFACES
    /// \copydoc PAMI::Interface::QueueInterface::popTail
    inline Element *popTail_impl()
      {
	PAMI_abort();
	return NULL;
      }
    
    /// \copydoc PAMI::Interface::QueueInterface::peekTail
    inline Element *peekTail_impl()
      {
	PAMI_abort();
	return NULL;
      }
#endif
    
    /// \copydoc PAMI::Interface::DequeInterface::tail
    inline Element *tail_impl()
      {
	PAMI_abort();
	return NULL;
      }
    
    /// \copydoc PAMI::Interface::DequeInterface::before
    inline Element *before_impl(Element *reference)
      {
	PAMI_abort();
	return NULL;
      }
    
    /// \copydoc PAMI::Interface::DequeInterface::insert
    inline void insert_impl(Element *reference,
			    Element *element)
      {
	PAMI_abort();
      }
    
    /// \copydoc PAMI::Interface::DequeInterface::append
    inline void append_impl(Element *reference,
			    Element *element)
      {
	PAMI_abort();
      }
    
    /// \copydoc PAMI::Interface::DequeInterface::remove
    inline void remove_impl(Element *element)
      {
	PAMI_abort();
	return;
      }
    
    /// \copydoc PAMI::Interface::QueueInfoInterface::size
    inline size_t size_impl()
      {
	advance();
	return _privateq.size_impl();
      }
    
    /// \copydoc PAMI::Interface::QueueInfoInterface::dump
    inline void dump_impl(const char *str, int n)
    {
      fprintf(stderr, "%s: size=%ld locked:%d\n", str,
	      size_impl(), _mutex.isLocked());
    }
    
#ifdef VALIDATE_ON
    /// \copydoc PAMI::Interface::QueueInfoInterface::validate
    inline void validate_impl()
      {
	PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
      }
#endif
    
    // Iterator implementation:
    // This all works because there is only one thread removing (the iterator),
    // all others only append new work.
    
    inline void iter_init_impl(Iterator *iter) {
      iter->curr = iter->next = NULL;      
    }
    
    inline bool iter_begin_impl(Iterator *iter) {      
      bool newwork = advance();
      
      iter->curr =  (Element *)_privateq.peek();
      return newwork;
    }
    
    inline bool iter_check_impl(Iterator *iter) {
      if (iter->curr == NULL) {
	// done with this pass...
	return false;
      }
      iter->next = _privateq.nextElem(iter->curr);
      return  true;
    }
    
    inline void iter_end_impl(Iterator *iter) {
      iter->curr = iter->next;
    }
    
    inline Element *iter_current_impl(Iterator *iter) {
      return iter->curr;
    }
    
    inline pami_result_t iter_remove_impl(Iterator *iter) {
      _privateq.remove(iter->curr);
      return PAMI_SUCCESS;
    }
    
  protected:
      
    volatile uint64_t                              * _atomicCounters;
    volatile uint64_t                              * _counterAddress[4];
    ArrayBasedQueueElement                         * _queueArray;
    volatile uint64_t                              * _flushAddress[4];
    Queue                                            _overflowq;
    Queue                                            _privateq;
    T_Mutex                                          _mutex;
    volatile uint64_t                                _wakeup;

  }; // class PAMI::ArrayBasedQueue
}; // namespace PAMI

#endif // __util_queue_queue_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
