
#include "interfaces/Schedule.h"

namespace CCMI {
  namespace Schedule {

    ///
    /// \brief The Map interface for the Multinomial schedule. The
    ///        multinomial schedule just operates on ranks 0-N, where 
    ///        0-m nodes for a multinomial tree of radix k. The remaining 
    ///        nodes are auxilliary nodes that send messages to the 
    ///        internal nodes.
    ///
    template <class T> class MultinomialMap {      
    public:
      MultinomialMap () {}

      void setRoot (unsigned r ) {
	static_cast<T*>(this)->setRoot(r);
      }

      ///
      /// \brief Is the rank an auxilary processor
      /// \param [in] rank the rank of the processor
      ///
      bool isAuxProc   (unsigned rank) {
	return static_cast<T*>(this)->isAuxProc(rank);
      }

      ///
      /// \brief Is the rank a peer processor that takes the data from
      ///        the auxillary processor and participates in the 
      ///        binomial collective. At the end of the collective 
      ///        operation result will be returned to the auxillary 
      ///        processor
      /// \param [in] the rank of the processor      
      ///
      bool isPeerProc  (unsigned rank) {
	return static_cast<T*>(this)->isPeerProc(rank);
      }

      ///
      /// \brief Get the aux processor for this peer processor
      /// \param [in] the rank of the aux processor
      /// \retval the rank of the peer processor
      ///
      unsigned  getAuxForPeer (unsigned rank){
	return static_cast<T*>(this)->getAuxForPeer(rank);
      }

      ///
      /// \brief Get the rank of the peer processor for the aux processor
      /// \param [in] The rank of the peer processor
      /// \retval the rank of the aux processor
      ///
      unsigned  getPeerForAux (unsigned rank) {
	return static_cast<T*>(this)->getPeerForAux(rank);
      }      

      ///
      /// \brief Convert the rank to the global rank for the msend
      ///        interface
      ///
      unsigned getGlobalRank (unsigned relrank) {
	return static_cast<T*>(this)->getGlobalRank(relrank);
      }

      ///
      /// \brief Get the root of the collective
      ///
      unsigned getRoot () {
	return static_cast<T*>(this)->getRoot();
      }
      
      ///
      /// \brief Get my rank in the collective
      ///
      unsigned getMyRank () {
	return static_cast<T*>(this)->getMyRank();
      }

      unsigned getNumRanks () {
	return static_cast<T*>(this)->getNumRanks();
      }

    };  //Multinomial map


    class LinearMap : public MultinomialMap <LinearMap> {
    public:

      LinearMap (unsigned myrank, XMI::Topology *topology) 
      {
        CCMI_assert (topology->analyzeTopology() == XMI_RANGE_TOPOLOGY);
	topology->rankRange(&_x0, &_xN);       
	_nranks = _xN - _x0 + 1;
	_xM = myrank - _x0;
	
	unsigned nph = 0;
	for(unsigned i = _nranks; i > 1; i >>= 1) {
	  nph++;
	}
	_hnranks = 1 << nph;

	_xR      = (unsigned) -1;
      }          

      void setRoot (unsigned r) {
	CCMI_assert (r >= _x0 && r <= _xN);	
	_xR = r - _x0;
      }

      ///
      /// \brief Is the rank an auxilary processor
      /// \param [in] rank the rank of the processor
      ///
      bool isAuxProc   (unsigned rank) {
	if (rank >= _hnranks)
	  return true;
	return false;
      }

      ///
      /// \brief Is the rank a peer processor that takes the data from
      ///        the auxillary processor and participates in the 
      ///        binomial collective. At the end of the collective 
      ///        operation result will be returned to the auxillary 
      ///        processor
      /// \param [in] the rank of the processor      
      ///
      bool isPeerProc  (unsigned rank) {
	if (rank < (_nranks - _hnranks))
	  return true;
	return false;
      }

      ///
      /// \brief Get the aux processor for this peer processor
      /// \param [in] the rank of the aux processor
      /// \retval the rank of the peer processor
      ///
      unsigned  getAuxForPeer (unsigned rank){
	return rank + _hnranks;
      }

