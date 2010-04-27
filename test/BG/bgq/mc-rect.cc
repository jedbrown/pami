///
/// \file test/BG/bgq/mc-rect.cc
/// \brief Simple Bcast test
///

#include "Global.h"
#include "sys/pami.h"
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include "algorithms/schedule/MCRect.cc"

#define net_coord(n)	u.n_torus.coords[n]

#define PXCOLOR                                                                0
#define PYCOLOR                                                                1
#define PZCOLOR                                                                2
#define NXCOLOR                                                                3
#define NYCOLOR                                                                4
#define NZCOLOR                                                                5

char colors[][4] = {"+X", "+Y", "+Z", "-X", "-Y", "-Z"};

pami_task_t task_id, id;
pami_coord_t self;
int root;
void printSchedule(CCMI::Schedule::MCRect schedule, int start, int nphases,
                   char *op, int color)
{
  pami_result_t result;
  PAMI::Topology topo;
  if (task_id == (pami_task_t) root)
  {
    printf("%s %s Schedule Root %d Start phase: %d  Nphases %d\n",
           colors[color], op, root, start, nphases);
  }

  for (unsigned i = start; i < (unsigned) (start + nphases); i++)
  {
    new (&topo) PAMI:: Topology();
    schedule.getDstTopology(i, &topo);

    if (topo.size())
    {
      if (topo.type() == PAMI_COORD_TOPOLOGY)
        topo.convertTopology(PAMI_LIST_TOPOLOGY);

      if (topo.type() == PAMI_AXIAL_TOPOLOGY)
      {
        pami_network type;
        pami_task_t ref_task = 0;
        pami_coord_t low, high, ref;
        unsigned char tl[PAMI_MAX_DIMS] = {0};

        // Get the axial members
        result = topo.axial(&low, &high, &ref, tl);

        PAMI_assert(result == PAMI_SUCCESS);
        if (task_id == id || id == (pami_task_t) -1)
        {
          //char dir[32] = "";
          //for (int c = 0; c < PAMI_MAX_DIMS; c++)
          //strcat(dir, tl[c]);
          __global.mapping.network2task(&ref, &ref_task, &type);
          printf("Phase %d: R: %d L:<%zu, %zu, %zu %zu> H: <%zu, %zu, %zu %zu> \n",
                 i, ref_task, low.net_coord(0), low.net_coord(1),
                 low.net_coord(2), low.net_coord(3),high.net_coord(0),
                 high.net_coord(1),high.net_coord(2),high.net_coord(3));
        }
      }

      if (topo.type() == PAMI_LIST_TOPOLOGY)
      {
        int c;
        pami_coord_t dst;
        pami_task_t *list;
        result = topo.rankList(&list);
        PAMI_assert(result == PAMI_SUCCESS);
        PAMI_assert(list != NULL);
        if (task_id == id || id == (pami_task_t)-1)
        {
          printf("Phase %d: Rank %d sends to:", i, task_id);
          for (c = 0; c < (int) topo.size(); c++)
          {
            __global.mapping.task2network(list[c], &dst, PAMI_N_TORUS_NETWORK);
            printf(" <%zu %zu %zu %zu>", dst.net_coord(0),dst.net_coord(1),
                   dst.net_coord(2), dst.net_coord(3));
          }
          printf("\n");
        }
      }

      if (topo.type() == PAMI_SINGLE_TOPOLOGY)
      {
        pami_coord_t dst;
        pami_task_t dst_task;
        dst_task = topo.index2Rank(0);

        if (task_id == id || id == (pami_task_t) -1)
        {
          __global.mapping.task2network(dst_task, &dst, PAMI_N_TORUS_NETWORK);
          printf("Phase %d: Rank %d: <%zu %zu %zu %zu> send to %d: <%zu %zu %zu %zu>\n", i, task_id, self.net_coord(0),self.net_coord(1), self.net_coord(2),
                 self.net_coord(3),
                 dst_task, dst.net_coord(0),dst.net_coord(1), dst.net_coord(2),
                 dst.net_coord(3));
        }
      }
    }

    new (&topo) PAMI:: Topology();
    schedule.getSrcTopology(i, &topo);

    if (topo.size())
    {
      if (topo.type() == PAMI_COORD_TOPOLOGY)
        topo.convertTopology(PAMI_LIST_TOPOLOGY);

      if (topo.type() == PAMI_LIST_TOPOLOGY)
      {
        int c;
        pami_coord_t rcv;
        pami_task_t *list;
        result = topo.rankList(&list);
        PAMI_assert(result == PAMI_SUCCESS);
        PAMI_assert(list != NULL);
        if (task_id == id || id == (pami_task_t) -1)
        {
          printf("Phase %d: Rank %d recv from:", i, task_id);
          for (c = 0; c < (int) topo.size(); c++)
          {
            __global.mapping.task2network(list[c], &rcv, PAMI_N_TORUS_NETWORK);
            printf(" <%zu %zu %zu %zu>", rcv.net_coord(0),rcv.net_coord(1),
                   rcv.net_coord(2), rcv.net_coord(3));
          }
          printf("\n");
        }
      }

      if (topo.type() == PAMI_SINGLE_TOPOLOGY)
      {
        pami_coord_t rcv;
        pami_task_t rcv_task;
        rcv_task = topo.index2Rank(0);

        if (task_id == id || id == (pami_task_t) -1)
        {
          __global.mapping.task2network(rcv_task, &rcv, PAMI_N_TORUS_NETWORK);
          printf("Phase %d: Rank %d: <%zu %zu %zu %zu> recv from %d: <%zu %zu %zu %zu>\n", i, task_id,
                 self.net_coord(0),self.net_coord(1), self.net_coord(2),
                 self.net_coord(3),rcv_task, rcv.net_coord(0),rcv.net_coord(1),
                 rcv.net_coord(2),   rcv.net_coord(3));
        }
      }
    }
  }
}



