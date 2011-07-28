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
#include "p2p/protocols/Get.h"
#include "p2p/protocols/RPut.h"
#include "p2p/protocols/RGet.h"

namespace PAMI
{
  template <unsigned T_DispatchCount>
  class Dispatch
  {
    protected:

      Protocol::Send::Error    _default;
      Protocol::Send::Send  *  _protocol_fence_disabled[T_DispatchCount];
      Protocol::Send::Send  *  _protocol_fence_enabled[T_DispatchCount];
      Protocol::Send::Send  ** _protocol;


    public:

      int id;

      inline Dispatch () :
        id (T_DispatchCount-1)
      {
        size_t i;
        for (i=0; i<T_DispatchCount; i++)
        {
          _protocol_fence_disabled[i] = & _default;
          _protocol_fence_enabled[i]  = & _default;
        };
        _protocol = _protocol_fence_disabled;
      };

      inline ~Dispatch () {};

      /// use this to set the fence protocol pointer, or is it just a data member?
      inline void init (Protocol::Put::Put * fence_supported_put,
                        Protocol::Put::Put * fence_unsupported_put,
                        Protocol::Get::Get * fence_supported_get,
                        Protocol::Get::Get * fence_unsupported_get,
                        Protocol::Put::RPut * fence_supported_rput,
                        Protocol::Put::RPut * fence_unsupported_rput,
                        Protocol::Get::RGet * fence_supported_rget,
                        Protocol::Get::RGet * fence_unsupported_rget
                        )
      {
      };

      inline pami_result_t query (size_t                dispatch_id,
                                  pami_configuration_t  configuration[],
                                  size_t                num_configs)
      {
        if (dispatch_id >= T_DispatchCount) return PAMI_INVAL;
        return _protocol[id]->getAttributes (configuration, num_configs);
      };

      inline pami_result_t update (size_t                dispatch_id,
                                   pami_configuration_t  configuration[],
                                   size_t                num_configs)
      {
        if (dispatch_id >= T_DispatchCount) return PAMI_INVAL;
        return PAMI_INVAL;
      };

      inline void fence (bool enable)
      {
        if (enable)
          _protocol = _protocol_fence_enabled;
        else
          _protocol = _protocol_fence_disabled;
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

      inline pami_result_t set (size_t                 dispatch_id,
                                Protocol::Send::Send * fence_supported_protocol,
                                Protocol::Send::Send * fence_unsupported_protocol)
      {
        if (dispatch_id >= T_DispatchCount) return PAMI_INVAL;
        if (_protocol_fence_enabled[dispatch_id]  != (& _default)) return PAMI_INVAL;
        if (_protocol_fence_disabled[dispatch_id] != (& _default)) return PAMI_INVAL;

        _protocol_fence_enabled[dispatch_id]  = fence_supported_protocol;
        _protocol_fence_disabled[dispatch_id] = fence_unsupported_protocol;

        return PAMI_SUCCESS;
      };

      inline pami_result_t send (pami_send_t * parameters)
      {
        size_t dispatch_id = (size_t)(parameters->send.dispatch);
        PAMI_assert_debug (dispatch_id < T_DispatchCount);
        return _protocol[dispatch_id]->simple (parameters);
      };

      inline pami_result_t send (pami_send_immediate_t * parameters)
      {
        size_t dispatch_id = (size_t)(parameters->dispatch);
        PAMI_assert_debug (dispatch_id < T_DispatchCount);
        return _protocol[dispatch_id]->immediate (parameters);
      };
  };
};
#endif // __components_dispatch_Dispatch_h__
