/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file interface/MultiSend.h
 * \brief Structures and abstract multisend classes which
 * must be derived in the adaptor.
 */

#ifndef   __kernel_multisend_h__
#define   __kernel_multisend_h__

#include "interface/ccmi_internal.h"
#include "util/ccmi_util.h"
#include "util/ccmi_debug.h"
#include "ll_topology.h" 
#include "ll_pipeworkqueue.h" 

namespace CCMI
{
  namespace MultiSend
  {

    ///
    /// \brief Callback function for unexpected async broadcasts
    ///


    #ifndef __ccmi_recv_multicast_callback_defined__ // may be pre-defined in the adaptor
#error no RecvMulticast defined
    #endif

    struct CCMI_Multicast_t
    {
      // This MUST be kept identical to LL_Multicast_t!
      void              * registration;
      CM_Request_t    * request;
      size_t              req_size;
      CM_Callback_t     cb_done;
      unsigned            connection_id;
      unsigned            roles;
      size_t              bytes;
      LL_PipeWorkQueue_t *src;
      LL_Topology_t      *src_participants;
      LL_PipeWorkQueue_t *dst;
      LL_Topology_t      *dst_participants;
      const CMQuad    * msginfo;
      unsigned            count;

      CCMI_Multicast_t ()
      {
        registration   = NULL;
        request        = NULL;
        req_size = 0;
        connection_id  = ((unsigned) -1);
        roles          = 0;
        bytes          = 0;
        src            = NULL;
        src_participants= NULL;
        dst            = NULL;
        dst_participants= NULL;
        msginfo        = NULL;
        count          = 0;
      }

      void setRequestBuffer (CM_Request_t *request, size_t req_size)
      {
        this->request = request;
        this->req_size = req_size;
      }

      void setConnectionId (unsigned conn)
      {
        this->connection_id = conn;
      }

      void setRoles (unsigned roles)
      {
        this->roles = roles;
      }

      void setSendData (LL_PipeWorkQueue_t *src, size_t bytes)
      {
        this->src = src;
        this->bytes = bytes;
      }

      void setSendRanks (LL_Topology_t *src_participants)
      {
        this->src_participants = src_participants;
      }

      void setRecvData (LL_PipeWorkQueue_t *dst, size_t bytes)
      {
        this->dst = dst;
        this->bytes = bytes;
      }

      void setRecvRanks (LL_Topology_t *dst_participants)
      {
        this->dst_participants = dst_participants;
      }

      void setCallback (void (*fn) (void *, CM_Error_t *),  void *cd)
      {
        cb_done.function = fn;
        cb_done.clientdata = cd;
      }

      void setInfo (CMQuad *info, int count)
      {
        this->msginfo = info;
        this->count   = count;
      }

    }; // struct CCMI_Multicast_t

    // This MUST be kept identical to LL_Multicombine_t!
    struct CCMI_Multicombine_t
    {
      void              * registration;
      CM_Request_t    * request;
      size_t              req_size;
      CM_Callback_t     cb_done;
      unsigned            connection_id;
      unsigned            roles;
      LL_PipeWorkQueue_t *data;
      LL_Topology_t      *data_participants;
      LL_PipeWorkQueue_t *results;
      LL_Topology_t      *results_participants;
      CM_Op             op;
      CM_Dt             dt;
      size_t              count;
#ifdef NOT_YET /* These are only needed if we support one-sided multicombine directly */
      const CMQuad    * msginfo;
      unsigned            count;
#endif

      CCMI_Multicombine_t ()
      {
        registration = NULL;
        request = NULL;
        req_size = 0;
        connection_id = ((unsigned) -1);
        roles = 0;
        data = NULL;
        data_participants = NULL;
        results = NULL;
        results_participants = NULL;
        op = CM_UNDEFINED_OP;
        dt = CM_UNDEFINED_DT;
        count = 0;
#ifdef NOT_YET /* These are only needed if we support one-sided multicombine directly */
        msginfo = NULL;
        count = 0;
#endif
      }

      void setRequestBuffer(CM_Request_t *request, size_t req_size)
      {
        this->request = request;
        this->req_size = req_size;
      }

      void setConnectionId (unsigned conn)
      {
        this->connection_id = conn;
      }

      void setRoles (unsigned roles)
      {
        this->roles = roles;
      }

