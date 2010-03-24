/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/internals/bgq/personality.cc
 * \brief Simple multicast tests.
 */


#include "Global.h"

#include <hwi/include/bqc/nd_500_dcr.h>

#include <unistd.h>

int main(int argc, char ** argv)
{
  char* var;
  unsigned dcr_num = ND_500_DCR_base + ND_500_DCR__CTRL_COORDS_offset;

  unsigned long long dcr = DCRReadUser(dcr_num);

  fprintf(stderr,"main(): address of TORUS NODE_COORDINATES DCR(%#X) = %p\n", dcr_num, &dcr);
  fprintf(stderr,"main(): contents of TORUS NODE_COORDINATES DCR = %#16.16llx\n", dcr);

  fprintf(stderr,"NODE_COORD A = %#llx\n", ND_500_DCR__CTRL_COORDS__NODE_COORD_A_get(dcr));
  fprintf(stderr,"NODE_COORD B = %#llx\n", ND_500_DCR__CTRL_COORDS__NODE_COORD_B_get(dcr));
  fprintf(stderr,"NODE_COORD C = %#llx\n", ND_500_DCR__CTRL_COORDS__NODE_COORD_C_get(dcr));
  fprintf(stderr,"NODE_COORD D = %#llx\n", ND_500_DCR__CTRL_COORDS__NODE_COORD_D_get(dcr));
  fprintf(stderr,"NODE_COORD E = %#llx\n", ND_500_DCR__CTRL_COORDS__NODE_COORD_E_get(dcr));

  fprintf(stderr,"MAX_COORD A = %#llx\n", ND_500_DCR__CTRL_COORDS__MAX_COORD_A_get(dcr));
  fprintf(stderr,"MAX_COORD B = %#llx\n", ND_500_DCR__CTRL_COORDS__MAX_COORD_B_get(dcr));
  fprintf(stderr,"MAX_COORD C = %#llx\n", ND_500_DCR__CTRL_COORDS__MAX_COORD_C_get(dcr));
  fprintf(stderr,"MAX_COORD D = %#llx\n", ND_500_DCR__CTRL_COORDS__MAX_COORD_D_get(dcr));
  fprintf(stderr,"MAX_COORD E = %#llx\n", ND_500_DCR__CTRL_COORDS__MAX_COORD_E_get(dcr));


  var = getenv("BG_PROCESSESPERNODE");
  fprintf(stderr,"BG_PROCESSESPERNODE %s\n",var? var: "NULL");

  var = getenv("BG_SHAREDMEMSIZE");
  fprintf(stderr,"BG_SHAREDMEMSIZE %s\n",var? var: "NULL");

  var = getenv("USER");
  fprintf(stderr,"USER %s\n",var? var: "NULL");

  var = getenv("PATH");
  fprintf(stderr,"PATH %s\n",var? var: "NULL");

  fprintf(stderr,"BGQ Personality A coord %zd\n",__global.personality.aCoord());
  fprintf(stderr,"BGQ Personality B coord %zd\n",__global.personality.bCoord());
  fprintf(stderr,"BGQ Personality C coord %zd\n",__global.personality.cCoord());
  fprintf(stderr,"BGQ Personality D coord %zd\n",__global.personality.dCoord());
  fprintf(stderr,"BGQ Personality E coord %zd\n",__global.personality.eCoord());

  fprintf(stderr,"BGQ Personality A size %zd\n",__global.personality.aSize());
  fprintf(stderr,"BGQ Personality B size %zd\n",__global.personality.bSize());
  fprintf(stderr,"BGQ Personality C size %zd\n",__global.personality.cSize());
  fprintf(stderr,"BGQ Personality D size %zd\n",__global.personality.dSize());
  fprintf(stderr,"BGQ Personality E size %zd\n",__global.personality.eSize());

  size_t task_id = __global.mapping.task();
  fprintf(stderr,"HELLO from task %zd\n",task_id);

  char clientname[] = "PAMI";
  pami_client_t client;
  fprintf(stderr, "PAMI_Client_initialize()\n");
  PAMI_Client_initialize (clientname, &client);
  pami_context_t context;
  fprintf(stderr, "PAMI_Context_create()\n");
  { size_t _n = 1; PAMI_Context_createv (client, NULL, 0, &context, _n); }

  pami_configuration_t configuration;
  pami_result_t result;

  configuration.name = PAMI_TASK_ID;
  result = PAMI_Configuration_query(client, &configuration);
  size_t my_rank = configuration.value.intval;
  fprintf(stderr, "PAMI_TASK_ID %zd\n",my_rank);

  configuration.name = PAMI_NUM_TASKS;
  result = PAMI_Configuration_query(client, &configuration);
  size_t num_tasks = configuration.value.intval;
  fprintf(stderr, "PAMI_NUM_TASKS %zd\n",num_tasks);

  configuration.name = PAMI_WTICK;
  result = PAMI_Configuration_query(client, &configuration);
  double tick = configuration.value.doubleval;
  fprintf(stderr, "PAMI_WTICK %f\n",tick);

  fprintf(stderr,"GOODBYE from task %zd/%zd\n",task_id,my_rank);

  return 0;
}
