/**
 * \file xmi_collectives.h
 * \brief Common external collective layer interface.
 */

#ifndef __xmi_collective_h__
#define __xmi_collective_h__

#include "xmi_types.h"

#ifdef __cplusplus
extern "C"
{
#endif
    typedef size_t  xmi_geometry_t;
    typedef size_t  xmi_algorithm_t;

    typedef struct
    {
        size_t lo;
        size_t hi;
    }xmi_geometry_range_t;

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
            XMI_XFER_ALLGATHERV_INT,
            XMI_XFER_SCATTER,
            XMI_XFER_SCATTERV,
            XMI_XFER_SCATTERV_INT,
            XMI_XFER_BARRIER,
            XMI_XFER_ALLTOALL,
            XMI_XFER_ALLTOALLV,
            XMI_XFER_ALLTOALLV_INT,
            XMI_XFER_SCAN,
            XMI_XFER_AMBROADCAST,
            XMI_XFER_AMSCATTER,
            XMI_XFER_AMGATHER,
            XMI_XFER_AMREDUCE,
            XMI_XFER_COUNT
        }xmi_xfer_type_t;

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
    xmi_result_t XMI_Geometry_initialize (xmi_context_t               context,
                                          xmi_geometry_t            * geometry,
                                          unsigned                    id,
                                          xmi_geometry_range_t      * rank_slices,
                                          unsigned                    slice_count);

    /**
     * \brief Initialize the geometry
     *
     * \param[in]  context         xmi context
     * \param[in]  world_geometry  world geometry object
     */
    xmi_result_t XMI_Geometry_world (xmi_context_t               context,
                                     xmi_geometry_t            * world_geometry);


    /**
     * \brief Analyze a protocol to ensure that it will run on the specified geometry
     *
     * \param[in]     geometry   An input geometry to be analyzed.
     * \param[in/out] algorithm  Storage for a list of algorithm id's
     * \param[in/out] num        number of algorithms in the list in/requested, out/actual
     * \retval        XMI_SUCCESS The protocol will run on the current geometry
     * \retval        ?????      The protocol does not support the current geometry
     */
    xmi_result_t XMI_Geometry_algorithm (xmi_context_t              context,
                                         xmi_geometry_t             geometry,
                                         xmi_algorithm_t           *algorithm,
                                         int                       *num);

    /**
     * \brief Free any memory allocated inside of a geometry.
     * \param[in] geometry The geometry object to free
     * \retval XMI_SUCCESS Memory free didn't fail
     */
    xmi_result_t XMI_Geometry_finalize(xmi_context_t   context,
                                       xmi_geometry_t  geometry);

    /**
     * \brief Create and post a non-blocking alltoall vector operation.
     *
     * The alltoallv operation ...
     *
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     * \param[in]  sndbuf       The base address of the buffers containing data to be sent
     * \param[in]  stype        A single type datatype
     * \param[in]  stypecounts  An array of type replication counts.  Size of geometry length
     * \param[in]  sdispls      Array of offsets into the sndbuf.  Size of geometry length
     * \param[out] rcvbuf       The base address of the buffer for data reception
     * \param[in]  rtype        A single type datatype
     * \param[in]  rtypecounts  Array of type replication counts.  Size of geometry length
     * \param[in]  rdispls      Array of offsets into the rcvbuf.  Size of geometry length
     *
     * \retval     0            Success
     *
     */
    typedef struct
    {
        xmi_xfer_type_t            xfer_type;
        xmi_event_function         cb_done;
        void                     * cookie;
        xmi_geometry_t             geometry;
        xmi_algorithm_t            algorithm;
        char                     * sndbuf;
        xmi_type_t               * stype;
        size_t                   * stypecounts;
        size_t                   * sdispls;
        char                     * rcvbuf;
        xmi_type_t               * rtype;
        size_t                   * rtypecounts;
        size_t                   * rdispls;
    }xmi_alltoallv_t;

    /**
     * \brief Create and post a non-blocking alltoall vector operation.
     *
     * The alltoallv_int operation ...
     *
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     * \param[in]  sndbuf       The base address of the buffers containing data to be sent
     * \param[in]  stype        A single type datatype
     * \param[in]  stypecounts  An array of type replication counts.  Size of geometry length
     * \param[in]  sdispls      Array of offsets into the sndbuf.  Size of geometry length
     * \param[out] rcvbuf       The base address of the buffer for data reception
     * \param[in]  rtype        A single type datatype
     * \param[in]  rtypecounts  Array of type replication counts.  Size of geometry length
     * \param[in]  rdispls      Array of offsets into the rcvbuf.  Size of geometry length
     *
     * \retval     0            Success
     *
     */
    typedef struct
    {
        xmi_xfer_type_t           xfer_type;
        xmi_event_function        cb_done;
        void                    * cookie;
        xmi_geometry_t            geometry;
        xmi_algorithm_t           algorithm;
        char                    * sndbuf;
        xmi_type_t              * stype;
        int                     * stypecounts;
        int                     * sdispls;
        char                    * rcvbuf;
        xmi_type_t              * rtype;
        int                     * rtypecounts;
        int                     * rdispls;
    }xmi_alltoallv_int_t;


    /**
     * \brief Create and post a non-blocking alltoall operation.
     * The alltoall operation ...
     *
     * \param[in]  cb_done     Callback to invoke when message is complete.
     * \param[in]  geometry    Geometry to use for this collective operation.
     * \param[in]  sndbuf      The base address of the buffers containing data to be sent
     * \param[in]  stype       Single datatype of the send buffer
     * \param[in]  stypecount  Single type replication count
     * \param[out] rbuf        The base address of the buffer for data reception
     * \param[in]  rtype       Single datatype of the receive buffer
     * \param[in]  rtypecount  Single type replication count
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        xmi_xfer_type_t             xfer_type;
        xmi_event_function          cb_done;
        void                      * cookie;
        xmi_geometry_t              geometry;
        xmi_algorithm_t             algorithm;
        char                      * sndbuf;
        xmi_type_t                * stype;
        size_t                      stypecount;
        char                      * rcvbuf;
        xmi_type_t                * rtype;
        size_t                      rtypecount;
    }xmi_alltoall_t;

    /**
     * \brief Create and post a non-blocking reduce operation.
     * The reduce operation ...
     *
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     * \param[in]  root         Rank of the reduce root node.
     * \param[in]  sbuffer      Source buffer.
     * \param[in]  stype        Source buffer type
     * \param[in]  stypecount   Source buffer type count
     * \param[in]  rbuffer      Receive buffer.
     * \param[in]  rtype        Receive buffer layout
     * \param[in]  rtypecount   Receive buffer type count
     * \param[in]  dt           Element data type
     * \param[in]  op           Reduce operation
     *
     * \retval     0            Success
     */
    typedef struct
    {
        xmi_xfer_type_t            xfer_type;
        xmi_event_function         cb_done;
        void                     * cookie;
        xmi_geometry_t             geometry;
        xmi_algorithm_t            algorithm;
        size_t                     root;
        char                     * sbuffer;
        xmi_type_t               * stype;
        size_t                     stypecount;
        char                     * rbuffer;
        xmi_type_t               * rtype;
        size_t                     rtypecount;
        xmi_dt                     dt;
        xmi_op                     op;
    }xmi_reduce_t;

    /**
     * \brief Create and post a non-blocking reduce_scatter operation.
     *
     * The reduce_scatter operation ...
     *
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     * \param[in]  sbuffer      Source buffer.
     * \param[in]  stype        Source buffer type
     * \param[in]  stypecount   Source buffer type count
     * \param[in]  rbuffer      Receive buffer.
     * \param[in]  rtype        Receive buffer layout
     * \param[in]  rtypecount   Receive buffer type count
     * \param[in]  rcounts      number of elements to receive from the destinations(common on all nodes)
     * \param[in]  dt           Element data type
     * \param[in]  op           Reduce operation
     *
     * \retval     0            Success
     *
     */
    typedef struct
    {
        xmi_xfer_type_t           xfer_type;
        xmi_event_function        cb_done;
        void                    * cookie;
        xmi_geometry_t            geometry;
        xmi_algorithm_t           algorithm;
        char                    * sbuffer;
        xmi_type_t              * stype;
        size_t                    stypecount;
        char                    * rbuffer;
        xmi_type_t              * rtype;
        size_t                    rtypecount;
        size_t                  * rcounts;
        xmi_dt                    dt;
        xmi_op                    op;
    }xmi_reduce_scatter_t;

    /**
     * \brief Create and post a non-blocking broadcast operation.
     *
     * The broadcast operation ...
     *
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     * \param[in]  root         Rank of the node performing the broadcast.
     * \param[in]  buf          Source buffer to broadcast on root, dest buffer on non-root
     * \param[in]  type         data type layout, may be different on root/destinations
     * \param[in]  count        Single type replication count
     *
     * \retval     0            Success
     *
     */
    typedef struct
    {
        xmi_xfer_type_t             xfer_type;
        xmi_event_function          cb_done;
        void                      * cookie;
        xmi_geometry_t              geometry;
        xmi_algorithm_t             algorithm;
        size_t                      root;
        char                      * buf;
        xmi_type_t                * type;
        size_t                      typecount;
    }xmi_broadcast_t;


    /**
     * \brief Create and post a non-blocking allgather
     *
     * The allgather
     *
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     * \param[in]  src          Source buffer to send
     * \param[in]  stype        data layout of send buffer
     * \param[in]  stypecount   replication count of the type
     * \param[in]  rcv          Source buffer to receive the data
     * \param[in]  rtype        data layout of each receive buffer
     * \param[in]  rtypecount   replication count of the type
     *
     * \retval     0            Success
     *
     */
    typedef struct
    {
        xmi_xfer_type_t             xfer_type;
        xmi_event_function          cb_done;
        void                      * cookie;
        xmi_geometry_t              geometry;
        xmi_algorithm_t             algorithm;
        char                      * src;
        xmi_type_t                * stype;
        size_t                      stypecount;
        char                      * rcv;
        xmi_type_t                * rtype;
        size_t                      rtypecount;
    }xmi_allgather_t;


    /**
     * \brief Create and post a non-blocking allgather
     *
     * The gather
     *
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     * \param[in]  root         The root node of the gather operation
     * \param[in]  src          Source buffer to send
     * \param[in]  stype        data layout of send buffer
     * \param[in]  stypecount   replication count of the type
     * \param[in]  rcv          Source buffer to receive the data
     * \param[in]  rtype        data layout of each receive buffer
     * \param[in]  rtypecount   replication count of the type
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        xmi_xfer_type_t             xfer_type;
        xmi_event_function          cb_done;
        void                      * cookie;
        xmi_geometry_t              geometry;
        xmi_algorithm_t             algorithm;
        size_t                      root;
        char                      * src;
        xmi_type_t                * stype;
        size_t                      stypecount;
        char                      * rcv;
        xmi_type_t                * rtype;
        size_t                      rtypecount;
    }xmi_gather_t;

    /**
     * \brief Create and post a non-blocking gatherv
     *
     * The gatherv
     *
     * \param[in]  request      Opaque memory to maintain internal message state.
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     * \param[in]  root         The root node for the gatherv operation
     * \param[in]  sndbuf       The base address of the buffers containing data to be sent
     * \param[in]  stype        A single type datatype
     * \param[in]  stypecount   type replication count.
     * \param[out] rcvbuf       The base address of the buffer for data reception
     * \param[in]  rtype        A single type datatype
     * \param[in]  rtypecounts  Array of type replication counts.  Size of geometry length
     * \param[in]  rdispls      Array of offsets into the rcvbuf.  Size of geometry length
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        xmi_xfer_type_t            xfer_type;
        xmi_event_function         cb_done;
        void                     * cookie;
        xmi_geometry_t             geometry;
        xmi_algorithm_t            algorithm;
        size_t                     root;
        char                     * sndbuf;
        xmi_type_t               * stype;
        size_t                     stypecount;
        char                     * rcvbuf;
        xmi_type_t               * rtype;
        size_t                   * rtypecounts;
        size_t                   * rdispls;
    }xmi_gatherv_t;

    /**
     * \brief Create and post a non-blocking gatherv
     *
     * The gatherv_int routine
     *
     * \param[in]  request      Opaque memory to maintain internal message state.
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     * \param[in]  root         The root node for the gatherv operation
     * \param[in]  sndbuf       The base address of the buffers containing data to be sent
     * \param[in]  stype        A single type datatype
     * \param[in]  stypecount   type replication count.
     * \param[out] rcvbuf       The base address of the buffer for data reception
     * \param[in]  rtype        A single type datatype
     * \param[in]  rtypecounts  Array of type replication counts.  Size of geometry length
     * \param[in]  rdispls      Array of offsets into the rcvbuf.  Size of geometry length
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        xmi_xfer_type_t            xfer_type;
        xmi_event_function         cb_done;
        void                     * cookie;
        xmi_geometry_t             geometry;
        xmi_algorithm_t            algorithm;
        int                        root;
        char                     * sndbuf;
        xmi_type_t               * stype;
        int                        stypecount;
        char                     * rcvbuf;
        xmi_type_t               * rtype;
        int                      * rtypecounts;
        int                      * rdispls;
    }xmi_gatherv_int_t;


    /**
     * \brief Create and post a non-blocking allgatherv
     *
     * The allgatherv
     *
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     * \param[in]  sndbuf       The base address of the buffers containing data to be sent
     * \param[in]  stype        A single type datatype
     * \param[in]  stypecount   type replication count.
     * \param[out] rcvbuf       The base address of the buffer for data reception
     * \param[in]  rtype        A single type datatype
     * \param[in]  rtypecounts  Array of type replication counts.  Size of geometry length
     * \param[in]  rdispls      Array of offsets into the rcvbuf.  Size of geometry length
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        xmi_xfer_type_t            xfer_type;
        xmi_event_function         cb_done;
        void                     * cookie;
        xmi_geometry_t             geometry;
        xmi_algorithm_t            algorithm;
        char                     * sndbuf;
        xmi_type_t               * stype;
        size_t                     stypecount;
        char                     * rcvbuf;
        xmi_type_t               * rtype;
        size_t                   * rtypecounts;
        size_t                   * rdispls;
    }xmi_allgatherv_t;

    /**
     * \brief Create and post a non-blocking allgatherv
     *
     * The allgatherv_int
     *
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     * \param[in]  sndbuf       The base address of the buffers containing data to be sent
     * \param[in]  stype        A single type datatype
     * \param[in]  stypecount   type replication count.
     * \param[out] rcvbuf       The base address of the buffer for data reception
     * \param[in]  rtype        A single type datatype
     * \param[in]  rtypecounts  Array of type replication counts.  Size of geometry length
     * \param[in]  rdispls      Array of offsets into the rcvbuf.  Size of geometry length
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        xmi_xfer_type_t            xfer_type;
        xmi_event_function         cb_done;
        void                     * cookie;
        xmi_geometry_t             geometry;
        xmi_algorithm_t            algorithm;
        char                     * sndbuf;
        xmi_type_t               * stype;
        int                        stypecount;
        char                     * rcvbuf;
        xmi_type_t               * rtype;
        int                      * rtypecounts;
        int                      * rdispls;
    }xmi_allgatherv_int_t;


    /**
     * \brief Create and post a non-blocking scatter
     *
     * The scatter
     *
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     * \param[in]  root         Rank of the reduce root node.
     * \param[in]  sbuffer      Source buffer.
     * \param[in]  stype        Source buffer type
     * \param[in]  stypecount   Source buffer type count
     * \param[in]  rbuffer      Receive buffer.
     * \param[in]  rtype        Receive buffer layout
     * \param[in]  rtypecount   Receive buffer type count
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        xmi_xfer_type_t             xfer_type;
        xmi_event_function          cb_done;
        void                      * cookie;
        xmi_geometry_t              geometry;
        xmi_algorithm_t             algorithm;
        size_t                      root;
        char                      * sbuffer;
        xmi_type_t                * stype;
        size_t                      stypecount;
        char                      * rbuffer;
        xmi_type_t                * rtype;
        size_t                      rtypecount;
    }xmi_scatter_t;

    /**
     * \brief Create and post a non-blocking scatterv
     *
     * The scatterv
     *
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     * \param[in]  sndbuf       The base address of the buffers containing data to be sent
     * \param[in]  stype        A single type datatype
     * \param[in]  stypecounts  An array of type replication counts.  Size of geometry length
     * \param[in]  sdispls      Array of offsets into the sndbuf.  Size of geometry length
     * \param[in]  rbuffer      Receive buffer.
     * \param[in]  rtype        A single type datatype
     * \param[in]  rtypecount   Receive buffer type replication count
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        xmi_xfer_type_t             xfer_type;
        xmi_event_function          cb_done;
        void                      * cookie;
        xmi_geometry_t              geometry;
        xmi_algorithm_t             algorithm;
        size_t                      root;
        char                      * sndbuf;
        xmi_type_t                * stype;
        size_t                    * stypecounts;
        size_t                    * sdispls;
        char                      * rcvbuf;
        xmi_type_t                * rtype;
        size_t                      rtypecount;
    }xmi_scatterv_t;

    /**
     * \brief Create and post a non-blocking scatterv
     *
     * The scatterv_int
     *
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     * \param[in]  sndbuf       The base address of the buffers containing data to be sent
     * \param[in]  stype        A single type datatype
     * \param[in]  stypecounts  An array of type replication counts.  Size of geometry length
     * \param[in]  sdispls      Array of offsets into the sndbuf.  Size of geometry length
     * \param[in]  rbuffer      Receive buffer.
     * \param[in]  rtype        A single type datatype
     * \param[in]  rtypecount   Receive buffer type replication count
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        xmi_xfer_type_t             xfer_type;
        xmi_event_function          cb_done;
        void                      * cookie;
        xmi_geometry_t              geometry;
        xmi_algorithm_t             algorithm;
        int                         root;
        char                      * sndbuf;
        xmi_type_t                * stype;
        int                       * stypecounts;
        int                       * sdispls;
        char                      * rcvbuf;
        xmi_type_t                * rtype;
        int                         rtypecount;
    }xmi_scatterv_int_t;


    /**
     * \brief Create and post a non-blocking allreduce operation.
     *
     * The allreduce operation ...
     *
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     * \param[in]  sbuffer      Source buffer.
     * \param[in]  stype        Source buffer type
     * \param[in]  stypecount   Source buffer type count
     * \param[in]  rbuffer      Receive buffer.
     * \param[in]  rtype        Receive buffer layout
     * \param[in]  rtypecount   Receive buffer type count
     * \param[in]  dt           Element data type
     * \param[in]  op           Reduce operation
     *
     * \retval     0            Success
     *
     * \todo doxygen
     * \todo discuss collapsing XMI_dt into type type.
     */
    typedef struct
    {
        xmi_xfer_type_t            xfer_type;
        xmi_event_function         cb_done;
        void                     * cookie;
        xmi_geometry_t             geometry;
        xmi_algorithm_t            algorithm;
        char                     * sbuffer;
        xmi_type_t               * stype;
        size_t                     stypecount;
        char                     * rbuffer;
        xmi_type_t               * rtype;
        size_t                     rtypecount;
        xmi_dt                     dt;
        xmi_op                     op;
    }xmi_allreduce_t;


    /**
     * \brief Create and post a non-blocking scan operation.
     *
     * The scan operation ...
     *
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     * \param[in]  sbuffer      Source buffer.
     * \param[in]  stype        Source buffer type
     * \param[in]  stypecount   Source buffer type count
     * \param[in]  rbuffer      Receive buffer.
     * \param[in]  rtype        Receive buffer layout
     * \param[in]  rtypecount   Receive buffer type count
     * \param[in]  dt           Element data type
     * \param[in]  op           Reduce operation
     * \param[in]  exclusive    scan operation is exclusive of current node?
     *
     * \retval     0            Success
     *
     * \todo doxygen
     * \todo discuss collapsing XMI_dt into type type.
     */
    typedef struct
    {
        xmi_xfer_type_t            xfer_type;
        xmi_event_function         cb_done;
        void                     * cookie;
        xmi_geometry_t             geometry;
        xmi_algorithm_t            algorithm;
        char                     * sbuffer;
        xmi_type_t               * stype;
        size_t                     stypecount;
        char                     * rbuffer;
        xmi_type_t               * rtype;
        size_t                     rtypecount;
        xmi_dt                     dt;
        xmi_op                     op;
        int                        exclusive;
    }xmi_scan_t;

    /**
     * \brief Create and post a non-blocking barrier operation.
     * The barrier operation ...
     * \param      geometry     Geometry to use for this collective operation.
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \retval  0            Success
     *
     * \see XMI_Barrier_register
     *
     * \todo doxygen
     */
    typedef struct
    {
        xmi_xfer_type_t            xfer_type;
        xmi_event_function         cb_done;
        void                     * cookie;
        xmi_geometry_t             geometry;
        xmi_algorithm_t            algorithm;
    }xmi_barrier_t;


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
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     * \param[in]  user_header  single metadata to send to destination in the header
     * \param[in]  headerlen    length of the metadata (can be 0)
     * \param[in]  src          Base source buffer to broadcast.
     * \param[in]  stype        Datatype of the send buffer
     * \param[in]  stypecount   replication count of the send buffer data type
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        xmi_xfer_type_t             xfer_type;
        xmi_event_function          cb_done;
        void                      * cookie;
        xmi_geometry_t              geometry;
        xmi_algorithm_t             algorithm;
        void                      * user_header;
        size_t                      headerlen;
        void                      * src;
        xmi_type_t                * stype;
        size_t                      stypecount;
    }xmi_ambroadcast_t;
    /**
     * \brief The active message callback function, delivered to the user
     * \param[in]   root       system defined metadata:  root initiating the broadcast
     * \param[in]   comm       system defined metadata:  geometry id of the broadcast
     * \param[in]   sndlen     system defined metadata:  total number of bytes of the send
     * \param[in]   user_header user defined metadata:  active message header
     * \param[in]   headerlen  length of the user defined header (may be 0 bytes)
     * \param[out]  rcvbuf     target buffer for the incoming collective
     * \param[out]  rtype      data layout of the incoming collective
     * \param[out]  rtypecount replication count of the incoming collective
     * \param[out]  cb_info    data done callback to call on completion
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef void (*xmi_dispatch_ambroadcast_fn) (unsigned              root,
                                                 xmi_geometry_t        geometry,
                                                 const unsigned        sndlen,
                                                 void                * user_header,
                                                 const size_t          headerlen,
                                                 void               ** rcvbuf,
                                                 xmi_type_t          * rtype,
                                                 size_t              * rtypecount,
                                                 xmi_event_function  * const cb_info);


    /**
     * \brief Create and post a non-blocking active message scatter operation.
     * The Active Message scatter operation ...
     *
     * This is slightly more complicated than an AMBroadcast, because it allows
     * different headers and data buffers to be sent to everyone in the team.
     *
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     * \param[in]  headers      array of  metadata to send to destination
     * \param[in]  headerlength length of every header in the headers array
     * \param[in]  src          Base source buffer to scatter (size of geometry)
     * \param[in]  stype        single Datatype of the send buffer
     * \param[in]  stypecount   replication count of the send buffer data type
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        xmi_xfer_type_t             xfer_type;
        xmi_event_function          cb_done;
        void                      * cookie;
        xmi_geometry_t              geometry;
        void                      * headers;
        size_t                      headerlen;
        void                      * src;
        xmi_type_t                * stype;
        size_t                      stypecount;
    }xmi_amscatter_t;
    /**
     * \brief The active message callback function, delivered to the user
     * \param[in]   root       system defined metadata:  root initiating the scatter
     * \param[in]   comm       system defined metadata:  geometry id of the scatter
     * \param[in]   sndlen     system defined metadata:  total number of bytes of the send
     * \param[in]   user_header user defined metadata:  active message header
     * \param[in]   headerlen  length of the user defined header (may be 0 bytes)
     * \param[out]  rcvbuf     target buffer for the incoming collective
     * \param[out]  rtype      data layout of the incoming collective
     * \param[out]  rtypecount replication count of the incoming collective
     * \param[out]  cb_info    data done callback to call on completion
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef void (*xmi_dispatch_amscatter_fn) (unsigned             root,
                                               xmi_geometry_t       geometry,
                                               const unsigned       sndlen,
                                               void               * user_header,
                                               const size_t         headerlen,
                                               void              ** rcvbuf,
                                               xmi_type_t         * rtype,
                                               size_t             * rtypecount,
                                               xmi_event_function * const cb_info);

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
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     * \param[in]  headers      array of metadata to send to destination
     * \param[in]  headerlen    length of every header in headers array
     * \param[in]  rcvbuf       target buffer of the gather operation (size of geometry)
     * \param[in]  rtype        data layout of the incoming gather
     * \param[in]  rtypecount   replication count of the incoming gather
     * \param[in]  cb_info      data done callback to call on completion
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        xmi_xfer_type_t             xfer_type;
        xmi_event_function          cb_done;
        void                      * cookie;
        xmi_geometry_t              geometry;
        void                      * headers;
        size_t                      headerlen;
        void                      * rcvbuf;
        xmi_type_t                * rtype;
        size_t                      rtypecount;
    }xmi_amgather_t;
    /**
     * \brief The active message callback function, delivered to the user
     * \param[in]   root       system defined metadata:  root initiating the gather
     * \param[in]   comm       system defined metadata:  geometry id of the gather
     * \param[in]   sndlen     system defined metadata:  total number of bytes of the send
     * \param[in]   user_header user defined metadata:  active message header
     * \param[in]   headerlen  length of the user defined header (may be 0 bytes)
     * \param[out]  sbuf       source buffer for the incoming collective
     * \param[out]  stype      data layout of the send buffer
     * \param[out]  stypecount replication count of the send buffer
     * \param[out]  cb_info    data done callback to call on completion
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef void (*xmi_dispatch_amgather_fn) (unsigned             root,
                                              xmi_geometry_t       geometry_id,
                                              const unsigned       sndlen,
                                              void               * user_header,
                                              const size_t         headerlen,
                                              void              ** sbuf,
                                              xmi_type_t         * stype,
                                              size_t             * stypecount,
                                              xmi_event_function * const cb_info);

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
     * \param[in]  registration Protocol registration.
     * \param[in]  request      Opaque memory to maintain internal message state.
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     *                          \c NULL indicates the global geometry.
     * \param[in]  headers      metadata to send to destinations in the header
     * \param[in]  rcvbuf       target buffer of the reduce operation (size of geometry)
     * \param[in]  rtype        data layout of the incoming reduce
     * \param[in]  rtypecount   replication count of the incoming reduce
     * \param[in]  dt           datatype of reduction operation
     * \param[in]  op           operation type
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        xmi_xfer_type_t             xfer_type;
        xmi_event_function          cb_done;
        void                      * cookie;
        xmi_geometry_t              geometry;
        void                      * user_header;
        size_t                      headerlen;
        void                      * rcvbuf;
        xmi_type_t                * rtype;
        size_t                      rtypecount;
        xmi_dt                      dt;
        xmi_op                      op;
    }xmi_amreduce_t;

    /**
     * \brief The active message callback function, delivered to the user
     *        Note that the user does no math in these routines, just provides the buffer.
     *        The system provided metadata is information that the underlying messaging stack
     *        must transmit anyways, so it is delivered to the user for extra information about
     *        the incoming message
     * \param[in]   root       system defined metadata:  root initiating the reduce
     * \param[in]   comm       system defined metadata:  geometry id of the reduce
     * \param[in]   dt         system defined metadata:  datatype of reduction operation
     * \param[in]   op         system defined metadata:  operation type
     * \param[in]   sndlen     system defined metadata:  total number of bytes of the send
     * \param[in]   user_header user defined metadata:  active message header
     * \param[in]   headerlen  length of the user defined header (may be 0 bytes)
     * \param[out]  sbuf       source buffer for the incoming collective
     * \param[out]  stype      data layout of the send buffer
     * \param[out]  stypecount replication count of the send buffer
     * \param[out]  cb_info    data done callback to call on completion
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef void (*xmi_dispatch_amreduce_fn) (unsigned             root,
                                              xmi_geometry_t       geometry_id,
                                              const unsigned       sndlen,
                                              xmi_dt               dt,
                                              xmi_op               op,
                                              void               * user_header,
                                              const size_t         headerlen,
                                              void              ** sbuf,
                                              xmi_type_t         * stype,
                                              size_t             * stypecount,
                                              xmi_event_function * const cb_info);

    typedef union
    {
        xmi_xfer_type_t        xfer_type;
        xmi_allreduce_t        xfer_allreduce;
        xmi_broadcast_t        xfer_broadcast;
        xmi_reduce_t           xfer_reduce;
        xmi_allgather_t        xfer_allgather;
        xmi_allgatherv_t       xfer_allgatherv;
        xmi_allgatherv_int_t   xfer_allgatherv_int;
        xmi_scatter_t          xfer_scatter;
        xmi_scatterv_t         xfer_scatterv;
        xmi_scatterv_int_t     xfer_scatterv_int;
        xmi_scatter_t          xfer_gather;
        xmi_scatter_t          xfer_gatherv;
        xmi_scatterv_t         xfer_gatherv_int;
        xmi_alltoall_t         xfer_alltoall;
        xmi_alltoallv_t        xfer_alltoallv;
        xmi_alltoallv_int_t    xfer_alltoallv_int;
        xmi_ambroadcast_t      xfer_ambroadcast;
        xmi_amscatter_t        xfer_amscatter;
        xmi_amgather_t         xfer_amgather;
        xmi_amreduce_t         xfer_amreduce;
        xmi_scan_t             xfer_scan;
        xmi_barrier_t          xfer_barrier;
    }xmi_xfer_t;

    xmi_result_t XMI_Collective (xmi_context_t context, xmi_xfer_t *cmd);

#ifdef __cplusplus
};
#endif

#endif /* __xmi_collective_h__ */
