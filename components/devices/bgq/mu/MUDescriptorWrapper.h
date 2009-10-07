/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu/MUDescriptorWrapper.h
 * \brief ???
 */
#ifndef __devices_bgq_mu_mudescriptorwrapper_h__
#define __devices_bgq_mu_mudescriptorwrapper_h__

#include <spi/include/mu/DescriptorWrapperXX.h>

#include "sys/xmi.h"

namespace XMI
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

          inline void setCallback (xmi_event_function function,
                                   void *             cookie,
                                   xmi_context_t      context)
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
            _function (_context, _cookie, XMI_SUCCESS);
          };

        protected:

          bool               _requiresCallback;
          xmi_event_function _function;
          void *             _cookie;
          xmi_context_t      _context;

      }; // XMI::Device::MU::MUDescriptorWrapper class
    };   // XMI::Device::MU namespace
  };     // XMI::Device namespace
};       // XMI namespace

//#undef TRACE

#endif // __devices_bgq_mu_mudescriptorwrapper_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