      void setData (LL_PipeWorkQueue_t *data, size_t count)
      {
        this->data = data;
        this->count = count;
      }

      void setDataRanks (LL_Topology_t *data_participants)
      {
        this->data_participants = data_participants;
      }

      void setResults (LL_PipeWorkQueue_t *results, size_t count)
      {
        this->results = results;
        this->count = count;
      }

      void setResultsRanks (LL_Topology_t *results_participants)
      {
        this->results_participants = results_participants;
      }

      void setReduceInfo (CM_Op op,  CM_Dt dt)
      {
        this->op = op;
        this->dt = dt;
      }

      void setCallback (void (*fn) (void *, CM_Error_t *),  void *cd)
      {
        cb_done.function = fn;
        cb_done.clientdata = cd;
      }

#ifdef NOT_YET /* These are only needed if we support one-sided multicombine directly */
      void setInfo (CMQuad *info, int count)
      {
        this->msginfo = info;
        this->count   = count;
      }
#endif

    }; // struct CCMI_Multicombine_t

    // This MUST be kept identical to LL_Multisync_t!
    struct CCMI_Multisync_t
    {
      void              * registration;
      CM_Request_t    * request;
      size_t              req_size;
      CM_Callback_t     cb_done;
      unsigned            connection_id;
      unsigned            roles;
      size_t              bytes;
      LL_Topology_t    *participants;

      CCMI_Multisync_t ()
      {
        registration = NULL;
        request = NULL;
        req_size = 0;
        connection_id = ((unsigned) -1);
        roles = 0;
        participants = NULL;
      }

      void setRequestBuffer (CM_Request_t *request, size_t req_size)
      {
        this->request = request;
        this->req_size = req_size;
      }

      void setConnectionId (unsigned conn)
      {
        this->connection_id = conn;
      }

      void setRoles (unsigned roles)
      {
        this->roles = roles;
      }

      void setRanks (LL_Topology_t *participants)
      {
        this->participants = participants;
      }

      void setCallback (void (*fn) (void *, CM_Error_t *),  void *cd)
      {
        cb_done.function = fn;
        cb_done.clientdata = cd;
      }

    }; // struct CCMI_Multisync_t

    ///
    /// \brief Message passing interface suitable for broadcasts
    ///
    class MulticastInterface
    {
    protected:
      ///
      ///  \brief the arguments for async mode of operation where no
      ///  receive is posted
      ///
      CCMI_RecvMulticast_t   _cb_async_head;
      void                 * _async_arg;

    public:
      ///
      /// \brief Constructor
      ///
      MulticastInterface ()
      {
        _cb_async_head = NULL;
        _async_arg = NULL;
      }

      ///
      /// \brief Destructor
      ///
      virtual ~MulticastInterface ()
      {
      }

      /// NOTE: This is required to make "C" programs link successfully with virtual destructors
      void operator delete(void * p)
      {
        CCMI_abort();
      }

      /// Set the async head packet callback
      inline void setCallback (CCMI_RecvMulticast_t cb_recv, void *arg)
      {
        _cb_async_head    =  cb_recv;
        _async_arg        =  arg;
      }

      ///
      /// \brief Create a multicast message
      ///
      /// \param mcastinfo  : mcast parameters in a structure
      ///
      virtual CM_Result generate(CCMI_Multicast_t *mcastinfo) = 0;

    };  //-- MulticastInterface

    ///
    /// \brief Message passing interface suitable for reduce, allreduce, ...
    ///
    class MulticombineInterface
    {
    protected:
#ifdef NOT_YET /* These are only needed if we support one-sided multicombine directly */
      ///
      ///  \brief the arguments for one-sided operation
      ///
      CCMI_RecvMulticombine_t   _cb_async_head;
      void                 * _async_arg;
#endif

    public:
      ///
      /// \brief Constructor
      ///
      MulticombineInterface ()
      {
#ifdef NOT_YET /* These are only needed if we support one-sided multicombine directly */
        _cb_async_head = NULL;
        _async_arg = NULL;
#endif
      }

      ///
      /// \brief Destructor
      ///
      virtual ~MulticombineInterface ()
      {
      }

