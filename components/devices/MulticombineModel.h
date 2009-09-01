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
                inline void setRequestBuffer(XMI_Request_t *request, size_t req_size);
                inline void setConnectionId (unsigned conn);
                inline void setRoles (unsigned roles);
                inline void setData (XMI_PipeWorkQueue_t *data, size_t count);
                inline void setDataRanks (XMI_Topology_t *data_participants);
                inline void setResults (XMI_PipeWorkQueue_t *results, size_t count);
                inline void setResultsRanks (XMI_Topology_t *results_participants);
                inline void setReduceInfo (xmi_op op,  xmi_dt dt);
                inline void setCallback (void (*fn) (void *, xmi_error_t *),  void *cd);
                inline bool postMulticombine (T_Object * obj);
            };
            template <class T_Model, class T_Device, class T_Object>
            void MulticombineModel<T_Model, T_Device, T_Object>::setRequestBuffer (XMI_Request_t *request,
                                                                                   size_t req_size)
            {
                static_cast<T_Model*>(this)->setRequestBuffer_impl(request, req_size);
            }
        
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
            void MulticombineModel<T_Model, T_Device, T_Object>::setData (XMI_PipeWorkQueue_t *src,
                                                                          size_t count)
            {
                static_cast<T_Model*>(this)->setData_impl(src, count);
            }

            template <class T_Model, class T_Device, class T_Object>
            void MulticombineModel<T_Model, T_Device, T_Object>::setDataRanks (XMI_Topology_t *src_participants)
            {
                static_cast<T_Model*>(this)->setDataRanks_impl(src_participants);
            }

            template <class T_Model, class T_Device, class T_Object>
            void MulticombineModel<T_Model, T_Device, T_Object>::setResults (XMI_PipeWorkQueue_t *results,
                                                                             size_t count)
            {
                static_cast<T_Model*>(this)->setResults_impl(results,count);
            }
        
            template <class T_Model, class T_Device, class T_Object>        
            void MulticombineModel<T_Model, T_Device, T_Object>::setResultsRanks (XMI_Topology_t *results_participants)
            {
                static_cast<T_Model*>(this)->setResultsRanks_impl(results_participants);
            }
        
            template <class T_Model, class T_Device, class T_Object>
            void MulticombineModel<T_Model, T_Device, T_Object>::setReduceInfo (xmi_op op, xmi_dt dt)
            {
                static_cast<T_Model*>(this)->setReduceInfo_impl(op, dt);
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
