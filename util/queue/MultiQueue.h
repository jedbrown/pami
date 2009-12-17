/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
///  \file util/queue/MultiQueue.h
///  \brief QueueElem and Queue Classes for multi-queue objects
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
#ifndef __util_queue_Queue_h__
#define __util_queue_Queue_h__

#include <stdio.h>
#include "util/common.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x)
#endif

////////////////////////////////////////////////////////////////////////////////
// The following are similar to Queue but support multi-threaded queues in the
// sense that an object may be queued from multiple places at the same time.
// (the object belongs to multiple linked lists at one time)
//
// It is possible that users of the Queue could be replaced by the multiqueue
// classes below.

namespace XMI {

//////////////////////////////////////////////////////////////////////
///  \brief Base Class for Queue
//////////////////////////////////////////////////////////////////////
template<int numElems>
class MultiQueueElem {
public:
	//////////////////////////////////////////////////////////////////
	/// \brief  Queue Element constructor.  Initializes the next
	///         and previous pointers to NULL
	//////////////////////////////////////////////////////////////////
	MultiQueueElem() {
		for (int x = 0; x < numElems; ++x) {
		          new (&_elem[x]) QueueElem();
		}
	}

	//////////////////////////////////////////////////////////////////
	/// \brief  Queue Element destructor.
	//////////////////////////////////////////////////////////////////
	virtual ~MultiQueueElem() { }
	/// NOTE: This is required to make "C" programs link successfully with virtual destructors
	inline void operator delete(void *p) {
		XMI_abort();
	}

	//////////////////////////////////////////////////////////////////
	/// \brief  Get the previous Queue Element
	/// \returns: The previous element
	//////////////////////////////////////////////////////////////////
	MultiQueueElem<numElems> *prev(int n = 0)  const { return (MultiQueueElem<numElems> *)_elem[n].prev(); }

	//////////////////////////////////////////////////////////////////
	/// \brief  Get the next Queue Element
	/// \returns: The next element
	//////////////////////////////////////////////////////////////////
	MultiQueueElem<numElems> *next(int n = 0)  const { return (MultiQueueElem<numElems> *)_elem[n].next(); }

	//////////////////////////////////////////////////////////////////
	/// \brief  Set the previous Queue Element
	/// \param msg: A queue element to set to the previous element
	//////////////////////////////////////////////////////////////////
	void setPrev(MultiQueueElem<numElems> *msg, int n = 0) { _elem[n].setPrev((QueueElem *)msg); }

	//////////////////////////////////////////////////////////////////
	/// \brief  Set the next Queue Element
	/// \param msg: A queue element to set to the next element
	//////////////////////////////////////////////////////////////////
	void setNext(MultiQueueElem<numElems> *msg, int n = 0) { _elem[n].setNext((QueueElem *)msg); }

	//////////////////////////////////////////////////////////////////
	/// \brief  Set the previous and next Queue Elements
	/// \param prev: A queue element to set to the next element
	/// \param next: A queue element to set to the next element
	//////////////////////////////////////////////////////////////////
	void set(MultiQueueElem<numElems> *prev, MultiQueueElem<numElems> *next, int n = 0) {
		_elem[n].setPrev((QueueElem *)prev);
		_elem[n].setNext((QueueElem *)next);
	}

	//////////////////////////////////////////////////////////////////
	/// \brief allocate a new element, assuming storage is available
	/// \param size: ignored
	/// \param addr: address to storage of object to be created
	/// \returns:  The address of the storage used for the object
	//////////////////////////////////////////////////////////////////
	// void *operator new(size_t size, void *addr) { return addr; }
protected:
	QueueElem _elem[numElems];
}; // class MultiQueueElem

template<int numElems, int elemNum>
class MultiQueue {
public:
	//////////////////////////////////////////////////////////////////
	/// \brief  Queue constructor.  Initializes the head and tail
	///         pointers to NULL
	//////////////////////////////////////////////////////////////////
	MultiQueue() { _head = _tail = NULL; _size = 0;}

	//////////////////////////////////////////////////////////////////
	/// \brief Add an element to the tail of the queue
	/// \param msg:  A message to push onto the tail of the queue
	//////////////////////////////////////////////////////////////////
	void pushTail(MultiQueueElem<numElems> *msg);

	//////////////////////////////////////////////////////////////////
	/// \brief Add an element to the head of the queue
	/// \param msg:  A message to push onto the head of the queue
	//////////////////////////////////////////////////////////////////
	void pushHead(MultiQueueElem<numElems> *msg);

	//////////////////////////////////////////////////////////////////
	/// \brief Remove an element from the head of the queue
	/// \returns:  The removed head element of the queue
	//////////////////////////////////////////////////////////////////
	MultiQueueElem<numElems> *popHead();

	//////////////////////////////////////////////////////////////////
	/// \brief Remove an element from the tail of the queue
	/// \returns:  The removed tail element of the queue
	//////////////////////////////////////////////////////////////////
	MultiQueueElem<numElems> *popTail();

	//////////////////////////////////////////////////////////////////
	/// \brief Remove an element from the queue
	/// \returns:
	//////////////////////////////////////////////////////////////////
	void deleteElem(MultiQueueElem<numElems> *item);

	//////////////////////////////////////////////////////////////////
	/// \brief    Access the head element of the queue without removing
	/// \returns: The head element of the queue
	//////////////////////////////////////////////////////////////////
	MultiQueueElem<numElems> *peekHead() const { return _head; }

	//////////////////////////////////////////////////////////////////
	/// \brief   Access the tail element of the queue without removing
	/// \returns:The tail element of the queue
	//////////////////////////////////////////////////////////////////
	MultiQueueElem<numElems> *peekTail() const { return _tail; }

