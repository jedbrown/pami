/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/geometry/PGASWrapper.h
 * \brief ???
 */

#ifndef __algorithms_geometry_PGASWrapper_h__
#define __algorithms_geometry_PGASWrapper_h__



#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "algorithms/protocols/tspcoll/Collectives.h"
#include "algorithms/composite/Composite.h"
#include "algorithms/geometry/Metadata.h"

namespace PAMI
{
  namespace CollRegistration
  {
    // The following classes are used to implement a CCMI wrapper
    // around the PGAS RT classes

    // --------------  PGAS Wrapper base class -------------
    template <class T_Geometry, class T_TSPColl, class T_P2P_NI, class T_Device, class T_TSPCollBarrier = xlpgas::Collective<T_P2P_NI> >
    class PGExec:public CCMI::Executor::Composite
    {
    public:
      virtual void start() = 0;
      virtual void setcmd(T_TSPColl         *collexch,
                          pami_xfer_t       *cmd,
                          T_Device          *dev,
                          T_P2P_NI          *model,
                          T_TSPCollBarrier  *collbarrier,
                          pami_geometry_t    geometry,
                          T_P2P_NI          *barmodel)

        {
          _cmd         = cmd;
          _collexch    = collexch;
          _dev         = dev;
          _model       = model;
          _collbarrier = collbarrier;
          _barmodel    = barmodel;
          _geometry    = (T_Geometry*)geometry;
          _collexch->setNI(model);
          if(_collbarrier) _collbarrier->setNI(barmodel);
        }
    public:
      pami_xfer_t                      *_cmd;
      T_TSPColl                       *_collexch;
      T_Device                        *_dev;
      T_P2P_NI                        *_model;
      T_TSPCollBarrier                *_collbarrier;
      T_P2P_NI                        *_barmodel;
      T_Geometry                      *_geometry;
    };

    // --------------  PGAS Factory base class -------------
    template <class       T_TSPColl,
              class       T_P2P_NI,
              class       T_Device,
              class       T_Exec,
              class       T_TSPCollBarrier = xlpgas::Collective<T_P2P_NI> >
    class PGFactory : public CCMI::Adaptor::CollectiveProtocolFactory
    {
    public:
      PGFactory<T_TSPColl,
                T_P2P_NI,
                T_Device,
                T_Exec,
                T_TSPCollBarrier>(T_Device         *dev,
                                  T_P2P_NI          *model,
                                  T_TSPColl        *coll,
				  const char *string,
                                  T_TSPCollBarrier *collbarrier = NULL,
                                  T_P2P_NI          *barmodel    = NULL  ):
        _coll(coll),
        _dev(dev),
        _model(model),
	_string(string),
        _barmodel(barmodel),
        _collbarrier(collbarrier)
        {
        }
      virtual CCMI::Executor::Composite * generate(pami_geometry_t  geometry,
                                                   void           *cmd)
        {
          _exec.setcmd(_coll,
                       (pami_xfer_t*)cmd,
                       _dev,
                       _model,
                       _collbarrier,
                       geometry,
                       _barmodel);
	  
	  CCMI::Executor::Composite *composite = (CCMI::Executor::Composite*)&_exec;
	  pami_xfer_t *xfer = (pami_xfer_t *)cmd;
	  composite->setDoneCallback(xfer->cb_done, xfer->cookie);
          return composite; 
        }
      virtual void metadata(pami_metadata_t *mdata)
        {
          new(mdata) PAMI::Geometry::Metadata(_string);
          if(strstr(_string,"Short")) /// \todo arbitrary hack for now
          {  
            mdata->check_correct.values.rangeminmax = 1;
            mdata->range_hi = 512; 
          }
        }
      T_Exec            _exec;
      T_TSPColl        *_coll;
      T_Device         *_dev;
      T_P2P_NI          *_model;
      const char        *_string;
      T_P2P_NI          *_barmodel;
      T_TSPCollBarrier *_collbarrier;
    };