      ///
      /// \brief Get the rank of the peer processor for the aux processor
      /// \param [in] The rank of the peer processor
      /// \retval the rank of the aux processor
      ///
      unsigned  getPeerForAux (unsigned rank) {
	return rank - _hnranks;
      }      

      ///
      /// \brief Convert the rank to the global rank for the msend
      ///        interface
      ///
      unsigned getGlobalRank (unsigned relrank) {
	relrank += _xR;
        if(relrank >= _nranks) relrank -= _nranks;
	return relrank + _x0;
      }
      
      ///
      /// \brief Get my rank in the collective
      ///
      unsigned getMyRank () {
	if (_xM > _xR)
	  return _xM - _xR;
	
	return _xM + _nranks - _xR;	  
      }

      unsigned getNumRanks () { return _nranks; }

    protected:
      unsigned              _x0;     /** The rank of the first node  */
      unsigned              _xN;     /** The rank of the last node  */
      unsigned              _xR;     /** The relative rank of the root */
      unsigned              _xM;     /** My relative rank */
      unsigned              _nranks;  /** Number of ranks */       
      unsigned              _hnranks; /** Nearest power of 2 */
    };  

    
    class ListMap : public MultinomialMap <ListMap> {
    public:
      
      ListMap (unsigned myrank, Topology *topology)
      {
	topology->list (&ranks);
	_nranks = topology->size();	
	
	unsigned nph = 0;
	for(unsigned i = _nranks; i > 1; i >>= 1) {
	  nph++;
	}
	_hnranks = 1 << nph;	
	_rootindex = 0;
	
#if 0
	for (unsigned count = 0; count < _nranks; count++) 
	  if (_ranks[count] == _mapping->rank()) {
	    _myindex = count;
	    break;
	  }
#endif
	_myindex = myrank;
      }          

      void setRoot (unsigned gr) {	
	for (unsigned count = 0; count < _nranks; count++) 
	  if (_ranks[count] == gr) {
	    _rootindex = count;
	    break;
	  }
      }
      
      ///
      /// \brief Is the rank an auxilary processor
      /// \param [in] rank the rank of the processor
      ///
      bool isAuxProc   (unsigned rank) {
	if (rank >= _hnranks)
	  return true;
	return false;
      }

      ///
      /// \brief Is the rank a peer processor that takes the data from
      ///        the auxillary processor and participates in the 
      ///        binomial collective. At the end of the collective 
      ///        operation result will be returned to the auxillary 
      ///        processor
      /// \param [in] the rank of the processor      
      ///
      bool isPeerProc  (unsigned rank) {
	if (rank < (_nranks - _hnranks))
	  return true;
	return false;
      }

      ///
      /// \brief Get the aux processor for this peer processor
      /// \param [in] the rank of the aux processor
      /// \retval the rank of the peer processor
      ///
      unsigned  getAuxForPeer (unsigned rank){
	return rank + _hnranks;
      }

      ///
      /// \brief Get the rank of the peer processor for the aux processor
      /// \param [in] The rank of the peer processor
      /// \retval the rank of the aux processor
      ///
      unsigned  getPeerForAux (unsigned rank) {
	return rank - _hnranks;
      }      

      ///
      /// \brief Convert the rank to the global rank for the msend
      ///        interface
      ///
      unsigned getGlobalRank (unsigned relrank) {
	relrank += _rootindex;
        if(relrank >= _nranks) relrank -= _nranks;
	return _ranks[relrank];
      }
      
      ///
      /// \brief Get my rank in the collective
      ///
      unsigned getMyRank () {
	if (_myindex >= _rootindex)
	  return _myindex - _rootindex;
	
	return _myindex + _nranks - _rootindex; 
      }

      unsigned getNumRanks () { return _nranks; }

    protected:
      unsigned             * _ranks;     /** List of ranks */
      unsigned              _nranks;    /** Number of ranks */       
      unsigned              _hnranks;   /** Nearest power of 2 */
      unsigned              _rootindex; /** Index of the root */
      unsigned              _myindex;   /** Index of my node */
    };  
  };
};
