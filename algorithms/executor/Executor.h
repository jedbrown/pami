/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/executor/Executor.h
 * \brief ???
 */
#ifndef         __executor_h__
#define         __executor_h__

#include "algorithms/ccmi.h"
#include "util/ccmi_util.h"
#include "util/ccmi_debug.h"

namespace CCMI
{
  namespace Executor
  {

    /**
     * \brief Base Class for all Executors
     **/

    class Executor
    {

    public:

      /**
       * \brief  Constructor
       */
      Executor ()
      {
        _cb_done      =  NULL;
        _clientdata   =  NULL;
        _consistency  =  XMI_UNDEFINED_CONSISTENCY;
      }

      /**
       * \brief  Destructor
       */
      virtual ~Executor () 
      {
//           _cb_done      =  NULL;
//           _clientdata   =  NULL;
//           _consistency  =  CCMI_UNDEFINED_CONSISTENCY;
      }

      /// NOTE: This is required to make "C" programs link successfully with virtual destructors
      inline void operator delete(void * p)
      {
        CCMI_abort();
      }

      /**
       * \brief Start method which is called when the collective
       *  operation cal be started
       */

      virtual void start () = 0;


      /**
       * \brief notification called when the send/multisend
       * operation has finished
       */

      virtual void notifySendDone ( const xmi_quad_t &info ) = 0;


      /**
       * \brief notify when a message has been recived
       * \param src : source of the message
       * \param buf : address of the incoming message
       * \param bytes : number of bytes received
       */

      virtual void notifyRecv (unsigned src, const xmi_quad_t &info, char * buf, unsigned bytes) = 0;

      void setDoneCallback (xmi_event_function cb_done, void *cd)
      {
        _cb_done    =   cb_done;
        _clientdata =   cd;
      }

      void setConsistency  (xmi_consistency_t consistency)
      {
        _consistency = consistency;
      }

      xmi_consistency_t getConsistency  ()
      {
        return _consistency;
      }

    protected:
      ///
      ///  \brief Callback to call when the barrier has finished
      ///
      xmi_event_function    _cb_done;
      void                * _clientdata;

      ///
      ///  \brief Consistency required to perform the collective
      ///
      xmi_consistency_t      _consistency;
    };  //--  Executor class
  };  //-- Executor Name Space
};  //-- CCMI


#endif
