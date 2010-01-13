/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
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

/// \todo All these following methods should be protected, and only used
/// from the Queue classes. But that seems to require excessive 'friend'
/// classing - including listing all possible "friendly" user classes here.
//
// protected:
//      //friend class ...too-much...;

	//////////////////////////////////////////////////////////////////
	/// \brief  Get the previous Queue Element
	/// \returns: The previous element
	//////////////////////////////////////////////////////////////////
	inline T_QueueElem *prev(int n = 0);

	//////////////////////////////////////////////////////////////////
	/// \brief  Get the next Queue Element
	/// \returns: The next element
	//////////////////////////////////////////////////////////////////
	inline T_QueueElem *next(int n = 0);

	//////////////////////////////////////////////////////////////////
	/// \brief  Set the previous Queue Element
	/// \param qelem: A queue element to set to the previous element
	//////////////////////////////////////////////////////////////////
	inline void setPrev(T_QueueElem *qelem, int n = 0);

	//////////////////////////////////////////////////////////////////
	/// \brief  Set the next Queue Element
	/// \param qelem: A queue element to set to the next element
	//////////////////////////////////////////////////////////////////
	inline void setNext(T_QueueElem *qelem, int n = 0);

	//////////////////////////////////////////////////////////////////
	/// \brief  Set the previous and next Queue Elements
	/// \param prev: A queue element to set to the next element
	/// \param next: A queue element to set to the next element
	//////////////////////////////////////////////////////////////////
	inline void set(T_QueueElem *prev, T_QueueElem *next, int n = 0);
}; // class QueueElem

template<class T_Queue, class T_QueueElem>
class Queue {
public:
	/// \brief  Queue constructor.  Initializes the head and tail
	///         pointers to NULL
	///
	Queue() { }

	/// \brief Add an element to the tail of the queue
	/// \param[in] qelem  A message to push onto the tail of the queue
	///
	inline void pushTail(T_QueueElem *qelem);

	/// \brief Add an element to the head of the queue
	/// \param[in] qelem  A message to push onto the head of the queue
	///
	inline void pushHead(T_QueueElem *qelem);

	/// \brief Remove an element from the head of the queue
	/// \return	The removed head element of the queue
	///
	inline T_QueueElem *popHead();

	/// \brief Remove an element from the tail of the queue
	/// \returns:	The removed tail element of the queue
	///
	inline T_QueueElem *popTail();

	/// \brief    Access the head element of the queue without removing
	/// \return	The head element of the queue
	///
	inline T_QueueElem *peekHead();

	/// \brief   Access the tail element of the queue without removing
	/// \return	The tail element of the queue
	///
	inline T_QueueElem *peekTail();

	/// \brief     Query the queue to see if it's empty.
	/// \return	Queue empty status
	///
	inline bool isEmpty();

	/// \brief     Query the size of the queue
	/// \return	The size of the queue
	///
	inline int size();

	/// \brief Get next element in queue after this one
	/// \param[in] elem	Element to take 'next' of
	/// \return	The next element in list/queue order
	///
	inline T_QueueElem *nextElem(T_QueueElem *elem);

	/// \brief Remove element from queue
	/// \param[in] elem	Element to be removed
	///
	inline void deleteElem(T_QueueElem *elem);

	/// \brief Insert element into queue at specified position
	/// \param[in] elem	Element to insert
	/// \param[in] position	Position to insert, relative to head (0 = before head)
	///
	inline void insertElem(T_QueueElem *elem, size_t position);

	/// \brief      Dump the queue state
	/// \param[in] str A string to prepend to the output
	/// \param[in] n   An integer to print and append to the 'str'
	///
	inline void dump(const char *str, int n);
#ifdef VALIDATE_ON
	/// \brief Validate queue for errors, inconsistencies, etc.
	///
	/// This is a diagnostics tool.
	///
	inline void validate();
#endif
}; // class Queue

}; // namespace Interface
}; // namespace XMI

template<class T_QueueElem>
inline T_QueueElem *XMI::Interface::QueueElem<T_QueueElem>::prev(int n) {
	return static_cast<T_QueueElem *>(this)->prev_impl(n);
}

