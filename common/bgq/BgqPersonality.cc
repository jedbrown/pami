/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common/bgq/BgqPersonality.cc
 * \brief ???
 */


#include "BgqPersonality.h"

#ifdef ENABLE_MAMBO_WORKAROUNDS
#include <hwi/include/bqc/nd_500_dcr.h>
#endif

PAMI::BgqPersonality::BgqPersonality ()
{
  Personality_t * p = (Personality_t *) this;

#ifdef ENABLE_MAMBO_WORKAROUNDS

  Kernel_GetPersonality(p, sizeof(Personality_t));

  _is_mambo = false; // Indicates whether mambo is being used

  char* var = getenv("BG_PROCESSESPERNODE");
  TRACE_MAMBO((stderr, "BG_PROCESSESPERNODE %s\n", var ? var : "NULL"));

  var = getenv("BG_SHAREDMEMSIZE");
  TRACE_MAMBO((stderr, "BG_SHAREDMEMSIZE %s\n", var ? var : "NULL"));


  TRACE_MAMBO((stderr, "BGQPersonality Kernel_Config.NodeConfig %#llX\n", (unsigned long long)(Kernel_Config.NodeConfig)));
  TRACE_MAMBO((stderr, "BGQPersonality Kernel_Config.TraceConfig %#llX\n", (unsigned long long)(Kernel_Config.TraceConfig)));
  TRACE_MAMBO((stderr, "BGQPersonality Network_Config.MuFlags %#llX\n", (unsigned long long)(Network_Config.MuFlags)));
  TRACE_MAMBO((stderr, "BGQPersonality Network_Config.NetFlags %#llX\n", (unsigned long long)(Network_Config.NetFlags)));

  uint64_t p1 = Kernel_Config.NodeConfig & PERS_ENABLE_Mambo;

  if (p1) _is_mambo = true;

  TRACE_MAMBO((stderr, "BGQPersonality is mambo enabled: %s\n", _is_mambo ? "true" : "false"));

  TRACE_MAMBO((stderr, "Network_Config Coord A = %#x\n", Network_Config.Acoord));
  TRACE_MAMBO((stderr, "Network_Config Coord B = %#x\n", Network_Config.Bcoord));
  TRACE_MAMBO((stderr, "Network_Config Coord C = %#x\n", Network_Config.Ccoord));
  TRACE_MAMBO((stderr, "Network_Config Coord D = %#x\n", Network_Config.Dcoord));
  TRACE_MAMBO((stderr, "Network_Config Coord E = %#x\n", Network_Config.Ecoord));

  TRACE_MAMBO((stderr, "Network_Config Nodes A = %#x\n", Network_Config.Anodes));
  TRACE_MAMBO((stderr, "Network_Config Nodes B = %#x\n", Network_Config.Bnodes));
  TRACE_MAMBO((stderr, "Network_Config Nodes C = %#x\n", Network_Config.Cnodes));
  TRACE_MAMBO((stderr, "Network_Config Nodes D = %#x\n", Network_Config.Dnodes));
  TRACE_MAMBO((stderr, "Network_Config Nodes E = %#x\n", Network_Config.Enodes));

  if (_is_mambo)
    {
      unsigned dcr_num = ND_500_DCR_base + ND_500_DCR__CTRL_COORDS_offset;

      unsigned long long dcr = DCRReadUser(dcr_num);

      TRACE_MAMBO((stderr, "BGQPersonality() NODE_COORDINATES DCR = 0x%016llx\n", dcr));

#if 0 // no longer need to set personality from ND 500 if using runmm correctly
      Network_Config.Acoord = ND_500_DCR__CTRL_COORDS__NODE_COORD_A_get(dcr);
      Network_Config.Bcoord = ND_500_DCR__CTRL_COORDS__NODE_COORD_B_get(dcr);
      Network_Config.Ccoord = ND_500_DCR__CTRL_COORDS__NODE_COORD_C_get(dcr);
      Network_Config.Dcoord = ND_500_DCR__CTRL_COORDS__NODE_COORD_D_get(dcr);
      Network_Config.Ecoord = ND_500_DCR__CTRL_COORDS__NODE_COORD_E_get(dcr);

      Network_Config.Anodes = ND_500_DCR__CTRL_COORDS__MAX_COORD_A_get(dcr) + 1;
      Network_Config.Bnodes = ND_500_DCR__CTRL_COORDS__MAX_COORD_B_get(dcr) + 1;
      Network_Config.Cnodes = ND_500_DCR__CTRL_COORDS__MAX_COORD_C_get(dcr) + 1;
      Network_Config.Dnodes = ND_500_DCR__CTRL_COORDS__MAX_COORD_D_get(dcr) + 1;
      Network_Config.Enodes = ND_500_DCR__CTRL_COORDS__MAX_COORD_E_get(dcr) + 1;
#endif

      TRACE_MAMBO((stderr, "ND_500_DCR__CTRL_COORDS__NODE_COORD_A = %#llx\n", ND_500_DCR__CTRL_COORDS__NODE_COORD_A_get(dcr)));
      TRACE_MAMBO((stderr, "ND_500_DCR__CTRL_COORDS__NODE_COORD_B = %#llx\n", ND_500_DCR__CTRL_COORDS__NODE_COORD_B_get(dcr)));
      TRACE_MAMBO((stderr, "ND_500_DCR__CTRL_COORDS__NODE_COORD_C = %#llx\n", ND_500_DCR__CTRL_COORDS__NODE_COORD_C_get(dcr)));
      TRACE_MAMBO((stderr, "ND_500_DCR__CTRL_COORDS__NODE_COORD_D = %#llx\n", ND_500_DCR__CTRL_COORDS__NODE_COORD_D_get(dcr)));
      TRACE_MAMBO((stderr, "ND_500_DCR__CTRL_COORDS__NODE_COORD_E = %#llx\n", ND_500_DCR__CTRL_COORDS__NODE_COORD_E_get(dcr)));

      TRACE_MAMBO((stderr, "ND_500_DCR__CTRL_COORDS__MAX_COORD_A = %#llx\n", ND_500_DCR__CTRL_COORDS__MAX_COORD_A_get(dcr)));
      TRACE_MAMBO((stderr, "ND_500_DCR__CTRL_COORDS__MAX_COORD_B = %#llx\n", ND_500_DCR__CTRL_COORDS__MAX_COORD_B_get(dcr)));
      TRACE_MAMBO((stderr, "ND_500_DCR__CTRL_COORDS__MAX_COORD_C = %#llx\n", ND_500_DCR__CTRL_COORDS__MAX_COORD_C_get(dcr)));
      TRACE_MAMBO((stderr, "ND_500_DCR__CTRL_COORDS__MAX_COORD_D = %#llx\n", ND_500_DCR__CTRL_COORDS__MAX_COORD_D_get(dcr)));
      TRACE_MAMBO((stderr, "ND_500_DCR__CTRL_COORDS__MAX_COORD_E = %#llx\n", ND_500_DCR__CTRL_COORDS__MAX_COORD_E_get(dcr)));

      PAMI_assertf(Network_Config.Acoord < Network_Config.Anodes, "assert Network_Config.Acoord(%#X) < Network_Config.Anodes(%#X)\n", Network_Config.Acoord, Network_Config.Anodes);
      PAMI_assertf(Network_Config.Bcoord < Network_Config.Bnodes, "assert Network_Config.Bcoord(%#X) < Network_Config.Bnodes(%#X)\n", Network_Config.Bcoord, Network_Config.Bnodes);
      PAMI_assertf(Network_Config.Ccoord < Network_Config.Cnodes, "assert Network_Config.Ccoord(%#X) < Network_Config.Cnodes(%#X)\n", Network_Config.Ccoord, Network_Config.Cnodes);
      PAMI_assertf(Network_Config.Dcoord < Network_Config.Dnodes, "assert Network_Config.Dcoord(%#X) < Network_Config.Dnodes(%#X)\n", Network_Config.Dcoord, Network_Config.Dnodes);
      PAMI_assertf(Network_Config.Ecoord < Network_Config.Enodes, "assert Network_Config.Ecoord(%#X) < Network_Config.Enodes(%#X)\n", Network_Config.Ecoord, Network_Config.Enodes);
    }

#else // no mambo workarounds..

  if (Kernel_GetPersonality(p, sizeof(Personality_t)) != 0)
    PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);

