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
#ifndef __util_queue_MultiQueue_h__
#define __util_queue_MultiQueue_h__

#include <stdio.h>
#include "util/common.h"
#include "util/queue/QueueInterface.h"
#include "util/queue/Queue.h"

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
class MultiQueueElem : public XMI::Interface::QueueElem<MultiQueueElem<numElems> > {
public:
	MultiQueueElem() :
	XMI::Interface::QueueElem<MultiQueueElem<numElems> >() {
		for (int x = 0; x < numElems; ++x) {
		          new (&_elem[x]) QueueElem();
		}
	}

	inline MultiQueueElem<numElems> *prev_impl(int n) { return (MultiQueueElem<numElems> *)_elem[n].prev(); }

	inline MultiQueueElem<numElems> *next_impl(int n) { return (MultiQueueElem<numElems> *)_elem[n].next(); }

	inline void setPrev_impl(MultiQueueElem<numElems> *msg, int n) { _elem[n].setPrev((QueueElem *)msg); }

	inline void setNext_impl(MultiQueueElem<numElems> *msg, int n) { _elem[n].setNext((QueueElem *)msg); }

	inline void set_impl(MultiQueueElem<numElems> *prev, MultiQueueElem<numElems> *next, int n) {
		_elem[n].setPrev((QueueElem *)prev);
		_elem[n].setNext((QueueElem *)next);
	}

protected:
	QueueElem _elem[numElems];
}; // class MultiQueueElem

template<int numElems, int elemNum>
class MultiQueue : public XMI::Interface::Queue<MultiQueue<numElems,elemNum>,MultiQueueElem<numElems> > {
public:
	//////////////////////////////////////////////////////////////////
	/// \brief  Queue constructor.  Initializes the head and tail
	///         pointers to NULL
	//////////////////////////////////////////////////////////////////
	MultiQueue() :
	XMI::Interface::Queue<MultiQueue<numElems,elemNum>,MultiQueueElem<numElems> >() {
		_head = _tail = NULL;
		_size = 0;
	}

	inline void pushTail_impl(MultiQueueElem<numElems> *msg);

	inline void pushHead_impl(MultiQueueElem<numElems> *msg);

	inline MultiQueueElem<numElems> *popHead_impl();

	inline MultiQueueElem<numElems> *popTail_impl();

	inline MultiQueueElem<numElems> *peekHead_impl() { return _head; }

	inline MultiQueueElem<numElems> *peekTail_impl() { return _tail; }

	inline bool isEmpty_impl() { return _head == NULL; }

	inline int size_impl();

	inline void dump_impl(const char *str, int n);

	inline MultiQueueElem<numElems> *nextElem_impl(MultiQueueElem<numElems> *item);

	inline void deleteElem_impl(MultiQueueElem<numElems> *item);

	inline void insertElem_impl(MultiQueueElem<numElems> *item, size_t position);

#ifdef VALIDATE_ON
	inline void validate_impl();
#endif

protected:
	/// \brief  Head Pointer
	MultiQueueElem<numElems> *_head;

	/// \brief  Tail Pointer
	MultiQueueElem<numElems> *_tail;

	/// \brief  Queue Size
	int _size;
}; // class MultiQueue
}; // namespace XMI

template<int numElems, int elemNum>
inline void XMI::MultiQueue<numElems, elemNum>::pushTail_impl(MultiQueueElem<numElems> *msg) {
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
inline void XMI::MultiQueue<numElems, elemNum>::pushHead_impl(MultiQueueElem<numElems> *msg) {
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
inline XMI::MultiQueueElem<numElems> *XMI::MultiQueue<numElems, elemNum>::popHead_impl() {
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
inline XMI::MultiQueueElem<numElems> *XMI::MultiQueue<numElems, elemNum>::popTail_impl() {
	MultiQueueElem<numElems> *p = _tail;
	if (!p) return NULL;
	_tail = p->prev(elemNum);
	if (_tail == NULL) _head = NULL;
	else _tail->setNext(NULL, elemNum);
	p->setPrev(NULL, elemNum);
	_size--;
	return p;
}

#ifdef VALIDATE_ON
template<int numElems, int elemNum>
inline void XMI::MultiQueue<numElems, elemNum>::validate_impl() {
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
inline int XMI::MultiQueue<numElems, elemNum>::size_impl() {
	return _size;
}

template<int numElems, int elemNum>
inline void XMI::MultiQueue<numElems, elemNum>::dump_impl(const char *strg, int n) {
	int s = size_impl();
	if (s) printf ("%s %d: %d elements\n", strg, n, s);
}

template<int numElems, int elemNum>
inline XMI::MultiQueueElem<numElems> *XMI::MultiQueue<numElems, elemNum>::nextElem_impl(XMI::MultiQueueElem<numElems> *item) {
	return item->next(elemNum);
}

template<int numElems, int elemNum>
inline void XMI::MultiQueue<numElems, elemNum>::deleteElem_impl(XMI::MultiQueueElem<numElems> *item) {
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

template<int numElems, int elemNum>
inline void XMI::MultiQueue<numElems, elemNum>::insertElem_impl(MultiQueueElem<numElems> *item, size_t position) {
	if (position == 0) {
		pushHead(item);
		++_size;
		return;
	}
	size_t i;
	MultiQueueElem<numElems> *insert = _head;
	for (i = 1; i < position; ++i) {
		insert = insert->next(elemNum);
	}
	item->setPrev(insert, elemNum);
	item->setNext(insert->next(elemNum), elemNum);
	insert->setNext(item, elemNum);
	++_size;
}

#endif // __util_queue_MultiQueue_h__
