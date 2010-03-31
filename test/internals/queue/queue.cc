/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/internals/queue/queue.cc
 * \brief Test the queue interface features
 */

#include <stdio.h>
#include "sys/pami.h"

#include "Global.h"
#include "util/queue/Queue.h"
#ifdef __bgp__
#include "components/atomic/bgp/BgpAtomic.h"
#endif
#include "components/atomic/noop/Noop.h"

#define ELEMENTS 10240

class TestElement : public PAMI::Queue::Element
{
  public:

    TestElement () :
      PAMI::Queue::Element (),
      _value (0)
    {};

    void set (size_t value)
    {
      _value = value;
    };

    size_t get ()
    {
      return _value;
    };

  private:

    size_t _value;
};


int main(int argc, char **argv)
{
#ifdef __bgp__
  PAMI::MutexedQueue<PAMI::Mutex::BGP::BgpProcMutex> q;
#else
  PAMI::Queue q;
#endif
  TestElement element[ELEMENTS];
  TestElement * e = NULL;

  size_t i, tmp;
  for (i=0; i<10; i++)
  {
    element[i].set (10-i);
    tmp = q.size();
    q.push ((PAMI::Queue::Element *) &element[i]);
    fprintf (stdout, "Push element (%zu) .. q.size () = %zu -> %zu\n", element[i].get(), tmp, q.size());
  }

  fprintf (stdout, "\n");

  for (i=0; i<10; i++)
  {
    tmp = q.size();
    e = (TestElement *) q.pop ();
    fprintf (stdout, "Pop queue, e->get() = %zu .. q.size () = %zu -> %zu\n", e->get(), tmp, q.size());
  }

  for (i=0; i<ELEMENTS; i++) element[i].set (i);


  PAMI::Queue simpleq;
#ifdef __bgp__
  PAMI::MutexedQueue<PAMI::Mutex::BGP::BgpProcMutex> atomicq;
#endif

  unsigned long long t0, t1;

  fprintf (stdout, "\n");
#ifdef __bgp__
  t0 = __global.time.timebase();
  for (i=0; i<ELEMENTS; i++) atomicq.push ((PAMI::Queue::Element *) &element[i]);
  for (i=0; i<ELEMENTS; i++) e = (TestElement *) atomicq.pop ();
  t1 = __global.time.timebase();
  fprintf (stdout, "avg. atomic queue push-pop cycles: %lld\n", (t1-t0)/ELEMENTS);
#endif
  t0 = __global.time.timebase();
  for (i=0; i<ELEMENTS; i++) simpleq.push ((PAMI::Queue::Element *) &element[i]);
  for (i=0; i<ELEMENTS; i++) e = (TestElement *) simpleq.pop ();
  t1 = __global.time.timebase();
  fprintf (stdout, "avg. simple queue push-pop cycles: %lld\n", (t1-t0)/ELEMENTS);

  fprintf (stdout, "\n");

  return 0;
};
