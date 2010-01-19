/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/topology/topology.cc
 * \brief Simple topology interface tests
 */

#include "Global.h"


#define DBG_FPRINTF(x) //fprintf x

#define FPRINTF_TOPOLOGY(topo)    // disable fprintf's

#ifndef FPRINTF_TOPOLOGY // otherwise enable fprintfs
#define FPRINTF_TOPOLOGY(topo)                                                 \
{                                                                              \
size_t global_size = __global.topology_global.size();                          \
xmi_topology_type_t type = topo.type();                                        \
fprintf(stderr,"type %d/%s, size %d\n",                                       \
        type,                                                                  \
        type==XMI_EMPTY_TOPOLOGY?"XMI_EMPTY_TOPOLOGY":                         \
        ((type==XMI_SINGLE_TOPOLOGY)?"XMI_SINGLE_TOPOLOGY":                    \
         ((type==XMI_RANGE_TOPOLOGY)?"XMI_RANGE_TOPOLOGY":                     \
          ((type==XMI_LIST_TOPOLOGY)?"XMI_LIST_TOPOLOGY":                      \
           ((type==XMI_COORD_TOPOLOGY)?"XMI_COORD_TOPOLOGY":"bogus")))),       \
        topo.size());                                                          \
for(unsigned j=0; j< topo.size(); ++j)                                         \
{                                                                              \
  fprintf(stderr,"index2Rank(%d)=%d\n",                                       \
                 j,topo.index2Rank(j));                                        \
}                                                                              \
for(unsigned j=0; j< global_size; ++j)                                         \
{                                                                              \
  fprintf(stderr,"Is rank %d a member? %s. rank2Index(%d)=%d\n",              \
                 j,topo.isRankMember(j)?"yes":"no",                            \
                 j,topo.rank2Index(j));                                        \
}                                                                              \
fprintf(stderr,"\n");                                                          \
}
#endif

