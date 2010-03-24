/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file p2p/protocols/Send.h
/// \brief Defines base class interface for sends.
///
#ifndef __p2p_protocols_Send_h__
#define __p2p_protocols_Send_h__

#include "sys/pami.h"

namespace PAMI
{
  namespace Protocol
  {
    namespace Send
    {
      ///
      /// \brief Base class for point-to-point send implementations.
      ///
      class Send
      {
        public:

          ///
          /// \brief Base class constructor for point-to-point sends.
          ///
          inline Send () {};

          ///
          /// \brief Base class destructor.
          ///
          /// \note Any class with virtual functions must define a virtual
          ///       destructor.
          ///
          virtual ~Send () {};

          /// \note This is required to make "C" programs link successfully with virtual destructors
          inline void operator delete(void * p)
          {
            PAMI_abortf("%s<%d>\n",__FILE__,__LINE__);
          }

          ///
          /// \brief Start a new immediate send message.
          ///
          /// \param[in]  task      Destination task.
          /// \param[in]  src       Send data buffer.
          /// \param[in]  bytes     Send data length in bytes.
          /// \param[in]  msginfo   Opaque application header information.
          /// \param[in]  mbytes    Number of opaque application header bytes.
          ///
#if 0
          virtual pami_result_t immediate (pami_task_t           task,
                                          void               * src,
                                          size_t               bytes,
                                          void               * msginfo,
                                          size_t               mbytes) = 0;
#endif
          virtual pami_result_t immediate (pami_send_immediate_t * parameters) = 0;

          ///
          /// \brief Start a new simple send message.
          ///
          /// \param[in]  local_fn  Callback to invoke on local node when
          ///                       message local source data is completely sent.
          /// \param[in]  remote_fn Callback to invoke on local node when
          ///                       message is completely received on remote node.
          /// \param[in]  cookie    Opaque application callback data.
          /// \param[in]  task      Destination task.
          /// \param[in]  bytes     Send data length in bytes.
          /// \param[in]  src       Send data buffer.
          /// \param[in]  msginfo   Opaque application header information.
          /// \param[in]  mbytes    Number of msginfo bytes.
          ///
#if 0
          virtual pami_result_t simple (pami_event_function   local_fn,
                                       pami_event_function   remote_fn,
                                       void               * cookie,
                                       pami_task_t           task,
                                       void               * src,
                                       size_t               bytes,
                                       void               * msginfo,
                                       size_t               mbytes) = 0;
#endif
          virtual pami_result_t simple (pami_send_t * parameters) = 0;

      }; // PAMI::Protocol::Send class
    }; // PAMI::Protocol::Send class
  };   // PAMI::Protocol namespace
};     // PAMI namespace

#endif // __pami_p2p_protocols_send_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
