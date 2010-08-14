///
/// \file components/devices/lapiunix/regTable.cc
/// \brief ???
///

#include <map>
#include "common/lapiunix/lapifunc.h"

namespace PAMI
{
  namespace Device
  {
    std::map<lapi_handle_t,void*> _g_context_to_device_table;
    std::map<lapi_handle_t,void*> _g_id_to_device_table;
    std::map<int,int> _g_lapi_mcast_to_pami_mcast;
    std::map<int,int> _g_pami_mcast_to_lapi_mcast;
    
  }
}
