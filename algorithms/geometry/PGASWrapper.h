/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common/default/PGASWrapper.h
 * \brief ???
 */

#ifndef __common_pgas_wrapper_h__
#define __common_pgas_wrapper_h__


#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "algorithms/protocols/tspcoll/NBCollManager.h"
#include "algorithms/composite/Composite.h"
#include "algorithms/geometry/Geometry.h"
namespace XMI
{
  namespace CollRegistration
  {
    // The following classes are used to implement a CCMI wrapper
    // around the PGAS RT classes

    // --------------  PGAS Wrapper base class -------------
    template <class T_TSPColl, class T_Mcast, class T_Device, class T_TSPCollBarrier = void*>
    class PGExec:public CCMI::Executor::Composite
    {
    public:
      virtual void start() = 0;
      virtual void setcmd(T_TSPColl        *collexch,
                          xmi_xfer_t       *cmd,
                          T_Device         *dev,
                          T_Mcast          *model,
                          T_TSPCollBarrier *collbarrier,
                          xmi_geometry_t    geometry,
                          T_Mcast          *barmodel)
        {

          _cmd         = cmd;
          _collexch    = collexch;
          _dev         = dev;
          _model       = model;
          _collbarrier = collbarrier;
          _barmodel    = barmodel;
          _geometry    = (XMI::Geometry::Common*)geometry;
        }
    public:
      xmi_xfer_t                      *_cmd;
      T_TSPColl                       *_collexch;
      T_Device                        *_dev;
      T_Mcast                         *_model;
      T_TSPCollBarrier                *_collbarrier;
      T_Mcast                         *_barmodel;
      XMI::Geometry::Common           *_geometry;
    };

    // --------------  PGAS Factory base class -------------
    template <class       T_TSPColl,
              class       T_Mcast,
              class       T_Device,
              class       T_Exec,
              const char *T_MDString,
              class       T_TSPCollBarrier = void*>
    class PGFactory : public CCMI::Adaptor::CollectiveProtocolFactory
    {
    public:
      PGFactory<T_TSPColl,
                T_Mcast,
                T_Device,
                T_Exec,
                T_MDString,
                T_TSPCollBarrier>(T_Device         *dev,
                                  T_Mcast          *model,
                                  T_TSPColl        *coll,
                                  T_TSPCollBarrier *collbarrier = NULL,
                                  T_Mcast          *barmodel    = NULL  ):
        _coll(coll),
        _dev(dev),
        _model(model),
        _barmodel(barmodel),
        _collbarrier(collbarrier)
        {
        }
      virtual CCMI::Executor::Composite * generate(xmi_geometry_t  geometry,
                                                   void           *cmd)
        {
          _exec.setcmd(_coll,
                       (xmi_xfer_t*)cmd,
                       _dev,
                       _model,
                       _collbarrier,
                       geometry,
                       _barmodel);
          return (CCMI::Executor::Composite*)&_exec;
        }
      virtual void metadata(xmi_metadata_t *mdata)
        {
          strcpy(mdata->name, T_MDString);
        }
      T_Exec            _exec;
      T_TSPColl        *_coll;
      T_Device         *_dev;
      T_Mcast          *_model;
      T_Mcast          *_barmodel;
      T_TSPCollBarrier *_collbarrier;
    };

    // --------------  PGAS Barrier wrapper classes -------------
    template <class T_TSPColl, class T_Mcast, class T_Device>
    class PGBarrierExec:public PGExec<T_TSPColl,T_Mcast,T_Device>
    {
    public:
      virtual void start()
        {
          while(!this->_collexch->isdone()) this->_dev->advance();
          this->_collexch->reset();
          this->_collexch->setComplete(this->_cmd->cb_done, this->_cmd->cookie);
          this->_collexch->kick(this->_model);
        }
    };
    // --------------  PGAS Allgather wrapper classes -------------
    template <class T_TSPColl, class T_Mcast, class T_Device>
    class PGAllgatherExec:public PGExec<T_TSPColl,T_Mcast,T_Device>
    {
    public:
      virtual void start()
        {
          if (!this->_collexch->isdone()) this->_dev->advance();
          this->_collexch->reset (this->_cmd->cmd.xfer_allgather.sndbuf,
                                  this->_cmd->cmd.xfer_allgather.rcvbuf,
                                  this->_cmd->cmd.xfer_allgather.stypecount);
          this->_collexch->setComplete(this->_cmd->cb_done, this->_cmd->cookie);
          this->_collexch->kick(this->_model);
        }
    };

