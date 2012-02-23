/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/geometry/FCAWrapper.h
 * \brief ???
 */

#ifndef __algorithms_geometry_FCAWrapper_h__
#define __algorithms_geometry_FCAWrapper_h__

#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "algorithms/composite/Composite.h"
#include "algorithms/geometry/Metadata.h"
#include "components/devices/fca/fcafunc.h"

namespace PAMI{namespace CollRegistration{
// The following classes are used to implement a CCMI wrapper
// around the FCA RT classes
// --------------  FCA Wrapper base class -------------
template <class T_Composite>
class FCAFactory;

template <class T_Geometry>
class FCAComposite:public CCMI::Executor::Composite
{
  typedef FCAFactory<FCAComposite> Factory;
public:
  typedef T_Geometry GeometryType;
  FCAComposite(T_Geometry *g, Factory *f):
    _g(g),
    _f(f)
    {
    }
  virtual void start()                  = 0;
  inline  void setComm(fca_comm_t *c)
    {
      _c = c;
    }
  inline void executeDoneCallback()
    {
      if(this->_cb_done)
        this->_cb_done(this->_context, this->_clientdata, PAMI_SUCCESS);
    }
protected:
  T_Geometry        *_g;
  Factory           *_f;
  fca_comm_t        *_c;
};


// --------------  FCA Factory base class -------------
template <class T_Composite>
class FCAFactory : public CCMI::Adaptor::CollectiveProtocolFactory
{
typedef typename T_Composite::GeometryType T_Geometry;
public:
  FCAFactory(pami_context_t           ctxt,
             size_t                   ctxt_id,
             pami_mapidtogeometry_fn  cb_geometry):
    CollectiveProtocolFactory(ctxt,ctxt_id,cb_geometry)
  {
  }
  virtual CCMI::Executor::Composite * generate(pami_geometry_t  geometry,
                                               void            *cmd)
  {
    T_Composite *c    = (T_Composite*)_alloc.allocateObject();
    pami_xfer_t *xfer = (pami_xfer_t *)cmd;
    T_Geometry  *g    = (T_Geometry*)geometry;
    fca_comm_t  *comm = (fca_comm_t*)(g->getKey(this->_context_id,
                                                Geometry::CKEY_FCAGEOMETRYINFO));
    c->setDoneCallback(xfer->cb_done, xfer->cookie);
    c->setContext(_context);
    c->setComm(comm);
    c->setxfer(xfer);
    return c;
  }
  virtual void metadata(pami_metadata_t *mdata, pami_geometry_t geometry)
  {
    T_Geometry  *g    = (T_Geometry*)geometry;
    fca_comm_t  *comm = (fca_comm_t*)(g->getKey(this->_context_id,
                                                Geometry::CKEY_FCAGEOMETRYINFO));
    T_Composite::metadata(mdata, comm);
  }
  virtual void returnComposite(T_Composite *composite)
    {
      _alloc.returnObject(composite);
    }
  PAMI::MemoryAllocator<sizeof(T_Composite),16>  _alloc;
  const char                                    *_string;
};

const fca_reduce_op_t    _fca_reduce_op_tbl[PAMI_OP_COUNT] =
  {
    FCA_OP_LAST                /*PAMI_COPY (invalid FCA operation) */,
    FCA_OP_LAST                /*PAMI_NOOP (invalid FCA operation) */,
    FCA_OP_MAX                 /*PAMI_MAX*/,
    FCA_OP_MIN                 /*PAMI_MIN*/,
    FCA_OP_SUM                 /*PAMI_SUM*/,
    FCA_OP_PROD                /*PAMI_PROD*/,
    FCA_OP_LAND                /*PAMI_LAND*/,
    FCA_OP_LOR                 /*PAMI_LOR*/,
    FCA_OP_LXOR                /*PAMI_LXOR*/,
    FCA_OP_BAND                /*PAMI_BAND*/,
    FCA_OP_BOR                 /*PAMI_BOR*/,
    FCA_OP_BXOR                /*PAMI_BXOR*/,
    FCA_OP_MAXLOC              /*PAMI_MAXLOC*/,
    FCA_OP_MINLOC              /*PAMI_MINLOC*/
  };

const fca_reduce_dtype_t _fca_reduce_dtype_tbl[PAMI_DT_COUNT] = 
  {
    FCA_DTYPE_UNSIGNED_CHAR    /*PAMI_BYTE  According to standard, MPI_BYTE is similar to MPI_UNSIGNED_CHAR*/,
    FCA_DTYPE_CHAR             /*PAMI_SIGNED_CHAR*/,
    FCA_DTYPE_SHORT            /*PAMI_SIGNED_SHORT*/,
    FCA_DTYPE_INT              /*PAMI_SIGNED_INT*/,
    FCA_DTYPE_LONG             /*PAMI_SIGNED_LONG*/,
    FCA_DTYPE_LAST             /*PAMI_SIGNED_LONG_LONG (invalid FCA type)*/,

    FCA_DTYPE_UNSIGNED_CHAR    /*PAMI_UNSIGNED_CHAR*/,
    FCA_DTYPE_UNSIGNED_SHORT   /*PAMI_UNSIGNED_SHORT*/,
    FCA_DTYPE_UNSIGNED         /*PAMI_UNSIGNED_INT*/,
    FCA_DTYPE_UNSIGNED_LONG    /*PAMI_UNSIGNED_LONG*/,
    FCA_DTYPE_LAST             /*PAMI_UNSIGNED_LONG_LONG (invalid FCA type)*/,

    FCA_DTYPE_FLOAT            /*PAMI_FLOAT*/,
    FCA_DTYPE_DOUBLE           /*PAMI_DOUBLE*/,
    FCA_DTYPE_LAST             /*PAMI_LONG_DOUBLE (invalid FCA type)*/,

    FCA_DTYPE_LAST             /*PAMI_LOGICAL (invalid FCA type)*/,

    FCA_DTYPE_LAST             /*PAMI_SINGLE_COMPLEX (invalid FCA type)*/,
    FCA_DTYPE_LAST             /*PAMI_DOUBLE_COMPLEX (invalid FCA type)*/,

    FCA_DTYPE_2INT             /*PAMI_LOC_2INT*/,
    FCA_DTYPE_LAST             /*PAMI_LOC_2FLOAT (invalid FCA type)*/,
    FCA_DTYPE_LAST             /*PAMI_LOC_2DOUBLE (invalid FCA type)*/,
    FCA_DTYPE_SHORT_INT        /*PAMI_LOC_SHORT_INT*/,
    FCA_DTYPE_FLOAT_INT        /*PAMI_LOC_FLOAT_INT*/,
    FCA_DTYPE_DOUBLE_INT       /*PAMI_LOC_DOUBLE_INT*/
  };


static inline fca_reduce_dtype_t p_dtype_to_fca_dtype(pami_dt dt)
{
  PAMI_assert(dt < PAMI_DT_COUNT);
  fca_reduce_dtype_t fca_dt = _fca_reduce_dtype_tbl[dt];
  PAMI_assert(fca_dt != FCA_DTYPE_LAST);
  return fca_dt;
}
static inline fca_reduce_op_t p_func_to_fca_op(pami_op op)
{
  PAMI_assert(op < PAMI_OP_COUNT);
  fca_reduce_op_t fca_op = _fca_reduce_op_tbl[op];
  PAMI_assert(fca_op != FCA_OP_LAST);
  return fca_op;
}


template <class T_reduce_type>
inline metadata_result_t op_dt_metadata_function(struct pami_xfer_t *in)
{
  T_reduce_type     *reduction = (T_reduce_type*)&in->cmd;
  metadata_result_t  result    = {0};
  uintptr_t          op;
  uintptr_t          dt;
  PAMI::Type::TypeFunc::GetEnums(reduction->stype,
                                 reduction->op,
                                 dt,
                                 op);
  fca_reduce_dtype_t fca_dt = p_dtype_to_fca_dtype((pami_dt)dt);
  fca_reduce_op_t    fca_op = p_func_to_fca_op((pami_op)op);
  if(fca_op < FCA_OP_MAX && fca_dt < FCA_DTYPE_LAST)
    result.check.datatype_op = false;
  else
    result.check.datatype_op = true;

  return(result);
}
// TODO:  convert endpoint based roots to TASKS
// TODO:  figure out progress function details
// --------------  FCA Reduce wrapper classes -------------
template <class T_Geometry>
class FCAReduceExec:public FCAComposite<T_Geometry>
{
  typedef PAMI::Type::TypeCode     Type;
  typedef FCAComposite<T_Geometry> FCAComp;
  typedef FCAFactory<FCAComp>      Factory;
public:
  FCAReduceExec(T_Geometry *g, Factory *f):
    FCAComp(g,f)
  {
  }
  virtual void start()
  {
    int rc = FCA_Do_reduce(this->_c, &_spec);
    PAMI_assertf(rc == 0, "FCA_Do_reduce failed with rc=%d",rc);
    this->executeDoneCallback();
    this->_f->returnComposite(this);
  }
  static inline void metadata(pami_metadata_t *m, fca_comm_t  *comm)
    {
      fca_comm_caps_t comm_caps;
      FCA_Comm_get_caps(comm, &comm_caps);
      new(m) PAMI::Geometry::Metadata(myName());
      m->check_correct.values.alldtop   = 0;
      m->check_fn                       = op_dt_metadata_function<pami_reduce_t>;
      m->check_perf.values.hw_accel     = 1;
      m->range_lo_perf                  = 0;
      m->range_hi_perf                  = comm_caps.max_payload;
    }
  static inline const char *myName()
    {
      return "I1:Reduce:FCA:FCA";
    }
  inline void setxfer(pami_xfer_t *xfer)
    {
      pami_reduce_t *cmd = &(xfer->cmd.xfer_reduce);
      uintptr_t      dt,op;
      PAMI::Type::TypeFunc::GetEnums(cmd->stype,cmd->op,
                                     dt,op);
      _spec.root     = cmd->root;
      _spec.sbuf     = cmd->sndbuf;
      _spec.rbuf     = cmd->rcvbuf;
      _spec.dtype    = p_dtype_to_fca_dtype((pami_dt)dt);
      _spec.length   = cmd->stypecount * ((Type*)cmd->stype)->GetExtent();
      _spec.op       = p_func_to_fca_op((pami_op)op);
    }
private:
  fca_reduce_spec_t  _spec;
};

// --------------  FCA Allreduce wrapper classes -------------
template <class T_Geometry>
class FCAAllreduceExec:public FCAComposite<T_Geometry>
{
public:
  typedef PAMI::Type::TypeCode     Type;
  typedef FCAComposite<T_Geometry> FCAComp;
  typedef FCAFactory<FCAComp>      Factory;
  FCAAllreduceExec(T_Geometry *g, Factory *f):
    FCAComp(g,f)
  {
  }
  virtual void start()
  {
    int rc = FCA_Do_all_reduce(this->_comm, &_spec);
    PAMI_assertf(rc == 0, "FCA_Do_all_reduce failed with rc=%d",rc);
    this->executeDoneCallback();
    this->_f->returnComposite(this);
  }
  static inline void metadata(pami_metadata_t *m, fca_comm_t  *comm)
    {
      fca_comm_caps_t comm_caps;
      FCA_Comm_get_caps(comm, &comm_caps);
      new(m) PAMI::Geometry::Metadata(myName());
      m->check_correct.values.alldtop   = 0;
      m->check_fn                       = op_dt_metadata_function<pami_allreduce_t>;
      m->check_perf.values.hw_accel     = 1;
      m->range_lo_perf                  = 0;
      m->range_hi_perf                  = comm_caps.max_payload;
    }
  static inline const char *myName()
    {
      return "I1:Allreduce:FCA:FCA";
    }
  inline void setxfer(pami_xfer_t *xfer)
    {
      pami_allreduce_t *cmd = &(xfer->cmd.xfer_allreduce);
      uintptr_t         dt,op;
      PAMI::Type::TypeFunc::GetEnums(cmd->stype,cmd->op,
                                     dt,op);
      _spec.root     = -1; // ? this ok?
      _spec.sbuf     = cmd->sndbuf;
      _spec.rbuf     = cmd->rcvbuf;
      _spec.dtype    = p_dtype_to_fca_dtype((pami_dt)dt);
      _spec.length   = cmd->stypecount * ((Type*)cmd->stype)->GetExtent();
      _spec.op       = p_func_to_fca_op((pami_op)op);
    }
private:
  fca_reduce_spec_t  _spec;
};

// --------------  FCA Broadcast wrapper classes -------------
template <class T_Geometry>
class FCABroadcastExec:public FCAComposite<T_Geometry>
{
public:
  typedef PAMI::Type::TypeCode     Type;
  typedef FCAComposite<T_Geometry> FCAComp;
  typedef FCAFactory<FCAComp>      Factory;
  FCABroadcastExec(T_Geometry *g, Factory *f):
    FCAComp(g,f)
    {
    }
  virtual void start()
  {
    int rc = FCA_Do_bcast(this->_comm, &_spec);
    PAMI_assertf(rc == 0, "FCA_Do_bcast failed with rc=%d",rc);
    this->executeDoneCallback();
    this->_f->returnComposite(this);
  }
  static inline void metadata(pami_metadata_t *m, fca_comm_t  *comm)
    {
      new(m) PAMI::Geometry::Metadata(myName());
      m->check_perf.values.hw_accel     = 1;
    }
  static inline const char *myName()
    {
      return "I1:Broadcast:FCA:FCA";
    }
  inline void setxfer(pami_xfer_t *xfer)
  {
    pami_broadcast_t *cmd = &(xfer->cmd.xfer_broadcast);
    _spec.root            = cmd->root;
    _spec.buf             = cmd->buf;
    _spec.size            = cmd->typecount * ((Type*)cmd->type)->GetExtent();
  }
private:
  fca_bcast_spec_t _spec;
};

// --------------  FCA Allgather wrapper classes -------------
template <class T_Geometry>
class FCAAllgatherExec:public FCAComposite<T_Geometry>
{
public:
  typedef PAMI::Type::TypeCode     Type;
  typedef FCAComposite<T_Geometry> FCAComp;
  typedef FCAFactory<FCAComp>      Factory;
  FCAAllgatherExec(T_Geometry *g, Factory *f):
    FCAComp(g,f)
    {
    }
  virtual void start()
  {
    int rc = FCA_Do_allgather(this->_comm, &_spec);
    PAMI_assertf(rc == 0, "FCA_Do_allgather failed with rc=%d",rc);
    this->executeDoneCallback();
    this->_f->returnComposite(this);
  }
  static inline void metadata(pami_metadata_t *m, fca_comm_t  *comm)
    {
      new(m) PAMI::Geometry::Metadata(myName());
      m->check_perf.values.hw_accel     = 1;
    }
  static inline const char *myName()
    {
      return "I1:Allgather:FCA:FCA";
    }
  inline void setxfer(pami_xfer_t *xfer)
    {
      pami_allgather_t *cmd = &(xfer->cmd.xfer_allgather);
      _spec.root            = -1;  // this OK?
      _spec.sbuf            = cmd->sndbuf;
      _spec.rbuf            = cmd->rcvbuf;
      _spec.size            = cmd->stypecount * ((Type*)cmd->stype)->GetExtent();
    }
private:
  fca_gather_spec_t _spec;
};

// --------------  FCA Allgatherv_int wrapper classes -------------
template <class T_Geometry>
class FCAAllgathervIntExec:public FCAComposite<T_Geometry>
{
public:
  typedef PAMI::Type::TypeCode     Type;
  typedef FCAComposite<T_Geometry> FCAComp;
  typedef FCAFactory<FCAComp>      Factory;
  FCAAllgathervIntExec(T_Geometry *g, Factory *f):
    FCAComp(g,f)
    {
    }
  virtual void start()
  {
    int rc = FCA_Do_allgatherv(this->_comm, &_spec);
    PAMI_assertf(rc == 0, "FCA_Do_allgatherv failed with rc=%d",rc);
  }
  static inline void metadata(pami_metadata_t *m, fca_comm_t  *comm)
    {
      new(m) PAMI::Geometry::Metadata(myName());
      m->check_perf.values.hw_accel     = 1;
    }
  static inline const char *myName()
    {
      return "I1:AllgathervInt:FCA:FCA";
    }
  inline void setxfer(pami_xfer_t *xfer)
    {
      pami_allgatherv_int_t *cmd = &(xfer->cmd.xfer_allgatherv_int);
      _spec.root             = -1;  // this OK?
      _spec.sbuf             = cmd->sndbuf;
      _spec.rbuf             = cmd->rcvbuf;
      _spec.sendsize         = cmd->stypecount * ((Type*)cmd->stype)->GetExtent();
      _spec.recvsizes        = cmd->rtypecounts;
      _spec.displs           = cmd->rdispls;
    }
private:
  fca_gatherv_spec_t _spec;
};

// --------------  FCA Barrier wrapper classes -------------
template <class T_Geometry>
class FCABarrierExec:public FCAComposite<T_Geometry>
{
public:
  typedef FCAComposite<T_Geometry> FCAComp;
  typedef FCAFactory<FCAComp>      Factory;
  FCABarrierExec(T_Geometry *g, Factory *f):
    FCAComp(g,f)
    {
    }
  virtual void start()
  {
    int rc = FCA_Do_barrier(this->_comm);
    PAMI_assertf(rc == 0, "FCA_Do_barrier failed with rc=%d",rc);
    this->executeDoneCallback();
    this->_f->returnComposite(this);
  }
  static inline void metadata(pami_metadata_t *m, fca_comm_t  *comm)
    {
      new(m) PAMI::Geometry::Metadata(myName());
      m->check_perf.values.hw_accel     = 1;
    }
  static inline const char *myName()
    {
      return "I1:Barrier:FCA:FCA";
    }
  inline void setxfer(pami_xfer_t *xfer)
    {
    }
private:

};

}//namespace CollRegistration
}//namespace PAMI


#endif
