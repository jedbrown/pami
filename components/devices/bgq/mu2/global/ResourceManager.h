/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

////////////////////////////////////////////////////////////////////////////////
///
/// \file components/devices/bgq/mu2/global/ResourceManager.h
///
/// \brief MU Resource Manager Definitions
///
/// The MU Resource Manager controls resources associated with the MU Device.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef __components_devices_bgq_mu2_global_ResourceManager_h__
#define __components_devices_bgq_mu2_global_ResourceManager_h__

#ifdef __FWEXT__

#include <firmware/include/fwext/fwext.h>
#include <firmware/include/Firmware.h>
#include <hwi/include/bqc/A2_core.h>
#include <hwi/include/bqc/A2_inlines.h>
#include <firmware/include/fwext/fwext_lib.h>
#include <firmware/include/fwext/fwext_nd.h>
#include <firmware/include/fwext/fwext_mu.h>

#endif // __FWEXT__

#include <stdio.h>
#include <stdlib.h>
#include <hwi/include/bqc/MU_Macros.h>
#include <hwi/include/bqc/classroute.h>
#include <spi/include/mu/InjFifo.h>
#include <spi/include/mu/RecFifo.h>
#include <spi/include/mu/Addressing.h>
#include <spi/include/mu/Addressing_inlines.h>
#include <spi/include/kernel/MU.h>
#include <common/bgq/ResourceManager.h>
#include <common/bgq/BgqPersonality.h>
#include <components/devices/bgq/mu2/InjGroup.h>
#include <components/devices/generic/Device.h>

#include "algorithms/geometry/Geometry.h"
#include "components/atomic/bgq/L2Mutex.h"
#include "components/devices/misc/AtomicMutexMsg.h"
typedef PAMI::Mutex::BGQ::L2ProcMutex MUCR_mutex_t;
typedef PAMI::Device::SharedAtomicMutexMdl<MUCR_mutex_t> MUCR_mutex_model_t;
#include <spi/include/kernel/collective.h>
#include <spi/include/mu/Classroute_inlines.h>
#include <spi/include/kernel/gi.h>

#ifdef TRACE
#undef TRACE
#endif
#define TRACE(x) //fprintf x

