/**
 * \file algorithms/interfaces/Executor.h
 * \brief ???
 */

#ifndef __algorithms_interfaces_Executor_h__
#define __algorithms_interfaces_Executor_h__

#include "algorithms/ccmi.h"
#include "util/ccmi_util.h"
#include "util/ccmi_debug.h"
#include "PipeWorkQueue.h"

namespace CCMI
{
  namespace Interfaces
  {
    /**
     * \brief Base Class for all Executors.
     */
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
      }

      /**
       * \brief  Destructor
       */
      virtual ~Executor ()
      {
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
       * \brief notify when a message has been recived
       * \param src : source of the message
       * \param buf : address of the pipeworkqueue to produce incoming message
       * \param cb_done: completion callback
       */
      virtual void   notifyRecv     (unsigned             src,
				     const pami_quad_t   & info,
				     PAMI::PipeWorkQueue ** pwq,
				     pami_callback_t      * cb_done) = 0;

      void setDoneCallback (pami_event_function cb_done, void *cd)
      {
        _cb_done    =   cb_done;
        _clientdata =   cd;
      }


    protected:
      ///
      ///  \brief Callback to call when the barrier has finished
      ///
      pami_event_function    _cb_done;
      void                * _clientdata;

    };  //--  Executor class
  };  //-- Executor Name Space
};  //-- CCMI


#endif
