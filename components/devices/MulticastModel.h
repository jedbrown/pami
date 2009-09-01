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

#ifndef __components_device_multicastmodel_h__
#define __components_device_multicastmodel_h__

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
            /// \param T_Model   Multicast model template class
            /// \param T_Device  Multicast device template class
            /// \param T_Object  Multicast object template class
            ///
            /// \see Multicast::Model
            /// \see Multicast::Device
            ///
            template <class T_Model, class T_Device, class T_Object>
            class MulticastModel
            {
            public:
                /// \param[in] device                Multicast device reference
                MulticastModel (T_Device & device) {};
                ~MulticastModel () {};
                inline void setRequestBuffer (XMI_Request_t *request, size_t req_size);
                inline void setConnectionId (unsigned conn);
                inline void setRoles (unsigned roles);
                inline void setSendData (XMI_PipeWorkQueue_t *src, size_t bytes);
                inline void setSendRanks (XMI_Topology_t *src_participants);
                inline void setRecvData (XMI_PipeWorkQueue_t *dst, size_t bytes);
                inline void setRecvRanks (XMI_Topology_t *dst_participants);
                inline void setCallback (void (*fn) (void *, xmi_error_t *),  void *cd);
                inline void setInfo (XMIQuad *info, int count);
                inline bool postMultiCast (T_Object * obj);
            };
            template <class T_Model, class T_Device, class T_Object>
            void MulticastModel<T_Model, T_Device, T_Object>::setRequestBuffer (XMI_Request_t *request,
                                                                                size_t req_size)
            {
                static_cast<T_Model*>(this)->setRequestBuffer_impl(request, req_size);
            }
        
            template <class T_Model, class T_Device, class T_Object>
            void MulticastModel<T_Model, T_Device, T_Object>::setConnectionId (unsigned conn)
            {
                static_cast<T_Model*>(this)->setConnectionId_impl(conn);
            }
        
            template <class T_Model, class T_Device, class T_Object>
            void MulticastModel<T_Model, T_Device, T_Object>::setRoles (unsigned roles)
            {
                static_cast<T_Model*>(this)->setRoles_impl(roles);
            }
        
            template <class T_Model, class T_Device, class T_Object>
            void MulticastModel<T_Model, T_Device, T_Object>::setSendData (XMI_PipeWorkQueue_t *src,
                                                                           size_t bytes)
            {
                static_cast<T_Model*>(this)->setSendData_impl(src, bytes);
            }

            template <class T_Model, class T_Device, class T_Object>
            void MulticastModel<T_Model, T_Device, T_Object>::setSendRanks (XMI_Topology_t *src_participants)
            {
                static_cast<T_Model*>(this)->setSendRanks_impl(src_participants);
            }

            template <class T_Model, class T_Device, class T_Object>
            void MulticastModel<T_Model, T_Device, T_Object>::setRecvData (XMI_PipeWorkQueue_t *dst,
                                                                           size_t bytes)
            {
                static_cast<T_Model*>(this)->setRecvData_impl(dst,bytes);
            }
        
            template <class T_Model, class T_Device, class T_Object>        
            void MulticastModel<T_Model, T_Device, T_Object>::setRecvRanks (XMI_Topology_t *dst_participants)
            {
                static_cast<T_Model*>(this)->setRecvRanks_impl(dst_participants);
            }
        
            template <class T_Model, class T_Device, class T_Object>
            void MulticastModel<T_Model, T_Device, T_Object>::setCallback (void (*fn) (void *, xmi_error_t *),
                                                                           void *cd)
            {
                static_cast<T_Model*>(this)->setCallback_impl(fn, cd);
            }
        
            template <class T_Model, class T_Device, class T_Object>
            void MulticastModel<T_Model, T_Device, T_Object>::setInfo (XMIQuad *info, int count)
            {
                static_cast<T_Model*>(this)->setInfo_impl(info, count);
            }
        
            template <class T_Model, class T_Device, class T_Object>
            bool MulticastModel<T_Model, T_Device, T_Object>::postMultiCast (T_Object * obj)
            {
                static_cast<T_Model*>(this)->postMultiCast_impl(obj);
            }

        };
    };
};
#endif // __components_device_packetmodel_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
