#ifndef __components_device_multisyncmodel_h__
#define __components_device_multisyncmodel_h__

#include "sys/xmi.h"

namespace XMI
{
    namespace Device
    {
        namespace Interface
        {
            ///
            /// \todo Need A LOT MORE documentation on this interface and its use
            /// \param T_Model   Multisync model template class
            ///
            /// \see Multisync::Model
            ///
            template <class T_Model>
            class MultisyncModel
            {
            public:
                /// \param[in] device                Multisync device reference
                MultisyncModel (xmi_result_t &status) { status = XMI_SUCCESS; };
                ~MultisyncModel () {};
                inline bool postMultisync (xmi_multisync_t *msync);
            };

            template <class T_Model>
            bool MultisyncModel<T_Model>::postMultisync (xmi_multisync_t *msync)
            {
	      return static_cast<T_Model*>(this)->postMultisync_impl(msync);
            }

        };
    };
};
#endif // __components_device_multisyncmodel_h__
