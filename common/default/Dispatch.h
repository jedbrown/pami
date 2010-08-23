/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common/default/Dispatch.h
 * \brief ???
 */

#ifndef __common_default_Dispatch_h__
#define __common_default_Dispatch_h__

#include <pami.h>

#include "p2p/protocols/Send.h"
#include "p2p/protocols/Put.h"
#include "p2p/protocols/GetInterface.h"
#include "p2p/protocols/RPut.h"
#include "p2p/protocols/RGet.h"

namespace PAMI
{
  class Dispatch
  {
    protected:

      static const size_t dispatch_id_max = 1024;

      typedef enum
      {
        FENCE_MODE_DISABLED = 0,
        FENCE_MODE_ENABLED
      } fence_mode_t;

      Protocol::Send::Error   _default;
      Protocol::Send::Send  * _protocol[dispatch_id_max][2];
      fence_mode_t            _mode;


    public:

      inline Dispatch () :
        _mode (FENCE_MODE_DISABLED)
      {
        size_t i;
        for (i=0; i<dispatch_id_max; i++)
        {
          _protocol[i][FENCE_MODE_ENABLED] = & _default;
          _protocol[i][FENCE_MODE_DISABLED] = & _default;
        };
      };

      inline ~Dispatch () {};

      /// use this to set the fence protocol pointer, or is it just a data member?
      inline void init (Protocol::Put::Put * fence_supported_put,
                        Protocol::Put::Put * fence_unsupported_put,
                        Protocol::Get::GetInterface * fence_supported_get,
                        Protocol::Get::GetInterface * fence_unsupported_get,
                        Protocol::Put::RPut * fence_supported_rput,
                        Protocol::Put::RPut * fence_unsupported_rput,
                        Protocol::Get::RGet * fence_supported_rget,
                        Protocol::Get::RGet * fence_unsupported_rget
                        )
      {
      };

      inline pami_result_t query (size_t                id,
                                  pami_configuration_t  configuration[],
                                  size_t                num_configs)
      {
        if (id >= dispatch_id_max) return PAMI_INVAL;
        return _protocol[id][_mode]->getAttributes (configuration, num_configs);
      };

      inline pami_result_t update (size_t                id,
                                   pami_configuration_t  configuration[],
                                   size_t                num_configs)
      {
        if (id >= dispatch_id_max) return PAMI_INVAL;
        return PAMI_INVAL;
      };

      inline void fence (bool enable)
      {
        _mode = (fence_mode_t) enable;
      };

      /// "fence all"
      inline pami_result_t fence (pami_event_function   done_fn,
                                  void                * cookie)
      {
        return PAMI_UNIMPL;
      };

      /// "fence endpoint"
      inline pami_result_t fence (pami_event_function   done_fn,
                                  void                * cookie,
                                  pami_endpoint_t       endpoint)
      {
        return PAMI_UNIMPL;
      };

      inline pami_result_t set (size_t                 id,
                                Protocol::Send::Send * fence_supported_protocol,
                                Protocol::Send::Send * fence_unsupported_protocol)
      {
        if (id >= dispatch_id_max) return PAMI_INVAL;
        if (_protocol[id][FENCE_MODE_ENABLED]  != (& _default)) return PAMI_INVAL;
        if (_protocol[id][FENCE_MODE_DISABLED] != (& _default)) return PAMI_INVAL;

        _protocol[id][FENCE_MODE_ENABLED]  = fence_supported_protocol;
        _protocol[id][FENCE_MODE_DISABLED] = fence_unsupported_protocol;

        return PAMI_SUCCESS;
      };

      inline pami_result_t start (pami_send_t * parameters)
      {
        size_t id = (size_t)(parameters->send.dispatch);
        PAMI_assert_debug (id < dispatch_id_max);
        return _protocol[id][_mode]->simple (parameters);
      };

      inline pami_result_t start (pami_send_immediate_t * parameters)
      {
        size_t id = (size_t)(parameters->dispatch);
        PAMI_assert_debug (id < dispatch_id_max);
        return _protocol[id][_mode]->immediate (parameters);
      };
  };
};
#endif // __components_dispatch_Dispatch_h__
