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

/* defined in lapi_itrace.c */
#include "lapi_itrace.h"

unsigned PAMI::PeExtension::trace_mask = (unsigned)-1; /* initialize trace_mask to show every thing */

void PAMI::PeExtension::itrace(unsigned type, char* fmt, ...)
{
#ifdef USE_ITRACE
    if ((type & trace_mask) == 0)
       return; 
    va_list ap;
    va_start( ap, fmt );
    _itrace(IT_USER, fmt, ap); /* it is user's trace */
    va_end(ap);
#endif /* USE_ITRACE */
}

void PAMI::PeExtension::itrace_read_masks(pe_extension_itrace_mask_t* masks, unsigned masks_cnt, char* mask_env)
{
    trace_mask = _trace_read_mask((trc_mask_t*) masks, masks_cnt, mask_env);
}
