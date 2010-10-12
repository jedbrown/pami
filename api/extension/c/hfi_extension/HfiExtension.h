/**
 * \file api/extension/c/hfi_extension/HfiExtension.h
 * \brief PAMI extension "HFI" interface
 */
#ifndef __api_extension_c_hfi_extension_HfiExtension_h__
#define __api_extension_c_hfi_extension_HfiExtension_h__

#include <stdio.h>
#include <pami.h>
#include "pami_ext_hfi.h"
#include "lapi.h"
#include "Context.h"

namespace PAMI
{
  class HfiExtension
  {
    public:
      static pami_result_t hfi_pkt_counters (pami_context_t context,
              hfi_pkt_counter_t *counters);

      static pami_result_t hfi_remote_update (pami_context_t context,
              uint count, hfi_remote_update_info_t* info);
  };
};

#endif // __api_extension_c_hfi_extension_HfiExtension_h__
