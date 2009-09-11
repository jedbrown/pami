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
            /// \param T_Device  Multicombine device template class
            /// \param T_Object  Multicombine object template class
            ///
            /// \see Multicombine::Model
            /// \see Multicombine::Device
            ///
            template <class T_Model, class T_Device, class T_Object>
            class MulticombineModel
            {
            public:
                /// \param[in] device                Multicombine device reference
                MulticombineModel (T_Device & device) {};
                ~MulticombineModel () {};
                inline void setConnectionId (unsigned conn);
                inline void setRoles (unsigned roles);
                inline void setCount (size_t count);
                inline void setData (xmi_pipeworkqueue_t *data);
                inline void setDataRanks (xmi_topology_t *data_participants);
                inline void setResults (xmi_pipeworkqueue_t *results);
                inline void setResultsRanks (xmi_topology_t *results_participants);
                inline void setReduceInfo (xmi_op op,  xmi_dt dt);
                inline void setCallback (xmi_callback_t &cb_done);
                inline bool postMulticombine (T_Object * obj);
            };
            template <class T_Model, class T_Device, class T_Object>
            void MulticombineModel<T_Model, T_Device, T_Object>::setConnectionId (unsigned conn)
            {
                static_cast<T_Model*>(this)->setConnectionId_impl(conn);
            }
        
            template <class T_Model, class T_Device, class T_Object>
            void MulticombineModel<T_Model, T_Device, T_Object>::setRoles (unsigned roles)
            {
                static_cast<T_Model*>(this)->setRoles_impl(roles);
            }

            template <class T_Model, class T_Device, class T_Object>
            void MulticombineModel<T_Model, T_Device, T_Object>::setData (xmi_pipeworkqueue_t *src)
            {
                static_cast<T_Model*>(this)->setData_impl(src);
            }

            template <class T_Model, class T_Device, class T_Object>
            void MulticombineModel<T_Model, T_Device, T_Object>::setCount (size_t count)
            {
                static_cast<T_Model*>(this)->setCount_impl(count);
            }

            template <class T_Model, class T_Device, class T_Object>
            void MulticombineModel<T_Model, T_Device, T_Object>::setDataRanks (xmi_topology_t *src_participants)
            {
                static_cast<T_Model*>(this)->setDataRanks_impl(src_participants);
            }

            template <class T_Model, class T_Device, class T_Object>
            void MulticombineModel<T_Model, T_Device, T_Object>::setResults (xmi_pipeworkqueue_t *results)
            {
                static_cast<T_Model*>(this)->setResults_impl(results);
            }
        
            template <class T_Model, class T_Device, class T_Object>        
            void MulticombineModel<T_Model, T_Device, T_Object>::setResultsRanks (xmi_topology_t *results_participants)
            {
                static_cast<T_Model*>(this)->setResultsRanks_impl(results_participants);
            }

            template <class T_Model, class T_Device, class T_Object>
            void MulticombineModel<T_Model, T_Device, T_Object>::setReduceInfo (xmi_op op, xmi_dt dt)
            {
                static_cast<T_Model*>(this)->setReduceInfo_impl(op, dt);
            }

            template <class T_Model, class T_Device, class T_Object>
            void MulticombineModel<T_Model, T_Device, T_Object>::setCallback (xmi_callback_t &cb_done)
            {
                static_cast<T_Model*>(this)->setCallback_impl(cb_done);
            }
            
            template <class T_Model, class T_Device, class T_Object>
            bool MulticombineModel<T_Model, T_Device, T_Object>::postMulticombine (T_Object * obj)
            {
                static_cast<T_Model*>(this)->postMulticombine_impl(obj);
            }

        };
    };
};
#endif // __components_device_packetmodel_h__
