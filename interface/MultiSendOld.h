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
//#include "ll_topology.h" 
//#include "ll_pipeworkqueue.h" 

namespace CCMI
{
  namespace MultiSend
  {

    ///
    /// \brief Callback function for unexpected async broadcasts
    ///


    #ifndef __ccmi_recv_multicast_callback_defined__ // may be pre-defined in the adaptor
    #warning old ms
    typedef CCMI_Request_t * (*CCMI_RecvMulticast_t) (const CMQuad  * info,
                                            unsigned          count,
                                            unsigned          peer,
                                            unsigned          sndlen,
                                            unsigned          conn_id,
                                            void            * arg,
                                            unsigned        * rcvlen,
                                            char           ** rcvbuf,
                                            unsigned        * pipewidth,
                                            CM_Callback_t * cb_done);
    #endif

    ///
    /// \brief Callback function for unexpected async many to many
    /// operations
    ///
    typedef CCMI_Request_t * (*manytomany_recv) (unsigned          conn_id,
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

    struct CCMI_Multicast_t 
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
    
      CCMI_Multicast_t () 
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

      void setRequestBuffer (CCMI_Request_t *request)
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

    struct CCMI_MulticastRecv_t 
    {
      void              * registration;
      CCMI_Request_t    * request;     
      CM_Callback_t     cb_done;     
      unsigned            connection_id;   
      unsigned            bytes;     
      char              * rcvbuf; 
      unsigned            pipelineWidth;
      CCMI_Subtask             opcode;  
      CM_Op             op;      
      CM_Dt             dt;           

      CCMI_MulticastRecv_t () 
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

      void setRequestBuffer (CCMI_Request_t *request)
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

    ///
    /// \brief Message passing interface suitable for broadcasts, barriers
    /// and reductions
    ///
    class MulticastInterface
    {
    protected:
    public:
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

      virtual unsigned  send  (CCMI_Request_t         * request,
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

      virtual unsigned send (CCMI_Multicast_t  *mcastinfo) = 0;


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

      virtual unsigned postRecv (CCMI_Request_t         * request,
                                 const CM_Callback_t  * cb_done,
                                 unsigned                 conn_id,
                                 char                   * buf,
                                 unsigned                 size,
                                 unsigned                 pwidth,
                                 unsigned                 hint   = CCMI_UNDEFINED_SUBTASK,
                                 CM_Op                  op     = CM_UNDEFINED_OP,
                                 CM_Dt                  dtype  = CM_UNDEFINED_DT ) = 0;

      virtual unsigned postRecv (CCMI_MulticastRecv_t  *mrecv) = 0;

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

    };  //-- MulticastInterface

    ///
    /// \brief Message passing interface suitable for alltoall
    /// communiction
    ///


    class ManytomanyInterface
    {
    protected:

      manytomany_recv     _cb_async_head;
      void              * _async_arg;

    public:
      /// \brief Constructor
      inline ManytomanyInterface ()
      {
        _cb_async_head = NULL;
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
        _cb_async_head    =  cb_recv;
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

      virtual void send  (CCMI_Request_t         * request,
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

      virtual void postRecv (CCMI_Request_t         * request,
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
