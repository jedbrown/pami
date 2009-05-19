
/** \file include/ll_multisend.h
 * \brief Multisend interface.
 *    A multisend operation allows many message passing transactions
 * 	to be performed in the same call to amortize software
 *    overheads.  It has two flavors
 *    - multicast, which sends the same buffer to a list of
 *	  processors and also supports deposit-bit torus line broadcasts
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

#include "./ll_collectives_base.h"

#ifdef __cplusplus
extern "C"
{
#endif


    /* ************************************************************************* */
    /* **************     ALGORITHMS  ****************************************** */
    /* ************************************************************************* */

    typedef enum
    {
        LL_DEFAULT_MCAST_PROTOCOL=0,   /**< Default multicast. */
        LL_MEMFIFO_DMA_MCAST_PROTOCOL, /**< Memory fifo multicast, BG/P specific */
        LL_TREE_MCAST_PROTOCOL,        /**< Multicast/reduce on the collective network. */
        LL_TREE_DPUT_MCAST_PROTOCOL,   /**< not used. BG/P specific */
        LL_MCAST_PROTOCOL_COUNT
    } LL_Multicast_Protocol;

    typedef enum
    {
        LL_DEFAULT_M2M_PROTOCOL=0,       /**< Default point-to-point send. */
        LL_MEMFIFO_DMA_M2M_PROTOCOL,     /**< DMA MemFifo - BG/P specific  */
        LL_M2M_PROTOCOL_COUNT
    } LL_Manytomany_Protocol;

    typedef enum
    {
        LL_DEFAULT_MULTICOMBINE_PROTOCOL = 0,
        LL_GLOBAL_MULTICOMBINE_PROTOCOL,
        LL_INTRANODE_MULTICOMBINE_PROTOCOL,
        LL_MULTICOMBINE_PROTOCOL_COUNT
    } LL_Multicombine_protocol_t;

    typedef enum
    {
        LL_DEFAULT_MULTISYNC_PROTOCOL = 0,
        LL_GLOBAL_MULTISYNC_PROTOCOL,
        LL_INTRANODE_MULTISYNC_PROTOCOL,
        LL_LOCKMANAGER_MULTISYNC_PROTOCOL,
        LL_MULTISYNC_PROTOCOL_COUNT
    } LL_Multisync_protocol_t;


    /* *********************************************************************************** */
    /* **************    "Special Callback Types Objects  ******************************** */
    /* *********************************************************************************** */
    /**
     * \brief Callback for a received multicast message
     *
     * The receive-side takes available information and determines appropriate
     * values for rcvlen and rcvbuf.  That may include consulting a PipeWorkQueue.
     * Complications ensue if that workqueue is circular and does not contain
     * enough space for the receive, as we are committed at this point to finish
     * the receive. Theoretically, protocols that permit a circular workqueue
     * would have to setup a temporary buffer and copy the data into the workqueue later.
     * This could be aleviated by "smart" transports that stall receives (packets) until the
     * destination workqueue has enough space.
     *
     * \param[in] info	Protocol info supplied by sender
     * \param[in] count	Number of quads in protocol info
     * \param[in] peer	Rank of sender
     * \param[in] sndlen	Amount of data received
     * \param[in] conn_id Stream data belongs to
     * \param[in] clientdata	Opaque data setup at registration
     * \param[out] rcvlen	Amount of data to actually receive
     * \param[out] rcvbuf	Location to store received data
     * \param[out] cb_done Callback to be invoked when all data is received
     */
    typedef LL_Request_t * (*LL_RecvMulticast) (const Quad      * info,
                                                unsigned          count,
                                                unsigned          peer,
                                                unsigned          sndlen,
                                                unsigned          conn_id,
                                                void            * clientdata,
                                                unsigned        * rcvlen,
                                                char           ** rcvbuf,
                                                LL_Callback_t   * cb_done);
    /**
     * \brief Callback for Manytomany Receive operations
     *
     * \warning After registering the protocol implementation it is illegal to
     *          deallocate the registration object.
     *
     * NOTE, this receive callback is designed for AllToAll operations.
     * It really needs to be simplified (maybe identical to multicast recv)
     * and the specifics of AllToAll kept in the upper layers of protocol(s).
     * Each LL_RecvManytomany callback represents the start of a message
     * from a given rank. So the callback should have enough information to
     * determine a unique location in which to store the data. None of the
     * rcvlens, rcvdispls, rcvcounters, nranks, rankIndex need be exposed to
     * this layer.
     *
     * i.e. typedef LL_RecvMulticast LL_RecvManytomany;
     *
     * ***** FIX THIS! *****
     * \param[in] conn_id 	Stream data belongs to
     * \param[in] clientdata	Opaque pointer setup at registration
     * \param[out] rcvbuf		Location to store received data
     * \param[out] rcvlens	Amount of data to actually receive from each rank
     * \param[out] rcvdispls	Displacements into rcvbuf of data from each rank
     * \param[out] rcvcounters	Specialized info for reception
     * \param[out] nranks		Number of ranks to receive from
     * \param[out] rankIndex	Position of local rank in above arrays
     * \param[out] cb_done 	Callback to be invoked when all data is received
     */
    typedef LL_Request_t * (*LL_RecvManytomany) (unsigned          conn_id,
                                                 void            * clientdata,
                                                 LL_PipeWorkQueue_t ** rcvbuf,
                                                 unsigned       ** rcvlens,
                                                 unsigned       ** rcvdispls,
                                                 unsigned       ** rcvcounters,
                                                 unsigned        * nranks,
                                                 unsigned        * rankIndex,
                                                 LL_Callback_t * cb_done);

    /* ************************************************************************* */
    /* **************    Configuration Objects  ******************************** */
    /* ************************************************************************* */
    typedef struct
    {
        LL_Config_t               cfg_type;
        LL_Multicast_Protocol     protocol;       /**< The multicast protocol to initialize*/
        LL_RecvMulticast          cb_recv;        /**< Async callback */
        void                     *clientdata;     /**< Callback clientdata */
    } LL_Multicast_Configuration_t;

    typedef struct
    {
        LL_Config_t               cfg_type;
        LL_Manytomany_Protocol    protocol;       /**< protocol to be registered */
        LL_RecvManytomany         cb_recv;        /**< receive callback */
        void                     *clientdata;     /**< opaque data for recv callback */
        unsigned                  nconnections;   /**< number of connections used */
    } LL_Manytomany_Configuration_t;

    typedef struct
    {
        LL_Config_t               cfg_type;
        LL_Multisync_protocol_t   protocol;       /**< The protocol to register */
    } LL_Multisync_Configuration_t;

    typedef struct
    {
        LL_Config_t                    cfg_type;
        LL_Multicombine_protocol_t     protocol;       /**< The protocol to register */
        void                          *clientdata;     /**< Callback clientdata */
        void                         **connectionlist; /**< connection list vector */
    } LL_Multicombine_Configuration_t;

    typedef union
    {
        LL_Config_t                       cfg_type;
        LL_Multicast_Configuration_t      cfg_multicast;
        LL_Manytomany_Configuration_t     cfg_manytomany;
        LL_Multisync_Configuration_t      cfg_multisync;
        LL_Multicombine_Configuration_t   cfg_multicombine;
    }   LL_CollectiveConfiguration_t;

    int LL_register(LL_CollectiveProtocol_t      *registration,
                    LL_CollectiveConfiguration_t *configuration);




    /* *********************************************************************************** */
    /* **************    "Transfer Routines and Objects   ******************************** */
    /* *********************************************************************************** */

    /**
     * \file include/ll_multisend.h
     * \brief Multisend interface.
     *
     *    A multisend operation allows many message passing transactions
     * 	to be performed in the same call to amortize software
     *    overheads.  It has four flavors:
     *
     *    - multicast, which sends the same buffer to a list of
     *	  destinations (also supports ioptimizations such as deposit-bit
     *      torus line broadcasts).
     *
     *	- manytomany, which sends different offsets from the same
     *      buffer to a list of destinations.
     *
     *    - multisync, which does synchronization (e.g. barrier) of participants.
     *
     *    - multicombine, which does reduction-like operations (e.g. allreduce)
     *      over a set of data participants, with results to a (possibly different)
     *      set of results participants.
     *
     * Each multisend operation carries a connection id to identify the
     * data on the reciever.
     *
     * In a multisend or manytomany operation the end-points must post receive(s).
     * Moreover the processor lists can be created on the fly.
     */

    /**
     * \brief A structure to pass parameters for the multisend multicast
     * operation.
     *
     */
    typedef struct
    {
        LL_CollectiveProtocol_t  *registration;     /**< Pointer to registration */
        LL_Request_t             *request;          /**< Temporary storage for the multicast message */
        LL_Callback_t             cb_done;          /**< Completion callback */
        unsigned                  connection_id;    /**< A connection is a distinct stream of data */
	unsigned                  roles;            /**< bitmap of roles to perform */
        unsigned                  bytes;            /**< size of the message */
        LL_PipeWorkQueue_t       *src;              /**< source buffer */
        LL_Topology_t            *src_participants; /**< destinations to multicast to*/
	LL_PipeWorkQueue_t       *dst;              /**< dest buffer (ignored for one-sided) */
	LL_Topology_t            *dst_participants; /**< destinations to multicast to*/
        const Quad               *msginfo;          /**< A extra info field to be send with the msg */
        unsigned                  msgcount;	    /**< info count */
    } LL_Multicast_t;

    /**
     * \brief structure defining interface to Manytomany.
     *
     * NOTE, sndlens and sdispls probably should be done differently.
     * sndbuf,sndlens,sdispls could be replaced by array of PipeWorkQueues
     * (but the requires a very large array for global or large-communicator broadcasts)
     * or possibly a special "vectored" PipeWorkQueue. Still would need some extra info
     * so we know when the operation is done - right now the sum of all sndlens[*]
     * is the total amount of data sent (i.e. "done"). Perhaps PipeWorkQueue needs
     * an EOF indicator, but that adds complexity to them.
     */
    typedef struct
    {
        LL_CollectiveProtocol_t       *registration;    /**< Opaque registration object */
        LL_Request_t        *request;	      /**< Opaque request object */
        LL_Callback_t        cb_done;	      /**< User's completion callback */
        unsigned             connection_id;   /**< (remove?) differentiate data streams */
        LL_PipeWorkQueue_t  *sndbuf;	      /**< Data source */
        unsigned            *sndlens;	      /**< lengths of chunks */
        unsigned            *sdispls;	      /**< displacements of chunks */
        LL_Topology_t       *participants;    /**< Ranks to receive chunks */
        unsigned            *permutation;     /**< Mapping of Ranks to chunks */
    } LL_Manytomany_t;


    /**
     * \brief structure defining interface to Multicombine
     */
    typedef struct
    {
        LL_CollectiveProtocol_t       *registration;         /**< Pointer to registration */
        LL_Request_t        *request;              /**< Temporary storage for the multi* */
        LL_Callback_t        cb_done;	           /**< User's completion callback */
        unsigned             connection_id;	   /**< (remove?) differentiate data streams */
        unsigned             roles;		   /**< bitmap of roles to perform */
        LL_PipeWorkQueue_t  *data;		   /**< Data source */
        LL_Topology_t       *data_participants;	   /**< Ranks contributing data */
        LL_PipeWorkQueue_t  *results;	           /**< Results destination */
        LL_Topology_t       *results_participants; /**< Ranks receiving results */
        LL_Op                op;	           /**< Operation to perform on data */
        LL_Dt                datatype;	           /**< Datatype of elements */
        unsigned             count;		   /**< Number of elements */
        const Quad          *msginfo;              /**< A extra info field to be send */
        unsigned             msgcount;             /**< info count*/
    } LL_Multicombine_t;

    /**
     * \brief structure defining interface to Multisync
     */
    typedef struct {
        LL_CollectiveProtocol_t   * registration;	/**< Opaque registration object */
        LL_Request_t    * request;	/**< Opaque request object */
        LL_Callback_t     cb_done;	/**< User's completion callback */
        unsigned          connection_id;	/**< (remove?) differentiate data streams */
        LL_Topology_t   * participants;	/**< Ranks involved in synchronization */
    } LL_Multisync_t;


    typedef union
    {
        LL_Multisync_t     xfer_msync;
        LL_Multicombine_t  xfer_mcombine;
        LL_Multicast_t     xfer_mcast;
        LL_Manytomany_t    xfer_mtom;
    }LL_xfer_t;


    /**
     * \brief Multi-cast call
     *
     * \param[in] mcastinfo	Struct of all params needed to perform operation
     * \return	LL_SUCCESS or error codes
     */
    LL_Result LL_Multicast(LL_Multicast_t *mcastinfo);

    /**
     * \brief Manytomany communication
     *
     * Sends a different chunk of data to each destination.
     *
     * \param[in] m2minfo	Struct of all params needed to perform operation
     * \return	LL_SUCCESS or error codes
     */
    LL_Result LL_Manytomany(LL_Manytomany_t *m2minfo);

    /**
     * \brief Barriers and the like.
     *
     * All participants make this call. So, there is no "send" or "recv"
     * distinction needed.
     *
     * \param[in] msyncinfo	Struct of all params needed to perform operation
     * \return	LL_SUCCESS or error codes
     */
    LL_Result LL_Multisync(LL_Multisync_t *msyncinfo);

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
     * \return	LL_SUCCESS or error codes
     */
    LL_Result LL_Multicombine(LL_Multicombine_t *mcombineinfo);

    int LL_Xfer (void* context, LL_xfer_t cmd);


    /* *********************************************************************************** */
    /* **************    "Roles"   ******************************************************* */
    /* *********************************************************************************** */
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
    LL_Result LL_Multisend_getRoles(LL_CollectiveProtocol_t *registration,
                                    int           *numRoles,
                                    int           *replRole);

#ifdef __cplusplus
};
#endif

#endif /* __ll_multisend_h__ */
