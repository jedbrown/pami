///
/// \file api/extension/c/hfi_extension/HfiExtension.cc
/// \brief ???
///
#include "HfiExtension.h"
#include "pami.h"

extern internal_error_t _dbg_hfi_perf_counters(lapi_handle_t hndl, lapi_pkt_counter_t* cnt,
        bool is_dump);
extern int LAPI__Remote_update(lapi_handle_t ghndl, uint count,
        lapi_remote_update_t *info);

pami_result_t
PAMI::HfiExtension::hfi_pkt_counters (pami_context_t context,
        hfi_pkt_counter_t *counters)
{

  internal_error_t rst =  _dbg_hfi_perf_counters( ((Context*)context)->my_hndl,
          (lapi_pkt_counter_t*)counters, false);

  PAMI_RC( rst );
}

//TODO replace LAPI function with an internal func

pami_result_t
PAMI::HfiExtension::hfi_remote_update (pami_context_t context,
        uint count, hfi_remote_update_info_t* info)
{
  int rst = LAPI__Remote_update( ((Context*)context)->my_hndl,
          count, (lapi_remote_update_t*)info);

  if (rst == 0)
      return PAMI_SUCCESS;
  else
      return PAMI_ERROR;
}
