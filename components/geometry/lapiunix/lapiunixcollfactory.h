/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/geometry/lapiunix/lapiunixcollfactory.h
 * \brief ???
 */

#ifndef   __xmi_lapicollfactory__h__
#define   __xmi_lapicollfactory__h__


#define XMI_COLLFACTORY_CLASS XMI::CollFactory::LAPI<Device::LAPIDevice<SysDep>, SysDep>

#include "sys/xmi.h"
#include "components/geometry/CollFactory.h"
#include "components/geometry/lapiunix/lapiunixcollinfo.h"
#include "util/common.h"
#include "algorithms/ccmi.h"

namespace XMI
{
  namespace CollFactory
  {

    template <class T_Device, class T_Sysdep>
    class LAPI : public CollFactory<XMI::CollFactory::LAPI<Device::LAPIDevice<SysDep>, SysDep> >
    {
    public:
      inline LAPI(T_Sysdep *sd):
        CollFactory<XMI::CollFactory::LAPI<T_Device, T_Sysdep> >(),
        _sd(sd)
        {
        }

      class reqObj
      {
      public:
        LAPI                     *factory;
        xmi_event_function       user_done_fn;
        void                    *user_cookie;
        // \todo:  we need to figure out exact sizes with ccmi....this is a bit of guesswork
        //         to know if we got enough storage.
        XMI_CollectiveRequest_t  req[4];
      };

      static void client_done(void *context, void *rdata, xmi_result_t res)
        {
          reqObj * robj = (reqObj*)rdata;
          LAPI    * lapi  = robj->factory;
          if(robj->user_done_fn)
            robj->user_done_fn(context, robj->user_cookie, res);
          lapi->_reqAllocator.returnObject(robj);
        }




      inline RegQueue * getRegQ(xmi_xfer_type_t       collective)
        {
          RegQueue *rq = NULL;
          switch (collective)
              {
                  case XMI_XFER_BROADCAST:
                    rq = &_broadcasts;
                    break;
                  case XMI_XFER_ALLREDUCE:
                    rq = &_allreduces;
                    break;
                  case XMI_XFER_REDUCE:
                    return NULL;
                    break;
                  case XMI_XFER_ALLGATHER:
                    rq = &_allgathers;
                    break;
                  case XMI_XFER_ALLGATHERV:
                    rq = &_allgathervs;
                    break;
                  case XMI_XFER_ALLGATHERV_INT:
                    return NULL;
                    break;
                  case XMI_XFER_SCATTER:
                    rq = &_scatters;
                    break;
                  case XMI_XFER_SCATTERV:
                    rq = &_scattervs;
                    break;
                  case XMI_XFER_SCATTERV_INT:
                    break;
                  case XMI_XFER_BARRIER:
                    rq = &_barriers;
                    break;
                  case XMI_XFER_ALLTOALL:
                    return NULL;
                    break;
                  case XMI_XFER_ALLTOALLV:
                    rq = &_alltoallvs;
                    break;
                  case XMI_XFER_ALLTOALLV_INT:
                    return NULL;
                    break;
                  case XMI_XFER_SCAN:
                    return NULL;
                    break;
                  case XMI_XFER_AMBROADCAST:
                    return NULL;
                    break;
                  case XMI_XFER_AMSCATTER:
                    return NULL;
                    break;
                  case XMI_XFER_AMGATHER:
                    return NULL;
                    break;
                  case XMI_XFER_AMREDUCE:
                    return NULL;
                    break;
                  default:
                    return NULL;
              }
          return rq;
        }

      inline xmi_result_t  algorithm_impl(xmi_xfer_type_t       collective,
                                          xmi_algorithm_t      *alglist,
                                          int                  *num)
        {
          RegQueue *rq = getRegQ(collective);
          if(rq==NULL)
            return XMI_UNIMPL;
          int i = rq->size();
          *num = MIN(*num, i);
          for(i=0; i<*num; i++)
            alglist[i] = (size_t)i;
          return XMI_SUCCESS;
        }

      inline size_t        num_algorithm_impl   (xmi_xfer_type_t           collective)
        {
          RegQueue *rq = getRegQ(collective);
          if(rq==NULL)
            return XMI_UNIMPL;
          return rq->size();
        }




