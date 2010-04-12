/*
 * \file algorithms/interfaces/NativeInterface.h
 * \brief ???
 */

#ifndef __algorithms_interfaces_NativeInterface_h__
#define __algorithms_interfaces_NativeInterface_h__

#include "sys/pami.h"

namespace CCMI {
  namespace Interfaces {
    class NativeInterface {
    protected:
      unsigned         _myrank;
      unsigned         _numranks;

    public:
      NativeInterface(unsigned myrank, unsigned numranks): _myrank(myrank),_numranks(numranks){}
      ///
      /// \brief Virtual destructors make compilers happy.
      ///
      virtual inline ~NativeInterface() {};

      unsigned myrank()   { return _myrank; }
      unsigned numranks() { return _numranks; }

      /// \brief this call is called when an active message native interface is initialized and
      /// is not supported on all sided native interfaces
      virtual pami_result_t setDispatch  (pami_dispatch_callback_fn fn, void *cookie)
      {
        PAMI_abort();
      }
      virtual pami_result_t multicast    (pami_multicast_t *mcast) = 0;
      virtual pami_result_t multisync    (pami_multisync_t *msync) = 0;
      virtual pami_result_t multicombine (pami_multicombine_t *mcombine) = 0;
    };
  };
};

#endif
