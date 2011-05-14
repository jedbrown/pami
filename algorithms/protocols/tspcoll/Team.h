#ifndef __xlpgas_communicator_h__
#define __xlpgas_communicator_h__

#include "stdio.h"
#include "algorithms/protocols/tspcoll/Collective.h"


/* ************************************************************************* */
/* ************************************************************************* */

namespace xlpgas
{
#if 0
  template <class T_NI>
  class Team
  {
  public:
    static const int MAX_COMMS = 1024;

    static void Initialize (int ncontexts);
    static Team * get (int ctxt, int teamID);
    static void set (int ctxt, int teamID, Team *);
    int allocate_team_id () { return ++_max_team_id;}

  public:
    int                      commID     (void)        const { return _id; }
    virtual int              ordinal    (void)        const;
    virtual int              size       (void)        const;
    virtual xlpgas_endpoint_t endpoint   (void)       const;//current endpoint
    virtual xlpgas_endpoint_t endpoint   (int ordinal) const;
    Collective<T_NI>             * coll       (CollectiveKind) const;
    Team(){}
    Team (int commID, xlpgas_endpoint_t me, xlpgas_endpoint_t size);
    void * operator new (size_t, void * addr) { return addr; }

    /* ---------------------- */
    /* communicator utilities */
    /* ---------------------- */

    virtual void split(int ctxt, int newID, int mynewrank, int nsize, xlpgas_endpoint_t *plist);

  protected:
    int               _id;
    xlpgas_endpoint_t  _me;
    xlpgas_endpoint_t  _size;
    Collective<T_NI>      * _colls[MAXKIND];

  protected:
    static Team *     _instances[MAX_COMMS];
    static int        _ncontexts;
    int        _max_team_id;
  };
#endif



  /*
  template <class Tm>
  void create_teams(Tm*& inst, int& _nctxt, int ncontexts){
    assert (ncontexts > 0);
    _nctxt = ncontexts;

    //inst = (Team *) __global.heap_mm->malloc (sizeof(Team) *
    //		    ncontexts * Tm::MAX_COMMS);
    //assert (inst != NULL);

    for (int i=0; i<ncontexts; i++)
      {
	xlpgas_endpoint_t me    = { XLPGAS_MYNODE, i };
	xlpgas_endpoint_t size  = { XLPGAS_NODES, ncontexts };
	Tm* nt = (Tm*) __global.heap_mm->malloc (sizeof(Tm));
	new (nt) Tm (0, me, size);
	inst
      }
  }//end create teams
  */

}//end namespace

#endif
