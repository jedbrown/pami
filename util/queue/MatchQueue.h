/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file util/queue/MatchQueue.h
 * \brief ???
 */

#ifndef   __util_queue_matchqueue_h__
#define   __util_queue_matchqueue_h__

#include "Queue.h"

#define NUMQS    16

namespace XMI
{
  class MatchQueueElem  : public QueueElem
  {
    protected:
      unsigned               _key;       ///The key to insert and search elements

    public:
      ///Constructor
      MatchQueueElem (unsigned key) : QueueElem (), _key(key)
      {
      }

      ///Get the key
      unsigned    key()
      {
        return _key;
      }
  };


  class MatchQueue
  {
    protected:

      ///The queues to store keys and later search for them
      Queue                    _localQ[NUMQS];

    public:

      MatchQueue ()
      {
      }

      ///\brief Inserts an element at the head of the queue
      void pushHead (MatchQueueElem  * elem)
      {
        unsigned qid = (elem->key()) % NUMQS;

        _localQ[qid].pushHead (elem);
      }

      ///\brief Inserts an element at the head of the queue
      void pushTail (MatchQueueElem  * elem)
      {
        unsigned qid = (elem->key()) % NUMQS;

        _localQ[qid].pushTail (elem);
      }

      ///\brief Deletes the element at the queue
      void deleteElem (MatchQueueElem   * elem)
      {
        unsigned qid = (elem->key()) % NUMQS;

        _localQ[qid].deleteElem (elem);
      }

      ///\brief Find the first the element that has the key
      MatchQueueElem   *findAndDelete (unsigned key)
      {
        unsigned qid = key % NUMQS;
        Queue &queue = _localQ[qid];
        MatchQueueElem *head = (MatchQueueElem *) queue.peekHead();

        while (head != NULL)
          {
            if (head->key() == key)
              {
                ///Element was found in posted queue
                queue.deleteElem (head);
                return  head;
              }

            head =  (MatchQueueElem *) head->next();
          }

        return NULL;
      }
  };  //- MatchQueue
};  //- XMI

#endif // __util_queue_matchqueue_h__
