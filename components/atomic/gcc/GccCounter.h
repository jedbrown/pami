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
 * \brief gcc builtin atomics implementation of Node- or Process-scoped Counter
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

    class GccNodeCounter : public PAMI::Atomic::Interface::Counter<GccNodeCounter>
    {
      public:
        GccNodeCounter() {}
        ~GccNodeCounter() {}
        inline void init_impl(PAMI::Memory::MemoryManager *mm)
        {
          TRACE_ERR((stderr,  "%s enter\n", __PRETTY_FUNCTION__));
          mm->memalign((void **)&_addr, sizeof(*_addr), sizeof(*_addr));
          _addr->init(mm);
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
        void *returnLock_impl() { return _addr->returnLock(); }
      protected:
        PAMI::Atomic::GccBuiltin *_addr;
    }; // class GccNodeCounter

    class GccProcCounter : public PAMI::Atomic::Interface::Counter<GccProcCounter>
    {
      public:
        GccProcCounter() {}
        ~GccProcCounter() {}
        inline void init_impl(PAMI::Memory::MemoryManager *mm)
        {
          _addr.init(mm);
          // This is ok for proc counters since only one calls init()
          _addr.clear();
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
        void *returnLock_impl() { return _addr.returnLock(); }
      protected:
        PAMI::Atomic::GccBuiltin _addr;
    }; // class GccProcCounter

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
