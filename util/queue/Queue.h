/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
///  \file util/queue/Queue.h
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
#ifndef __util_queue_Queue_h__
#define __util_queue_Queue_h__

#include <stdio.h>
#include "util/common.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x)
#endif

namespace XMI
{
  //////////////////////////////////////////////////////////////////////
  ///  \brief Base Class for Queue
  //////////////////////////////////////////////////////////////////////
  class QueueElem
  {
    public:
      //////////////////////////////////////////////////////////////////
      /// \brief  Queue Element constructor.  Initializes the next
      ///         and previous pointers to NULL
      //////////////////////////////////////////////////////////////////
      QueueElem    ()
      {
        _prev = _next = NULL;
      }

      //////////////////////////////////////////////////////////////////
      /// \brief  Queue Element destructor.
      //////////////////////////////////////////////////////////////////
      virtual ~QueueElem ()
      {
      }
      /// NOTE: This is required to make "C" programs link successfully with virtual destructors
      inline void operator delete(void * p)
      {
        XMI_abort();
      }

      //////////////////////////////////////////////////////////////////
      /// \brief  Get the previous Queue Element
      /// \returns: The previous element
      //////////////////////////////////////////////////////////////////
      QueueElem    * prev()  const
      {
        return _prev;
      }

      //////////////////////////////////////////////////////////////////
      /// \brief  Get the next Queue Element
      /// \returns: The next element
      //////////////////////////////////////////////////////////////////
      QueueElem    * next()  const
      {
        return _next;
      }

      //////////////////////////////////////////////////////////////////
      /// \brief  Set the previous Queue Element
      /// \param qelem: A queue element to set to the previous element
      //////////////////////////////////////////////////////////////////
      void           setPrev (QueueElem * qelem)
      {
        _prev = qelem;
      }

      //////////////////////////////////////////////////////////////////
      /// \brief  Set the next Queue Element
      /// \param qelem: A queue element to set to the next element
      //////////////////////////////////////////////////////////////////
      void           setNext (QueueElem * qelem)
      {
        _next = qelem;
      }

      //////////////////////////////////////////////////////////////////
      /// \brief  Set the previous and next Queue Elements
      /// \param prev: A queue element to set to the next element
      /// \param next: A queue element to set to the next element
      //////////////////////////////////////////////////////////////////
      void           set     (QueueElem * prev, QueueElem * next)
      {
        _prev = prev;
        _next = next;
      }

    protected:
      //////////////////////////////////////////////////////////////////
      /// \brief  Previous pointer
      //////////////////////////////////////////////////////////////////
      QueueElem    * _prev;

      //////////////////////////////////////////////////////////////////
      /// \brief  Next pointer
      //////////////////////////////////////////////////////////////////
      QueueElem    * _next;
  };

  class Queue
  {
    public:
      //////////////////////////////////////////////////////////////////
      /// \brief  Queue constructor.  Initializes the head and tail
      ///         pointers to NULL
      //////////////////////////////////////////////////////////////////
      Queue()
      {
        _head = _tail = NULL;
        _size = 0;
      }

      //////////////////////////////////////////////////////////////////
      /// \brief Add an element to the tail of the queue
      /// \param qelem:  A message to push onto the tail of the queue
      //////////////////////////////////////////////////////////////////
      void pushTail (QueueElem * qelem);

      //////////////////////////////////////////////////////////////////
      /// \brief Add an element to the head of the queue
      /// \param qelem:  A message to push onto the head of the queue
      //////////////////////////////////////////////////////////////////
      void pushHead (QueueElem * qelem);

      //////////////////////////////////////////////////////////////////
      /// \brief Remove an element from the head of the queue
      /// \returns:  The removed head element of the queue
      //////////////////////////////////////////////////////////////////
      QueueElem * popHead();

      //////////////////////////////////////////////////////////////////
      /// \brief Remove an element from the tail of the queue
      /// \returns:  The removed tail element of the queue
      //////////////////////////////////////////////////////////////////
      QueueElem * popTail();

      //////////////////////////////////////////////////////////////////
      /// \brief    Access the head element of the queue without removing
      /// \returns: The head element of the queue
      //////////////////////////////////////////////////////////////////
      QueueElem * peekHead() const
      {
        return _head;
      }

      //////////////////////////////////////////////////////////////////
      /// \brief   Access the tail element of the queue without removing
      /// \returns:The tail element of the queue
      //////////////////////////////////////////////////////////////////
      QueueElem * peekTail() const
      {
        return _tail;
      }

