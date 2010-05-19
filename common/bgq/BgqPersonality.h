/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __common_bgq_BgqPersonality_h__
#define __common_bgq_BgqPersonality_h__
///
///  \file common/bgq/BgqPersonality.h
///  \brief Blue Gene/Q Personality Object
///
///  This object currently provides the following features:
///  - Access to coordinate information
///  - Access to partition size information
///  - Access to operating modes (vnm, etc)
///  - Access to pset information
///  - Ability to dump the entire personality
///
///  Definitions:
///  - Personality is an object that returns machine specific hardware info
///

#define PERS_SIZE 1024

#include <sys/types.h>
#include <util/common.h>

#include <unistd.h>
#include <stdlib.h>

#include <firmware/include/personality.h>
#include <kernel/location.h>
#include <kernel/process.h>

#ifdef ENABLE_MAMBO_WORKAROUNDS
#undef TRACE_MAMBO
#define TRACE_MAMBO(x) //fprintf x
#else
#undef TRACE_MAMBO
#define TRACE_MAMBO(x)
#endif

//#define FAKE_PERSONALITY
#ifdef ENABLE_MAMBO_WORKAROUNDS
#include <hwi/include/bqc/nd_500_dcr.h>
#endif

namespace PAMI
{
  class BgqPersonality : public Personality_t
  {
    public:

      BgqPersonality ()
      {
        Personality_t * p = (Personality_t *) this;

#ifdef ENABLE_MAMBO_WORKAROUNDS

        Kernel_GetPersonality(p, sizeof(Personality_t));

        _is_mambo = false; // Indicates whether mambo is being used

        char* var = getenv("BG_PROCESSESPERNODE");
        TRACE_MAMBO((stderr, "BG_PROCESSESPERNODE %s\n", var ? var : "NULL"));

        var = getenv("BG_SHAREDMEMSIZE");
        TRACE_MAMBO((stderr, "BG_SHAREDMEMSIZE %s\n", var ? var : "NULL"));

        var = getenv("BG_MEMSIZE");
        TRACE_MAMBO((stderr, "BG_MEMSIZE %s\n", var ? var : "NULL"));

	// this seems really lame...
        if (Network_Config.Anodes == 0) Network_Config.Anodes = 1;
        if (Network_Config.Bnodes == 0) Network_Config.Bnodes = 1;
        if (Network_Config.Cnodes == 0) Network_Config.Cnodes = 1;
        if (Network_Config.Dnodes == 0) Network_Config.Dnodes = 1;
        if (Network_Config.Enodes == 0) Network_Config.Enodes = 1;

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

            // funky code to avoid "unused variable" warning when TRACE_MAMBO is off.
            unsigned long long dcr = 0;
            dcr = DCRReadUser(dcr_num);

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

      void location (char location[], size_t size)
      {
      };

      void dumpPersonality ()
      {
      };

      ///
      /// \brief Retrieves the rank of the task in this job.
      ///
      /// deprecated - Rank is not part of the personality
      //size_t rank() const { return Network_Config.Rank; }


      ///
      /// \brief Retrieves the 'A' coordinate of the node
      ///
      size_t aCoord() const { return Network_Config.Acoord; }

      ///
      /// \brief Retrieves the 'B' coordinate of the node
      ///
      size_t bCoord() const { return Network_Config.Bcoord; }

      ///
      /// \brief Retrieves the 'C' coordinate of the node
      ///
      size_t cCoord() const { return Network_Config.Ccoord; }

      ///
      /// \brief Retrieves the 'D' coordinate of the node
      ///
      size_t dCoord() const { return Network_Config.Dcoord; }

      ///
      /// \brief Retrieves the 'E' coordinate of the node
      ///
      size_t eCoord() const { return Network_Config.Ecoord; }

      ///
      /// \brief Retrieves the core id of the node
      ///
      ///
      size_t core() const { return _core; }

      ///
      /// \brief Retrieves the 'P' coordinate of the node
      ///
      /// a.k.a. the processor id starting at 0 and
      /// incrementing to pSize. Not the same as hardware core!
      ///
      size_t pCoord() const { return _pCoord; }

      ///
      /// \brief Retrieves the 'T' coordinate of the node
      ///
      /// a.k.a. the thread id on the core starting at 0 and
      /// incrementing to tSize.  Not the same as hwThread!
      ///
      size_t tCoord() const { return _tCoord; }

      ///
      /// \brief Retrieves the 'hardware thread id' on the core
      ///
      size_t thread() const { return _hwthread; }

      ///
      /// \brief Retrieves the 'thread id' on the node
      ///
      size_t tid() const { return thread() + core()*maxThreads(); }

      ///
      /// \brief Retrieves the size of the 'A' dimension.
      /// \note  Does not consider the mapping.
      ///
      size_t aSize()  const { return Network_Config.Anodes; }

      ///
      /// \brief Retrieves the size of the 'B' dimension.
      /// \note  Does not consider the mapping.
      ///
      size_t bSize()  const { return Network_Config.Bnodes; }

      ///
      /// \brief Retrieves the size of the 'C' dimension.
      /// \note  Does not consider the mapping.
      ///
      size_t cSize()  const { return Network_Config.Cnodes; }

      ///
      /// \brief Retrieves the size of the 'D' dimension.
      /// \note  Does not consider the mapping.
      ///
      size_t dSize()  const { return Network_Config.Dnodes; }

      ///
      /// \brief Retrieves the size of the 'E' dimension.
      /// \note  Does not consider the mapping.
      ///
      size_t eSize()  const { return Network_Config.Enodes; }

      ///
      /// \brief Retrieves the size of the 'P' dimension.
      ///
      /// The 'P' coordinate identifies the core
      /// starting at 0 and incrementing to pSize.
      ///
      size_t pSize()  const { return _cores; }

      ///
      /// \brief Retrieves the max number of cores, not the number active
      ///
      size_t maxCores() const { return 16; }/// \todo max cores == 16?

      ///
      /// \brief Retrieves the max number of threads, not the number active
      ///
      size_t maxThreads() const { return 4; }/// \todo max threads == 4?

      ///
      /// \brief Retrieves the size of the 'T' dimension.
      ///
      /// The 'T' coordinate identifies the hardware thread.
      ///
      /// \note  Does not consider the mapping.
      ///
      size_t tSize()  const { return _hwthreads; }

      ///
      /// \brief Get the size of NODE memory
      /// \return _node_ memory size in MiB
      ///
      size_t  memSize()  const { return DDR_Config.DDRSizeMB;}

      ///
      /// \brief Gets the clock speed in MEGA-Hz
      /// \return MHz
      ///
      size_t  clockMHz()  const { return Kernel_Config.FreqMHz; }

    protected:

      size_t _tCoord;
      size_t _pCoord;
      size_t _core;
      size_t _hwthread;

      bool   _torusA;
      bool   _torusB;
      bool   _torusC;
      bool   _torusD;
      bool   _torusE;
#ifdef ENABLE_MAMBO_WORKAROUNDS
    public:
      bool   _is_mambo; // Indicates whether mambo is being used
    protected:
#endif

      size_t _cores;
      size_t _hwthreads;
  };	// class BgqPersonality
};	// namespace PAMI


//PAMI::BgqPersonality::BgqPersonality ()
//{
//};
#if 0
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
#endif
#endif // __pami_components_sysdep_bgq_bgqpersonnality_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
