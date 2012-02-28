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

#include "util/trace.h"

#undef DO_TRACE_ENTEREXIT
#undef DO_TRACE_DEBUG

#ifdef CCMI_TRACE_ALL
#define DO_TRACE_ENTEREXIT 1
#define DO_TRACE_DEBUG     1
#else
#define DO_TRACE_ENTEREXIT 0
#define DO_TRACE_DEBUG     0
#endif

namespace CCMI{namespace Interfaces{

class NativeInterface
{
public:
  NativeInterface(size_t          context_id,
                  pami_endpoint_t endpoint):
    _endpoint(endpoint),
    _context_id(context_id),
    _status(PAMI_SUCCESS)
    {
        TRACE_FN_ENTER();
        TRACE_FORMAT( "<%p> context %zu, endpoint %u",this,_context_id, _endpoint);
        TRACE_FN_EXIT();
    }
  ///
  /// \brief Virtual destructors make compilers happy.
  ///
  virtual inline ~NativeInterface() {};
  inline pami_endpoint_t endpoint()    { return _endpoint;     }
  inline size_t          contextid()   { return _context_id;   }
  inline pami_result_t   status()      { return _status;       }

  /// \brief this call is called when an active message native interface is initialized and
  /// is not supported on all sided native interfaces
  virtual pami_result_t setMulticastDispatch(pami_dispatch_multicast_function fn,
                                             void *cookie)
    {
      (void)fn;(void)cookie;
      //PAMI_abort();
      return PAMI_ERROR;
    }
  virtual pami_result_t setManytomanyDispatch(pami_dispatch_manytomany_function fn,
                                              void *cookie)
    {
      (void)fn;(void)cookie;
      //PAMI_abort();
      return PAMI_ERROR;
    }
  virtual pami_result_t setSendDispatch(pami_dispatch_p2p_function fn,
                                        void *cookie)
    {
      (void)fn;(void)cookie;
      //PAMI_abort();
      return PAMI_ERROR;
    }
  virtual pami_result_t setSendPWQDispatch(pami_dispatch_p2p_function fn,
                                           void *cookie)
    {
      (void)fn;(void)cookie;
      //PAMI_abort();
      return PAMI_ERROR;
    }
  virtual pami_result_t destroy () = 0;
  virtual pami_result_t multicast(pami_multicast_t *mcast, void *devinfo = NULL) = 0;
  virtual pami_result_t multisync(pami_multisync_t *msync, void *devinfo = NULL) = 0;
  virtual pami_result_t multicombine(pami_multicombine_t *mcombine, void *devinfo = NULL) = 0;
  virtual pami_result_t manytomany(pami_manytomany_t *m2minfo, void *devinfo = NULL) = 0;
  virtual pami_result_t send (pami_send_t * parameters)
    {
      (void)parameters;
      PAMI_abort();
      return PAMI_ERROR;
    }
  virtual pami_result_t sendPWQ(pami_context_t       context,
                                pami_endpoint_t      dest,
                                size_t               header_length,
                                void                *header,
                                size_t               length,
                                PAMI::PipeWorkQueue *pwq,
                                pami_send_event_t   *events)
    {
      (void)context;(void)dest;(void)header_length;(void)header;
      (void)length;(void)pwq;(void)events;
      PAMI_abort();
      return PAMI_ERROR;
    }
  ///
  /// \brief NI hook to override metadata for collective
  ///
  virtual void metadata(pami_metadata_t *m, pami_xfer_type_t t)
  {
    (void)m;(void)t;
    /* no override */
  };

  ///
  /// \brief post a work function to be executed on the
  /// communication thread. The color identifier can choose the
  /// commuication thread relative to the context parameter.
  ///
  virtual void postWork (pami_context_t         context,
                         int                    color,
                         pami_work_t          * work,
                         pami_work_function     fn,
                         void                 * clientdata)
    {
      (void)context;(void)color;(void)work;(void)fn;
      (void)clientdata;
      CCMI_abort();
    }
protected:
  pami_endpoint_t  _endpoint;
  size_t           _context_id;
  pami_result_t    _status;
};

};
};

#undef DO_TRACE_ENTEREXIT
#undef DO_TRACE_DEBUG

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
