/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
///  \file util/queue/QueueInterface.h
///  \brief QueueElem and Queue Classes
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
#ifndef __util_queue_QueueInterface_h__
#define __util_queue_QueueInterface_h__

namespace XMI {
namespace Interface {

//////////////////////////////////////////////////////////////////////
///  \brief Base Class for Queue
//////////////////////////////////////////////////////////////////////
template<class T_QueueElem>
class QueueElem {
  public:
	//////////////////////////////////////////////////////////////////
	/// \brief  Queue Element constructor.  Initializes the next
	///         and previous pointers to NULL
	//////////////////////////////////////////////////////////////////
	QueueElem() { }

	//////////////////////////////////////////////////////////////////
	/// \brief  Queue Element destructor.
	//////////////////////////////////////////////////////////////////
	virtual ~QueueElem() { }

	//////////////////////////////////////////////////////////////////
	/// \brief  Get the previous Queue Element
	/// \returns: The previous element
	//////////////////////////////////////////////////////////////////
	inline QueueElem *prev();

	//////////////////////////////////////////////////////////////////
	/// \brief  Get the next Queue Element
	/// \returns: The next element
	//////////////////////////////////////////////////////////////////
	inline QueueElem *next();

	//////////////////////////////////////////////////////////////////
	/// \brief  Set the previous Queue Element
	/// \param qelem: A queue element to set to the previous element
	//////////////////////////////////////////////////////////////////
	inline void setPrev(QueueElem *qelem);

	//////////////////////////////////////////////////////////////////
	/// \brief  Set the next Queue Element
	/// \param qelem: A queue element to set to the next element
	//////////////////////////////////////////////////////////////////
	inline void setNext(QueueElem *qelem);

	//////////////////////////////////////////////////////////////////
	/// \brief  Set the previous and next Queue Elements
	/// \param prev: A queue element to set to the next element
	/// \param next: A queue element to set to the next element
	//////////////////////////////////////////////////////////////////
	inline void set(QueueElem *prev, QueueElem *next);

protected:
	//////////////////////////////////////////////////////////////////
	/// \brief  Previous pointer
	//////////////////////////////////////////////////////////////////
	QueueElem    *_prev;

	//////////////////////////////////////////////////////////////////
	/// \brief  Next pointer
	//////////////////////////////////////////////////////////////////
	QueueElem    *_next;
};

template<class T_Queue, T_QueueElem>
class Queue {
public:
	//////////////////////////////////////////////////////////////////
	/// \brief  Queue constructor.  Initializes the head and tail
	///         pointers to NULL
	//////////////////////////////////////////////////////////////////
	Queue() { }

	//////////////////////////////////////////////////////////////////
	/// \brief Add an element to the tail of the queue
	/// \param qelem:  A message to push onto the tail of the queue
	//////////////////////////////////////////////////////////////////
	inline void pushTail(T_QueueElem *qelem);

	//////////////////////////////////////////////////////////////////
	/// \brief Add an element to the head of the queue
	/// \param qelem:  A message to push onto the head of the queue
	//////////////////////////////////////////////////////////////////
	inline void pushHead(T_QueueElem *qelem);

	//////////////////////////////////////////////////////////////////
	/// \brief Remove an element from the head of the queue
	/// \returns:  The removed head element of the queue
	//////////////////////////////////////////////////////////////////
	inline T_QueueElem *popHead();

	//////////////////////////////////////////////////////////////////
	/// \brief Remove an element from the tail of the queue
	/// \returns:  The removed tail element of the queue
	//////////////////////////////////////////////////////////////////
	inline T_QueueElem *popTail();

	//////////////////////////////////////////////////////////////////
	/// \brief    Access the head element of the queue without removing
	/// \returns: The head element of the queue
	//////////////////////////////////////////////////////////////////
	inline T_QueueElem *peekHead();

	//////////////////////////////////////////////////////////////////
	/// \brief   Access the tail element of the queue without removing
	/// \returns:The tail element of the queue
	//////////////////////////////////////////////////////////////////
	inline T_QueueElem *peekTail();

	//////////////////////////////////////////////////////////////////
	/// \brief     Query the queue to see if it's empty.
	/// \returns:  Queue empty status
	//////////////////////////////////////////////////////////////////
	inline bool isEmpty();

