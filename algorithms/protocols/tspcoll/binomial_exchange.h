/**
 * \file algorithms/protocols/tspcoll/binomial_exchange.h
 * \brief ???
 */
#ifndef __algorithms_protocols_tspcoll_binomial_exchange_h__
#define __algorithms_protocols_tspcoll_binomial_exchange_h__

/* **********************************************************************
    Binomial exchange tree interface and implementation; Used by one sided
    collectives;
   ********************************************************************** */
namespace xlpgas {

template <class Team>
struct  binomial_exchange {

  static int children(int node_id, int root_idx, Team* team){
    //return the number of children for this node
    size_t nc;
    int _numphases = -1;
    int myrelrank = (team->ordinal() + team->size() - root_idx) % team->size();
    for (int n=team->size()-1; n>0; n>>=1) _numphases++;
    for (int i=0, phase=_numphases; i<_numphases; i++)
    {
      int  dist       = 1<<(_numphases-1-i);
      int  sendmask   = (1<<(_numphases-i))-1;
      int  destrelrank= myrelrank + dist;
      bool dosend     = ((myrelrank&sendmask)==0)&&(destrelrank < team->size());
      int  destindex  = (destrelrank + root_idx)%team->size();
    }
    return 2;
  }

  static int child(int node_id, int child, int root_id, Team* team){
    return 2;
  }

  static int parent(int node_id, int root_id,Team* team){
    return 1;
  }

};

}//end namespace pgas
#endif
