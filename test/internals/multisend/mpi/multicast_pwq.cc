/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/internals/multisend/mpi/multicast_pwq.cc 
 * \brief Simple multicast test using pwq's to chain operations.  
 */
#include <stdio.h>
#include "sys/xmi.h"
#include "test/internals/multisend/mpi/multicast.h"
#include "components/devices/mpi/mpiglobalmcastmodel.h"
#include "components/devices/mpi/mpimessage.h"
#include "components/devices/mpi/mpiglobaldevice.h"
#include "common/mpi/SysDep.h"
#ifndef TEST_BUF_SIZE
  #define TEST_BUF_SIZE	1024
#endif // TEST_BUF_SIZE

#define GLOBAL_BCAST_DEVICE  XMI::Device::MPIGlobalDevice<XMI::SysDep>
#define GLOBAL_BCAST_MESSAGE XMI::Device::MPIGlobalMcastMessage
#define GLOBAL_BCAST_MODEL   XMI::Device::MPIGlobalMcastModel<GLOBAL_BCAST_DEVICE,GLOBAL_BCAST_MESSAGE>

int main(int argc, char ** argv) {
  unsigned x;
  xmi_client_t client;
  xmi_context_t context;
  xmi_result_t status = XMI_ERROR;

  status = XMI_Client_initialize("multicast test", &client);
  if(status != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", status);
    return 1;
  }

  int n = 1; 
  status = XMI_Context_createv(client, NULL, 0, &context, &n); 
  if(status != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create xmi context. result = %d\n", status);
    return 1;
  }

  xmi_configuration_t configuration;

  configuration.name = XMI_TASK_ID;
  status = XMI_Configuration_query(context, &configuration);
  if(status != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
    return 1;
  }
  size_t task_id = configuration.value.intval;
  //fprintf(stderr, "My task id = %zd\n", task_id);

  configuration.name = XMI_NUM_TASKS;
  status = XMI_Configuration_query(context, &configuration);
  if(status != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
    return 1;
  }
  size_t num_tasks = configuration.value.intval;
  if(task_id == 0) fprintf(stderr, "Number of tasks = %zd\n", num_tasks);
  if(__global.topology_local.size() < 2)
  {
    fprintf(stderr, "requires at least 2 ranks to be local\n");
    exit(1);
  }

// END standard setup
// ------------------------------------------------------------------------

  xmi_result_t rc;
  // Register some multicasts, C++ style

  //For testing ease, I'm assuming rank list topology.
  __global.topology_global.convertTopology(XMI_LIST_TOPOLOGY);
  __global.topology_local.convertTopology(XMI_LIST_TOPOLOGY);

  size_t root = __global.topology_local.index2Rank(0);
  size_t * rankList; __global.topology_local.rankList(&rankList);
  size_t tSize = __global.topology_local.size();
  size_t lSize=0;
  xmi_multicast_t mcast1,mcast2;
  size_t * gRankList; __global.topology_global.rankList(&gRankList);
  size_t gSize = __global.topology_global.size();

  XMI::Topology itopo(root);
  XMI::Topology otopo; // uninitialized storage

  DBG_PRINTFS_1;

  // simple bcast to all


  XMI::MPI::Test::Multisend::Multicast<GLOBAL_BCAST_MODEL, GLOBAL_BCAST_DEVICE, TEST_BUF_SIZE> globalTest(task_id); 


  mcast1.src_participants = (xmi_topology_t *)&itopo;
  mcast1.dst_participants = (xmi_topology_t *)&otopo;

  mcast1.src = (xmi_pipeworkqueue_t *)&globalTest._ipwq;
  mcast1.dst = (xmi_pipeworkqueue_t *)&globalTest._opwq;

  mcast1.context = (xmi_context_t*) &context;
  mcast1.roles = -1;
  mcast1.bytes = TEST_BUF_SIZE;

  mcast2.src_participants = (xmi_topology_t *)&itopo;
  mcast2.dst_participants = (xmi_topology_t *)&otopo;

  mcast2.src = (xmi_pipeworkqueue_t *)&globalTest._ipwq;
  mcast2.dst = (xmi_pipeworkqueue_t *)&globalTest._opwq;

  mcast2.context = (xmi_context_t*) &context;
  mcast2.roles = -1;
  mcast2.bytes = TEST_BUF_SIZE;

  char middle[TEST_BUF_SIZE];
  XMI::PipeWorkQueue mpwq;
  mpwq.configure(NULL, middle, sizeof(middle), 0);
  mpwq.reset();
// ------------------------------------------------------------------------
  // Global output topology with everyone including root
  new (&otopo) XMI::Topology(gRankList, (gSize));

  DBG_PRINTFS_2;                                                       


  rc = globalTest.setup_test(task_id, num_tasks, &mcast1);
  rc = globalTest.setup_test(task_id, num_tasks, &mcast2); 


  memset((void*)((XMI::PipeWorkQueue*)mcast1.dst)->bufferToConsume(),0x00,TEST_BUF_SIZE);
  memset((void*)((XMI::PipeWorkQueue*)mcast2.src)->bufferToProduce(),0x00,TEST_BUF_SIZE);
 
  mcast1.dst = (xmi_pipeworkqueue_t*) &mpwq;
  mcast2.src = mcast1.dst;

  rc = globalTest.start_test(task_id, num_tasks, &mcast1);
  rc = globalTest.start_test(task_id, num_tasks, &mcast2);

  rc = globalTest.finish_test(task_id, num_tasks, &mcast2);

  // Fake out validate.  Source from mcast1, destination from mcast2.
  mcast2.src = mcast1.src;
  rc = globalTest.validate_test(task_id, num_tasks, &mcast2);
  sleep(5);

// ------------------------------------------------------------------------
  DBG_FPRINTF((stderr, "XMI_Context_destroy(context);\n"));
  status = XMI_Context_destroy(context);
  if(status != XMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable to destroy xmi context. result = %d\n", status);
    return 1;
  }

  DBG_FPRINTF((stderr, "XMI_Client_finalize(client);\n"));
  status = XMI_Client_finalize(client);
  if(status != XMI_SUCCESS)
  {
    fprintf(stderr, "Error. Unable to finalize xmi client. result = %d\n", status);
    return 1;
  }

  DBG_FPRINTF((stderr, "return 0;\n"));
  return 0;
}
