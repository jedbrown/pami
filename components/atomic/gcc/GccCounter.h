/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __components_atomic_gcc_GccCounter_h__
#define __components_atomic_gcc_GccCounter_h__

/**
 * \file components/atomic/gcc/GccCounter.h
 * \brief gcc builtin atomics implementation of in-place Counter
 *
 */
#include "components/atomic/Counter.h"
#include "components/atomic/gcc/GccBuiltin.h"

#undef TRACE_ERR
#define TRACE_ERR(x) //fprintf x

namespace PAMI
{
  namespace Counter
  {

    class GccInPlaceCounter : public PAMI::Atomic::Interface::InPlaceCounter<GccInPlaceCounter>
    {
      public:
        GccInPlaceCounter() {}
        ~GccInPlaceCounter() {}

	static bool checkCtorMm(PAMI::Memory::MemoryManager *mm) {
		return true;
	}

	static bool checkDataMm(PAMI::Memory::MemoryManager *mm) {
		return true;
	}

        inline void init_impl()
        {
          TRACE_ERR((stderr,  "%s enter\n", __PRETTY_FUNCTION__));
	  new (&_addr) PAMI::Atomic::GccBuiltin();
        }
        inline size_t fetch_impl()
        {
          return _addr.fetch();
        }
        inline size_t fetch_and_inc_impl()
        {
          return _addr.fetch_and_inc();
        }
        inline size_t fetch_and_dec_impl()
        {
          return _addr.fetch_and_dec();
        }
        inline size_t fetch_and_clear_impl()
        {
          return _addr.fetch_and_clear();
        }
        inline void clear_impl()
        {
          _addr.clear();
        }
        void *returnLock_impl() { return &_addr; }
      protected:
        PAMI::Atomic::GccBuiltin _addr;
    }; // class GccInPlaceCounter

    class GccIndirCounter : public PAMI::Atomic::Interface::IndirCounter<GccIndirCounter>
    {
      public:
        GccIndirCounter() {}
        ~GccIndirCounter() {}

	static bool checkCtorMm(PAMI::Memory::MemoryManager *mm) {
		return ((mm->attrs() & PAMI::Memory::PAMI_MM_NODESCOPE) == 0);
	}

	static bool checkDataMm(PAMI::Memory::MemoryManager *mm) {
		return true;
	}

        inline void init_impl(PAMI::Memory::MemoryManager *mm, const char *key)
        {
          TRACE_ERR((stderr,  "%s enter\n", __PRETTY_FUNCTION__));
	  pami_result_t rc;
	  rc = mm->memalign((void **)&_addr, sizeof(*_addr), sizeof(*_addr), key);
	  PAMI_assertf(rc == PAMI_SUCCESS, "Failed to get memory for GccIndirCounter");
	  new (_addr) PAMI::Atomic::GccBuiltin();
        }
        inline size_t fetch_impl()
        {
          return _addr->fetch();
        }
        inline size_t fetch_and_inc_impl()
        {
          return _addr->fetch_and_inc();
        }
        inline size_t fetch_and_dec_impl()
        {
          return _addr->fetch_and_dec();
        }
        inline size_t fetch_and_clear_impl()
        {
          return _addr->fetch_and_clear();
        }
        inline void clear_impl()
        {
          _addr->clear();
        }
        void *returnLock_impl() { return _addr; }
      protected:
        PAMI::Atomic::GccBuiltin *_addr;
    }; // class GccIndirCounter

  }; // Counter namespace
}; // PAMI namespace

#endif // __components_atomic_gcccounter_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
