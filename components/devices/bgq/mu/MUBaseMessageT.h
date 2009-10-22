#ifndef __components_devices_bgq_mu_MUBaseMessageT_h__
#define __components_devices_bgq_mu_MUBaseMessageT_h__

///
/// \brief The MU Message base class template. This explore Barton Nackman and
/// the Common Device Interface
///

#include "util/queue/Queue.h"

namespace XMI
{
  namespace Device
  {
    namespace MU
    {

      template <class T>
      class MUBaseMessageT : public QueueElem
      {
        public:
          ///
          /// \brief Default and only constructor. This class is a bare
          /// bones base class template to optimize overheads
          ///
          MUBaseMessageT () : QueueElem() { }

          //////////////////////////////////////////////////////////////////////
          ///  \brief posts a message and begins the send
          //////////////////////////////////////////////////////////////////////
          int       start();

          //////////////////////////////////////////////////////////////////
          /// \brief     Reset the message, set state to initialized
          /// \returns:  Initialized, and resets the message
          //////////////////////////////////////////////////////////////////
          int       reset();

      }; // XMI::Device::MU::MUBaseMessageT class
    };   // XMI::Device::MU namespace
  };     // XMI::Device namespace
};       // XMI namespace

template <class T>
inline int XMI::Device::MU::MUBaseMessageT<T>::start()
{
  return static_cast<T*>(this)->start_impl();
}

template <class T>
inline int XMI::Device::MU::MUBaseMessageT<T>::reset()
{
  return static_cast<T*>(this)->reset_impl();
}

#endif

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
