
#ifndef _PAMI_GEOMETRY_UNEXPBARRIERQUEUEELEMENT__
#define _PAMI_GEOMETRY_UNEXPBARRIERQUEUEELEMENT__

#include "util/queue/MatchQueue.h"

namespace PAMI {
  namespace Geometry {

    class UnexpBarrierQueueElement : public PAMI::MatchQueueElem {
      
    protected:
      unsigned          _comm;
      pami_quad_t       _info;
      unsigned          _srcrank;
      unsigned          _algorithm;
      
    public:
      UnexpBarrierQueueElement (unsigned comm, pami_quad_t &info, unsigned src, unsigned algorithm) : PAMI::MatchQueueElem(comm),
	_comm (comm), _info(info), _srcrank(src), _algorithm(algorithm) 
      {
      }
      
      unsigned       getComm() { return _comm; }
      
      pami_quad_t  & getInfo() { return _info; }
      
      unsigned       getSrcRank() { return _srcrank; }
      
      unsigned       getAlgorithm() { return _algorithm; }
    };

  };
};

#endif
