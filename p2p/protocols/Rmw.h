/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file p2p/protocols/Rmw.h
/// \brief Defines base class interface for read-modify-write.
///
#ifndef __p2p_protocols_Rmw_h__
#define __p2p_protocols_Rmw_h__

#include <pami.h>

namespace PAMI
{
  namespace Protocol
  {
    namespace Rmw
    {
      ///
      /// \brief Base class for point-to-point rmw implementations.
      ///
      class Rmw
      {
        public:

          ///
          /// \brief Base class constructor for point-to-point rmw protocols.
          ///
          inline Rmw () {};

          ///
          /// \brief Base class destructor.
          ///
          /// \note Any class with virtual functions must define a virtual
          ///       destructor.
          ///
          virtual ~Rmw () {};

          /// \note This is required to make "C" programs link successfully with virtual destructors
          inline void operator delete(void * p)
          {
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
          }

          ///
          /// \brief Start a new read-modify-write operation
          ///
          /// \param[in] parameters read-modify-write parameter structure
          ///
          virtual pami_result_t start (pami_rmw_t * parameters) = 0;

      }; // PAMI::Protocol::Rmw::Rmw class

      class Error : public Rmw
      {
        public:
          inline Error () {};
          virtual ~Error () {};

          virtual pami_result_t start (pami_rmw_t * parameters)
          {
            return PAMI_ERROR;
          };

      }; // PAMI::Protocol::Rmw::Error class
    };   // PAMI::Protocol::Rmw namespace
  };     // PAMI::Protocol namespace
};       // PAMI namespace

#endif // __p2p_protocols_Rmw_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
