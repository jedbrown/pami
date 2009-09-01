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

#ifndef __components_device_manytomanymodel_h__
#define __components_device_manytomanymodel_h__

#include "sys/xmi.h"
namespace XMI
{
    namespace Device
    {
        namespace Interface
        {
            ///
            /// \todo Need A LOT MORE documentation on this interface and its use
            /// \param T_Model   Manytomany model template class
            /// \param T_Device  Manytomany device template class
            /// \param T_Object  Manytomany object template class
            ///
            /// \see Manytomany::Model
            /// \see Manytomany::Device
            ///
            template <class T_Model, class T_Device, class T_Object>
            class ManytomanyModel
            {
            public:
                /// \param[in] device                Manytomany device reference
                ManytomanyModel (T_Device & device) {};
                ~ManytomanyModel () {};

                inline void setCallback (manytomany_recv cb_recv, void *arg);
                inline void send  (XMI_Manytomany_t parameters);
                
                inline void postRecv (XMI_Request_t         * request,
                                      const XMI_Callback_t  * cb_done,
                                      unsigned                 connid,
                                      char                   * buf,
                                      unsigned               * sizes,
                                      unsigned               * offsets,
                                      unsigned               * counters,
                                      unsigned                 nranks,
                                      unsigned                 myindex) = 0;
            };
            template <class T_Model, class T_Device, class T_Object>
            void ManytomanyModel<T_Model, T_Device, T_Object>::setRequestBuffer (XMI_Request_t *request,
                                                                                size_t req_size)
            {
                static_cast<T_Model*>(this)->setRequestBuffer_impl(request, req_size);
            }
        

        };
    };
};
#endif // __components_device_manytomanymodel_h__