#ifndef abs_x
#define abs_x(x) ((x^(x>>31)) - (x>>31))
#endif

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      static const size_t numTorusDirections           = BGQ_TDIMS<<1; // 10 directions
      static const size_t numFifoPinIndices            = 16;
      static const size_t optimalNumInjFifosPerContext = numTorusDirections;
      static const size_t optimalNumRecFifosPerContext = 1;

      ///
      /// \brief Point-to-Point Injection Fifo Pin Map Values
      ///
      /// There are 10 pinInfoEntries, one for each "number of inj fifos in the context".
      /// The 1st entry is for when there is 1 inj fifo in the context.
      /// The 2nd entry is for when there are 2 inj fifos in the context.
      /// ...
      /// The 10th entry is for when there are 10 inj fifos in the context.
      ///
      /// Within each entry, there are two arrays of 16 elements each.
      /// The torusInjFifoMaps array contains torus inj fifo map bits, 10
      /// for the torus directions, and 6 local transfers.
      /// The injFifoIds array contains inj fifo numbers to be used for the 10
      /// torus directions and 6 locals.
      /// The reason these are paired like this is to optimize context
      /// pinFifo()'s access to this info.  We want all of this info to be in
      /// one L2 cache line.  Each context will "get" one of these pairs
      /// and use it in its pinFifo().
      ///
      /// The Context PinFifo function chooses an inj fifo (0..9) that is
      /// optimal for sending to the destination, assuming there are 10
      /// inj fifos in the context.  Then, it indexes into
      /// the appropriate injFifoIds array, based on how many inj fifos are actually in
      /// the context, to get the actual fifo number to pin to.  It then indexes
      /// into the torusInjFifoMaps array to get that value for the descriptor.
      ///
      /// For torus transfers, the array index is 0..9, corresponding to A-,A+,...,E-,E+.
      /// For local transfers, the array index is 10..15, derived from the destination's
      /// T coordinate (to spread the local transfers among as many of the actual
      /// imFifos fifos as possible.
      /// In either case, the value in the injFifoIds array at that index is the inj fifo
      /// number to use to inject a descriptor for that transfer.
      ///
      /// One additional twist:
      /// For the 10 torus imFifos (1st 10 array elements), the A- maps to element
      /// 0, A+ to element 1, etc.  But, when there are less than 10 actual imFifos,
      /// the remaining directions must map to imFifos that have already been
      /// used for a direction.  We want this to be spread evenly so that all imFifos
      /// are kept evenly busy, so the values in the array will be adjusted during
      /// runtime initialization based on our T coordinate.
      ///
      /// For example, if there are 8 actual imFifos, the first 8 fifopin array values
      /// would be 0,1,2,3,4,5,6,7, saying that for messages traveling in the
      /// A-,A+,B-,B+,C-,C+,D-,D+ directions, those imFifos will be used.
      /// But, for messages needing to travel in the E- or E+ direction, which
      /// imFifos do we use?  We set those 2 values to 0 and 1, implying that
      /// imFifos 0 and 1 will take those messages.  But, this puts added pressure
      /// on imFifos 0 and 1.  To alleviate this, those 2 values will be
      /// adjusted based on our T coordinate.  We add our 2*T coordinate (2 being
      /// the number of over-stressed fifos) to the values in the table (0,1)
      /// and mod it with the number of fifos (8) to give a value in the range 0..7,
      /// depending on our T.  Thus, T=0 will use imFifos 0 and 1, while T=1 will
      /// use imFifos 2 and 3, and so on.
      ///
      /// For the last 6 elements of the injFifoIds array (for local transfers), we do a
      /// similar distribution among 6 of the imFifos.  Note we are limited to
      /// 6 for local transfers since there are only 6 more slots in the 16 element
      /// array left for us to use...the fifoPin value we store in the map cache
      /// can only be in the range 0..15.  We try to evenly distribute the local
      /// traffic among all of the imFifos so that the imFifos chosen for local
      /// traffic in each process are not always the same, thereby more evenly
      /// impacting the effect of local traffic across all of the directions.
      /// So, we adjust the last 6 values in the array by adding 6*T to each array
      /// element and then mod'ing the result with #fifos.  For example, if there
      /// are 10 imFifos, the array elements initially are set to 0,1,2,3,4,5.
      /// For a process with T=0, those are the imFifos that are used to send
      /// local messages to the different T destinations.  For a process with T=1,
      /// they are set to 6,7,8,9,0,1.
      ///
      // There will be numTorusDirections of these structures packed in an array...
      typedef struct pinInfoEntry
      {
	uint16_t injFifoIds[numFifoPinIndices];
	uint16_t torusInjFifoMaps[numFifoPinIndices];
      } __attribute__((__packed__)) pinInfoEntry_t;

      // Later, this will be used to initialize the structures by copying it into them...
      static const uint16_t pinInfo[2*numTorusDirections][numFifoPinIndices] =
	{
	  // 1 inj fifo in the context
	  { 0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0 },   // injFifoIds array
	  { MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_BM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_BP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_DM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_DP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1 }, // torus inj fifo map array
	  // 2 inj fifo in the context
	  { 0,1,0,1,0,1,0,1,0,1,  0,1,0,1,0,1 },   // injFifoIds array
	  { MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_BM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_BP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_DM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_DP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1 }, // torus inj fifo map array
	  // 3 inj fifo in the context
	  { 0,1,2,0,1,2,0,1,2,0,  1,2,0,1,2,0 },   // injFifoIds array
	  { MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_BM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_BP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_DM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_DP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1 }, // torus inj fifo map array
	  // 4 inj fifo in the context
	  { 0,1,2,3,0,1,2,3,0,1,  2,3,0,1,2,3 },   // injFifoIds array
	  { MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_BM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_BP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_DM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_DP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1 }, // torus inj fifo map array
	  // 5 inj fifo in the context
	  { 0,1,2,3,4,0,1,2,3,4,  0,1,2,3,4,0 },   // injFifoIds array
	  { MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_BM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_BP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_DM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_DP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1 }, // torus inj fifo map array
	  // 6 inj fifo in the context
	  { 0,1,2,3,4,5,0,1,2,3,  4,5,0,1,2,3 },   // injFifoIds array
	  { MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_BM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_BP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_DM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_DP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1 }, // torus inj fifo map array
	  // 7 inj fifo in the context
	  { 0,1,2,3,4,5,6,0,1,2,  3,4,5,6,0,1 },   // injFifoIds array
	  { MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_BM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_BP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_DM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_DP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1 }, // torus inj fifo map array
	  // 8 inj fifo in the context
	  { 0,1,2,3,4,5,6,7,0,1,  2,3,4,5,6,7 },   // injFifoIds array
	  { MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_BM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_BP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_DM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_DP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1 }, // torus inj fifo map array
	  // 9 inj fifo in the context
	  { 0,1,2,3,4,5,6,7,8,0,  1,2,3,4,5,6 },   // injFifoIds array
	  { MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_BM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_BP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_DM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_DP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1 }, // torus inj fifo map array
	  // 10 inj fifo in the context
	  { 0,1,2,3,4,5,6,7,8,9,  0,1,2,3,4,5 },    // injFifoIds array
	  { MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_BM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_BP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_DM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_DP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EM,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_EP,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL0,
	    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_LOCAL1 } // torus inj fifo map array
	};

      ///
      /// \brief Point-to-Point Broadcast Fifo Pin Map Values
      ///
      /// There are 10 arrays of 16 elements each.
      /// The 1st array is for when there is 1 inj fifo in the context.
      /// The 2nd array is for when there are 2 inj fifos in the context.
      /// ...
      /// The 10th array is for when there are 10 inj fifos in the context.
      ///
      /// The second dimension is indexed by the collective class route Id (0..15)
      /// to select the injection fifo to use for this class route.
      ///
      /// Note that the values in the slots beyond the actual number of fifos
      /// are adjusted during runtime initialization based on our T coordinate,
      /// in order to more evenly distribute the impact of these broadcasts across
      /// the imFifos.
      ///
      static const uint16_t pinBroadcastFifoMap[numTorusDirections][BGQ_COLL_CLASS_MAX_CLASSROUTES] =
	{ { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },   // 1 Inj fifo in the context
	  { 0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1 },   // 2 Inj fifo in the context
	  { 0,1,2,0,1,2,0,1,2,0,1,2,0,1,2,0 },   // 3 Inj fifo in the context
	  { 0,1,2,3,0,1,2,3,0,1,2,3,0,1,2,3 },   // 4 Inj fifo in the context
	  { 0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0 },   // 5 Inj fifo in the context
	  { 0,1,2,3,4,5,0,1,2,3,4,5,0,1,2,3 },   // 6 Inj fifo in the context
	  { 0,1,2,3,4,5,6,0,1,2,3,4,5,6,0,1 },   // 7 Inj fifo in the context
	  { 0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7 },   // 8 Inj fifo in the context
	  { 0,1,2,3,4,5,6,7,8,0,1,2,3,4,5,6 },   // 9 Inj fifo in the context
	  { 0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5 } }; // 10 Inj fifos in the context

      typedef struct muResources
      {
	size_t numInjFifos;
	size_t numRecFifos;
      } muResources_t;

      typedef struct pamiResources
      {
	size_t numContexts;
      } pamiResources_t;

      typedef struct injFifoResources
      {
	MUSPI_InjFifoSubGroup_t  *subgroups;
	char                    **fifoPtrs;
	uint32_t                 *globalFifoIds;
	char                    **lookAsidePayloadPtrs;
	uint64_t                 *lookAsidePayloadPAs;
	Kernel_MemoryRegion_t    *lookAsidePayloadMemoryRegions;
	pami_event_function     **lookAsideCompletionFnPtrs;
	void                   ***lookAsideCompletionCookiePtrs;
      } injFifoResources_t;

      typedef struct recFifoResources
      {
	MUSPI_RecFifoSubGroup_t  *subgroups;
	char                    **fifoPtrs;
	uint32_t                 *globalFifoIds;
      } recFifoResources_t;

      typedef struct clientResources
      {
	size_t              numContexts;
	uint32_t           *startingSubgroupIds; // One entry per context
	uint32_t           *endingSubgroupIds;   // One entry per context
	injFifoResources_t *injResources;        // One entry per context
	recFifoResources_t *recResources;        // One entry per context
	uint16_t           *pinRecFifo;          // 2D array [tcoord][contextOffset] containing
	                                         // globalRecFifoId.
      } clientResources_t;


      class ResourceManager
      {
      public:

	//////////////////////////////////////////////////////////////////////////
	///
	/// \brief PAMI Resource Manager Default Constructor
	///
	/////////////////////////////////////////////////////////////////////////
	ResourceManager ( PAMI::ResourceManager &pamiRM,
			  PAMI::Mapping         &mapping,
			  PAMI::BgqPersonality  &pers,
			  PAMI::Memory::MemoryManager   &mm )  :
	  _pamiRM( pamiRM ),
	  _mapping( mapping ),
	  _pers( pers ),
	  _tSize( mapping.tSize() ),
	  _perProcessMaxPamiResources( NULL ),
	  _perProcessOptimalPamiResources( NULL ),
	  _globalRecSubGroups( NULL )
	{
	 // Only initialize global MU resources if we are using the MU
	 // Otherwise, machine checks will result when touching the
	 // MU MMIO storage.
	 if ( __global.useMU() )
	 {
	  // For each task, cache the optimal fifo pin in the mapcache.
	  initFifoPin();

	  // Calculate the following for each client
	  calculatePerProcessMaxPamiResources();
	  calculatePerProcessOptimalPamiResources();
	  calculatePerCorePerProcessPerClientMUResources();

	  // Set up the global resources
	  allocateGlobalResources();
	  TRACE((stderr,"MU ResourceManager: Done allocating global resources\n"));

	  // might want more shmem here, to use for coordinating locals in VN.
	  // possibly changing this to a structure.
	  mm.memalign((void **)&_lowest_geom_id, sizeof(void *), 1 * sizeof(void *));
	  PAMI_assertf(_lowest_geom_id, "Failed to get shmem for _lowest_geom_id");
	  *_lowest_geom_id = 0xffffffff;

	  TRACE((stderr,"MU ResourceManager: Inside ENABLE_MU_CLASSROUTES code\n"));
	  // This init code is performed ONCE per process and is shared by
	  // all clients and their contexts...
	  __global.topology_global.subTopologyNthGlobal(&_node_topo, 0);
	  _node_topo.convertTopology(PAMI_COORD_TOPOLOGY);

	  _cr_mtx.init(&mm);
	  _cr_mtx_mdls = (MUCR_mutex_model_t **)malloc(_pamiRM.getNumClients() * sizeof(*_cr_mtx_mdls));

	  // Note, we NEVER use BGQ_CLASS_INPUT_VC_USER. Only BGQ_CLASS_INPUT_VC_SUBCOMM.

	  uint32_t crs[BGQ_COLL_CLASS_MAX_CLASSROUTES];
	  uint32_t ncr = 0;
	  int rc = Kernel_QueryCollectiveClassRoutes(&ncr, crs, sizeof(crs));
	  rc = rc;
	  // first, locally "block-out" any already reserved classroutes...
	  // we take the "last" N ids, where "N" is some number we get from a
	  // resource manager - TBD.
	  uint32_t N = ncr; // take all, until we know otherwise...
	  if (N > ncr) N = ncr;
	  MUSPI_InitClassrouteIds(&crs[ncr - N], N, 0, &_cncrdata);
	  uint32_t x;
	  for (x = ncr - N; x < N; ++x)
	  {
	    Kernel_AllocateCollectiveClassRoute(crs[x]);
	  }
	  rc = Kernel_QueryGlobalInterruptClassRoutes(&ncr, crs, sizeof(crs));
	  rc = rc;
	  // first, locally "block-out" any already reserved classroutes...
	  // we take the "last" N ids, where "N" is some number we get from a
	  // resource manager - TBD.
	  N = ncr; // take all, until we know otherwise...
	  if (N > ncr) N = ncr;
	  MUSPI_InitClassrouteIds(&crs[ncr - N], N, 1, &_gicrdata);
	  for (x = ncr - N; x < N; ++x)
	  {
	    Kernel_AllocateGlobalInterruptClassRoute(crs[x], NULL);
	  }

	  // next, add classroute #0 (user comm-world) which is
	  // pre-fabricated by controlsystem and CNK (not in free list, above).
	  // but, we need to cnostruct the rectangle for it ourself...

	  // this code should be very similar to classroute_test.c
	  // Get all the rectangles and coords we need to describe the system.
	  *CR_RECT_LL(&_refcomm) = CR_COORD_INIT(0,0,0,0,0);
	  *CR_RECT_UR(&_refcomm) = CR_COORD_INIT(
	    _pers.Network_Config.Anodes - 1,
	    _pers.Network_Config.Bnodes - 1,
	    _pers.Network_Config.Cnodes - 1,
	    _pers.Network_Config.Dnodes - 1,
	    _pers.Network_Config.Enodes - 1
	  );
	  _mycoord = CR_COORD_INIT(
	    _pers.Network_Config.Acoord,
	    _pers.Network_Config.Bcoord,
	    _pers.Network_Config.Ccoord,
	    _pers.Network_Config.Dcoord,
	    _pers.Network_Config.Ecoord
	  );
	  MUSPI_PickWorldRoot(&_refcomm, NULL, &_refroot, &_pri_dim);
	  char *s = getenv("BG_MAPFILE");
	  int i = 0;
	  int map[CR_NUM_DIMS];
	  if (s)
	  {
	    char *x;
	    const char *m = CR_DIM_NAMES;
	    for (; *s && i < CR_NUM_DIMS; ++s)
	    {
	      if (*s == 'T') continue;
	      x = strchr(m, *s);
	      if (!x) break;
	      map[i++] = (x - m);
	    }
	  }
	  if ((s && *s) || i != CR_NUM_DIMS)
	  {
	    // error - invalid map string, or an actual map file...
	    // no classroutes supported (no MU collectives at all?)
	    // \todo #warning need to handle mapfiles somehow

	    // for now, just use simple 1:1 mapping
	    for (i = 0; i < CR_NUM_DIMS; ++i) map[i] = i;
	  }
	  BG_JobCoords_t subblk;
	  int is_subblockjob = Kernel_JobCoords(&subblk);
	  if (is_subblockjob)
	  {
	    PAMI_assertf(subblk.shape.core == 16, "Sub-node jobs not supported");
	  }
	  *CR_RECT_LL(&_communiv) = CR_COORD_INIT(
	    subblk.corner.a,
	    subblk.corner.b,
	    subblk.corner.c,
	    subblk.corner.d,
	    subblk.corner.e
	  );
	  *CR_RECT_UR(&_communiv) = CR_COORD_INIT(
	    subblk.corner.a + subblk.shape.a - 1,
	    subblk.corner.b + subblk.shape.b - 1,
	    subblk.corner.c + subblk.shape.c - 1,
	    subblk.corner.d + subblk.shape.d - 1,
	    subblk.corner.e + subblk.shape.e - 1
	  );
	  size_t univz = __MUSPI_rect_size(&_communiv);
	  // now, factor in any -np...
	  s = getenv("BG_NP");
	  if (s)
	  {
	    int n = strtoul(s, NULL, 0);
	    s = getenv("BG_PROCESSESPERNODE");
	    if (s)
	    {
	      int v = strtoul(s, NULL, 0);
	      n = (n + v - 1) / v;
	    }
	    _np = n;
	  }
	  else
	  {
	    _np = __MUSPI_rect_size(&_communiv);
	  }
	  if (_np < univz) { // must exclude some nodes...
	    _excluded = (CR_COORD_T *)malloc((univz - _np) * sizeof(CR_COORD_T));
	    // PAMI_assert(_excluded != NULL);

	    // Note, this discards previous _pri_dim... is that ok?
	    MUSPI_MakeNpRectMap(&_communiv, _np, _map,
	            &_commworld, _excluded, &_nexcl, &_pri_dim);
	  }
	  else
	  {
	    // do we really need so many rectangles?
	    _commworld = _communiv;
	  }
	  // could do: new (t) Topology(CR_RECT_LL(&_commworld),
	  //                              CR_RECT_UR(&_commworld), ... );
	  //
	  // Must be able to determine if a geometry is "comm world"
	  // (taking into account excluded nodes), but otherwise every
	  // classroute will be a full rectangle.
	  // (how will commworld (topology) be represented in those cases?)

	  // Caution: this is abnormal use of the classroute API.
	  // This sets up our tables for a pre-existing classroute for the
	  // entire job. So, we only update our table, we don't write DCRs
	  // or any such thing.
	  MUSPI_SetClassrouteId(0, BGQ_CLASS_INPUT_VC_SUBCOMM, &_commworld, &_cncrdata);
	  MUSPI_SetClassrouteId(0, BGQ_CLASS_INPUT_VC_SUBCOMM, &_commworld, &_gicrdata);
	  // do we need to place this rectangle someplace? like the "world geom" topology?

	  // Now, we should be ready to get requests for classroutes...
	  // via the geomOptimize() method below...
	  TRACE((stderr,"MU ResourceManager: Exiting constructor\n"));
	 } // End: UseMU
	} // End: ResourceManager Default Constructor
	// Note, we NEVER use BGQ_CLASS_INPUT_VC_USER. Only BGQ_CLASS_INPUT_VC_SUBCOMM.
	#define CR_ALLREDUCE_DT_CT 3
	struct cr_cookie
	{
	  ResourceManager *thus;
	  PAMI::Geometry::Common *geom;
	  bool master;
	  unsigned geom_id;
	  PAMI::Topology topo;
	  pami_multisync_t msync;
	  MUCR_mutex_model_t *cr_mtx_mdl;
	  uint8_t mbuf[MUCR_mutex_model_t::sizeof_msg];
	  pami_xfer_t xfer;
	  uint64_t abuf[CR_ALLREDUCE_DT_CT];
	  uint64_t bbuf[CR_ALLREDUCE_DT_CT];
	  pami_work_t post;
	  pami_callback_t cb_done;
	};

	// this code should be very similar to classroute_test.c,
	// however, this must be non-blocking code so we have to
	// do this as work posted to context(s).
	// we only need one attempt per geometry, so use a special value
	// to indicate "can't". Note, this routine will be called several times
	// for a given geometry, during create. Once for each potential algorithm.
	// but the answer we get is valid for all algorithms, so don't keep trying.
	inline pami_result_t geomOptimize(PAMI::Geometry::Common *geom,
	    size_t clientid, size_t contextid, pami_context_t context,
	    pami_event_function fn, void *clientdata)
	{
	  // we always try to optimize, since this is called only once
	  // per geometry (per change in PAMI_GEOMETRY_OPTIMIZE config)
	  // however, if already fully optimized, don't bother.
	  if (geom->getKey(PAMI::Geometry::PAMI_GKEY_MCAST_CLASSROUTEID) &&
	  	geom->getKey(PAMI::Geometry::PAMI_GKEY_MSYNC_CLASSROUTEID))
	  {
	    if (fn) fn(context, clientdata, PAMI_SUCCESS);
	    return PAMI_SUCCESS;
	  }

	  // check for comm-world and just use well-known classroute id "0"...

	  // simple check - works for all numbers of processes-per-node
	  if (geom->nranks() == __global.topology_global.size())
	  {
	    geom->setKey(PAMI::Geometry::PAMI_GKEY_MCAST_CLASSROUTEID, (void *)(0 + 1));
	    geom->setKey(PAMI::Geometry::PAMI_GKEY_MCOMB_CLASSROUTEID, (void *)(0 + 1));
	    geom->setKey(PAMI::Geometry::PAMI_GKEY_MSYNC_CLASSROUTEID, (void *)(0 + 1));
	    if (fn) fn(context, clientdata, PAMI_SUCCESS);
	    return PAMI_SUCCESS;
	  }

	  /// \todo #warning needs to be modified to work with -np comm-worlds

	  PAMI::Topology *topo = (PAMI::Topology *)geom->getTopology(0);
	  // try to convert to a rectangle... will NO-OP if already rectangle...
	  // this will change the topology in the geometry... should be OK but
	  // if there are other ways to optimize we need to avoid thrashing.
	  (void)topo->convertTopology(PAMI_COORD_TOPOLOGY);

	  // for now, just bail-out if not a rectangle...
	  // we could try to convert to rectangle, or see if subTopologyNthGlobal
	  // produces a rectangle, etc.
	  if (topo->type() != PAMI_COORD_TOPOLOGY)
	  {
	    if (fn) fn(context, clientdata, PAMI_SUCCESS);
	    return PAMI_SUCCESS; // not really failure, just can't/won't do it.
	  }

	  // a more-exhaustive check for "comm-world" equivalent classroute...
	  PAMI::Topology node_topo, local_topo;
 	  topo->subTopologyNthGlobal(&node_topo, 0);
	  topo->subTopologyLocalToMe(&local_topo);
	  // assert(local_topo.size() > 0);
	  bool master = (__global.mapping.task() == local_topo.index2Rank(0));
	  /// \todo #warning must confirm that T==0 exists in rectangle...

	  CR_RECT_T rect1, rect2;
	  node_topo.rectSeg(CR_RECT_LL(&rect1), CR_RECT_UR(&rect1));
	  _node_topo.rectSeg(CR_RECT_LL(&rect2), CR_RECT_UR(&rect2));
	  if (__MUSPI_rect_compare(&rect1, &rect2) == 0) {
	    geom->setKey(PAMI::Geometry::PAMI_GKEY_MCAST_CLASSROUTEID, (void *)(0 + 1));
	    geom->setKey(PAMI::Geometry::PAMI_GKEY_MCOMB_CLASSROUTEID, (void *)(0 + 1));
	    geom->setKey(PAMI::Geometry::PAMI_GKEY_MSYNC_CLASSROUTEID, (void *)(0 + 1));
	    if (fn) fn(context, clientdata, PAMI_SUCCESS);
	    return PAMI_SUCCESS;
	  }

	  /// \todo Only the primary task of the node should actually alter MU DCRs...
	  // Need to also ensure that no one uses this classroute until all nodes'
	  // DCRs have been set. This requires a "global" barrier after this,
	  // which means two barriers total in the case of geometry_create.

	  // we could do even more checking and allow more geometries...

	  unsigned geom_id = geom->comm();

	  // from this point, we must have a valid context.
	  PAMI_assertf(context != NULL, "geomOptimize called on non-world w/o context");

	  // This topology is part of the geometry, so we know it won't
	  // "go away" after this method returns...
	  cr_cookie *cookie = (cr_cookie *)malloc(sizeof(cr_cookie)); // how to alloc?
	  cookie->thus = this;
	  cookie->master = master;
	  cookie->cb_done = (pami_callback_t){fn, clientdata};
	  cookie->topo = node_topo;
	  cookie->cr_mtx_mdl = &_cr_mtx_mdls[clientid][contextid];
	  cookie->msync.client = clientid;
	  cookie->msync.context = contextid;
	  cookie->geom = geom;
	  cookie->geom_id = geom_id;
	  cookie->abuf[1] = geom_id;
	  cookie->abuf[2] = ~geom_id;
	  cookie->xfer.cmd.xfer_allreduce.sndbuf = (char *)&cookie->abuf[0];
	  cookie->xfer.cmd.xfer_allreduce.stype = PAMI_BYTE;
	  cookie->xfer.cmd.xfer_allreduce.stypecount = sizeof(cookie->abuf);
	  cookie->xfer.cmd.xfer_allreduce.rcvbuf = (char *)&cookie->bbuf[0];
	  cookie->xfer.cmd.xfer_allreduce.rtype = PAMI_BYTE;
	  cookie->xfer.cmd.xfer_allreduce.rtypecount = sizeof(cookie->bbuf);
	  cookie->xfer.cmd.xfer_allreduce.dt = PAMI_UNSIGNED_LONG_LONG;
	  cookie->xfer.cmd.xfer_allreduce.op = PAMI_BAND;
	  // tell geometry completion to wait for us...
	  geom->addCompletion();
	  start_over(context, cookie, PAMI_SUCCESS);
	  return PAMI_SUCCESS;
	}

	// a.k.a. Geometry_destroy... this happens to be immediate and local...
	// but, do we need the MU Coll device mutex? (_cr_mtx)
	inline pami_result_t geomDeoptimize(PAMI::Geometry::Common *geom)
	{
	  void *val = geom->getKey(PAMI::Geometry::PAMI_GKEY_MCAST_CLASSROUTEID);
	  if (val && val != PAMI_CR_GKEY_FAIL)
	  {
	    int id = (int)((uintptr_t)val & 0xffffffff) - 1;
	    if (id != 0) // never free classroute 0 - a.k.a. comm-world
	    {
	      int last = MUSPI_ReleaseClassrouteId(id, BGQ_CLASS_INPUT_VC_SUBCOMM,
	                                                        NULL, &_cncrdata);
	      if (last)
	      {
	        // This "frees" the id to others... don't want that.
	        // rc = Kernel_DeallocateCollectiveClassRoute(id);
	        // it is probably OK to leave the old one around,
	        // else need to be sure what value we can set it to
	        // and not have CNK think it is "free"...
	        // cr = (some bit pattern);
	        // rc = Kernel_SetCollectiveClassRoute(id, &cr);
	        // for now, this code will just overwrite the bits next
	        // time this id gets used.
	      }
	    }
	  }
	  val = geom->getKey(PAMI::Geometry::PAMI_GKEY_MSYNC_CLASSROUTEID);
	  if (val && val != PAMI_CR_GKEY_FAIL)
	  {
	    int id = (int)((uintptr_t)val & 0xffffffff) - 1;
	    if (id != 0) // never free classroute 0 - a.k.a. comm-world
	    {
	      int last = MUSPI_ReleaseClassrouteId(id, BGQ_CLASS_INPUT_VC_SUBCOMM,
	                                                        NULL, &_gicrdata);
	      if (last)
	      {
	        // see Coll case above...
	      }
	    }
	  }
	  geom->setKey(PAMI::Geometry::PAMI_GKEY_MCAST_CLASSROUTEID, NULL);
	  geom->setKey(PAMI::Geometry::PAMI_GKEY_MCOMB_CLASSROUTEID, NULL);
	  geom->setKey(PAMI::Geometry::PAMI_GKEY_MSYNC_CLASSROUTEID, NULL);
	  return PAMI_SUCCESS;
	}

	static void start_over(pami_context_t ctx, void *cookie, pami_result_t result)
	{
	  cr_cookie *crck = (cr_cookie *)cookie;
	  if (result != PAMI_SUCCESS && result != PAMI_EAGAIN)
	  {
	    if (crck->cb_done.function) crck->cb_done.function(ctx, crck->cb_done.clientdata, result);
	    // tell geometry completion "we're done"...
	    crck->geom->rmCompletion(ctx, result);
	    free(cookie);
	    return; // mutex error?!
	  }
	  crck->msync.cb_done = (pami_callback_t){ got_mutex, crck };
	  crck->msync.roles = MUCR_mutex_model_t::LOCK_ROLE;
	  pami_result_t rc = crck->cr_mtx_mdl->postMultisync(crck->mbuf, &crck->msync);
	  if (rc != PAMI_SUCCESS)
	  {
	    // this frees 'cookie' if needed...
	    // this also tells geom we're done.
	    got_mutex(ctx, cookie, rc); // should this retry?
	    return;
	  }
	}

	static void got_mutex(pami_context_t ctx, void *cookie, pami_result_t result)
	{
	  // we now "own" this device class instance, on the entire node.
	  cr_cookie *crck = (cr_cookie *)cookie;
	  if (result != PAMI_SUCCESS)
	  {
	    if (crck->cb_done.function) crck->cb_done.function(ctx, crck->cb_done.clientdata, result);
	    // tell geometry completion "we're done"...
	    crck->geom->rmCompletion(ctx, result);
	    free(cookie);
	    return; // mutex error?! cleanup?
	  }

	  if (crck->geom_id > *crck->thus->_lowest_geom_id)
	  {
	    // we should not be trying right now, let another...
	    crck->thus->release_mutex(ctx, cookie, PAMI_SUCCESS);
	    start_over(ctx, cookie, PAMI_EAGAIN);
	    return;
	  }
	  *crck->thus->_lowest_geom_id = crck->geom_id;

	  // for now, this is only for true rectangles... (_nexcl == 0)
	  CR_RECT_T rect;
	  crck->topo.rectSeg(CR_RECT_LL(&rect), CR_RECT_UR(&rect));
	  /// \todo #warning _cncrdata (_gicrdata) must be in shared memory!
	  uint32_t mask1 = 0, mask2 = 0;

	  void *val = crck->geom->getKey(PAMI::Geometry::PAMI_GKEY_MCAST_CLASSROUTEID);
	  if (!val || val == PAMI_CR_GKEY_FAIL) {
	    mask1 = MUSPI_GetClassrouteIds(BGQ_CLASS_INPUT_VC_SUBCOMM,
	            &rect, &crck->thus->_cncrdata);
	  }
	  val = crck->geom->getKey(PAMI::Geometry::PAMI_GKEY_MSYNC_CLASSROUTEID);
	  if (!val || val == PAMI_CR_GKEY_FAIL) {
	    mask2 = MUSPI_GetClassrouteIds(BGQ_CLASS_INPUT_VC_SUBCOMM,
	            &rect, &crck->thus->_gicrdata);
	  }
	  // is this true? can we be sure ALL nodes got the same result?
	  // if so, and we are re-using existing classroute, then we can skip
	  // the allreduce...
	  crck->abuf[0] = ((uint64_t)mask1 & ~CR_MATCH_RECT_FLAG) |
	  		(((uint64_t)mask2 & ~CR_MATCH_RECT_FLAG) << 32);
#if 0 // can't do this with two indepoendent masks (GI+Coll)?
	  if (mask & CR_MATCH_RECT_FLAG)
	  {
	    crck->bbuf[0] = crck->abuf[0]; // "nil" allreduce
	    crck->bbuf[1] = crck->abuf[1];
	    crck->bbuf[2] = crck->abuf[2];
	    // this frees 'cookie' if needed...
	    // this also tells geom we're done.
	    cr_allreduce_done(ctx, cookie, PAMI_SUCCESS);
	    return;
	  }
#endif
	  // Now, must perform an "allreduce(&mask, 1, PAMI_LONGLONG, PAMI_AND)"
	  pami_algorithm_t algo = 0;
	  PAMI_Geometry_algorithms_query(ctx, crck->geom, PAMI_XFER_ALLREDUCE,
	                                        &algo, NULL, 1, NULL, NULL, 0);
	  if (!algo)
	  {
	    cr_allreduce_done(ctx, cookie, PAMI_ERROR);
	    return;
	  }
	  crck->xfer.cb_done = cr_allreduce_done;
	  crck->xfer.cookie = crck;
	  crck->xfer.algorithm = algo;
	  // because of circular dependencies, can't dereference the context here,
	  // so we must use the C API. Would be best to directly setup MU Coll
	  // descriptor and inject, but this is currently on the new geom which
	  // has no classroute, and if we use the parent then we have to do something
	  // special to get the non-member nodes involved, since the parent members
	  // that are not part of the sub-geometry don't even call analyze...
	  pami_result_t rc = PAMI_Collective(ctx, &crck->xfer);
	  if (rc != PAMI_SUCCESS)
	  {
	    // this frees 'cookie' if needed...
	    // this also tells geom we're done.
	    cr_allreduce_done(ctx, cookie, rc); // should this retry?
	    return;
	  }
	}

	static void cr_allreduce_done(pami_context_t ctx, void *cookie, pami_result_t result)
	{
	  cr_cookie *crck = (cr_cookie *)cookie;
	  if (result != PAMI_SUCCESS)
	  {
	    if (crck->cb_done.function) crck->cb_done.function(ctx, crck->cb_done.clientdata, result);
	    // tell geometry completion "we're done"...
	    crck->geom->rmCompletion(ctx, result);
	    free(cookie); // don't do this if it retries...
	    // release mutex?!
	    return; // need cleanup??? retry?
	  }

	  if (crck->bbuf[1] != crck->abuf[1] ||
	      crck->bbuf[2] != crck->abuf[2])
	  {
	    // failed (collided), must reset and start over...
	    crck->thus->release_mutex(ctx, cookie, PAMI_SUCCESS);
	    start_over(ctx, cookie, PAMI_EAGAIN);
	    return;
	  }

	  ClassRoute_t cr = {0};
	  crck->thus->set_classroute(crck->bbuf[0] & 0x0000ffff, crck, &cr,
					PAMI::Geometry::PAMI_GKEY_MCAST_CLASSROUTEID);
	  crck->thus->set_classroute((crck->bbuf[0] >> 32) & 0x0000ffff, crck, &cr,
					PAMI::Geometry::PAMI_GKEY_MSYNC_CLASSROUTEID);
	  // we got the answer we needed... no more trying...
	  *crck->thus->_lowest_geom_id = 0xffffffff;
	  crck->thus->release_mutex(ctx, cookie, PAMI_SUCCESS);
	  pami_result_t rc = crck->geom->default_barrier(cr_barrier_done, cookie,
							crck->msync.context, ctx);
	  if (rc != PAMI_SUCCESS) {
	    cr_barrier_done(ctx, cookie, rc);
	  }
	}

	static void cr_barrier_done(pami_context_t ctx, void *cookie, pami_result_t result)
	{
	  cr_cookie *crck = (cr_cookie *)cookie;
	  if (crck->cb_done.function) crck->cb_done.function(ctx, crck->cb_done.clientdata, result);
	  // tell geometry completion "we're done"...
	  crck->geom->rmCompletion(ctx, result);
	  free(cookie);
	}

	inline void release_mutex(pami_context_t ctx, void *cookie, pami_result_t result)
	{
	  cr_cookie *crck = (cr_cookie *)cookie;
	  crck->msync.cb_done = (pami_callback_t){ NULL, NULL };
	  crck->msync.roles = MUCR_mutex_model_t::UNLOCK_ROLE;
	  pami_result_t rc = crck->cr_mtx_mdl->postMultisync(crck->mbuf, &crck->msync);
	  rc = rc;
	  // no such thing as failure... nore is there any delay...
	  // mutex is now unlocked, so we are "done" in every way that matters.
	}

	// Only holder of mutex calls this... thread safe in process.
	inline void set_classroute(uint32_t mask, cr_cookie *crck, ClassRoute_t *cr, PAMI::Geometry::gkeys_t key) {
	  uint32_t id = ffs(mask);
	  bool gi = (key == PAMI::Geometry::PAMI_GKEY_MSYNC_CLASSROUTEID);
	  void **envpp = (gi ? &_gicrdata : &_cncrdata);
	  if (id)
	  {
	    --id; // ffs() returns bit# + 1
	    CR_RECT_T rect;
	    crck->topo.rectSeg(CR_RECT_LL(&rect), CR_RECT_UR(&rect));
	    int first = MUSPI_SetClassrouteId(id, BGQ_CLASS_INPUT_VC_SUBCOMM,
								&rect, envpp);
	    // Note: need to detect if classroute was already programmed...
	    if (crck->master && first)
	    {
	      if (!cr->input) {
	        MUSPI_BuildNodeClassroute(&_refcomm, &_refroot, &_mycoord, &rect,
							_map, _pri_dim, cr);
	        cr->input |= BGQ_CLASS_INPUT_LINK_LOCAL;
	        cr->input |= BGQ_CLASS_INPUT_VC_SUBCOMM;
	      }
	      int rc;
	      if (gi) {
	        rc = Kernel_SetGlobalInterruptClassRoute(id, cr);
	      } else {
	        rc = Kernel_SetCollectiveClassRoute(id, cr);
	      }
	      rc = rc; // until error checking
	    }
	    crck->geom->setKey(key, (void *)(id + 1));
	    if (!gi) {
	      crck->geom->setKey(PAMI::Geometry::PAMI_GKEY_MCOMB_CLASSROUTEID, (void *)(id + 1));
	    }
	  }
	}

        // \brief Get Per Process PAMI Max Number of Contexts For A Client
	inline size_t getPerProcessMaxPamiResources ( size_t RmClientId )
	{ return _perProcessMaxPamiResources[RmClientId].numContexts; }

        // \brief Get Per Process PAMI Max Number of Contexts Across All Clients
	inline size_t getPerProcessMaxPamiResources ( )
	{
	  size_t rmClientId;
	  size_t numClients       = _pamiRM.getNumClients();
	  size_t totalNumContexts = 0;

	  for ( rmClientId=0; rmClientId<numClients; rmClientId++ )
	    totalNumContexts += _perProcessMaxPamiResources[rmClientId].numContexts;

	  TRACE((stderr,"MU Context: getPerProcessMaxPamiResourcesAcrossAllClients = %zu\n",totalNumContexts));
	  return totalNumContexts;
	}

        // \brief Get Per Process PAMI Optimal Number of Contexts For A Client
	inline size_t getPerProcessOptimalPamiResources ( size_t RmClientId )
	{ return _perProcessOptimalPamiResources[RmClientId].numContexts; }

        // \brief Get Per Process PAMI Optimal Number of Contexts Across All Clients
	inline size_t getPerProcessOptimalPamiResources ( )
	{
	  size_t rmClientId;
	  size_t numClients       = _pamiRM.getNumClients();
	  size_t totalNumContexts = 0;

	  for ( rmClientId=0; rmClientId<numClients; rmClientId++ )
	    totalNumContexts += _perProcessOptimalPamiResources[rmClientId].numContexts;

	  TRACE((stderr,"MU Context: getPerProcessOptimalPamiResourcesAcrossAllClients = %zu\n",totalNumContexts));
	  return totalNumContexts;
	}

	inline MUSPI_InjFifo_t * getGlobalCombiningInjFifoPtr ()
	{
	  uint32_t subgroup = (_globalCombiningInjFifo.globalFifoIds[0] / BGQ_MU_NUM_INJ_FIFOS_PER_SUBGROUP) - 64;
	  uint32_t fifoId   = _globalCombiningInjFifo.globalFifoIds[0] % BGQ_MU_NUM_INJ_FIFOS_PER_SUBGROUP;
	  return &(_globalCombiningInjFifo.subgroups[subgroup]._injfifos[fifoId]);
	}

	inline char **getGlobalCombiningLookAsidePayloadBufferVAs()
	  { return _globalCombiningInjFifo.lookAsidePayloadPtrs; }

	inline uint64_t *getGlobalCombiningLookAsidePayloadBufferPAs()
	  { return _globalCombiningInjFifo.lookAsidePayloadPAs; }

	inline pami_event_function **getGlobalCombiningLookAsideCompletionFnPtrs()
	  { return _globalCombiningInjFifo.lookAsideCompletionFnPtrs; }

	inline void                ***getGlobalCombiningLookAsideCompletionCookiePtrs()
	  { return _globalCombiningInjFifo.lookAsideCompletionCookiePtrs; }

	/// \brief Get Global Base Address Table Id
	///
	/// This base address table entry has a zero value stored in it so
	/// that the physical address can be used as a put offset or counter offset.
	///
	inline uint32_t getGlobalBatId ()
	{ return _globalBatIds[0]; }

	/// \brief Get Shared Counter Base Address Table Id
	///
	/// This base address table entry has an atomic address of a
	/// reception counter that is shared...the counter's value is
	/// not useful (ignored).  The counter offset in the descriptor
	/// can be zero.
	inline uint32_t getSharedCounterBatId ()
	{ return _globalBatIds[1]; }

	inline size_t mapClientIdToRmClientId ( size_t clientId );

	inline void initializeContexts( size_t rmClientId,
				 size_t numContexts,
                                 PAMI::Device::Generic::Device * devices );

	inline void getNumFifosPerContext( size_t  rmClientId,
				    size_t *numInjFifos,
				    size_t *numRecFifos );

	inline uint16_t *getPinBroadcastFifoMap( size_t numInjFifos )
	  { return &(_pinBroadcastFifoMap[numInjFifos-1][0]); }

	inline pinInfoEntry_t *getPinInfo( size_t numInjFifos )
	  { return &(_pinInfo[numInjFifos-1]); }

	inline pinInfoEntry_t *getRgetPinInfo()
	  {
	    // Fill in the global rget fifo IDs.
	    // Couldn't do this during global init without a barrier,
	    // so doing it here instead, after the barrier.
	    uint32_t i;
	    for ( i=0; i<numFifoPinIndices; i++ )
	      {
		_rgetPinInfo->injFifoIds[i] =
		  _globalRgetInjFifoIds[ _pinInfo[9].injFifoIds[i] ];
	      }

	    TRACE((stderr,"MU Context: getRgetPinInfo: _rgetPinInfo->injFifoIds[] = %u,%u,%u,%u,%u,%u,%u,%u,%u,%u, %u,%u,%u,%u,%u,%u\n",_rgetPinInfo->injFifoIds[0],_rgetPinInfo->injFifoIds[1],_rgetPinInfo->injFifoIds[2],_rgetPinInfo->injFifoIds[3],_rgetPinInfo->injFifoIds[4],_rgetPinInfo->injFifoIds[5],_rgetPinInfo->injFifoIds[6],_rgetPinInfo->injFifoIds[7],_rgetPinInfo->injFifoIds[8],_rgetPinInfo->injFifoIds[9],_rgetPinInfo->injFifoIds[10],_rgetPinInfo->injFifoIds[11],_rgetPinInfo->injFifoIds[12],_rgetPinInfo->injFifoIds[13],_rgetPinInfo->injFifoIds[14],_rgetPinInfo->injFifoIds[15]));

	    return _rgetPinInfo;
	  }

	inline uint16_t getPinRecFifo( size_t rmClientId,
				       size_t contextOffset,
				       size_t t )
	{ return _clientResources[rmClientId].pinRecFifo[(contextOffset*_tSize) + t]; }

	inline void getInjFifosForContext( size_t            rmClientId,
					   size_t            contextOffset,
					   size_t            numInjFifos,
					   MUSPI_InjFifo_t **injFifoPtrs );

	inline void getRecFifosForContext( size_t            rmClientId,
					   size_t            contextOffset,
					   size_t            numRecFifos,
					   MUSPI_RecFifo_t **recFifoPtrs,
					   uint32_t         *globalFifoIds );

	inline char **getLookAsidePayloadBufferVAs( size_t rmClientId,
						    size_t contextOffset )
	  { return _clientResources[rmClientId].injResources[contextOffset].lookAsidePayloadPtrs; }

	inline uint64_t *getLookAsidePayloadBufferPAs( size_t rmClientId,
						       size_t contextOffset )
	  { return _clientResources[rmClientId].injResources[contextOffset].lookAsidePayloadPAs; }

	inline pami_event_function **getLookAsideCompletionFnPtrs( size_t rmClientId,
								   size_t contextOffset )
	  { return _clientResources[rmClientId].injResources[contextOffset].lookAsideCompletionFnPtrs; }

	inline void                ***getLookAsideCompletionCookiePtrs( size_t rmClientId,
								 size_t contextOffset )
	  { return _clientResources[rmClientId].injResources[contextOffset].lookAsideCompletionCookiePtrs; }
	inline size_t getMaxNumDescInInjFifo()
	{ return ( _pamiRM.getInjFifoSize() / sizeof( MUHWI_Descriptor_t ) ) -1; }

	inline uint32_t *getRgetInjFifoIds()
	  { return _globalRgetInjFifoIds; }

      private:
	inline uint16_t chooseFifo (size_t  sourceDim,
				    ssize_t hops,
				    ssize_t sizeHalved,
				    bool    isTorus);
	inline uint16_t pinFifo( size_t task );
	inline void initFifoPin();
	inline void calculatePerCoreMUResourcesBasedOnAvailability();
	inline void calculatePerCoreMUResourcesBasedOnConfig();
	inline void calculatePerCoreMUResources();
	inline void calculatePerCorePerProcessMUResources();
	inline void dividePAMIResourceAmongClients( pamiResources_t &resourceValue,
						    size_t           numClients,
						    pamiResources_t *clientResource );
	inline void divideMUResourceAmongClients( muResources_t &resourceValue,
						  size_t         numClients,
						  muResources_t *clientResource );
	inline void calculatePerProcessMaxPamiResources();
	inline void calculatePerProcessOptimalPamiResources();
	inline void calculatePerCorePerProcessPerClientMUResources();
	inline void setupSharedMemory();
	inline void allocateMemory( bool    useSharedMemory,
				    void ** memptr,
				    size_t  alignment,
				    size_t  bytes );
	inline uint32_t setupInjFifos( uint32_t startingSubgroup,
				       uint32_t endingSubgroup,
				       size_t   numFifos,
				       size_t   fifoSize,
				       Kernel_InjFifoAttributes_t  *fifoAttr,
				       bool                         useSharedMemory,
				       MUSPI_InjFifoSubGroup_t    **subgroups,
				       char                      ***fifoPtrs,
				       uint32_t                   **globalFifoIds );
	inline uint32_t setupRecFifos( uint32_t startingSubgroup,
				       uint32_t endingSubgroup,
				       size_t   numFifos,
				       size_t   fifoSize,
				       Kernel_RecFifoAttributes_t  *fifoAttr,
				       bool                         useSharedMemory,
				       MUSPI_RecFifoSubGroup_t    **subgroups,
				       char                      ***fifoPtrs,
				       uint32_t                   **globalFifoIds );
	inline uint32_t setupBatIds(
				    uint32_t                           startingSubgroup,
				    uint32_t                           endingSubgroup,
				    uint32_t                           numBatIds,
				    bool                               useSharedMemory,
				    MUSPI_BaseAddressTableSubGroup_t **subgroups,
				    uint32_t                         **globalBatIds);
	inline void allocateGlobalResources();
	inline void allocateGlobalInjFifos();
	inline void allocateGlobalBaseAddressTableEntries();
	inline void allocateGlobalRecFifos();
	inline void allocateContextResources( size_t rmClientId,
					      size_t contextOffset );
	inline void allocateLookasideResources(
					  size_t                   numInjFifos,
					  size_t                   injFifoSize,
					  char                  ***lookAsidePayloadPtrs,
					  Kernel_MemoryRegion_t  **lookAsidePayloadMemoryRegions,
					  uint64_t               **lookAsidePayloadPAs,
					  pami_event_function   ***lookAsideCompletionFnPtrs,
					  void                 ****lookAsideCompletionCookiePtrs);

	//////////////////////////////////////////////////////////////////////////
	///
	/// Member data:
	///
	/////////////////////////////////////////////////////////////////////////

	PAMI::ResourceManager &_pamiRM;
	PAMI::Mapping         &_mapping;
	PAMI::BgqPersonality  &_pers;
	MUCR_mutex_t _cr_mtx; // each context has MUCR_mutex_model_t pointing to this
	MUCR_mutex_model_t **_cr_mtx_mdls;
	PAMI::Topology _node_topo;
	CR_RECT_T _refcomm;
	CR_COORD_T _refroot;
	CR_COORD_T _mycoord;
	int _pri_dim;
	int _map[CR_NUM_DIMS];
	CR_RECT_T _communiv;
	size_t _np;
	CR_RECT_T _commworld;
	CR_COORD_T *_excluded;
	int _nexcl;
	unsigned *_lowest_geom_id; // in shared memory! (protected by _cr_mtx)
	void *_cncrdata; // used by MUSPI routines to keep track of
	               // classroute assignments - persistent!
	void *_gicrdata; // (ditto)
	// _pinInfo is an array of entries.  Each entry has info for when there
	// are a specific number of inj fifos in the context.
	// There are 10 entries, for 1 fifo, 2 fifos, ..., 10 fifos.
	pinInfoEntry_t *_pinInfo;

	// _rgetPinInfo is a single entry set of global rget inj fifo Ids.
	// The first 10 ids are for torus transfers.
	// The last 6 are for local transfers.
	pinInfoEntry_t *_rgetPinInfo;

	uint16_t _pinBroadcastFifoMap[numTorusDirections][BGQ_COLL_CLASS_MAX_CLASSROUTES];

	size_t  _tSize;
	ssize_t _aSizeHalved;
	ssize_t _bSizeHalved;
	ssize_t _cSizeHalved;
	ssize_t _dSizeHalved;
	ssize_t _eSizeHalved;
	bool    _isTorusA;
	bool    _isTorusB;
	bool    _isTorusC;
	bool    _isTorusD;
	bool    _isTorusE;

	// MU Resources
	muResources_t _perCoreMUResourcesBasedOnAvailability;
	muResources_t _perCoreMUResourcesBasedOnConfig;
	muResources_t _perCoreMUResources;
	muResources_t _perCorePerProcessMUResources;

	// Per client resource arrays
	pamiResources_t   *_perProcessMaxPamiResources;
	pamiResources_t   *_perProcessOptimalPamiResources;
	muResources_t     *_perCorePerProcessPerClientMUResources;
	pamiResources_t   *_perCorePerProcessPamiResources;
	muResources_t     *_perContextMUResources;
	clientResources_t *_clientResources;

	// Shared Memory Manager
	PAMI::Memory::MemoryManager        _mm;
	size_t                             _memSize;

	// Node-wide resources
	unsigned int                       _calculateSizeOnly;      // 1 = Calculate _memSize
                                                                    // 0 = Allocate _memSize
	unsigned int                       _allocateOnly;           // 1 = Allocate, no init
                                                                    // 0 = Allocate and init

	MUSPI_InjFifoSubGroup_t           *_globalRgetInjSubGroups; // Rget Subgroups 64 and 65.
	char                             **_globalRgetInjFifoPtrs;  // RgetPointers to fifos.
	uint32_t                          *_globalRgetInjFifoIds;   // Rget Ids.

	injFifoResources_t                 _globalCombiningInjFifo; // Global Inj Fifo (for
	                                                            // combining collectives)
	                                                            // in subgroup 64 or 65.

	MUSPI_BaseAddressTableSubGroup_t  *_globalBatSubGroups;     // BAT Subgroups 64 and 64.
	uint32_t                          *_globalBatIds;           // BAT ids.

	// Process-wide resources
	MUSPI_RecFifoSubGroup_t           *_globalRecSubGroups;     // Subgroups for cores 0..15.

      }; // ResourceManager class
    }; // MU     namespace
  };   // Device namespace
};     // PAMI   namespace


