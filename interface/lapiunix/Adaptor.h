
#ifndef  __ccmi_collectives_adaptor_h__
#define  __ccmi_collectives_adaptor_h__

#include "util/ccmi_util.h"
#include "interface/CollectiveMapping.h"
#include "util/logging/LogMgr.h"
#include "util/queue/Queue.h"
#include "util/ccmi_debug.h"
#include "interface/lapiunix/common/include/pgasrt.h"
#include <unistd.h>
#include <pthread.h>
#include <lapi.h>

extern lapi_handle_t __pgasrt_lapi_handle;

namespace CCMI
{
  namespace Adaptor
  {
    class Message : public QueueElem {
    public:
      Message () {}

      virtual void advance () = 0;
    };

    class Adaptor
    {
    protected:
      CCMI::CollectiveMapping                     _mapping;
      CCMI::Logging::LogMgr             _logmgr;
      CCMI::Queue                       _queue;

    public:
      pthread_mutex_t                   _adaptor_lock;
      lapi_thread_func_t                _tf;

      Adaptor () :  _mapping (NULL), _logmgr()
      {
	  pthread_mutexattr_t   mta;
	  pthread_mutexattr_init(&mta);
	  pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);
	  pthread_mutex_init(&_adaptor_lock, &mta);
	  TRACE_ADAPTOR((stderr,"<%#.8X>CCMI::Adaptor::Adaptor()\n",(int)this));

	  _tf.Util_type = LAPI_GET_THREAD_FUNC;
	  int rc;
	  rc = LAPI_Util(__pgasrt_lapi_handle, (lapi_util_t *)&_tf);
	  if (rc != LAPI_SUCCESS) {
	      printf("Lock sharing is not supported by this library.\n");
	  }


      }

      CCMI::CollectiveMapping  * mapping ()
      {
        return &_mapping;
      }
      CCMI::Logging::LogMgr  * getLogMgr ()
      {
        return &_logmgr;
      }

      void lock()
      {
	  //MUTEX_LOCK(&_adaptor_lock);
	  _tf.mutex_lock(__pgasrt_lapi_handle);

      }
      void unlock()
      {
	  //MUTEX_UNLOCK(&_adaptor_lock);
	  _tf.mutex_unlock(__pgasrt_lapi_handle);
      }


      void registerMessage (Message *msg) {
	//printf ("Calling Register\n");
	lock();
        TRACE_ADAPTOR((stderr,"<%#.8X>CCMI::Adaptor::Adaptor::register() %#.8X\n",(int)this,(int)msg));
	_queue.pushTail (static_cast<QueueElem *>(msg));
	unlock();
      }

      void advance ()
      {
	  lock();
	  __pgasrt_tsp_wait(NULL);
	  TRACE_ADVANCE((stderr,"<%#.8X>CCMI::Adaptor::Adaptor::advance()\n",(int)this));
	  int i = 0;
	  Message *msg = static_cast<Message *> (_queue.peekHead());

	  while (i < _queue.size())
	      {
		  i++;
		  msg->advance();
		  msg = static_cast<Message *>(msg->next());
	      }
	  unlock();
      }

    };
  };
};

extern CCMI::Adaptor::Adaptor *_g_generic_adaptor;

#endif
