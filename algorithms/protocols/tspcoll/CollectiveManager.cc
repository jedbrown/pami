/**
 * \file algorithms/protocols/tspcoll/CollectiveManager.cc
 * \brief ???
 */
#include "algorithms/protocols/tspcoll/Collective.h"

template <class T_NI>
std::vector<xlpgas::CollectiveManager<T_NI>*> xlpgas::CollectiveManager<T_NI>::_instances;

/* ************************************************************************ */
/*            Initialize the collective manager singleton                   */
/* ************************************************************************ */
template <class T_NI>
void xlpgas::CollectiveManager<T_NI>::Initialize (int context_id, xlpgas::CollectiveManager<T_NI>* cmgr)
{
  assert(context_id == _instances.size());
  _instances.push_back(cmgr);
}

/* ************************************************************************ */
/*       Collective life cycle manager: constructor                         */
/* ************************************************************************ */
template <class T_NI>
xlpgas::CollectiveManager<T_NI>::CollectiveManager(int ctxt): _ctxt(ctxt)
{
  for (int i=0; i<MAXKIND; i++)
    {
      _kindlist [i] = (CollArray_t *) __global.heap_mm->malloc (sizeof(CollArray_t));
      assert (_kindlist[i] != NULL);
      new (_kindlist[i]) CollArray_t ();
    }
}

/* ************************************************************************ */
/*              find an instance                                            */
/* ************************************************************************ */
template <class T_NI>
xlpgas::Collective<T_NI> *
xlpgas::CollectiveManager<T_NI>::find (CollectiveKind kind, int tag)
{
  assert (0 <= kind && kind < MAXKIND);
  return (*_kindlist[kind])[tag];
}
