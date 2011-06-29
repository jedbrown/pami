/*
 * \file algorithms/interfaces/NativeInterface.h
 * \brief ???
 */

#ifndef __algorithms_interfaces_NativeInterface_h__
#define __algorithms_interfaces_NativeInterface_h__

#include <pami.h>
#include "util/common.h"
#include "PipeWorkQueue.h"
#include "components/devices/MulticastModel.h"
#include "components/devices/MultisyncModel.h"
#include "components/devices/MulticombineModel.h"
#include "components/devices/ManytomanyModel.h"

namespace CCMI
{
  namespace Interfaces
  {


    class NativeInterface
    {
      protected:
        unsigned         _myrank;
        unsigned         _numranks;
        pami_result_t    _status;

      public:
        NativeInterface(unsigned myrank,
                        unsigned numranks): _myrank(myrank), _numranks(numranks), _status(PAMI_SUCCESS) {}
        ///
        /// \brief Virtual destructors make compilers happy.
        ///
        virtual inline ~NativeInterface() {};

        unsigned myrank()   { return _myrank; }
        unsigned numranks() { return _numranks; }
        pami_result_t status() { return _status; }

        /// \brief this call is called when an active message native interface is initialized and
        /// is not supported on all sided native interfaces
        virtual pami_result_t setMulticastDispatch(pami_dispatch_multicast_function fn,
                                                   void *cookie)
        {
          //PAMI_abort();
          return PAMI_ERROR;
        }
        virtual pami_result_t setManytomanyDispatch(pami_dispatch_manytomany_function fn,
                                                    void *cookie)
        {
          //PAMI_abort();
          return PAMI_ERROR;
        }
        virtual pami_result_t setSendDispatch(pami_dispatch_p2p_function fn,
                                              void *cookie)
        {
          //PAMI_abort();
          return PAMI_ERROR;
        }
        virtual pami_result_t setSendPWQDispatch(pami_dispatch_p2p_function fn,
                                                 void *cookie)
        {
          //PAMI_abort();
          return PAMI_ERROR;
        }
        virtual pami_result_t multicast(pami_multicast_t *mcast, void *devinfo = NULL) = 0;
        virtual pami_result_t multisync(pami_multisync_t *msync, void *devinfo = NULL) = 0;
        virtual pami_result_t multicombine(pami_multicombine_t *mcombine, void *devinfo = NULL) = 0;
        virtual pami_result_t manytomany(pami_manytomany_t *m2minfo, void *devinfo = NULL) = 0;
        virtual pami_result_t send (pami_send_t * parameters)
        {
          PAMI_abort();
          return PAMI_ERROR;
        }
        virtual pami_result_t sendPWQ(pami_context_t       context,
                                           pami_endpoint_t      dest,
                                           size_t               length,
                                           PAMI::PipeWorkQueue *pwq,
                                           pami_send_event_t   *events)
        {
          PAMI_abort();
          return PAMI_ERROR;
        }
        ///
        /// \brief NI hook to override metadata for collective
        ///
        virtual void metadata(pami_metadata_t *m, pami_xfer_type_t t) {/* no override */};

    };
  };
};

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
