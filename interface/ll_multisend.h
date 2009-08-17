/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file ll_multisend.h
 * \brief Multisend interface.
 *    A multisend operation allows many message passing transactions
 * 	to be performed in the same call to amortize software
 *    overheads.  It has two flavors
 *    - multicast, which sends the same buffer to a list of
 *	  processors and also supports depost-bit torus line broadcasts
 *	- manytomany, which sends different offsets from the same
 *      buffer to a list of processors
 *
 *      As reductions have a single source buffer and destination
 *	  buffer, we have extended the multicast call to support
 *	  reductions.
 *
 * Each multisend operation carries a connection id to identify the
 * data on the reciever.
 *
 * In a multisend operation the cores are typically involved at the
 * end-points. Moreover the processor lists can be created on the
 * fly.
 */
#ifndef __ll_multisend_h__
#define __ll_multisend_h__

#include "cm_types.h"
#include "ll_topology.h"
#include "ll_pipeworkqueue.h"

// These are now defined in cm_impl.h (via cm_types.h), according to target needs
// #define DEPRECATED_MULTICAST
// #define DEPRECATED_MANYTOMANY

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
  CM_Result LL_Multisend_getRoles(CM_Protocol_t *registration,
					int *numRoles, int *replRole);

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
   * \param[in] root		Sending rank
   * \param[in] sndlen		Length of data sent
   * \param[in] clientdata	Opaque arg
   * \param[out] rcvlen		Length of data to receive
   * \param[out] rcvbuf		Where to put recv data
   * \param[out] cb_done	Completion callback to invoke when data received
   * \return	CM_Request opaque memory for message
   */
  typedef CM_Request_t *(*LL_RecvMulticast)(const CMQuad *msginfo,
                                                unsigned msgcount,
                                                size_t root,
                                                size_t sndlen,
                                                void *clientdata,
                                                size_t *rcvlen,
                                                LL_PipeWorkQueue_t **rcvbuf,
                                                CM_Callback_t *cb_done);

#ifdef DEPRECATED_MULTICAST
  typedef CM_Request_t * (*DCMF_OldRecvMulticast) (const CMQuad    * info, /* DEPRECATED */
                                                  unsigned          count,
                                                  unsigned          peer,
                                                  unsigned          sndlen,
                                                  unsigned          conn_id,
                                                  void            * arg,
                                                  unsigned        * rcvlen,
                                                  char           ** rcvbuf,
                                                  unsigned        * pipewidth,
                                                  CM_Callback_t * cb_done);



  /**
   * Note, opcodes are not used in the new interface!
   *
   */
/** \brief Mask constant that contains all possible line bcast variations */
#define DCMF_LINE_BCAST_MASK	(DCMF_LINE_BCAST_XP|DCMF_LINE_BCAST_XM| /* DEPRECATED */\
                                 DCMF_LINE_BCAST_YP|DCMF_LINE_BCAST_YM| /* DEPRECATED */\
                                 DCMF_LINE_BCAST_ZP|DCMF_LINE_BCAST_ZM) /* DEPRECATED */

  /**
   * \brief ???
   * \Warning Any changes to this enum must also be applied to
   * Schedule::Subtask in interface/ccmi_internal.h 
   */
  typedef enum {
    DCMF_PT_TO_PT_SEND         =  0,	/* DEPRECATED */
    DCMF_LINE_BCAST_XP         =  0x20,   /**< Bcast along x+ */ /* DEPRECATED */
    DCMF_LINE_BCAST_XM         =  0x10,   /**< Bcast along x- */ /* DEPRECATED */
    DCMF_LINE_BCAST_YP         =  0x08,   /**< Bcast along y+ */ /* DEPRECATED */
    DCMF_LINE_BCAST_YM         =  0x04,   /**< Bcast along y- */ /* DEPRECATED */
    DCMF_LINE_BCAST_ZP         =  0x02,   /**< Bcast along z+ */ /* DEPRECATED */
    DCMF_LINE_BCAST_ZM         =  0x01, /* DEPRECATED */
    DCMF_COMBINE               =  0x0100, /**< Not used by msend? */ /* DEPRECATED */
    DCMF_GI_BARRIER            =  0x0200, /* DEPRECATED */
    DCMF_LOCKBOX_BARRIER       =  0x0300, /* DEPRECATED */
    DCMF_TREE_BARRIER          =  0x0400, /* DEPRECATED */
    DCMF_TREE_BCAST            =  0x0500, /* DEPRECATED */
    DCMF_TREE_ALLREDUCE        =  0x0600, /* DEPRECATED */
    DCMF_REDUCE_RECV_STORE     =  0x0700, /* DEPRECATED */
    DCMF_REDUCE_RECV_NOSTORE   =  0x0800, /* DEPRECATED */
    DCMF_BCAST_RECV_STORE      =  0x0900, /* DEPRECATED */
    DCMF_BCAST_RECV_NOSTORE    =  0x0a00, /* DEPRECATED */
    DCMF_LOCALCOPY             =  0x0b00, /* DEPRECATED */

    CM_UNDEFINED_OPCODE      =  (~DCMF_LINE_BCAST_MASK), /* DEPRECATED */
  } CM_Opcode_t;