    // --------------  PGAS Allgatherv wrapper classes -------------
    template <class T_TSPColl, class T_Mcast, class T_Device>
    class PGAllgathervExec:public PGExec<T_TSPColl,T_Mcast,T_Device>
    {
    public:
      virtual void start()
        {
          if (!this->_collexch->isdone()) this->_dev->advance();
          this->_collexch->reset (this->_cmd->cmd.xfer_allgatherv.sndbuf,
                                  this->_cmd->cmd.xfer_allgatherv.rcvbuf,
                                  this->_cmd->cmd.xfer_allgatherv.rtypecounts);
          this->_collexch->setComplete(this->_cmd->cb_done, this->_cmd->cookie);
          this->_collexch->kick(this->_model);
        }
    };

    // --------------  PGAS Scatter wrapper classes -------------
    template <class T_TSPColl, class T_Mcast, class T_Device, class T_TSPCollBarrier >
    class PGScatterExec:public PGExec<T_TSPColl,T_Mcast,T_Device,T_TSPCollBarrier>
    {
    public:
      virtual void start()
        {
          if (!this->_collexch->isdone()) this->_dev->advance();
          this->_collexch->reset (this->_geometry->virtrankof(this->_cmd->cmd.xfer_scatter.root),
                                  this->_cmd->cmd.xfer_scatter.sndbuf,
                                  this->_cmd->cmd.xfer_scatter.rcvbuf,
                                  this->_cmd->cmd.xfer_scatter.stypecount);
          this->_collexch->setComplete(this->_cmd->cb_done, this->_cmd->cookie);

          while(!this->_collbarrier->isdone()) this->_dev->advance();
          this->_collbarrier->reset();
          this->_collbarrier->setComplete(NULL, NULL);
          this->_collbarrier->kick(this->_barmodel);
          while(!this->_collbarrier->isdone()) this->_dev->advance();
          this->_collexch->kick(this->_model);
        }
    };

    // --------------  PGAS Scatterv wrapper classes -------------
    template <class T_TSPColl, class T_Mcast, class T_Device, class T_TSPCollBarrier >
    class PGScattervExec:public PGExec<T_TSPColl,T_Mcast,T_Device,T_TSPCollBarrier>
    {
    public:
      virtual void start()
        {
          if (!this->_collexch->isdone()) this->_dev->advance();
          this->_collexch->reset (this->_geometry->virtrankof(this->_cmd->cmd.xfer_scatterv.root),
                                  this->_cmd->cmd.xfer_scatterv.sndbuf,
                                  this->_cmd->cmd.xfer_scatterv.rcvbuf,
                                  this->_cmd->cmd.xfer_scatterv.stypecounts);
          this->_collexch->setComplete(this->_cmd->cb_done, this->_cmd->cookie);

          while(!this->_collbarrier->isdone()) this->_dev->advance();
          this->_collbarrier->reset();
          this->_collbarrier->setComplete(NULL, NULL);
          this->_collbarrier->kick(this->_barmodel);
          while(!this->_collbarrier->isdone()) this->_dev->advance();
          this->_collexch->kick(this->_model);
        }
    };

    // --------------  PGAS Allreduce wrapper classes -------------
    template <class T_TSPColl, class T_Mcast, class T_Device>
    class PGAllreduceExec:public PGExec<T_TSPColl,T_Mcast,T_Device>
    {
    public:
      virtual void start()
        {
          if (!this->_collexch->isdone()) this->_dev->advance();
          this->_collexch->reset (this->_cmd->cmd.xfer_allreduce.sndbuf,
                                  this->_cmd->cmd.xfer_allreduce.rcvbuf,
                                  this->_cmd->cmd.xfer_allreduce.op,
                                  this->_cmd->cmd.xfer_allreduce.dt,
                                  this->_cmd->cmd.xfer_allreduce.stypecount);
          this->_collexch->setComplete(this->_cmd->cb_done, this->_cmd->cookie);
          this->_collexch->kick(this->_model);
        }
    };

    // --------------  PGAS Broadcast wrapper classes -------------
    template <class T_TSPColl, class T_Mcast, class T_Device>
    class PGBroadcastExec:public PGExec<T_TSPColl,T_Mcast,T_Device>
    {
    public:
      virtual void start()
        {
          if (!this->_collexch->isdone()) this->_dev->advance();
          this->_collexch->reset (this->_geometry->virtrankof(this->_cmd->cmd.xfer_broadcast.root),
                                  this->_cmd->cmd.xfer_broadcast.buf,
                                  this->_cmd->cmd.xfer_broadcast.buf,
                                  this->_cmd->cmd.xfer_broadcast.typecount);
          this->_collexch->setComplete(this->_cmd->cb_done, this->_cmd->cookie);
          this->_collexch->kick(this->_model);
        }
    };

  }
}

#endif