    // --------------  PGAS Barrier wrapper classes -------------
    template <class T_Geometry, class T_TSPColl, class T_P2P_NI, class T_Device>
    class PGBarrierExec:public PGExec<T_Geometry,T_TSPColl,T_P2P_NI,T_Device>
    {
    public:
      virtual void start()
        {
          while(!this->_collexch->isdone()) this->_dev->advance();
          this->_collexch->reset();
          this->_collexch->setContext(this->_context);
          this->_collexch->setComplete(this->_cmd->cb_done, this->_cmd->cookie);
          this->_collexch->kick();
        }
    };
    // --------------  PGAS Allgather wrapper classes -------------
    template <class T_Geometry, class T_TSPColl, class T_P2P_NI, class T_Device>
    class PGAllgatherExec:public PGExec<T_Geometry,T_TSPColl,T_P2P_NI,T_Device>
    {
    public:
      virtual void start()
        {
          if (!this->_collexch->isdone()) this->_dev->advance();
          PAMI::Type::TypeCode * allgathersType = (PAMI::Type::TypeCode *)this->_cmd->cmd.xfer_allgather.stype;
          PAMI::Type::TypeCode * allgatherrType = (PAMI::Type::TypeCode *)this->_cmd->cmd.xfer_allgather.rtype;
          this->_collexch->reset (this->_cmd->cmd.xfer_allgather.sndbuf,
                                  this->_cmd->cmd.xfer_allgather.rcvbuf,
                                  allgathersType,
                                  this->_cmd->cmd.xfer_allgather.stypecount,
                                  allgatherrType,
                                  this->_cmd->cmd.xfer_allgather.rtypecount);
          this->_collexch->setContext(this->_context);
          this->_collexch->setComplete(this->_cmd->cb_done, this->_cmd->cookie);
          this->_collexch->kick();
        }
    };

    // --------------  PGAS Allgatherv wrapper classes -------------
    template <class T_Geometry, class T_TSPColl, class T_P2P_NI, class T_Device>
    class PGAllgathervExec:public PGExec<T_Geometry,T_TSPColl,T_P2P_NI,T_Device>
    {
    public:
      virtual void start()
        {
          if (!this->_collexch->isdone()) this->_dev->advance();
          PAMI::Type::TypeCode * allgathervsType = (PAMI::Type::TypeCode *)this->_cmd->cmd.xfer_allgatherv.stype;
          PAMI::Type::TypeCode * allgathervrType = (PAMI::Type::TypeCode *)this->_cmd->cmd.xfer_allgatherv.rtype;
          this->_collexch->reset (this->_cmd->cmd.xfer_allgatherv.sndbuf,
                                  this->_cmd->cmd.xfer_allgatherv.rcvbuf,
								  allgathervsType,
                                  this->_cmd->cmd.xfer_allgatherv.stypecount,
								  allgathervrType,
								  this->_cmd->cmd.xfer_allgatherv.rtypecounts,
								  this->_cmd->cmd.xfer_allgatherv.rdispls);
          this->_collexch->setContext(this->_context);
          this->_collexch->setComplete(this->_cmd->cb_done, this->_cmd->cookie);
          this->_collexch->kick();
        }
    };

    // --------------  PGAS Scatter wrapper classes -------------
    template <class T_Geometry, class T_TSPColl, class T_P2P_NI, class T_Device, class T_TSPCollBarrier >
    class PGScatterExec:public PGExec<T_Geometry,T_TSPColl,T_P2P_NI,T_Device,T_TSPCollBarrier>
    {
    public:
      virtual void start()
        {
          if (!this->_collexch->isdone()) this->_dev->advance();
		  PAMI::Type::TypeCode * sndType = (PAMI::Type::TypeCode *)this->_cmd->cmd.xfer_scatter.stype;
		  PAMI::Type::TypeCode * rcvType = (PAMI::Type::TypeCode *)this->_cmd->cmd.xfer_scatter.rtype;
          this->_collexch->reset (this->_geometry->virtrankof(this->_cmd->cmd.xfer_scatter.root),
                                  this->_cmd->cmd.xfer_scatter.sndbuf,
                                  this->_cmd->cmd.xfer_scatter.rcvbuf,
                                  sndType,
                                  this->_cmd->cmd.xfer_scatter.stypecount,
                                  rcvType,
                                  this->_cmd->cmd.xfer_scatter.rtypecount);

          this->_collexch->setContext(this->_context);
          this->_collexch->setComplete(this->_cmd->cb_done, this->_cmd->cookie);
          while(!this->_collbarrier->isdone()) this->_dev->advance();
          this->_collbarrier->reset();
          this->_collbarrier->setContext(this->_context);
          this->_collbarrier->setComplete(NULL, NULL);
          this->_collbarrier->kick();
          while(!this->_collbarrier->isdone()) this->_dev->advance();
          this->_collexch->kick();
        }
    };