/// \brief Choose Fifo
///
/// Select an injection fifo (0..9) based on inputs.
///
uint16_t PAMI::Device::MU::ResourceManager::chooseFifo (size_t  sourceDim,
							ssize_t hops,
							ssize_t sizeHalved,
							bool    isTorus)
{
  // If it is a torus, choose the direction that is within the number of hops
  // allowed by the configuration set up for the hardware cutoffs.
  if ( isTorus )
    {
      bool odd = ((sourceDim & 0x01) == 0x01); // Source node is odd or even in
                                               // the dimension.
      if (odd)
      {
	// We are allowed sizeHalved-1 hops in + and sizeHalved hops in -.
	if ( hops > 0 ) // Is natural direction positive?
	{
	  if ( hops < sizeHalved ) // Within allowed hops for + ?
	    return 0;              // Yes.  Use +.
	  else
	    return 1;              // No.   Use -.
	}
	else // Natural direction is negative
	{
	  if ( abs_x(hops) <= sizeHalved ) // Within allowed hops for - ?
	    return 1;                      // Yes.  Use -.
	  else
	    return 0;                      // No.   Use +.
	}
      }
      else // We are even in this dimension.
      {
	// We are allowed sizeHalved hops in + and sizeHalved-1 hops in -.
	if ( hops > 0 ) // Is natural direction positive?
	{
	  if ( hops <= sizeHalved ) // Within allowed hops for + ?
	    return 0;               // Yes.  Use +.
	  else
	    return 1;               // No.   Use -.
	}
	else // Natural direction is negative
	{
	  if ( abs_x(hops) < sizeHalved ) // Within allowed hops for - ?
	    return 1;                     // Yes.  Use -.
	  else
	    return 0;                     // No.   Use +.
	}
      }
    }
    // It is a mesh.  Go in natural direction.
    else return ( hops > 0 ) ? 0 : 1;

} // End: chooseFifo()


