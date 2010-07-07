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

#ifdef TRACE
#undef TRACE
#endif
#define TRACE(x) //fprintf x

extern PAMI::Device::MU::Global __MUGlobal;

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      class Factory : public Interface::FactoryInterface<MU::Factory, MU::Context, PAMI::Device::Generic::Device>
      {
        public:

          ///
          /// \copydoc Interface::Factory::generate
          ///
          static inline MU::Context * generate_impl (size_t                          id_client,
                                                     size_t                          id_count,
                                                     Memory::MemoryManager         & mm,
                                                     PAMI::Device::Generic::Device * devices)
          {
	    // Initialize the MU resources for all contexts for this client
	    __MUGlobal.getMuRM().initializeContexts( id_client, id_count, devices );

            // Allocate an array of mu contexts, one for each pami context
            // in this _task_ (from heap, not from shared memory)
            MU::Context * mu;
            int rc = posix_memalign((void **) & mu, 16, sizeof(*mu) * id_count);
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

	    bool master;
	    (__global.mapping.t() == 0) ? master=true : master=false;

	    TRACE((stderr, "MU::Context::generate_impl: Initializing local barrier, size=%zu, master=%d\n", __global.topology_local.size(), master));
	    PAMI::Barrier::CounterBarrier<PAMI::Counter::GccNodeCounter> barrier;
	    barrier.init(&__global.mm,
			 __global.topology_local.size(),
			 master );

	    TRACE((stderr, "ResourceManager: Entering Local Barrier\n"));
	    barrier.enter();
	    TRACE((stderr, "ResourceManager: Exiting Local Barrier\n"));

#ifdef ENABLE_MAMBO_WORKAROUNDS

	    // \todo Replace this with a real MU barrier when it is available

	    // If multi-node, and master, need to sleep
	    if ( master && (__global.mapping.size() > __global.personality.tSize()) )
	      {
		double seconds = 5; // wait 5 pseudo-seconds
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

	    TRACE((stderr, "ResourceManager: Entering Local Barrier\n"));
	    barrier.enter();
	    TRACE((stderr, "ResourceManager: Exiting Local Barrier\n"));

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
            return getDevice_impl (devices, clientid, contextid).init (clientid, context);
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