    // --------------  PGAS Gather wrapper classes -------------
    template <class T_Geometry, class T_TSPColl, class T_P2P_NI, class T_Device, class T_TSPCollBarrier >
    class PGGatherExec:public PGExec<T_Geometry,T_TSPColl,T_P2P_NI,T_Device,T_TSPCollBarrier>
    {
    public:
      virtual void start()
        {
          if (!this->_collexch->isdone()) this->_dev->advance();
		  PAMI::Type::TypeCode * sndType = (PAMI::Type::TypeCode *)this->_cmd->cmd.xfer_gather.stype;
		  PAMI::Type::TypeCode * rcvType = (PAMI::Type::TypeCode *)this->_cmd->cmd.xfer_gather.rtype;
          this->_collexch->reset (this->_geometry->virtrankof(this->_cmd->cmd.xfer_gather.root),
                                  this->_cmd->cmd.xfer_gather.sndbuf,
                                  this->_cmd->cmd.xfer_gather.rcvbuf,
                                  sndType,
                                  this->_cmd->cmd.xfer_gather.stypecount,
                                  rcvType,
                                  this->_cmd->cmd.xfer_gather.rtypecount);

          this->_collexch->setContext(this->_context);
          this->_collexch->setComplete(this->_cmd->cb_done, this->_cmd->cookie);
          while(!this->_collbarrier->isdone()) this->_dev->advance();
          this->_collbarrier->reset();
          this->_collbarrier->setContext(this->_context);
          this->_collbarrier->setComplete(NULL, NULL);
          this->_collbarrier->kick();
          while(!this->_collbarrier->isdone()) this->_dev->advance();
          this->_collexch->kick();
        }
    };

    // --------------  PGAS Alltoall wrapper classes -------------
    template <class T_Geometry, class T_TSPColl, class T_P2P_NI, class T_Device, class T_TSPCollBarrier >
    class PGAlltoallExec:public PGExec<T_Geometry,T_TSPColl,T_P2P_NI,T_Device,T_TSPCollBarrier>
    {
    public:
      virtual void start()
        {
          if (!this->_collexch->isdone()) this->_dev->advance();
		  PAMI::Type::TypeCode * sndType = (PAMI::Type::TypeCode *)this->_cmd->cmd.xfer_alltoall.stype;
		  PAMI::Type::TypeCode * rcvType = (PAMI::Type::TypeCode *)this->_cmd->cmd.xfer_alltoall.rtype;
          this->_collexch->reset (this->_cmd->cmd.xfer_alltoall.sndbuf,
                                  this->_cmd->cmd.xfer_alltoall.rcvbuf,
                                  sndType,
                                  this->_cmd->cmd.xfer_alltoall.stypecount,
                                  rcvType,
                                  this->_cmd->cmd.xfer_alltoall.rtypecount);
          this->_collexch->setContext(this->_context);
          this->_collexch->setComplete(this->_cmd->cb_done, this->_cmd->cookie);
          while(!this->_collbarrier->isdone()) this->_dev->advance();
          this->_collbarrier->reset();
          this->_collbarrier->setContext(this->_context);
          this->_collbarrier->setComplete(NULL, NULL);
          this->_collbarrier->kick();
          while(!this->_collbarrier->isdone()) this->_dev->advance();
          this->_collexch->kick();
        }
    };

    // --------------  PGAS Alltoallv wrapper classes -------------
    template <class T_Geometry, class T_TSPColl, class T_P2P_NI, class T_Device, class T_TSPCollBarrier >
    class PGAlltoallvExec:public PGExec<T_Geometry,T_TSPColl,T_P2P_NI,T_Device,T_TSPCollBarrier>
    {
    public:
      virtual void start()
        {
          if (!this->_collexch->isdone()) this->_dev->advance();
		  PAMI::Type::TypeCode * sndType = (PAMI::Type::TypeCode *)this->_cmd->cmd.xfer_alltoallv.stype;
		  PAMI::Type::TypeCode * rcvType = (PAMI::Type::TypeCode *)this->_cmd->cmd.xfer_alltoallv.rtype;
          this->_collexch->reset (this->_cmd->cmd.xfer_alltoallv.sndbuf,
                                  this->_cmd->cmd.xfer_alltoallv.rcvbuf,
                                  sndType,
                                  this->_cmd->cmd.xfer_alltoallv.stypecounts,
                                  this->_cmd->cmd.xfer_alltoallv.sdispls,
                                  rcvType,
                                  this->_cmd->cmd.xfer_alltoallv.rtypecounts,
                                  this->_cmd->cmd.xfer_alltoallv.rdispls);
          this->_collexch->setContext(this->_context);
          this->_collexch->setComplete(this->_cmd->cb_done, this->_cmd->cookie);
          while(!this->_collbarrier->isdone()) this->_dev->advance();
          this->_collbarrier->reset();
          this->_collbarrier->setContext(this->_context);
          this->_collbarrier->setComplete(NULL, NULL);
          this->_collbarrier->kick();
          while(!this->_collbarrier->isdone()) this->_dev->advance();
          this->_collexch->kick();
        }
    };



