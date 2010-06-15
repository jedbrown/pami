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
#include "components/devices/bgq/mu2/Context.h"
#include "components/devices/generic/Device.h"

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
                new (&mu[i])
                MU::Context (__global.mapping, id_base, id_offset, id_count);
              }

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
            return getDevice_impl (devices, clientid, contextid).init (clientid, client, context);
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