      //////////////////////////////////////////////////////////////////
      /// \brief     Query the queue to see if it's empty.
      /// \returns:  Queue empty status
      //////////////////////////////////////////////////////////////////
      bool isEmpty() const
      {
        return _head == NULL;
      }

      //////////////////////////////////////////////////////////////////
      /// \brief     Query the size of the queue
      /// \returns:  The size of the queue
      /// \todo:     use a private data member to track size
      //////////////////////////////////////////////////////////////////
      int size() const;


      void deleteElem (QueueElem *elem);

      void insertElem (QueueElem *elem, size_t position);

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

    private:
      //////////////////////////////////////////////////////////////////
      /// \brief  Head Pointer
      //////////////////////////////////////////////////////////////////
      QueueElem * _head;

      //////////////////////////////////////////////////////////////////
      /// \brief  Tail Pointer
      //////////////////////////////////////////////////////////////////
      QueueElem * _tail;

      //////////////////////////////////////////////////////////////////
      /// \brief  Queue Size
      //////////////////////////////////////////////////////////////////
      int _size;
  };
};

inline void XMI::Queue::pushTail (QueueElem * qelem)
{
  TRACE_ERR ((stderr, "push tail \n"));

  qelem->setNext(NULL);
  qelem->setPrev(_tail);

  if (!_tail) _head = _tail = qelem;
  else
    {
      _tail->setNext(qelem);
      _tail = qelem;
    }

  _size++;
}

inline void XMI::Queue::pushHead (QueueElem * qelem)
{
  qelem->setPrev(NULL);
  qelem->setNext(_head);

  if (!_head) _tail = _head = qelem;
  else
    {
      _head->setPrev(qelem);
      _head = qelem;
    }

  _size++;
}

inline XMI::QueueElem * XMI::Queue::popHead()
{
  QueueElem * p = _head;

  if (!p) return NULL;

  _head = p->next();

  if (_head == NULL) _tail = NULL;
  else _head->setPrev(NULL);

  p->setNext(NULL);
  _size--;
  return p;
}

inline XMI::QueueElem * XMI::Queue::popTail()
{
  QueueElem * p = _tail;

  if (!p) return NULL;

  _tail = p->prev();

  if (_tail == NULL) _head = NULL;
  else _tail->setNext(NULL);

  p->setPrev(NULL);
  _size--;
  return p;
}
#ifdef VALIDATE_ON
inline void XMI::Queue::validate()
{
  QueueElem *t = _tail;
  QueueElem *h = _head;
  int a = 0, b = 0;

  while ((t || h) && a < 100 && b < 100)
    {
      if (t)
        {
          ++b;
          t = t->prev();
        }

      if (h)
        {
          ++a;
          h = h->next();
        }
    }

  if (a != _size || b != _size)
    {
      static char buf[4096];
      char *s = buf;

      if (a != _size) s += sprintf(s, "size != count(_head): %d %d\n", _size, a);

      if (b != _size) s += sprintf(s, "size != count(_tail): %d %d\n", _size, b);

      s += sprintf(s, "Head: %p -> %p "
                   "Tail: %p -> %p\n",
                   _head,
                   (_head ? _head->next() : 0UL),
                   _tail,
                   (_tail ? _tail->prev() : 0UL));
      fprintf(stderr, buf);
      XMI_assert(a == _size && b == _size);
    }
}
#endif
inline int XMI::Queue::size() const
{
  return _size;
}

inline void XMI::Queue::dump(const char * strg, int n) const
{
  int s = size();

  if (s) printf ("%s %d: %d elements\n", strg, n, s);
}

inline void XMI::Queue::deleteElem (QueueElem *elem)
{
  ///We should check to see if the current element is actually a
  ///member of this queue

  TRACE_ERR ((stderr, "Delete Element \n"));

  XMI_assert (_size > 0);

  if (elem == _head)
    popHead ();
  else if (elem == _tail)
    popTail ();
  else
    {
      XMI_assert (elem->prev() != NULL);
      XMI_assert (elem->next() != NULL);

      elem->prev()->setNext (elem->next());
      elem->next()->setPrev (elem->prev());

      _size --;
    }
}

inline void XMI::Queue::insertElem (QueueElem *elem, size_t position)
{
  if (position == 0)
  {
    pushHead (elem);
    return;
  }

  size_t i;
  QueueElem * insert = _head;
  for (i=1; i<position; i++)
  {
    insert = insert->next();
  }
  elem->setPrev (insert);
  elem->setNext (insert->next());
  insert->setNext (elem);
  _size++;

  return;
}

#endif // __util_queue_queue_h__