      /// NOTE: This is required to make "C" programs link successfully with virtual destructors
      void operator delete(void * p)
      {
        CCMI_abort();
      }

#ifdef NOT_YET /* These are only needed if we support one-sided multicombine directly */
      /// Set the async head packet callback
      inline void setCallback (CCMI_RecvMulticombine_t cb_recv, void *arg)
      {
        _cb_async_head    =  cb_recv;
        _async_arg        =  arg;
      }
#endif

      ///
      /// \brief Create a multicombine message
      ///
      /// \param mcombinfo  : mcombine parameters in a structure
      ///
      virtual CM_Result generate(CCMI_Multicombine_t *mcombinfo) = 0;

    };  //-- MulticombineInterface

    ///
    /// \brief Message passing interface suitable for barriers
    ///
    class MultisyncInterface
    {
    protected:

    public:
      ///
      /// \brief Constructor
      ///
      MultisyncInterface ()
      {
      }

      ///
      /// \brief Destructor
      ///
      virtual ~MultisyncInterface ()
      {
      }

      /// NOTE: This is required to make "C" programs link successfully with virtual destructors
      void operator delete(void * p)
      {
        CCMI_abort();
      }

      ///
      /// \brief Create a multisync message
      ///
      /// \param msyncinfo  : msync parameters in a structure
      ///
      virtual CM_Result generate(CCMI_Multisync_t *msyncinfo) = 0;

    };  //-- MultisyncInterface

//==============================================================================
// older stuff... disposition TBD

    ///
    /// \brief Callback function for unexpected async many to many
    /// operations
    ///
    typedef CM_Request_t * (*manytomany_recv) (unsigned          conn_id,
                                                 void            * arg,
                                                 char           ** rcvbuf,
                                                 unsigned       ** rcvdispls,
                                                 unsigned       ** rcvlens,
                                                 unsigned       **rcvcounters,
                                                 unsigned        * nranks,
                                                 CM_Callback_t * cb_done);

    typedef enum
    {
      CCMI_FLAGS_UNSET                =  0,
      CCMI_PERSISTENT_MESSAGE         =  1,
    } CCMI_MulticastFlags_t;

    struct CCMI_OldMulticast_t
    {
      void              * registration;
      CM_Request_t    * request;
      CM_Callback_t     cb_done;
      CCMI_Consistency    consistency;
      unsigned            connection_id;
      unsigned            bytes;
      const char        * src;
      unsigned            nranks;
      unsigned          * ranks;
      CCMI_Subtask      * opcodes;
      const CMQuad    * msginfo;
      unsigned            count;
      CM_Op             op;
      CM_Dt             dt;
      unsigned            flags;

      CCMI_OldMulticast_t ()
      {
        registration   = NULL;
        request        = NULL;
        consistency    = CCMI_MATCH_CONSISTENCY;
        connection_id  = ((unsigned) -1);
        bytes          = 0;;
        src            = NULL;
        nranks         = 0;
        ranks          = NULL;
        opcodes        = NULL;
        msginfo        = NULL;
        count          = 0;
        op             = CM_UNDEFINED_OP;
        dt             = CM_UNDEFINED_DT;
        flags          = CCMI_FLAGS_UNSET;
      }

      void setRequestBuffer (CM_Request_t *request)
      {
        this->request = request;
      }

      void setConnectionId (unsigned conn)
      {
        this->connection_id = conn;
      }

      void setSendData (const char *src, unsigned bytes)
      {
        this->src = src;
        this->bytes = bytes;
      }

      void setCallback (void (*fn) (void *, CM_Error_t *),  void *cd)
      {
        cb_done.function = fn;
        cb_done.clientdata = cd;
      }

      void setReduceInfo (CM_Op op,  CM_Dt dt)
      {
        this->op = op;
        this->dt = dt;
      }

      void setConsistency (CCMI_Consistency c)
      {
        this->consistency = c;
      }

      void setOpcodes (CCMI_Subtask *op)
      {
        this->opcodes = op;
      }

      void setInfo (CMQuad *info, int count)
      {
        this->msginfo = info;
        this->count   = count;
      }

      void setRanks (unsigned *ranks, unsigned nranks)
      {
        this->ranks = ranks;
        this->nranks = nranks;
      }

