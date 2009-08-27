/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/BaseDevice.h
 * \brief ???
 */

#ifndef __components_devices_basedevice_h__
#define __components_devices_basedevice_h__

#include "xmi.h"

namespace XMI
{
  namespace Device
  {
    namespace Interface
    {
      ///
      /// \param T  device template class
      ///
      template <class T>
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
          inline int init (SysDep & sd);

          ///
          /// \brief Is the device initialized?
          ///
          /// \attention All device derived classes \b must
          ///            implement the isInit_impl() method.
          ///
          /// \todo return type
          ///
          inline bool isInit ();

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

      template <class T>
      inline int BaseDevice<T>::init (SysDep & sd)
      {
        return static_cast<T*>(this)->init_impl(sd);
      }

      template <class T>
      inline bool BaseDevice<T>::isInit ()
      {
        return static_cast<T*>(this)->isInit_impl();
      }

      template <class T>
      inline int BaseDevice<T>::advance ()
      {
        return static_cast<T*>(this)->advance_impl();
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
