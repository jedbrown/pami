/**
 * \file api/c/pami.cc
 * \brief PAMI C interface common implementation
 */
#ifndef PAMI_LAPI_IMPL
#include "config.h"
#endif //PAMI_LAPI_IMPL

#include "Global.h"
#include "SysDep.h"
#include "Client.h"
#include "Context.h"
#include "PipeWorkQueue.h"
#include "Topology.h"
#include <pami.h>
#include "util/common.h"


///
/// \copydoc PAMI_Task2Network
///
extern "C" pami_result_t PAMI_Task2Network(pami_task_t task,
                                           pami_coord_t *ntw)
{
  return __global.mapping.task2network(task, ntw, PAMI_N_TORUS_NETWORK);
}


///
/// \copydoc PAMI_Network2Task
///
extern "C" pami_result_t PAMI_Network2Task(pami_coord_t ntw,
                                           pami_task_t *task)
{
  pami_network type;
  return __global.mapping.network2task(&ntw, task, &type);
}