#endif

#ifdef FAKE_PERSONALITY
#warning FAKE PERSONALITY ?
  Kernel_GetPersonality(p, sizeof(Personality_t));

  Network_Config.Anodes = 1;
  Network_Config.Bnodes = 1;
  Network_Config.Cnodes = 1;
  Network_Config.Dnodes = 1;
  Network_Config.Enodes = 1;

  Network_Config.Acoord = 0;
  Network_Config.Bcoord = 0;
  Network_Config.Ccoord = 0;
  Network_Config.Dcoord = 0;
  Network_Config.Ecoord = 0;
#endif

  //size_t num_local_processes = Kernel_ProcessCount();

  // Set the hw core id of this process [0..16]
  _core = Kernel_PhysicalProcessorID();

  // get the number of active processes (peers) on this node [0..(p x t)]
  size_t pCount = Kernel_ProcessCount();

  // Set the number of active hardware threads on this node [0..3]
  //_hwthreads = Kernel_PhysicalHWThreadID();
  //_hwthreads = 1;
  _hwthreads = (pCount + maxCores() - 1) / maxCores();

  // Set the hardware thread id of this process [0..hwthreads]
  _hwthread = Kernel_PhysicalHWThreadID();

  // Set the number of active cores on this node [0..16]
  _cores = pCount > maxCores() ? maxCores() : pCount;

  // Set the id of this process [0..cores]
  _pCoord = core() / (maxCores() / pSize());

  // Set the id of this thread [0..tSize]
  _tCoord = _hwthread / (maxThreads() / _hwthreads);

  TRACE_MAMBO((stderr, "BGQPersonality() tid %zu, p %zu, t %zu, core %zu, thread %zu, pSize %zu, tSize %zu\n", tid(),  pCoord(),  tCoord(),  core(), thread(), pSize(), tSize()));

  _torusA = (bool) (ND_ENABLE_TORUS_DIM_A & Network_Config.NetFlags);
  _torusB = (bool) (ND_ENABLE_TORUS_DIM_B & Network_Config.NetFlags);
  _torusC = (bool) (ND_ENABLE_TORUS_DIM_C & Network_Config.NetFlags);
  _torusD = (bool) (ND_ENABLE_TORUS_DIM_D & Network_Config.NetFlags);
  _torusE = (bool) (ND_ENABLE_TORUS_DIM_E & Network_Config.NetFlags);

  TRACE_MAMBO((stderr, "BGQPersonality() _torusA %d, _torusB %d, _torusC %d, _torusD %d, _torusE %d\n", _torusA, _torusB, _torusC, _torusD, _torusE));

};

