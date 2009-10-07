/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/MulticastModel.h
 * \brief ???
 */

#ifndef __components_device_multicastmodel_h__
#define __components_device_multicastmodel_h__

#include <sys/uio.h>

#include "sys/xmi.h"

namespace XMI
{
    namespace Device
    {
        namespace Interface
        {
            ///
            /// \todo Need A LOT MORE documentation on this interface and its use
            /// \param T_Model   Multicast model template class
            ///
            /// \see Multicast::Model
            ///
            template <class T_Model>
            class MulticastModel
            {
            public:
                MulticastModel (xmi_result_t &status) { status = XMI_SUCCESS; };
                ~MulticastModel () {};
                inline bool postMulticast (xmi_multicast_t *mcast);
            };

            template <class T_Model>
            bool MulticastModel<T_Model>::postMulticast (xmi_multicast_t *mcast)
            {
	      return static_cast<T_Model*>(this)->postMulticast_impl(mcast);
            }

        };
    };
};
#endif // __components_device_multicastmodel_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