/// \brief Pin Fifo
///
/// For the specified task, determine the optimal injection fifo to use
/// to communicate with that task.  This is based on there being an
/// optimal number of injection fifos.
///
uint16_t PAMI::Device::MU::ResourceManager::pinFifo( size_t task )
{
  // Get our torus coords
  size_t ourA = _mapping.a();
  size_t ourB = _mapping.b();
  size_t ourC = _mapping.c();
  size_t ourD = _mapping.d();
  size_t ourE = _mapping.e();

  // Get the destination task's torus coords (addr[0..4]) and t coordinate (addr[5]).
  size_t addr[BGQ_TDIMS + BGQ_LDIMS];
  _mapping.task2global( task, addr );

  TRACE((stderr,"MU ResourceManager: pinFifo: task=%zu,destA=%zu, B=%zu, C=%zu, D=%zu, E=%zu\n",task,addr[0],addr[1],addr[2],addr[3],addr[4]));

  // Calculate the signed number of hops between our task and the destination task
  // in each dimension
  ssize_t dA = addr[0] - ourA;
  ssize_t dB = addr[1] - ourB;
  ssize_t dC = addr[2] - ourC;
  ssize_t dD = addr[3] - ourD;
  ssize_t dE = addr[4] - ourE;

  TRACE((stderr,"MU ResourceManager: pinFifo: ourA=%zu,ourB=%zu,ourC=%zu,ourD=%zu,ourE=%zu, dA=%zd,dB=%zd,dC=%zd,dD=%zd,dE=%zd,t=%zd\n",ourA,ourB,ourC,ourD,ourE,dA,dB,dC,dD,dE,addr[5]));

  // If local, select the fifo based on the T coordinate.
  if ( dA == 0 && dB == 0 && dC == 0 && dD == 0 && dE == 0 )
    {
      return ( 10 + (addr[5] % 6) );
    }

  // If communicating only along the A dimension, select either the A- or A+ fifo
  if ( dB == 0 && dC == 0 && dD == 0 && dE == 0 )
  {
    return ( chooseFifo (ourA,
			 dA,
			 _aSizeHalved,
			 _isTorusA) ? 0 : 1 ); // Return A- if chooseFifo
                                               // returns 1, else return A+.
  }

  // If communicating only along the B dimension, select either the B- or B+ fifo
  if ( dA == 0 && dC == 0 && dD == 0 && dE == 0 )
  {
    return ( chooseFifo (ourB,
			 dB,
			 _bSizeHalved,
			 _isTorusB) ? 2 : 3 ); // Return B- if chooseFifo
                                               // returns 1, else return B+.
  }

  // If communicating only along the C dimension, select either the C- or C+ fifo
  if ( dA == 0 && dB == 0 && dD == 0 && dE == 0 )
  {
    return ( chooseFifo (ourC,
			 dC,
			 _cSizeHalved,
			 _isTorusC) ? 4 : 5 ); // Return C- if chooseFifo
                                               // returns 1, else return C+.
  }

  // If communicating only along the D dimension, select either the D- or D+ fifo
  if ( dA == 0 && dB == 0 && dC == 0 && dE == 0 )
  {
    return ( chooseFifo (ourD,
			 dD,
			 _dSizeHalved,
			 _isTorusD) ? 6 : 7 ); // Return D- if chooseFifo
                                               // returns 1, else return D+.
  }

  // If communicating only along the E dimension, select either the E- or E+ fifo
  if ( dA == 0 && dB == 0 && dC == 0 && dD == 0 )
  {
    return ( chooseFifo (ourE,
			 dE,
			 _eSizeHalved,
			 _isTorusE) ? 8 : 9 ); // Return E- if chooseFifo
                                               // returns 1, else return E+.
  }

  // Communicating along a diagonal or to ourself.
  // Sum the distance between our node and the destination node, and map that
  // number to one of the fifos.  This is essentially a random fifo that is
  // chosen.
  uint16_t loc = abs_x(dA) + abs_x(dB) + abs_x(dC) + abs_x(dD) + abs_x(dE);
  return loc % numTorusDirections;

} // End: pinFifo()


/// \brief Init Fifo Pin
///
/// For each task, calculate the optimal injection fifo to use when sending to
/// that task, and cache it in the mapcache.  This will be used later during
/// fifo pinning.
///
/// The high-order bit of the A,B,C, and D coordinate in each map cache entry
/// will house this optimal fifo number.  This gives us 4 bits for a total of
/// 16 possible values.
/// - Values 0-9 for torus transfers, indicating which of the 10 inj fifos to use.
/// - Values 10-15 for local transfers, indicating which of 6 inj fifos to use
///   (ran out of bits, or it would also be 10 values).
///
/// In addition to calculating the above "fifoPin" value for the mapcache,
/// adjust the pinInjFifoMap and pinBroadcastFifoMap arrays based on our
/// T coordinate.  Refer to the details in the comments where each
/// of these arrays is delcared.
///
void PAMI::Device::MU::ResourceManager::initFifoPin()
{
  size_t numTasks = _mapping.size();
  size_t i;
  size_t startingAdjustmentIndex;
  size_t tcoord = _mapping.t();
  size_t task;
  size_t numFifos;

  _aSizeHalved = _pers.aSize()>>1;
  _bSizeHalved = _pers.bSize()>>1;
  _cSizeHalved = _pers.cSize()>>1;
  _dSizeHalved = _pers.dSize()>>1;
  _eSizeHalved = _pers.eSize()>>1;

  _isTorusA    = _pers.isTorusA();
  _isTorusB    = _pers.isTorusB();
  _isTorusC    = _pers.isTorusC();
  _isTorusD    = _pers.isTorusD();
  _isTorusE    = _pers.isTorusE();

  TRACE((stderr,"MU ResourceManager:: initFifoPin:  aH=%zd,bH=%zd,cH=%zd,dH=%zd,eH=%zd, aT=%d,bT=%d,cT=%d,dT=%d,eT=%d\n",_aSizeHalved,_bSizeHalved,_cSizeHalved,_dSizeHalved,_eSizeHalved,_isTorusA,_isTorusB,_isTorusC,_isTorusD,_isTorusE));

  for ( task=0; task<numTasks; task++ )
    {
      uint16_t fifo = pinFifo( task );
      PAMI_assert( fifo < 16 );
      TRACE((stderr,"MU ResourceManager: initFifoPin: task=%zu, fifo=%u\n",task,fifo));

      _mapping.setFifoPin( task, fifo );
    }

  // Copy the pinInfo arrary into this process' storage.  These contain
  // initial fifo pin values that will be modified later.
  posix_memalign ( (void **)&_pinInfo, 64, numTorusDirections * sizeof(pinInfoEntry_t) );
  PAMI_assertf( _pinInfo != NULL, "The heap is full.\n" );

  memcpy ( _pinInfo, pinInfo, numTorusDirections * sizeof(pinInfoEntry_t) );
  memcpy ( _pinBroadcastFifoMap, pinBroadcastFifoMap, sizeof(_pinBroadcastFifoMap) );

  // Adjust the local injFifoIds (array elements 10-15) based on our T coordinate.
  // This is an attempt to spread the local traffic among all of the inj fifos.
  // Refer to comments where pinInfo is declared.
  for ( numFifos=1; numFifos <= numTorusDirections; numFifos++ )
    {
      for ( i=10; i<16; i++ )
	{
	  _pinInfo[numFifos-1].injFifoIds[i] =
	    ( _pinInfo[numFifos-1].injFifoIds[i] + ( 6*tcoord ) ) % numFifos;
	}
    }
  // Adjust the torus injFifoIds so the directions that don't have
  // their own inj fifo map to one of the other fifos based on our T coordinate.
  // Refer to comments where pinInjFifoMap is declared.
  TRACE((stderr,"MU ResourceManager: initFifoPin: Adjusted pinInjFifoMap values:\n"));
  for ( numFifos=1; numFifos <= numTorusDirections; numFifos++ )
    {
      // Only adjust the directions that are unbalanced.
      // For example, if there are 4 inj fifos, the first 8 values are
      // balanced (0,1,2,3,0,1,2,3) but the last 2 are not, and need to be
      // adjusted.  T=0 will set them to (0,1), T=1 will set them to (2,3),
      // T=2 will set them to (0,1), and so on.  In this case, the
      // startingAdjustmentIndex is 8, so that only indices 8 and 9 are adjusted.
      startingAdjustmentIndex =
	numTorusDirections -
	( numTorusDirections -
	  ( (numTorusDirections / numFifos) * numFifos ) );
      for ( i=startingAdjustmentIndex; i<numTorusDirections; i++ )
	{
	  // Note:  numTorusDirections-startingAdjustmentIndex is the number of overstressed fifos.
	  _pinInfo[numFifos-1].injFifoIds[i] =
	    ( _pinInfo[numFifos-1].injFifoIds[i] +
	      ( (numTorusDirections-startingAdjustmentIndex)*tcoord ) ) % numFifos;
	}
      TRACE((stderr,"For %zu fifos: %u,%u,%u,%u,%u,%u,%u,%u,%u,%u, %u,%u,%u,%u,%u,%u\n",
	     numFifos,
	     _pinInfo[numFifos-1].injFifoIds[0],
	     _pinInfo[numFifos-1].injFifoIds[1],
	     _pinInfo[numFifos-1].injFifoIds[2],
	     _pinInfo[numFifos-1].injFifoIds[3],
	     _pinInfo[numFifos-1].injFifoIds[4],
	     _pinInfo[numFifos-1].injFifoIds[5],
	     _pinInfo[numFifos-1].injFifoIds[6],
	     _pinInfo[numFifos-1].injFifoIds[7],
	     _pinInfo[numFifos-1].injFifoIds[8],
	     _pinInfo[numFifos-1].injFifoIds[9],
	     _pinInfo[numFifos-1].injFifoIds[10],
	     _pinInfo[numFifos-1].injFifoIds[11],
	     _pinInfo[numFifos-1].injFifoIds[12],
	     _pinInfo[numFifos-1].injFifoIds[13],
	     _pinInfo[numFifos-1].injFifoIds[14],
	     _pinInfo[numFifos-1].injFifoIds[15]));
    }

  // Adjust the torus pinBroadcastFifoMap values so the class routes that don't have
  // their own inj fifo map to one of the other fifos based on our T coordinate.
  // Refer to comments where pinBroadcastFifoMap is declared.
  TRACE((stderr,"MU ResourceManager: initFifoPin: Adjusted pinBroadcastFifoMap values:\n"));
  for ( numFifos=1; numFifos <= numTorusDirections; numFifos++ )
    {
      // Only adjust the class routes that are unbalanced.
      // For example, if there are 5 inj fifos, the first 15 values are
      // balanced (0,1,2,3,4,0,1,2,3,4,0,1,2,3,4) but the last 1 is not, and it needs to be
      // adjusted.  T=0 will set it to 0, T=1 will set it to 1,
      // T=2 will set it to 2, and so on.  In this case, the
      // startingAdjustmentIndex is 15, so that only index 15 is adjusted.
      startingAdjustmentIndex =
	BGQ_COLL_CLASS_MAX_CLASSROUTES -
	( BGQ_COLL_CLASS_MAX_CLASSROUTES -
	  ( (BGQ_COLL_CLASS_MAX_CLASSROUTES / numFifos) * numFifos ) );
      for ( i=startingAdjustmentIndex; i<BGQ_COLL_CLASS_MAX_CLASSROUTES; i++ )
	{
	  // Note:  BGQ_COLL_CLASS_MAX_CLASSROUTES-startingAdjustmentIndex is the
	  //        number of overstressed fifos.
	  _pinBroadcastFifoMap[numFifos-1][i] =
	    ( _pinBroadcastFifoMap[numFifos-1][i] +
	      ( (BGQ_COLL_CLASS_MAX_CLASSROUTES-startingAdjustmentIndex)*tcoord ) ) % numFifos;
	}
      TRACE((stderr,"For %zu fifos: %u,%u,%u,%u,%u,%u,%u,%u,%u,%u, %u,%u,%u,%u,%u,%u\n",
	     numFifos,
	     _pinBroadcastFifoMap[numFifos-1][0],
	     _pinBroadcastFifoMap[numFifos-1][1],
	     _pinBroadcastFifoMap[numFifos-1][2],
	     _pinBroadcastFifoMap[numFifos-1][3],
	     _pinBroadcastFifoMap[numFifos-1][4],
	     _pinBroadcastFifoMap[numFifos-1][5],
	     _pinBroadcastFifoMap[numFifos-1][6],
	     _pinBroadcastFifoMap[numFifos-1][7],
	     _pinBroadcastFifoMap[numFifos-1][8],
	     _pinBroadcastFifoMap[numFifos-1][9],
	     _pinBroadcastFifoMap[numFifos-1][10],
	     _pinBroadcastFifoMap[numFifos-1][11],
	     _pinBroadcastFifoMap[numFifos-1][12],
	     _pinBroadcastFifoMap[numFifos-1][13],
	     _pinBroadcastFifoMap[numFifos-1][14],
	     _pinBroadcastFifoMap[numFifos-1][15]));
    }


} // End: initFifoPin()


// \brief Calculate Per Core MU Resources Based On Availability
//
// Determine how many MU resources have already been consumed by the SPI user.
// The core with the minimum number of available MU resources is our model.
// Calculate the available MU resources per core:
// - number of inj fifos available per core
// - number of rec fifos available per core
void PAMI::Device::MU::ResourceManager::calculatePerCoreMUResourcesBasedOnAvailability()
{
  size_t minFreeInjFifosPerCore = BGQ_MU_NUM_INJ_FIFOS_PER_GROUP;
  size_t minFreeRecFifosPerCore = BGQ_MU_NUM_REC_FIFOS_PER_GROUP;
  size_t i;
  int32_t rc;
  uint32_t fifoids[BGQ_MU_NUM_REC_FIFOS_PER_SUBGROUP];

  // Examine each core's MU resources (not 17th core), and determine the
  // minimum over all of the cores.
  uint32_t group;        // Global group number
  uint32_t subgroup = 0; // Global subgroup number
  for (group=0; group<(BGQ_MU_NUM_FIFO_GROUPS-1); group++)
    {
      size_t numFreeInjFifosInGroup = 0;
      size_t numFreeRecFifosInGroup = 0;

      // Loop through the subgroups in this group
      for (i=0; i<BGQ_MU_NUM_FIFO_SUBGROUPS; i++)
	{
	  uint32_t numFreeResourcesInSubgroup;

	  rc = Kernel_QueryInjFifos( subgroup,
				     &numFreeResourcesInSubgroup,
				     fifoids );
	  PAMI_assertf( rc == 0, "Kernel_QueryInjFifos failed with rc=%d.\n",rc );

	  numFreeInjFifosInGroup += numFreeResourcesInSubgroup;

	  rc = Kernel_QueryRecFifos( subgroup,
				     &numFreeResourcesInSubgroup,
				     fifoids );
	  PAMI_assertf( rc == 0, "Kernel_QueryRecFifos failed with rc=%d.\n",rc);

	  numFreeRecFifosInGroup += numFreeResourcesInSubgroup;

	  subgroup++;
	}
      if ( numFreeInjFifosInGroup < minFreeInjFifosPerCore )
	minFreeInjFifosPerCore = numFreeInjFifosInGroup;
    }
  _perCoreMUResourcesBasedOnAvailability.numInjFifos = minFreeInjFifosPerCore;
  _perCoreMUResourcesBasedOnAvailability.numRecFifos = minFreeRecFifosPerCore;

  TRACE((stderr,"MU ResourceManager: _perCoreMUResourcesBasedOnAvailability.numInjFifos = %zu, _perCoreMUResourcesBasedOnAvailability.numRecFifos = %zu\n",_perCoreMUResourcesBasedOnAvailability.numInjFifos, _perCoreMUResourcesBasedOnAvailability.numRecFifos));

} // End: calculatePerCoreMUResourcesBasedOnAvailability()


// \brief Calculate Per Core MU Resources Based On Configuration
//
// Determine how many resources per core are reserved for the SPI user based on
// the specified configuration info.
// Calculate the available resources per core:
// - number of inj fifos available per core
// - number of rec fifos available per core
void PAMI::Device::MU::ResourceManager::calculatePerCoreMUResourcesBasedOnConfig()
{
  size_t numSpiUserInjFifosPerProcess = _pamiRM.getNumSpiUserInjFifosPerProcess();
  size_t numSpiUserInjFifos =
    numSpiUserInjFifosPerProcess / _pamiRM.getNumCoresPerProcess();
  if ( ( numSpiUserInjFifos == 0 ) && ( numSpiUserInjFifosPerProcess > 0 ) )
    numSpiUserInjFifos = 1;

  numSpiUserInjFifos = numSpiUserInjFifos * _pamiRM.getNumProcessesPerCore();

  size_t numSpiUserRecFifosPerProcess = _pamiRM.getNumSpiUserRecFifosPerProcess();
  size_t numSpiUserRecFifos =
    numSpiUserRecFifosPerProcess / _pamiRM.getNumCoresPerProcess();
  if ( ( numSpiUserRecFifos == 0 ) && ( numSpiUserRecFifosPerProcess > 0 ) )
    numSpiUserRecFifos = 1;

  numSpiUserRecFifos = numSpiUserRecFifos * _pamiRM.getNumProcessesPerCore();

  _perCoreMUResourcesBasedOnConfig.numInjFifos = BGQ_MU_NUM_INJ_FIFOS_PER_GROUP - numSpiUserInjFifos;

  _perCoreMUResourcesBasedOnConfig.numRecFifos = BGQ_MU_NUM_REC_FIFOS_PER_GROUP - numSpiUserRecFifos;

  TRACE((stderr,"MU ResourceManager: _perCoreMUResourcesBasedOnConfig.numInjFifos = %zu, _perCoreMUResourcesBasedOnConfig.numRecFifos = %zu\n", _perCoreMUResourcesBasedOnConfig.numInjFifos, _perCoreMUResourcesBasedOnConfig.numRecFifos));

} // End: calculatePerCoreMUResourcesBasedOnConfig()