      inline xmi_result_t  setGeometry(XMI_GEOMETRY_CLASS *g,
                                       XMI_NBCollManager  *mgr,
                                       T_Device           *dev,
                                       XMI::CollInfo::CCMIBinomBarrierInfo<T_Device, T_Sysdep> *default_bar)
        {
          _geometry = g;
          _dev      = dev;
	  // Setup PGAS style collectives
          _barrier    = mgr->allocate (g, TSPColl::BarrierTag);
          _allgather  = mgr->allocate (g, TSPColl::AllgatherTag);
          _allgatherv = mgr->allocate (g, TSPColl::AllgathervTag);
          _bcast      = mgr->allocate (g, TSPColl::BcastTag);
          _bcast2     = mgr->allocate (g, TSPColl::BcastTag2);
          _sar        = mgr->allocate (g, TSPColl::ShortAllreduceTag);
          _lar        = mgr->allocate (g, TSPColl::LongAllreduceTag);
          _sct        = mgr->allocate (g, TSPColl::ScatterTag);
          _sctv       = mgr->allocate (g, TSPColl::ScattervTag);

	  // Setup CCMI style collectives
          _ccmi_bar   = default_bar;
	  CCMI::Executor::Executor *exe = NULL;
          exe = default_bar->_barrier_registration.generate(&_barrier_executors[0],g);
          g->setKey(XMI::Geometry::XMI_GKEY_BARRIEREXECUTOR, (void*)exe);
          exe = default_bar->_barrier_registration.generate(&_barrier_executors[1],g);
          g->setKey(XMI::Geometry::XMI_GKEY_LOCALBARRIEREXECUTOR, (void*)exe);
	  return XMI_SUCCESS;
        }

      inline xmi_result_t  add_collective(xmi_xfer_type_t          collective,
					  XMI::CollInfo::CollInfo<T_Device>* ci)
        {
          RegQueue *rq = getRegQ(collective);
          if(rq==NULL)
            return XMI_UNIMPL;
          rq->push_back(ci);
          return XMI_SUCCESS;
        }

      inline xmi_result_t  collective_impl      (xmi_xfer_t           *collective)
        {
          switch (collective->xfer_type)
              {
                  case XMI_XFER_BROADCAST:
                    return ibroadcast_impl(&collective->xfer_broadcast);
                    break;
                  case XMI_XFER_ALLREDUCE:
                    return iallreduce_impl(&collective->xfer_allreduce);
                    break;
                  case XMI_XFER_REDUCE:
                    return ireduce_impl(&collective->xfer_reduce);
                    break;
                  case XMI_XFER_ALLGATHER:
                    return iallgather_impl(&collective->xfer_allgather);
                    break;
                  case XMI_XFER_ALLGATHERV:
                    return iallgatherv_impl(&collective->xfer_allgatherv);
                    break;
                  case XMI_XFER_ALLGATHERV_INT:
                    return iallgatherv_int_impl(&collective->xfer_allgatherv_int);
                    break;
                  case XMI_XFER_SCATTER:
                    return iscatter_impl(&collective->xfer_scatter);
                    break;
                  case XMI_XFER_SCATTERV:
                    return iscatterv_impl(&collective->xfer_scatterv);
                    break;
                  case XMI_XFER_SCATTERV_INT:
                    return iscatterv_int_impl(&collective->xfer_scatterv_int);
                    break;
                  case XMI_XFER_BARRIER:
                    return ibarrier_impl(&collective->xfer_barrier);
                    break;
                  case XMI_XFER_ALLTOALL:
                    return ialltoall_impl(&collective->xfer_alltoall);
                    break;
                  case XMI_XFER_ALLTOALLV:
                    return ialltoallv_impl(&collective->xfer_alltoallv);
                    break;
                  case XMI_XFER_ALLTOALLV_INT:
                    return ialltoallv_int_impl(&collective->xfer_alltoallv_int);
                    break;
                  case XMI_XFER_SCAN:
                    return iscan_impl(&collective->xfer_scan);
                    break;
                  case XMI_XFER_AMBROADCAST:
                    return ambroadcast_impl(&collective->xfer_ambroadcast);
                    break;
                  case XMI_XFER_AMSCATTER:
                    return amscatter_impl(&collective->xfer_amscatter);
                    break;
                  case XMI_XFER_AMGATHER:
                    return amgather_impl(&collective->xfer_amgather);
                    break;
                  case XMI_XFER_AMREDUCE:
                    return amreduce_impl(&collective->xfer_amreduce);
                    break;
                  default:
                    return XMI_UNIMPL;
              }
          return XMI_UNIMPL;
        }

