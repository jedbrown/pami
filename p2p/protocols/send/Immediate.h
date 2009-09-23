/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file p2p/protocols/send/Immediate.h
/// \brief Defines base class interface for immediate sends.
///
#ifndef __xmi_p2p_protocols_send_immediate_h__
#define __xmi_p2p_protocols_send_immediate_h__

#include "sys/xmi.h"

namespace XMI
{
  namespace Protocol
  {
    namespace Send
    {
      ///
      /// \brief Base class for point-to-point immediate send implementations.
      ///
      class Immediate
      {
        public:

          ///
          /// \brief Base class constructor for point-to-point immediate sends.
          ///
          inline Immediate () {};

          ///
          /// \brief Base class destructor.
          ///
          /// \note Any class with virtual functions must define a virtual
          ///       destructor.
          ///
          virtual ~Immediate () {};

          ///
          /// \brief Start a new simple send message.
          ///
          /// \param[in]  peer      Destination peer.
          /// \param[in]  src       Send data buffer.
          /// \param[in]  bytes     Send data length in bytes.
          /// \param[in]  msginfo   Opaque application header information.
          /// \param[in]  mbytes    Number of opaque application header bytes.
          ///
          virtual xmi_result_t start (size_t               peer,
                                      void               * src,
                                      size_t               bytes,
                                      void               * msginfo,
                                      size_t               mbytes) = 0;

      };
    };
  };
};

#endif // __xmi_p2p_protocols_send_immediate_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
