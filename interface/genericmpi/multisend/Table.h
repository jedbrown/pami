#ifndef   __multisend_table_h__
#define   __multisend_table_h__

#ifndef OMPI_SKIP_MPICXX
#define OMPI_SKIP_MPICXX
#endif
#include <mpi.h>
#include <list>
#include "collectives/interface/MultiSendOld.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Generic 
    {
      
      struct MsgHeader {
	CCMIQuad    _info;
	int         _size;
	int         _conn;
	MPI_Request*_req;
	int         _num;
	CCMI_Callback_t _cb_done;
	inline void *buffer() { return ((char *)this + sizeof (MsgHeader)); } 
	inline int  totalsize () { return _size + sizeof (MsgHeader); } 
      };      	
      extern      std::list<MsgHeader*>              _g_sendreq_list;
    }
  }
}
#endif