      inline xmi_result_t  ibroadcast_impl      (xmi_broadcast_t      *broadcast)
        {
          XMI::CollInfo::PGBroadcastInfo<T_Device> *info =
            (XMI::CollInfo::PGBroadcastInfo<T_Device> *)_broadcasts[broadcast->algorithm];
	  switch(info->_colltype)
              {
                  case XMI::CollInfo::CI_BROADCAST0:
                  {
                    if (!_bcast->isdone()) _dev->advance();

                    ((TSPColl::BinomBcast<LAPIMcastModel> *)_bcast)->reset (_geometry->virtrankof(broadcast->root),
                                                                           broadcast->buf,
                                                                           broadcast->buf,
                                                                           broadcast->typecount);
                    _bcast->setComplete(broadcast->cb_done, broadcast->cookie);
                    _bcast->kick(&info->_model);
                  }
                  break;
                  case XMI::CollInfo::CI_BROADCAST1:
                  {
                    XMI_Callback_t cb_done;
                    XMI::CollInfo::CCMIBinomBroadcastInfo<T_Device, T_Sysdep> *cinfo=
                      (XMI::CollInfo::CCMIBinomBroadcastInfo<T_Device, T_Sysdep>*)info;
                    reqObj * robj = (reqObj *)_reqAllocator.allocateObject();
                    XMI_assertf(robj,"bcast alg 1:  memory allocation failure\n");

                    robj->factory      = this;
                    robj->user_done_fn = broadcast->cb_done;
                    robj->user_cookie  = broadcast->cookie;

                    cb_done.function   = client_done;
                    cb_done.clientdata = robj;
                    cinfo->_broadcast_registration.generate(&robj->req[0],
                                                            sizeof(XMI_CollectiveRequest_t),
                                                            cb_done,
                                                            XMI_MATCH_CONSISTENCY,
                                                            _geometry,
                                                            broadcast->root,
                                                            broadcast->buf,
                                                            broadcast->typecount);
                  }
                  break;
                  case XMI::CollInfo::CI_BROADCAST2:
                  {
                    XMI_Callback_t cb_done;
                    XMI::CollInfo::CCMIRingBroadcastInfo<T_Device, T_Sysdep> *cinfo=
                      (XMI::CollInfo::CCMIRingBroadcastInfo<T_Device, T_Sysdep>*)info;

                    reqObj * robj = (reqObj *)_reqAllocator.allocateObject();
                    XMI_assertf(robj,"bcast alg 2:  memory allocation failure\n");

                    robj->factory      = this;
                    robj->user_done_fn = broadcast->cb_done;
                    robj->user_cookie  = broadcast->cookie;

                    cb_done.function   = client_done;
                    cb_done.clientdata = robj;

                    cinfo->_broadcast_registration.generate(&robj->req[0],
                                                            sizeof(XMI_CollectiveRequest_t),
                                                            cb_done,
                                                            XMI_MATCH_CONSISTENCY,
                                                            _geometry,
                                                            broadcast->root,
                                                            broadcast->buf,
                                                            broadcast->typecount);
                  }
                  break;
                  default:
                    assert(0);
                    return XMI_UNIMPL;
              }
          return XMI_SUCCESS;
        }

