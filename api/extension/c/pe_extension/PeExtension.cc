/**
 * \file api/extension/c/pe_extension/PeExtension.cc
 * \brief ???
 */
#include "PeExtension.h"

PAMI::PamiActiveClients _pami_act_clients;

pami_result_t
PAMI::PeExtension::global_query(pami_configuration_t    configs[], 
                                size_t                  num_configs)
{
  pami_result_t result = PAMI_SUCCESS;
  size_t i;
  for (i = 0; i < num_configs; i ++)
  {
    switch (configs[i].name) {
      case PAMI_ACTIVE_CLIENT:
        configs[i].value.chararray = (char*)&(_pami_act_clients);
        break;
      default:
        result = PAMI_INVAL;
    }
  }
  return result;
}
