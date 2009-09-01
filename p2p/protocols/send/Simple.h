/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file p2p/protocols/send/Simple.h
/// \brief Defines base class interface for simple sends.
///
#ifndef __xmi_p2p_protocols_send_simple_h__
#define __xmi_p2p_protocols_send_simple_h__

#include "sys/xmi.h"

namespace XMI
{
  namespace Protocol
  {
    namespace Send
    {
      ///
      /// \brief Base class for point-to-point simple send implementations.
      ///
      class Simple
      {
        public:

          ///
          /// \brief Base class constructor for point-to-point simple sends.
          ///
          inline Simple () {};

          ///
          /// \brief Start a new simple send message.
          ///
          /// \param[in]  local_fn  Callback to invoke on local node when
          ///                       message local source data is completely sent.
          /// \param[in]  remote_fn Callback to invoke on local node when
          ///                       message is completely received on remote node.
          /// \param[in]  cookie    Opaque application callback data.
          /// \param[in]  rank      Destination rank.
          /// \param[in]  bytes     Send data length in bytes.
          /// \param[in]  src       Send data buffer.
          /// \param[in]  msginfo   Opaque application header information.
          /// \param[in]  mbytes    Number of msginfo bytes.
          ///
          virtual xmi_result_t start (xmi_event_function   local_fn,
                                      xmi_event_function   remote_fn,
                                      void               * cookie,
                                      size_t               peer,
                                      void               * src,
                                      size_t               bytes,
                                      void               * msginfo,
                                      size_t               mbytes) = 0;

      };
    };
  };
};

#endif // __xmi_p2p_protocols_send_simple_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//

