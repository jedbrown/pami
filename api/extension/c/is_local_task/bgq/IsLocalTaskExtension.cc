/**
 * \file api/extension/c/is_local_task/bgq/IsLocalTaskExtension.cc
 * \brief PAMI extension "is local task" bgq implementation
 */
#include <pami.h>

#include "api/extension/c/is_local_task/IsLocalTaskExtension.h"
#include "Global.h"

PAMI::IsLocalTaskExtension::IsLocalTaskExtension (pami_client_t client, pami_result_t & result)
{
  base    = __global.getMapCache()->torus.task2coords;
  stride  = sizeof(bgq_coords_t);
  bitmask = 0x40;

  result = PAMI_SUCCESS;
};

PAMI::IsLocalTaskExtension::~IsLocalTaskExtension ()
{
  base = NULL;
};
