///
/// \file components/devices/lapiunix/regTable.cc
/// \brief ???
///

#include <map>
#include "common/lapiunix/lapifunc.h"

namespace XMI
{
  namespace Device
  {
    std::map<lapi_handle_t,void*> _g_context_to_device_table;
  }
}
