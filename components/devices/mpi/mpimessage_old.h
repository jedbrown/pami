/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/mpi/mpimessage.h
 * \brief ???
 */

#ifndef __components_devices_mpi_mpimessage_old_h__
#define __components_devices_mpi_mpimessage_old_h__

#ifndef TRACE_ADAPTOR
#define TRACE_ADAPTOR(x) //fprintf x
#endif


namespace XMI
{
  namespace Device
  {
    class MPIMessage
    {
    public:
      inline MPIMessage (xmi_context_t       context,
                         size_t              dispatch_id,
                         xmi_event_function  done_fn,
                         void               *cookie):
      _context(context),
      _done_fn(done_fn),
      _cookie(cookie)
      {
        TRACE_ADAPTOR((stderr,"%s dispatch_id %d\n",__PRETTY_FUNCTION__, dispatch_id));
        _p2p_msg._dispatch_id=dispatch_id;
      };
      xmi_context_t       _context;
      xmi_event_function  _done_fn;
      void               *_cookie;
      int                 _freeme;
      MPI_Request         _request;

      struct _p2p_msg
      {
        size_t              _dispatch_id;
        int                 _metadatasize;
        int                 _payloadsize0;
        int                 _payloadsize1;
        char                _metadata[128];
        char                _payload[224];
      }_p2p_msg;
    protected:
    private:
    };


    class MPIMcastMessage
    {
    public:
      xmi_context_t  _context;
      size_t         _dispatch_id;
      xmi_quad_t     _info[2];
      int            _info_count;
      int            _size;
      unsigned       _conn;
      MPI_Request    *_req;
      int            _num;
      xmi_callback_t _cb_done;
      inline void *buffer() { return((char *)this + sizeof (*this));}
      inline int  totalsize () { return _size + sizeof (*this);}
    };

    class MPIGlobalMcastMessage
    {
    public:
      MPIGlobalMcastMessage(xmi_multicast_t *&mcast, size_t dispatch_id):
        context(mcast->context),
        cb_done(mcast->cb_done),
        connection_id(mcast->connection_id),
        roles(mcast->roles),
        bytes(mcast->bytes),
        src(mcast->src),
        src_participants(mcast->src_participants),
        dst(mcast->dst),
        dst_participants(mcast->dst_participants),
        msginfo(mcast->msginfo),
        msgcount(mcast->msgcount),
        _dispatch_id(dispatch_id) {};
      xmi_context_t        context;          /**< context to operate within */
      xmi_callback_t       cb_done;          /**< Completion callback */
      unsigned             connection_id;    /**< A connection is a distinct stream of
                                                  traffic. The connection id identifies the
                                                  connection */
      unsigned             roles;            /**< bitmap of roles to perform */
      size_t               bytes;            /**< size of the message*/
      xmi_pipeworkqueue_t *src;              /**< source buffer */
      xmi_topology_t      *src_participants; /**< root */
      xmi_pipeworkqueue_t *dst;              /**< dest buffer (ignored for one-sided) */
      xmi_topology_t      *dst_participants; /**< destinations to multicast to*/
      const xmi_quad_t    *msginfo;          /**< A extra info field to be sent with the message.
                                                  This might include information about
                                                  the data being sent, for one-sided. */
      unsigned            msgcount;          /**< info count*/

      size_t              _dispatch_id;
    };

    class MPIMcastRecvMessage
    {
    public:
      size_t              _dispatch_id;
      unsigned            _conn;
      xmi_event_function  _done_fn;
      void               *_cookie;
      char               *_buf;
      size_t              _size;
      size_t              _pwidth;
      unsigned            _nranks;
      unsigned            _hint;
      xmi_op              _op;
      xmi_dt              _dtype;
      size_t              _counter;
    };

    class MPIM2MMessage
    {
    public:
      xmi_context_t       _context;
      size_t              _dispatch_id;
      unsigned            _conn;
      xmi_event_function  _done_fn;
      void               *_cookie;
      int                 _num;
      MPI_Request        *_reqs;
      int                 _totalsize;
      char               *_bufs;
    };

    template <class T_Counter>
    class MPIM2MRecvMessage
    {
    public:
      size_t              _dispatch_id;
      unsigned            _conn;
      xmi_event_function  _done_fn;
      void               *_cookie;
      int                 _num;
      char               *_buf;
      T_Counter          *_sizes;
      T_Counter          *_offsets;
      unsigned            _nranks;
    };

    class MPIM2MHeader
    {
    public:
      size_t      _dispatch_id;
      unsigned    _size;
      unsigned    _conn;
      inline void *buffer() { return((char *)this + sizeof (MPIM2MHeader));}
      inline int  totalsize () { return _size + sizeof (MPIM2MHeader);}
    };


  };
};

#endif // __components_devices_mpi_mpibasemessage_old_h__
