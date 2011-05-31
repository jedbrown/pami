/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/onetask/OneTaskT.h
 * \brief ???
 */

#ifndef __algorithms_protocols_onetask_OneTaskT_h__
#define __algorithms_protocols_onetask_OneTaskT_h__

#include "common/type/TypeMachine.h"
#include "algorithms/connmgr/SimpleConnMgr.h"
#include "algorithms/protocols/CollectiveProtocolFactoryT.h"
#include "algorithms/executor/Barrier.h"



#define CHECK_ROOT    {                      \
   pami_task_t me = (pami_task_t)xfer->root; \
   if( me != _geometry->rank())              \
     return PAMI_ERROR;                      \
}


namespace CCMI
{
namespace Adaptor
{
namespace OneTask
{

typedef PAMI::Type::TypeCode    TypeCode;
typedef PAMI::Type::TypeMachine TypeMachine;

// OneTask Factory for generate routine
// generate
//
template < class T_Composite, MetaDataFn get_metadata, class T_Conn >
class OneTaskFactoryT : public CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>
{
public:
    OneTaskFactoryT(T_Conn                      *cmgr,
                    Interfaces::NativeInterface *native,
                    pami_dispatch_multicast_function cb_head = NULL):
        CollectiveProtocolFactoryT<T_Composite, get_metadata, T_Conn>(cmgr, native, cb_head)
    {
        TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));
    }

};




//Based on PAMI_Type_transform_data in pami_type.cc
inline pami_result_t copyData (void               * src_addr,
                               TypeCode           * src_type_obj,
                               void               * dst_addr,
                               TypeCode           * dst_type_obj,
                               size_t               dst_type_count,
                               size_t               dst_displs,
                               size_t               src_displs)
{
    size_t size = dst_type_obj->GetDataSize() * dst_type_count;
    if (!src_type_obj->IsCompleted() || !dst_type_obj->IsCompleted()) {
        RETURN_ERR_PAMI(PAMI_INVAL, "Using incomplete type.\n");
    }

    if (src_type_obj->IsContiguous()) {
        // unpacking: contiguous to non-contiguous (or contiguous)
        TypeMachine unpacker(dst_type_obj);
        unpacker.SetCopyFunc(PAMI_DATA_COPY, NULL);
        unpacker.MoveCursor(dst_displs);
        unpacker.Unpack(dst_addr, (char*)src_addr + src_displs, size);

    } else if (dst_type_obj->IsContiguous()) {
        // packing: non-contiguous to contiguous
        TypeMachine packer(src_type_obj);
        packer.SetCopyFunc(PAMI_DATA_COPY, NULL);
        packer.MoveCursor(src_displs);
        packer.Pack((char*)dst_addr + dst_displs, src_addr, size);

    } else {
        // generic: non-contiguous to non-contiguous
        TypeMachine packer(src_type_obj);
        packer.MoveCursor(src_displs);

        TypeMachine unpacker(dst_type_obj);
        unpacker.SetCopyFunc(PAMI_DATA_COPY, NULL);
        unpacker.MoveCursor(dst_displs);

        // use a temporary buffer to copy in and out data
        const size_t TMP_BUF_SIZE = 8192;
        char tmp_buf[TMP_BUF_SIZE];

        for (size_t offset = 0; offset < size; offset += TMP_BUF_SIZE) {
            size_t bytes_to_copy = std::min(size - offset, TMP_BUF_SIZE);
            packer.Pack(tmp_buf, (char *)src_addr + offset, bytes_to_copy);
            unpacker.Unpack((char *)dst_addr + offset, tmp_buf, bytes_to_copy);
        }
    }

    return PAMI_SUCCESS;
}




template <class T_Collective_type>
inline pami_result_t doAction(T_Collective_type *xfer, PAMI_GEOMETRY_CLASS *_geometry)
{
    COMPILE_TIME_ASSERT(0 == 1);
    return PAMI_SUCCESS;
}

template <>
inline pami_result_t doAction<pami_barrier_t>(pami_barrier_t *xfer, PAMI_GEOMETRY_CLASS *_geometry)
{
   //no op
   return PAMI_SUCCESS;
}

//template <>
//inline pami_result_t doAction<pami_fence_t>(pami_fence_t *xfer, PAMI_GEOMETRY_CLASS *_geometry)
//{
   //no op
//   return PAMI_SUCCESS;
//}

template <>
inline pami_result_t doAction<pami_allreduce_t>(pami_allreduce_t *xfer, PAMI_GEOMETRY_CLASS *_geometry)
{
   return copyData((void*)xfer->sndbuf, (TypeCode*)xfer->stype, (void*)xfer->rcvbuf, (TypeCode*)xfer->rtype, xfer->rtypecount, 0, 0);
}

template <>
inline pami_result_t doAction<pami_broadcast_t>(pami_broadcast_t *xfer, PAMI_GEOMETRY_CLASS *_geometry)
{
   CHECK_ROOT
   return PAMI_SUCCESS;
}

template <>
inline pami_result_t doAction<pami_reduce_t>(pami_reduce_t *xfer, PAMI_GEOMETRY_CLASS *_geometry)
{
   CHECK_ROOT
   return copyData((void*)xfer->sndbuf, (TypeCode*)xfer->stype, (void*)xfer->rcvbuf, (TypeCode*)xfer->rtype, xfer->rtypecount, 0, 0);
}

template <>
inline pami_result_t doAction<pami_allgather_t>(pami_allgather_t *xfer, PAMI_GEOMETRY_CLASS *_geometry)
{
   return copyData((void*)xfer->sndbuf, (TypeCode*)xfer->stype, (void*)xfer->rcvbuf, (TypeCode*)xfer->rtype, xfer->rtypecount, 0, 0);
}

template <>
inline pami_result_t doAction<pami_allgatherv_t>(pami_allgatherv_t *xfer, PAMI_GEOMETRY_CLASS *_geometry)
{
   if(xfer->rtypecounts == NULL || xfer->rdispls == NULL)
     return PAMI_ERROR;
   return copyData((void*)xfer->sndbuf, (TypeCode*)xfer->stype, (void*)xfer->rcvbuf, (TypeCode*)xfer->rtype, xfer->rtypecounts[0], xfer->rdispls[0], 0);
}

template <>
inline pami_result_t doAction<pami_allgatherv_int_t>(pami_allgatherv_int_t *xfer, PAMI_GEOMETRY_CLASS *_geometry)
{
   if(xfer->rtypecounts == NULL || xfer->rdispls == NULL)
     return PAMI_ERROR;
   return copyData((void*)xfer->sndbuf, (TypeCode*)xfer->stype, (void*)xfer->rcvbuf, (TypeCode*)xfer->rtype, xfer->rtypecounts[0], xfer->rdispls[0], 0);
}

template <>
inline pami_result_t doAction<pami_scatter_t>(pami_scatter_t *xfer, PAMI_GEOMETRY_CLASS *_geometry)
{
   CHECK_ROOT
   return copyData((void*)xfer->sndbuf, (TypeCode*)xfer->stype, (void*)xfer->rcvbuf, (TypeCode*)xfer->rtype, xfer->rtypecount, 0, 0);
}

template <>
inline pami_result_t doAction<pami_scatterv_t>(pami_scatterv_t *xfer, PAMI_GEOMETRY_CLASS *_geometry)
{
   CHECK_ROOT
   if(xfer->stypecounts == NULL || xfer->sdispls == NULL)
     return PAMI_ERROR;
   return copyData((void*)xfer->sndbuf, (TypeCode*)xfer->stype, (void*)xfer->rcvbuf, (TypeCode*)xfer->rtype, xfer->rtypecount, 0, xfer->sdispls[0]);
}

template <>
inline pami_result_t doAction<pami_scatterv_int_t>(pami_scatterv_int_t *xfer, PAMI_GEOMETRY_CLASS *_geometry)
{
   CHECK_ROOT
   if(xfer->stypecounts == NULL || xfer->sdispls == NULL)
     return PAMI_ERROR;
   return copyData((void*)xfer->sndbuf, (TypeCode*)xfer->stype, (void*)xfer->rcvbuf, (TypeCode*)xfer->rtype, xfer->rtypecount, 0, xfer->sdispls[0]);
}

template <>
inline pami_result_t doAction<pami_gather_t>(pami_gather_t *xfer, PAMI_GEOMETRY_CLASS *_geometry)
{
   CHECK_ROOT
   return copyData((void*)xfer->sndbuf, (TypeCode*)xfer->stype, (void*)xfer->rcvbuf, (TypeCode*)xfer->rtype, xfer->rtypecount, 0, 0);
}

template <>
inline pami_result_t doAction<pami_gatherv_t>(pami_gatherv_t *xfer, PAMI_GEOMETRY_CLASS *_geometry)
{
   CHECK_ROOT
   if(xfer->rtypecounts == NULL || xfer->rdispls == NULL)
     return PAMI_ERROR;
   return copyData((void*)xfer->sndbuf, (TypeCode*)xfer->stype, (void*)xfer->rcvbuf, (TypeCode*)xfer->rtype, xfer->rtypecounts[0], xfer->rdispls[0], 0);
}

template <>
inline pami_result_t doAction<pami_gatherv_int_t>(pami_gatherv_int_t *xfer, PAMI_GEOMETRY_CLASS *_geometry)
{
   CHECK_ROOT
   if(xfer->rtypecounts == NULL || xfer->rdispls == NULL)
     return PAMI_ERROR;
   return copyData((void*)xfer->sndbuf, (TypeCode*)xfer->stype, (void*)xfer->rcvbuf, (TypeCode*)xfer->rtype, xfer->rtypecounts[0], xfer->rdispls[0], 0);
}

template <>
inline pami_result_t doAction<pami_alltoall_t>(pami_alltoall_t *xfer, PAMI_GEOMETRY_CLASS *_geometry)
{
   return copyData((void*)xfer->sndbuf, (TypeCode*)xfer->stype, (void*)xfer->rcvbuf, (TypeCode*)xfer->rtype, xfer->rtypecount, 0, 0);
}

template <>
inline pami_result_t doAction<pami_alltoallv_t>(pami_alltoallv_t *xfer, PAMI_GEOMETRY_CLASS *_geometry)
{
   if(xfer->rtypecounts == NULL || xfer->rdispls == NULL || xfer->stypecounts == NULL || xfer->sdispls == NULL)
     return PAMI_ERROR;
   return copyData((void*)xfer->sndbuf, (TypeCode*)xfer->stype, (void*)xfer->rcvbuf, (TypeCode*)xfer->rtype, xfer->rtypecounts[0], xfer->rdispls[0], xfer->sdispls[0]);
}

template <>
inline pami_result_t doAction<pami_alltoallv_int_t>(pami_alltoallv_int_t *xfer, PAMI_GEOMETRY_CLASS *_geometry)
{
   if(xfer->rtypecounts == NULL || xfer->rdispls == NULL || xfer->stypecounts == NULL || xfer->sdispls == NULL)
     return PAMI_ERROR;
   return copyData((void*)xfer->sndbuf, (TypeCode*)xfer->stype, (void*)xfer->rcvbuf, (TypeCode*)xfer->rtype, xfer->rtypecounts[0], xfer->rdispls[0], xfer->sdispls[0]);
}

template <>
inline pami_result_t doAction<pami_scan_t>(pami_scan_t *xfer, PAMI_GEOMETRY_CLASS *_geometry)
{
   return copyData((void*)xfer->sndbuf, (TypeCode*)xfer->stype, (void*)xfer->rcvbuf, (TypeCode*)xfer->rtype, xfer->rtypecount, 0, 0);
}

template <>
inline pami_result_t doAction<pami_reduce_scatter_t>(pami_reduce_scatter_t *xfer, PAMI_GEOMETRY_CLASS *_geometry)
{
   return copyData((void*)xfer->sndbuf, (TypeCode*)xfer->stype, (void*)xfer->rcvbuf, (TypeCode*)xfer->rtype, xfer->rcounts[0], 0, 0);
}


///
/// \brief OneTaskT class
///
template < typename T_Collective_type >
class OneTaskT : public CCMI::Executor::Composite
{
public:

    ///
    /// \brief Constructor for OneTaskT protocol.
    ///
    /// \param[in] geometry    Geometry object
    ///
    OneTaskT ( Interfaces::NativeInterface          * mInterface,
               ConnectionManager::SimpleConnMgr     * cmgr,
               pami_geometry_t                         geometry,
               void                                 * cmd,
               pami_event_function                     fn,
               void                                 * cookie)
    {
        TRACE_INIT((stderr, "<%p>CCMI::Adaptors::OneTask::OneTaskT::ctor()\n",
                    this));//, geometry->comm()));

        CCMI_assert( mInterface == NULL );
        CCMI_assert( cmgr       == NULL );
        pami_xfer_t *xfer = (pami_xfer_t*)cmd;
        _geometry = (PAMI_GEOMETRY_CLASS*)geometry;
        _cmd      = *xfer;
        _fn       = fn;
        _cookie   = cookie;
        _res      = PAMI_SUCCESS;

        setDoneCallback(xfer->cb_done, xfer->cookie);

    }


    virtual void start()
    {
        TRACE_ADAPTOR((stderr, "%s\n", __PRETTY_FUNCTION__));
        T_Collective_type *coll_xfer = (T_Collective_type*)&_cmd.cmd;

        _res = doAction<T_Collective_type>(coll_xfer, _geometry);

        _fn( NULL, _cookie, _res);
    }



protected:

    PAMI_GEOMETRY_CLASS *_geometry;
    pami_xfer_t          _cmd;
    pami_event_function  _fn;
    void                *_cookie;
    pami_result_t        _res;

}; //-OneTaskT

//////////////////////////////////////////////////////////////////////////////
};
};
};  //namespace CCMI::Adaptor::OneTask

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