      inline xmi_result_t  iallreduce_impl      (xmi_allreduce_t      *allreduce)
        {
          XMI::CollInfo::PGAllreduceInfo<T_Device> *info =
            (XMI::CollInfo::PGAllreduceInfo<T_Device> *)_allreduces[allreduce->algorithm];

	  switch(info->_colltype)
              {
                  case XMI::CollInfo::CI_ALLREDUCE0:
                  {
                    unsigned datawidth;
                    coremath cb_allreduce;
                    CCMI::Adaptor::Allreduce::getReduceFunction(allreduce->dt,
                                                                allreduce->op,
                                                                allreduce->stypecount,
                                                                datawidth,
                                                                cb_allreduce);
                    if (datawidth * allreduce->stypecount < (unsigned)TSPColl::Allreduce::Short<LAPIMcastModel>::MAXBUF)
                        {
                          if (!_sar->isdone()) _dev->advance();
                          ((TSPColl::Allreduce::Short<LAPIMcastModel> *)_sar)->reset (allreduce->sndbuf,
                                                                                     allreduce->rcvbuf,
                                                                                     allreduce->op,
                                                                                     allreduce->dt,
                                                                                     allreduce->stypecount);
                          _sar->setComplete(allreduce->cb_done, allreduce->cookie);
                          _sar->kick(&info->_model);
                          return XMI_SUCCESS;
                        }
                    else
                        {
                          if (!_lar->isdone()) _dev->advance();
                          ((TSPColl::Allreduce::Long<LAPIMcastModel> *)_lar)->reset (allreduce->sndbuf,
                                                                                    allreduce->rcvbuf,
                                                                                    allreduce->op,
                                                                                    allreduce->dt,
                                                                                    allreduce->stypecount);
                          _lar->setComplete(allreduce->cb_done, allreduce->cookie);
                          _lar->kick(&info->_model);
                          return XMI_SUCCESS;
                        }
                    break;
                  }
                  case XMI::CollInfo::CI_ALLREDUCE1:
                  {
                    CCMI::Adaptor::Allreduce::BaseComposite * arcomposite =
                      (CCMI::Adaptor::Allreduce::BaseComposite *)_geometry->getAllreduceComposite();
                    XMI::CollInfo::CCMIRingAllreduceInfo<T_Device, T_Sysdep> *cinfo=
                      (XMI::CollInfo::CCMIRingAllreduceInfo<T_Device, T_Sysdep>*)info;

                    XMI_Callback_t cb_done;
                    reqObj * robj = (reqObj *)_reqAllocator.allocateObject();
                    XMI_assertf(robj,"allreduce alg 1:  memory allocation failure\n");

                    robj->factory      = this;
                    robj->user_done_fn = allreduce->cb_done;
                    robj->user_cookie  = allreduce->cookie;

                    cb_done.function   = client_done;
                    cb_done.clientdata = robj;

                    CCMI::Adaptor::Allreduce::Ring::Factory *factory =
                      (CCMI::Adaptor::Allreduce::Ring::Factory *) &cinfo->_allreduce_registration;
                    if(arcomposite != NULL  &&  arcomposite->getFactory() == factory)
                        {
                          xmi_result_t status =  (xmi_result_t)arcomposite->restart(&robj->req[0],
                                                                                    cb_done,
                                                                                    XMI_MATCH_CONSISTENCY,
                                                                                    allreduce->sndbuf,
                                                                                    allreduce->rcvbuf,
                                                                                    allreduce->stypecount,
                                                                                    allreduce->dt,
                                                                                    allreduce->op);
                          if(status == XMI_SUCCESS)
                              {
                                _geometry->setAllreduceComposite(arcomposite);
                                return status;
                              }
                        }

                    if(arcomposite != NULL) // Different factory?  Cleanup old executor.
                        {
                          _geometry->setAllreduceComposite(NULL);
                          arcomposite->~BaseComposite();
                        }
                    void *ptr =factory->generate(&robj->req[0],
                                                 cb_done,
                                                 XMI_MATCH_CONSISTENCY,
                                                 _geometry,
                                                 allreduce->sndbuf,
                                                 allreduce->rcvbuf,
                                                 allreduce->stypecount,
                                                 allreduce->dt,
                                                 allreduce->op);
                    if(ptr == NULL)
                        {
                          return XMI_UNIMPL;
                        }
                    return XMI_SUCCESS;
                    break;
                  }
                  case XMI::CollInfo::CI_ALLREDUCE2:
                  {
                    CCMI::Adaptor::Allreduce::BaseComposite * arcomposite =
                      (CCMI::Adaptor::Allreduce::BaseComposite *)_geometry->getAllreduceComposite();
                    XMI::CollInfo::CCMIBinomialAllreduceInfo<T_Device, T_Sysdep> *cinfo=
                      (XMI::CollInfo::CCMIBinomialAllreduceInfo<T_Device, T_Sysdep>*)info;

                    XMI_Callback_t cb_done;
                    reqObj * robj = (reqObj *)_reqAllocator.allocateObject();
                    XMI_assertf(robj,"allreduce alg 2:  memory allocation failure\n");

                    robj->factory      = this;
                    robj->user_done_fn = allreduce->cb_done;
                    robj->user_cookie  = allreduce->cookie;

                    cb_done.function   = client_done;
                    cb_done.clientdata = robj;

                    CCMI::Adaptor::Allreduce::Binomial::Factory *factory =
                      (CCMI::Adaptor::Allreduce::Binomial::Factory *) &cinfo->_allreduce_registration;
                    if(arcomposite != NULL  &&  arcomposite->getFactory() == factory)
                        {
                          xmi_result_t status =  (xmi_result_t)arcomposite->restart(&robj->req[0],
                                                                                    cb_done,
                                                                                    XMI_MATCH_CONSISTENCY,
                                                                                    allreduce->sndbuf,
                                                                                    allreduce->rcvbuf,
                                                                                    allreduce->stypecount,
                                                                                    allreduce->dt,
                                                                                    allreduce->op);
                          if(status == XMI_SUCCESS)
                              {
                                _geometry->setAllreduceComposite(arcomposite);
                                return status;
                              }
                        }

                    if(arcomposite != NULL) // Different factory?  Cleanup old executor.
                        {
                          _geometry->setAllreduceComposite(NULL);
                          arcomposite->~BaseComposite();
                        }
                    void *ptr =factory->generate(&robj->req[0],
                                                 cb_done,
                                                 XMI_MATCH_CONSISTENCY,
                                                 _geometry,
                                                 allreduce->sndbuf,
                                                 allreduce->rcvbuf,
                                                 allreduce->stypecount,
                                                 allreduce->dt,
                                                 allreduce->op);
                    if(ptr == NULL)
                        {
                          return XMI_UNIMPL;
                        }
                    return XMI_SUCCESS;
                    break;
                  }
                  default:
                    assert(0);
                    return XMI_UNIMPL;
                    break;
              }
          return XMI_SUCCESS;
        }

