/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file p2p/protocols/Fence.h
/// \brief Defines base class interface for fence operations.
///
#ifndef __p2p_protocols_Fence_h__
#define __p2p_protocols_Fence_h__

#include <pami.h>

#include "util/trace.h"

#define DO_TRACE_ENTEREXIT 0
#define DO_TRACE_DEBUG     0

namespace PAMI
{
  namespace Protocol
  {
    namespace Fence
    {
      ///
      /// \brief Base class for point-to-point fence implementations.
      ///
      class Fence
      {
        protected:

          ///
          /// \brief Base class constructor for point-to-point fence operations.
          ///
          inline Fence () {};

          ///
          /// \brief Base class destructor.
          ///
          /// \note Any class with virtual functions must define a virtual
          ///       destructor.
          ///
          virtual ~Fence () {};

          /// \note This is required to make "C" programs link successfully with virtual destructors
          inline void operator delete(void * p)
          {
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
          }

        public:

          ///
          /// \brief Start a new fence operation to a single endpoint
          ///
          /// \param [in] done_fn Completion callback function
          /// \param [in] cookie  Completion callback cookie
          /// \param [in] target  Fence target endpoint
          ///
          virtual pami_result_t endpoint (pami_event_function   done_fn,
                                          void                * cookie,
                                          pami_endpoint_t       target) = 0;

          ///
          /// \brief Start a new fence operation to all endpoints
          ///
          /// \param [in] done_fn Completion callback function
          /// \param [in] cookie  Completion callback cookie
          ///
          virtual pami_result_t all (pami_event_function   done_fn,
                                     void                * cookie) = 0;

      }; // PAMI::Protocol::Fence::Fence class

      class Error : public Fence
      {
        public:

          inline Error () {};

          virtual ~Error () {};

          virtual pami_result_t endpoint (pami_event_function   done_fn,
                                          void                * cookie,
                                          pami_endpoint_t       target)
          {
            TRACE_FN_ENTER();
            TRACE_FORMAT( "return PAMI_ERROR (%d)", PAMI_ERROR);
            TRACE_FN_EXIT();
            return PAMI_ERROR;
          };

          virtual pami_result_t all (pami_event_function   done_fn,
                                     void                * cookie)
          {
            TRACE_FN_ENTER();
            TRACE_FORMAT( "return PAMI_ERROR (%d)", PAMI_ERROR);
            TRACE_FN_EXIT();
            return PAMI_ERROR;
          };

      }; // PAMI::Protocol::Fence::Error class
    };   // PAMI::Protocol::Fence namespace
  };     // PAMI::Protocol namespace
};       // PAMI namespace
#undef DO_TRACE_ENTEREXIT
#undef DO_TRACE_DEBUG

#endif // __p2p_protocols_Fence_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