// \brief Calculate Per Core MU Resources
//
// Calculate the availability of resources based on what has already been
// allocated by the SPI user, and what has been specified in the configuration
// options.
void PAMI::Device::MU::ResourceManager::calculatePerCoreMUResources()
{
  calculatePerCoreMUResourcesBasedOnAvailability();
  calculatePerCoreMUResourcesBasedOnConfig();

  // Our available resources is the minimum of the available resources and what
  // has been specified in configuration options.

  _perCoreMUResources.numInjFifos = _perCoreMUResourcesBasedOnAvailability.numInjFifos;
  if ( _perCoreMUResourcesBasedOnConfig.numInjFifos < _perCoreMUResources.numInjFifos )
    _perCoreMUResources.numInjFifos = _perCoreMUResourcesBasedOnConfig.numInjFifos;

  _perCoreMUResources.numRecFifos = _perCoreMUResourcesBasedOnAvailability.numRecFifos;
  if ( _perCoreMUResourcesBasedOnConfig.numRecFifos < _perCoreMUResources.numRecFifos )
    _perCoreMUResources.numRecFifos = _perCoreMUResourcesBasedOnConfig.numRecFifos;

  TRACE((stderr,"MU ResourceManager: _perCoreMUResources.numInjFifos = %zu, _perCoreMUResources.numRecFifos = %zu\n", _perCoreMUResources.numInjFifos, _perCoreMUResources.numRecFifos));

} // End: calculatePerCoreMUResources()


// \brief Calculate Per Core Per Process MU Resources
//
// Based on the availability of resources per core, calculate the availability
// of resources per process per core.
void PAMI::Device::MU::ResourceManager::calculatePerCorePerProcessMUResources()
{
  calculatePerCoreMUResources();

  size_t numProcessesPerCore = _pamiRM.getNumProcessesPerCore();

  _perCorePerProcessMUResources.numInjFifos =
    _perCoreMUResources.numInjFifos / numProcessesPerCore;

  PAMI_assertf( _perCorePerProcessMUResources.numInjFifos > 0, "No injection fifos available for PAMI\n" );

  _perCorePerProcessMUResources.numRecFifos =
    _perCoreMUResources.numRecFifos / numProcessesPerCore;

  PAMI_assertf( _perCorePerProcessMUResources.numRecFifos > 0, "No reception fifos available for PAMI\n" );

  TRACE((stderr,"MU ResourceManager: _perCorePerProcessMUResources.numInjFifos = %zu, _perCorePerProcessMUResources.numRecFifos = %zu\n",_perCorePerProcessMUResources.numInjFifos,_perCorePerProcessMUResources.numRecFifos));
}


void PAMI::Device::MU::ResourceManager::dividePAMIResourceAmongClients(
						       pamiResources_t &resourceValue,
						       size_t           numClients,
						       pamiResources_t *clientResource )
{
  size_t resourceRemaining = resourceValue.numContexts;
  size_t i;
  for (i=0; i<numClients; i++)
    {
      double numContexts = ( (double)resourceValue.numContexts *
			     (double)_pamiRM.getClientWeight(i) ) / (double)100;
      size_t numContexts_int = numContexts;
      if ( ( numContexts - (double)numContexts_int ) >= 0.5 ) numContexts_int++; // Round up.
      if ( numContexts_int == 0 ) numContexts_int = 1; // Don't let it be zero.
      while ( ( resourceRemaining - numContexts_int ) <
	      ( numClients - i - 1 ) ) // Not enough remain to go around?
	numContexts_int--;
      if ( numContexts_int > resourceRemaining )
	numContexts_int = resourceRemaining;
      resourceRemaining -= numContexts_int;
      clientResource[i].numContexts = numContexts_int;
    }
}


void PAMI::Device::MU::ResourceManager::divideMUResourceAmongClients(
						       muResources_t &resourceValue,
						       size_t         numClients,
						       muResources_t *clientResource )
{
  // Divide Number of Injection Fifos
  size_t resourceRemaining = resourceValue.numInjFifos;
  size_t i;
  for (i=0; i<numClients; i++)
    {
      double numInjFifos = ( (double)resourceValue.numInjFifos *
			     (double)_pamiRM.getClientWeight(i) ) / (double)100;
      size_t numInjFifos_int = numInjFifos;
      if ( ( numInjFifos - (double)numInjFifos_int ) >= 0.5 ) numInjFifos_int++; // Round up.
      if ( numInjFifos_int == 0 ) numInjFifos_int = 1; // Don't let it be zero.
      while ( ( resourceRemaining - numInjFifos_int ) <
	      ( numClients - i - 1 ) ) // Not enough remain to go around?
	numInjFifos_int--;
      if ( numInjFifos_int > resourceRemaining )
	numInjFifos_int = resourceRemaining;
      resourceRemaining -= numInjFifos_int;
      clientResource[i].numInjFifos = numInjFifos_int;
    }

  // Divide Number of Reception Fifos
  resourceRemaining = resourceValue.numRecFifos;
  for (i=0; i<numClients; i++)
    {
      double numRecFifos = ( (double)resourceValue.numRecFifos *
			     (double)_pamiRM.getClientWeight(i) ) / (double)100;
      size_t numRecFifos_int = numRecFifos;
      if ( ( numRecFifos - (double)numRecFifos_int ) >= 0.5 ) numRecFifos_int++; // Round up.
      if ( numRecFifos_int == 0 ) numRecFifos_int = 1; // Don't let it be zero.
      while ( ( resourceRemaining - numRecFifos_int ) <
	      ( numClients - i - 1 ) ) // Not enough remain to go around?
	numRecFifos_int--;
      if ( numRecFifos_int > resourceRemaining )
	numRecFifos_int = resourceRemaining;
      resourceRemaining -= numRecFifos_int;
      clientResource[i].numRecFifos = numRecFifos_int;
    }
}


void PAMI::Device::MU::ResourceManager::calculatePerProcessMaxPamiResources()
{
  size_t i;
  calculatePerCorePerProcessMUResources();

  size_t numClients = _pamiRM.getNumClients();

  // Allocate space for the array of MaxPamiResources.  Free any previous.
  if ( _perProcessMaxPamiResources ) free( _perProcessMaxPamiResources );
  _perProcessMaxPamiResources = (pamiResources_t *)malloc( numClients * sizeof( *_perProcessMaxPamiResources ) );
  PAMI_assertf ( _perProcessMaxPamiResources != NULL, "The heap is full.\n" );

  // Compute the max number of contexts per process as the minimum of the
  // number of inj and rec fifos per process.
  size_t totalNumInjFifosPerProcess = _perCorePerProcessMUResources.numInjFifos *
                                        _pamiRM.getNumCoresPerProcess();
  size_t totalNumRecFifosPerProcess = _perCorePerProcessMUResources.numRecFifos *
                                        _pamiRM.getNumCoresPerProcess();
  pamiResources_t maxContextsPerProcess;
  maxContextsPerProcess.numContexts = totalNumInjFifosPerProcess;
  if ( totalNumRecFifosPerProcess < maxContextsPerProcess.numContexts )
    maxContextsPerProcess.numContexts = totalNumRecFifosPerProcess;

  dividePAMIResourceAmongClients( maxContextsPerProcess,
				  numClients,
				  _perProcessMaxPamiResources );

  for (i=0; i<numClients; i++)
    {
      TRACE((stderr,"MU ResourceManager: _perProcessMaxPamiResources[%lu].numContexts = %lu\n",i,_perProcessMaxPamiResources[i].numContexts));
    }

} // End: calculatePerProcessMaxPamiResources()


void PAMI::Device::MU::ResourceManager::calculatePerProcessOptimalPamiResources()
{
  size_t numClients = _pamiRM.getNumClients();
  size_t numInjContextsPerCore;
  size_t numRecContextsPerCore;
  size_t numContextsPerCore;
  size_t i;
  size_t numCoresPerProcess = _pamiRM.getNumCoresPerProcess();

  // Allocate space for the array of OptimalPamiResources.  Free any previous.
  if ( _perProcessOptimalPamiResources ) free( _perProcessOptimalPamiResources );
  _perProcessOptimalPamiResources = (pamiResources_t *)malloc( numClients * sizeof( *_perProcessOptimalPamiResources ) );
  PAMI_assertf ( _perProcessOptimalPamiResources != NULL, "The heap is full.\n" );

  // Calculate optimal number of Inj and Rec contexts per process
  size_t clientOptimalNumInjFifosPerContext = optimalNumInjFifosPerContext;
  size_t numInjFifosPerCore = _perCorePerProcessMUResources.numInjFifos;
  while ( clientOptimalNumInjFifosPerContext > 0 )
    {
      numInjContextsPerCore = numInjFifosPerCore / clientOptimalNumInjFifosPerContext;
      if ( numInjContextsPerCore > 0 ) break;
      clientOptimalNumInjFifosPerContext--;
    }
  PAMI_assertf ( clientOptimalNumInjFifosPerContext > 0, "Not enough injection fifos are available.\n" );

  size_t clientOptimalNumRecFifosPerContext = optimalNumRecFifosPerContext;
  size_t numRecFifosPerCore = _perCorePerProcessMUResources.numRecFifos;
  while (1)
    {
      numRecContextsPerCore = numRecFifosPerCore / clientOptimalNumRecFifosPerContext;
      if ( numRecContextsPerCore > 0 ) break;
      clientOptimalNumRecFifosPerContext--;
    }
  PAMI_assertf ( clientOptimalNumRecFifosPerContext > 0, "Not enough reception fifos are available.\n" );

  // Calculate optimal number of contexts per process as the minimum of the
  // Inj and Rec.
  numContextsPerCore = numInjContextsPerCore;
  if ( numRecContextsPerCore < numContextsPerCore ) numContextsPerCore = numRecContextsPerCore;
  pamiResources_t numContextsPerProcess;
  numContextsPerProcess.numContexts = numContextsPerCore * numCoresPerProcess;

  dividePAMIResourceAmongClients( numContextsPerProcess,
				  numClients,
				  _perProcessOptimalPamiResources );

  for (i=0; i<numClients; i++)
    {
      TRACE((stderr,"MU ResourceManager: _perProcessOptimalPamiResources[%lu].numContexts = %lu\n",i,_perProcessOptimalPamiResources[i].numContexts));
    }

} // End: calculatePerProcessOptimalPamiResources()


void PAMI::Device::MU::ResourceManager::calculatePerCorePerProcessPerClientMUResources()
{
  size_t i;
  size_t numClients = _pamiRM.getNumClients();

  // Allocate space for the client array
  _perCorePerProcessPerClientMUResources = (muResources_t *)malloc( numClients * sizeof( *_perCorePerProcessPerClientMUResources ) );
  PAMI_assertf ( _perCorePerProcessPerClientMUResources != NULL, "The heap is full.\n" );

  divideMUResourceAmongClients( _perCorePerProcessMUResources,
				numClients,
				_perCorePerProcessPerClientMUResources );

  for (i=0; i<numClients; i++)
    {
      TRACE((stderr,"MU ResourceManager: _perCorePerProcessPerClientMUResources[%lu].numInjFifos = %lu, .numRecFifos = %lu\n",i,_perCorePerProcessPerClientMUResources[i].numInjFifos,_perCorePerProcessPerClientMUResources[i].numRecFifos));
    }

} // End: calculatePerCorePerProcessPerClientMUResources()