#endif /* DEPRECATED_MULTICAST */

  /**
   * \brief Multi Send protocol implementations.
   */

  typedef enum {
    LL_DEFAULT_MULTICAST_PROTOCOL = 0,

#ifdef DEPRECATED_MULTICAST
/**************** obsolete, legacy enums ****************/
    DCMF_DEFAULT_MSEND_PROTOCOL=LL_DEFAULT_MULTICAST_PROTOCOL,         /**< Default multicast. */ /* DEPRECATED */
    DCMF_MEMFIFO_DMA_MSEND_PROTOCOL, /* DEPRECATED */      /**< Memory fifo multicast,
					     pipeline width should be
					     a multiple of 240 bytes.*/
    DCMF_GI_MSEND_PROTOCOL,  /* DEPRECATED */              /**< GI barrier */
    DCMF_TREE_MSEND_PROTOCOL,  /* DEPRECATED */            /**< Multicast/reduce on the
					     collective network. The
					     pipeline width should be
					     a multiple of 256 bytes.
					     A combined memfifo and
					     tree protocol will need
					     the pipeline width to be
					     3840 bytes. */
    DCMF_DPUT_DMA_MSEND_PROTOCOL, /* DEPRECATED */	  /**< Direct put multicast,
					     requires a receive to be
					     posted on the
					     connection. The pipeline
					     width should be a
					     multiple of 240 bytes. */
/**************** all of the above are obsolete. ****************/
#endif /* DEPRECATED_MULTICAST */
    LL_GLOBAL_MULTICAST_PROTOCOL,
    LL_INTRANODE_MULTICAST_PROTOCOL,
    LL_MFIFO_LINEBCAST_MULTICAST_PROTOCOL,
    LL_DPUT_LINEBCAST_MULTICAST_PROTOCOL,
    /* add more protocols here */
    LL_MULTICAST_PROTOCOL_COUNT,

#ifdef DEPRECATED_MULTICAST
    /**************** this is also obsolete. ****************/
    DCMF_TREE_DPUT_MSEND_PROTOCOL = LL_MULTICAST_PROTOCOL_COUNT /* DEPRECATED */
#endif /* DEPRECATED_MULTICAST */
  } LL_Multicast_Protocol;

  /**
   * \brief Multicast configuration. The connection ids here go from
   * 0 - (nconnections-1).
   */
  typedef struct {
    LL_Multicast_Protocol protocol;   /**< The multicast protocol to initialize */
    LL_RecvMulticast      cb_recv;    /**< Async callback */
    void                 *clientdata; /**< Callback clientdata */
#ifdef DEPRECATED_MULTICAST
    DCMF_OldRecvMulticast        cb_oldrecv;   /**< Async callback */ /* DEPRECATED */
    void                       **connectionlist; /**< connection list vector */
    unsigned                     nconnections;  /**< number of connections */
#endif /* DEPRECATED_MULTICAST */
  } LL_Multicast_Configuration_t;


  CM_Result LL_Multicast_register (CM_Protocol_t                *registration,
				   LL_Multicast_Configuration_t *configuration);

