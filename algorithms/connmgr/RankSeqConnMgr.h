/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/connmgr/RankSeqConnMgr.h
 * \brief ???
 */

#ifndef __algorithms_connmgr_RankSeqConnMgr_h__
#define __algorithms_connmgr_RankSeqConnMgr_h__

#include "algorithms/connmgr/ConnectionManager.h"

#include <map>

///
/// This connection manager encodes collective sequence and the rank of the collective in
/// connection id. In the connection id, bit 31 to bit 9 (23 bits) are for rank; bit 8 through 0 (9 bits)
/// are for collective sequence number
///

namespace CCMI
{
  namespace ConnectionManager
  {

    class RankSeqConnMgr : public ConnectionManager<RankSeqConnMgr>
    {

      public :
        static const unsigned _rankbits = 23;
        static const unsigned _seqbits  = 9;
        static const unsigned _seqmask  = 0x1FF;
        static const unsigned _rankmask = 0x7FFFFF;

      protected:
        std::map<unsigned, unsigned>  _rank_seq_map;
        unsigned                      _connid;
        int                           _nconn;

      public:

        /// Constructor
        RankSeqConnMgr (unsigned connid = (unsigned) - 1) :
            ConnectionManager<RankSeqConnMgr> (),
            _connid(connid)
        {
        }

        ///
        /// \brief return the connection id given a set of inputs
        /// \param comm The communicator id of the collective
        /// \param root The root of the collective
        /// \param phase The phase of the collective operation
        ///

        unsigned getConnectionId_impl (unsigned comm, unsigned root, unsigned color,
                                       unsigned phase, unsigned dst)
        {
          if (_connid != (unsigned) - 1) return _connid;

          unsigned cid = (((root & _rankmask) << _seqbits) | ((_rank_seq_map[root] & _seqmask)));
          return cid;
        }

        unsigned getRecvConnectionId_impl (unsigned comm, unsigned root, unsigned src, unsigned phase,
                                           unsigned color)
        {
          if (_connid != (unsigned) - 1) return _connid;

          unsigned cid = (((src & _rankmask) << _seqbits) | ((_rank_seq_map[src] & _seqmask)));
          return cid;
        }

        void setNumConnections_impl(int nconn)
        {
          _nconn = nconn;
        }

        int getNumConnections_impl ()
        {
          return _nconn;
        }

        unsigned updateConnectionId (unsigned root)
        {
          // STL default constructs POD types (to zero), hence no explicit check/initialization needed
          _rank_seq_map[root] ++;
          _rank_seq_map[root] &= _seqmask;
          unsigned cid = (((root & _rankmask) << _seqbits) | ((_rank_seq_map[root] & _seqmask)));
          return cid;
        }
    };
  };
};

#endif /* __algorithms_connmgr_RankSeqConnMgrT_h__ */