	//////////////////////////////////////////////////////////////////
	/// \brief     Query the size of the queue
	/// \returns:  The size of the queue
	/// \todo:     use a private data member to track size
	//////////////////////////////////////////////////////////////////
	inline int size();

	inline void deleteElem (T_QueueElem *elem);

	inline void insertElem (T_QueueElem *elem, size_t position);

	//////////////////////////////////////////////////////////////////
	/// \brief      Dump the queue state
	/// \param str: A string to append to the output
	/// \param n:   An integer to print and append to the output
	/// \returns:   The size of the queue
	//////////////////////////////////////////////////////////////////
	inline void dump(const char *str, int n);
#ifdef VALIDATE_ON
	inline void validate();
#endif

private:
	//////////////////////////////////////////////////////////////////
	/// \brief  Head Pointer
	//////////////////////////////////////////////////////////////////
	T_QueueElem *_head;

	//////////////////////////////////////////////////////////////////
	/// \brief  Tail Pointer
	//////////////////////////////////////////////////////////////////
	T_QueueElem *_tail;

	//////////////////////////////////////////////////////////////////
	/// \brief  Queue Size
	//////////////////////////////////////////////////////////////////
	int _size;
}; // class Queue

}; // namespace Interface
}; // namespace XMI

template<class T_QueueElem>
inline QueueElem *prev() {
	return static_cast<T_QueueElem *>(this)->prev_impl();
}

template<class T_QueueElem>
inline QueueElem *next() {
	return static_cast<T_QueueElem *>(this)->next_impl();
}

template<class T_QueueElem>
inline void setPrev(QueueElem *qelem) {
	static_cast<T_QueueElem *>(this)->setPrev_impl(qelem);
}

template<class T_QueueElem>
inline void setNext(QueueElem *qelem) {
	static_cast<T_QueueElem *>(this)->setNext_impl(qelem);
}

template<class T_QueueElem>
void set(QueueElem *prev, QueueElem *next) {
	static_cast<T_QueueElem *>(this)->set_impl(prev, next);
}

template<class T_Queue, class T_QueueElem>
inline void XMI::Interface::Queue::pushTail(QueueElem *qelem) {
	static_cast<T_Queue,T_QueueElem *>(this)->pushTail_impl(qelem);
}

template<class T_Queue, class T_QueueElem>
inline void XMI::Interface::Queue::pushHead(QueueElem *qelem) {
	static_cast<T_Queue,T_QueueElem *>(this)->pushHead_impl(qelem);
}

template<class T_Queue, class T_QueueElem>
inline XMI::Interface::QueueElem *XMI::Interface::Queue::popHead() {
	return static_cast<T_Queue,T_QueueElem *>(this)->popHead_impl();
}

template<class T_Queue, class T_QueueElem>
inline XMI::Interface::QueueElem *XMI::Interface::Queue::popTail() {
	return static_cast<T_Queue,T_QueueElem *>(this)->popTail_impl();
}

#ifdef VALIDATE_ON
template<class T_Queue, class T_QueueElem>
inline void XMI::Interface::Queue::validate() {
	static_cast<T_Queue,T_QueueElem *>(this)->validate_impl();
}
#endif

template<class T_Queue, class T_QueueElem>
inline int XMI::Interface::Queue::size() {
	return static_cast<T_Queue,T_QueueElem *>(this)->size_impl();
}

template<class T_Queue, class T_QueueElem>
inline void XMI::Interface::Queue::dump(const char *strg, int n) {
	static_cast<T_Queue,T_QueueElem *>(this)->dump_impl(s, n);
}

template<class T_Queue, class T_QueueElem>
inline void XMI::Interface::Queue::deleteElem(QueueElem *elem) {
	static_cast<T_Queue,T_QueueElem *>(this)->deleteElem_impl(elem);
}

template<class T_Queue, class T_QueueElem>
inline void XMI::Interface::Queue::insertElem(QueueElem *elem, size_t position) {
	static_cast<T_Queue,T_QueueElem *>(this)->insertElem_impl(elem, position);
}

#endif // __util_queue_QueueInterface_h__