      void setFlags (CCMI_MulticastFlags_t flag)
      {
        this->flags = flag;
      }
    };
//    #warning CCMI OldMulticastRecv t
    struct CCMI_OldMulticastRecv_t
    {
      void              * registration;
      CM_Request_t    * request;
      CM_Callback_t     cb_done;
      unsigned            connection_id;
      unsigned            bytes;
      char              * rcvbuf;
      unsigned            pipelineWidth;
      CCMI_Subtask             opcode;
      CM_Op             op;
      CM_Dt             dt;

      CCMI_OldMulticastRecv_t ()
      {
        registration   = NULL;
        request        = NULL;
        connection_id  = ((unsigned) -1);
        bytes          = 0;;
        rcvbuf         = NULL;
        opcode         = CCMI_PT_TO_PT_SUBTASK;
        op             = CM_UNDEFINED_OP;
        dt             = CM_UNDEFINED_DT;
      }

      void setRequestBuffer (CM_Request_t *request)
      {
        this->request = request;
      }

      void setConnectionId (unsigned conn)
      {
        this->connection_id = conn;
      }

      void setRecvData (char *buf, unsigned bytes)
      {
        this->rcvbuf = buf;
        this->bytes = bytes;
      }

      void setCallback (void (*fn) (void *, CM_Error_t *),  void *cd)
      {
        cb_done.function = fn;
        cb_done.clientdata = cd;
      }

      void setReduceInfo (CM_Op op,  CM_Dt dt)
      {
        this->op = op;
        this->dt = dt;
      }

      void setOpcode (CCMI_Subtask op)
      {
        this->opcode = op;
      }

      void setPipelineWidth (unsigned pw)
      {
        pipelineWidth = pw;
      }
    };

#ifdef DEPRECATED_MULTICAST
    ///
    /// \brief Message passing interface suitable for broadcasts, barriers
    /// and reductions
    ///
    class OldMulticastInterface
    {
    protected:
      ///
      ///  \brief the arguments for async mode of operation where no
      ///  receive is posted
      ///
      DCMF_OldRecvMulticast   _cb_async_head;
      void                 * _async_arg;

    public:
      ///
      /// \brief Constructor
      ///
      OldMulticastInterface ()
      {
        _cb_async_head = NULL;
        _async_arg = NULL;
      }

      ///
      /// \brief Destructor
      ///
      virtual ~OldMulticastInterface ()
      {
      }

      /// NOTE: This is required to make "C" programs link successfully with virtual destructors
      void operator delete(void * p)
      {
        CCMI_abort();
      }

      /// Set the async head packet callback
      inline void setCallback (DCMF_OldRecvMulticast cb_recv, void *arg)
      {
        TRACE_INIT((stderr, "<%#.8X>CCMI::MultiSend::MulticastInterface::setCallback() %#.8X %#.8X\n", (int)this,(int)cb_recv,(int)arg));
        _cb_async_head    =  cb_recv;
        _async_arg        =  arg;
      }

      ///
      /// \brief Create a multicast message
      /// \param buf  : start of the buffer to scatter
      /// \param cb_done : Callback when the multicast is sent
      /// \param consisteny : ordering model
      /// \param info : A 16 byte field that will
      ///               with the first packet. Info can be NULL if
      ///               a Recv has guaranteed to be posted.
      /// \param connection_id : The connection id for this message
      /// \param size : size of the multicast buffer
      /// \param ranks   : Ranks of the destinations
      /// \param nranks  : Number of destinations
      /// \param hints   : deposit bit bcast vs pt-to-pt
      /// \param op      : operator in case of reduction ( for pre-processing )
      /// \param dtype   : datatype in case of reduction ( for pre-processing )
      ///

      virtual unsigned  send  (CM_Request_t         * request,
                               const CM_Callback_t  * cb_done,
                               CCMI_Consistency         consistency,
                               const CMQuad         * info,
                               unsigned                 info_count,
                               unsigned                 connection_id,
                               const char             * buf,
                               unsigned                 size,
                               unsigned               * hints,
                               unsigned               * ranks,
                               unsigned                 nranks,
                               CM_Op                  op    = CM_UNDEFINED_OP,
                               CM_Dt                  dtype = CM_UNDEFINED_DT ) = 0;

      virtual unsigned send (CCMI_OldMulticast_t  *mcastinfo) = 0;