template<class T_QueueElem>
inline T_QueueElem *XMI::Interface::QueueElem<T_QueueElem>::next(int n) {
	return static_cast<T_QueueElem *>(this)->next_impl(n);
}

template<class T_QueueElem>
inline void XMI::Interface::QueueElem<T_QueueElem>::setPrev(T_QueueElem *qelem, int n) {
	static_cast<T_QueueElem *>(this)->setPrev_impl(qelem, n);
}

template<class T_QueueElem>
inline void XMI::Interface::QueueElem<T_QueueElem>::setNext(T_QueueElem *qelem, int n) {
	static_cast<T_QueueElem *>(this)->setNext_impl(qelem, n);
}

template<class T_QueueElem>
void XMI::Interface::QueueElem<T_QueueElem>::set(T_QueueElem *prev, T_QueueElem *next, int n) {
	static_cast<T_QueueElem *>(this)->set_impl(prev, next, n);
}

template<class T_Queue, class T_QueueElem>
inline void XMI::Interface::Queue<T_Queue,T_QueueElem>::pushTail(T_QueueElem *qelem) {
	static_cast<T_Queue *>(this)->pushTail_impl(qelem);
}

template<class T_Queue, class T_QueueElem>
inline void XMI::Interface::Queue<T_Queue,T_QueueElem>::pushHead(T_QueueElem *qelem) {
	static_cast<T_Queue *>(this)->pushHead_impl(qelem);
}

template<class T_Queue, class T_QueueElem>
inline T_QueueElem *XMI::Interface::Queue<T_Queue,T_QueueElem>::popHead() {
	return static_cast<T_Queue *>(this)->popHead_impl();
}

template<class T_Queue, class T_QueueElem>
inline T_QueueElem *XMI::Interface::Queue<T_Queue,T_QueueElem>::popTail() {
	return static_cast<T_Queue *>(this)->popTail_impl();
}

template<class T_Queue, class T_QueueElem>
inline T_QueueElem *XMI::Interface::Queue<T_Queue,T_QueueElem>::peekHead() {
	return static_cast<T_Queue *>(this)->peekHead_impl();
}

template<class T_Queue, class T_QueueElem>
inline T_QueueElem *XMI::Interface::Queue<T_Queue,T_QueueElem>::peekTail() {
	return static_cast<T_Queue *>(this)->peekTail_impl();
}

template<class T_Queue, class T_QueueElem>
inline bool XMI::Interface::Queue<T_Queue,T_QueueElem>::isEmpty() {
	return static_cast<T_Queue *>(this)->isEmpty_impl();
}

#ifdef VALIDATE_ON
template<class T_Queue, class T_QueueElem>
inline void XMI::Interface::Queue<T_Queue,T_QueueElem>::validate() {
	static_cast<T_Queue *>(this)->validate_impl();
}
#endif

template<class T_Queue, class T_QueueElem>
inline int XMI::Interface::Queue<T_Queue,T_QueueElem>::size() {
	return static_cast<T_Queue *>(this)->size_impl();
}

template<class T_Queue, class T_QueueElem>
inline void XMI::Interface::Queue<T_Queue,T_QueueElem>::dump(const char *strg, int n) {
	static_cast<T_Queue *>(this)->dump_impl(strg, n);
}

template<class T_Queue, class T_QueueElem>
inline T_QueueElem *XMI::Interface::Queue<T_Queue,T_QueueElem>::nextElem(T_QueueElem *elem) {
	return static_cast<T_Queue *>(this)->nextElem_impl(elem);
}

template<class T_Queue, class T_QueueElem>
inline void XMI::Interface::Queue<T_Queue,T_QueueElem>::deleteElem(T_QueueElem *elem) {
	static_cast<T_Queue *>(this)->deleteElem_impl(elem);
}

template<class T_Queue, class T_QueueElem>
inline void XMI::Interface::Queue<T_Queue,T_QueueElem>::insertElem(T_QueueElem *elem, size_t position) {
	static_cast<T_Queue *>(this)->insertElem_impl(elem, position);
}

#endif // __util_queue_QueueInterface_h__