      inline xmi_result_t  ireduce_impl         (xmi_reduce_t         *reduce)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  iallgather_impl      (xmi_allgather_t      *allgather)
        {
          XMI::CollInfo::PGAllgatherInfo<T_Device> *info =
            (XMI::CollInfo::PGAllgatherInfo<T_Device> *)_allgathers[allgather->algorithm];
          if (!_allgather->isdone()) _dev->advance();
          ((TSPColl::Allgather<LAPIMcastModel> *)_allgather)->reset (allgather->sndbuf,
                                                                    allgather->rcvbuf,
                                                                    allgather->stypecount);
          _allgather->setComplete(allgather->cb_done, allgather->cookie);
          _allgather->kick(&info->_model);
          return XMI_SUCCESS;
        }

      inline xmi_result_t  iallgatherv_impl     (xmi_allgatherv_t     *allgatherv)
        {

          XMI::CollInfo::PGAllgathervInfo<T_Device> *info =
            (XMI::CollInfo::PGAllgathervInfo<T_Device> *)_allgathervs[allgatherv->algorithm];
          if (!_allgatherv->isdone()) _dev->advance();
          ((TSPColl::Allgatherv<LAPIMcastModel> *)_allgatherv)->reset (allgatherv->sndbuf,
                                                                      allgatherv->rcvbuf,
                                                                      allgatherv->rtypecounts);
          _allgatherv->setComplete(allgatherv->cb_done, allgatherv->cookie);
          _allgatherv->kick(&info->_model);
          return XMI_SUCCESS;
        }

      inline xmi_result_t  iallgatherv_int_impl (xmi_allgatherv_int_t *allgatherv_int)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  iscatter_impl        (xmi_scatter_t        *scatter)
        {
          XMI::CollInfo::PGScatterInfo<T_Device> *info =
            (XMI::CollInfo::PGScatterInfo<T_Device> *)_scatters[scatter->algorithm];
          if (!_sct->isdone()) _dev->advance();
          ((TSPColl::Scatter<LAPIMcastModel> *)_sct)->reset (_geometry->virtrankof(scatter->root),
                                                            scatter->sndbuf,
                                                            scatter->rcvbuf,
                                                            scatter->stypecount);
          _sct->setComplete(scatter->cb_done, scatter->cookie);

          while(!_barrier->isdone()) _dev->advance();
          ((TSPColl::Barrier<LAPIMcastModel> *)_barrier)->reset();
          _barrier->setComplete(NULL, NULL);
          _barrier->kick(&info->_bmodel);
          while(!_barrier->isdone()) _dev->advance();

          _sct->kick(&info->_smodel);
          return XMI_SUCCESS;
        }

