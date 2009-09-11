/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/common/MultisyncModelImpl.h
 * \brief ???
 */

#ifndef __components_device_multisyncmodelimpl_h__
#define __components_device_multisyncmodelimpl_h__

#include "sys/xmi.h"

namespace XMI
{
    namespace Device
    {
        namespace Impl
        {
            ///
            /// \todo Need A LOT MORE documentation on this interface and its use
            ///
            class MultisyncModelImpl
            {
	    private:
		xmi_multisync_t _msync;
            public:
                /// \param[in] device                Multisync device reference
                MultisyncModelImpl (T_Device &device, xmi_result &status) { status = XMI_SUCCESS; };
                ~MultisyncModelImpl () {};
                inline void setConnectionId_impl(unsigned conn) { _msync.connection_id = conn; }
                inline void setRoles_impl(unsigned roles) { _msync.roles = roles; }
                inline void setRanks_impl(xmi_topology_t *participants) { _msync.participants = results_participants; }
                inline void setCallback_impl(xmi_callback_t &cb_done) { _msync.cb_done = cb_done; }

		// internal - used by specific models which inherit from this class
                inline unsigned _getRoles() { return _msync.roles; }
                inline unsigned _getConnectionId() { return _msync.connection_id; }
                inline xmi_topology_t *_getRanks() { return _msync.participants; }
		inline xmi_callback_t &_getCallback() { return _msync.cb_done; }
            };
        };
    };
};
#endif // __components_device_multisyncmodelimpl_h__
