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
            /// \param T_Device  Multisync device template class
            /// \param T_Object  Multisync object template class
            ///
            /// \see Multisync::Model
            /// \see Multisync::Device
            ///
            template <class T_Model, class T_Device, class T_Object>
            class MultisyncModel
            {
            public:
                /// \param[in] device                Multisync device reference
                MultisyncModel (T_Device & device) {};
                ~MultisyncModel () {};
                inline bool postMultisync (T_Object * obj);
            };
            template <class T_Model, class T_Device, class T_Object>
            bool MultisyncModel<T_Model, T_Device, T_Object>::postMultisync (T_Object * obj)
            {
                static_cast<T_Model*>(this)->postMultisync_impl(obj);
            }

        };
    };
};
#endif // __components_device_multisyncmodel_h__
