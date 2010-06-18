/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/connmgr/CommSeqConnMgr.h
 * \brief ???
 */

#ifndef __algorithms_connmgr_CommSeqConnMgr_h__
#define __algorithms_connmgr_CommSeqConnMgr_h__

#include "algorithms/connmgr/ConnectionManager.h"

#include <map>

///
/// This connection manager encodes geometry id, collective sequnece and the phase of the collective in
/// connection id. In the connection id, bit 31 to bit 21 (11bits) are for geometry; bit 20 through 10 (11bits)
/// are collective sequence number; and bit 9 throught bit 0 (10bits) are identify phases of a collective
///

namespace CCMI
{
  namespace ConnectionManager
  {

    class CommSeqConnMgr : public ConnectionManager<CommSeqConnMgr>
    {

    public :
    static const unsigned COMMBITS=11;
    static const unsigned SEQBITS =11;
    static const unsigned PHABITS =10;
    static const unsigned COMMMASK=0x7FF;
    static const unsigned SEQMASK =0x7FF;

    protected:
      std::map<unsigned, unsigned>  _comm_seq_map;

    public:

      /// Constructor
      CommSeqConnMgr () : ConnectionManager<CommSeqConnMgr> ()
      {
      }

      ///
      /// \brief return the connection id given a set of inputs
      /// \param comm the communicator id of the collective
      /// \param phase the phase of the collective operation
      ///
      int getNumConnections_impl ()
        {
          PAMI_abort();
          return 0;
        }

      void setNumConnections_impl (size_t sz)
        {
          PAMI_abort();
          return;
        }


      unsigned getConnectionId_impl (unsigned comm, unsigned root, unsigned color,
                                     unsigned phase, unsigned dst)
      {
         unsigned cid = (((comm & COMMMASK) << SEQBITS) | (_comm_seq_map[comm] & SEQMASK));
         return cid;

      }

      unsigned getRecvConnectionId_impl (unsigned comm, unsigned root,
                                         unsigned src, unsigned phase, unsigned color)
      {
        PAMI_abort();
        return 0;
      }

      unsigned updateConnectionId (unsigned comm)
      {
        _comm_seq_map[comm] ++;
        _comm_seq_map[comm] &= COMMMASK;
        unsigned cid = (((comm & COMMMASK) << SEQBITS) | (_comm_seq_map[comm] & SEQMASK));
        return cid;
      }

      void setSequence (unsigned comm)
      {
	//XMI_assert(_comm_seq_map.size() < (0x1 << COMMBITS));
	_comm_seq_map[comm] = 0;
      }
    };
  };
};

#endif /* __algorithms_connmgr_CommSeqConnMgrT_h__ */
