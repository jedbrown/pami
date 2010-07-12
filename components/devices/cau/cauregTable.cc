///
/// \file components/devices/cau/cauregTable.cc
/// \brief ???
///

#include <map>
#include "common/lapiunix/lapifunc.h"

namespace PAMI
{
  namespace Device
  {
    std::map<lapi_handle_t,void*> _g_context_to_cau_device_table;
  }
}