    // --------------  PGAS Scatterv wrapper classes -------------
    template <class T_Geometry, class T_TSPColl, class T_P2P_NI, class T_Device, class T_TSPCollBarrier >
    class PGScattervExec:public PGExec<T_Geometry,T_TSPColl,T_P2P_NI,T_Device,T_TSPCollBarrier>
    {
    public:
      virtual void start()
        {
          if (!this->_collexch->isdone()) this->_dev->advance();
		  PAMI::Type::TypeCode * sndType = (PAMI::Type::TypeCode *)this->_cmd->cmd.xfer_scatterv.stype;
		  PAMI::Type::TypeCode * rcvType = (PAMI::Type::TypeCode *)this->_cmd->cmd.xfer_scatterv.rtype;
          this->_collexch->reset (this->_geometry->virtrankof(this->_cmd->cmd.xfer_scatterv.root),
                                  this->_cmd->cmd.xfer_scatterv.sndbuf,
                                  this->_cmd->cmd.xfer_scatterv.rcvbuf,
								  sndType,
                                  this->_cmd->cmd.xfer_scatterv.stypecounts,
								  rcvType,
								  this->_cmd->cmd.xfer_scatterv.rtypecounts);
          this->_collexch->setContext(this->_context);
          this->_collexch->setComplete(this->_cmd->cb_done, this->_cmd->cookie);

          while(!this->_collbarrier->isdone()) this->_dev->advance();
          this->_collbarrier->reset();
          this->_collbarrier->setContext(this->_context);
          this->_collbarrier->setComplete(NULL, NULL);
          this->_collbarrier->kick();
          while(!this->_collbarrier->isdone()) this->_dev->advance();
          this->_collexch->kick();
        }
    };

    // --------------  PGAS Allreduce wrapper classes -------------
    template <class T_Geometry, class T_TSPColl, class T_P2P_NI, class T_Device>
    class PGAllreduceExec:public PGExec<T_Geometry,T_TSPColl,T_P2P_NI,T_Device>
    {
    public:
      virtual void start()
        {
          if (!this->_collexch->isdone()) this->_dev->advance();
          PAMI::Type::TypeCode * allreducesType = (PAMI::Type::TypeCode *)this->_cmd->cmd.xfer_allreduce.stype;
          PAMI::Type::TypeCode * allreducerType = (PAMI::Type::TypeCode *)this->_cmd->cmd.xfer_allreduce.rtype;
          user_func_t fn;
          this->_collexch->reset (this->_cmd->cmd.xfer_allreduce.sndbuf,
                                  this->_cmd->cmd.xfer_allreduce.rcvbuf,
                                  this->_cmd->cmd.xfer_allreduce.op,
                                  allreducesType,
                                  this->_cmd->cmd.xfer_allreduce.stypecount,
                                  allreducerType,
                                  &fn);
          this->_collexch->setContext(this->_context);
          this->_collexch->setComplete(this->_cmd->cb_done, this->_cmd->cookie);
          this->_collexch->kick();
        }
    };

    // --------------  PGAS Scan wrapper classes -------------
    template <class T_Geometry, class T_TSPColl, class T_P2P_NI, class T_Device>
    class PGScanExec:public PGExec<T_Geometry,T_TSPColl,T_P2P_NI,T_Device>
    {
    public:
      virtual void start()
        {
          if (!this->_collexch->isdone()) this->_dev->advance();
          this->_collexch->setExclusive(this->_cmd->cmd.xfer_scan.exclusive);
          PAMI::Type::TypeCode * scanType = (PAMI::Type::TypeCode *)this->_cmd->cmd.xfer_scan.stype;
          this->_collexch->reset (this->_cmd->cmd.xfer_scan.sndbuf,
                                  this->_cmd->cmd.xfer_scan.rcvbuf,
                                  this->_cmd->cmd.xfer_scan.op,
                                  scanType,
                                  this->_cmd->cmd.xfer_scan.stypecount);
          this->_collexch->setContext(this->_context);
          this->_collexch->setComplete(this->_cmd->cb_done, this->_cmd->cookie);
          this->_collexch->kick();
        }
    };

    // --------------  PGAS Broadcast wrapper classes -------------
    template <class T_Geometry, class T_TSPColl, class T_P2P_NI, class T_Device>
    class PGBroadcastExec:public PGExec<T_Geometry,T_TSPColl,T_P2P_NI,T_Device>
    {
    public:
      virtual void start()
        {
          PAMI::Type::TypeCode * bcastType = (PAMI::Type::TypeCode *)this->_cmd->cmd.xfer_broadcast.type;
          if (!this->_collexch->isdone()) this->_dev->advance();
          this->_collexch->reset (this->_geometry->virtrankof(this->_cmd->cmd.xfer_broadcast.root),
                                  this->_cmd->cmd.xfer_broadcast.buf,
                                  this->_cmd->cmd.xfer_broadcast.buf,
                                  bcastType,
                                  this->_cmd->cmd.xfer_broadcast.typecount);
          this->_collexch->setContext(this->_context);
          this->_collexch->setComplete(this->_cmd->cb_done, this->_cmd->cookie);
          this->_collexch->kick();
        }
    };

  }
}

#endif
