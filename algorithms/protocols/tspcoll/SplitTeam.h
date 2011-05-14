#ifndef __xlpgas_split_team_h__
#define __xlpgas_split_team_h__

#include <Team.h>
#include <algorithms/protocols/tspcoll/Collectives.h>

namespace xlpgas{

  /* ******************************************************************* */
  /*                      Enumerated Team                                */
  /* ******************************************************************* */

  template <class Base>
  class EnumTeam: public Base
  {
  public:
    void * operator new (size_t, void * addr) { return addr; }

    EnumTeam (int commID, xlpgas_endpoint_t my_ep, int mynewrank, int nsize, xlpgas_endpoint_t* proclist) {
      //assert (size > 0);
      _ordinal_rank = mynewrank;
      _proclist = proclist; //the endpoints corresponding to this team;

      //what used to be in the base constructor
      this->_id = commID;
      this->_me = my_ep;
      this->_size.node = nsize;
      for (int i=0; i<MAXKIND; i++)
	this->_colls[i] = CollectiveManager::
	  instance(this->_me.ctxt)->allocate (this, (CollectiveKind)i, base_coll_defs());
    }

    int size () const {
      return _size.node;
    }

    int ordinal () const {
      return this->_ordinal_rank;
    }

    xlpgas_endpoint_t endpoint (int ordinal) const {
      return _proclist[ordinal];
    }

  protected:
    xlpgas_endpoint_t * _proclist; /* list of absolute (node) ranks */
    int _ordinal_rank;//stored; not computed as it may not be a formula anymore
  };

  /* ******************************************************************* */
  /*                     blocked communicator                            */
  /* ******************************************************************* */
  /*
  class BC_Comm: public Team
  {
  public:
    void * operator new (size_t, void * addr) { return addr; }
    BC_Comm (int commID, int BF, int ncomms);

    virtual int  absrankof  (int rank) const;
    virtual int  virtrankof (int rank) const;

  protected:
    int _BF, _ncomms, _mycomm;
  };
  */
}//end namespace

#endif
