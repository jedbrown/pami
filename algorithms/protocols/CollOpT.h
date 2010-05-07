/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/CollOpT.h
 * \brief ???
 */

#ifndef __algorithms_protocols_CollOpT_h__
#define __algorithms_protocols_CollOpT_h__

#include "util/queue/MatchQueue.h"
#include "util/ccmi_util.h"
#include "algorithms/ccmi.h"
#include "algorithms/composite/Composite.h"

#define MAX_NUM_PREALLOCATED        64

namespace CCMI
{
  namespace Adaptor
  {
    class EADescriptor : public PAMI::Queue::Element
    {
    public:

      CollHeaderData cdata;          //copy of the header metadata
      char           *buf;           //pointer to early arrival buffer
      unsigned       bytes;          //size of message in early arrival buffer
      unsigned       flag;           //flags
    };

    enum {EANODATA = 0, EASTARTED, EACOMPLETED};
    enum {LocalPosted = 1, EarlyArrival};
    enum {TS_INPROGRESS = 1};

    template <typename T_xfer, typename T_composite>
      class CollOpT : public PAMI::MatchQueueElem
      {
      protected:

        T_composite                 _composite __attribute__((__aligned__(16)));
        T_xfer                      _xfer;   ///copy of calling parameters
        unsigned                    _ntokens;/// number of tokens required
        unsigned                    _flags;  ///TokenBlocked,EarlyArrival,LocalPosted, OpCompleted
                                             ///ActiveMessage, NonBlockingCC etc.
        void                        *_collfac;///pointer to collective registration/factory
        PAMI::Queue                  _eaq;    ///early arrival queue, should be matchqueue to support multiple early arrivals

      public:

        ///
        /// \brief Default constructor
        ///
        CollOpT(unsigned key) : PAMI::MatchQueueElem (key),
        _composite(),
        _ntokens(0),
        _flags(0),
        _collfac(NULL),
        _eaq()
        {
        }

        void *getFactory()
        {
          return _collfac;
        }

        void setFactory(void *factoryp)
        {
          _collfac = factoryp;
        }

        T_composite *getComposite()
        {
          return &_composite;
        }

        T_xfer *getXfer()
        {
          return &_xfer;
        }

        void setXfer(T_xfer *xfer)
        {
          memcpy(&_xfer, xfer, sizeof(T_xfer));
        }

        void setFlag (unsigned flag)
        {
          _flags |= flag ;
        }

        void unsetFlag (unsigned flag)
        {
          _flags &= (~flag);
        }

        unsigned getFlags ()
        {
          return _flags;
        }

/*
        void enqueueEA(EADescriptor *ea)
        {
          _eaq.pushHead(ea);
        }

*/

        PAMI::Queue *getEAQ()
        {
          return &(_eaq);
        }

        static void common_cleanup()
        {

           PAMI_assert(0);

        }


      }; //- CollOpT


      template <typename T_xfer, typename T_composite>
       class CollOpPoolT : public PAMI::Queue
       {

        public:

          ///
          /// \brief Constructor
          ///
          CollOpPoolT () :
          PAMI::Queue()
          {
          }

          /// \brief free the element of the pool
          ~CollOpPoolT()
          {
             while(! isEmpty())
             {
               delete (popHead());
             }
          }

          ///
          /// \brief Keep a pool of 64 buffers
          ///  to optimize malloc/free overheads
          ///
          CollOpT<T_xfer, T_composite> * allocate (unsigned connid)
          {
            if(! isEmpty())
            {
              CollOpT<T_xfer, T_composite> *co = (CollOpT<T_xfer, T_composite> *) popHead();
              return ( new (co) CollOpT<T_xfer, T_composite>(connid) );
            } else {
              return( new CollOpT<T_xfer, T_composite>(connid) );
            }
          }

          ///
          /// \brief Free the buffer allocated
          /// Enqueue to pool if size is less than
          /// MAX_PREALLOCATED_BYTES
          ///
          void free (CollOpT<T_xfer, T_composite> *co)
          {
            if (size() < MAX_NUM_PREALLOCATED)
              pushHead (co);
            else
              delete co;
          }

        };

    };  //- Adaptor
};  //- CCMI

#endif /* __algorithms_protocols_CollOpT_h__ */
