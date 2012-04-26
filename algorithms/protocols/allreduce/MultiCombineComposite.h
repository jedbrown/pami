/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing,  DO NOT MODIFY OR MOVE                       */
/*  --------------------------------------------------------------- */
/* Licensed Materials - Property of IBM                             */
/* Blue Gene/Q 5765-PER 5765-PRP                                    */
/*                                                                  */
/* (C) Copyright IBM Corp. 2011, 2012 All Rights Reserved           */
/* US Government Users Restricted Rights -                          */
/* Use, duplication, or disclosure restricted                       */
/* by GSA ADP Schedule Contract with IBM Corp.                      */
/*                                                                  */
/*  --------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/MultiCombineComposite.h
 * \brief Simple composite based on multicombine
 */
#ifndef __algorithms_protocols_allreduce_MultiCombineComposite_h__
#define __algorithms_protocols_allreduce_MultiCombineComposite_h__

#include "algorithms/composite/Composite.h"
#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "algorithms/protocols/allreduce/ReduceFunctions.h"

#include "util/ccmi_debug.h"
#include "util/ccmi_util.h"


// Use a local done function for testing
//#define LOCAL_TEST
//#undef  DO_DEBUG
//#define DO_DEBUG(x) x

//#undef DO_TRACE_ENTEREXIT
#undef DO_TRACE_DEBUG

//#define CCMI_TRACE_ALL
#ifdef CCMI_TRACE_ALL
  #define DO_TRACE_ENTEREXIT 1
  #define DO_TRACE_DEBUG     1
// Use a local done function for testing
  #undef LOCAL_TEST
  #define LOCAL_TEST
#else
  #define DO_TRACE_ENTEREXIT 0
  #define DO_TRACE_DEBUG     0
#endif



namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {
      template <bool T_inline=false, class T_Native=Interfaces::NativeInterface>
      class MultiCombineComposite : public CCMI::Executor::Composite
      {
        protected:
          //Interfaces::NativeInterface        * _native;
          //PAMI_GEOMETRY_CLASS                * _geometry;
          PAMI::PipeWorkQueue                  _srcPwq;
          PAMI::PipeWorkQueue                  _dstPwq;
          //pami_multicombine_t                  _minfo;
          //void                               * _deviceInfo;

        public:
          MultiCombineComposite (pami_context_t               ctxt,
                                 size_t                       ctxt_id,
                                 Interfaces::NativeInterface          * native,
                                 ConnectionManager::SimpleConnMgr     * cmgr,
                                 pami_geometry_t                        g,
                                 pami_xfer_t                          * cmd,
                                 pami_event_function                    fn,
                                 void                                 * cookie) :
            Composite()//, _native(mInterface), _geometry((PAMI_GEOMETRY_CLASS*)g)
          {
            TRACE_FN_ENTER();
            uintptr_t op, dt;
            PAMI::Type::TypeFunc::GetEnums(cmd->cmd.xfer_allreduce.stype,
                                           cmd->cmd.xfer_allreduce.op,
                                           dt,op);

            TRACE_FORMAT( "type %#zX/%#zX, count %zu/%zu, op %#X, dt %#X",
                           (size_t)cmd->cmd.xfer_allreduce.stype, (size_t)cmd->cmd.xfer_allreduce.rtype,
                          cmd->cmd.xfer_allreduce.stypecount, cmd->cmd.xfer_allreduce.rtypecount, (pami_op)op, (pami_dt)dt);

            PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS*)g;
            void *deviceInfo                  = geometry->getKey(0,PAMI::Geometry::CKEY_MCOMB_CLASSROUTEID);

            TypeCode * stype_obj = (TypeCode *)cmd->cmd.xfer_allreduce.stype;
            TypeCode * rtype_obj = (TypeCode *)cmd->cmd.xfer_allreduce.rtype;

            /// \todo Support non-contiguous
            PAMI_assert(stype_obj->IsContiguous() &&  stype_obj->IsPrimitive());
            PAMI_assert(rtype_obj->IsContiguous() &&  rtype_obj->IsPrimitive());

            unsigned        ssizeOfType = stype_obj->GetDataSize();
            size_t sbytes = cmd->cmd.xfer_allreduce.stypecount * ssizeOfType;
            unsigned        rsizeOfType = rtype_obj->GetDataSize();
            size_t rbytes = cmd->cmd.xfer_allreduce.rtypecount * rsizeOfType;
            TRACE_FORMAT( "srcPwq.configure %zu",sbytes);
            _srcPwq.configure(cmd->cmd.xfer_allreduce.sndbuf, sbytes, sbytes, stype_obj, rtype_obj);

            TRACE_FORMAT( "dstPwq.configure %zu",rbytes);
            _dstPwq.configure(cmd->cmd.xfer_allreduce.rcvbuf, rbytes, 0, stype_obj, rtype_obj);// SSS: Should the types be in this order???

            DO_DEBUG(PAMI::Topology all);
            DO_DEBUG(all = *(PAMI::Topology*)geometry->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX));

            DO_DEBUG(for (unsigned j = 0; j < all.size(); ++j) TRACE_FORMAT("all[%u]=%zu, size %zu", j, (size_t)all.index2Endpoint(j), all.size()));

            pami_multicombine_t minfo;
            minfo.cb_done.function     = fn;
            minfo.cb_done.clientdata   = cookie;
            minfo.connection_id        = 0;
            minfo.roles                = -1U;
            minfo.results_participants = geometry->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX);
            minfo.data_participants    = geometry->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX);
            minfo.data                 = (pami_pipeworkqueue_t *) & _srcPwq;
            minfo.results              = (pami_pipeworkqueue_t *) & _dstPwq;
            minfo.optor                = (pami_op)op;
            minfo.dtype                = (pami_dt)dt;
            minfo.count                = cmd->cmd.xfer_allreduce.stypecount;

            if (T_inline) {
              T_Native *t_native = (T_Native *) native;
              t_native->T_Native::multicombine(&minfo, deviceInfo);
            }
            else {
              native->multicombine(&minfo, deviceInfo);
            }

            TRACE_FORMAT( "count %zu", minfo.count);
            TRACE_FN_EXIT();
          }

          virtual void start()
          {
            TRACE_FN_ENTER();
            TRACE_FN_EXIT();
          }

	  void setContext (pami_context_t context) {}
      };

      /// \brief This is a factory for a 2 device MultiCombine
      /// The two device MultiCombine takes two native interfaces, the first is
      /// the local interface, and the second is the global interface
      template <class T_Composite, MetaDataFn get_metadata, class T_Connmgr, int LookupNI=0>
      class MultiCombineComposite2DeviceFactoryT: public CollectiveProtocolFactory
      {
          class collObj
          {
            public:
              collObj(Interfaces::NativeInterface          *native0,
                      Interfaces::NativeInterface          *native1,
                      T_Connmgr                            *cmgr,
                      pami_geometry_t                       geometry,
                      pami_xfer_t                          *cmd,
                      pami_event_function                   fn,
                      void                                 *cookie,
                      MultiCombineComposite2DeviceFactoryT *factory):
                  _factory(factory),
                  _user_done_fn(cmd->cb_done),
                  _user_cookie(cmd->cookie),
                  _obj(native0, native1, cmgr, geometry, cmd, fn, cookie)
              {
              }
              void done_fn( pami_context_t   context,
                            pami_result_t    result )
              {
                _user_done_fn(context, _user_cookie, result);
              }

              MultiCombineComposite2DeviceFactoryT *_factory;
              pami_event_function                   _user_done_fn;
              void                                 *_user_cookie;
              T_Composite                           _obj;
              unsigned                              _connection_id;
          };
        public:
          MultiCombineComposite2DeviceFactoryT (pami_context_t               ctxt,
                                                size_t                       ctxt_id,
                                                pami_mapidtogeometry_fn      cb_geometry,
                                                T_Connmgr                   *cmgr,
                                                Interfaces::NativeInterface *native_l,
                                                Interfaces::NativeInterface *native_g):
            CollectiveProtocolFactory(ctxt,ctxt_id,cb_geometry),
              _cmgr(cmgr),
              _native_l(native_l),
              _native_g(native_g)
          {
          }

          virtual ~MultiCombineComposite2DeviceFactoryT()
          {
          }
          virtual void metadata(pami_metadata_t *mdata, pami_geometry_t geometry = PAMI_GEOMETRY_NULL)
          {
            get_metadata(mdata);
          }
          static void done_fn(pami_context_t  context,
                              void           *clientdata,
                              pami_result_t   res)
          {
            collObj *cobj = (collObj *)clientdata;
            cobj->done_fn(context, res);
            cobj->_factory->_alloc.returnObject(cobj);
          }
          virtual Executor::Composite * generate(pami_geometry_t  geometry,
                                                 void            *cmd)
          {
            TRACE_FN_ENTER();

            // This should compile out if native interfaces are scoped
            // globally.
            if(LookupNI)
            {
              PAMI_GEOMETRY_CLASS *g = (PAMI_GEOMETRY_CLASS*)geometry;
              _native_l = _ni_local_map[g->comm()];
              _native_g = _ni_global_map[g->comm()];
            }

            collObj *cobj = (collObj*) _alloc.allocateObject();
            TRACE_FORMAT( "<%p>", cobj);
            new(cobj) collObj(_native_l,          // Native interface local
                              _native_g,          // Native Interface global
                              _cmgr,              // Connection Manager
                              geometry,           // Geometry Object
                              (pami_xfer_t*)cmd,  // Parameters
                              done_fn,            // Intercept function
                              cobj,               // Intercept cookie
                              this);
            TRACE_FN_EXIT();
            return (Executor::Composite *)&cobj->_obj;

          }
        inline void setNI(pami_geometry_t geometry,
                           Interfaces::NativeInterface *nativeL,
                           Interfaces::NativeInterface *nativeG)
          {
            PAMI_assert(LookupNI == 1); // no local master?
            PAMI_GEOMETRY_CLASS *g = (PAMI_GEOMETRY_CLASS*)geometry;
            _ni_local_map[g->comm()]  = nativeL;
            _ni_global_map[g->comm()] = nativeG;
            _native_l = NULL;
            _native_g = NULL;
          }

        protected:
          T_Connmgr                                       *_cmgr;
          Interfaces::NativeInterface                     *_native_l;
          Interfaces::NativeInterface                     *_native_g;
          PAMI::MemoryAllocator < sizeof(collObj), 16 >    _alloc;
          std::map<size_t,Interfaces::NativeInterface *>   _ni_local_map;
          std::map<size_t,Interfaces::NativeInterface *>   _ni_global_map;
      };

      /// \brief All sided allreduce over active message multicombines
      /// A local device will chain into a global multicombine
      template <int ReduceOnly>
      class MultiCombineComposite2Device : public CCMI::Executor::Composite
      {
        public:
          static void composite_done(pami_context_t  context,
                                     void           *cookie,
                                     pami_result_t   result)
          {
            TRACE_FN_ENTER();
            MultiCombineComposite2Device *m = (MultiCombineComposite2Device*) cookie;
            m->_count--;
            TRACE_FORMAT( "count=%zd", m->_count);

            if (m->_count == 0)
            {
              m->_fn(context, m->_cookie, result);
              TRACE_FORMAT( "Delivered Callback Function To User=%zd", m->_count);
              if(m->_temp_results)
                __global.heap_mm->free(m->_temp_results);
              if(m->_throwaway_results)
                __global.heap_mm->free(m->_throwaway_results);
            }
            TRACE_FORMAT( "Phases Left: count=%zd", m->_count);
            TRACE_FN_EXIT();
          }


        template <class T_ReduceType>
        inline void setupReduceCommon(Interfaces::NativeInterface      *native_l,
                                      Interfaces::NativeInterface      *native_g,
                                      ConnectionManager::SimpleConnMgr *cmgr,
                                      pami_geometry_t                   g,
                                      pami_xfer_t                      *cmd,
                                      pami_event_function               fn,
                                      void                             *cookie,
                                      pami_endpoint_t                   root)
          {

            T_ReduceType    * xfer       = (T_ReduceType*)&cmd->cmd;
            TRACE_FN_ENTER();
            pami_result_t    rc          = PAMI_SUCCESS;
            PAMI::Topology  *t_master    = (PAMI::Topology*)_geometry->getTopology(PAMI::Geometry::MASTER_TOPOLOGY_INDEX);
            PAMI::Topology  *t_local     = (PAMI::Topology*)_geometry->getTopology(PAMI::Geometry::LOCAL_TOPOLOGY_INDEX);
            PAMI::Topology  *t_my_master = (PAMI::Topology*)_geometry->getTopology(PAMI::Geometry::LOCAL_MASTER_TOPOLOGY_INDEX);
            bool             amMaster    = t_my_master->isEndpointMember(native_l->endpoint());
            _deviceInfo                  = _geometry->getKey(0,PAMI::Geometry::CKEY_MCOMB_CLASSROUTEID);
            // todo:  shared mem may need its own devinfo
            unsigned        typesize;
            coremath        func;
            uintptr_t op, dt;
            PAMI::Type::TypeFunc::GetEnums(xfer->stype,
                                           xfer->op,
                                           dt,op);

            getReduceFunction((pami_dt)dt,
                              (pami_op)op,
                              typesize,
                              func);
            TypeCode       * stype       = (TypeCode *)xfer->stype;
            TypeCode       * rtype       = (TypeCode *)xfer->rtype;
            size_t           sizeOfType  = stype->GetDataSize();
            size_t           sbytes      = xfer->stypecount * sizeOfType;
            size_t           scountDt    = xfer->stypecount;
            bool             amRoot      = false;

            if(root != 0xFFFFFFFF)
            {
              _root_ep    = root;
              amRoot      = (native_l->endpoint() == root);
              new(&_t_root) PAMI::Topology(&_root_ep, 1, PAMI::tag_eplist());
              PAMI_assert(rc == PAMI_SUCCESS);
            }

            char *rcvBuf;
            // Create a "flat pwq" for the send buffer
            _pwq_src.configure(xfer->sndbuf,                       // buffer
                               sbytes,                             // buffer bytes
                               sbytes,                             // amount initially in buffer
                               stype,
                               rtype);
            if(amRoot || root == 0xFFFFFFFF)  // I am the real root of a Reduce, or I am in an allreduce
            {
              rcvBuf = xfer->rcvbuf;
              _pwq_dest.configure(xfer->rcvbuf,                    // buffer
                                  sbytes,                          // buffer bytes
                                  0,                               // amount initially in buffer
                                  stype,
                                  rtype);
            }
            else                              // I am a non-root, and I must throw away the results
            {
              rc = __global.heap_mm->memalign((void **)&_throwaway_results, 16, sbytes);
              if(sbytes)PAMI_assert(rc == PAMI_SUCCESS && _throwaway_results != NULL);
              rcvBuf = _throwaway_results;
              _pwq_dest.configure(_throwaway_results,                   // buffer
                                  sbytes,                          // buffer bytes
                                  0,                               // amount initially in buffer
                                  stype,
                                  rtype);
            }

            _user_done.clientdata = cmd->cookie;
            _user_done.function   = cmd->cb_done;

            // The "Only Local" case
            // This means the geometry only contains local tasks
            // A single multicombine will suffice to handle the allreduce
            if (t_local->size() == _geometry->size() && amMaster)
              {
                _pwq_inter0.configure(
                                      rcvBuf,  // buffer
                                      sbytes,                          // buffer bytes
                                      0,                               // amount initially in buffer
                                      stype,
                                      rtype);
                _mcombine_l.cb_done.clientdata   = this;
                _mcombine_l.cb_done.function     = composite_done;
                _mcombine_l.connection_id        = _geometry->comm();
                _mcombine_l.roles                = -1U;
                _mcombine_l.data                 = (pami_pipeworkqueue_t*) & _pwq_src;
                _mcombine_l.data_participants    = (pami_topology_t*)t_local;
                _mcombine_l.results              = (pami_pipeworkqueue_t*) & _pwq_inter0;
                _mcombine_l.results_participants = (pami_topology_t*)t_my_master;
                _mcombine_l.optor                = (pami_op)op;
                _mcombine_l.dtype                = (pami_dt)dt;
                _mcombine_l.count                = scountDt;

                // Also, prepare the local multicast, with the root as the local master
                // We are guaranteed to not have an early arrival with this multicast if we post
                // it first.  This protocol expects the local multicast to be 2 sided
                _mcast_l.cb_done.function        = composite_done;
                _mcast_l.cb_done.clientdata      = this;
                _mcast_l.connection_id           = _geometry->comm();
                _mcast_l.roles                   = -1U;
                _mcast_l.bytes                   = xfer->stypecount*stype->GetDataSize();
                _mcast_l.src                     = (pami_pipeworkqueue_t*) & _pwq_inter0;
                _mcast_l.src_participants        = (pami_topology_t*)t_my_master;
                _mcast_l.dst                     = (pami_pipeworkqueue_t*) & _pwq_dest;
                _mcast_l.dst_participants        = (pami_topology_t*)t_local;
                _mcast_l.msginfo                 = 0;
                _mcast_l.msgcount                = 0;
                _startFcn                        = &MultiCombineComposite2Device::start2;
                _count                           = 2;

                TRACE_FORMAT( "<%p>Local Only MASTER Setting up start2: local native() %p", this, _native_l);
                return;
              }

            // The "Only Global Master" case
            // My task only belongs to a global master, with no other local task
            // A single multicombine will suffice in this case as well
            // To detect this case, the local size will be only me
            if (t_local->size() == 1)
              {
                _mcombine_g.cb_done.clientdata   = this;
                _mcombine_g.cb_done.function     = composite_done;
                _mcombine_g.connection_id        = _geometry->comm();
                _mcombine_g.roles                = -1U;
                _mcombine_g.data                 = (pami_pipeworkqueue_t*) & _pwq_src;
                _mcombine_g.data_participants    = (pami_topology_t*)t_master;
                _mcombine_g.results              = (pami_pipeworkqueue_t*) & _pwq_dest;
                _mcombine_g.results_participants = (pami_topology_t*)t_master;
                _mcombine_g.optor                = (pami_op)op;
                _mcombine_g.dtype                = (pami_dt)dt;
                _mcombine_g.count                = scountDt;
                _startFcn                        = &MultiCombineComposite2Device::start1;
                _count                           = 1;
                TRACE_FORMAT( "<%p>Global Only Setting up start1:", this);
                return;
              }

            // This is the non-master participant in the reduction
            // In this case, the task will participate in a local multicombine as a contributer(only)
            // followed by a local multicast(as a recipient).  To detect this case
            // we must not be a master task

            // This extra PWQ is pointing at the reception buffer.  This means that the
            // reception buffers will be overwritten, maybe more than once
            // \todo Do we need some scratch space if we want to do something like in place?
            _pwq_inter0.configure(
                                  rcvBuf,  // buffer
                                  sbytes,                          // buffer bytes
                                  0,                               // amount initially in buffer
                                  stype,
                                  rtype);

            if (!amMaster)
              {
                // The local multicombine
                // Source is local topology
                // Destination is the global master, a reduction
                //
                // \note we supply a result pwq in case the mcombine_l expects reduce
                // and can't do a simple reduction (probably ignores results_participants)
                //
                _mcombine_l.cb_done.clientdata   = this;
                _mcombine_l.cb_done.function     = composite_done;
                _mcombine_l.connection_id        = _geometry->comm();
                _mcombine_l.roles                = -1U;
                _mcombine_l.data                 = (pami_pipeworkqueue_t*) & _pwq_src;
                _mcombine_l.data_participants    = (pami_topology_t*)t_local;
                _mcombine_l.results              = (pami_pipeworkqueue_t*) & _pwq_inter0; // results can go in dest buffer
                _mcombine_l.results_participants = (pami_topology_t*)t_my_master;
                _mcombine_l.optor                = (pami_op)op;
                _mcombine_l.dtype                = (pami_dt)dt;
                _mcombine_l.count                = scountDt;

                // Also, prepare the local multicast, with the root as the local master
                // We are guaranteed to not have an early arrival with this multicast if we post
                // it first.  This protocol expects the local multicast to be 2 sided
                _mcast_l.cb_done.function        = composite_done;
                _mcast_l.cb_done.clientdata      = this;
                _mcast_l.connection_id           = _geometry->comm();
                _mcast_l.roles                   = -1U;
                _mcast_l.bytes                   = xfer->stypecount * stype->GetDataSize();
                _mcast_l.src                     = NULL;
                _mcast_l.src_participants        = (pami_topology_t*)t_my_master;
                _mcast_l.dst                     = (pami_pipeworkqueue_t*) & _pwq_dest;
                _mcast_l.dst_participants        = (pami_topology_t*)t_local;
                _mcast_l.msginfo                 = 0;
                _mcast_l.msgcount                = 0;
                _startFcn                        = &MultiCombineComposite2Device::start2;
                _count                           = 2;
                TRACE_FORMAT( "<%p>Non Master Setting up start2:", this);
                return;
              }

            // The final case:  this task a global master, with local nodes as well
            // In this case, we will be the target of a local reduction, and participate
            // in this reduction.  The results will chain into a global reduction, which
            // will chain into a local mcast:
            // local_mc[local_topo,me]-->global_mc[master topo, master topo]-->local_mcast[me, local_topo]

            // This extra PWQ is pointing at the reception buffer.  This means that the
            // reception buffers will be overwritten, maybe more than once
            // \todo Do we need some scratch space if we want to do something like in place?
            _pwq_inter1.configure(
                                  rcvBuf,  // buffer
                                  sbytes,                          // buffer bytes
                                  0,                               // amount initially in buffer
                                  stype,
                                  rtype);

            _mcombine_l.cb_done.clientdata   = this;
            _mcombine_l.cb_done.function     = composite_done;
            _mcombine_l.connection_id        = _geometry->comm();
            _mcombine_l.roles                = -1U;
            _mcombine_l.data                 = (pami_pipeworkqueue_t*) & _pwq_src;
            _mcombine_l.data_participants    = (pami_topology_t*)t_local;
            _mcombine_l.results              = (pami_pipeworkqueue_t*) & _pwq_inter0; // results can go in dest buffer
            _mcombine_l.results_participants = (pami_topology_t*)t_my_master;       // me!
            _mcombine_l.optor                = (pami_op)op;
            _mcombine_l.dtype                = (pami_dt)dt;
            _mcombine_l.count                = scountDt;

            _mcombine_g.cb_done.clientdata   = this;
            _mcombine_g.cb_done.function     = composite_done;
            _mcombine_g.connection_id        = _geometry->comm();
            _mcombine_g.roles                = -1U;
            _mcombine_g.data                 = (pami_pipeworkqueue_t*) & _pwq_inter0;
            _mcombine_g.data_participants    = (pami_topology_t*)t_master;
            _mcombine_g.results              = (pami_pipeworkqueue_t*) & _pwq_inter1;
            _mcombine_g.results_participants = (pami_topology_t*)t_master;
            _mcombine_g.optor                = (pami_op)op;
            _mcombine_g.dtype                = (pami_dt)dt;
            _mcombine_g.count                = scountDt;

            _mcast_l.cb_done.function        = composite_done;
            _mcast_l.cb_done.clientdata      = this;
            _mcast_l.connection_id           = _geometry->comm();
            _mcast_l.roles                   = -1U;
            _mcast_l.bytes                   = xfer->stypecount * stype->GetDataSize();
            _mcast_l.src                     = (pami_pipeworkqueue_t*) & _pwq_inter1;
            _mcast_l.src_participants        = (pami_topology_t*)t_my_master;  // me!
            _mcast_l.dst                     = (pami_pipeworkqueue_t*) & _pwq_dest;
            _mcast_l.dst_participants        = (pami_topology_t*)t_local;
            _mcast_l.msginfo                 = 0;
            _mcast_l.msgcount                = 0;
            _count                           = 3;
            _startFcn                        = &MultiCombineComposite2Device::start3;
            TRACE_FORMAT( "<%p>Master(local and global) Setting up start3:", this);
            TRACE_FN_EXIT();
          }


        // This protocol currently has some issues with the CollShm device.
        // It is currently disabled in favor of the "over allreduce" template
        inline void setupReduce(Interfaces::NativeInterface      *native_l,
                                Interfaces::NativeInterface      *native_g,
                                ConnectionManager::SimpleConnMgr *cmgr,
                                pami_geometry_t                   g,
                                pami_xfer_t                      *cmd,
                                pami_event_function               fn,
                                void                             *cookie)
          {
            TRACE_FN_ENTER();
            pami_result_t    rc             = PAMI_SUCCESS;
            PAMI::Topology  *t_master       = (PAMI::Topology*)_geometry->getTopology(PAMI::Geometry::MASTER_TOPOLOGY_INDEX);
            PAMI::Topology  *t_local        = (PAMI::Topology*)_geometry->getTopology(PAMI::Geometry::LOCAL_TOPOLOGY_INDEX);
            PAMI::Topology  *t_my_master    = (PAMI::Topology*)_geometry->getTopology(PAMI::Geometry::LOCAL_MASTER_TOPOLOGY_INDEX);
            bool             amMaster       = t_my_master->isEndpointMember(native_l->endpoint());
            bool             amRoot         = native_l->endpoint() == cmd->cmd.xfer_reduce.root;
            bool             sameNodeAsRoot = false;
            _deviceInfo                     = _geometry->getKey(0,PAMI::Geometry::CKEY_MCOMB_CLASSROUTEID);
            unsigned        typesize;
            coremath        func;
            uintptr_t op, dt;
            PAMI::Type::TypeFunc::GetEnums(cmd->cmd.xfer_reduce.stype,
                                           cmd->cmd.xfer_reduce.op,
                                           dt,op);

            getReduceFunction((pami_dt)dt,(pami_op)op,
                              typesize,
                              func);
            TypeCode       * stype       = (TypeCode *) cmd->cmd.xfer_reduce.stype;
            TypeCode       * rtype       = (TypeCode *) cmd->cmd.xfer_reduce.rtype;
            size_t           sizeOfType  = stype->GetDataSize();
            size_t           sbytes      = cmd->cmd.xfer_reduce.stypecount * sizeOfType;
            size_t           scountDt    = cmd->cmd.xfer_reduce.stypecount;

            // TODO:  fix for multi-ep
            new(&_t_root) PAMI::Topology(cmd->cmd.xfer_reduce.root);
            new(&_t_me)   PAMI::Topology(native_l->endpoint());

            TRACE_FORMAT( "setting up PWQ's %p %p, sbytes=%zd buf=%p:  root=%d, me=%u, amRoot=%d _t_root=%d _t_me=%d _t_root=%p _t_me=%p",
                          &_pwq_src, &_pwq_dest, sbytes, cmd->cmd.xfer_reduce.sndbuf,
                          cmd->cmd.xfer_reduce.root, native_l->endpoint(), amRoot,
                          _t_root.index2Endpoint(0),_t_me.index2Endpoint(0),
                          &_t_root, &_t_me);

            // Create a "flat pwq" for the send buffer
            _pwq_src.configure(cmd->cmd.xfer_reduce.sndbuf,  // buffer
                               sbytes,                       // buffer bytes
                               sbytes,                       // amount initially in buffer
                               stype,
                               rtype);
            _pwq_dest.configure(cmd->cmd.xfer_reduce.rcvbuf, // buffer
                                sbytes,                      // buffer bytes
                                0,                           // amount initially in buffer
                                stype,
                                rtype);
            _user_done.clientdata = cmd->cookie;
            _user_done.function   = cmd->cb_done;

            // The "Only Local" case
            // This means the geometry only contains local tasks
            // A single multicombine will suffice to handle the reduction
            if (t_local->size() == _geometry->size())
              {
                _mcombine_l.cb_done.clientdata   = this;
                _mcombine_l.cb_done.function     = composite_done;
                _mcombine_l.connection_id        = _geometry->comm();
                _mcombine_l.roles                = -1U;
                _mcombine_l.data                 = (pami_pipeworkqueue_t*) & _pwq_src;
                _mcombine_l.data_participants    = (pami_topology_t*)t_local;
                _mcombine_l.results              = (pami_pipeworkqueue_t*) & _pwq_dest;
                _mcombine_l.results_participants = (pami_topology_t*)&_t_root;
                _mcombine_l.optor                = (pami_op)op;
                _mcombine_l.dtype                = (pami_dt)dt;
                _mcombine_l.count                = scountDt;
                _startFcn                        = &MultiCombineComposite2Device::start0;
                _count                           = 1;
                TRACE_FORMAT( "<%p>Local Only MASTER Setting up start0: local native() %p", this, _native_l);
                return;
              }

            // Find the master proxy task
            // TODO:  Fix for endpoints, do not use mapping!
            size_t count, total = t_local->size();
            size_t masterNode=0;
            total  = t_master->size();
            for(count=0; count<total; count++)
            {
              fprintf(stderr, "FATAL!  This code should be fixed when enabled!");
              PAMI_assert(0);
              size_t rank = t_master->index2Endpoint(count);
              if(__global.mapping.isPeer(rank, cmd->cmd.xfer_reduce.root))
              {
                masterNode = rank;
                break;
              }
            }
            PAMI_assert(count < total); // no local master?
            new(&_t_masterproxy) PAMI::Topology(masterNode);
            TRACE_FORMAT( "<%p>Master Proxy is %zd:", this, masterNode);


            // The "Only Global Master" case
            // My task only belongs to a global master, with no other local task
            // A single multicombine will suffice in this case as well
            // To detect this case, the local size will be only me
            // Do not store any results unless I am root
            if (t_local->size() == 1)
              {
                _mcombine_g.cb_done.clientdata   = this;
                _mcombine_g.cb_done.function     = composite_done;
                _mcombine_g.connection_id        = _geometry->comm();
                _mcombine_g.roles                = -1U;
                _mcombine_g.data                 = (pami_pipeworkqueue_t*) & _pwq_src;
                _mcombine_g.data_participants    = (pami_topology_t*)t_master;
                if(amRoot)
                  _mcombine_g.results              = (pami_pipeworkqueue_t*)&_pwq_dest;
                else
                  _mcombine_g.results              = NULL;
                _mcombine_g.results_participants = (pami_topology_t*)&_t_masterproxy;
                _mcombine_g.optor                = (pami_op)op;
                _mcombine_g.dtype                = (pami_dt)dt;
                _mcombine_g.count                = scountDt;
                _startFcn                        = &MultiCombineComposite2Device::start1;
                _count                           = 1;
                TRACE_FORMAT( "<%p>Global Only Setting up start1:", this);
                return;
              }


            // Detect if we are on the same node as the root
            // TODO:  This search is linear and should be optimized
            // for large numbers of local nodes
            total = t_local->size();
            for(count=0; count<total; count++)
            {
              if(t_local->index2Endpoint(count) == cmd->cmd.xfer_reduce.root)
              {
                sameNodeAsRoot = true;
                break;
              }
            }
            // This is the non-master participant in the reduction
            // We have two cases here.  If we are the root task, and not the master,
            // we have to receive the data via a multicast from the global master
            // If we are not the root task, a simple local reduction will suffice

            // This extra PWQ is pointing at the reception buffer.  This means that the
            // reception buffers will be overwritten, maybe more than once
            // \todo Do we need some scratch space if we want to do something like in place?

            if (!amMaster)
              {
                rc = __global.heap_mm->memalign((void **)&_temp_results, 16, sbytes);
                PAMI_assert(rc == PAMI_SUCCESS && _temp_results != NULL);
                PAMI_assert(_temp_results != NULL);   // no local master?
                _pwq_inter0.configure(_temp_results,  // buffer
                                      sbytes,         // buffer bytes
                                      0,              // amount initially in buffer
                                      stype,
                                      rtype);
                _pwq_inter1.configure(_temp_results,  // buffer
                                      sbytes,         // buffer bytes
                                      0,              // amount initially in buffer
                                      stype,
                                      rtype);

                if(sameNodeAsRoot)
                {
                  _mcombine_l.cb_done.clientdata   = this;
                  _mcombine_l.cb_done.function     = composite_done;
                  _mcombine_l.connection_id        = _geometry->comm();
                  _mcombine_l.roles                = -1U;
                  _mcombine_l.data                 = (pami_pipeworkqueue_t*) &_pwq_src;
                  _mcombine_l.data_participants    = (pami_topology_t*)t_local;
                  _mcombine_l.results              = (pami_pipeworkqueue_t*) & _pwq_inter0; // results can go in dest buffer
                  _mcombine_l.results_participants = (pami_topology_t*)t_my_master;
                  _mcombine_l.optor                = (pami_op)op;
                  _mcombine_l.dtype                = (pami_dt)dt;
                  _mcombine_l.count                = scountDt;

                  _mcast_l.cb_done.function        = composite_done;
                  _mcast_l.cb_done.clientdata      = this;
                  _mcast_l.connection_id           = _geometry->comm();
                  _mcast_l.roles                   = -1U;
                  _mcast_l.bytes                   = cmd->cmd.xfer_reduce.stypecount * stype->GetDataSize();
                  _mcast_l.src_participants        = (pami_topology_t*)t_my_master;
                  if(amRoot)
                    _mcast_l.dst                   = (pami_pipeworkqueue_t*)&_pwq_dest;
                  else
                    _mcast_l.dst                   = (pami_pipeworkqueue_t*)&_pwq_inter1;
                  _mcast_l.dst_participants        = (pami_topology_t*)t_local;
                  _mcast_l.msginfo                 = 0;
                  _mcast_l.msgcount                = 0;

                  _startFcn                        = &MultiCombineComposite2Device::start2;
                  _count                           = 2;
                  TRACE_FORMAT( "<%p>Non Master, same node as root (local multicombine+local multicast):start2", this);
                  return;
                }
                else
                {
                  _mcombine_l.cb_done.clientdata   = this;
                  _mcombine_l.cb_done.function     = composite_done;
                  _mcombine_l.connection_id        = _geometry->comm();
                  _mcombine_l.roles                = -1U;
                  _mcombine_l.data                 = (pami_pipeworkqueue_t*) &_pwq_src;
                  _mcombine_l.data_participants    = (pami_topology_t*)t_local;
                  _mcombine_l.results              = (pami_pipeworkqueue_t*) & _pwq_inter0; // results can go in dest buffer
                  _mcombine_l.results_participants = (pami_topology_t*)t_my_master;
                  _mcombine_l.optor                = (pami_op)op;
                  _mcombine_l.dtype                = (pami_dt)dt;
                  _mcombine_l.count                = scountDt;
                  _startFcn                        = &MultiCombineComposite2Device::start0;
                  _count                           = 1;
                  TRACE_FORMAT( "<%p>Non Master, different node than root (local multicombine only):start0", this);
                  return;
                }
              }

            // This task a global master, with local nodes as well
            // In this case, we will be the target of a local reduction, and participate
            // in this reduction.  The results will chain into a global reduction, which
            // will chain into a local mcast:
            // local_mc[local_topo,me]-->global_mc[master topo, master topo]-->local_mcast[me, local_topo]

            // This extra PWQ is pointing at the reception buffer.  This means that the
            // reception buffers will be overwritten, maybe more than once
            // \todo Do we need some scratch space if we want to do something like in place?
            if(!_temp_results)
            {
              rc = __global.heap_mm->memalign((void **)&_temp_results, 16, sbytes);
              PAMI_assert(rc == PAMI_SUCCESS);
              PAMI_assert(_temp_results != NULL);   // no local master?
            }
            _pwq_inter0.configure(_temp_results,  // buffer
                                  sbytes,         // buffer bytes
                                  0,              // amount initially in buffer
                                  stype,
                                  rtype);
            _pwq_inter1.configure(_temp_results,  // buffer
                                  sbytes,         // buffer bytes
                                  0,              // amount initially in buffer
                                  stype,
                                  rtype);

            _mcombine_l.cb_done.clientdata   = this;
            _mcombine_l.cb_done.function     = composite_done;
            _mcombine_l.connection_id        = _geometry->comm();
            _mcombine_l.roles                = -1U;
            _mcombine_l.data                 = (pami_pipeworkqueue_t*) & _pwq_src;
            _mcombine_l.data_participants    = (pami_topology_t*)t_local;
            _mcombine_l.results              = (pami_pipeworkqueue_t*) & _pwq_inter0; // results can go in dest buffer
            _mcombine_l.results_participants = (pami_topology_t*)&_t_me;       // me!
            _mcombine_l.optor                = (pami_op)op;
            _mcombine_l.dtype                = (pami_dt)dt;
            _mcombine_l.count                = scountDt;

            _mcombine_g.cb_done.clientdata   = this;
            _mcombine_g.cb_done.function     = composite_done;
            _mcombine_g.connection_id        = _geometry->comm();
            _mcombine_g.roles                = -1U;
            _mcombine_g.data                 = (pami_pipeworkqueue_t*) & _pwq_inter0;
            _mcombine_g.data_participants    = (pami_topology_t*)t_master;
            if(amRoot)
              _mcombine_g.results              = (pami_pipeworkqueue_t*) & _pwq_dest;
            else
              _mcombine_g.results              = (pami_pipeworkqueue_t*) & _pwq_inter1;
            _mcombine_g.results_participants = (pami_topology_t*)&_t_masterproxy;
            _mcombine_g.optor                = (pami_op)op;
            _mcombine_g.dtype                = (pami_dt)dt;
            _mcombine_g.count                = scountDt;

            if(sameNodeAsRoot) // Am global master am on same node as root
            {
              _pwq_inter2.configure(_temp_results,  // buffer
                                    sbytes,         // buffer bytes
                                    0,              // amount initially in buffer
                                    stype,
                                    rtype);
              _mcast_l.cb_done.function        = composite_done;
              _mcast_l.cb_done.clientdata      = this;
              _mcast_l.connection_id           = _geometry->comm();
              _mcast_l.roles                   = -1U;
              _mcast_l.bytes                   = cmd->cmd.xfer_reduce.stypecount * stype->GetDataSize();
              _mcast_l.src                     = (pami_pipeworkqueue_t*) & _pwq_inter1;
              _mcast_l.src_participants        = (pami_topology_t*)t_my_master;  // me!
              _mcast_l.dst                     = (pami_pipeworkqueue_t*) & _pwq_inter2;
              _mcast_l.dst_participants        = (pami_topology_t*)t_local;
              _mcast_l.msginfo                 = 0;
              _mcast_l.msgcount                = 0;
              _count                           = 3;
              _startFcn                        = &MultiCombineComposite2Device::start3;
              TRACE_FORMAT( "<%p>Master(local multicombine + global multicombine + local multicast) Setting up start3:", this);
              return;
            }
            else // Am global master, am NOT root, different node than root
            {
              _count                           = 2;
              _startFcn                        = &MultiCombineComposite2Device::start4;
              TRACE_FORMAT( "<%p>Master(local multicombine and global multicombine):Setting up start4:", this);
              return;
            }
            PAMI_abort();
            TRACE_FN_EXIT();
            return;
          }

          MultiCombineComposite2Device (Interfaces::NativeInterface      *native_l,
                                        Interfaces::NativeInterface      *native_g,
                                        ConnectionManager::SimpleConnMgr *cmgr,
                                        pami_geometry_t                   g,
                                        pami_xfer_t                      *cmd,
                                        pami_event_function               fn,
                                        void                             *cookie) :
              Composite(),
              _native_l(native_l),
              _native_g(native_g),
              _geometry((PAMI_GEOMETRY_CLASS*)g),
              _temp_results(NULL),
              _throwaway_results(NULL),
              _fn(fn),
              _cookie(cookie)
          {
            // This should not generate a branch
            // ReduceOnly is a template parameter
            if(ReduceOnly==1)
              setupReduce(native_l, native_g, cmgr, g, cmd, fn, cookie);
            else if(ReduceOnly==2)
              setupReduceCommon<pami_reduce_t> (native_l, native_g, cmgr, g, cmd,
                                                fn, cookie,cmd->cmd.xfer_reduce.root);
            else
              setupReduceCommon<pami_allreduce_t> (native_l, native_g, cmgr, g, cmd,
                                                   fn, cookie,0xFFFFFFFF);
          }


          void start0()
          {
            TRACE_FN_ENTER();
            TRACE_FORMAT( "<%p>local native multicombine %p", this, _native_l);
            _native_l->multicombine(&_mcombine_l, _deviceInfo);
            TRACE_FN_EXIT();
          }
          void start1()
          {
            TRACE_FN_ENTER();
            TRACE_FORMAT( "<%p>global native multicombine %p", this, _native_g);
            _native_g->multicombine(&_mcombine_g, _deviceInfo);
            TRACE_FN_EXIT();
          }
          void start2()
          {
            TRACE_FN_ENTER();
            TRACE_FORMAT( "<%p>local mcast+local multicombine %p", this, _native_l);
            _native_l->multicombine(&_mcombine_l, _deviceInfo);
            _native_l->multicast(&_mcast_l, _deviceInfo);
            TRACE_FN_EXIT();
          }
          void start3()
          {
            TRACE_FN_ENTER();
            TRACE_FORMAT( "<%p>local mcast+local multicombine+global multicombine l=%p g=%p"
                   , this, _native_l, _native_g);
            _native_l->multicombine(&_mcombine_l, _deviceInfo);
            _native_g->multicombine(&_mcombine_g, _deviceInfo);
            _native_l->multicast(&_mcast_l, _deviceInfo);
            TRACE_FN_EXIT();
          }

        void start4()
          {
            TRACE_FN_ENTER();
            TRACE_FORMAT( "<%p>local multicombine+global multicombine l=%p g=%p"
                   , this, _native_l, _native_g);
            _native_l->multicombine(&_mcombine_l, _deviceInfo);
            _native_g->multicombine(&_mcombine_g, _deviceInfo);
            TRACE_FN_EXIT();
          }

          virtual void start()
          {
            TRACE_FN_ENTER();
            TRACE_FORMAT( "<%p>", this);
            (this->*_startFcn)();
            TRACE_FN_EXIT();
          }
          void (MultiCombineComposite2Device::*_startFcn)();
          Interfaces::NativeInterface         *_native_l;
          Interfaces::NativeInterface         *_native_g;
          PAMI_GEOMETRY_CLASS                 *_geometry;
          void                                *_deviceInfo;
          PAMI::Topology                      *_l_topology;
          PAMI::Topology                      *_g_topology;
          pami_multicombine_t                  _mcombine_l;
          pami_multicombine_t                  _mcombine_g;
          pami_multicast_t                     _mcast_l;
          PAMI::PipeWorkQueue                  _pwq_src;
          PAMI::PipeWorkQueue                  _pwq_dest;
          PAMI::PipeWorkQueue                  _pwq_inter0;
          PAMI::PipeWorkQueue                  _pwq_inter1;
          PAMI::PipeWorkQueue                  _pwq_inter2;
          pami_callback_t                      _user_done;
          size_t                               _count;
          pami_endpoint_t                      _root_ep;
          pami_endpoint_t                      _me_ep;
          PAMI::Topology                       _t_root;
          PAMI::Topology                       _t_me;
          PAMI::Topology                       _t_masterproxy;
          char                                *_temp_results;
          char                                *_throwaway_results;
          pami_event_function                  _fn;
          void                                *_cookie;
      };


      /// \brief All sided allreduce over all sided multicombines/multicasts
      /// A local device multicombine will chain into a global multicombine which
      /// will chain into a local multicast
      ///
      /// No real 'pipelining' is done - it waits until each step
      /// completes before starting the next step.

      class MultiCombineComposite2DeviceNP : public CCMI::Executor::Composite
      {

          static void local_master_done_fn(pami_context_t  context,
                                           void           *cookie,
                                           pami_result_t   result )
          {
            (void)context;(void)result;
            TRACE_FN_ENTER();
            MultiCombineComposite2DeviceNP *m = (MultiCombineComposite2DeviceNP*) cookie;
            TRACE_FORMAT( "<%p>bytes produced:%zd buffer:%p",
                         cookie, m->_pwq_temp.bytesAvailableToConsume(),
                         m->_pwq_temp.bufferToConsume());
            DO_DEBUG(dumpDbuf((double*)m->_pwq_temp.bufferToConsume() , m->_pwq_temp.bytesAvailableToConsume() / sizeof(double)));
            m->_native_g->multicombine(&m->_mcomb_g, m->_deviceMcombInfo);
            TRACE_FN_EXIT();
          }

          static void local_done_fn(pami_context_t  context,
                                    void           *cookie,
                                    pami_result_t   result )
          {
            (void)context;(void)result;
            TRACE_FN_ENTER();
            TRACE_FORMAT( "<%p>", cookie);
            MultiCombineComposite2DeviceNP *m = (MultiCombineComposite2DeviceNP*) cookie;
            m->_native_l->multicast(&m->_mcast_l, m->_deviceMcastInfo);
            TRACE_FN_EXIT();
          }

          static void global_done_fn(pami_context_t  context,
                                     void           *cookie,
                                     pami_result_t   result )
          {
            (void)context;(void)result;
            TRACE_FN_ENTER();
            MultiCombineComposite2DeviceNP *m = (MultiCombineComposite2DeviceNP*) cookie;
            TRACE_FORMAT( "<%p>bytes produced:%zd buffer:%p",
                         cookie, m->_pwq_dst.bytesAvailableToConsume(),
                         m->_pwq_dst.bufferToConsume());
            DO_DEBUG(dumpDbuf((double*)m->_pwq_dst.bufferToConsume() , m->_pwq_dst.bytesAvailableToConsume() / sizeof(double)));
            m->_native_l->multicast(&m->_mcast_l, m->_deviceMcastInfo);
            TRACE_FN_EXIT();
          }

#ifdef LOCAL_TEST
          static void dumpDbuf(double* dbuf, size_t count)
          {
            TRACE_FORMAT("dbuf=%p, size %zu", dbuf, count);

            for (size_t i = 0; i < count; i++)
              TRACE_FORMAT("dbuf[%zu]=%f", i, dbuf[i]);
          }

          static void test_local_done_fn(pami_context_t  context,
                                         void           *cookie,
                                         pami_result_t   result )
          {
            TRACE_FN_ENTER();
            MultiCombineComposite2DeviceNP *m = (MultiCombineComposite2DeviceNP*) cookie;
            TRACE_FORMAT( "<%p>MultiCombineComposite2DeviceNP::test_local_done_fn() ", cookie);
            DO_DEBUG(dumpDbuf((double*)m->_pwq_dst.bufferToConsume() , m->_pwq_dst.bytesAvailableToConsume() / sizeof(double)));

            if (m->_cb_done.function)
                   m->_cb_done.function(context, m->_cb_done.clientdata, result);
            TRACE_FN_EXIT();
          }
#else
          static void dumpDbuf(double* dbuf,  unsigned count) {(void)dbuf;(void)count;}
#endif

        public:
          ~MultiCombineComposite2DeviceNP ()
          {
            TRACE_FN_ENTER();
            TRACE_FN_EXIT();
          }
          MultiCombineComposite2DeviceNP (Interfaces::NativeInterface      *mInterface,
                                          ConnectionManager::SimpleConnMgr *cmgr,
                                          pami_geometry_t                   g,
                                          pami_xfer_t                      *cmd,
                                          pami_event_function               fn,
                                          void                             *cookie) :
              Composite(),
              // This is a small hack to get around making a whole new set of factories
              // and classes for a 2 device NI.  We just treat the first parameter like a
              // 2 element pinter array.
              _native_l((Interfaces::NativeInterface*)((void **)mInterface)[0]),
              _native_g((Interfaces::NativeInterface*)((void **)mInterface)[1]),
              _geometry((PAMI_GEOMETRY_CLASS*)g),
              _amMaster(false),
              _pwq_src(),
              _pwq_dst(),
              _pwq_temp(),
              _topology_l((PAMI::Topology*)_geometry->getTopology(PAMI::Geometry::LOCAL_TOPOLOGY_INDEX)),
              _topology_lm((PAMI::Topology*)_geometry->getTopology(PAMI::Geometry::LOCAL_MASTER_TOPOLOGY_INDEX)),
              _topology_g((PAMI::Topology*)_geometry->getTopology(PAMI::Geometry::MASTER_TOPOLOGY_INDEX)),
              _deviceMcombInfo(NULL),
              _deviceMcastInfo(NULL)
          {
            (void)cmgr;
            TRACE_FN_ENTER();
            uintptr_t op, dt;
            PAMI::Type::TypeFunc::GetEnums(cmd->cmd.xfer_allreduce.stype,cmd->cmd.xfer_allreduce.op,
                                           dt,op);

            TRACE_FORMAT( "<%p>type %#zX/%#zX, count %zu/%zu, op %lu, dt %lu",
                         this, (size_t)cmd->cmd.xfer_allreduce.stype, (size_t)cmd->cmd.xfer_allreduce.rtype,
                          cmd->cmd.xfer_allreduce.stypecount, cmd->cmd.xfer_allreduce.rtypecount,
                          (unsigned long)op, (unsigned long)dt);

            DO_DEBUG(for (unsigned j = 0; j < _topology_l->size(); ++j) TRACE_FORMAT("_topology_l[%u]=%zu, size %zu", j, (size_t)_topology_l->index2Endpoint(j), _topology_l->size()));

            DO_DEBUG(for (unsigned j = 0; j < _topology_g->size(); ++j) TRACE_FORMAT("_topology_g[%u]=%zu, size %zu", j, (size_t)_topology_g->index2Endpoint(j), _topology_g->size()));

            DO_DEBUG(for (unsigned j = 0; j < _topology_lm.size(); ++j) TRACE_FORMAT("_topology_lm[%u]=%zu, size %zu", j, (size_t)_topology_lm.index2Rank(j), _topology_lm.size()));

            PAMI_assert(_topology_lm.index2Endpoint(0) != (unsigned) - 1); // no local master?

            TypeCode * stype_obj = (TypeCode *)cmd->cmd.xfer_allreduce.stype;
            TypeCode * rtype_obj = (TypeCode *)cmd->cmd.xfer_allreduce.rtype;

            /// \todo Support non-contiguous
            PAMI_assert(stype_obj->IsContiguous() &&  stype_obj->IsPrimitive());

            unsigned        sizeOfType   = stype_obj->GetDataSize();

            _bytes                       = cmd->cmd.xfer_allreduce.stypecount * sizeOfType;
            size_t          countDt      = cmd->cmd.xfer_allreduce.stypecount;

            // Discover the root node and intereesting topology information
            size_t           numMasters  = _topology_g->size();
            size_t           numLocals   = _topology_l->size();
            PAMI_assertf(numMasters > 1 && numLocals > 1, "numMasters %zu, numLocals %zu\n", numMasters, numLocals); /// \todo keep it simple for now.
            numLocals = numLocals;
            numMasters = numMasters; // silence warnings when ASSERTS=0

            _deviceMcombInfo = _geometry->getKey(0,PAMI::Geometry::CKEY_MCOMB_CLASSROUTEID);
            _deviceMcastInfo = _geometry->getKey(0,PAMI::Geometry::CKEY_MCAST_CLASSROUTEID);

            _pwq_src.configure(
                               cmd->cmd.xfer_allreduce.sndbuf,  // buffer
                               _bytes,                          // buffer bytes
                               _bytes,                          // amount initially in buffer
                               stype_obj,
                               rtype_obj);
            TRACE_FORMAT( "<%p>send buffer %p, bytes available to consume:%zd, produce:%zd",
                         this, _pwq_src.bufferToConsume(),
                           _pwq_src.bytesAvailableToConsume(), _pwq_src.bytesAvailableToProduce());
            DO_DEBUG(dumpDbuf((double*)_pwq_src.bufferToConsume() , _pwq_src.bytesAvailableToConsume() / sizeof(double)));


            /// \todo Support non-contiguous
            PAMI_assert(rtype_obj->IsContiguous() &&  rtype_obj->IsPrimitive());

            sizeOfType = rtype_obj->GetDataSize();

            PAMI_assert(_bytes <= cmd->cmd.xfer_allreduce.rtypecount * sizeOfType);

            _pwq_dst.configure(
                               cmd->cmd.xfer_allreduce.rcvbuf,  // buffer
                               _bytes,                          // buffer bytes
                               0,                               // amount initially in buffer
                               stype_obj,
                               rtype_obj);
            TRACE_FORMAT( "<%p>receive buffer %p, bytes available to consume:%zd, produce:%zd",
                          this, _pwq_dst.bufferToProduce(),
                           _pwq_dst.bytesAvailableToConsume(), _pwq_dst.bytesAvailableToProduce());

            // This extra PWQ is pointing at the reception buffer.  This means that the
            // reception buffers will be overwritten, maybe more than once
            // \todo Do we need some scratch space if we want to do something like in place?
            _pwq_temp.configure(
                                cmd->cmd.xfer_allreduce.rcvbuf,  // buffer
                                _bytes,                          // buffer bytes
                                0,                               // amount initially in buffer
                                stype_obj,
                                rtype_obj);

            _amMaster       = _topology_lm.isEndpointMember(_native_l->endpoint());
            _mcomb_l.connection_id        = 0;
            _mcomb_l.roles                = -1U;
            _mcomb_l.data_participants    = (pami_topology_t*)_topology_l;
            _mcomb_l.results_participants = (pami_topology_t*) & _topology_lm;
            _mcomb_l.data                 = (pami_pipeworkqueue_t*) & _pwq_src;
            _mcomb_l.results              = (pami_pipeworkqueue_t*) & _pwq_temp;
            _mcomb_l.cb_done.function     =  _amMaster ? local_master_done_fn : local_done_fn;
            _mcomb_l.cb_done.clientdata   =  this;
            _mcomb_l.count                = countDt;
            _mcomb_l.dtype                = (pami_dt)dt;
            _mcomb_l.optor                = (pami_op)op;

            _mcomb_g.connection_id        = 0;
            _mcomb_g.roles                = -1U;
            _mcomb_g.data_participants    = (pami_topology_t*)_topology_g;
            _mcomb_g.results_participants = (pami_topology_t*)_topology_g;
            _mcomb_g.data                 = (pami_pipeworkqueue_t*) & _pwq_temp;
            _mcomb_g.results              = (pami_pipeworkqueue_t*) & _pwq_dst;
            _mcomb_g.cb_done.function     = global_done_fn;
            _mcomb_g.cb_done.clientdata   = this;
            _mcomb_g.count                = countDt;
            _mcomb_g.dtype                = (pami_dt)dt;
            _mcomb_g.optor                = (pami_op)op;

            _mcast_l.connection_id        = 0;
            _mcast_l.roles                = -1U;
            _mcast_l.src_participants     = (pami_topology_t*) & _topology_lm;
            _mcast_l.dst_participants     = (pami_topology_t*)_topology_l;
            _mcast_l.src                  = (pami_pipeworkqueue_t*) & _pwq_dst;
            _mcast_l.dst                  = (pami_pipeworkqueue_t*) & _pwq_temp;
#ifdef LOCAL_TEST
            // Intercept the user's done for testing purposes.
            _cb_done.function   = fn;
            _cb_done.clientdata = cookie;
            _mcast_l.cb_done.function     =  test_local_done_fn;
            _mcast_l.cb_done.clientdata   =  this;
#else
            _mcast_l.cb_done.function     =  fn;
            _mcast_l.cb_done.clientdata   =  cookie;
#endif
            _mcast_l.bytes                = _bytes;
            _mcast_l.dispatch             = -1; /// \todo assuming all-sided?
            _mcast_l.msgcount             = 0;
            _mcast_l.msginfo              = NULL;

            TRACE_FORMAT("client data:%p", cookie);

            TRACE_FN_EXIT();
          }
          virtual void start()
          {
            TRACE_FN_ENTER();
            TRACE_FORMAT( "<%p>", this);
            _native_l->multicombine(&_mcomb_l, _deviceMcombInfo);
            TRACE_FN_EXIT();
          }
        protected:
          Interfaces::NativeInterface        *_native_l;
          Interfaces::NativeInterface        *_native_g;
          Interfaces::NativeInterface        *_active_native;
          PAMI_GEOMETRY_CLASS                *_geometry;
          bool                                _amMaster;
          PAMI::PipeWorkQueue                 _pwq_src;
          PAMI::PipeWorkQueue                 _pwq_dst;
          PAMI::PipeWorkQueue                 _pwq_temp;
          PAMI::Topology                     *_topology_l;
          PAMI::Topology                      _topology_lm;
          PAMI::Topology                     *_topology_g;
          void                               *_deviceMcombInfo;
          void                               *_deviceMcastInfo;
          pami_multicombine_t                 _mcomb_l;
          pami_multicombine_t                 _mcomb_g;
          pami_multicast_t                    _mcast_l;
          pami_multicombine_t                *_active_mcomb;
          size_t                              _bytes;
#ifdef LOCAL_TEST
          pami_callback_t                     _cb_done;   // User's completion callback
#endif
      };
    };
  };
};

#undef DO_TRACE_ENTEREXIT
#undef DO_TRACE_DEBUG

#endif
