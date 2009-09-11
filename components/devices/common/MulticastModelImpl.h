/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/common/MulticastModelImpl.h
 * \brief ???
 */

#ifndef __components_device_multicastmodelimpl_h__
#define __components_device_multicastmodelimpl_h__

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
            class MulticastModelImpl
            {
	    private:
		xmi_multicast_t _mcast;
            public:
                /// \param[in] device                Multicast device reference
                MulticastModelImpl (T_Device & device) {};
                ~MulticastModelImpl () {};
                inline void setConnectionId_impl (unsigned conn) { _mcast.connection_id = conn; }
                inline void setRoles_impl (unsigned roles) { _mcast.roles = roles; }
                inline void setBytes_impl (size_t bytes) { _mcast.bytes = bytes; }
                inline void setSendData_impl (xmi_pipeworkqueue_t *src) { _mcast.src = src; }
                inline void setSendRanks_impl (xmi_topology_t *src_participants) { _mcast.src_participants = src_participants; }
                inline void setRecvData_impl (xmi_pipeworkqueue_t *dst) { _mcast.dst = dst; }
                inline void setRecvRanks_impl (xmi_topology_t *dst_participants) { _mcast.dst_participants = dst_participants; }
                inline void setCallback_impl (xmi_callback_t &cb_done) { _mcast.cb_done = cb_done; }
                inline void setInfo_impl (xmi_quad_t *info, int count) { _mcast.msginfo = info; _mcast.msgcount = count; }

		// internal... used by specific models which inherit from this class.
		inline xmi_multicast_t *_getMcast() { return &_mcast; }
		inline unsigned _getConnectionId() { return _mcast.connection_id; }
		inline unsigned _getRoles() { return _mcast.roles; }
		inline xmi_pipeworkqueue_t *_getSendData() { return _mcast.src; }
		inline xmi_pipeworkqueue_t *_getRecvData() { return _mcast.dst; }
		inline size_t _getBytes() { return _mcast.bytes; }
		inline xmi_topology_t *_getSendRanks() { return _mcast.src_participants; }
		inline xmi_topology_t *_getRecvRanks() { return _mcast.dst_participants; }
		inline xmi_callback_t &_getCallback() { return _mcast.cb_done; }
            };	// class MulticastModelImpl
        };	// namespace Impl
    };		// namespace Device
};		// namespace XMI
#endif // __components_device_packetmodelimpl_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
