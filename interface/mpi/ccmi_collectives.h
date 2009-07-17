
#ifndef __ccmi_collective_h__
#define __ccmi_collective_h__

#include "interface/cm_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

  typedef CMQuad CCMI_CollectiveProtocol_t [32*2];     /**< Opaque datatype for collective protocols */

  /*  increased to 8196 - JEB */
  /*  ...doubled again - BRC  */
  typedef CMQuad CCMI_CollectiveRequest_t  [32*8*4]; /**< Opaque datatype for collective requests */

  typedef enum
  {
    CCMI_UNDEFINED_CONSISTENCY = -1,
    CCMI_RELAXED_CONSISTENCY,
    CCMI_MATCH_CONSISTENCY,
    CCMI_WEAK_CONSISTENCY,
    CCMI_CONSISTENCY_COUNT
  } CCMI_Consistency;
  
  //--------------------------------------------------
  //----- Communication CCMI_Subtasks ---------------------
  //----- Each communication operation is split ------
  //----- into phases of one or more sub tasks -------
  //--------------------------------------------------  
  #define  LINE_BCAST_MASK    (CCMI_LINE_BCAST_XP|CCMI_LINE_BCAST_XM|	\
			       CCMI_LINE_BCAST_YP|CCMI_LINE_BCAST_YM|	\
			       CCMI_LINE_BCAST_ZP|CCMI_LINE_BCAST_ZM)
  typedef enum
  {
    CCMI_PT_TO_PT_SUBTASK           =  0,      //Send a pt-to-point message
    CCMI_LINE_BCAST_XP              =  0x20,   //Bcast along x+
    CCMI_LINE_BCAST_XM              =  0x10,   //Bcast along x-
    CCMI_LINE_BCAST_YP              =  0x08,   //Bcast along y+
    CCMI_LINE_BCAST_YM              =  0x04,   //Bcast along y-
    CCMI_LINE_BCAST_ZP              =  0x02,   //Bcast along z+
    CCMI_LINE_BCAST_ZM              =  0x01,   //Bcast along z-
    CCMI_COMBINE_SUBTASK            =  0x0100,   //Combine the incoming message
    //with the local state
    CCMI_GI_BARRIER                 =  0x0200,
    CCMI_LOCKBOX_BARRIER            =  0x0300,
    CCMI_TREE_BARRIER               =  0x0400,
    CCMI_TREE_BCAST                 =  0x0500,
    CCMI_TREE_ALLREDUCE             =  0x0600,
    CCMI_REDUCE_RECV_STORE          =  0x0700,
    CCMI_REDUCE_RECV_NOSTORE        =  0x0800,
    CCMI_BCAST_RECV_STORE           =  0x0900,
    CCMI_BCAST_RECV_NOSTORE         =  0x0a00,
    CCMI_LOCALCOPY                  =  0x0b00,
    
    CCMI_UNDEFINED_SUBTASK          =  (~LINE_BCAST_MASK),
  } CCMI_Subtask;


  /* ********************************************************************* */
  /*                                                                       */
  /*               Initialize the collective API                           */
  /*                                                                       */
  /* ********************************************************************* */


  /**
   * \brief Initialize collective API. Not thread safe. This
   * function can be called only once.
   */

  int CCMI_Collective_initialize ();


  int CCMI_Generic_adaptor_advance();

  int CCMI_Collective_finalize ();

  /* ********************************************************************* */
  /*                                                                       */
  /*  geometry                                                             */
  /*                                                                       */
  /* ********************************************************************* */

  /**
   * \brief Opaque data type that holds geometry information.
   *
   * The external API does not reveal the internal structure of the object.
   *
   * \internal Implementations should verify that geometry objects
   *           need less space than \c sizeof(CCMI_Geometry_t).
   *
   * \todo     Need to adjust size to optimal level (currently 2048 bytes).
   */

  typedef CMQuad  CCMI_Geometry_t [32];


  /**
   * \brief The maximum number of geometries supported in the CCMI Runtime
   *
   */
  static const unsigned MAX_GEOMETRIES = 65536;

  /**
   *  \brief A callback to map the geometry id to the geometry
   *  structure. The runtime has to be able to freely translate
   *  between geometry id's and geometry structures.
   */
  typedef CCMI_Geometry_t * (*CCMI_mapIdToGeometry) (int comm);


  /* ********************************************************************* */
  /*                                                                       */
  /*      Barrier  Protocol Registration                                   */
  /*                                                                       */
  /* ********************************************************************* */


  /**
   * \brief Barrier protocol implementations.
   */

  /*
   * \brief Number of barrier protocols is meant to be number of network
   * barrier protocols. It is used in geometry initialization to pick the
   * best barrier for a given geometry. So move IPC and lockbox after
   * CCMI_NUM_BARRIER_PROTOCOLS
   */

  typedef enum
    {      
      CCMI_DEFAULT_BARRIER_PROTOCOL,
      CCMI_BINOMIAL_BARRIER_PROTOCOL,
      CCMI_NUM_BARRIER_PROTOCOLS,
    }
  CCMI_Barrier_Protocol;

  /**
   * \brief Barrier configuration.
   */

  typedef struct
  {
    CCMI_Barrier_Protocol   protocol;    /**< The barrier protocol implementation to register. */
    CCMI_mapIdToGeometry    cb_geometry; /**< This function is used to look-up the associated geometry when doing a barrier */
  }
  CCMI_Barrier_Configuration_t;

  /**
   * \brief Register the barrier protocol implementation specified by the
   *        barrier configuration.
   *
   * \warning After registering the protocol implementation it is illegal to
   *          deallocate the registration object.
   *
   * \param[out] registration  Opaque memory to maintain registration information.
   * \param[in]  configuration Barrier configuration information.
   *
   * \retval     0            Success
   * \retval     1            Invalid configuration
   *
   * \see CCMI_Barrier
   */

  int CCMI_Barrier_register (CCMI_CollectiveProtocol_t    * registration,
                             CCMI_Barrier_Configuration_t * configuration);


  /************************************************************************/
  /*             Geometry initialzation and query functions               */
  /*                                                                      */
  /************************************************************************/

  /**
   * \brief Initialize the geometry
   *
   * \param[out] geometry        Opaque geometry object to initialize
   * \param[in]  id              Unique identifier for this geometry
   *                             which globally represents this geometry
   * \param[in]  ranks           Array of nodes participating in the geometry
   * \param[in]  count           Number of nodes participating in the geometry
   * \param[in]  protocols       The barrier protocols to use on this geometry
   * \param[in]  nprotocols      The number of barrier protocols available
   * \param[in]  localprotocols  The local barrier protocols to use on this geometry
   * \param[in]  nlocalprotocols The number of local barrier protocols available
   * \param[in]  request         Storage for the barrier
   * \param[in]  numcolors       The number of "colors" to use in rectangular algorithms
   * \param[in]  globalcontext   True if this is the default global context
   */

  int CCMI_Geometry_initialize (CCMI_Geometry_t            * geometry,
                                unsigned                     id,
                                unsigned                   * ranks,
                                unsigned                     count,
                                CCMI_CollectiveProtocol_t ** protocols,
                                unsigned                     nprotocols,
                                CCMI_CollectiveProtocol_t ** localprotocols,
                                unsigned                     nlocalprotocols,
                                CCMI_CollectiveRequest_t   * request,
                                unsigned                     numcolors,
                                unsigned                     globalcontext);

  /**
   * \brief Analyze a protocol to ensure that it will run on the specified geometry
   *
   * \param[in] geometry  The current geometry object.
   * \param[in] protocol  A protocol to be tested.
   *
   * \return  Can the protocol run on the current geometry
   * \retval  CM_SUCCESS The protocl will run on the current geometry
   * \retval  ?????        The protocol does not support the current geometry
   */

  int CCMI_Geometry_analyze (CCMI_Geometry_t *geometry,
                             CCMI_CollectiveProtocol_t *protocol);

  /**
   * \brief Free any memory allocated inside of a geometry. Mostly
   * the alltoall permutation array right now.
   * \param[in] geometry The geometry object to free 
   * \retval CM_SUCCESS Memory free didn't fail
   */
  int CCMI_Geometry_free(CCMI_Geometry_t *geometry);

  /**
   * \brief Query devices available in the geometry
   *
   * \param[in]  geometry Opaque geometry object to query
   *
   * \return              Bitmask of available devices
   *
   * \see CCMI_Device
   */
  unsigned CCMI_Geometry_available (CCMI_Geometry_t * geometry);


  /**
   * \brief Create and post a non-blocking barrier operation.
   *
   * The barrier operation ...
   *
   * \param   geometry     Geometry to use for this collective operation.
   *                       \c NULL indicates the global geometry.
   * \param[in]  cb_done      Callback to invoke when message is complete.
   * \param[in]  consistency  Required consistency level.
   *
   * \retval  0            Success
   *
   * \see CCMI_Barrier_register
   *
   * \todo doxygen
   */

  int CCMI_Barrier (CCMI_Geometry_t     * geometry,
                    CM_Callback_t       cb_done,
                    CCMI_Consistency      consistency);


  /* ********************************************************************* */
  /*                                                                       */
  /*      Broadcast                                                        */
  /*                                                                       */
  /* ********************************************************************* */

  /**
   * \brief Asynchronous broadcast receive callback.
   *
   * \todo doxygen
   */
  typedef void * (*CCMI_RecvAsyncBroadcast) (unsigned           root,
                                             unsigned           comm,
                                             const unsigned     sndlen,
                                             unsigned         * rcvlen,
                                             char            ** rcvbuf,
                                             CM_Callback_t  * const cb_info);

  /**
   * \brief Asynchronous broadcast protocol implementations.
   */

  /**
   * \brief Broadcast protocol implementations.
   */

  typedef enum
    {
      CCMI_BINOMIAL_BROADCAST_PROTOCOL,   /**< Binomial broadcast. */
      CCMI_RING_BROADCAST_PROTOCOL,   /**< Ring broadcast. */
      CCMI_ASYNCBINOMIAL_BROADCAST_PROTOCOL,   /**< Async Binomial broadcast. */
      CCMI_NUM_BROADCAST_PROTOCOLS,
    }
  CCMI_Broadcast_Protocol;

  /**
   * \brief Broadcast configuration.
   */
  typedef struct
  {
    CCMI_Broadcast_Protocol protocol;  /**< The broadcast protocol implementation to register. */
    CCMI_RecvAsyncBroadcast cb_recv;   /**< Callback to invoke to receive an asynchronous broadcast message. */
    CCMI_mapIdToGeometry    cb_geometry;    /**< Callback to get the geometry when the async bcast packet arrives **/
    unsigned                isBuffered;     /**< Should the broadcast buffer unexpected incoming broadcast messages as opposed to calling cb_recv to allocate a buffer for the message*/
  }
  CCMI_Broadcast_Configuration_t;

  /**
   * \brief Register the broadcast protocol implementation specified by the
   *        broadcast configuration.
   *
   * \warning After registering the protocol information it is illegal to
   *          deallocate the registration object.
   *
   * \param[out] registration  Opaque memory to maintain registration information.
   * \param[in]  configuration Broadcast configuration information.
   *
   * \retval     0            Success
   *
   * \see CCMI_Broadcast
   */

  int CCMI_Broadcast_register (CCMI_CollectiveProtocol_t      * registration,
                               CCMI_Broadcast_Configuration_t * configuration);

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
   * \param[in]  consistency  Required consistency level.
   * \param[in]  geometry     Geometry to use for this collective operation.
   *                          \c NULL indicates the global geometry.
   * \param[in]  root         Rank of the node performing the broadcast.
   * \param[in]  src          Source buffer to broadcast.
   * \param[in]  bytes        Number of bytes to broadcast.
   *
   * \retval     0            Success
   *
   * \see CCMI_Broadcast_register
   *
   * \todo doxygen
   */

  int CCMI_Broadcast (CCMI_CollectiveProtocol_t  * registration,
                      CCMI_CollectiveRequest_t   * request,
                      CM_Callback_t    cb_done,
                      CCMI_Consistency   consistency,
                      CCMI_Geometry_t  * geometry,
                      unsigned           root,
                      char             * src,
                      unsigned           bytes);

  /* ********************************************************************* */
  /*                                                                       */
  /*      Allreduce                                                        */
  /*                                                                       */
  /* ********************************************************************* */

  /**
   * \brief Allreduce protocol implementations.
   */

  typedef enum
    {
      CCMI_RING_ALLREDUCE_PROTOCOL, /**< Rectangle/ring allreduce. */
      CCMI_ASYNC_SHORT_BINOMIAL_ALLREDUCE_PROTOCOL,   /**< Binomial short async allreduce. */
      CCMI_NUM_ALLREDUCE_PROTOCOLS
    }
  CCMI_Allreduce_Protocol;

  /**
   * \brief Allreduce configuration.
   */

  typedef struct
  {
    CCMI_Allreduce_Protocol protocol;    /**< The allreduce protocol implementation to register. */
    CCMI_mapIdToGeometry    cb_geometry; /**< Callback to get the geometry when the async packet arrives **/
    unsigned  reuse_storage:1;           /**< Reuse malloc'd storage across calls if set. Otherwise, free it. */
    unsigned  reserved:31;               /**< Currently unused */
  }
  CCMI_Allreduce_Configuration_t;


  /**
   * \brief Register the allreduce protocol implementation
   *        specified by the allreduce configuration.
   *
   * \warning After registering the protocol information it is illegal to
   *          deallocate the registration object.
   *
   * \param[out] registration  Opaque memory to maintain registration information.
   * \param[in]  configuration Allreduce configuration information.
   *
   * \retval     0            Success
   *
   * \see CCMI_Allreduce
   */

  int CCMI_Allreduce_register (CCMI_CollectiveProtocol_t      * registration,
                               CCMI_Allreduce_Configuration_t * configuration);


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
   * \param[in]  consistency  Required consistency level.
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
   * \see CCMI_Allreduce_register
   *
   * \todo doxygen
   */
  int CCMI_Allreduce (CCMI_CollectiveProtocol_t * registration,
                      CCMI_CollectiveRequest_t  * request,
                      CM_Callback_t   cb_done,
                      CCMI_Consistency  consistency,
                      CCMI_Geometry_t * geometry,
                      char            * sbuffer,
                      char            * rbuffer,
                      unsigned          count,
                      CM_Dt           dt,
                      CM_Op           op);

  /* ********************************************************************* */
  /*                                                                       */
  /*      Reduce                                                           */
  /*                                                                       */
  /* ********************************************************************* */


  /**
   * \brief Reduce protocol implementations.
   */

  typedef enum
    {
      CCMI_RING_REDUCE_PROTOCOL,  /**<  rectangle/ring reduce. */
      CCMI_NUM_REDUCE_PROTOCOLS
    }
  CCMI_Reduce_Protocol;

  /**
   * \brief Reduce configuration.
   */

  typedef struct
  {
    CCMI_Reduce_Protocol protocol;    /**< The reduce protocol implementation to register. */
    CCMI_mapIdToGeometry cb_geometry; /**< Callback to get the geometry when the async packet arrives. **/
    unsigned reuse_storage:1;         /**< Reuse malloc'd storage across calls if set. Otherwise, free it. */
    unsigned reserved:31;             /**< Reserved for future use. */
  }
  CCMI_Reduce_Configuration_t;


  /**
   * \brief Register the reduce protocol implementation
   *        specified by the reduce configuration.
   *
   * \warning After registering the protocol information it is illegal to
   *          deallocate the registration object.
   *
   * \param[out] registration  Opaque memory to maintain registration information.
   * \param[in]  configuration Reduce configuration information.
   *
   * \retval     0            Success
   *
   * \see CCMI_Reduce
   */

  int CCMI_Reduce_register (CCMI_CollectiveProtocol_t   * registration,
                            CCMI_Reduce_Configuration_t * configuration);

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
   * \param[in]  consistency  Required consistency level.
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
   * \see CCMI_Reduce_register
   *
   * \todo doxygen
   */
  int CCMI_Reduce (CCMI_CollectiveProtocol_t * registration,
                   CCMI_CollectiveRequest_t  * request,
                   CM_Callback_t   cb_done,
                   CCMI_Consistency  consistency,
                   CCMI_Geometry_t * geometry,
                   int               root,
                   char            * sbuffer,
                   char            * rbuffer,
                   unsigned          count,
                   CM_Dt           dt,
                   CM_Op           op);


#ifdef __cplusplus
};
#endif

#endif