#ifdef DEPRECATED_MULTICAST
#warning Exposing Deprecated Multicast interfaces
  /**
   * \brief A structure to pass parameters for the multisend multicast
   * operation.
   *
   * This is overly complex due to temporary need to provide backward-compatibility
   * while the old multicast interface and hacks are being obsoleted and removed.
   * This will eventually be compressed into a single, flat, structure (Multicast2 only).
   */

  typedef struct {
    CM_Protocol_t    *registration;     /**< Pointer to registration */
    CM_Request_t    * request;         /**< Temporary storage for the
                                              multicast message */
    CM_Callback_t     cb_done;         /**< Completion callback */
    DCMF_Consistency    consistency;     /**< Consistency model */ /* DEPRECATED */
    unsigned            connection_id;	 /**< A connection is a distinct stream of
					      traffic. The connection id
					      identifies the connection */
    unsigned            bytes;		 /**< size of the message*/
    const char        * src;		 /**< source buffer */
    unsigned            nranks;		 /**< number of destinations
                                              to multicast to*/
    unsigned          * ranks;		 /**< Array of destination ranks*/
    CM_Opcode_t     * opcodes;         /**< A hardare hint to do a
                                              specific operation. For
                                              example, send a point to
                                              point message or send a
                                              deposit bit broadcast along a line*/
    const CMQuad      * msginfo;         /**< A extra info field to be
                                              sent with the message*/
    unsigned            count;           /**< info count*/
    CM_Op             op;              /**< Operation for a reduce*/
    CM_Dt             dt;              /**< Data type for the reduce */
    unsigned            flags;           /**< A flags parameter to
                                              configure special options in
                                              the multicast. For example
                                              enable or disable
                                              checksumming. */
  } DCMF_OldMulticast_t; /* DEPRECATED */

  /**
   * \brief Multisend multicast/reduce call
   */

  int DCMF_OldMulticast (DCMF_OldMulticast_t   *mcastinfo); /* DEPRECATED */


  /**
   * \brief A structure to pass parameters for the multisend multicast
   * post recv operation.
   *
   * Note, postRecv is not used in the new interface!
   */

  typedef struct {
    CM_Protocol_t *registration;  /**< Pointer to registration */
    CM_Request_t  *request;       /**< Temporary storage for the
					    multisend message */
    CM_Callback_t  cb_done;       /**< Completion callback */
    unsigned       connection_id; /**< A connection is a
					    distinct stream of
					    traffic. The connection id
					    identifies the
					    connection */
    unsigned       bytes;	       /**< size of the message*/
    char          *dst;	       /**< source buffer */
    unsigned            pwidth;        /**< pipelining parameter */
    CM_Opcode_t       opcode;        /**< A hardare hint to do a
					    specific operation. For
					    example, send a point to
					    point message or send a
					    deposit bit broadcast along a line*/
    CM_Op             op;	       /**< Operation for a reduce*/
    CM_Dt             dt;            /**< Data type for the reduce */
  } DCMF_OldMulticastRecv_t; /* DEPRECATED */


  /**
   * \brief Multisend multicast post recv call. The post recv call is
   * an alternative to asynchronous callbacks. It is also required to
   * support synchronous devices where a receive does a setup before
   * the data can arrive. The Post recevie should typically be called
   * before the send is initiated. There can be only one post receive
   * per connection and no matching is performed at this level.
   */

  int DCMF_OldMulticast_postrecv (DCMF_OldMulticastRecv_t    *recv); /* DEPRECATED */
