/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/PacketModel.h
 * \brief ???
 */

#ifndef __components_device_multicombinemodel_h__
#define __components_device_multicombinemodel_h__

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
            /// \param T_Model   Multicombine model template class
            ///
            /// \see Multicombine::Model
            ///
            template <class T_Model>
            class MulticombineModel
            {
            public:
                /// \param[in] device                Multicombine device reference
                MulticombineModel (xmi_result_t &status) { status = XMI_SUCCESS; };
                ~MulticombineModel () {};
                inline bool postMulticombine (xmi_multicombine_t *mcomb);
            };

            template <class T_Model>
            bool MulticombineModel<T_Model>::postMulticombine (xmi_multicombine_t *mcomb)
            {
                static_cast<T_Model*>(this)->postMulticombine_impl(mcomb);
            }

        };
    };
};
#endif // __components_device_packetmodel_h__
