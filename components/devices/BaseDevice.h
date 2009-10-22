/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
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

namespace XMI
{
  namespace Device
  {
    namespace Interface
    {
      ///
      /// \param T  device template class
      ///
      template <class T_Device, class T_SysDep>
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
          /// \param[in] sd Sysdep reference
          ///
          /// \todo return type
          ///
          /// \return  Return code of the device init status
          ///
          inline int init (T_SysDep * sd);

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

      template <class T_Device, class T_SysDep>
      inline int BaseDevice<T_Device, T_SysDep>::init (T_SysDep * sd)
      {
        return static_cast<T_Device*>(this)->init_impl(sd);
      }

      template <class T_Device, class T_SysDep>
      inline bool BaseDevice<T_Device, T_SysDep>::isInit ()
      {
        return static_cast<T_Device*>(this)->isInit_impl();
      }

      template <class T_Device, class T_SysDep>
      inline size_t BaseDevice<T_Device, T_SysDep>::peers ()
      {
        return static_cast<T_Device*>(this)->peers_impl();
      }

      template <class T_Device, class T_SysDep>
      inline size_t BaseDevice<T_Device, T_SysDep>::task2peer (size_t task)
      {
        return static_cast<T_Device*>(this)->task2peer_impl(task);
      }

      template <class T_Device, class T_SysDep>
      inline int BaseDevice<T_Device, T_SysDep>::advance ()
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
