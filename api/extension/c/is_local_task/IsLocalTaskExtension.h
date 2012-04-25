/**
 * \file api/extension/c/is_local_task/IsLocalTaskExtension.h
 * \brief PAMI extension "is local task" interface
 */
#ifndef __api_extension_c_is_local_task_IsLocalTaskExtension_h__
#define __api_extension_c_is_local_task_IsLocalTaskExtension_h__

#include <pami.h>
#include <stdint.h>

namespace PAMI
{
  class IsLocalTaskExtension
  {
    public:

      void      * base;
      uintptr_t   stride;
      uintptr_t   bitmask;

      IsLocalTaskExtension (pami_client_t client, pami_result_t & result);

      ~IsLocalTaskExtension ();
  };
};

#endif // __api_extension_c_is_local_task_IsLocalTaskExtension_h__
