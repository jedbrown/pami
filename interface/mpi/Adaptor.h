
#ifndef  __ccmi_collectives_adaptor_h__
#define  __ccmi_collectives_adaptor_h__
#warning mpi adaptor

#include "util/ccmi_util.h"
#include "interface/CollectiveMapping.h"
#include "util/logging/LogMgr.h"
#include "util/queue/Queue.h"
#include "util/ccmi_debug.h"

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

      Adaptor () :  _mapping (NULL), _logmgr()
      {
        TRACE_ADAPTOR((stderr,"<%#.8X>CCMI::Adaptor::Adaptor()\n",(int)this));
      }

      CCMI::CollectiveMapping  * mapping ()
      {
        return &_mapping;
      }
      CCMI::Logging::LogMgr  * getLogMgr ()
      {
        return &_logmgr;
      }

      void registerMessage (Message *msg) {
	//printf ("Calling Register\n");
        TRACE_ADAPTOR((stderr,"<%#.8X>CCMI::Adaptor::Adaptor::register() %#.8X\n",(int)this,(int)msg));
	_queue.pushTail (static_cast<QueueElem *>(msg));
      }

      void advance () {       
	//	printf ("In Adaptor advance %d\n", _queue.size());
	
        TRACE_ADVANCE((stderr,"<%#.8X>CCMI::Adaptor::Adaptor::advance()\n",(int)this));
	int i = 0;
	Message *msg = static_cast<Message *> (_queue.peekHead());
	
	while (i < _queue.size()) {
	  i++;
	  msg->advance();
	  msg = static_cast<Message *>(msg->next());
	}
      } 
      
    };
  };
};

extern CCMI::Adaptor::Adaptor *_g_mpi_adaptor;

#endif
