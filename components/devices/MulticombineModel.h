/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/MulticombineModel.h
 * \brief ???
 */

#ifndef __components_devices_MulticombineModel_h__
#define __components_devices_MulticombineModel_h__

#include <sys/uio.h>

#include "sys/xmi.h"
#include "util/common.h"

namespace XMI
{
    namespace Device
  {
        namespace Interface
        {
            ///
            /// \todo Need A LOT MORE documentation on this interface and its use
            /// \param T_Model   Multicombine model template class
            ///
            /// \see Multicombine::Model
            ///
            template <class T_Model, unsigned T_StateBytes>
            class MulticombineModel
            {
            public:
                /// \param[in] device                Multicombine device reference
                MulticombineModel (xmi_result_t &status) {
			COMPILE_TIME_ASSERT(T_Model::sizeof_msg == T_StateBytes);
			status = XMI_SUCCESS;
		};
                ~MulticombineModel () {};
                inline xmi_result_t postMulticombine (uint8_t (&state)[T_StateBytes],
						xmi_multicombine_t *mcomb);
            }; // class MulticombineModel

            template <class T_Model, unsigned T_StateBytes>
            xmi_result_t MulticombineModel<T_Model, T_StateBytes>::postMulticombine(
							uint8_t (&state)[T_StateBytes],
							xmi_multicombine_t *mcomb)
            {
	      return static_cast<T_Model*>(this)->postMulticombine_impl(state, mcomb);
            }

        }; // namespace Interface
    }; // namespace Device
}; // namespace XMI
#endif // __components_devices_MulticombineModel_h__