#endif /* DEPRECATED_MULTICAST */

  /**
   * \brief The new structure to pass parameters for the multisend multicast operation.
   *
   * The LL_Multicast_t object is re-useable immediately, but objects referred to
   * (src, etc) cannot be re-used until cb_done.
   */
  typedef struct {
    CM_Protocol_t      *registration;	/**< Pointer to registration */
    void               *request;	/**< Temporary storage for the multicast message */
    size_t              req_size;	/**< space available in request, bytes */
    CM_Callback_t       cb_done;	/**< Completion callback */
    unsigned            connection_id;	/**< A connection is a distinct stream of
					     traffic. The connection id identifies the
					     connection */
    unsigned            roles;		/**< bitmap of roles to perform */
    size_t              bytes;		/**< size of the message*/
    LL_PipeWorkQueue_t *src;		/**< source buffer */
    LL_Topology_t      *src_participants; /**< root */
    LL_PipeWorkQueue_t *dst;		/**< dest buffer (ignored for one-sided) */
    LL_Topology_t      *dst_participants; /**< destinations to multicast to*/
    const CMQuad       *msginfo;	/**< A extra info field to be sent with the message.
					     This might include information about
					     the data being sent, for one-sided. */
    unsigned            msgcount;	/**< info count*/
  } LL_Multicast_t;

  int LL_Multicast(LL_Multicast_t *mcastinfo);


#ifdef DEPRECATED_MANYTOMANY
#warning Exposing Deprecated ManyToMany interfaces
  /**********************************************************************************
   *       Old Many To Many Personalized communication
   * \todo convert to new interface!
   **********************************************************************************/
  /**
   * \brief Callback for Manytomany Receive operations
   *
   * \warning After registering the protocol implementation it is illegal to
   *          deallocate the registration object.
   *
   * \param[in]  rankIndex - used to identify which array index this rank is in
   *                         the passed in arrays (may not be equal to the rank
   *                         number).  A value of 0xffffffff means that receiver
   *                         does not receive and should not be used.
   */

  typedef CM_Request_t * (*DCMF_RecvManytomany) (unsigned          conn_id, /* DEPRECATED */
                                                   void            * arg,
                                                   char           ** rcvbuf,
                                                   unsigned       ** rcvlens,
                                                   unsigned       ** rcvdispls,
                                                   unsigned       ** rcvcounters,
                                                   unsigned        * nranks,
                                                   unsigned        * rankIndex,
                                                   CM_Callback_t * cb_done);

  /**
   * \brief Manytomany protocol implementations.
   */
  typedef enum
  {
    DCMF_DEFAULT_M2M_PROTOCOL=0,         /**< Default point-to-point send. */ /* DEPRECATED */
    DCMF_MEMFIFO_DMA_M2M_PROTOCOL        /**< Number of allowed connections is 32 */ /* DEPRECATED */
  }
  DCMF_Manytomany_Protocol; /* DEPRECATED */

  /**
   * \brief Many to many configuration.
   */
  typedef struct
  {
    DCMF_Manytomany_Protocol      protocol; /* DEPRECATED */
    DCMF_RecvManytomany           cb_recv;  /* DEPRECATED */
    void                        * arg;
    unsigned                      nconnections;
  }
  DCMF_Manytomany_Configuration_t; /* DEPRECATED */


  CM_Result DCMF_Manytomany_register (CM_Protocol_t                 * registration, /* DEPRECATED */
                                        DCMF_Manytomany_Configuration_t * configuration); /* DEPRECATED */

  int DCMF_Manytomany         (CM_Protocol_t   * registration, /* DEPRECATED */
                               CM_Request_t    * request,
                               CM_Callback_t     cb_done,
                               DCMF_Consistency    consistency, /* DEPRECATED */
                               unsigned            connection_id,
                               unsigned            rindex,
                               const char              * sndbuf,
                               unsigned          * sndlens,
                               unsigned          * sdispls,
                               unsigned          * sndcounters,
                               unsigned          * ranks,
                               unsigned          * permutation,
                               unsigned            nranks);
