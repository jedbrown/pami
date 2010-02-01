/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/BaseDevice.h
 * \brief ???
 */

#ifndef __components_devices_BaseDevice_h__
#define __components_devices_BaseDevice_h__

#include "sys/xmi.h"
#include "SysDep.h"

namespace XMI
{
  namespace Device
  {
    namespace Interface
    {
      ///
      /// \tparam T_device  device template class
      ///
      template <class T_Device>
      class BaseDevice
      {
        public:
          ///
          /// \brief Base device constructor
          ///
          BaseDevice ()
          {}

          ///
          /// \brief Initialize the device
          ///
          /// \attention All device derived classes \b must
          ///            implement the init_impl() method.
          ///
          /// \param[in] sd      Sysdep reference
          /// \param[in] context Communication context associated with the
          ///                    device instantiation
          /// \param[in] offset  Communcation context offset indentifier
          ///
          /// \todo return type
          ///
          /// \return  Return code of the device init status
          ///
          inline int init (SysDep        *sd,
			   size_t        clientid,
			   size_t        num_ctx,
                           xmi_context_t   context,
                           size_t          contextid);

          inline xmi_context_t getContext ();

          inline size_t getContextOffset ();

          ///
          /// \brief Is the device initialized?
          ///
          /// \attention All device derived classes \b must
          ///            implement the isInit_impl() method.
          ///
          inline bool isInit ();

          ///
          /// \brief Number of addressable 'peer' tasks for this device
          ///
          /// \attention All device derived classes \b must
          ///            implement the peers_impl() method.
          ///
          inline size_t peers ();

          ///
          /// \brief Converts a global task id to a device peer id.
          ///
          /// The device peer ids monotonically increase from zero to peers-1.
          ///
          /// \attention All device derived classes \b must
          ///            implement the task2peer_impl() method.
          ///
          inline size_t task2peer (size_t task);

          ///
          /// \brief Advance routine for the device.
          ///
          /// \attention All device derived classes \b must
          ///            implement the advance_impl() method.
          ///
          /// \todo Define advance function return value
          ///
          /// \return Return code of the advance routine
          ///
          inline int advance ();
      };

      template <class T_Device>
      inline int BaseDevice<T_Device>::init (SysDep        * sd,
			   size_t        clientid,
			   size_t        num_ctx,
                           xmi_context_t   context,
                           size_t          contextid)
      {
        return static_cast<T_Device*>(this)->init_impl(sd, clientid, num_ctx, context, contextid);
      }

      template <class T_Device>
      inline xmi_context_t BaseDevice<T_Device>::getContext ()
      {
        return static_cast<T_Device*>(this)->getContext_impl();
      }

      template <class T_Device>
      inline size_t BaseDevice<T_Device>::getContextOffset ()
      {
        return static_cast<T_Device*>(this)->getContextOffset_impl();
      }

      template <class T_Device>
      inline bool BaseDevice<T_Device>::isInit ()
      {
        return static_cast<T_Device*>(this)->isInit_impl();
      }

      template <class T_Device>
      inline size_t BaseDevice<T_Device>::peers ()
      {
        return static_cast<T_Device*>(this)->peers_impl();
      }

      template <class T_Device>
      inline size_t BaseDevice<T_Device>::task2peer (size_t task)
      {
        return static_cast<T_Device*>(this)->task2peer_impl(task);
      }

      template <class T_Device>
      inline int BaseDevice<T_Device>::advance ()
      {
        return static_cast<T_Device*>(this)->advance_impl();
      }
    };
  };
};
#undef TRACE
#endif // __components_devices_basedevice_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
