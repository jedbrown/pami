/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/Factory.h
 * \brief ???
 */

#ifndef __components_devices_bgq_mu2_Factory_h__
#define __components_devices_bgq_mu2_Factory_h__

#include "Global.h"

#include "components/devices/FactoryInterface.h"
#include "components/devices/bgq/mu2/global/Global.h"
#include "components/devices/bgq/mu2/Context.h"
#include "components/devices/generic/Device.h"
#include "components/atomic/gcc/GccCounter.h"
#include "components/atomic/counter/CounterBarrier.h"
#include "spi/include/kernel/gi.h"
#include "spi/include/mu/GIBarrier.h"

#ifdef TRACE
#undef TRACE
#endif
#define TRACE(x) //fprintf x

extern PAMI::Device::MU::Global     __MUGlobal;
extern PAMI::Device::MU::Context  * __mu_contexts;

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      class Factory : public Interface::FactoryInterface<MU::Factory, MU::Context, PAMI::Device::Generic::Device>
      {
        public:
	static inline int32_t myMUSPI_GIBarrierInit (
						     MUSPI_GIBarrier_t                    *GIBarrier,
						     uint32_t                              classRouteId )
	  {
	    uint64_t system;
	    uint64_t dcr_value;

	    MUSPI_assert ( GIBarrier != NULL );
	    MUSPI_assert ( classRouteId < BGQ_GI_CLASS_MAX_CLASSROUTES );

	    /* Determine whether this class route is for "system" use or for "user" use.
	     * This will dictate whether we use privileged or non-privileged MU MMIO
	     * addresses.
	     *
	     * Initialize a mask to assume it is for "system" use.
	     * This will be used to mask against the privileged addresses.
	     * So, for "system" use, it won't turn off the privileged bit.
	     */
	    system = 0xFFFFFFFFFFFFFFFFULL;
	    dcr_value = DCRReadUser(MU_DCR(SYS_BARRIER));
	    if ( (dcr_value & ( 1 << (BGQ_GI_CLASS_MAX_CLASSROUTES-1-classRouteId) ) ) == 0 )
	      system = ~PHYMAP_PRIVILEGEDOFFSET;

	    /* Point to the first group's control registers, and offset into that by the
	     * classRouteId to get the control and status register pointers.  The
	     * registers are mirrored in all subgroups, so it does not matter which
	     * subgroup we use.
	     */
	    GIBarrier->controlRegPtr = (uint64_t*)(BGQ_MU_GI_CONTROL_OFFSET(0,classRouteId) & system);
	    GIBarrier->statusRegPtr  = (uint64_t*)(BGQ_MU_GI_STATUS_OFFSET(0,classRouteId)  & system);

	    GIBarrier->classRouteId  = classRouteId; /* Save the class route id */

	    /* Set the state based on the current contents of the control register */
	    GIBarrier->state = *(GIBarrier->controlRegPtr) & 0x7;

	    return 0;
	  }

          ///
          /// \copydoc Interface::Factory::generate
          ///
          static inline MU::Context * generate_impl (size_t                          id_client,
                                                     size_t                          id_count,
                                                     Memory::MemoryManager         & mm,
                                                     PAMI::Device::Generic::Device * devices)
          {
	    int32_t rc;
	    // Set up a local barrier (needed below).
	    bool master;
	    size_t myTask        = __global.mapping.t();
	    size_t numLocalTasks = __global.topology_local.size();
	    ( myTask == __global.mapping.lowestT() ) ? master=true : master=false;

	    TRACE((stderr, "MU::Context::generate_impl: Initializing local barrier, size=%zu, master=%d\n", numLocalTasks, master));
	    PAMI::Barrier::CounterBarrier<PAMI::Counter::GccNodeCounter> barrier;
	    barrier.init(&__global.mm,
			 numLocalTasks,
			 master );

	    //
	    // The MU resource manager requires that only one task on the local node
	    // initialize their contexts at a time to prevent race conditions
	    // in the MU SPIs.  To enforce this, loop through each task on the
	    // local node performing a local barrier.  During each loop iteration,
	    // only one task performs the init.
	    size_t task;

	    TRACE((stderr,"MU Factory: Entering local barrier before context create loop\n"));
	    barrier.enter();
	    TRACE((stderr,"MU Factory: Exiting  local barrier before context create loop\n"));

	    for ( task=0; task<numLocalTasks; task++)
	      {
		if ( myTask == task ) // Is it my turn to initialize?
		  {
		    // Initialize the MU resources for all contexts for this client
		    __MUGlobal.getMuRM().initializeContexts( id_client, id_count, devices );
		  }
		TRACE((stderr,"MU Factory: Entering local barrier. Initializing task=%zu\n",task));
		barrier.enter();
		TRACE((stderr,"MU Factory: Exiting  local barrier. Initializing task=%zu\n",task));
	      }

            // Allocate an array of mu contexts, one for each pami context
            // in this _task_ (from heap, not from shared memory)
            MU::Context * mu;
            rc = posix_memalign((void **) & mu, 64, sizeof(*mu) * id_count);
            PAMI_assertf(rc == 0, "posix_memalign failed for mu[%zu], errno=%d\n", id_count, errno);

// !!!!
// need to determine base context identifier
// !!!!
            size_t id_offset, id_base = 0;

            // Instantiate the mu context objects
	    for (id_offset = 0; id_offset < id_count; ++id_offset)
	      {
		new (&mu[id_offset])
		  MU::Context (__global.mapping, id_base, id_offset, id_count);
	      }

	    // Barrier among all nodes to ensure that the resources for this client
	    // are ready on all of the nodes.
	    // - Barrier among the processes on this node
	    // - Barrier among other nodes
	    // - Barrier among the processes on this node
	    TRACE((stderr,"MU Factory: Entering local barrier after creating contexts\n"));
	    barrier.enter();
	    TRACE((stderr,"MU Factory: Exiting  local barrier after creating contexts\n"));


	    // If multi-node, and master, need to barrier
	    if ( master && (__global.mapping.numActiveNodes() > 1) )
	      {
#ifdef ENABLE_MAMBO_WORKAROUNDS
		if (!__global.personality._is_mambo)
#endif
		  {
		    // Do an MU barrier
		    uint32_t classRouteId = 0;
		    MUSPI_GIBarrier_t commworld_barrier;
		    rc = Kernel_GetGlobalBarrierUserClassRouteId( &classRouteId );
		    PAMI_assert(rc==0);
		    rc = myMUSPI_GIBarrierInit ( &commworld_barrier,
						 classRouteId );
		    PAMI_assert(rc==0);
		    rc = Kernel_GetGlobalBarrierUserClassRouteId( &classRouteId );
		    TRACE((stderr,"MU Factory: enter global barrier on class route %u\n",classRouteId));
		    rc = MUSPI_GIBarrierEnterAndWait ( &commworld_barrier );
		    PAMI_assert(rc==0);
		  }
#ifdef ENABLE_MAMBO_WORKAROUNDS
		else
		  {
		    double seconds = 20; // wait 20 pseudo-seconds
		    double dseconds = ((double)seconds) / 1000; //mambo seconds are loooong.
		    double start = PAMI_Wtime (), d = 0;
		    TRACE((stderr, "%s sleep - %.0f,start %f < %f\n", __PRETTY_FUNCTION__, d, start, start + dseconds));

		    while (PAMI_Wtime() < (start + dseconds))
		      {
			for (int i = 0; i < 200000; ++i) ++d;

			TRACE((stderr, "%s sleep - %.0f, %f < %f\n", __PRETTY_FUNCTION__, d, PAMI_Wtime(), start + dseconds));
		      }

		    TRACE((stderr, "%s sleep - %.0f, start %f, end %f\n", __PRETTY_FUNCTION__, d, start, PAMI_Wtime()));
		  }
#endif
		TRACE((stderr,"MU Factory: exit global barier\n"));
	      }

	    TRACE((stderr, "MU Factory: Entering Local Barrier after global barrier\n"));
	    barrier.enter();
	    TRACE((stderr, "MU Factory: Exiting Local Barrier after global barrier\n"));

	    __mu_contexts = mu;

            return mu;
          };

          ///
          /// \copydoc Interface::Factory::init
          ///
          static inline pami_result_t init_impl (MU::Context           * devices,
                                                 size_t                  clientid,
                                                 size_t                  contextid,
                                                 pami_client_t           client,
                                                 pami_context_t          context,
                                                 Memory::MemoryManager * mm,
                                                 Generic::Device       * progress)
          {
            return getDevice_impl (devices, clientid, contextid).init (clientid, context, progress);
          };

          ///
          /// \copydoc Interface::Factory::advance
          ///
          static inline size_t advance_impl (MU::Context * devices,
                                             size_t        clientid,
                                             size_t        contextid)
          {
            return getDevice_impl (devices, clientid, contextid).advance ();
          };

          ///
          /// \copydoc Interface::Factory::getDevice
          ///
          static inline MU::Context & getDevice_impl (MU::Context * devices,
                                                      size_t     clientid,
                                                      size_t     contextid)
          {
            return devices[contextid];
          };
      }; // class     PAMI::Device::MU::Factory
    };   // namespace PAMI::Device::MU
  };     // namespace PAMI::Device
};       // namespace PAMI


#endif // __components_devices_bgq_mu2_Factory_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
