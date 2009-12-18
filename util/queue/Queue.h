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
#include "util/queue/QueueInterface.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x)
#endif

namespace XMI
{

  //////////////////////////////////////////////////////////////////////
  ///  \brief Base Class for Queue
  //////////////////////////////////////////////////////////////////////
  class QueueElem : public XMI::Interface::QueueElem<QueueElem>
  {
    public:
      //////////////////////////////////////////////////////////////////
      /// \brief  Queue Element constructor.  Initializes the next
      ///         and previous pointers to NULL
      //////////////////////////////////////////////////////////////////
      QueueElem()
      {
        _prev = _next = NULL;
      }

      QueueElem *prev_impl(int n)
      {
        return _prev;
      }

      QueueElem *next_impl(int n)
      {
        return _next;
      }

      void setPrev_impl(QueueElem *qelem, int n)
      {
        _prev = qelem;
      }

      void setNext_impl(QueueElem *qelem, int n)
      {
        _next = qelem;
      }

      void set_impl(QueueElem *prev, QueueElem *next, int n)
      {
        _prev = prev;
        _next = next;
      }

    protected:
      /// \brief  Previous pointer
      QueueElem *_prev;

      /// \brief  Next pointer
      QueueElem *_next;
  }; // class QueueElem

  class Queue : public XMI::Interface::Queue<Queue,QueueElem>
  {
    public:
      Queue() : XMI::Interface::Queue<Queue,QueueElem>()
      {
        _head = _tail = NULL;
        _size = 0;
      }

      inline void pushTail_impl(QueueElem * qelem);

      inline void pushHead_impl(QueueElem * qelem);

      inline QueueElem *popHead_impl();

      inline QueueElem *popTail_impl();

      inline QueueElem *peekHead_impl()
      {
        return _head;
      }

      inline QueueElem *peekTail_impl()
      {
        return _tail;
      }

      inline bool isEmpty_impl()
      {
        return _head == NULL;
      }

      inline int size_impl();
      inline QueueElem *nextElem_impl(QueueElem *elem);
      inline void deleteElem_impl(QueueElem *elem);
      inline void insertElem_impl(QueueElem *elem, size_t position);

      inline void dump_impl(const char *str, int n);
#ifdef VALIDATE_ON
      inline void validate_impl();
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

inline void XMI::Queue::pushTail_impl(QueueElem *qelem)
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

inline void XMI::Queue::pushHead_impl(QueueElem *qelem)
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

inline XMI::QueueElem *XMI::Queue::popHead_impl()
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

inline XMI::QueueElem * XMI::Queue::popTail_impl()
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
inline void XMI::Queue::validate_impl()
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
inline int XMI::Queue::size_impl()
{
  return _size;
}

inline void XMI::Queue::dump_impl(const char * strg, int n)
{
  int s = size_impl();

  if (s) printf ("%s %d: %d elements\n", strg, n, s);
}

inline XMI::QueueElem *XMI::Queue::nextElem_impl(XMI::QueueElem *elem)
{
	return elem->next();
}

inline void XMI::Queue::deleteElem_impl(QueueElem *elem)
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

inline void XMI::Queue::insertElem_impl(QueueElem *elem, size_t position)
{
  if (position == 0)
  {
    pushHead (elem);
    _size++;
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