// \brief Setup Shared Memory
//
// Set up a shared memory area for global resources to use.
//
void PAMI::Device::MU::ResourceManager::setupSharedMemory()
{
  int fd, rc = -1;
  const char *shmemfile = "/unique-pami-globalRM-shmem-file";
  void *ptr = NULL;

  fd = shm_open (shmemfile, O_CREAT | O_RDWR, 0600);
  TRACE((stderr, "MU ResourceManager: setupSharedMemory: After shm_open, fd = %d\n", fd));

  // There is shared memory...
  if (fd != -1)
    {
      rc = ftruncate( fd, _memSize );
      TRACE((stderr, "MU ResourceManager: setupSharedMemory: After ftruncate(%d,%zu), rc = %d\n", fd, _memSize, rc));

      if (rc != -1)
	{
	  ptr = mmap( NULL, _memSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	  TRACE((stderr, "MU ResourceManager: setupSharedMemory: After mmap, ptr = %p\n", ptr));

	  if (ptr != MAP_FAILED)
	    {
	      TRACE((stderr, "MU ResourceManager: setupSharedMemory: Shmem file <%s> %zu bytes mapped at %p\n", shmemfile, _memSize, ptr));
	      DUMP_HEXDATA("Shared memory map", (const uint32_t *)ptr, 16);
	      _mm.init(ptr, _memSize);
	    }
	  else
	    {
	      rc = -1;
	    }
	}
    }

  // There is not shared memory...
  if (rc == -1)
    {
      fprintf(stderr, "MU ResourceManager: setupSharedMemory: %s:%d Failed to create shared memory (rc=%d, ptr=%p, _memSize=%zu) errno %d %s\n", __FILE__, __LINE__, rc, ptr, _memSize, errno, strerror(errno));
      // There was a failure obtaining the shared memory segment, most
      // likely because the application is running in SMP mode. Allocate
      // memory from the heap instead.
      //
      // TODO - verify the run mode is actually SMP.
      posix_memalign ((void **)&ptr, 16, _memSize);
      memset (ptr, 0, _memSize);
      _mm.init(ptr, _memSize);
    }

} // End: setupSharedMemory()


void PAMI::Device::MU::ResourceManager::allocateMemory( bool    useSharedMemory,
							void ** memptr,
							size_t  alignment,
							size_t  bytes )
{
  if ( useSharedMemory )
    {
      _mm.memalign( memptr, alignment, bytes );
    }
  else
    {
      posix_memalign( memptr, alignment, bytes );
    }
} // End:  allocateMemory()


// \brief Setup Injection Fifos
//
// Allocates and enables the specified injection fifos.  It will get them
// from the specified range of subgroups.
//
// \param[in]  startingSubgroup  Starting subgroup number
// \param[in]  endingSubgroup    Ending subgroup number
// \param[in]  numFifos          Number of fifos to set up
// \param[in]  fifoSize          The size of the fifos (all are same)
// \param[in]  fifoAttr          Pointer to the attributes of the fifos (all are same)
// \param[in]  useSharedMemory   Boolean indicating whether to allocate
//                               resources from shared memory (true) or heap (false).
// \param[out] subgroups         Pointer to an array of subgroup structures.
//                               This array is malloc'd and initialized
//                               by this function, and this pointer is set
//                               to point to that array.
// \param[out] fifoPtrs          Pointer to an array of injection fifos.
//                               This array is malloc'd and the fifos are
//                               malloc'd and anchored in thie array.
//                               by this function, and this pointer is set
//                               to point to that array.
// \param[out] globalFifoIds     Pointer to an array of fifo ids.
//                               This array is malloc'd and initialized
//                               by this function, and this pointer is set
//                               to point to that array.
//
// \retval  numFifosSetUp
//
uint32_t PAMI::Device::MU::ResourceManager::setupInjFifos(
                       uint32_t startingSubgroup,
		       uint32_t endingSubgroup,
		       size_t   numFifos,
		       size_t   fifoSize,
		       Kernel_InjFifoAttributes_t  *fifoAttr,
		       bool                         useSharedMemory,
		       MUSPI_InjFifoSubGroup_t    **subgroups,
		       char                      ***fifoPtrs,
		       uint32_t                   **globalFifoIds)
{
  uint32_t subgroup, fifo;
  int rc;
  uint32_t numFree;
  uint32_t freeIds[BGQ_MU_NUM_INJ_FIFOS_PER_SUBGROUP];

  size_t outputStructuresSize = (endingSubgroup-startingSubgroup+1) *
    sizeof(MUSPI_InjFifoSubGroup_t) + 16;
  size_t fifoPtrsArraySize    = numFifos * sizeof(char*) + 16;
  size_t fifosSize            = numFifos * (fifoSize + 64);
  size_t fifoIdsSize          = numFifos * sizeof(uint32_t) + 16;

  if ( _calculateSizeOnly == 1 )
    {
      if ( useSharedMemory )
	{
	  _memSize += outputStructuresSize +
	              fifoPtrsArraySize  +
              	      fifosSize +
	              fifoIdsSize;
	  TRACE((stderr,"MU ResourceManager: setupInjFifos: _memSize = %zu\n",_memSize));
	}
      return 0;

    } // End: _calculateOnly

  // Allocate space for the output structures
  allocateMemory( useSharedMemory, (void **)(subgroups), 16, outputStructuresSize );
  TRACE((stderr,"MU ResourceManager: setupInjFifos: subgroups ptr = %p\n",*subgroups));
  if ( useSharedMemory ) PAMI_assertf( *subgroups != NULL, "Shared memory is full.\n" )
  else PAMI_assertf( *subgroups != NULL, "The heap is full.\n" );

  allocateMemory( useSharedMemory, (void **)(fifoPtrs), 16, fifoPtrsArraySize );
  TRACE((stderr,"MU ResourceManager: setupInjFifos: fifoPtrs ptr = %p\n",*fifoPtrs));
  if ( useSharedMemory ) PAMI_assertf ( *fifoPtrs != NULL, "Shared memory is full.\n" )
  else PAMI_assertf( *fifoPtrs != NULL, "The heap is full.\n" );

  for ( fifo=0; fifo<numFifos; fifo++ )
    {
      allocateMemory( useSharedMemory, (void **)&((*fifoPtrs)[fifo]), 64, fifoSize );
      TRACE((stderr,"MU ResourceManager: setupInjFifos: fifo ptr address = %p, fifoptr = %p\n",&((*fifoPtrs)[fifo]), (*fifoPtrs)[fifo]));
      if ( useSharedMemory ) PAMI_assertf ( (*fifoPtrs)[fifo] != NULL, "Shared memory is full.\n" )
      else PAMI_assertf ( (*fifoPtrs)[fifo] != NULL, "The heap is full.\n" );
    }

  allocateMemory( useSharedMemory, (void **)(globalFifoIds), 16, fifoIdsSize );
  TRACE((stderr,"MU ResourceManager: setupInjFifos: globalFifoIds ptr = %p\n",*globalFifoIds));
  if ( useSharedMemory ) PAMI_assertf ( *globalFifoIds != NULL, "Shared memory is full.\n" )
  else PAMI_assertf ( *globalFifoIds != NULL, "The heap is full.\n" );

  if ( _allocateOnly == 1 ) return 0;

  uint32_t numLeftToAllocate = numFifos;
  uint32_t subgroupIndex     = 0;
  uint32_t fifoIndex         = 0;

  // Loop through each subgroup
  for ( subgroup=startingSubgroup; subgroup<=endingSubgroup; subgroup++ )
    {
      // Determine the free fifos in this subgroup.
      rc = Kernel_QueryInjFifos( subgroup,
				 &numFree,
				 freeIds );
      PAMI_assertf( rc==0, "Kernel_QueryInjFifos failed with rc=%d.\n",rc );
      TRACE((stderr,"MU ResourceManager: setupInjFifos: subgroup = %u, numFree = %u\n",subgroup,numFree));

      if ( numFree == 0 )
	{
	  subgroupIndex++;
	  continue; // Nothing free in this subgroup?  Go to next.
	}

      if ( numFree > numLeftToAllocate ) numFree = numLeftToAllocate;

      Kernel_InjFifoAttributes_t *fifoAttrs =
	(Kernel_InjFifoAttributes_t *)malloc( numFree * sizeof(Kernel_InjFifoAttributes_t) );
      PAMI_assertf( fifoAttrs != NULL, "The heap is full.\n" );
      for (fifo=0; fifo<numFree; fifo++)
	memcpy(&fifoAttrs[fifo],fifoAttr,sizeof(Kernel_InjFifoAttributes_t));

      // Allocate the fifos
      rc = Kernel_AllocateInjFifos( subgroup,
				    &((*subgroups)[subgroupIndex]),
				    numFree,
				    freeIds,
				    fifoAttrs );
      PAMI_assertf( rc == 0, "Kernel_AllocateInjFifos failed with rc=%d, errno=%d.\n",rc,errno );
      TRACE((stderr,"MU ResourceManager: setupInjFifos: Allocated subgroup ptr = %p\n",&((*subgroups)[subgroupIndex])));

      free(fifoAttrs); fifoAttrs=NULL;

      // Init the MU MMIO for the fifos.
      for (fifo=0; fifo<numFree; fifo++)
	{
	  Kernel_MemoryRegion_t memRegion;
	  rc = Kernel_CreateMemoryRegion ( &memRegion,
					   (*fifoPtrs)[fifoIndex+fifo],
					   fifoSize );
	  PAMI_assertf( rc == 0, "Kernel_CreateMemoryRegion failed with rc=%d\n",rc );

	  rc = Kernel_InjFifoInit( &((*subgroups)[subgroupIndex]),
				   freeIds[fifo],
				   &memRegion,
				   (uint64_t)(*fifoPtrs)[fifoIndex+fifo] -
				   (uint64_t)memRegion.BaseVa,
				   fifoSize-1 );
	  PAMI_assertf( rc == 0, "Kernel_InjFifoInit failed with rc=%d\n",rc );

	  (*globalFifoIds)[fifoIndex+fifo] =
	    subgroup * BGQ_MU_NUM_INJ_FIFOS_PER_SUBGROUP + freeIds[fifo];
	  TRACE((stderr,"MU ResourceManager: setupInjFifos: Initialized subgroup ptr = %p, id=%u, fifoSize=%zu, globalId = %u, fifostruct=%p, hw_injfifo=%p\n",&((*subgroups)[subgroupIndex]), freeIds[fifo], fifoSize, (*globalFifoIds)[fifoIndex+fifo], &(((*subgroups)[subgroupIndex])._injfifos[freeIds[fifo]]), (((*subgroups)[subgroupIndex])._injfifos[freeIds[fifo]]).hw_injfifo ));
	}

      // Activate the fifos.
      rc = Kernel_InjFifoActivate( &((*subgroups)[subgroupIndex]),
				   numFree,
				   freeIds,
				   KERNEL_INJ_FIFO_ACTIVATE );
      PAMI_assertf( rc == 0, "Kernel_InjFifoActivate failed with rc=%d\n",rc );

      fifoIndex         += numFree;

      numLeftToAllocate -= numFree;
      if ( numLeftToAllocate == 0 ) break;

      subgroupIndex++;

    } // End: Loop through subgroups

  return ( fifoIndex );

} // End: setupInjFifos()


// \brief Allocate Global Injection Fifos
//
// Allocate resources needed before main().  These include:
// -  1 collective combining injection fifo (somewhere in subgroup 64 or 65).
// - 10 remote get injection fifos (somewhere in subgroups 64 and 65).
//
void PAMI::Device::MU::ResourceManager::allocateGlobalInjFifos()
{
  uint32_t numFifosSetup;
  Kernel_InjFifoAttributes_t  fifoAttr;

  // Set up the combining collective fifo

  memset( &fifoAttr, 0x00, sizeof(fifoAttr) );

  numFifosSetup = setupInjFifos( 64, // Starting subgroup
				 65, // Ending subgroup
				 1,  // Number of fifos
				 _pamiRM.getInjFifoSize(),
				 &fifoAttr,
				 false, // Do not use shared memory
				 &(_globalCombiningInjFifo.subgroups),
				 &(_globalCombiningInjFifo.fifoPtrs),
				 &(_globalCombiningInjFifo.globalFifoIds) );
  PAMI_assertf( (_calculateSizeOnly == 1) || (_allocateOnly == 1) || (numFifosSetup == 1), "Only %u injection fifos were set up.  Expected 1.\n",numFifosSetup );

  if ( (_calculateSizeOnly == 0) && (_allocateOnly == 0) )
    {
      allocateLookasideResources(
				 1, /* numInjFifos */
				 _pamiRM.getRgetInjFifoSize(),
				 &(_globalCombiningInjFifo.lookAsidePayloadPtrs),
				 &(_globalCombiningInjFifo.lookAsidePayloadMemoryRegions),
				 &(_globalCombiningInjFifo.lookAsidePayloadPAs),
				 &(_globalCombiningInjFifo.lookAsideCompletionFnPtrs),
				 &(_globalCombiningInjFifo.lookAsideCompletionCookiePtrs));
    }

  // Set up the remote get fifos

  memset( &fifoAttr, 0x00, sizeof(fifoAttr) );

  fifoAttr.RemoteGet = 1;
  fifoAttr.Priority  = 1;

  numFifosSetup = setupInjFifos( 64, // Starting subgroup
				 65, // Ending subgroup
				 10, // Number of fifos
				 _pamiRM.getRgetInjFifoSize(),
				 &fifoAttr,
				 true, // Use shared memory
				 &_globalRgetInjSubGroups,
				 &_globalRgetInjFifoPtrs,
				 &_globalRgetInjFifoIds );
  PAMI_assertf( (_calculateSizeOnly == 1) || (_allocateOnly == 1) || (numFifosSetup == 10), "Only %u injection fifos were set up.  Expected 10.\n",numFifosSetup );

  // Set up the remote get fifo pin information entry.
  // 0.  Allocate space for the entry.
  // 1.  Copy the torus_inj_fifo_map array to the entry.
  // 2.  Initialize the fifo numbers to the global rget fifo numbers.
  //     Use the same relative rget fifos as the 9 normal fifos.
  if ( _calculateSizeOnly == 1 ) return;

  allocateMemory( false /*do not useSharedMemory*/,
		  (void **)(&_rgetPinInfo),
		  64,
		  sizeof(*_rgetPinInfo) );
  TRACE((stderr,"MU ResourceManager: allocateGlobalInjFifos: _rgetPinInfo = %p\n",_rgetPinInfo));
  PAMI_assertf ( _rgetPinInfo != NULL, "The heap is full.\n" );

  memcpy ( &_rgetPinInfo->torusInjFifoMaps[0],
	   &_pinInfo[9].torusInjFifoMaps[0],
	   sizeof(_rgetPinInfo->torusInjFifoMaps) );

  TRACE((stderr,"MU ResourceManager: allocateGlobalInjFifos: _rgetPinInfo->torusInjFifoMaps{} = 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x, 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",_rgetPinInfo->torusInjFifoMaps[0],_rgetPinInfo->torusInjFifoMaps[1],_rgetPinInfo->torusInjFifoMaps[2],_rgetPinInfo->torusInjFifoMaps[3],_rgetPinInfo->torusInjFifoMaps[4],_rgetPinInfo->torusInjFifoMaps[5],_rgetPinInfo->torusInjFifoMaps[6],_rgetPinInfo->torusInjFifoMaps[7],_rgetPinInfo->torusInjFifoMaps[8],_rgetPinInfo->torusInjFifoMaps[9],_rgetPinInfo->torusInjFifoMaps[10],_rgetPinInfo->torusInjFifoMaps[11],_rgetPinInfo->torusInjFifoMaps[12],_rgetPinInfo->torusInjFifoMaps[13],_rgetPinInfo->torusInjFifoMaps[14],_rgetPinInfo->torusInjFifoMaps[15]));

} // End: allocateGlobalInjFifos()


// \brief Setup Base Address Table Ids
//
// Allocates the specified Base Address Table Ids.  It will get them
// from the specified range of subgroups.
//
// \param[in]  startingSubgroup  Starting subgroup number
// \param[in]  endingSubgroup    Ending subgroup number
// \param[in]  numBatIds         Number of BAT ids to set up
// \param[in]  useSharedMemory   Boolean indicating whether to allocate
//                               resources from shared memory (true) or heap (false).
// \param[out] subgroups         Pointer to an array of subgroup structures.
//                               This array is malloc'd and initialized
//                               by this function, and this pointer is set
//                               to point to that array.
// \param[out] globalBatIds      Pointer to an array of BAT ids.
//                               This array is malloc'd and initialized
//                               by this function, and this pointer is set
//                               to point to that array.
//
// \retval  numBatIdssSetUp
//
uint32_t PAMI::Device::MU::ResourceManager::setupBatIds(
                       uint32_t                           startingSubgroup,
		       uint32_t                           endingSubgroup,
		       uint32_t                           numBatIds,
		       bool                               useSharedMemory,
		       MUSPI_BaseAddressTableSubGroup_t **subgroups,
		       uint32_t                         **globalBatIds)
{
  uint32_t subgroup, batId;
  int rc;
  uint32_t numFree;
  uint32_t freeIds[BGQ_MU_NUM_DATA_COUNTERS_PER_SUBGROUP];

  size_t outputStructuresSize = (endingSubgroup-startingSubgroup+1) *
    sizeof(MUSPI_BaseAddressTableSubGroup_t) + 16;
  size_t batIdsSize           = numBatIds * sizeof(uint32_t) + 16;

  if ( _calculateSizeOnly == 1 )
    {
      _memSize += outputStructuresSize + batIdsSize;
      TRACE((stderr,"MU ResourceManager: setupBatIds: _memSize = %zu\n",_memSize));
      return 0;

    } // End: _calculateOnly

  // Allocate space for the output structures
  allocateMemory( useSharedMemory, (void **)(subgroups), 16, outputStructuresSize );
  TRACE((stderr,"MU ResourceManager: setupBatIds: subgroups ptr = %p\n",*subgroups));
  if ( useSharedMemory ) PAMI_assertf( *subgroups != NULL, "Shared memory is full.\n" )
  else PAMI_assertf( *subgroups != NULL, "The heap is full.\n" );

  allocateMemory( useSharedMemory, (void **)(globalBatIds), 16, batIdsSize );
  TRACE((stderr,"MU ResourceManager: setupBatIds: globalBatIds ptr = %p\n",*globalBatIds));
  if ( useSharedMemory ) PAMI_assertf ( *globalBatIds != NULL, "Shared memory is full.\n" )
  else PAMI_assertf ( *globalBatIds != NULL, "The heap is full.\n" );

  if ( _allocateOnly == 1 ) return 0;

  uint32_t numLeftToAllocate = numBatIds;
  uint32_t subgroupIndex     = 0;
  uint32_t batIdIndex        = 0;

  // Loop through each subgroup
  for ( subgroup=startingSubgroup; subgroup<=endingSubgroup; subgroup++ )
    {
      // Determine the free BAT Ids in this subgroup.
      rc = Kernel_QueryBaseAddressTable( subgroup,
					 &numFree,
					 freeIds );
      PAMI_assertf( rc==0, "Kernel_QueryBaseAddressTable failed with rc=%d\n",rc );
      TRACE((stderr,"MU ResourceManager: setupBatIds: subgroup = %u, numFree = %u\n",subgroup,numFree));

      if ( numFree > numLeftToAllocate ) numFree = numLeftToAllocate;

      // Allocate the BAT Ids
      rc = Kernel_AllocateBaseAddressTable( subgroup,
					    &((*subgroups)[subgroupIndex]),
					    numFree,
					    freeIds,
					    0 /* "User" access */);
      PAMI_assertf( rc == 0, "Kernel_AllocateBaseAddressTable failed with rc=%d\n",rc );
      TRACE((stderr,"MU ResourceManager: setupBatIds: Allocated subgroup ptr = %p\n",&((*subgroups)[subgroupIndex])));

      // Calculate the global BAT ID and return it.
      for ( batId=0; batId<numFree; batId++ )
	{
	  (*globalBatIds)[batIdIndex+batId] =
	    subgroup * BGQ_MU_NUM_DATA_COUNTERS_PER_SUBGROUP + freeIds[batId];
	  TRACE((stderr,"MU ResourceManager: setupBatIds: Initialized subgroup ptr = %p, id=%u, globalId = %u\n",&((*subgroups)[subgroupIndex]), freeIds[batId], (*globalBatIds)[batIdIndex+batId] ));
	}

      batIdIndex += numFree;

      numLeftToAllocate -= numFree;
      if ( numLeftToAllocate == 0 ) break;

      subgroupIndex++;

    } // End: Loop through subgroups

  return ( batIdIndex );

} // End: setupBatIds()


// \brief Allocate Global Base Address Table Entry
//
// Allocate resources needed before main().  These include:
// -  1 base address table entry in subgroup 64 or 65, initialized to 0.
// -  1 base address table entry in subgroup 64 or 65, initialized to the atomic
//    address of a shared counter, whose value is ignored.
//
void PAMI::Device::MU::ResourceManager::allocateGlobalBaseAddressTableEntries()
{
  uint32_t numBatIdsSetup;
  int32_t  rc;

  numBatIdsSetup = setupBatIds( 64, // Starting subgroup
				65, // Ending subgroup
				2,  // Number of BAT ids
				true, // Use shared memory
				&_globalBatSubGroups,
				&_globalBatIds );
  PAMI_assertf( (_calculateSizeOnly == 1) || (_allocateOnly == 1) || (numBatIdsSetup == 2), "Only %u base address Ids were set up.  Expected 2.\n",numBatIdsSetup );

  // Incorporate the size of the shared counter into the shared memory space requirement.
  if ( _calculateSizeOnly == 1 )
    {
      _memSize += sizeof(uint64_t) + 16;
      TRACE((stderr,"MU ResourceManager: allocateGlobalBaseAddressTableEntries: _memSize = %zu\n",_memSize));
      return;

    } // End: _calculateOnly

  // Allocate space for the shared counter in shared memory.
  uint64_t *sharedCounterPtr;
  allocateMemory( true /*useSharedMemory*/, (void **)(&sharedCounterPtr), 8, sizeof(uint64_t) );
  TRACE((stderr,"MU ResourceManager: allocateGlobalBaseAddressTableEntries: Shared counter address = %p\n",sharedCounterPtr));
  PAMI_assertf( sharedCounterPtr != NULL, "Shared memory is full.\n" );

  if ( _allocateOnly == 1 ) return;

  // Set the first BAT slot to a 0 physical address.
  uint32_t batSubgroup = ( _globalBatIds[0] / BGQ_MU_NUM_DATA_COUNTERS_PER_SUBGROUP ) - 64;
  uint8_t  batId       = _globalBatIds[0] % BGQ_MU_NUM_DATA_COUNTERS_PER_SUBGROUP;
  rc = MUSPI_SetBaseAddress ( &_globalBatSubGroups[batSubgroup],
			      batId,
			      0 /* MUHWI_BaseAddress_t */ );
  TRACE((stderr,"MU ResourceManager: allocateGlobalBaseAddressTableEntry: Zero BAT entry: Relative batSubgroup=%u, Relative batId=%u, Global batId=%u, rc=%d\n",batSubgroup, batId, _globalBatIds[0], rc));
  PAMI_assertf( rc == 0, "MUSPI_SetBaseAddress failed with rc=%d\n",rc );

  // Set the second BAT slot to the atomic physical address of the shared counter.
  Kernel_MemoryRegion_t memRegion;
  rc = Kernel_CreateMemoryRegion ( &memRegion,
				   sharedCounterPtr,
				   sizeof(uint64_t) );
  PAMI_assertf( rc == 0, "Kernel_CreateMemoryRegion failed with rc=%d\n",rc );

  uint64_t sharedCounterPA = (uint64_t)memRegion.BasePa +
    ((uint64_t)sharedCounterPtr - (uint64_t)memRegion.BaseVa);

  batSubgroup = ( _globalBatIds[1] / BGQ_MU_NUM_DATA_COUNTERS_PER_SUBGROUP ) - 64;
  batId       = _globalBatIds[1] % BGQ_MU_NUM_DATA_COUNTERS_PER_SUBGROUP;

  uint64_t sharedCounterBATvalue = (uint64_t)MUSPI_GetAtomicAddress (
					       sharedCounterPA,
					       MUHWI_ATOMIC_OPCODE_STORE_ADD_COHERENCE_ON_ZERO);

  rc = MUSPI_SetBaseAddress ( &_globalBatSubGroups[batSubgroup],
			      batId,
			      sharedCounterBATvalue );
  TRACE((stderr,"MU ResourceManager: allocateGlobalBaseAddressTableEntry: Shared Counter BAT entry: Relative batSubgroup=%u, Relative batId=%u, Global batId=%u, rc=%d, Shared Counter PA=0x%lx, BAT value = 0x%lx\n",batSubgroup, batId, _globalBatIds[1], rc, sharedCounterPA, sharedCounterBATvalue ));
  PAMI_assertf( rc == 0, "MUSPI_SetBaseAddress failed with rc=%d\n",rc );

} // End: allocateGlobalBaseAddressTableEntry()


// \brief Setup Reception Fifos
//
// Allocates and enables the specified reception fifos.  It will get them
// from the specified range of subgroups.
//
// \param[in]  startingSubgroup  Starting subgroup number
// \param[in]  endingSubgroup    Ending subgroup number
// \param[in]  numFifos          Number of fifos to set up
// \param[in]  fifoSize          The size of the fifos (all are same)
// \param[in]  fifoAttr          Pointer to the attributes of the fifos (all are same)
// \param[in]  useSharedMemory   Boolean indicating whether to allocate
//                               resources from shared memory (true) or heap (false).
// \param[out] subgroups         Pointer to an array of subgroup structures.
//                               This array is malloc'd and initialized
//                               by this function, and this pointer is set
//                               to point to that array.
// \param[out] fifoPtrs          Pointer to an array of reception fifos.
//                               This array is malloc'd and the fifos are
//                               malloc'd and anchored in thie array.
//                               by this function, and this pointer is set
//                               to point to that array.
// \param[out] globalFifoIds     Pointer to an array of fifo ids.
//                               This array is malloc'd and initialized
//                               by this function, and this pointer is set
//                               to point to that array.
//
// \retval  numFifosSetUp
//
uint32_t PAMI::Device::MU::ResourceManager::setupRecFifos(
                       uint32_t startingSubgroup,
		       uint32_t endingSubgroup,
		       size_t   numFifos,
		       size_t   fifoSize,
		       Kernel_RecFifoAttributes_t  *fifoAttr,
		       bool                         useSharedMemory,
		       MUSPI_RecFifoSubGroup_t    **subgroups,
		       char                      ***fifoPtrs,
		       uint32_t                   **globalFifoIds)
{
  uint32_t subgroup, fifo;
  int rc;
  uint32_t numFree;
  uint32_t freeIds[BGQ_MU_NUM_REC_FIFOS_PER_SUBGROUP];

  size_t outputStructuresSize = (endingSubgroup-startingSubgroup+1) *
    sizeof(MUSPI_RecFifoSubGroup_t) + 16;
  size_t fifoPtrsArraySize    = numFifos * sizeof(char*) + 16;
  size_t fifosSize            = numFifos * (fifoSize + 32);
  size_t fifoIdsSize          = numFifos * sizeof(uint32_t) + 16;

  if ( _calculateSizeOnly == 1 )
    {
      _memSize += outputStructuresSize +
	            fifoPtrsArraySize  +
	            fifosSize +
                    fifoIdsSize;
      TRACE((stderr,"MU ResourceManager: setupRecFifos: _memSize = %zu\n",_memSize));
      return 0;

    } // End: _calculateOnly

  // Allocate space for the output structures
  allocateMemory( useSharedMemory, (void **)(subgroups), 16, outputStructuresSize );
  TRACE((stderr,"MU ResourceManager: setupRecFifos: subgroups ptr = %p\n",*subgroups));
  if ( useSharedMemory ) PAMI_assertf( *subgroups != NULL, "Shared memory is full.\n" )
  else PAMI_assertf( *subgroups != NULL, "The heap is full.\n" );

  allocateMemory( useSharedMemory, (void **)(fifoPtrs), 16, fifoPtrsArraySize );
  TRACE((stderr,"MU ResourceManager: setupRecFifos: fifoPtrs ptr = %p\n",*fifoPtrs));
  if ( useSharedMemory ) PAMI_assertf ( *fifoPtrs != NULL, "Shared memory is full.\n" )
  else PAMI_assertf( *fifoPtrs != NULL, "The heap is full.\n" );

  for ( fifo=0; fifo<numFifos; fifo++ )
    {
      allocateMemory( useSharedMemory, (void **)&((*fifoPtrs)[fifo]), 32, fifoSize );
      TRACE((stderr,"MU ResourceManager: setupRecFifos: fifo ptr address = %p, fifoptr = %p\n",&((*fifoPtrs)[fifo]), (*fifoPtrs)[fifo]));
      if ( useSharedMemory ) PAMI_assertf ( (*fifoPtrs)[fifo] != NULL, "Shared memory is full.\n" )
      else PAMI_assertf ( (*fifoPtrs)[fifo] != NULL, "The heap is full.\n" );
    }

  allocateMemory( useSharedMemory, (void **)(globalFifoIds), 16, fifoIdsSize );
  TRACE((stderr,"MU ResourceManager: setupRecFifos: globalFifoIds ptr = %p\n",*globalFifoIds));
  if ( useSharedMemory ) PAMI_assertf ( *globalFifoIds != NULL, "Shared memory is full.\n" )
  else PAMI_assertf ( *globalFifoIds != NULL, "The heap is full.\n" );

  if ( _allocateOnly == 1 ) return 0;

  uint32_t numLeftToAllocate = numFifos;
  uint32_t subgroupIndex     = 0;
  uint32_t fifoIndex         = 0;

  // Loop through each subgroup
  for ( subgroup=startingSubgroup; subgroup<=endingSubgroup; subgroup++ )
    {
      // Determine the free fifos in this subgroup.
      rc = Kernel_QueryRecFifos( subgroup,
				 &numFree,
				 freeIds );
      PAMI_assertf( rc==0, "Kernel_QueryRecFifos failed with rc=%d.\n",rc );
      TRACE((stderr,"MU ResourceManager: setupRecFifos: subgroup = %u, numFree = %u\n",subgroup,numFree));

      if ( numFree == 0 )
	{
	  subgroupIndex++;
	  continue; // Nothing free in this subgroup?  Go to next.
	}

      if ( numFree > numLeftToAllocate ) numFree = numLeftToAllocate;

      Kernel_RecFifoAttributes_t *fifoAttrs =
	(Kernel_RecFifoAttributes_t *)malloc( numFree * sizeof(Kernel_RecFifoAttributes_t) );
      PAMI_assertf( fifoAttrs != NULL, "The heap is full.\n" );
      for (fifo=0; fifo<numFree; fifo++)
	memcpy(&fifoAttrs[fifo],fifoAttr,sizeof(Kernel_RecFifoAttributes_t));

      // Allocate the fifos
      rc = Kernel_AllocateRecFifos( subgroup,
				    &((*subgroups)[subgroupIndex]),
				    numFree,
				    freeIds,
				    fifoAttrs );
      PAMI_assertf( rc == 0, "Kernel_AllocateRecFifos failed with rc=%d.\n",rc );
      TRACE((stderr,"MU ResourceManager: setupRecFifos: Allocated subgroup ptr = %p\n",&((*subgroups)[subgroupIndex])));

      free(fifoAttrs); fifoAttrs=NULL;

      uint64_t enableBits = 0;

      // Init the MU MMIO for the fifos.
      for (fifo=0; fifo<numFree; fifo++)
	{
	  Kernel_MemoryRegion_t memRegion;
	  rc = Kernel_CreateMemoryRegion ( &memRegion,
					   (*fifoPtrs)[fifoIndex+fifo],
					   fifoSize );
	  PAMI_assertf( rc == 0, "Kernel_CreateMemoryRegion failed with rc=%d\n",rc );

	  rc = Kernel_RecFifoInit( &((*subgroups)[subgroupIndex]),
				   freeIds[fifo],
				   &memRegion,
				   (uint64_t)(*fifoPtrs)[fifoIndex+fifo] -
				   (uint64_t)memRegion.BaseVa,
				   fifoSize-1 );
	  PAMI_assertf( rc == 0, "Kernel_RecFifoInit failed with rc=%d\n",rc );

	  (*globalFifoIds)[fifoIndex+fifo] =
	    subgroup * BGQ_MU_NUM_REC_FIFOS_PER_SUBGROUP + freeIds[fifo];
	  TRACE((stderr,"MU ResourceManager: setupRecFifos: Initialized subgroup ptr = %p, id=%u, fifoSize=%zu, globalId = %u\n",&((*subgroups)[subgroupIndex]), freeIds[fifo], fifoSize, (*globalFifoIds)[fifoIndex+fifo] ));

	  enableBits |= 0x8000ULL >>
	    ( (subgroup % BGQ_MU_NUM_REC_FIFO_SUBGROUPS)*BGQ_MU_NUM_REC_FIFOS_PER_SUBGROUP + freeIds[fifo] );
	}

      // Enable the fifos.
      TRACE((stderr,"MU ResourceManager: setupRecFifos: Enabling RecFifos in group %u, bits 0x%lx\n",subgroup/BGQ_MU_NUM_REC_FIFO_SUBGROUPS,enableBits));
      rc = Kernel_RecFifoEnable( subgroup/BGQ_MU_NUM_REC_FIFO_SUBGROUPS,
				 enableBits );
      PAMI_assertf( rc == 0, "Kernel_RecFifoEnable failed with rc=%d\n",rc );

      fifoIndex         += numFree;

      numLeftToAllocate -= numFree;
      if ( numLeftToAllocate == 0 ) break;

      subgroupIndex++;

    } // End: Loop through subgroups

  return ( fifoIndex );

} // End: setupRecFifos()


// \brief Allocate Global Reception Fifos
//
// Allocate resources needed before main().  These include:
// -  N reception fifos, where N is in the range 1..256.  All available reception
//    fifos will be allocated at this time.
//
void PAMI::Device::MU::ResourceManager::allocateGlobalRecFifos()
{
} // End: allocateGlobalRecFifos()


void PAMI::Device::MU::ResourceManager::allocateLookasideResources(
					  size_t                   numInjFifos,
					  size_t                   injFifoSize,
					  char                  ***lookAsidePayloadPtrs,
					  Kernel_MemoryRegion_t  **lookAsidePayloadMemoryRegions,
					  uint64_t               **lookAsidePayloadPAs,
					  pami_event_function   ***lookAsideCompletionFnPtrs,
					  void                 ****lookAsideCompletionCookiePtrs)
{
  int32_t  rc;
  uint32_t fifo;
  size_t lookAsidePayloadBufferSize = (injFifoSize / sizeof(MUHWI_Descriptor_t)) *
    sizeof(InjGroup::immediate_payload_t);
  size_t lookAsideCompletionFnArraySize = (injFifoSize / sizeof(MUHWI_Descriptor_t)) *
    sizeof(pami_event_function);
  size_t lookAsideCompletionCookieArraySize = (injFifoSize / sizeof(MUHWI_Descriptor_t)) *
    sizeof(void *);

  // Set up array of pointers to the lookaside payload buffers.  There is pointer per inj fifo.
  char **_lookAsidePayloadPtrs;
  allocateMemory( false, // Use heap
		  (void **)(&_lookAsidePayloadPtrs),
		  16,
		  numInjFifos * sizeof(char**) );
  TRACE((stderr,"MU ResourceManager: allocateLookasideResources: lookAsidePayloadPtrs = %p\n",_lookAsidePayloadPtrs));
  PAMI_assertf( _lookAsidePayloadPtrs != NULL, "The heap is full.\n" );
  *lookAsidePayloadPtrs = _lookAsidePayloadPtrs;

  // Allocate each of the lookaside payload buffers.  They have the same number of slots
  // as the inj fifo.
  for ( fifo=0; fifo<numInjFifos; fifo++ )
    {
      allocateMemory( false, // Use heap
		      (void **)&(_lookAsidePayloadPtrs[fifo]),
		      32,
		      lookAsidePayloadBufferSize );
      TRACE((stderr,"MU ResourceManager: allocateLookasideResources: lookAsidePayloadPtr[%u] = %p\n",fifo,_lookAsidePayloadPtrs[fifo]));
      PAMI_assertf ( _lookAsidePayloadPtrs[fifo] != NULL, "The heap is full.\n" );
    }

  // Set up array of memory regions corresponding to the lookaside payload buffers.
  // We need this to get the physical addresses of the lookaside payload buffers.
  // There is 1 memory region per lookaside buffer (ie. per inj fifo).
  Kernel_MemoryRegion_t *_lookAsidePayloadMemoryRegions;
  allocateMemory( false, // Use heap
		  (void **)(&_lookAsidePayloadMemoryRegions),
		  16,
		  numInjFifos * sizeof(Kernel_MemoryRegion_t) );
TRACE((stderr,"MU ResourceManager: allocateLookasideResources: lookAsidePayloadMemoryRegions = %p\n",_lookAsidePayloadMemoryRegions));
  PAMI_assertf( _lookAsidePayloadMemoryRegions != NULL, "The heap is full.\n" );
  *lookAsidePayloadMemoryRegions = _lookAsidePayloadMemoryRegions;

  // Initialize each of the lookaside payload memory regions.
  for ( fifo=0; fifo<numInjFifos; fifo++ )
    {
      rc = Kernel_CreateMemoryRegion ( &_lookAsidePayloadMemoryRegions[fifo],
				       _lookAsidePayloadPtrs[fifo],
				       lookAsidePayloadBufferSize );
      PAMI_assertf( rc==0, "Kernel_CreateMemoryRegion failed with rc=%d\n",rc);
    }

  // Set up array of lookaside payload buffer physical addresses.
  // There is 1 PA for each lookaside buffer (ie. for each inj fifo).
  uint64_t *_lookAsidePayloadPAs;
  allocateMemory( false, // Use heap
		  (void **)(&_lookAsidePayloadPAs),
		  16,
		  numInjFifos * sizeof(uint64_t) );
  TRACE((stderr,"MU ResourceManager: allocateLookasideResources: lookAsidePayloadPAs = %p\n",_lookAsidePayloadPAs));
  PAMI_assertf( _lookAsidePayloadPAs != NULL, "The heap is full.\n" );
  *lookAsidePayloadPAs = _lookAsidePayloadPAs;

  // Calculate the lookaside payload PAs.
  for ( fifo=0; fifo<numInjFifos; fifo++ )
    {
      _lookAsidePayloadPAs[fifo] =
	(uint64_t)_lookAsidePayloadPtrs[fifo] -
	(uint64_t)_lookAsidePayloadMemoryRegions[fifo].BaseVa +
	(uint64_t)_lookAsidePayloadMemoryRegions[fifo].BasePa;
      TRACE((stderr,"MU ResourceManager: allocateLookasideResources: lookAsidePayloadPAs[%u] = 0x%lx\n",fifo,_lookAsidePayloadPAs[fifo]));
    }

  // Set up array of pointers to lookaside completion function arrays.
  // There is 1 pointer for each inj fifo.
  pami_event_function **_lookAsideCompletionFnPtrs;
  allocateMemory( false, // Use heap
		  (void **)(&_lookAsideCompletionFnPtrs),
		  16,
		  numInjFifos * sizeof(pami_event_function *) );
  TRACE((stderr,"MU ResourceManager: allocateLookasideResources: lookAsideCompletionFnPtrs = %p\n",_lookAsideCompletionFnPtrs));
  PAMI_assertf( _lookAsideCompletionFnPtrs != NULL, "The heap is full.\n" );
  *lookAsideCompletionFnPtrs = _lookAsideCompletionFnPtrs;

  // Allocate each of the lookaside completion function arrays.  They have the same number of slots
  // as the inj fifo.
  for ( fifo=0; fifo<numInjFifos; fifo++ )
    {
      allocateMemory( false, // Use heap
		      (void **)&(_lookAsideCompletionFnPtrs[fifo]),
		      16,
		      lookAsideCompletionFnArraySize );
      TRACE((stderr,"MU ResourceManager: allocateLookasideResources: lookAsideCompletionFnPtrs[%u] = %p\n",fifo,_lookAsideCompletionFnPtrs[fifo]));
      PAMI_assertf ( _lookAsideCompletionFnPtrs[fifo] != NULL, "The heap is full.\n" );
      memset( _lookAsideCompletionFnPtrs[fifo], 0x00, lookAsideCompletionFnArraySize );
    }

  // Set up array of pointers to lookaside completion cookie arrays.
  // There is 1 pointer for each inj fifo.
  void ***_lookAsideCompletionCookiePtrs;
  allocateMemory( false, // Use heap
		  (void **)(&_lookAsideCompletionCookiePtrs),
		  16,
		  numInjFifos * sizeof(void **) );
  TRACE((stderr,"MU ResourceManager: allocateLookasideResources: lookAsideCompletionCookiePtrs = %p\n",_lookAsideCompletionCookiePtrs));
  PAMI_assertf( _lookAsideCompletionCookiePtrs != NULL, "The heap is full.\n" );
  *lookAsideCompletionCookiePtrs = _lookAsideCompletionCookiePtrs;

  // Allocate each of the lookaside completion cookie arrays.  They have the same number of slots
  // as the inj fifo.
  for ( fifo=0; fifo<numInjFifos; fifo++ )
    {
      allocateMemory( false, // Use heap
		      (void **)&(_lookAsideCompletionCookiePtrs[fifo]),
		      16,
		      lookAsideCompletionCookieArraySize );
      TRACE((stderr,"MU ResourceManager: allocateLookasideResources: lookAsideCompletionCookiePtrs[%u] = %p\n",fifo,_lookAsideCompletionCookiePtrs[fifo]));
      PAMI_assertf ( _lookAsideCompletionCookiePtrs[fifo] != NULL, "The heap is full.\n" );
      memset( _lookAsideCompletionCookiePtrs[fifo], 0x00, lookAsideCompletionCookieArraySize );
    }
} // End: allocateLookasideResources()


// \brief Allocate Global Resources
//
// Allocate resources needed before main().  These include:
// - 10 remote get injection fifos (8 in subgroup 64, 2 in subgroup 65).
// -  1 collective combining injection fifo (in subgroup 65).
// -  1 base address table entry in subgroup 64, initialized to 0.
// -  N reception fifos, where N is in the range 1..256.  All available reception
//    fifos will be allocated at this time.
// -  Array of clientResources_t structures, one per client.
//
void PAMI::Device::MU::ResourceManager::allocateGlobalResources()
{
  size_t numClients = _pamiRM.getNumClients();

  // Prepare to allocate resources that are global to the node
  // - Calculates the amount of memory needed - no resources are allocated.
  //   Then, shared memory is set up so it can be allocated-from.
  _calculateSizeOnly = 1;
  allocateGlobalInjFifos();
  allocateGlobalBaseAddressTableEntries();
  setupSharedMemory();

  // The master process allocates space for AND initializes the resources, while
  // all other processes just allocate space for the resources.  The idea is that
  // all processes on the node perform the allocation so they know what the
  // addresses are...since they allocate the resources in the same sequence, they
  // all get the same addresses.  Only the master actually initializes that storage.
  // When the storage is shared memory, there are multiple processes.  The
  // shared memory allocator doesn't actually write into shared memory to track
  // the storage like malloc does.  It just calculates and returns addresses.
  // Whereas when there is only one process (SMP mode), there is no shared memory,
  // and malloc is used to allocate off the heap.  Since there is only one process,
  // that process will use malloc and initialize the storage.  There are no
  // processes that only allocate the storage.

  _calculateSizeOnly = 0;
  if ( _mapping.t() == _mapping.lowestT() ) // Master?
    _allocateOnly = 0;
  else
    _allocateOnly = 1;

  allocateGlobalInjFifos();
  allocateGlobalBaseAddressTableEntries();

  // Allocate process-scoped resources.
  _calculateSizeOnly = 0;
  _allocateOnly      = 0;
  allocateGlobalRecFifos();

  // Allocate space for the client resources array
  _clientResources = (clientResources_t *)malloc( numClients * sizeof( *_clientResources ) );
  PAMI_assertf ( _clientResources != NULL, "The heap is full.\n" );

  // Allocate space for the perCorePerProcessPamiResources array
  _perCorePerProcessPamiResources = (pamiResources_t*)malloc( numClients * sizeof( *_perCorePerProcessPamiResources ) );
  PAMI_assertf ( _perCorePerProcessPamiResources != NULL, "The heap is full.\n" );

  // Allocate space for the perContextMUResources array
  _perContextMUResources = (muResources_t*)malloc( numClients * sizeof( *_perContextMUResources ) );
  PAMI_assertf ( _perContextMUResources != NULL, "The heap is full.\n" );

} // End: allocateGlobalResources()


void PAMI::Device::MU::ResourceManager::allocateContextResources( size_t rmClientId,
								  size_t contextOffset )
{
  uint32_t numFifosSetup;
  size_t   numInjFifos = _perContextMUResources[rmClientId].numInjFifos;
  Kernel_InjFifoAttributes_t  injFifoAttr;

  // Set up the injection fifos for this context

  memset( &injFifoAttr, 0x00, sizeof(injFifoAttr) );

  numFifosSetup = setupInjFifos(
		    _clientResources[rmClientId].startingSubgroupIds[contextOffset],
		    _clientResources[rmClientId].endingSubgroupIds[contextOffset],
		    numInjFifos,
		    _pamiRM.getInjFifoSize(),
		    &injFifoAttr,
		    false, // Do not use shared memory...use heap.
		    &(_clientResources[rmClientId].injResources[contextOffset].subgroups),
		    &(_clientResources[rmClientId].injResources[contextOffset].fifoPtrs),
		    &(_clientResources[rmClientId].injResources[contextOffset].globalFifoIds) );
  PAMI_assertf( numFifosSetup == numInjFifos, "Only %u injection fifos were set up.  Expected %zu.\n",numFifosSetup,numInjFifos );
  // Set up the reception fifos for this context

  Kernel_RecFifoAttributes_t  recFifoAttr;
  memset( &recFifoAttr, 0x00, sizeof(recFifoAttr) );

  numFifosSetup = setupRecFifos(
		    _clientResources[rmClientId].startingSubgroupIds[contextOffset],
		    _clientResources[rmClientId].endingSubgroupIds[contextOffset],
		    _perContextMUResources[rmClientId].numRecFifos,
		    _pamiRM.getRecFifoSize(),
		    &recFifoAttr,
		    false, // Do not use shared memory...use heap.
		    &(_clientResources[rmClientId].recResources[contextOffset].subgroups),
		    &(_clientResources[rmClientId].recResources[contextOffset].fifoPtrs),
		    &(_clientResources[rmClientId].recResources[contextOffset].globalFifoIds) );
  PAMI_assertf( numFifosSetup == _perContextMUResources[rmClientId].numRecFifos, "Only %u reception fifos were set up.  Expected %zu.\n",numFifosSetup,_perContextMUResources[rmClientId].numRecFifos );
  // Store the adjusted global rec fifo id in the pinRecFifo array for each tcoord
  // 1. Find the first subgroup in our process
  size_t tSize = _mapping.tSize();
  size_t t;
  uint64_t threadMask = Kernel_ThreadMask( _mapping.t() );
  PAMI_assertf ( threadMask, "Kernel_ThreadMask returned no threads\n" ); // Cannot be zero
  uint32_t myStartingSubgroup = 0;
  uint64_t mask = 0x8000000000000000ULL;
  while ( (threadMask & mask) == 0 )
    {
      myStartingSubgroup++;
      mask = mask >> 1;
    }
  uint16_t myRelativeGlobalFifoId = _clientResources[rmClientId].recResources[contextOffset].globalFifoIds[0] - ( myStartingSubgroup * BGQ_MU_NUM_REC_FIFOS_PER_SUBGROUP);
  TRACE((stderr,"MU ResourceManager: allocateContextResources: rmClientId=%zu, contextOffset=%zu, globalFifoIds[0]=%u, myStartingSubgroup=%u, myRelativeRecFifoGlobalFifoId=%u\n",rmClientId,contextOffset,_clientResources[rmClientId].recResources[contextOffset].globalFifoIds[0],myStartingSubgroup,myRelativeGlobalFifoId));

  for ( t=0; t<tSize; t++ )
    {
      // Obtain the list of HW threads for this process (corresponding to subgroups) from the kernel
      // We will use this to determine the starting subgroup for each tcoord.
      // Starting Subgroup * number of rec fifos per subgroup + relativeGlobalFifoId is the
      // global rec fifo id for that tcoord.

      uint64_t threadMask = Kernel_ThreadMask( t );
      PAMI_assertf ( threadMask, "Kernel_ThreadMask returned no threads\n" ); // Cannot be zero

      // Find first subgroup for this t
      uint32_t startingSubgroup = 0;
      uint64_t mask = 0x8000000000000000ULL;
      while ( (threadMask & mask) == 0 )
	{
	  startingSubgroup++;
	  mask = mask >> 1;
	}

      _clientResources[rmClientId].pinRecFifo[(contextOffset*_tSize)+t] =
	(startingSubgroup * BGQ_MU_NUM_REC_FIFOS_PER_SUBGROUP) + myRelativeGlobalFifoId;

      TRACE((stderr,"MU ResourceManager: allocateContextResources: pinRecFifo[t=%zu][contextOffset=%zu] = %u\n",t,contextOffset,_clientResources[rmClientId].pinRecFifo[(contextOffset*_tSize) + t]));
    }

  allocateLookasideResources( numInjFifos,
			      _pamiRM.getInjFifoSize(),

			      &(_clientResources[rmClientId].
				injResources[contextOffset].
				lookAsidePayloadPtrs),

			      &(_clientResources[rmClientId].
				injResources[contextOffset].
				lookAsidePayloadMemoryRegions),

			      &(_clientResources[rmClientId].
				injResources[contextOffset].
				lookAsidePayloadPAs),

			      &(_clientResources[rmClientId].
				injResources[contextOffset].
				lookAsideCompletionFnPtrs),

			      &(_clientResources[rmClientId].
				injResources[contextOffset].
				lookAsideCompletionCookiePtrs));

} // End: allocateContextResources()


void PAMI::Device::MU::ResourceManager::initializeContexts( size_t rmClientId,
							    size_t numContexts,
                                                            PAMI::Device::Generic::Device * devices )
{
  size_t i;

  _clientResources[rmClientId].numContexts = numContexts;

  _clientResources[rmClientId].pinRecFifo = (uint16_t*)malloc( _mapping.tSize() *
							       numContexts *
							       sizeof(uint16_t) );
  PAMI_assertf( _clientResources[rmClientId].pinRecFifo, "The heap is full.\n" );

  _clientResources[rmClientId].startingSubgroupIds = (uint32_t*)malloc( numContexts * sizeof(uint32_t) );
  PAMI_assertf( _clientResources[rmClientId].startingSubgroupIds, "The heap is full.\n" );

  _clientResources[rmClientId].endingSubgroupIds = (uint32_t*)malloc( numContexts * sizeof(uint32_t) );
  PAMI_assertf( _clientResources[rmClientId].endingSubgroupIds, "The heap is full.\n" );

  _clientResources[rmClientId].injResources = (injFifoResources_t*)malloc( numContexts * sizeof(injFifoResources_t) );
  PAMI_assertf( _clientResources[rmClientId].injResources, "The heap is full.\n" );

  _clientResources[rmClientId].recResources = (recFifoResources_t*)malloc( numContexts * sizeof(recFifoResources_t) );
  PAMI_assertf( _clientResources[rmClientId].recResources, "The heap is full.\n" );

  // Determine the number of contexts per core needed by this client.
  size_t numCoresPerProcess = _pamiRM.getNumCoresPerProcess();
  size_t numContextsPerCore = ( numContexts + numCoresPerProcess -1 ) / numCoresPerProcess;
  _perCorePerProcessPamiResources[rmClientId].numContexts = numContextsPerCore;

  TRACE((stderr,"MU ResourceManager: initializeContexts: RmClientId=%zu, numCoresPerProcess = %zu, numContextsPerCore = %zu\n",rmClientId,numCoresPerProcess,numContextsPerCore));

  _perContextMUResources[rmClientId].numInjFifos =
    _perCorePerProcessPerClientMUResources[rmClientId].numInjFifos / numContextsPerCore;
  _perContextMUResources[rmClientId].numRecFifos =
    _perCorePerProcessPerClientMUResources[rmClientId].numRecFifos / numContextsPerCore;

  PAMI_assertf( _perContextMUResources[rmClientId].numInjFifos, "Not enough injection fifos are available\n" );
  PAMI_assertf( _perContextMUResources[rmClientId].numRecFifos, "Not enough reception fifos are available\n" );

  TRACE((stderr,"MU ResourceManager: initializeContexts: Each context for client %zu gets %zu injFifos and %zu recFifos\n",rmClientId, _perContextMUResources[rmClientId].numInjFifos, _perContextMUResources[rmClientId].numRecFifos));

  // Adjust the number of fifos needed if they exceed the max that we need.
  if ( _perContextMUResources[rmClientId].numInjFifos > optimalNumInjFifosPerContext )
    _perContextMUResources[rmClientId].numInjFifos = optimalNumInjFifosPerContext;
  if ( _perContextMUResources[rmClientId].numRecFifos > optimalNumRecFifosPerContext )
    _perContextMUResources[rmClientId].numRecFifos = optimalNumRecFifosPerContext;

  TRACE((stderr,"MU ResourceManager: initializeContexts: Each context for client %zu gets %zu injFifos and %zu recFifos after lowering to needed limits\n",rmClientId, _perContextMUResources[rmClientId].numInjFifos, _perContextMUResources[rmClientId].numRecFifos));

  // Obtain the list of HW threads for this process (corresponding to subgroups) from the kernel
  // We will use this to determine which subgroups to allocate resources from.
  // Start with the last core in this process and round robin backwards from there.

  uint64_t threadMask = Kernel_ThreadMask( _mapping.t() );
  PAMI_assertf ( threadMask, "Kernel_ThreadMask returned no threads\n" ); // Cannot be zero

  // Round robin among the cores in this process, creating one context each time.

  uint64_t subgroupMask     = 0x1ULL; // Start with the last thread on core 15.
  uint32_t startingSubgroup, endingSubgroup=63;

  for ( i=0; i<numContexts; i++ )
    {
      // Find the next subgroup where the context is to be created.
      while ( ( threadMask & subgroupMask ) == 0 )
	{
	  endingSubgroup--;
	  subgroupMask = subgroupMask << 1;  // Shift to next subgroup.
	  if ( subgroupMask == 0 )
	    {
	      endingSubgroup = 63;
	      subgroupMask = 0x1ULL; // Wrap
	    }
	}
      // Determine starting subgroup.  Don't go beyond the core boundary (multiple of 4).
      subgroupMask = subgroupMask << 1; // Shift to next subgroup.
      startingSubgroup = endingSubgroup - 1;
      while ( (( startingSubgroup & 0x3 ) != 3) && ( threadMask & subgroupMask ) )
	{
	  startingSubgroup--;
	  subgroupMask = subgroupMask << 1;
	}
      startingSubgroup++;  // Adjust, since we went 1 beyond.

      _clientResources[rmClientId].startingSubgroupIds[i] = startingSubgroup;
      _clientResources[rmClientId].endingSubgroupIds[i]   = endingSubgroup;

      TRACE((stderr,"MU ResourceManager: initializeContexts: Creating context %zu in starting subgroup %u, endingSubgroup %u\n",i,startingSubgroup,endingSubgroup));

      allocateContextResources( rmClientId, i );

      // Move to next core
      if ( subgroupMask == 0)
	{
	  endingSubgroup = 63;
	  subgroupMask = 0x1ULL; // Wrap
	}
      else
	{
	  endingSubgroup = startingSubgroup - 1; // Note: subgroupMask is already here.
	  while ( ( endingSubgroup & 0x3 ) != 3 )
	    {
	      endingSubgroup--;
	      subgroupMask = subgroupMask << 1;
	    }
	  if ( subgroupMask == 0)
	    {
	      endingSubgroup = 63;
	      subgroupMask = 0x1ULL; // Wrap
	    }
	}
    }
    _cr_mtx_mdls[rmClientId] = (MUCR_mutex_model_t *)malloc(numContexts * sizeof(*_cr_mtx_mdls[rmClientId]));
    for (i = 0; i < numContexts; i++)
    {
	  pami_result_t status = PAMI_ERROR;
	  AtomicMutexDev &device = PAMI::Device::AtomicMutexDev::Factory::getDevice((AtomicMutexDev *)devices, rmClientId, i);
	  new (&_cr_mtx_mdls[rmClientId][i]) MUCR_mutex_model_t(device, &_cr_mtx, status);
	  PAMI_assertf(status == PAMI_SUCCESS, "Failed to construct non-blocking mutex client %zd context %zd", rmClientId, i);
    }

} // End: initializeContexts()


void PAMI::Device::MU::ResourceManager::getNumFifosPerContext( size_t  rmClientId,
							       size_t *numInjFifos,
							       size_t *numRecFifos )
{
  *numInjFifos = _perContextMUResources[rmClientId].numInjFifos;
  *numRecFifos = _perContextMUResources[rmClientId].numRecFifos;

} // End: getNumFifosPerContext()


void PAMI::Device::MU::ResourceManager::getInjFifosForContext( size_t            rmClientId,
							       size_t            contextOffset,
							       size_t            numInjFifos,
							       MUSPI_InjFifo_t **injFifoPtrs )
{
  PAMI_assert( numInjFifos <= _perContextMUResources[rmClientId].numInjFifos );

  size_t fifo;
  for ( fifo=0; fifo<numInjFifos; fifo++ )
    {
      uint32_t globalFifoId     =
	_clientResources[rmClientId].injResources[contextOffset].globalFifoIds[fifo];
      uint32_t globalSubgroup   = globalFifoId / BGQ_MU_NUM_INJ_FIFOS_PER_SUBGROUP;
      uint32_t relativeSubgroup = globalSubgroup -
	_clientResources[rmClientId].startingSubgroupIds[contextOffset];
      uint32_t relativeFifo     = globalFifoId -
	(globalSubgroup * BGQ_MU_NUM_INJ_FIFOS_PER_SUBGROUP);

      injFifoPtrs[fifo] =
	&( _clientResources[rmClientId].
	   injResources[contextOffset].
	   subgroups[relativeSubgroup].
	   _injfifos[relativeFifo] );

      TRACE((stderr,"MU ResourceManager: getInjFifosForContext: context=%zu, globalFifoId=%u, globalSubgroup=%u, relativeSubgroup=%u, startingSubgroupId=%u, relativeFifo=%u, injFifoStructurePtr=%p, hwinjfifo=%p\n",contextOffset,globalFifoId,globalSubgroup,relativeSubgroup,_clientResources[rmClientId].startingSubgroupIds[contextOffset],relativeFifo,injFifoPtrs[fifo],injFifoPtrs[fifo]->hw_injfifo));
    }
} // End: getInjFifosForContext()


void PAMI::Device::MU::ResourceManager::getRecFifosForContext( size_t            rmClientId,
							       size_t            contextOffset,
							       size_t            numRecFifos,
							       MUSPI_RecFifo_t **recFifoPtrs,
							       uint32_t         *globalFifoIds )
{
  PAMI_assert( numRecFifos <= _perContextMUResources[rmClientId].numRecFifos );

  size_t fifo;
  for ( fifo=0; fifo<numRecFifos; fifo++ )
    {
      uint32_t globalFifoId     =
	_clientResources[rmClientId].recResources[contextOffset].globalFifoIds[fifo];
      uint32_t globalSubgroup   = globalFifoId / BGQ_MU_NUM_REC_FIFOS_PER_SUBGROUP;
      uint32_t relativeSubgroup = globalSubgroup - _clientResources[rmClientId].startingSubgroupIds[contextOffset];
      uint32_t relativeFifo     = globalFifoId -
	(globalSubgroup * BGQ_MU_NUM_REC_FIFOS_PER_SUBGROUP);

      recFifoPtrs[fifo] =
	&( _clientResources[rmClientId].
	   recResources[contextOffset].
	   subgroups[relativeSubgroup].
	   _recfifos[relativeFifo] );

      globalFifoIds[fifo] = globalFifoId;

      TRACE((stderr,"MU ResourceManager: getRecFifosForContext: globalFifoId=%u, globalSubgroup=%u, relativeSubgroup=%u, relativeFifo=%u, recFifoStructurePtr=%p, globalFifoId=%u\n",globalFifoId,globalSubgroup,relativeSubgroup,relativeFifo,recFifoPtrs[fifo],globalFifoIds[fifo]));
    }
} // End: getRecFifosForContext()


#undef TRACE

#endif   // __components_devices_bgq_mu_global_ResourceManager_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
