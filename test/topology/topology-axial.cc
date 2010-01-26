/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/topology/topology-axial.cc
 * \brief Simple topology interface tests
 */

#include "Global.h"



void printTopo(XMI::Topology topo)
{
  unsigned int i, j;
  size_t global_size = __global.topology_global.size();
  xmi_topology_type_t type = topo.type();
  printf("topo of size %d is of type: ", topo.size());

  if (type == XMI_EMPTY_TOPOLOGY)
    printf("empty\n");
  if (type == XMI_SINGLE_TOPOLOGY)
    printf("single\n");
  if (type == XMI_RANGE_TOPOLOGY)
    printf("range\n");
  if (type == XMI_LIST_TOPOLOGY)
    printf("list\n");
  if (type == XMI_COORD_TOPOLOGY)
    printf("rect\n");
  if (type == XMI_AXIAL_TOPOLOGY)
    printf("axial\n");
  if (type != XMI_AXIAL_TOPOLOGY) return;
  int array[]={5,1,4,6,7,9,13};
  for(i = 0; i< topo.size(); i++)
    printf("index2Rank(%d)=%d\n", i, topo.index2Rank(i));

  for(i = 0; i< topo.size(); i++)
    printf("Is rank %d a member? %s. rank2Index(%d)=%d\n",
           array[i], topo.isRankMember(array[i]) ? "yes" : "no",
           array[i], topo.rank2Index(array[i]));
}


int main(int argc, char ** argv)
{
  size_t n = 1;
  unsigned x;
  xmi_client_t client;
  xmi_context_t context;
  xmi_result_t status = XMI_ERROR;
  xmi_configuration_t configuration;

  XMI_Client_initialize("axial-top test", &client);
  XMI_Context_createv(client, NULL, 0, &context, &n);

  configuration.name = XMI_TASK_ID;
  status = XMI_Configuration_query(client, &configuration);
  size_t task_id = configuration.value.intval;

  configuration.name = XMI_NUM_TASKS;
  XMI_Configuration_query(client, &configuration);
  size_t num_tasks = configuration.value.intval;

  printf("My task id = %zu out of %zu\n", task_id, num_tasks);


  if(task_id == 0)
  {
    XMI::Topology topology, axial, copy_topology;

    printf("global\n");
    printTopo(__global.topology_global);

    __global.topology_global.convertTopology(XMI_LIST_TOPOLOGY);

    __global.topology_global.convertTopology(XMI_COORD_TOPOLOGY);
    printf("rect\n");
    printTopo(__global.topology_global);


    xmi_coord_t ll, ur, center;
    __global.topology_global.rectSeg(&ll, &ur, NULL);
    int i;
    printf("lower corner:<");
    for (i = 0; i < 4; i++)
    {
      printf(" %d", ll.u.n_torus.coords[i]);
    }
    printf(">\n");
    printf("upper corner:<");
    for (i = 0; i < 4; i++)
    {
      printf(" %d", ur.u.n_torus.coords[i]);
    }
    printf(">\n");

    ll.u.n_torus.coords[0] = 0;
    ll.u.n_torus.coords[1] = 0;
    ll.u.n_torus.coords[2] = 0;
    ll.u.n_torus.coords[3] = 0;

    ur.u.n_torus.coords[0] = 3;
    ur.u.n_torus.coords[1] = 3;
    ur.u.n_torus.coords[2] = 0;
    ur.u.n_torus.coords[3] = 0;

    center.u.n_torus.coords[0] = 1;
    center.u.n_torus.coords[1] = 1;
    center.u.n_torus.coords[2] = 0;
    center.u.n_torus.coords[3] = 0;

    unsigned char dir = 16;
    unsigned char tl = 0;

    new (&axial) XMI::Topology(&ll, &ur, &center, &dir, &tl);
    printTopo(axial);
    printf("----------------------\n");
    unsigned char dir1, tl1;
    xmi_coord_t low, high;
    axial.getAxialOrientation(&tl1);
    axial.getAxialDirs(&dir1);
    //  printf("dir: %d  tl: %d\n", dir1, tl1);
    axial.getAxialEndCoords(&low, &high,0);
    printf("low:<");
    for (i = 0; i < 4; i++)
    {
      printf(" %d", low.u.n_torus.coords[i]);
    }
    printf(">\n");

    printf("high:<");
    for (i = 0; i < 4; i++)
    {
      printf(" %d", high.u.n_torus.coords[i]);
    }
    printf(">\n");

    axial.getAxialEndCoords(&low, &high,1);
    printf("low:<");
    for (i = 0; i < 4; i++)
    {
      printf(" %d", low.u.n_torus.coords[i]);
    }
    printf(">\n");

    printf("high:<");
    for (i = 0; i < 4; i++)
    {
      printf(" %d", high.u.n_torus.coords[i]);
    }
    printf(">\n");
  }

  status = XMI_Context_destroy(context);

  status = XMI_Client_finalize(client);

  return 0;
}
