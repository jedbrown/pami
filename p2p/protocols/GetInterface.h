/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file p2p/protocols/GetInterface.h
/// \brief Defines base class interface for virtual address get operations.
///
#ifndef __p2p_protocols_GetInterface_h__
#define __p2p_protocols_GetInterface_h__

#include <pami.h>

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

namespace PAMI
{
  namespace Protocol
  {
    namespace Get
    {
      ///
      /// \brief Base class for point-to-point get implementations.
      ///
      class GetInterface
      {
        public:

          ///
          /// \brief Base class constructor for point-to-point get implementations.
          ///
          inline GetInterface () {};

          ///
          /// \brief Base class destructor.
          ///
          /// \note Any class with virtual functions must define a virtual
          ///       destructor.
          ///
          virtual ~GetInterface () {};

          ///
          /// \note This is required to make "C" programs link successfully
          ///       with virtual destructors
          ///
          inline void operator delete(void * p)
          {
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
          }

          ///
          /// \brief Start a new contiguous get operation
          ///
          virtual pami_result_t get (pami_get_simple_t * parameters) = 0;

          ///
          /// \brief Start a new non-contiguous get operation
          ///
          //virtual pami_result_t get (pami_get_typed_t * parameters) = 0;

      }; // PAMI::Protocol::GetInterface class


      class NoGet : public GetInterface
      {
        public:
          template <class T_Allocator>
          static GetInterface * generate (pami_context_t context, T_Allocator & allocator)
          {
            TRACE_ERR((stderr, ">> NoGet::generate()\n"));
            COMPILE_TIME_ASSERT(sizeof(NoGet) <= T_Allocator::objsize);

            GetInterface * get = (GetInterface *) allocator.allocateObject ();
            new ((void *)get) NoGet (context);

            TRACE_ERR((stderr, "<< NoGet::generate(), get = %p\n", get));
            return get;
          }

          inline NoGet (pami_context_t context) :
            GetInterface (),
            _context (context)
          {}

          inline ~NoGet () {};

          pami_result_t get (pami_get_simple_t * parameters)
          {
            if (parameters->rma.done_fn)
              parameters->rma.done_fn (_context, parameters->rma.cookie, PAMI_ERROR);

            return PAMI_ERROR;
          }

          pami_result_t get (pami_get_typed_t * parameters)
          {
            if (parameters->rma.done_fn)
              parameters->rma.done_fn (_context, parameters->rma.cookie, PAMI_ERROR);

            return PAMI_ERROR;
          }

        protected:

          pami_context_t _context;

      }; // PAMI::Protocol::NoGet class
    };   // PAMI::Protocol::Get namespace
  };     // PAMI::Protocol namespace
};       // PAMI namespace

#endif // __p2p_protocols_GetInterface_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