int main (int argc, char ** argv)
{
  pami_client_t  client;
  pami_context_t context;
  pami_result_t  result = PAMI_ERROR;
  char cl_string[] = "TEST";
  result = PAMI_Client_create (cl_string, &client);
  { size_t _n = 1; result = PAMI_Context_createv(client, NULL, 0, &context, _n);}

  pami_configuration_t configuration;
  configuration.name = PAMI_TASK_ID;
  result = PAMI_Configuration_query(client, &configuration);

  task_id = configuration.value.intval;
  if (argc != 5)
  {
    if (task_id == 0)
      printf("missing args\nUsage: a.out id op color root\n");
    return 0;
  }

  id = (pami_task_t) atoi(argv[1]);
  char op = *argv[2];
  int color = atoi(argv[3]);
  root = atoi(argv[4]);

  pami_geometry_t  world_geometry;

  result = PAMI_Geometry_world (context, &world_geometry);

  // __global.topology_global.convertTopology(PAMI_COORD_TOPOLOGY);

  __global.mapping.task2network(task_id, &self, PAMI_N_TORUS_NETWORK);
  PAMI::Topology topo;
  int start = -1, nphases = 0;

  CCMI::Schedule::MCRect reduce(&__global.mapping,
                                &__global.topology_global,
                                color);
  CCMI::Schedule::MCRect bcast(&__global.mapping,
                               &__global.topology_global,
                               color);

  if (op == 'r')
  {
    reduce.init(root, CCMI::REDUCE_OP, start, nphases);
    printSchedule(reduce, start, nphases, (char*)"Reduce", color);
  }
  if (op == 'b')
  {
    bcast.init(root, CCMI::BROADCAST_OP, start, nphases);
    printSchedule(bcast, start, nphases, (char *)"Bcast", color);
  }
  return 0;
};