      inline xmi_result_t  iscatterv_impl       (xmi_scatterv_t       *scatterv)
        {
          XMI::CollInfo::PGScattervInfo<T_Device> *info =
            (XMI::CollInfo::PGScattervInfo<T_Device> *)_scattervs[scatterv->algorithm];
          if (!_sctv->isdone()) _dev->advance();
          ((TSPColl::Scatterv<LAPIMcastModel> *)_sctv)->reset (_geometry->virtrankof(scatterv->root),
                                                              scatterv->sndbuf,
                                                              scatterv->rcvbuf,
                                                              scatterv->stypecounts);
          _sctv->setComplete(scatterv->cb_done, scatterv->cookie);

          while(!_barrier->isdone()) _dev->advance();
          ((TSPColl::Barrier<LAPIMcastModel> *)_barrier)->reset();
          _barrier->setComplete(NULL, NULL);
          _barrier->kick(&info->_bmodel);
          while(!_barrier->isdone()) _dev->advance();

          _sctv->kick(&info->_smodel);
          return XMI_SUCCESS;
        }

      inline xmi_result_t  iscatterv_int_impl   (xmi_scatterv_int_t   *scatterv)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  ibarrier_impl        (xmi_barrier_t        *barrier)
        {
	  XMI::CollInfo::CollInfo<T_Device> *info =
	    (XMI::CollInfo::CollInfo<T_Device> *)_barriers[barrier->algorithm];

	  switch(info->_colltype)
              {
                  case XMI::CollInfo::CI_BARRIER0:
                  {
                    XMI::CollInfo::PGBarrierInfo<T_Device> *binfo =
                      (XMI::CollInfo::PGBarrierInfo<T_Device> *)info;
                    while(!_barrier->isdone()) _dev->advance();
                    ((TSPColl::Barrier<LAPIMcastModel> *)_barrier)->reset();
                    _barrier->setComplete(barrier->cb_done, barrier->cookie);
                    _barrier->kick(&binfo->_model);
                  }
                  break;
                  case XMI::CollInfo::CI_BARRIER1:
                  {
                    xmi_callback_t cb_done_ccmi;
                    cb_done_ccmi.function   = barrier->cb_done;
                    cb_done_ccmi.clientdata = barrier->cookie;
                    CCMI::Executor::Executor *_c_bar = (CCMI::Executor::Executor *)
                      _geometry->getKey(XMI::Geometry::XMI_GKEY_BARRIEREXECUTOR);
                    _c_bar->setDoneCallback(barrier->cb_done,barrier->cookie);
                    _c_bar->setConsistency (XMI_MATCH_CONSISTENCY);
                    _c_bar->start();
                  }
                  break;
                  default:
                    assert(0);
                    return XMI_UNIMPL;
              }

          return XMI_SUCCESS;
        }

      inline xmi_result_t  ialltoall_impl       (xmi_alltoall_t       *alltoall)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  ialltoallv_impl      (xmi_alltoallv_t      *alltoallv)
        {
          XMI::CollInfo::CollInfo<T_Device> *info =
	    (XMI::CollInfo::CollInfo<T_Device> *)_alltoallvs[alltoallv->algorithm];
          switch(info->_colltype)
              {
                  case XMI::CollInfo::CI_ALLTOALLV0:
                  {
                    XMI::CollInfo::CCMIAlltoallvInfo<T_Device, T_Sysdep> *cinfo=
                       (XMI::CollInfo::CCMIAlltoallvInfo<T_Device, T_Sysdep>*)info;

                    XMI_CollectiveRequest_t *req = (XMI_CollectiveRequest_t *)malloc(sizeof(XMI_CollectiveRequest_t));
                    XMI_assertf(req,"malloc failure\n");
                    XMI_Callback_t cb_done;
		    cb_done.function   = alltoallv->cb_done;
                    cb_done.clientdata = alltoallv->cookie;

                    reqObj * robj      = (reqObj *)_reqAllocator.allocateObject();
                    XMI_assertf(robj,"allreduce alg 2:  memory allocation failure\n");

                    robj->factory      = this;
                    robj->user_done_fn = alltoallv->cb_done;
                    robj->user_cookie  = alltoallv->cookie;

                    cb_done.function   = client_done;
                    cb_done.clientdata = robj;

		    cinfo->_alltoallv_registration.generate(&robj->req[0],
                                                            cb_done,
                                                            XMI_MATCH_CONSISTENCY,
                                                            _geometry,
                                                            alltoallv->sndbuf,
                                                            alltoallv->stypecounts,
                                                            alltoallv->sdispls,
                                                            alltoallv->rcvbuf,
                                                            alltoallv->rtypecounts,
                                                            alltoallv->rdispls,
                                                            NULL,
                                                            NULL);
                  }
                  break;
                  default:
                    assert(0);
                    return XMI_UNIMPL;
                    break;
              }
          return XMI_UNIMPL;
        }

