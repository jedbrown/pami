/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file xmi_multisend.h
 * \brief Multisend interface.
 *
 * A multisend operation allows many message passing transactions to
 * be performed in the same call to amortize software overheads.  It
 * has two flavors
 *
 *   - multicast, which sends the same buffer to a list of processors
 *     and also supports depost-bit torus line broadcasts
 *
 *   - manytomany, which sends different offsets from the same buffer
 *     to a list of processors
 *
 * As reductions have a single source buffer and destination buffer,
 * we have extended the multicast call to support reductions.
 *
 * Each multisend operation carries a connection id to identify the
 * data on the reciever.
 *
 * In a multisend operation the cores are typically involved at the
 * end-points. Moreover the processor lists can be created on the fly.
 */
#ifndef __xmi_multisend_h__
#define __xmi_multisend_h__

#include "xmi_types.h"
#include "xmi_topology.h"
#include "xmi_pipeworkqueue.h"

/*
 * These are now defined in xmi_impl.h (via xmi_types.h), according to target needs
 */
#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * \brief Determine role information for a multisend protocol
     *
     * Roles are numbered 0..(N-1). Role 0 is the first role in the data-chain.
     * Role 0 performs the root task(s) of a broadcast operation, for example.
     * Role (N-1) (may also be 0 if numRoles is 1) is the last role in the data-chain.
     * It performs the root task(s) of a reduce operation, as an example. Roles between
     * 0 and (N-1) are intermediate roles of non-specific task. Typically, all roles
     * pass in "data" and "results", even though some roles may ignore them.
     * Roles also specify how to assign roles when there are more processes than roles.
     * The "R" (replication role) indicates which role to replicate for any additional
     * processes. Beyond that, each process is assigned role "N" (an invalid role).
     * Role information (numRoles and ReplRole) is determined from the factory that
     * was registered. So a typical procedure to generate a multicombine would be:
     *
     *	factory->getRoles(&numRoles, &replRole);
     *	// analyze and select role(s)
     *	factory->generate(..., roles, ...);
     *
     * Role information is static (?) so the analysis of roles might be done at
     * registration/init time and carried forward. If that is not the case, we may
     * need to add additional params or perhaps even split this out by multi* type.
     * For example, if there is not a 1:1 relationship between factory (registration)
     * and message/device, then some other way must be found to determine the roles
     * before calling factory->generate(). This might require passing some or all of
     * the generate() parameters into the getRoles() call (i.e. pass in the param struct).
     * Or, perhaps generate() returns the role information and then roles must be set
     * after generate(), but this creates problems not only with optimizations but
     * also because generate() often starts the message and roles must be known by that time.
     * Perhaps, generate() passes generic role info, such as "I am #2 of 4" and let
     * generate setup roles according to some standard algorithm. But that may not
     * allow for special treatment based on locality of data or other work assigned
     * to that core.
     *
     * Example 1: numRoles = 2 and replRole = 0 (e.g. a Reduce)
     *	All data participants use (replicate) role "0". The root uses role "1".
     *	If a participant is the root but is not supplying data, it would use
     *	the data_participants topology to indicate that.
     *
     * Example 2: numRoles = 2 and replRole = -1 (e.g. Tree device, injection/reception)
     *	One participant, closest to the input data, uses role "0". Another
     *	participant (closest to the results) uses role "1". If there is only one
     *	(local) participant, i.e. SMP mode, then the role is "0|1" (both roles).
     *
     * Example 3: numRoles = 1 and replRole = -1 (e.g. one-sided bcast)
     *	(a.k.a multicast)
     *	Origin uses role "0" and sets results participants to the destinations.
     *	Destinations all get data via recv callback setup at registration time.
     *
     * It is expected that the caller of a multisend has some knowledge of the type
     * of multisend being performed. This is required in order to even setup
     * topologies, let alone roles. There is much context required to make a
     * multisend call, so adding the roles as a dependency of the context should
     * not be overly restrictive.
     *
     * \param[in] registration	The protocol to be analysed
     * \param[out] numRoles	The number of different roles supported/required
     * \param[out] replRole	The role to replicate for additional participants,
     *				or -1 if no additional roles are used.
     * \return	success or failure
     */
    xmi_result_t XMI_Multisend_getroles(xmi_context_t   context,
                                        xmi_dispatch_t  dispatch,
                                        int            *numRoles,
                                        int            *replRole);

    /**
     * \brief Recv callback for Multicast
     *
     * Note, certain flavors of Multicast do not use a Receive Callback and
     * constructing or registering with a non-NULL cb_recv will result in error.
     *
     * Does this accept zero-byte (no data, no metadata) operations?
     *
     * \param[in] msginfo		Metadata
     * \param[in] msgcount	Count of metadata
     * \param[in] connection_id  Stream ID of data
     * \param[in] root		Sending rank
     * \param[in] sndlen		Length of data sent
     * \param[in] clientdata	Opaque arg
     * \param[out] rcvlen		Length of data to receive
     * \param[out] rcvbuf		Where to put recv data
     * \param[out] cb_done	Completion callback to invoke when data received
     * \return	XMI_Request opaque memory for message
     */
    typedef void (*xmi_dispatch_multicast_fn)(const xmi_quad_t        *msginfo,
                                              unsigned              msgcount,
                                              unsigned              connection_id,
                                              size_t                root,
                                              size_t                sndlen,
                                              void                 *clientdata,
                                              size_t               *rcvlen,
                                              xmi_pipeworkqueue_t **rcvbuf,
                                              xmi_event_function   *cb_done);
    
    /**
     * \brief The new structure to pass parameters for the multisend multicast operation.
     *
     * The XMI_Multicast_t object is re-useable immediately, but objects referred to
     * (src, etc) cannot be re-used until cb_done.
     */
    typedef struct {
        size_t               req_size;	       /**< space available in request, bytes */
        xmi_event_function   cb_done;          /**< Completion callback */
        unsigned             connection_id;    /**< A connection is a distinct stream of
                                                   traffic. The connection id identifies the
                                                   connection */
        unsigned             roles;            /**< bitmap of roles to perform */
        size_t               bytes;            /**< size of the message*/
        xmi_pipeworkqueue_t *src;              /**< source buffer */
        xmi_topology_t      *src_participants; /**< root */
        xmi_pipeworkqueue_t *dst;              /**< dest buffer (ignored for one-sided) */
        xmi_topology_t      *dst_participants; /**< destinations to multicast to*/
        const xmi_quad_t    *msginfo;	       /**< A extra info field to be sent with the message.
                                                  This might include information about
                                                  the data being sent, for one-sided. */
        unsigned            msgcount;          /**< info count*/
    } xmi_multicast_t;
    xmi_result_t XMI_Multicast(xmi_context_t context,xmi_multicast_t *mcastinfo);


    /**  Deprecated Multicast:  To be deleted soon!!! */
    /**********************************************************************/
  typedef enum
  {
    XMI_UNDEFINED_CONSISTENCY = -1,
    XMI_RELAXED_CONSISTENCY,
    XMI_MATCH_CONSISTENCY,
    XMI_WEAK_CONSISTENCY,
    XMI_CONSISTENCY_COUNT
  }xmi_consistency_t;
  
 #define  LINE_BCAST_MASK    (XMI_LINE_BCAST_XP|XMI_LINE_BCAST_XM|	\
                              XMI_LINE_BCAST_YP|XMI_LINE_BCAST_YM|	\
                              XMI_LINE_BCAST_ZP|XMI_LINE_BCAST_ZM)
    typedef enum
    {
      XMI_PT_TO_PT_SUBTASK           =  0,      //Send a pt-to-point message
      XMI_LINE_BCAST_XP              =  0x20,   //Bcast along x+
      XMI_LINE_BCAST_XM              =  0x10,   //Bcast along x-
      XMI_LINE_BCAST_YP              =  0x08,   //Bcast along y+
      XMI_LINE_BCAST_YM              =  0x04,   //Bcast along y-
      XMI_LINE_BCAST_ZP              =  0x02,   //Bcast along z+
      XMI_LINE_BCAST_ZM              =  0x01,   //Bcast along z-
      XMI_COMBINE_SUBTASK            =  0x0100,   //Combine the incoming message
      //with the local state
      XMI_GI_BARRIER                 =  0x0200,
      XMI_LOCKBOX_BARRIER            =  0x0300,
      XMI_TREE_BARRIER               =  0x0400,
      XMI_TREE_BCAST                 =  0x0500,
      XMI_TREE_ALLREDUCE             =  0x0600,
      XMI_REDUCE_RECV_STORE          =  0x0700,
      XMI_REDUCE_RECV_NOSTORE        =  0x0800,
      XMI_BCAST_RECV_STORE           =  0x0900,
      XMI_BCAST_RECV_NOSTORE         =  0x0a00,
      XMI_LOCALCOPY                  =  0x0b00,
      XMI_UNDEFINED_SUBTASK          =  (~LINE_BCAST_MASK),
    } xmi_subtask_t;


    typedef xmi_quad_t * (*xmi_olddispatch_multicast_fn) (const xmi_quad_t   * info,
                                                          unsigned             count,
                                                          unsigned             peer,
                                                          unsigned             sndlen,
                                                          unsigned             conn_id,
                                                          void               * arg,
                                                          unsigned           * rcvlen,
                                                          char              ** rcvbuf,
                                                          unsigned           * pipewidth,
                                                          xmi_callback_t     * cb_done);
    typedef struct xmi_oldmulticast_t
    {
      xmi_quad_t        * request;
      xmi_callback_t      cb_done;
      unsigned            connection_id;
      unsigned            bytes;
      const char        * src;
      unsigned            nranks;
      unsigned          * ranks;
      xmi_subtask_t     * opcodes;
      const xmi_quad_t  * msginfo;
      unsigned            count;
      unsigned            flags;
    }xmi_oldmulticast_t;
  
    typedef struct xmi_oldmulticast_recv_t
    {
      xmi_callback_t      cb_done;
      unsigned            connection_id;
      unsigned            bytes;
      char              * rcvbuf;
      unsigned            pipelineWidth;
      xmi_subtask_t       opcode;
      xmi_op              op;
      xmi_dt              dt;
    }xmi_oldmulticast_recv_t;
    /**********************************************************************/

  

    /**
     * \brief Sub-structure used to represent a vectored buffer for many-to-many.
     *
     * This is used for both send and receive parameters.
     *
     * This structure is likely to change as decisions are made about how
     * much of the vector to embed inside PipeWorkQueue and if Topology
     * contains permutation information.
     */
    typedef struct {
        xmi_pipeworkqueue_t *buffer;	    /**< Memory used for data (buffer) */
        xmi_topology_t      *participants;  /**< Ranks that are vectored in buffer */
        size_t              *lengths;       /**< Array of lengths in buffer for each rank */
        size_t              *offsets;       /**< Array of offsets in buffer for each rank */
        size_t               num_vecs;      /**< The number of entries in "lengths" and
                                               "offsets". May be a flag: either "1" or
                                               participants->size(). */
    } xmi_manytomanybuf_t;

    /**
     * \brief Callback for Manytomany Receive operations
     *
     * Note, certain flavors of ManyToMany do not use a Receive Callback and
     * constructing or registering with a non-NULL cb_recv will result in error.
     *
     * All senders of metadata for a given connection_id (instance) must send
     * the same, or at least compatible, metadata such that all would return the
     * same receive parameters (**recv). Note, the recv callback will be invoked
     * only once per connection_id. The first sender message to arrive will invoke
     * the callback and get recv params for ALL other senders in the instance.
     *
     * The myIndex parameter is the receiving rank's index in the recv arrays
     * (lengths and offsets) and is used by the manytomany as an optimization
     * for handling reception data and completion.
     *
     * \param[in] arg		Client Data
     * \param[in] conn_id		Instance ID
     * \param[in] metadata	Pointer to metadata, if any, in message header.
     * \param[in] metacount	Number of xmi_quad_ts of metadata.
     * \param[out] recv		Receive parameters for this connection (instance)
     * \param[out] myIndex	Index of Recv Rank in the receive parameters.
     * \param[out] cb_done	Completion callback when message complete
     * \return	Request object opaque storage for message.
     */
    typedef void (*xmi_dispatch_manytomany_fn)(void                 *arg,
                                               unsigned              conn_id,
                                               xmi_quad_t           *metadata,
                                               unsigned              metacount,
                                               xmi_manytomanybuf_t **recv,
                                               size_t               *myIndex,
                                               xmi_event_function   *cb_done);
    
    /**
     * \brief Structure of parameters used to initiate a ManyToMany
     *
     * The rankIndex parameter is transmitted to the receiver for use by cb_recv
     * for indexing into the recv parameter arrays (lengths and offsets).
     */
    typedef struct {
        size_t               req_size;	     /**< space available in request, bytes */
        xmi_event_function   cb_done;	     /**< User's completion callback */
        unsigned             connection_id;  /**< differentiate data streams */
        unsigned             roles;          /**< bitmap of roles to perform */
        size_t              *rankIndex;	     /**< Index of send in recv parameters */
        size_t               num_index;      /**< Number of entries in "rankIndex".
                                                should be multiple of send.participants->size()?
                                             */
        xmi_manytomanybuf_t  send;           /**< send data parameters */
        const xmi_quad_t    *metadata;	     /**< A extra info field to be sent with the message.
                                                This might include information about
                                                the data being sent, for one-sided. */
        unsigned             metacount;	     /**< metadata count*/
    } xmi_manytomany_t;

    /**
     * \brief Initiate a ManyToMany
     *
     * \param[in] m2minfo	Paramters for ManyToMany operation to be performed
     * \return	XMI_SUCCESS or error code
     */
    xmi_result_t XMI_Manytomany(xmi_context_t context,xmi_manytomany_t *m2minfo);

    /******************************************************************************
     *       Multisync Personalized synchronization/coordination
     ******************************************************************************/
    
    /**
     * \brief Recv callback for Multisync.
     *
     * Not normally used.
     *
     * Note, certain flavors of Multisync do not use a Receive Callback and
     * constructing or registering with a non-NULL cb_recv will result in error.
     *
     * \param[in] clientdata	Opaque arg
     * \param[in] msginfo		Metadata
     * \param[in] msgcount	Number of xmi_quad_ts in msginfo
     * \param[in] conn_id		Instance ID
     * \return	XMI_Request opaque memory for message
     */
    typedef void (*xmi_dispatch_multisync_fn)(void       *clientdata,
                                              xmi_quad_t *msginfo,
                                              unsigned    msgcount,
                                              unsigned    conn_id);
    /**
     * \brief structure defining interface to Multisync
     */
    typedef struct {
        size_t             req_size;		/**< space available in request, bytes */
        xmi_event_function cb_done;		/**< User's completion callback */
        unsigned           connection_id;	/**< (remove?) differentiate data streams */
        unsigned           roles;		/**< bitmap of roles to perform */
        xmi_topology_t    *participants;	/**< Ranks involved in synchronization */
    } xmi_multisync_t;
    /**
     * \brief Barriers and the like.
     *
     * All participants make this call. So, there is no "send" or "recv"
     * distinction needed.
     *
     * \param[in] msyncinfo	Struct of all params needed to perform operation
     * \return	XMI_SUCCESS or error codes
     */
    xmi_result_t XMI_Multisync(xmi_context_t context,xmi_multisync_t *msyncinfo);


    /******************************************************************************
     *       Multicombine Personalized reduction
     ******************************************************************************/
    /**
     * \brief structure defining interface to Multicombine
     *
     * The recv callback, and associated metadata parameters, are not valid for all
     * multicombines. Depending on the kind of multicombine being registered, it may
     * require that the recv callback be either NULL or valid. If the recv callback
     * is NULL then the metadata parameters should also be NULL (0) when inoking
     * the multicombine.
     *
     * data and results parameters may not always be required, depending on role (and other?).
     * For example, if a call the a multicombine specifies a single role of, say, "injection",
     * then the results parameters are not needed. Details of this are specified by the
     * type of multicombine being registered/used.
     */
    typedef struct {
        size_t              req_size;	              /**< space available in request, bytes */
        xmi_event_function  cb_done;	              /**< User's completion callback */
        unsigned            roles;		      /**< bitmap of roles to perform */
        xmi_pipeworkqueue_t *data;		      /**< Data source */
        xmi_topology_t      *data_participants;	      /**< Ranks contributing data */
        xmi_pipeworkqueue_t *results;	              /**< Results destination */
        xmi_topology_t      *results_participants;    /**< Ranks receiving results */
        xmi_op               optor;		      /**< Operation to perform on data */
        xmi_dt               dtype;		      /**< Datatype of elements */
        size_t              count;		      /**< Number of elements */
    } xmi_multicombine_t;
    
    /**
     * \brief Allreduce, Reduce, etc. (may include some specialized Broadcasts, too)
     *
     * All participants make this call. So, there is no "send" or "recv"
     * distinction needed. Send and/or recv are determined by calling rank's
     * membership in respective Topologies, as well as requirements of underlying
     * hardware.
     *
     * All participants == {data_participants .U. results_participants}.
     *
     * \param[in] mcombineinfo	Struct of all params needed to perform operation
     * \return	XMI_SUCCESS or error codes
     */
    xmi_result_t XMI_Multicombine(xmi_context_t context,xmi_multicombine_t *mcombineinfo);

#ifdef __cplusplus
};
#endif

#endif /* __xmi_multisend_h__ */
