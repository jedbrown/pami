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
	CCMIQuad    _info[2];
	int         _info_count;
	int         _size;
	int         _conn;
	MPI_Request*_req;
	int         _num;
	CCMI_Callback_t _cb_done;
	inline void *buffer() { return ((char *)this + sizeof (MsgHeader)); } 
	inline int  totalsize () { return _size + sizeof (MsgHeader); } 
      };      	
      extern      std::list<MsgHeader*>              _g_sendreq_list;

      struct M2MSendReq {
	unsigned          _conn;
	CCMI_Callback_t   _cb_done;
     	int               _num;
	MPI_Request     * _reqs;
        int               _totalsize;
	char            * _bufs;
      };

      struct M2MHeader {
	unsigned    _size;
	unsigned    _conn;
	inline void *buffer() { return ((char *)this + sizeof (M2MHeader)); } 
	inline int  totalsize () { return _size + sizeof (M2MHeader); } 
      };      	

      extern      std::list<M2MSendReq*>              _g_m2m_sendreq_list;

      struct M2MRecvReq {
	unsigned          _conn;
	CCMI_Callback_t   _cb_done;
     	int               _num;
	char            * _buf;
        unsigned        * _sizes;
        unsigned        * _offsets;
	unsigned          _nranks;
      };

      extern      std::list<M2MRecvReq*>              _g_m2m_recvreq_list;

    }
  }
}
#endif
