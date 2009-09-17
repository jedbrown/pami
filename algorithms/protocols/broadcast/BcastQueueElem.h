/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/broadcast/BcastQueueElem.h
 * \brief ???
 */

#ifndef  __ccmi_adaptor_broadcast_queue_elem__
#define  __ccmi_adaptor_broadcast_queue_elem__

#include "util/queue/Queue.h"
#include "algorithms/composite/Composite.h"
#include "util/ccmi_util.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {

      class BcastQueueElem : public XMI::MatchQueueElem
      {
      protected:
        //matchq
        unsigned            _bytes;  ///Bytes in the broadcast
        XMI_Callback_t     _cb_done;///Application completion callback

        char              * _rcvbuf;  ///buffer to receive bcast
        char              * _appbuf;  ///App buffer which will be
                                      ///different from rcvbuf for an
                                      ///unexpected bcast

        bool                _isFinished;  ///Bcast completed locally?
        CCMI::Executor::Composite   * _composite;  ///Executor Composite associated with this queue elem

      public:

        ///
        /// \brief Default constructor
        ///
        BcastQueueElem (CCMI::Executor::Composite *c=NULL, unsigned root=-1) : XMI::MatchQueueElem (root),
        _isFinished (false),
        _composite (c)
        {
        }

        void initUnexpMsg (unsigned bytes, char *unexpbuf)
        {
          _bytes    = bytes;
          _rcvbuf   = unexpbuf;

          _appbuf = NULL;  //Will be set by the application later
          _cb_done.function = NULL;
          _cb_done.clientdata = NULL;

          CCMI_assert(bytes > 0);
        }

        void initPostMsg (unsigned bytes, char *rcvbuf, XMI_Callback_t &cb)
        {
          _bytes    = bytes;
          _cb_done  = cb;
          _rcvbuf   = rcvbuf;
          _appbuf   = NULL;
        }

        ///
        /// \brief Call this function when the bcast was received
        ///        unexpected and the application wants to provide the final target
        ///        buffer
        void  setPosted (unsigned bytes, char *buf,
                         XMI_Callback_t &cb_done)
        {
          CCMI_assert(bytes >= _bytes);
          _appbuf = buf;
          _cb_done = cb_done;
        }

        CCMI::Executor::Composite *composite ()
        {
          return _composite;
        }

        XMI_Callback_t  &callback ()
        {
          return _cb_done;
        }

        unsigned bytes ()
        {
          return _bytes;
        }
        unsigned root  ()
        {
          return key();
        }

        char *rcvbuf ()
        {
          return _rcvbuf;
        }

        bool  isFinished ()
        {
          return _isFinished;
        }
        void  setFinished ()
        {
          _isFinished = true;
        }

        bool  isPosted ()
        {
          return(_appbuf != NULL);
        }

        void completeUnexpected ()
        {
          CCMI_assert(isPosted() && isFinished());
          memcpy (_appbuf, _rcvbuf, _bytes);

          if(_cb_done.function)
            _cb_done.function(NULL, _cb_done.clientdata, XMI_SUCCESS);
        }

        void completePosted ()
        {
          if(_cb_done.function)
            _cb_done.function(NULL, _cb_done.clientdata, XMI_SUCCESS);
        }

      } __attribute__((__aligned__(16))); //- BcastQueueElem
    };  //- Broadcast
  };  //- Adaptor
};  //- CCMI

#endif