#endif /* DEPRECATED_MANYTOMANY */

  /**
   * \brief Manytomany protocol implementations.
   */
  typedef enum {
    LL_DEFAULT_M2M_PROTOCOL=0,	/**< Default point-to-point send. */
    LL_M2M_PROTOCOL_COUNT	/**< Total number of protocols supported */
  } LL_Manytomany_Protocol;

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
    LL_PipeWorkQueue_t *buffer;		/**< Memory used for data (buffer) */
    LL_Topology_t *participants;	/**< Ranks that are vectored in buffer */
    size_t *lengths;			/**< Array of lengths in buffer for each rank */
    size_t *offsets;			/**< Array of offsets in buffer for each rank */
    size_t num_vecs;                    /**< The number of entries in "lengths" and
                                             "offsets". May be a flag: either "1" or
                                             participants->size(). */
  } LL_ManytomanyBuf_t;

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
   * \param[in] metacount	Number of CMQuads of metadata.
   * \param[out] recv		Receive parameters for this connection (instance)
   * \param[out] myIndex	Index of Recv Rank in the receive parameters.
   * \param[out] cb_done	Completion callback when message complete
   * \return	Request object opaque storage for message.
   */
  typedef CM_Request_t *(*LL_RecvManytomany)(void *arg,
					     unsigned conn_id,
					     CMQuad *metadata,
					     unsigned metacount,
					     LL_ManytomanyBuf_t **recv,
					     size_t *myIndex,
                                             CM_Callback_t *cb_done);

  /**
   * \brief Many to many configuration.
   */
  typedef struct {
    LL_Manytomany_Protocol protocol;	/**< Protocol (ManyToMany flavor) to register */
    LL_RecvManytomany      cb_recv;	/**< Recv callback to use */
    void                  *arg;		/**< Opaque arg to include in callback */
  } LL_Manytomany_Configuration_t;

  /**
   * \brief Register a ManyToMany "protocol" (flavor)
   *
   * Returns error if type of ManyToMany requires/disallows cb_recv and
   * cb_recv is/not NULL as appropriate.
   *
   * \param[out] registration	Opaque memory for ManyToMany class object.
   * \param[in] configuration	Configuration of ManyToMany to register/construct.
   * \return	CM_SUCCESS or error code.
   */
  CM_Result LL_Manytomany_register(CM_Protocol_t *registration,
                                   LL_Manytomany_Configuration_t *configuration);

  /**
   * \brief Structure of parameters used to initiate a ManyToMany
   *
   * The rankIndex parameter is transmitted to the receiver for use by cb_recv
   * for indexing into the recv parameter arrays (lengths and offsets).
   */
  typedef struct {
    CM_Protocol_t      *registration;	/**< Opaque registration object */
    void               *request;	/**< Opaque request object */
    size_t              req_size;	/**< space available in request, bytes */
    CM_Callback_t       cb_done;	/**< User's completion callback */
    unsigned            connection_id;	/**< differentiate data streams */
    unsigned            roles;		/**< bitmap of roles to perform */
    size_t              *rankIndex;	/**< Index of send in recv parameters */
    size_t              num_index;      /**< Number of entries in "rankIndex".
                                             should be multiple of send.participants->size()?
                                         */
    LL_ManytomanyBuf_t  send;		/**< send data parameters */
    const CMQuad       *metadata;	/**< A extra info field to be sent with the message.
					     This might include information about
					     the data being sent, for one-sided. */
    unsigned            metacount;	/**< metadata count*/
  } LL_Manytomany_t;

  /**
   * \brief Initiate a ManyToMany
   *
   * \param[in] m2minfo	Paramters for ManyToMany operation to be performed
   * \return	CM_SUCCESS or error code
   */
  CM_Result LL_Manytomany(LL_Manytomany_t *m2minfo);

  /******************************************************************************
   *       Multisync Personalized synchronization/coordination
   ******************************************************************************/

  /**
   * \brief protocols supported for Multisync
   */
  typedef enum {
    LL_DEFAULT_MULTISYNC_PROTOCOL = 0,
    LL_GLOBAL_MULTISYNC_PROTOCOL,
    LL_INTRANODE_MULTISYNC_PROTOCOL,
    LL_LOCKMANAGER_MULTISYNC_PROTOCOL,
    /* other protocols added here */
    LL_MULTISYNC_PROTOCOL_COUNT
  } LL_Multisync_protocol_t;

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
   * \param[in] msgcount	Number of CMQuads in msginfo
   * \param[in] conn_id		Instance ID
   * \return	CM_Request opaque memory for message
   */
  typedef CM_Request_t *(*LL_RecvMultisync)(void *clientdata,
					    CMQuad *msginfo,
					    unsigned msgcount,
					    unsigned conn_id);

  /**
   * \brief configuration interface for registering Multisync
   *
   * \todo Is there such a thing as a one-sided barrier?
   */
  typedef struct {
    LL_Multisync_protocol_t   protocol;		/**< The protocol to register */
    LL_RecvMultisync         *cb_recv;		/**< Recv callback */
    void                     *clientdata;	/**< Opaque arg for callback */
    /* additional configuration fields here */
  } LL_Multisync_configuration_t;

  /**
   * \brief structure defining interface to Multisync
   */
  typedef struct {
    CM_Protocol_t   *registration;	/**< Opaque registration object */
    void            *request;		/**< Opaque request object */
    size_t           req_size;		/**< space available in request, bytes */
    CM_Callback_t    cb_done;		/**< User's completion callback */
    unsigned         connection_id;	/**< (remove?) differentiate data streams */
    unsigned         roles;		/**< bitmap of roles to perform */
    LL_Topology_t   *participants;	/**< Ranks involved in synchronization */
  } LL_Multisync_t;

  /**
   * \brief Register a Multi-sync protocol
   *
   * \param[out] registration	Opaque memory used to construct protocol object
   * \param[in] configuration	Specifics of registration configuration
   * \return	CM_SUCCESS or error codes
   */
  CM_Result LL_Multisync_register(CM_Protocol_t   *registration,
                         LL_Multisync_configuration_t *configuration);

  /**
   * \brief Barriers and the like.
   *
   * All participants make this call. So, there is no "send" or "recv"
   * distinction needed.
   *
   * \param[in] msyncinfo	Struct of all params needed to perform operation
   * \return	CM_SUCCESS or error codes
   */
  CM_Result LL_Multisync(LL_Multisync_t *msyncinfo);


  /******************************************************************************
   *       Multicombine Personalized reduction
   ******************************************************************************/

  /**
   * \brief protocols supported for Multicombine
   */
  typedef enum {
    LL_DEFAULT_MULTICOMBINE_PROTOCOL = 0,
    LL_GLOBAL_MULTICOMBINE_PROTOCOL,
    LL_INTRANODE_MULTICOMBINE_PROTOCOL,
    /* add more protocols here */
    LL_MULTICOMBINE_PROTOCOL_COUNT
  } LL_Multicombine_protocol_t;

  /**
   * \brief configuration interface for registering Multicombine
   *
   * Note, reply_proto is a protocol structure used by the protocol being registered
   * to establish a data path for target replies. This protocol is setup by the
   * registration, not by the caller of LL_Multicombine_register.
   */
  typedef struct {
    LL_Multicombine_protocol_t  protocol;	/**< The protocol to register */
    /* add more configuration fields here */
  } LL_Multicombine_configuration_t;

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
    CM_Protocol_t      *registration;   /**< Pointer to registration */
    void               *request;        /**< Temporary storage for the multi* */
    size_t              req_size;	/**< space available in request, bytes */
    CM_Callback_t       cb_done;	/**< User's completion callback */
    unsigned            roles;		/**< bitmap of roles to perform */
    LL_PipeWorkQueue_t *data;		/**< Data source */
    LL_Topology_t      *data_participants;	/**< Ranks contributing data */
    LL_PipeWorkQueue_t *results;	/**< Results destination */
    LL_Topology_t      *results_participants;	/**< Ranks receiving results */
    CM_Op               optor;		/**< Operation to perform on data */
    CM_Dt               dtype;		/**< Datatype of elements */
    size_t              count;		/**< Number of elements */
  } LL_Multicombine_t;

  /**
   * \brief Register a Multi-combine protocol
   *
   * \param[out] registration	Opaque memory used to construct protocol object
   * \param[in] configuration	Specifics of registration configuration
   * \return	CM_SUCCESS or error codes
   */
  CM_Result LL_Multicombine_register(CM_Protocol_t   *registration,
                         LL_Multicombine_configuration_t *configuration);

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
   * \return	CM_SUCCESS or error codes
   */
  CM_Result LL_Multicombine(LL_Multicombine_t *mcombineinfo);

#ifdef __cplusplus
};
#endif

#endif /* __ll_multisend_h__ */