	//////////////////////////////////////////////////////////////////
	/// \brief     Query the queue to see if it's empty.
	/// \returns:  Queue empty status
	//////////////////////////////////////////////////////////////////
	bool isEmpty() const { return _head == NULL; }

	//////////////////////////////////////////////////////////////////
	/// \brief     Query the size of the queue
	/// \returns:  The size of the queue
	/// \todo:     use a private data member to track size
	//////////////////////////////////////////////////////////////////
	int size() const;

	//////////////////////////////////////////////////////////////////
	/// \brief      Dump the queue state
	/// \param str: A string to append to the output
	/// \param n:   An integer to print and append to the output
	/// \returns:   The size of the queue
	//////////////////////////////////////////////////////////////////
	void dump(const char *str, int n) const;

#ifdef VALIDATE_ON
	void validate();
#endif
	//////////////////////////////////////////////////////////////////
	/// \brief allocate a new element, assuming storage is available
	/// \param size: ignored
	/// \param addr: address to storage of object to be created
	/// \returns:  The address of the storage used for the object
	//////////////////////////////////////////////////////////////////
	void *operator new (size_t size, void *addr) { return addr; }

protected:
	//////////////////////////////////////////////////////////////////
	/// \brief  Head Pointer
	//////////////////////////////////////////////////////////////////
	MultiQueueElem<numElems> *_head;

	//////////////////////////////////////////////////////////////////
	/// \brief  Tail Pointer
	//////////////////////////////////////////////////////////////////
	MultiQueueElem<numElems> *_tail;

	//////////////////////////////////////////////////////////////////
	/// \brief  Queue Size
	//////////////////////////////////////////////////////////////////
	int _size;
}; // class MultiQueue
}; // namespace XMI

template<int numElems, int elemNum>
inline void XMI::MultiQueue<numElems, elemNum>::pushTail(MultiQueueElem<numElems> *msg) {
	msg->setNext(NULL, elemNum);
	msg->setPrev(_tail, elemNum);
	if (!_tail) {
		_head = _tail = msg;
	} else {
		_tail->setNext(msg, elemNum);
		_tail = msg;
	}
	_size++;
}

template<int numElems, int elemNum>
inline void XMI::MultiQueue<numElems, elemNum>::pushHead(MultiQueueElem<numElems> *msg) {
	msg->setPrev(NULL, elemNum);
	msg->setNext(_head, elemNum);
	if (!_head) {
	_tail = _head = msg;
	} else {
		_head->setPrev(msg, elemNum);
		_head = msg;
	}
	_size++;
}

template<int numElems, int elemNum>
inline XMI::MultiQueueElem<numElems> *XMI::MultiQueue<numElems, elemNum>::popHead() {
	MultiQueueElem<numElems> *p = _head;
	if (!p) return NULL;
	_head = p->next(elemNum);
	if (_head == NULL) _tail = NULL;
	else _head->setPrev(NULL, elemNum);
	p->setNext(NULL, elemNum);
	_size--;
	return p;
}

template<int numElems, int elemNum>
inline XMI::MultiQueueElem<numElems> *XMI::MultiQueue<numElems, elemNum>::popTail() {
	MultiQueueElem<numElems> *p = _tail;
	if (!p) return NULL;
	_tail = p->prev(elemNum);
	if (_tail == NULL) _head = NULL;
	else _tail->setNext(NULL, elemNum);
	p->setPrev(NULL, elemNum);
	_size--;
	return p;
}

template<int numElems, int elemNum>
inline void XMI::MultiQueue<numElems, elemNum>::deleteElem(MultiQueueElem<numElems> *item) {
	MultiQueueElem<numElems> *prev = (MultiQueueElem<numElems> *)item->prev(elemNum);
	MultiQueueElem<numElems> *next = (MultiQueueElem<numElems> *)item->next(elemNum);

	if(prev != NULL) {
		prev->setNext(next, elemNum);
	} else {
		_head = next;
	}
	if(next != NULL) {
		next->setPrev(prev, elemNum);
	} else {
		_tail = prev;
	}
	_size--;
}

#ifdef VALIDATE_ON
template<int numElems, int elemNum>
inline void XMI::MultiQueue<numElems, elemNum>::validate() {
	MultiQueueElem<numElems> *t = _tail;
	MultiQueueElem<numElems> *h = _head;
	int a = 0, b = 0;
	while ((t || h) && a < 100 && b < 100) {
		if (t) {
			++b;
			t = t->prev(elemNum);
		}
		if (h) {
			++a;
			h = h->next(elemNum);
		}
	}
	if (a != _size || b != _size) {
		static char buf[4096];
		char *s = buf;
		if (a != _size) s += sprintf(s, "size != count(_head): %d %d\n", _size, a);
		if (b != _size) s += sprintf(s, "size != count(_tail): %d %d\n", _size, b);
		s += sprintf(s, "Head: 0x%p -> 0x%p "
				"Tail: 0x%p -> 0x%p\n",
				_head,
				(_head ? _head->next(elemNum) : 0),
				_tail,
				(_tail ? _tail->prev(elemNum) : 0));
		fprintf(stderr, buf);
		XMI_assert(a == _size && b == _size);
	}
}
#endif
template<int numElems, int elemNum>
inline int XMI::MultiQueue<numElems, elemNum>::size() const {
	return _size;
}

template<int numElems, int elemNum>
inline void XMI::MultiQueue<numElems, elemNum>::dump(const char *strg, int n) const {
	int s = size();
	if (s) printf ("%s %d: %d elements\n", strg, n, s);
}

#endif // __util_queue_queue_h__