      ///
      /// \brief Post a multicast receive
      /// \param buf  : start of the buffer to scatter
      /// \param cb_done : Callback when the multicast is received
      /// \param conn_id : The connection id for this message
      /// \param size : size of the multicast buffer
      /// \param pwidth : Pipeline width for the mcast buffer.
      ///                 After pwidth data is received the
      ///                 cb_done handler will be called.
      /// \param op     : operator in case of reduction ( for post-processing )
      /// \param dtype  : datatype in case of reduction ( for post-processing )

      ///

      virtual unsigned postRecv (CM_Request_t         * request,
                                 const CM_Callback_t  * cb_done,
                                 unsigned                 conn_id,
                                 char                   * buf,
                                 unsigned                 size,
                                 unsigned                 pwidth,
                                 unsigned                 hint   = CCMI_UNDEFINED_SUBTASK,
                                 CM_Op                  op     = CM_UNDEFINED_OP,
                                 CM_Dt                  dtype  = CM_UNDEFINED_DT ) = 0;

      virtual unsigned postRecv (CCMI_OldMulticastRecv_t  *mrecv) = 0;

#if 0
      /// \todo temp hack until it's supported directly
      return postRecv(mrecv->request,
                      &mrecv->cb_done,
                      mrecv->connection_id,
                      mrecv->rcvbuf,
                      mrecv->bytes,
                      mrecv->pipelineWidth,
                      mrecv->opcode,
                      mrecv->op,
                      mrecv->dt);
#endif

    };  //-- OldMulticastInterface
#endif // DEPRECATED_MULTICAST

    ///
    /// \brief Message passing interface suitable for alltoall
    /// communiction
    ///
    class ManytomanyInterface
    {
    protected:

      manytomany_recv     _cb_m2m_head;
      void              * _async_arg;

    public:
      /// \brief Constructor
      inline ManytomanyInterface ()
      {
        _cb_m2m_head = NULL;
        _async_arg = NULL;
      }

      /// \breif Distructor
      virtual ~ManytomanyInterface ()
      {
      }

      /// NOTE: This is required to make "C" programs link successfully with virtual destructors
      void operator delete(void * p)
      {
        CCMI_abort();
      }


      /// Set the async first packet callback
      inline void setCallback (manytomany_recv cb_recv, void *arg)
      {
        TRACE_INIT((stderr, "<%#.8X>CCMI::MultiSend::ManytomanyInterface::setCallback() %#.8X %#.8X\n", (int)this,(int)cb_recv,(int)arg));
        _cb_m2m_head    =  cb_recv;
        _async_arg      =  arg;
      }

      ///
      /// \brief Create a many-to-many message
      /// \param request : request object
      /// \param cb_done : Callback when the many-to-many is sent
      /// \param connid  : connection id
      /// \param rcvindex : which slot of the receiver displacements to use
      /// \param buf  : start of the buffer to scatter
      /// \param sizes : size of the many-to-many buffer
      /// \param offsets : offsets from the start of the buffer
      /// \param ranks   : Ranks of the destinations
      /// \param permutation : what order to sent the messages
      /// \param nranks  : Number of destinations
      ///

      virtual void send  (CM_Request_t         * request,
                          const CM_Callback_t  * cb_done,
                          unsigned                 connid,
                          unsigned                 rcvindex,
                          const char             * buf,
                          unsigned               * sizes,
                          unsigned               * offsets,
                          unsigned               * counters,
                          unsigned               * ranks,
                          unsigned               * permutation,
                          unsigned                 nranks) = 0;

      ///
      /// \brief Post a many-to-many receive
      /// \param request :  the request object
      /// \param cb_done : Callback when the many-to-many is received
      /// \param connid : connection id on which the receive is to be
      ///                 posted
      /// \param buf  : start of the buffer to receive data
      /// \param sizes : sizes of the many-to-many buffers
      /// \param offsets : start offsets for the different senders
      /// \param counters: temporary storage for counters
      /// \param nranks  : number of ranks
      ///

      virtual void postRecv (CM_Request_t         * request,
                             const CM_Callback_t  * cb_done,
                             unsigned                 connid,
                             char                   * buf,
                             unsigned               * sizes,
                             unsigned               * offsets,
                             unsigned               * counters,
                             unsigned                 nranks,
                             unsigned                 myindex) = 0;

    };  //-- ManytomanyInterface
  };  //-- Multisend namespace
};  //-- CCMI

#endif
