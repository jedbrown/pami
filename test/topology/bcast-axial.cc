///
/// \file test/topology/bcast-axial.cc
/// \brief Simple Bcast test
///

#include "Global.h"
#include "sys/xmi.h"
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>

#ifndef __xmi_target_socklinux__
#include "algorithms/schedule/TorusRect.cc"
#define net_coord(n)	u.n_torus.coords[n]
#endif

int main (int argc, char ** argv)
{
#ifndef __xmi_target_socklinux__
  xmi_client_t  client;
  xmi_context_t context;
  xmi_result_t  result = XMI_ERROR;
  char cl_string[] = "TEST";
  result = XMI_Client_initialize (cl_string, &client);
  { size_t _n = 1; result = XMI_Context_createv(client, NULL, 0, &context, _n);}

  xmi_configuration_t configuration;
  configuration.name = XMI_TASK_ID;
  result = XMI_Configuration_query(client, &configuration);

  size_t task_id = configuration.value.intval;

  xmi_geometry_t  world_geometry;

  result = XMI_Geometry_world (context, &world_geometry);

  __global.topology_global.convertTopology(XMI_COORD_TOPOLOGY);

  unsigned char tl[4] = {0};
  xmi_coord_t myself;
  xmi_coord_t ll;
  xmi_coord_t ur;
  ll.net_coord(0) = 0;
  ll.net_coord(1) = 0;
  ll.net_coord(2) = 0;
  ll.net_coord(3) = 0;

  ur.net_coord(0) = 3;
  ur.net_coord(1) = 3;
  ur.net_coord(2) = 0;
  ur.net_coord(3) = 0;

  __global.mapping.task2network(task_id, &myself, XMI_N_TORUS_NETWORK);


  if (myself.net_coord(2) == 0 && myself.net_coord(3) == 0)
  {
    xmi_coord_t low, high, ref;
    unsigned char utl[4]={0};
    int start = -1, nphases = 0;


    XMI::Topology rect(&ll, &ur, &tl[0]);
    XMI::Topology union_topo = rect;

    CCMI::Schedule::TorusRect tr(&__global.mapping, &rect, myself, 0);
    tr.init(0, 1, start, nphases);


    xmi_result_t res = tr.getDstUnionTopology(&union_topo);
    union_topo.axial(&low, &high, &ref, &utl[0]);

    if (res == XMI_SUCCESS)
    printf("%zd start:%d nphases: %d ll: <%zd,%zd,%zd,%zd> ur:<%zd,%zd,%zd,%zd> "
           " tl <%d,%d,%d>\n", task_id, start,nphases,low.net_coord(0), low.net_coord(1),
           low.net_coord(2),low.net_coord(3),high.net_coord(0),
           high.net_coord(1),high.net_coord(2),high.net_coord(3),
           utl[0], utl[1], utl[2]);
  }
#endif // __xmi_target_socklinux__
  return 0;
};
