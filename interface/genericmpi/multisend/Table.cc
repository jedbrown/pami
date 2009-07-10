#include "Table.h"
#include <list>

namespace CCMI
{
  namespace Adaptor
  {
    namespace Generic
    {
      std::list<MsgHeader*>              _g_sendreq_list;
      std::list<M2MSendReq*>             _g_m2m_sendreq_list;
      std::list<M2MRecvReq*>             _g_m2m_recvreq_list;
    }
  }
}