      inline xmi_result_t  ialltoallv_int_impl  (xmi_alltoallv_int_t  *alltoallv_int)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  iscan_impl           (xmi_scan_t           *scan)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  ambroadcast_impl     (xmi_ambroadcast_t    *ambroadcast)
        {
          XMI::CollInfo::CCMIAmbroadcastInfo<T_Device, T_Sysdep> *info =
            (XMI::CollInfo::CCMIAmbroadcastInfo<T_Device, T_Sysdep> *)_ambroadcasts[ambroadcast->algorithm];

          if(ambroadcast->stypecount == 0)
            ambroadcast->cb_done(NULL, ambroadcast->cookie, XMI_SUCCESS);
          else
              {
#if 0
//                CCMI_assert (((CCMI::Adaptor::Geometry *) geometry)->getBarrierExecutor() != NULL);
                xmi_callback_t cb_done_ccmi;
                cb_done_ccmi.function   = ambroadcast->cb_done;
                cb_done_ccmi.clientdata = ambroadcast->cookie;
                XMI_CollectiveRequest_t *req = (XMI_CollectiveRequest_t *)malloc(sizeof(XMI_CollectiveRequest_t));
                XMI_assertf(req,"malloc failure\n");
                factory->generate(req,
                                  sizeof(XMI_CollectiveRequest_t),
                                  cb_done_ccmi,
                                  XMI_MATCH_CONSISTENCY,
                                  _geometry,
                                  __global.mapping->task(), //root
                                  ambroadcast->sndbuf,
                                  ambroadcast->stypecount);
#endif
              }
          return XMI_SUCCESS;
        }

      inline xmi_result_t  amscatter_impl       (xmi_amscatter_t      *amscatter)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  amgather_impl        (xmi_amgather_t       *amgather)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  amreduce_impl        (xmi_amreduce_t       *amreduce)
        {
          return XMI_UNIMPL;
        }
      T_Device                        *_dev;
      XMI_GEOMETRY_CLASS              *_geometry;
      T_Sysdep                        *_sd;
      RegQueue                         _broadcasts;
      RegQueue                         _ambroadcasts;
      RegQueue                         _allgathers;
      RegQueue                         _allgathervs;
      RegQueue                         _scatters;
      RegQueue                         _scattervs;
      RegQueue                         _allreduces;
      RegQueue                         _barriers;
      RegQueue                         _alltoallvs;
      TSPColl::NBColl<LAPIMcastModel>  *_barrier;
      TSPColl::NBColl<LAPIMcastModel>  *_allgather;
      TSPColl::NBColl<LAPIMcastModel>  *_allgatherv;
      TSPColl::NBColl<LAPIMcastModel>  *_bcast, *_bcast2;
      TSPColl::NBColl<LAPIMcastModel>  *_sar,   *_lar;
      TSPColl::NBColl<LAPIMcastModel>  *_sct,   *_sctv;
      CCMI_Executor_t                  _barrier_executors[2];
      XMI::CollInfo::CCMIBinomBarrierInfo<T_Device, T_Sysdep>  *_ccmi_bar;
      XMI::MemoryAllocator<sizeof(reqObj), 16> _reqAllocator;

    }; // class CollFactory
  };  // namespace CollFactory
}; // namespace XMI


#endif