// Topology, expected size (-1 means size should be > 0 but unknown)
// #define TEST_TOPOLOGY(topo,expected_size)
inline void TEST_TOPOLOGY(XMI::Topology topo, size_t expected_size)
{
size_t global_size = __global.topology_global.size();
FPRINTF_TOPOLOGY(topo);
xmi_topology_type_t type = topo.type();
if((expected_size==(size_t)-1) && (!topo.size()))
  fprintf(stderr,"FAIL: type %d/%s, expected size > 0, actual size %zu\n",
        type,
        type==XMI_EMPTY_TOPOLOGY?"XMI_EMPTY_TOPOLOGY":
        ((type==XMI_SINGLE_TOPOLOGY)?"XMI_SINGLE_TOPOLOGY":
         ((type==XMI_RANGE_TOPOLOGY)?"XMI_RANGE_TOPOLOGY":
          ((type==XMI_LIST_TOPOLOGY)?"XMI_LIST_TOPOLOGY":
           ((type==XMI_COORD_TOPOLOGY)?"XMI_COORD_TOPOLOGY":"bogus")))),
        topo.size());
if((expected_size!=(unsigned)-1) && (expected_size!=topo.size()))
  fprintf(stderr,"FAIL: type %d/%s, expected size %zu, actual size %zu\n",
        type,
        type==XMI_EMPTY_TOPOLOGY?"XMI_EMPTY_TOPOLOGY":
        ((type==XMI_SINGLE_TOPOLOGY)?"XMI_SINGLE_TOPOLOGY":
         ((type==XMI_RANGE_TOPOLOGY)?"XMI_RANGE_TOPOLOGY":
          ((type==XMI_LIST_TOPOLOGY)?"XMI_LIST_TOPOLOGY":
           ((type==XMI_COORD_TOPOLOGY)?"XMI_COORD_TOPOLOGY":"bogus")))),
        expected_size, topo.size());
if((type!=XMI_EMPTY_TOPOLOGY) &&
   (type!=XMI_SINGLE_TOPOLOGY) &&
   (type!=XMI_RANGE_TOPOLOGY) &&
   (type!=XMI_LIST_TOPOLOGY) &&
   (type!=XMI_COORD_TOPOLOGY))
  fprintf(stderr,"FAIL: type %d/%s, size %zu\n",
        type,
        type==XMI_EMPTY_TOPOLOGY?"XMI_EMPTY_TOPOLOGY":
        ((type==XMI_SINGLE_TOPOLOGY)?"XMI_SINGLE_TOPOLOGY":
         ((type==XMI_RANGE_TOPOLOGY)?"XMI_RANGE_TOPOLOGY":
          ((type==XMI_LIST_TOPOLOGY)?"XMI_LIST_TOPOLOGY":
           ((type==XMI_COORD_TOPOLOGY)?"XMI_COORD_TOPOLOGY":"bogus")))),
        topo.size());
for(unsigned j=0; j< topo.size(); ++j)
{
  if(!topo.isRankMember(topo.index2Rank(j)))
  fprintf(stderr,"FAIL: index2Rank(%d)=%d is not a member\n",
                 j,topo.index2Rank(j));
}
size_t nmembers = 0;
for(size_t j=0; j< global_size; ++j)
{
  if(topo.isRankMember(j)) nmembers++;
  if((topo.rank2Index(j)!=(unsigned)-1) && (topo.rank2Index(j) >= topo.size()))
    fprintf(stderr,"FAIL: rank2Index(%zu)= %zu, index >= size %zu\n",
            j,topo.rank2Index(j),topo.size());
  if(!topo.isRankMember(j) && (topo.rank2Index(j) != (unsigned)-1))
    fprintf(stderr,"FAIL: Is %zu a member? %s. rank2Index(%zu)=%zu\n",
            j,topo.isRankMember(j)?"yes":"no",
            j,topo.rank2Index(j));
  if(topo.isRankMember(j) && (topo.rank2Index(j) == (unsigned)-1))
    fprintf(stderr,"FAIL: Is %zu a member? %s. rank2Index(%zu)=%zu\n",
            j,topo.isRankMember(j)?"yes":"no",
            j,topo.rank2Index(j));
  if(topo.isRankMember(j) &&
     (j != topo.index2Rank(topo.rank2Index(j))))
    fprintf(stderr,"FAIL: Is %zu a member? %s. rank2Index(%zu)=%zu index2Rank(%zu)=%d\n",
            j,topo.isRankMember(j)?"yes":"no",
            j,topo.rank2Index(j),
            topo.rank2Index(j),topo.index2Rank(topo.rank2Index(j)));
}
if(nmembers != topo.size())
    fprintf(stderr,"FAIL: nmembers %zu != size %zu\n",
            nmembers,topo.size());
}


