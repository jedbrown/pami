/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu/MUDescriptorWrapper.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu_MUDescriptorWrapper_h__
#define __components_devices_bgq_mu_MUDescriptorWrapper_h__

#include <spi/include/mu/DescriptorWrapperXX.h>

#include <pami.h>

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      class MUDescriptorWrapper : public MUSPI_DescriptorWrapper
      {
        public:

          inline MUDescriptorWrapper (MUSPI_DescriptorBase *desc = NULL) :
            MUSPI_DescriptorWrapper (desc),
            _requiresCallback (false)
          {};

          inline ~MUDescriptorWrapper () {};

          inline void setCallback (pami_event_function function,
                                   void *             cookie,
                                   pami_context_t      context)
          {
            _requiresCallback = true;
            _function = function;
            _cookie   = cookie;
            _context  = context;
          };

          inline bool requiresCallback ()
          {
            return _requiresCallback;
          }

          inline void invokeCallback ()
          {
            _function (_context, _cookie, PAMI_SUCCESS);
          };

        protected:

          bool               _requiresCallback;
          pami_event_function _function;
          void *             _cookie;
          pami_context_t      _context;

      }; // PAMI::Device::MU::MUDescriptorWrapper class
    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

//#undef TRACE

#endif // __devices_bgq_mu_mudescriptorwrapper_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