void PAMI::BgqPersonality::location (char location[], size_t size)
{
#if 0
  _BGP_Personality_t * p = (_BGP_Personality_t *) this;
  char tmp[BGPPERSONALITY_MAX_LOCATION+1];
  BGP_Personality_getLocationString(p, tmp);

  snprintf (location, MIN(size, BGPPERSONALITY_MAX_LOCATION), " %s", tmp);

  return;
#endif
};

void PAMI::BgqPersonality::dumpPersonality ()
{
#if 0
  _BGP_Personality_t * p = (_BGP_Personality_t *) this;
  int i, tmp;

  printf("Personality: CRC 0x %04x, Version %d, Size %d.\n",
         p->CRC, p->Version, p->PersonalitySizeWords );

  printf(" BlockID:         0x %08x\n", p->Network_Config.BlockID );
  printf(" RASPolicy:       0x %08x\n", p->Kernel_Config.RASPolicy );
  printf(" ProcessConfig:   0x %08x\n", p->Kernel_Config.ProcessConfig );
  printf(" TraceConfig:     0x %08x\n", p->Kernel_Config.TraceConfig );
  printf(" NodeConfig:      0x %08x\n", p->Kernel_Config.NodeConfig );
  printf(" L1Config:        0x %08x\n", p->Kernel_Config.L1Config );
  printf(" L2Config:        0x %08x\n", p->Kernel_Config.L2Config );
  printf(" L3Config:        0x %08x\n", p->Kernel_Config.L3Config );
  printf(" L3Select:        0x %08x\n", p->Kernel_Config.L3Select );

  printf(" FreqMHz:         %d\n",      p->Kernel_Config.FreqMHz );

  printf(" Torus:         ( %2d, %2d, %2d) in ( %2d %c, %2d %c, %2d %c)\n",
         p->Network_Config.Xcoord,
         p->Network_Config.Ycoord,
         p->Network_Config.Zcoord,
         p->Network_Config.Xnodes,
         (p->Kernel_Config.NodeConfig & _BGP_PERS_ENABLE_TorusMeshX ? 'M' : 'T'),
         p->Network_Config.Ynodes,
         (p->Kernel_Config.NodeConfig & _BGP_PERS_ENABLE_TorusMeshY ? 'M' : 'T'),
         p->Network_Config.Znodes,
         (p->Kernel_Config.NodeConfig & _BGP_PERS_ENABLE_TorusMeshZ ? 'M' : 'T'));

  printf(" IOnodes:         %d\n", p->Network_Config.IOnodes );
  printf(" Rank:         %d\n", p->Network_Config.Rank );

  for (i = 0, tmp = 0 ; i < 16 ; i++)
    {
      if (p->Network_Config.TreeRoutes[i])
        {
          tmp++;
          printf(" TreeRoutes[%d]: 0x %04x\n",
                 i, p->Network_Config.TreeRoutes[i] );
        }
    }

  if (!tmp)
    printf(" TreeRoutes:           (none defined)\n" );


  printf(" DDRSizeMB:         %d\n",      p->DDR_Config.DDRSizeMB   );
  printf(" DDRChips:        0x %02x\n", p->DDR_Config.Chips    );
  printf(" DDRCAS:         %d\n",      p->DDR_Config.CAS      );
  printf(" DDRThrottle:         %d %% \n",    p->DDR_Config.Throttle );
  printf(" MTU:         %d\n", p->Ethernet_Config.MTU );

  printf(" EmacID:         %02x:%02x:%02x:%02x:%02x:%02x\n",
         p->Ethernet_Config.EmacID[0],
         p->Ethernet_Config.EmacID[1],
         p->Ethernet_Config.EmacID[2],
         p->Ethernet_Config.EmacID[3],
         p->Ethernet_Config.EmacID[4],
         p->Ethernet_Config.EmacID[5] );

  printf(" IPAddress:         %d.%d.%d.%d\n",
         p->Ethernet_Config.IPAddress.octet[12],
         p->Ethernet_Config.IPAddress.octet[13],
         p->Ethernet_Config.IPAddress.octet[14],
         p->Ethernet_Config.IPAddress.octet[15]  );

  printf(" IPNetmask:         %d.%d.%d.%d\n",
         p->Ethernet_Config.IPNetmask.octet[12],
         p->Ethernet_Config.IPNetmask.octet[13],
         p->Ethernet_Config.IPNetmask.octet[14],
         p->Ethernet_Config.IPNetmask.octet[15]  );

  printf(" IPBroadcast:       %d.%d.%d.%d\n",
         p->Ethernet_Config.IPBroadcast.octet[12],
         p->Ethernet_Config.IPBroadcast.octet[13],
         p->Ethernet_Config.IPBroadcast.octet[14],
         p->Ethernet_Config.IPBroadcast.octet[15]  );

  printf(" IPGateway:         %d.%d.%d.% d\n",
         p->Ethernet_Config.IPGateway.octet[12],
         p->Ethernet_Config.IPGateway.octet[13],
         p->Ethernet_Config.IPGateway.octet[14],
         p->Ethernet_Config.IPGateway.octet[15]  );

  printf(" NFSServer:         %d.%d.%d.%d\n",
         p->Ethernet_Config.NFSServer.octet[12],
         p->Ethernet_Config.NFSServer.octet[13],
         p->Ethernet_Config.NFSServer.octet[14],
         p->Ethernet_Config.NFSServer.octet[15]  );

  printf(" serviceNode:       %d.%d.%d.%d\n",
         p->Ethernet_Config.serviceNode.octet[12],
         p->Ethernet_Config.serviceNode.octet[13],
         p->Ethernet_Config.serviceNode.octet[14],
         p->Ethernet_Config.serviceNode.octet[15]  );

  printf(" NFSExportDir:         > %s < \n", p->Ethernet_Config.NFSExportDir );
  printf(" NFSMountDir:          > %s < \n", p->Ethernet_Config.NFSMountDir  );

  return;
#endif
};
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
