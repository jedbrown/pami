/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/FactoryInterface.h
 * \brief ???
 */

#ifndef __components_devices_FactoryInterface_h__
#define __components_devices_FactoryInterface_h__

#include "sys/xmi.h"

#include "components/memory/MemoryManager.h"

namespace XMI
{
  namespace Device
  {
    namespace Interface
    {
      ///
      /// \param T_Factory factory template class
      /// \param T_Device  device template class
      /// \param T_Device  progress template class
      ///
      template <class T_Factory, class T_Device, class T_Progress>
      class FactoryInterface
      {

        public:

          static inline T_Device * generate (size_t clientid, size_t num_ctx, Memory::MemoryManager & mm);

          static inline xmi_result_t init (T_Device      * devices,
                                           size_t          clientid,
                                           size_t          contextid,
                                           xmi_client_t    client,
                                           xmi_context_t   context,
                                           SysDep        * sysdep,
                                           T_Progress * progress);

          static inline size_t advance (T_Device * devices,
                                        size_t     clientid,
                                        size_t     contextid);

	  static inline T_Device & getDevice(T_Device * devices,
					     size_t clientid,
					     size_t contextid);

      };  // XMI::Device::Interface::FactoryInterface class
    };    // XMI::Device::Interface namespace
  };      // XMI::Device namespace
};        // XMI namespace

template <class T_Factory, class T_Device, class T_Progress>
T_Device * XMI::Device::Interface::FactoryInterface<T_Factory,T_Device,T_Progress>::generate (size_t clientid, size_t num_ctx, Memory::MemoryManager & mm)
{
   return T_Factory::generate_impl (clientid, num_ctx, mm);
};

template <class T_Factory, class T_Device, class T_Progress>
xmi_result_t XMI::Device::Interface::FactoryInterface<T_Factory,T_Device,T_Progress>::init (T_Device      * devices,
                                                                                            size_t          clientid,
                                                                                            size_t          contextid,
                                                                                            xmi_client_t    client,
                                                                                            xmi_context_t   context,
                                                                                            SysDep        * sysdep,
                                                                                            T_Progress * progress)
{
   return T_Factory::init_impl (devices, clientid, contextid, client, context, sysdep, progress);
};

template <class T_Factory, class T_Device, class T_Progress>
size_t XMI::Device::Interface::FactoryInterface<T_Factory,T_Device,T_Progress>::advance (T_Device * devices,
                                                                                         size_t     contextid,
                                                                                         size_t     clientid)
{
   return T_Factory::advance_impl (devices, contextid, clientid);
};

template <class T_Factory, class T_Device, class T_Progress>
T_Device & XMI::Device::Interface::FactoryInterface<T_Factory,T_Device,T_Progress>::getDevice (T_Device * devices,
                                                                                         size_t     contextid,
                                                                                         size_t     clientid)
{
   return T_Factory::getDevice_impl (devices, contextid, clientid);
};


#endif
