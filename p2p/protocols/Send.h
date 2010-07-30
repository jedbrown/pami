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

#include <pami.h>

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
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
          }

          ///
          /// \brief Query the value of one or more attributes
          ///
          /// \param [in,out] configuration  The configuration attribute(s) of interest
          /// \param [in]     num_configs    The number of configuration elements
          ///
          /// \see PAMI_Dispatch_query()
          ///
          /// \retval PAMI_SUCCESS  The query has completed successfully.
          /// \retval PAMI_INVAL    The query has failed due to invalid parameters.
          ///
          virtual pami_result_t getAttributes (pami_configuration_t  configuration[],
                                               size_t                num_configs) = 0;

          ///
          /// \brief Start a new immediate send message.
          ///
          /// \param[in] parameters Immediate send parameter structure
          ///
          virtual pami_result_t immediate (pami_send_immediate_t * parameters) = 0;

          ///
          /// \brief Start a new simple send message.
          ///
          /// \param[in] parameters Simple send parameter structure
          ///
          virtual pami_result_t simple (pami_send_t * parameters) = 0;

      }; // PAMI::Protocol::Send::Send class
    };   // PAMI::Protocol::Send namespace
  };     // PAMI::Protocol namespace
};       // PAMI namespace

#endif // __pami_p2p_protocols_send_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
