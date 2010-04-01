/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
///  \file util/queue/MutexedQueue.h
///  \brief Mutexed QueueElem and Queue Classes
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
#ifndef __util_queue_MutexedQueue_h__
#define __util_queue_MutexedQueue_h__

#include <stdio.h>

#include "SysDep.h"

#include "util/common.h"
#include "util/queue/QueueInterface.h"
#include "util/queue/QueueIteratorInterface.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x)
#endif

namespace PAMI {

class MutexedQueueElement {
public:
	inline MutexedQueueElement()
	{
		//TRACE_ERR((stderr, "template specialization\n"));
	}

	inline void set(MutexedQueueElement *previous, MutexedQueueElement *next)
	{
		_prev = previous;
		_next = next;
	}

	inline void setPrev(MutexedQueueElement *element)
	{
		_prev = element;
	}

	inline void setNext(MutexedQueueElement *element)
	{
		_next = element;
	}

	inline MutexedQueueElement *prev()
	{
		return _prev;
	}

	inline MutexedQueueElement *next()
	{
		return _next;
	}

protected:

	MutexedQueueElement *_prev;
	MutexedQueueElement *_next;
}; // class MutexedQueueElement

//
// template specialization of queue element
//

template <class T_Queue, class T_Element>
struct MutexedQueueIterator {
	T_Element *curr;
	T_Element *next;
};

template <class T_Mutex>
class MutexedQueue :	public PAMI::Interface::DequeInterface<
				MutexedQueue<T_Mutex>,
				MutexedQueueElement
				>,
			public PAMI::Interface::QueueInfoInterface<
				MutexedQueue<T_Mutex>,
				MutexedQueueElement
				>,
			public PAMI::Interface::QueueIterator<
				MutexedQueue<T_Mutex>, MutexedQueueElement,
				MutexedQueueIterator<MutexedQueue<T_Mutex>, MutexedQueueElement>
				> {
public:

	typedef MutexedQueueElement Element;
	typedef MutexedQueueIterator<MutexedQueue<T_Mutex>, MutexedQueueElement> Iterator;

	inline MutexedQueue() :
	PAMI::Interface::DequeInterface<
			MutexedQueue<T_Mutex>,
			MutexedQueueElement
			>(),
	PAMI::Interface::QueueInfoInterface<
			MutexedQueue<T_Mutex>,
			MutexedQueueElement
			>(),
	PAMI::Interface::QueueIterator<
			MutexedQueue<T_Mutex>, MutexedQueueElement,
			MutexedQueueIterator<MutexedQueue<T_Mutex>, MutexedQueueElement>
			>(),
	_mutex(),
	_head(NULL),
	_tail(NULL),
	_size(0)
	{}

	inline void init(PAMI::Memory::MemoryManager *mm)
	{
		_mutex.init(mm);
	}

	/// \copydoc PAMI::Interface::QueueInterface::enqueue
	inline void enqueue_impl(Element *element)
	{
		TRACE_ERR((stderr, "enqueue(%p)\n", element));

		element->set(_tail, NULL);

		if (!_tail) {
			_head = _tail = element;
		} else {
			_tail->setNext(element);
			_tail = element;
		}

		_size++;
	}

	/// \copydoc PAMI::Interface::QueueInterface::dequeue
	inline Element *dequeue_impl()
	{
		Element *element = _head;

		if (!element) return NULL;

		_head = element->next();

		if (_head == NULL) _tail = NULL;
		else _head->setPrev(NULL);

		// Clear the next/prev pointers in the dequeue'd element
		element->setNext(NULL);

		_size--;

		TRACE_ERR((stderr, "dequeue() => %p\n", element));
		return element;
	}

	/// \copydoc PAMI::Interface::QueueInterface::push
	inline void push_impl(Element *element)
	{
		TRACE_ERR((stderr, "push(%p)\n", element));

		element->set(NULL, _head);

		if (!_head) {
			_tail = _head = element;
		} else {
			_head->setPrev(element);
			_head = element;
		}

		_size++;
	}

	/// \copydoc PAMI::Interface::QueueInterface::peek
	inline Element *peek_impl()
	{
		return _head;
	}

	/// \copydoc PAMI::Interface::QueueInterface::isEmpty
	inline bool isEmpty_impl()
	{
		return (_head == NULL);
	}

	/// \copydoc PAMI::Interface::QueueInterface::next
	inline Element *next_impl(Element *reference)
	{
		return reference->next();
	}

#ifdef COMPILE_DEPRECATED_QUEUE_INTERFACES
	/// \copydoc PAMI::Interface::QueueInterface::popTail
	inline Element *popTail_impl()
	{
		Element *element = _tail;

		if (!element) return NULL;

		_tail = element->prev();

		if (_tail == NULL) _head = NULL;
		else _tail->setNext(NULL);

		element->setPrev(NULL);

		_size--;

		return element;
	}

	/// \copydoc PAMI::Interface::QueueInterface::peekTail
	inline Element *peekTail_impl()
	{
		return _tail;
	}
#endif

	/// \copydoc PAMI::Interface::DequeInterface::tail
	inline Element *tail_impl()
	{
		return _tail;
	}

	/// \copydoc PAMI::Interface::DequeInterface::before
	inline Element *before_impl(Element *reference)
	{
		return reference->prev();
	}

	/// \copydoc PAMI::Interface::DequeInterface::insert
	inline void insert_impl(Element *reference,
		Element *element)
	{
		Element *rprev = reference->prev();
		element->set(rprev, reference);
		rprev->setNext(element);
		reference->setPrev(element);
		_size++;
	}

	/// \copydoc PAMI::Interface::DequeInterface::append
	inline void append_impl(Element *reference,
		Element *element)
	{
		Element *rnext = reference->next();
		element->set(reference, rnext);
		reference->setNext(element);
		rnext->setPrev(element);
		_size++;
	}

	/// \copydoc PAMI::Interface::DequeInterface::remove
	inline void remove_impl(Element *element)
	{
		Element *prev = element->prev();
		Element *next = element->next();

		if (prev != NULL) {
			prev->setNext(next);
		} else {
			_head = next;
		}
		if (next != NULL) {
			next->setPrev(prev);
		} else {
			_tail = prev;
		}
		_size--;

		return;
	}

	/// \copydoc PAMI::Interface::QueueInfoInterface::size
	inline size_t size_impl()
	{
		return _size;
	}

	/// \copydoc PAMI::Interface::QueueInfoInterface::dump
	inline void dump_impl(const char *str, int n)
	{
		PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
	}

#ifdef VALIDATE_ON
	/// \copydoc PAMI::Interface::QueueInfoInterface::validate
	inline void validate_impl()
	{
		PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
	}
#endif

#ifdef COMPILE_DEPRECATED_QUEUE_INTERFACES
	/// \copydoc PAMI::Interface::QueueInfoInterface::insertElem
	inline void insertElem_impl(Element *element, size_t position) {
		if (position == 0) {
			this->push(element);
			_size++;
			return;
		}

		size_t i;
		Element *insert = _head;

		for (i = 1; i < position; i++) {
			insert = insert->next();
		}

		element->set(insert, insert->next());
		insert->setNext(element);
		_size++;

		return;
	}
#endif

// Iterator implementation:
// This all works because there is only one thread removing (the iterator),
// all others only append new work.

	inline void iter_init_impl(Iterator *iter) {
		iter->curr = iter->next = NULL;
	}

	inline bool iter_begin_impl(Iterator *iter) {
		iter->curr = this->peek();
		return false; // did not alter queue
	}

	inline bool iter_check_impl(Iterator *iter) {
		if (iter->curr == NULL) {
			// done with this pass...
			return false;
		}
		iter->next = this->nextElem(iter->curr);
		return true;
	}

	inline void iter_end_impl(Iterator *iter) {
		iter->curr = iter->next;
	}

	inline Element *iter_current_impl(Iterator *iter) {
		return iter->curr;
	}

	inline pami_result_t iter_remove_impl(Iterator *iter) {
		this->remove(iter->curr);
		return PAMI_SUCCESS;
	}

protected:

	T_Mutex _mutex;
	Element *_head;
	Element *_tail;
	size_t _size;

}; // class PAMI::MutexedQueue
}; // namespace PAMI

#endif // __util_queue_queue_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
