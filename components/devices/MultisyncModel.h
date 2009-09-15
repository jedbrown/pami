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
		inline void setConnectionId (unsigned conn);
		inline void setRoles(unsigned roles);
		inline void setRanks(xmi_topology_t *participants);
		inline void setCallback(xmi_callback_t &cb_done);
                inline bool postMultisync (T_Object * obj);
            };
            template <class T_Model, class T_Device, class T_Object>
            void MultisyncModel<T_Model, T_Device, T_Object>::setConnectionId (unsigned conn)
            {
                static_cast<T_Model*>(this)->setConnectionId_impl(conn);
            }

            template <class T_Model, class T_Device, class T_Object>
            void MultisyncModel<T_Model, T_Device, T_Object>::setRoles(unsigned roles)
            {
                static_cast<T_Model*>(this)->setRoles_impl(roles);
            }

            template <class T_Model, class T_Device, class T_Object>
            void MultisyncModel<T_Model, T_Device, T_Object>::setRanks(xmi_topology_t *participants)
            {
                static_cast<T_Model*>(this)->setRanks_impl(participants);
            }

            template <class T_Model, class T_Device, class T_Object>
            void MultisyncModel<T_Model, T_Device, T_Object>::setCallback(xmi_callback_t &cb_done)
            {
                static_cast<T_Model*>(this)->setCallback_impl(cb_done);
            }

            template <class T_Model, class T_Device, class T_Object>
            bool MultisyncModel<T_Model, T_Device, T_Object>::postMultisync (T_Object * obj)
            {
                static_cast<T_Model*>(this)->postMultisync_impl(obj);
            }

        };
    };
};
#endif // __components_device_multisyncmodel_h__