int main(int argc, char ** argv)
{
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
  DBG_FPRINTF((stderr,"Client %p\n",client));
  size_t n = 1;
  status = XMI_Context_createv(client, NULL, 0, &context, &n);
  if(status != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable to create xmi context. result = %d\n", status);
    return 1;
  }

  xmi_configuration_t configuration;

  configuration.name = XMI_TASK_ID;
  status = XMI_Configuration_query(client, &configuration);
  if(status != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
    return 1;
  }
  size_t task_id = configuration.value.intval;
  DBG_FPRINTF((stderr, "My task id = %zu\n", task_id));

  configuration.name = XMI_NUM_TASKS;
  status = XMI_Configuration_query(client, &configuration);
  if(status != XMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
    return 1;
  }
  size_t num_tasks = configuration.value.intval;
  if(task_id == 0) fprintf(stderr, "Number of tasks = %zu\n", num_tasks);

// END standard setup
// ------------------------------------------------------------------------
if(task_id == 0)
{
  COMPILE_TIME_ASSERT(sizeof(xmi_topology_t) >= sizeof(XMI::Topology));

  size_t  gSize    = __global.topology_global.size();
  XMI::Topology topology, subtopology, copy_topology;

  fprintf(stderr,"\n");fprintf(stderr,"global\n");
  TEST_TOPOLOGY(__global.topology_global,gSize);

  fprintf(stderr,"\n");fprintf(stderr,"global converted to list - DO NOT DO THIS TO GLOBAL IN REAL APPS\n");
  __global.topology_global.convertTopology(XMI_LIST_TOPOLOGY);
  TEST_TOPOLOGY(__global.topology_global,gSize);

  xmi_task_t *gRankList=NULL; __global.topology_global.rankList(&gRankList);//valid only after converting to list

  fprintf(stderr,"\n");fprintf(stderr,"local\n");
  TEST_TOPOLOGY(__global.topology_local,__global.topology_local.size());

  fprintf(stderr,"\n");fprintf(stderr,"local converted to list - DO NOT DO THIS TO GLOBAL IN REAL APPS\n");
  __global.topology_local.convertTopology(XMI_LIST_TOPOLOGY);
  TEST_TOPOLOGY(__global.topology_local,__global.topology_local.size());

  fprintf(stderr,"\n");fprintf(stderr,"global subTopologyNthGlobal(0) - probably not supported on XMI_LIST_TOPOLOGY?\n");
  __global.topology_global.subTopologyNthGlobal(&subtopology, 0); //0th rank on each locale
  TEST_TOPOLOGY(subtopology,(unsigned)-1);

  fprintf(stderr,"\n");fprintf(stderr,"global subTopologyNthGlobal(1) - probably not supported on XMI_LIST_TOPOLOGY?\n");
  __global.topology_global.subTopologyNthGlobal(&subtopology, 1); //1st rank on each locale
  TEST_TOPOLOGY(subtopology,(unsigned)-1);

  fprintf(stderr,"\n");fprintf(stderr,"make a global from a list size(%zu)\n",gSize);
  new (&topology) XMI::Topology(gRankList, (gSize));
  TEST_TOPOLOGY(topology,gSize);

  fprintf(stderr,"\n");fprintf(stderr,"copy the topology\n");
  copy_topology = topology;
  TEST_TOPOLOGY(copy_topology,topology.size());

  fprintf(stderr,"\n");fprintf(stderr,"make a global from a list: coverted to coord\n");
  topology.convertTopology(XMI_COORD_TOPOLOGY);
  TEST_TOPOLOGY(topology,gSize);

  fprintf(stderr,"\n");fprintf(stderr,"make a global from a list: subTopologyNthGlobal(0)\n");
  topology.subTopologyNthGlobal(&subtopology, 0);
  TEST_TOPOLOGY(subtopology,(unsigned)-1);

  fprintf(stderr,"\n");fprintf(stderr,"make a global from a list: subTopologyNthGlobal(1)\n");
  topology.subTopologyNthGlobal(&subtopology, 1);
  TEST_TOPOLOGY(subtopology,(unsigned)-1);

  fprintf(stderr,"\n");fprintf(stderr,"make a global from a list: coord coverted back to list\n");
  topology.convertTopology(XMI_LIST_TOPOLOGY);
  TEST_TOPOLOGY(topology,gSize);

  fprintf(stderr,"\n");fprintf(stderr,"make a global with last 1/2 list\n");
  new (&topology) XMI::Topology(gRankList+(gSize/2), (gSize/2)); // everyone except 0
  TEST_TOPOLOGY(topology,(gSize/2));

  fprintf(stderr,"\n");fprintf(stderr,"make a global with last 1/2 list: coverted to coord\n");
  topology.convertTopology(XMI_COORD_TOPOLOGY);
  TEST_TOPOLOGY(topology,(gSize/2));

  fprintf(stderr,"\n");fprintf(stderr,"make a global with last 1/2 list: subTopologyNthGlobal(0)\n");
  topology.subTopologyNthGlobal(&subtopology, 0);
  TEST_TOPOLOGY(subtopology,(unsigned)-1);

  fprintf(stderr,"\n");fprintf(stderr,"make a global with last 1/2 list: subTopologyNthGlobal(1)\n");
  topology.subTopologyNthGlobal(&subtopology, 1);
  TEST_TOPOLOGY(subtopology,(unsigned)-1);

  fprintf(stderr,"\n");fprintf(stderr,"make a global with last 1/2 list: coord coverted back to list\n");
  topology.convertTopology(XMI_LIST_TOPOLOGY);
  TEST_TOPOLOGY(topology,gSize/2);

  fprintf(stderr,"\n");fprintf(stderr,"make a nth topology and manually make a ranklist topology from it\n");
  new (&topology) XMI::Topology(gRankList, (gSize));
  topology.convertTopology(XMI_COORD_TOPOLOGY);
  topology.subTopologyNthGlobal(&subtopology, 0);

  xmi_task_t *ranklist = new xmi_task_t[subtopology.size()];

  // loop all global ranks, if they're in the subtopology, put then in a ranklist.
  for(size_t i = 0, j = 0; i < gSize; ++i)
  {
     if(subtopology.isRankMember(i)) ranklist[j++] = i;
     XMI_assert(j<=subtopology.size());
  }
  new (&topology) XMI::Topology(ranklist, subtopology.size());
  // See if we built a good list topology from the Nth global subtopology
  TEST_TOPOLOGY(topology,subtopology.size());

  struct{
    xmi_topology_t topology_t_array[11];
    XMI::Topology topology_array[11];
  } s;

  memset(s.topology_array, -1, (sizeof(XMI::Topology) * 11));
  for(size_t i = 0; i < 11; ++i)
    new (&s.topology_array[i]) XMI::Topology(gRankList, (gSize));
  memset(s.topology_t_array, -1, (sizeof(xmi_topology_t) * 11));
  for(size_t i = 0; i < 11; ++i)
    new (&s.topology_t_array[i]) XMI::Topology(gRankList, (gSize));

  fprintf(stderr,"\n");fprintf(stderr,"An array of XMI::Topology sizeof %zu * 11 = %zu\n",sizeof(XMI::Topology),sizeof(XMI::Topology)*11);
  for(size_t i = 0; i < 11; ++i)
    TEST_TOPOLOGY(s.topology_array[i],gSize);

  fprintf(stderr,"\n");fprintf(stderr,"An array of xmi_topology_t sizeof %zu * 11 = %zu\n",sizeof(xmi_topology_t),sizeof(xmi_topology_t)*11);
  for(size_t i = 0; i < 11; ++i)
    TEST_TOPOLOGY((*(XMI::Topology*)&s.topology_t_array[i]),gSize);

  for(size_t i = 0; i < 11; ++i)
    s.topology_array[i].convertTopology(XMI_COORD_TOPOLOGY);

  for(size_t i = 0; i < 11; ++i)
    ((XMI::Topology*)&s.topology_t_array[i])->convertTopology(XMI_COORD_TOPOLOGY);

  fprintf(stderr,"\n");fprintf(stderr,"An array of XMI::Topology sizeof %zu * 11 = %zu : converted to XMI_COORD_TOPOLOGY \n",sizeof(XMI::Topology),sizeof(XMI::Topology)*11);
  for(size_t i = 0; i < 11; ++i)
    TEST_TOPOLOGY(s.topology_array[i],gSize);

  fprintf(stderr,"\n");fprintf(stderr,"An array of xmi_topology_t sizeof %zu * 11 = %zu : converted to XMI_COORD_TOPOLOGY \n",sizeof(xmi_topology_t),sizeof(xmi_topology_t)*11);
  for(size_t i = 0; i < 11; ++i)
    TEST_TOPOLOGY((*(XMI::Topology*)&s.topology_t_array[i]),gSize);

  XMI_assertf(sizeof(xmi_topology_t) >= sizeof(XMI::Topology),"sizeof(xmi_topology_t) %zu >= %zu sizeof(XMI::Topology)\n",sizeof(xmi_topology_t),sizeof(XMI::Topology));
  fprintf(stderr,"\n");fprintf(stderr,"DONE\n");
}

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
