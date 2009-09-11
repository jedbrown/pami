/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/common/MulticombineModelImpl.h
 * \brief ???
 */

#ifndef __components_device_multicombinemodelimpl_h__
#define __components_device_multicombinemodelimpl_h__

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
            class MulticombineModelImpl
            {
	    private:
		xmi_multicombine_t _mcomb;
            public:
                /// \param[in] device                Multicombine device reference
                MulticombineModelImpl (T_Device &device, xmi_result &status) { status = XMI_SUCCESS; };
                ~MulticombineModelImpl () {};
                inline void setRoles_impl(unsigned roles) { _mcomb.roles = roles; }
                inline void setCount_impl(size_t count) { _mcomb.count = count; }
                inline void setData_impl(xmi_pipeworkqueue_t *data) { _mcomb.data = data; }
                inline void setDataRanks_impl(xmi_topology_t *data_participants) { _mcomb.data_participants = data_participants; }
                inline void setResults_impl(xmi_pipeworkqueue_t *results) { _mcomb.results = results; }
                inline void setResultsRanks_impl(xmi_topology_t *results_participants) { _mcomb.results_participants = results_participants; }
                inline void setReduceInfo_impl(xmi_op op,  xmi_dt dt) { _mcomb.dtype = dt; _mcomb.optor = op; }
                inline void setCallback_impl(xmi_callback_t &cb_done) { _mcomb.cb_done = cb_done; }

		// internal - used by specific models which inherit from this class
                inline unsigned _getRoles() { return _mcomb.roles; }
                inline xmi_pipeworkqueue_t *_getData() { return _mcomb.data; }
                inline xmi_topology_t *_getDataRanks() { return _mcomb.data_participants; }
                inline xmi_pipeworkqueue_t *_getResults() { return _mcomb.results; }
                inline size_t _getCount() { return _mcomb.count; }
                inline xmi_topology_t *_getResultsRanks() { return _mcomb.results_participants; }
                inline xmi_dt _getDt() { return _mcomb.dtype; }
                inline xmi_op _getOp() { return _mcomb.optor; }
		inline xmi_callback_t &_getCallback() { return _mcomb.cb_done; }

            };	// class MulticombineModelImpl
        };	// namespace Impl
    };		// namespace Device
};		// namespace XMI
#endif // __components_device_packetmodelimpl_h__
