/**
 * \file hl_collectives.h
 * \brief Common external collective layer interface.
 */

#ifndef __hl_collective_h__
#define __hl_collective_h__

#include "cm_types.h"
#include "ccmi_internal.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /* ************************************************************************* */
    /* **************     ALGORITHMS  ****************************************** */
    /* ************************************************************************* */
    /**
     * \brief Supported Algorithm Types:  Not Complete, add as many algorithms as you want!
     */
    typedef enum
    {
	XMI_DEFAULT_BARRIER_PROTOCOL,
        XMI_CAU_BARRIER_PROTOCOL,             /**< Global Interrupt barrier. */
        XMI_GI_BARRIER_PROTOCOL,             /**< Global Interrupt barrier. */
        XMI_TREE_BARRIER_PROTOCOL,           /**< Tree barrier. */
        XMI_RECTANGLE_BARRIER_PROTOCOL,      /**< Rectangular shape barrier. */
        XMI_BINOMIAL_BARRIER_PROTOCOL,       /**< Rectangular shape barrier. */
        XMI_LOCKBOX_BARRIER_PROTOCOL,        /**< Lockbox Barrier protocol. */
        XMI_IPC_BARRIER_PROTOCOL,            /**< IPC barrier. */
        XMI_NUM_BARRIER_PROTOCOLS
    }XMI_Barrier_Protocol_t;

    typedef enum
    {
	XMI_DEFAULT_BROADCAST_PROTOCOL,
        XMI_TREE_BROADCAST_PROTOCOL,            /**< Tree broadcast.  */
        XMI_RECTANGLE_BROADCAST_PROTOCOL,       /**< rect broadcast. */
        XMI_RECTANGLE_BROADCAST_PROTOCOL_DPUT,  /**< rect broadcast with direct put broadcasts. */
        XMI_BINOMIAL_BROADCAST_PROTOCOL,        /**< binom broadcast. */
        XMI_IPC_BROADCAST_PROTOCOL,             /**< IPC broadcast.   */
        XMI_NUM_BROADCAST_PROTOCOLS
    }XMI_Broadcast_Protocol_t;

    typedef enum
    {
	XMI_DEFAULT_ALLGATHER_PROTOCOL,
        XMI_TREE_ALLGATHER_PROTOCOL,            /**< Tree allgather.  */
        XMI_RECTANGLE_ALLGATHER_PROTOCOL,       /**< rect allgather. */
        XMI_RECTANGLE_ALLGATHER_PROTOCOL_DPUT,  /**< rect allgather with direct put allgathers. */
        XMI_BINOMIAL_ALLGATHER_PROTOCOL,        /**< binom allgather. */
        XMI_IPC_ALLGATHER_PROTOCOL,             /**< IPC allgather.   */
        XMI_NUM_ALLGATHER_PROTOCOLS
    }XMI_Allgather_Protocol_t;

    typedef enum
    {
	XMI_DEFAULT_ALLGATHERV_PROTOCOL,
        XMI_TREE_ALLGATHERV_PROTOCOL,            /**< Tree allgatherv.  */
        XMI_RECTANGLE_ALLGATHERV_PROTOCOL,       /**< rect allgatherv. */
        XMI_RECTANGLE_ALLGATHERV_PROTOCOL_DPUT,  /**< rect allgatherv with direct put allgathervs. */
        XMI_BINOMIAL_ALLGATHERV_PROTOCOL,        /**< binom allgatherv. */
        XMI_IPC_ALLGATHERV_PROTOCOL,             /**< IPC allgatherv.   */
        XMI_NUM_ALLGATHERV_PROTOCOLS
    }XMI_Allgatherv_Protocol_t;


    typedef enum
    {
	XMI_DEFAULT_SCATTER_PROTOCOL,
        XMI_TREE_SCATTER_PROTOCOL,            /**< Tree scatter.  */
        XMI_RECTANGLE_SCATTER_PROTOCOL,       /**< rect scatter. */
        XMI_RECTANGLE_SCATTER_PROTOCOL_DPUT,  /**< rect scatter with direct put scatters. */
        XMI_BINOMIAL_SCATTER_PROTOCOL,        /**< binom scatter. */
        XMI_IPC_SCATTER_PROTOCOL,             /**< IPC scatter.   */
        XMI_NUM_SCATTER_PROTOCOLS
    }XMI_Scatter_Protocol_t;

    typedef enum
    {
	XMI_DEFAULT_SCATTERV_PROTOCOL,
        XMI_TREE_SCATTERV_PROTOCOL,            /**< Tree scatterv.  */
        XMI_RECTANGLE_SCATTERV_PROTOCOL,       /**< rect scatterv. */
        XMI_RECTANGLE_SCATTERV_PROTOCOL_DPUT,  /**< rect scatterv with direct put scattervs. */
        XMI_BINOMIAL_SCATTERV_PROTOCOL,        /**< binom scatterv. */
        XMI_IPC_SCATTERV_PROTOCOL,             /**< IPC scatterv.   */
        XMI_NUM_SCATTERV_PROTOCOLS
    }XMI_Scatterv_Protocol_t;

    typedef enum
    {
	XMI_DEFAULT_ALLREDUCE_PROTOCOL,
        XMI_TREE_ALLREDUCE_PROTOCOL,                 /**< Tree allreduce. */
        XMI_BINOMIAL_ALLREDUCE_PROTOCOL,             /**< binomial allreduce. */
        XMI_RECTANGLE_ALLREDUCE_PROTOCOL,            /**< rectangle/binomial allreduce. */
        XMI_RECTANGLE_RING_ALLREDUCE_PROTOCOL,       /**< rectangle/ring allreduce. */
        XMI_TREE_PIPELINED_ALLREDUCE_PROTOCOL,       /**< Tree allreduce. */
        XMI_SHORT_BINOMIAL_ALLREDUCE_PROTOCOL,       /**< binomial short allreduce. */
        XMI_TREE_DPUT_PIPELINED_ALLREDUCE_PROTOCOL,  /**< Tree allreduce. */
        XMI_ASYNC_BINOMIAL_ALLREDUCE_PROTOCOL,       /**< binomial allreduce. */
        XMI_ASYNC_RECTANGLE_ALLREDUCE_PROTOCOL,      /**< rectangle/binomial async allreduce. */
        XMI_ASYNC_RECTANGLE_RING_ALLREDUCE_PROTOCOL, /**< rectangle/ring async allreduce. */
        XMI_ASYNC_SHORT_BINOMIAL_ALLREDUCE_PROTOCOL, /**< binomial short async allreduce. */
        XMI_ASYNC_SHORT_RECTANGLE_ALLREDUCE_PROTOCOL,/**< rectangle short async allreduce. */
        XMI_RRING_DPUT_ALLREDUCE_PROTOCOL_SINGLETH,  /**< rectangle allreduce with direct puts (needs 16B alignmened buffers) */
        XMI_IPC_ALLREDUCE_PROTOCOL,                  /**< IPC allreduce. */
        XMI_NUM_ALLREDUCE_PROTOCOLS
    }XMI_Allreduce_Protocol_t;

    typedef enum
    {
        XMI_TREE_REDUCE_PROTOCOL,           /**< Tree reduce. */
        XMI_BINOMIAL_REDUCE_PROTOCOL,       /**< binomial reduce. */
        XMI_RECTANGLE_REDUCE_PROTOCOL,      /**< rectangle/binomial reduce. */
        XMI_RECTANGLE_RING_REDUCE_PROTOCOL, /**< rectangle/ring reduce. */
        XMI_SHORT_BINOMIAL_REDUCE_PROTOCOL, /**< binomial reduce. */
        XMI_NUM_REDUCE_PROTOCOLS
    }XMI_Reduce_Protocol_t;

    typedef enum
    {
	XMI_DEFAULT_ALLTOALL_PROTOCOL,
        XMI_ALLTOALL_PROTOCOL,       /**< Alltoall. */
        XMI_IPC_ALLTOALL_PROTOCOL,   /**< IPC Alltoall. */
        XMI_NUM_ALLTOALL_PROTOCOLS
    }XMI_Alltoall_Protocol_t;

    typedef enum
    {
	XMI_DEFAULT_ALLTOALLV_PROTOCOL,
        XMI_ALLTOALLV_PROTOCOL,       /**< Alltoallv. */
        XMI_IPC_ALLTOALLV_PROTOCOL,   /**< IPC Alltoallv. */
        XMI_NUM_ALLTOALLV_PROTOCOLS
    }XMI_Alltoallv_Protocol_t;

    typedef enum
    {
	XMI_DEFAULT_AMBROADCAST_PROTOCOL
    }XMI_AMBroadcast_Protocol_t;

    typedef enum
    {
	XMI_DEFAULT_AMSCATTER_PROTOCOL
    }XMI_AMScatter_Protocol_t;

    typedef enum
    {
	XMI_DEFAULT_AMGATHER_PROTOCOL
    }XMI_AMGather_Protocol_t;

    typedef enum
    {
	XMI_DEFAULT_AMREDUCE_PROTOCOL
    }XMI_AMReduce_Protocol_t;


    typedef enum
	{
	    XMI_CFG_BROADCAST = 0,
	    XMI_CFG_ALLREDUCE,
	    XMI_CFG_REDUCE,
	    XMI_CFG_ALLGATHER,
	    XMI_CFG_ALLGATHERV,
	    XMI_CFG_SCATTER,
	    XMI_CFG_SCATTERV,
	    XMI_CFG_BARRIER,
	    XMI_CFG_ALLTOALL,
	    XMI_CFG_ALLTOALLV,
	    XMI_CFG_AMBROADCAST,
	    XMI_CFG_AMSCATTER,
	    XMI_CFG_AMGATHER,
	    XMI_CFG_AMREDUCE,
	    XMI_CFG_COUNT
	}hl_config_t;

    /**
     * \brief Opaque data type that holds geometry information.
     *
     * The external API does not reveal the internal structure of the object.
     *
     * \internal Implementations should verify that geometry objects
     *           need less space than \c sizeof(XMI_Geometry_t).
     *
     * \todo     Need to adjust size to optimal level (currently 2048 bytes).
     */
    typedef CMQuad  XMI_Geometry_t [512];
    typedef struct
    {
	size_t lo;
	size_t hi;
    }XMI_Geometry_range_t;

    extern XMI_Geometry_t  XMI_World_Geometry;
    extern unsigned       XMI_World_Geometry_id;

    /* *********************************************************************************** */
    /* **************    "Special Callback Types Objects  ******************************** */
    /* *********************************************************************************** */
    /**
     *  \brief A callback to map the geometry id to the geometry
     *  structure. The runtime has to be able to freely translate
     *  between geometry id's and geometry structures.
     */
    typedef XMI_Geometry_t * (*XMI_mapIdToGeometry) (int comm);
    typedef CMQuad          XMI_AMHeader_t[8];


    /**
     * \brief Asynchronous broadcast receive callback.
     *
     * \todo doxygen
     */
    typedef void * (*XMI_RecvAMBroadcast) (unsigned           root,
					  unsigned           comm,
					  const unsigned     sndlen,
					  unsigned         * rcvlen,
					  char            ** rcvbuf,
					  XMI_Callback_t    * const cb_info);

    typedef void * (*XMI_RecvAMScatter) (unsigned           root,
					unsigned           comm,
					const unsigned     sndlen,
					unsigned         * rcvlen,
					char            ** rcvbuf,
					XMI_Callback_t    * const cb_info);
    typedef void * (*XMI_RecvAMGather) (unsigned           root,
				       unsigned           comm,
				       const unsigned     sndlen,
				       unsigned         * rcvlen,
				       char            ** rcvbuf,
				       XMI_Callback_t    * const cb_info);
    typedef void * (*XMI_RecvAMReduce) (unsigned           root,
				       unsigned           comm,
				       const unsigned     sndlen,
				       unsigned         * rcvlen,
				       char            ** rcvbuf,
				       XMI_Callback_t    * const cb_info);

    /* ************************************************************************* */
    /* **************    Configuration Objects  ******************************** */
    /* ************************************************************************* */
    /**
     * \brief Configuration Objects for collectives
     *        Use these to set the algorithm types
     *        Register the protocol implementation
     *
     * \warning After registering the protocol information it is illegal to
     *          deallocate the registration object.
     *
     * \param[out] registration  Opaque memory to maintain registration information.
     * \param[in]  configuration broadcast configuration information.
     *
     * \retval     0            Success
     *
     * \see XMI_Broadcast
     */
    /*  Clear up mapId to geometry!!!! */

    typedef struct
    {
        hl_config_t             cfg_type;
        XMI_Broadcast_Protocol_t protocol;          /**< The broadcast protocol implementation to register. */
    }XMI_Broadcast_Configuration_t;

    typedef struct
    {
        hl_config_t             cfg_type;
        XMI_Allgather_Protocol_t protocol;          /**< The allgather protocol implementation to register. */
    }XMI_Allgather_Configuration_t;

    typedef struct
    {
        hl_config_t             cfg_type;
        XMI_Allgatherv_Protocol_t protocol;          /**< The allgather protocol implementation to register. */
    }XMI_Allgatherv_Configuration_t;

    typedef struct
    {
        hl_config_t             cfg_type;
        XMI_Scatter_Protocol_t protocol;          /**< The scatter protocol implementation to register. */
    }XMI_Scatter_Configuration_t;

    typedef struct
    {
        hl_config_t             cfg_type;
        XMI_Scatterv_Protocol_t protocol;          /**< The scatter protocol implementation to register. */
    }XMI_Scatterv_Configuration_t;

    typedef struct
    {
        hl_config_t           cfg_type;
        XMI_Allreduce_Protocol_t protocol;       /**< The allreduce protocol implementation to register. */
        unsigned              reuse_storage:1;/**< Reuse malloc'd storage across calls if set. Otherwise, free it. */
        unsigned              reserved:31;    /**< Currently unused */
    }XMI_Allreduce_Configuration_t;

    typedef struct
    {
        hl_config_t        cfg_type;
        XMI_Reduce_Protocol_t protocol;       /**< The reduce protocol implementation to register. */
        unsigned           reuse_storage:1;/**< Reuse malloc'd storage across calls if set. Otherwise, free it. */
        unsigned           reserved:31;    /**< Reserved for future use. */
    }XMI_Reduce_Configuration_t;

    typedef struct
    {
        hl_config_t           cfg_type;
        XMI_Barrier_Protocol_t protocol;    /**< The barrier protocol implementation to register. */
    }XMI_Barrier_Configuration_t;

    typedef struct
    {
        hl_config_t             cfg_type;
        XMI_Alltoall_Protocol_t  protocol;    /**< The alltoall protocol implementation to register. */
    }XMI_Alltoall_Configuration_t;

    typedef struct
    {
        hl_config_t             cfg_type;
        XMI_Alltoallv_Protocol_t protocol;    /**< The alltoallv protocol implementation to register. */
    }XMI_Alltoallv_Configuration_t;

    typedef struct
    {
        hl_config_t               cfg_type;
        XMI_RecvAMBroadcast        cb_recv;      /**< Callback to invoke to receive an asynchronous broadcast message. */
        XMI_AMBroadcast_Protocol_t protocol;     /**< The AMBroad protocol implementation to register. */
    }XMI_AMBroadcast_Configuration_t;

    typedef struct
    {
        hl_config_t             cfg_type;
        XMI_RecvAMScatter        cb_recv;      /**< Callback to invoke to receive an asynchronous broadcast message. */
        XMI_AMScatter_Protocol_t protocol;     /**< The AMScatter protocol implementation to register. */
    }XMI_AMScatter_Configuration_t;

    typedef struct
    {
        hl_config_t             cfg_type;
        XMI_RecvAMGather         cb_recv;      /**< Callback to invoke to receive an asynchronous broadcast message. */
        XMI_AMGather_Protocol_t  protocol;     /**< The AMGather protocol implementation to register. */
    }XMI_AMGather_Configuration_t;

    typedef struct
    {
        hl_config_t             cfg_type;
        XMI_RecvAMReduce         cb_recv;      /**< Callback to invoke to receive an asynchronous broadcast message. */
        XMI_AMReduce_Protocol_t  protocol;     /**< The AMReduce protocol implementation to register. */
    }XMI_AMReduce_Configuration_t;

    typedef union
    {
        hl_config_t                       cfg_type;
        XMI_Reduce_Configuration_t         cfg_reduce;
        XMI_Allreduce_Configuration_t      cfg_allreduce;
        XMI_Allgather_Configuration_t      cfg_allgather;
        XMI_Allgatherv_Configuration_t     cfg_allgatherv;
        XMI_Scatter_Configuration_t        cfg_scatter;
        XMI_Scatterv_Configuration_t       cfg_scatterv;
        XMI_Broadcast_Configuration_t      cfg_broadcast;
        XMI_Alltoall_Configuration_t       cfg_alltoall;
        XMI_Alltoallv_Configuration_t      cfg_alltoallv;
        XMI_AMBroadcast_Configuration_t    cfg_ambroadcast;
        XMI_AMScatter_Configuration_t      cfg_amscatter;
        XMI_AMGather_Configuration_t       cfg_amgather;
        XMI_AMReduce_Configuration_t       cfg_amreduce;
        XMI_Barrier_Configuration_t        cfg_barrier;
    }   XMI_CollectiveConfiguration_t;


    int XMI_Collectives_initialize(int *argc, char***argv, XMI_mapIdToGeometry cb_map);
    int XMI_Collectives_finalize();
    /* Rename  to CollectiveRegistration_T , doxygen with Storage variable storage, others*/
    int XMI_register(XMI_CollectiveProtocol_t      *registration,
                    XMI_CollectiveConfiguration_t *XMI_CollectiveConfiguration_t,
		    int                           key);

    /* ************************************************************************* */
    /* ********* Transfer Types, used by geometry and xfer routines ************ */
    /* ************************************************************************* */
    typedef enum
	{
            XMI_XFER_BROADCAST = 0,
            XMI_XFER_ALLREDUCE,
            XMI_XFER_REDUCE,
            XMI_XFER_ALLGATHER,
            XMI_XFER_ALLGATHERV,
            XMI_XFER_SCATTER,
            XMI_XFER_SCATTERV,
            XMI_XFER_BARRIER,
            XMI_XFER_ALLTOALL,
            XMI_XFER_ALLTOALLV,
            XMI_XFER_AMBROADCAST,
            XMI_XFER_AMSCATTER,
            XMI_XFER_AMGATHER,
            XMI_XFER_AMREDUCE,
            XMI_XFER_COUNT
        }hl_xfer_type_t;

    /* ************************************************************************* */
    /* **************     Geometry (like groups/communicators)  **************** */
    /* ************************************************************************* */
    /**
     * \brief Initialize the geometry
     *
     * \param[out] geometry        Opaque geometry object to initialize
     * \param[in]  id              Unique identifier for this geometry
     *                             which globally represents this geometry
     * \param[in]  rank_slices     Array of node slices participating in the geometry
     *                             User must keep the array of slices in memory for the
     *                             duration of the geometry's existence
     * \param[in]  slice_count     Number of nodes participating in the geometry
     */
    int XMI_Geometry_initialize (XMI_Geometry_t            * geometry,
                                unsigned                   id,
                                XMI_Geometry_range_t      * rank_slices,
                                unsigned                   slice_count);

    /**
     * \brief Analyze a protocol to ensure that it will run on the specified geometry
     *
     * \param[in]     geometry   An input geometry to be analyzed.
     * \param[in/out] protocol   A list of protocol pointers to be checked for this geometry
     *                           pointers will be set to NULL for invalid protocols on
     *                           this geometry.
     * \param[in/out] num        number of protocols in the list in/requested, out/actual
     * \retval        XMI_SUCCESS The protocol will run on the current geometry
     * \retval        ?????      The protocol does not support the current geometry
     */
    int XMI_Geometry_algorithm (XMI_Geometry_t            *geometry,
			       XMI_CollectiveProtocol_t **protocols,
			       int                      *num);

    /**
     * \brief Free any memory allocated inside of a geometry. Mostly
     * the alltoall permutation array right now.
     * \param[in] geometry The geometry object to free
     * \retval XMI_SUCCESS Memory free didn't fail
     */
    int XMI_Geometry_finalize(XMI_Geometry_t *geometry);

    /* *********************************************************************************** */
    /* **************    "Transfer Routine and Objects    ******************************** */
    /* *********************************************************************************** */
    /**
     * \brief Create and post a non-blocking alltoall vector operation.
     *
     * The alltoallv operation ...
     *
     * \warning Until the message callback is invoked, it is illegal to send it
     *          again, reset it, touch the attached buffers, or deallocate the
     *          request object.
     *
     * \param[in]  registration Protocol registration.
     * \param[in]  request      Opaque memory to maintain internal message state.
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     *                          \c NULL indicates the global geometry.
     * \param[in]  sndbuf      The base address of the buffers containing data to be sent
     * \param[in]  sndlens     How much data to send to each node
     * \param[in]  sdispls     Where to find the data in sndbuf
     * \param[out] rcvbuf      The base address of the buffer for data reception
     * \param[in]  rcvlens     How much data to receive from each node
     * \param[in]  rdispls     Where to put the data in rcvbuf
     * \param[in]  sndcounters ???
     * \param[in]  rcvcounters ???
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        hl_xfer_type_t             xfer_type;
        XMI_CollectiveProtocol_t  * registration;
        XMI_CollectiveRequest_t   * request;
        XMI_Callback_t              cb_done;
        XMI_Geometry_t            * geometry;
        char                     * sndbuf;
        unsigned                 * sndlens;
        unsigned                 * sdispls;
        char                     * rcvbuf;
        unsigned                 * rcvlens;
        unsigned                 * rdispls;
        unsigned                 * sndcounters;
        unsigned                 * rcvcounters;
    }hl_alltoallv_t;

    /**
     * \brief Create and post a non-blocking alltoall operation.
     * \todo:  FIX THIS TO NOT BE THE SAME AS ALLTOALL
     * The alltoall operation ...
     *
     * \warning Until the message callback is invoked, it is illegal to send it
     *          again, reset it, touch the attached buffers, or deallocate the
     *          request object.
     *
     * \param[in]  registration Protocol registration.
     * \param[in]  request      Opaque memory to maintain internal message state.
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     *                          \c NULL indicates the global geometry.
     * \param[in]  sndbuf      The base address of the buffers containing data to be sent
     * \param[in]  sndlens     How much data to send to each node
     * \param[in]  sdispls     Where to find the data in sndbuf
     * \param[out] rcvbuf      The base address of the buffer for data reception
     * \param[in]  rcvlens     How much data to receive from each node
     * \param[in]  rdispls     Where to put the data in rcvbuf
     * \param[in]  sndcounters ???
     * \param[in]  rcvcounters ???
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        hl_xfer_type_t             xfer_type;
        XMI_CollectiveProtocol_t  * registration;
        XMI_CollectiveRequest_t   * request;
        XMI_Callback_t              cb_done;
        XMI_Geometry_t            * geometry;
        char                     * sndbuf;
        unsigned                 * sndlens;
        unsigned                 * sdispls;
        char                     * rcvbuf;
        unsigned                 * rcvlens;
        unsigned                 * rdispls;
        unsigned                 * sndcounters;
        unsigned                 * rcvcounters;
    }hl_alltoall_t;

    /**
     * \brief Create and post a non-blocking reduce operation.
     *
     * The reduce operation ...
     *
     * \warning Until the message callback is invoked, it is illegal to send it
     *          again, reset it, touch the attached buffers, or deallocate the
     *          request object.
     *
     * \param[in]  registration Protocol registration.
     * \param[in]  request      Opaque memory to maintain internal message state.
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     *                          \c NULL indicates the global geometry.
     * \param[in]  root         Rank of the reduce root node.
     * \param[in]  sbuffer      Source buffer.
     * \param[in]  rbuffer      Receive buffer.
     * \param[in]  count        Number of elements to reduce.
     * \param[in]  dt           Element data type
     * \param[in]  op           Reduce operation
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        hl_xfer_type_t            xfer_type;
        XMI_CollectiveProtocol_t * registration;
        XMI_CollectiveRequest_t  * request;
        XMI_Callback_t             cb_done;
        XMI_Geometry_t           * geometry;
        int                       root;
        char                    * sbuffer;
        char                    * rbuffer;
        unsigned                  count;
        XMI_Dt                     dt;
        XMI_Op                     op;
    }hl_reduce_t;

    /**
     * \brief Create and post a non-blocking broadcast operation.
     *
     * The broadcast operation ...
     *
     * \warning Until the message callback is invoked, it is illegal to send it
     *          again, reset it, touch the attached buffers, or deallocate the
     *          request object.
     *
     * \param[in]  registration Protocol registration.
     * \param[in]  request      Opaque memory to maintain internal message state.
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     *                          \c NULL indicates the global geometry.
     * \param[in]  root         Rank of the node performing the broadcast.
     * \param[in]  src          Source buffer to broadcast.
     * \param[in]  bytes        Number of bytes to broadcast.
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        hl_xfer_type_t             xfer_type;
        XMI_CollectiveProtocol_t  * registration;
        XMI_CollectiveRequest_t   * request;
        XMI_Callback_t              cb_done;
        XMI_Geometry_t            * geometry;
        unsigned                   root;
        char                     * src;
        char                     * dst;
        unsigned                   bytes;
    }hl_broadcast_t;

    /**
     * \brief Create and post a non-blocking allgather
     *
     * The allgather
     *
     * \warning Until the message callback is invoked, it is illegal to send it
     *          again, reset it, touch the attached buffers, or deallocate the
     *          request object.
     *
     * \param[in]  registration Protocol registration.
     * \param[in]  request      Opaque memory to maintain internal message state.
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     *                          \c NULL indicates the global geometry.
     * \param[in]  src          Source buffer to allgather.
     * \param[in]  bytes        Number of bytes to allgather.
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        hl_xfer_type_t             xfer_type;
        XMI_CollectiveProtocol_t  * registration;
        XMI_CollectiveRequest_t   * request;
        XMI_Callback_t              cb_done;
        XMI_Geometry_t            * geometry;
        char                     * src;
        char                     * dst;
        size_t                     bytes;
    }hl_allgather_t;

    /**
     * \brief Create and post a non-blocking allgatherv
     *
     * The allgatherv
     *
     * \warning Until the message callback is invoked, it is illegal to send it
     *          again, reset it, touch the attached buffers, or deallocate the
     *          request object.
     *
     * \param[in]  registration Protocol registration.
     * \param[in]  request      Opaque memory to maintain internal message state.
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     *                          \c NULL indicates the global geometry.
     * \param[in]  src          Source buffer to allgatherv.
     * \param[in]  bytes        Number of bytes to allgatherv.
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        hl_xfer_type_t             xfer_type;
        XMI_CollectiveProtocol_t  * registration;
        XMI_CollectiveRequest_t   * request;
        XMI_Callback_t              cb_done;
        XMI_Geometry_t            * geometry;
        char                     * src;
        char                     * dst;
        size_t                   * lengths;
    }hl_allgatherv_t;

    /**
     * \brief Create and post a non-blocking scatter
     *
     * The scatter
     *
     * \warning Until the message callback is invoked, it is illegal to send it
     *          again, reset it, touch the attached buffers, or deallocate the
     *          request object.
     *
     * \param[in]  registration Protocol registration.
     * \param[in]  request      Opaque memory to maintain internal message state.
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     *                          \c NULL indicates the global geometry.
     * \param[in]  src          Source buffer to scatter.
     * \param[in]  bytes        Number of bytes to scatter.
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        hl_xfer_type_t             xfer_type;
        XMI_CollectiveProtocol_t  * registration;
        XMI_CollectiveRequest_t   * request;
        XMI_Callback_t              cb_done;
        XMI_Geometry_t            * geometry;
        unsigned                   root;
        char                     * src;
        char                     * dst;
        size_t                     bytes;
    }hl_scatter_t;

    /**
     * \brief Create and post a non-blocking scatterv
     *
     * The scatterv
     *
     * \warning Until the message callback is invoked, it is illegal to send it
     *          again, reset it, touch the attached buffers, or deallocate the
     *          request object.
     *
     * \param[in]  registration Protocol registration.
     * \param[in]  request      Opaque memory to maintain internal message state.
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     *                          \c NULL indicates the global geometry.
     * \param[in]  src          Source buffer to scatterv.
     * \param[in]  bytes        Number of bytes to scatterv.
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        hl_xfer_type_t             xfer_type;
        XMI_CollectiveProtocol_t  * registration;
        XMI_CollectiveRequest_t   * request;
        XMI_Callback_t              cb_done;
        XMI_Geometry_t            * geometry;
        unsigned                   root;
        char                     * src;
        char                     * dst;
        size_t                   * lengths;
    }hl_scatterv_t;

    /**
     * \brief Create and post a non-blocking allreduce operation.
     *
     * The allreduce operation ...
     *
     * \warning Until the message callback is invoked, it is illegal to send it
     *          again, reset it, touch the attached buffers, or deallocate the
     *          request object.
     *
     * \param[in]  registration Protocol registration.
     * \param[in]  request      Opaque memory to maintain internal message state.
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     *                          \c NULL indicates the global geometry.
     * \param[in]  sbuffer      Source buffer.
     * \param[in]  rbuffer      Receive buffer.
     * \param[in]  count        Number of elements to allreduce.
     * \param[in]  dt           Element data type
     * \param[in]  op           Reduce operation
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        hl_xfer_type_t            xfer_type;
        XMI_CollectiveProtocol_t * registration;
        XMI_CollectiveRequest_t  * request;
        XMI_Callback_t             cb_done;
        XMI_Geometry_t           * geometry;
        char                    * src;
        char                    * dst;
        unsigned                  count;
        XMI_Dt                     dt;
        XMI_Op                     op;
    }hl_allreduce_t;


    /**
     * \brief Create and post a non-blocking barrier operation.
     * The barrier operation ...
     * \param   geometry     Geometry to use for this collective operation.
     *                       \c NULL indicates the global geometry.
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \retval  0            Success
     *
     * \see XMI_Barrier_register
     *
     * \todo doxygen
     */
    typedef struct
    {
        hl_xfer_type_t            xfer_type;
        XMI_CollectiveProtocol_t * registration;
        XMI_CollectiveRequest_t  * request;
        XMI_Callback_t             cb_done;
        XMI_Geometry_t           * geometry;
    }hl_barrier_t;

    /**
     * \brief Create and post a non-blocking active message broadcast operation.
     * The Active Message broadcast operation ...
     *
     * This differs from AMSend in only one particular: it takes geometry/team 
     * as an argument. The semantics are as follows: the included header and data 
     * are broadcast to every place in the team. The completion handler is invoked 
     * on the sender side as soon as send buffers can be reused. On the receive 
     * side the usual two-phase reception protocol is executed: a header handler
     * determines the address to which to deposit the data and sets the address 
     * of a receive completion hander to be invoked once the data has arrived.
     *
     * \warning Until the message callback is invoked, it is illegal to send it
     *          again, reset it, touch the attached buffers, or deallocate the
     *          request object.
     *
     * \param[in]  registration Protocol registration.
     * \param[in]  request      Opaque memory to maintain internal message state.
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     *                          \c NULL indicates the global geometry.
     * \param[in]  header       metadata to send to destination in the header
     * \param[in]  src          Source buffer to broadcast.
     * \param[in]  bytes        Number of bytes to broadcast.
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        hl_xfer_type_t             xfer_type;
        XMI_CollectiveProtocol_t  * registration;
        XMI_CollectiveRequest_t   * request;
        XMI_Callback_t              cb_done;
        XMI_Geometry_t            * geometry;
	XMI_AMHeader_t            * header;
        char                     * src;
        unsigned                   bytes;
    }hl_ambroadcast_t;

    /**
     * \brief Create and post a non-blocking active message scatter operation.
     * The Active Message scatter operation ...
     *
     * This is slightly more complicated than an AMBroadcast, because it allows
     * different headers and data buffers to be sent to everyone in the team.
     *
     * \warning Until the message callback is invoked, it is illegal to send it
     *          again, reset it, touch the attached buffers, or deallocate the
     *          request object.
     *
     * \param[in]  registration Protocol registration.
     * \param[in]  request      Opaque memory to maintain internal message state.
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     *                          \c NULL indicates the global geometry.
     * \param[in]  headers      array of  metadata to send to destination
     * \param[in]  src          Source buffers to scatter.
     * \param[in]  srclengths   Number of bytes to scatter per destination.
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        hl_xfer_type_t             xfer_type;
        XMI_CollectiveProtocol_t  * registration;
        XMI_CollectiveRequest_t   * request;
        XMI_Callback_t              cb_done;
        XMI_Geometry_t            * geometry;
	XMI_AMHeader_t            * headers;
        char                     * src;
	size_t                     srclengths;
    }hl_amscatter_t;

    /**
     * \brief Create and post a non-blocking active message gather operation.
     * The Active Message gather operation ...
     *
     * This is the reverse of amscatter. It works as follows. The header only, 
     * no data, is broadcast to the team. Each place in the team executes the 
     * header handler and points to a data buffer in local space. A reverse transfer 
     * then takes place (the buffer is sent from the receiver back to the sender, 
     * and deposited in one of the buffers provided as part of the original call 
     * (the "data" parameter). 
     *
     * \warning Until the message callback is invoked, it is illegal to send it
     *          again, reset it, touch the attached buffers, or deallocate the
     *          request object.
     *
     * \param[in]  registration Protocol registration.
     * \param[in]  request      Opaque memory to maintain internal message state.
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     *                          \c NULL indicates the global geometry.
     * \param[in]  headers      array of metadata to send to destination
     * \param[in]  src          Source buffers to gather.
     * \param[in]  srclengths   Number of bytes to scatter per destination.
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        hl_xfer_type_t             xfer_type;
        XMI_CollectiveProtocol_t  * registration;
        XMI_CollectiveRequest_t   * request;
        XMI_Callback_t              cb_done;
        XMI_Geometry_t            * geometry;
	XMI_AMHeader_t            * headers;
        char                     * src;
	size_t                     srclengths;
    }hl_amgather_t;

    /**
     * \brief Create and post a non-blocking active message reduce operation.
     * The Active Message reduce operation ...
     *
     * This is fairly straightforward given how amgather works. Instead of 
     * collecting the data without processing, all buffers are reduced using the 
     * operation and data type provided by the sender. The final reduced data is 
     * deposited in the original buffer provided by the initiator. On the receive 
     * side the algorithm has the right to change the buffers provided by the header
     * handler (this may avoid having the implementor allocate more memory for 
     * internal buffering)
     *
     * \warning Until the message callback is invoked, it is illegal to send it
     *          again, reset it, touch the attached buffers, or deallocate the
     *          request object.
     *
     * \param[in]  registration Protocol registration.
     * \param[in]  request      Opaque memory to maintain internal message state.
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     *                          \c NULL indicates the global geometry.
     * \param[in]  headers      metadata to send to destinations in the header
     * \param[in]  src          Source buffers to reduce.
     * \param[in]  srclengths   Number of bytes to scatter per destination.
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        hl_xfer_type_t             xfer_type;
        XMI_CollectiveProtocol_t  * registration;
        XMI_CollectiveRequest_t   * request;
        XMI_Callback_t              cb_done;
        XMI_Geometry_t            * geometry;
	XMI_AMHeader_t            * headers;
        char                     * src;
	unsigned                   count;
	XMI_Dt                      dt;
        XMI_Op                      op;
    }hl_amreduce_t;


    typedef union
    {
	hl_xfer_type_t        xfer_type;
        hl_allreduce_t        xfer_allreduce;
        hl_broadcast_t        xfer_broadcast;
        hl_reduce_t           xfer_reduce;
        hl_allgather_t        xfer_allgather;
        hl_allgatherv_t       xfer_allgatherv;
        hl_scatter_t          xfer_scatter;
        hl_scatterv_t         xfer_scatterv;
        hl_alltoall_t         xfer_alltoall;
        hl_alltoallv_t        xfer_alltoallv;
	hl_ambroadcast_t      xfer_ambroadcast;
	hl_amscatter_t        xfer_amscatter;
	hl_amgather_t         xfer_amgather;
	hl_amreduce_t         xfer_amreduce;
	hl_barrier_t          xfer_barrier;
    }hl_xfer_t;

    int XMI_Xfer (void *context, hl_xfer_t *cmd);
    int XMI_Poll();
    int XMI_Rank();
    int XMI_Size();

#ifdef __cplusplus
};
#endif

#endif
