/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file include/dcmf.h
 * \brief Common external message layer interface.
 */
#ifndef __dcmf_h__
#define __dcmf_h__

#include <stdint.h>
#include <stddef.h>
#include <dcmf_impl.h>

/** ********************************************************************* **/
/**
 * \defgroup VERSION DCMF version information
 *
 * Applications may use the DCMF_VERSION_* information to enforce
 * compatibility with a specific DCMF version. For example:
 *
 * \code
 * #if (DCMF_VERSION_RELEASE == 0)
 *   #if (DCMF_VERSION_MAJOR == 1)
 *     #if (DCMF_VERSION_MINOR < 0)
 *       #error Incompatible dcmf minor version
 *     #endif
 *   #else
 *     #error Incompatible dcmf major version
 *   #endif
 * #else
 *   #error Incompatible dcmf release version
 * #endif
 * \endcode
 */
/** ********************************************************************* **/

/**
 * \brief The release version for this DCMF installation.
 *
 * The release version is incremented on a "release" boundary, as
 * as determined by the DCMF community.
 *
 * \ingroup VERSION
 */
#define DCMF_VERSION_RELEASE 0

/**
 * \brief The major version for this DCMF installation.
 *
 * The major version is incremented when changes are made to an existing
 * interface, typedef, structure, etc that will require an application
 * update. The major version is to be reset to zero for each time the
 * release version is incremented.
 *
 * \ingroup VERSION
 */
#define DCMF_VERSION_MAJOR 3

/**
 * \brief The minor version for this DCMF installation.
 *
 * The minor version is incremented when a new interface, typedef, structure,
 * etc is added that does not impact existing interfaces and will \b not
 * require an application update. The minor version is to be reset to zero
 * each time the release or major version is incremented.
 *
 * \ingroup VERSION
 */
#define DCMF_VERSION_MINOR 0

#ifdef __cplusplus
extern "C"
{
#endif

  /* ********************************************************************* */
  /*                                                                       */
  /*               Enumerations                                            */
  /*                                                                       */
  /* ********************************************************************* */

  /**
   * \brief Message consistency models specify the ordering characteristics
   *        of communication operations.
   */

  typedef enum
  {
    DCMF_MATCH_CONSISTENCY,        /**< Messages are sent in order and received in
                                    *   order, but not guaranteed to complete the
                                    *   send or receive in order. All protocol
                                    *   implementations \b MUST provide the match
                                    *   consistency model.
                                    */

    DCMF_RELAXED_CONSISTENCY,      /**< Messages are not sent or received in any
                                     *  order. Protocol implementations are not
                                     *  required to support the relaxed consistency
                                     *  model and may provide match consistency
                                     *  instead.
                                     */

    DCMF_SEQUENTIAL_CONSISTENCY,   /**< Messages are sent in order and received in
                                    *   order, and are guaranteed to complete the
                                    *   send and receive in order.
                                    */

    DCMF_WEAK_CONSISTENCY          /**< Deprecated. */
  }
  DCMF_Consistency;

  /**
   * \brief Communication devices
   * \todo Actually use these somewhere
   */

  typedef enum
  {
    DCMF_DEVICE_GI    =  1,  /**< Global Interrupt Device */
    DCMF_DEVICE_TREE  =  2,  /**< Tree Network Device */
    DCMF_DEVICE_TORUS =  4,  /**< Torus Network Device */
    DCMF_DEVICE_DMA   =  8,  /**< Torus DMA Device */
    DCMF_DEVICE_IPC   = 16,  /**< IPC Device */
  }
  DCMF_Device;

  /**
   * \brief DCMF Return codes
   */

  typedef enum
  {
    DCMF_SUCCESS = 0,  /**< Successful execution        */
    DCMF_NERROR  =-1,  /**< Generic error (-1)          */
    DCMF_ERROR   = 1,  /**< Generic error (+1)          */
    DCMF_INVAL,        /**< Invalid argument            */
    DCMF_UNIMPL,       /**< Function is not implemented */
    DCMF_EAGAIN,       /**< Not currently availible     */
    DCMF_SHUTDOWN,     /**< Rank has shutdown           */
    DCMF_CHECK_ERRNO,  /**< Check the errno val         */
    DCMF_OTHER,        /**< Other undefined error       */
  }
  DCMF_Result;

  /**
   * \brief DCMF Error callback results
   */
  typedef struct DCMF_Error_t
  {
    DCMF_Result result;
  }
  DCMF_Error_t;

  /**
   * \brief Message layer operation types
   */

  typedef enum
  {
    DCMF_UNDEFINED_OP = 0,
    DCMF_NOOP,
    DCMF_MAX,
    DCMF_MIN,
    DCMF_SUM,
    DCMF_PROD,
    DCMF_LAND,
    DCMF_LOR,
    DCMF_LXOR,
    DCMF_BAND,
    DCMF_BOR,
    DCMF_BXOR,
    DCMF_MAXLOC,
    DCMF_MINLOC,
    DCMF_USERDEFINED_OP,
    DCMF_OP_COUNT
  }
  DCMF_Op;

  /**
   * \brief Message layer data types
   */

  typedef enum
  {
    /* Standard/Primative DT's */
    DCMF_UNDEFINED_DT = 0,
    DCMF_SIGNED_CHAR,
    DCMF_UNSIGNED_CHAR,
    DCMF_SIGNED_SHORT,
    DCMF_UNSIGNED_SHORT,
    DCMF_SIGNED_INT,
    DCMF_UNSIGNED_INT,
    DCMF_SIGNED_LONG_LONG,
    DCMF_UNSIGNED_LONG_LONG,
    DCMF_FLOAT,
    DCMF_DOUBLE,
    DCMF_LONG_DOUBLE,
    DCMF_LOGICAL,
    DCMF_SINGLE_COMPLEX,
    DCMF_DOUBLE_COMPLEX,
    /* Max/Minloc DT's */
    DCMF_LOC_2INT,
    DCMF_LOC_SHORT_INT,
    DCMF_LOC_FLOAT_INT,
    DCMF_LOC_DOUBLE_INT,
    DCMF_LOC_2FLOAT,
    DCMF_LOC_2DOUBLE,
    DCMF_USERDEFINED_DT,
    DCMF_DT_COUNT
  }
  DCMF_Dt;


  /**
   * \brief Network type
   */
  typedef enum
  {
    DCMF_DEFAULT_NETWORK = 0, /**< Default network type. \b Guaranteed to work. */
    DCMF_TORUS_NETWORK,       /**< 3D-Torus / 1D-SMP network type. */
    DCMF_SOCKET_NETWORK,      /**< Unix socket network type. */
    DCMF_SHMEM_NETWORK,       /**< local shared memory "network" for smp nodes. */
    DCMF_NETWORK_COUNT        /**< Number of network types defined. */
  }
  DCMF_Network;

  #define DCMF_MAX_DIMS	4

  /**
   * \brief A structure to describe a network coordinate
   */
  typedef struct DCMF_NetworkCoord_t
  {
    DCMF_Network network; /**< Network type for the coordinates */
    union
    {
      struct
      {
        size_t x; /**< Torus network x coordinate */
        size_t y; /**< Torus network y coordinate */
        size_t z; /**< Torus network z coordinate */
        size_t t; /**< Torus network t coordinate */
      } torus;    /**< DCMF_TORUS_NETWORK coordinates */
      struct
      {
        size_t coords[DCMF_MAX_DIMS];
      } n_torus;
      struct
      {
        int recv_fd;   /**< Receive file descriptor */
        int send_fd;   /**< Send file descriptor    */
      } socket;   /**< DCMF_SOCKET_NETWORK coordinates */
      struct
      {
        size_t rank;   /**< Global rank of process */
        size_t peer;   /**< Local rank of process */
      } shmem;    /**< DCMF_SHMEM_NETWORK coordinates */
    };
  } DCMF_NetworkCoord_t;

  /* ********************************************************************* */
  /*                                                                       */
  /*               Data structures                                         */
  /*                                                                       */
  /* ********************************************************************* */

  /**
   * \brief Function signature of the completion callback function.
   *
   * \param[in] clientdata  Registered client data.
   * \param[in] error       Error result of the error event.
   */

  typedef void (*DCMF_Callback) (void         * clientdata,
                                 DCMF_Error_t * error);

  /**
   * \brief Completion callback information descriptor.
   *
   * Commonly used to signal completion of a particular operation to the
   * world outside the messaging library. These callbacks are invoked
   * during a call to DCMF_Messager_advance()
   */

  typedef struct DCMF_Callback_t
  {
    DCMF_Callback   function;    /**< Function to invoke */
    void          * clientdata;  /**< Argument to function */
  }
  DCMF_Callback_t;

  /** \brief The size of the processor name string (happens to be the same as the MPI version) */
#define DCMF_HARDWARE_MAX_PROCESSOR_NAME 128
  /** \brief The number of dimensions in the hardware information arrays */
#define DCMF_HARDWARE_NUM_DIMS 4

  /** \brief Hardware information data type */
  typedef struct
  {
    uint32_t lrank;  /**< Logical rank of the node in the existing mapping.                   */
    uint32_t lsize;  /**< Logical size of the partition, based on the mapping and -np value.  */
    uint32_t prank;  /**< Physical rank of the node (irrespective of mapping).                */
    uint32_t psize;  /**< Physical size of the partition (irrespective of mapping).           */

    union {
      uint32_t Size[DCMF_HARDWARE_NUM_DIMS];  /**< Array for the dimension size of the HW.    */
      struct {
        uint32_t tSize;  /**< The number of ranks possible on the node.                       */
        uint32_t zSize;  /**< The Z dimension size of the HW.                                 */
        uint32_t ySize;  /**< The Y dimension size of the HW.                                 */
        uint32_t xSize;  /**< The X dimension size of the HW.                                 */
      };
    };

    union {
      uint32_t Coord[DCMF_HARDWARE_NUM_DIMS]; /**< Array for the physical coordinates of the calling node. */
      struct {
        uint32_t tCoord; /**< The physical T rank       of the calling node.                  */
        uint32_t zCoord; /**< The physical Z coordinate of the calling node.                  */
        uint32_t yCoord; /**< The physical Y coordinate of the calling node.                  */
        uint32_t xCoord; /**< The physical X coordinate of the calling node.                  */
      };
    };

    union {
      uint32_t Torus[DCMF_HARDWARE_NUM_DIMS];  /**< Array for torus/mesh indicators.          */
      struct {
        uint32_t  tTorus; /**< Do we have a torus in the T dimension (Yes).                   */
        uint32_t  zTorus; /**< Do we have a torus in the Z dimension.                         */
        uint32_t  yTorus; /**< Do we have a torus in the Y dimension.                         */
        uint32_t  xTorus; /**< Do we have a torus in the X dimension.                         */
      };
    };

    uint32_t rankInPset; /**< Rank in the Pset (irrespective of mapping).                     */
    uint32_t sizeOfPset; /**< Number of nodes in the Pset (irrespective of mapping).          */
    uint32_t idOfPset;   /**< Rank of the Pset in the partition (irrespective of mapping).    */

    uint32_t clockMHz;   /**< Frequency of the core clock, in units of 10^6/seconds           */
    uint32_t memSize;    /**< Size of the core main memory, in units of 1024^2 Bytes          */
    char name[DCMF_HARDWARE_MAX_PROCESSOR_NAME]; /**< A unique name string for the calling node. */
  } DCMF_Hardware_t;

  /* ********************************************************************* */
  /*                                                                       */
  /*               Generic: messager object                                */
  /*                                                                       */
  /* ********************************************************************* */

  /**
   * \brief Opaque data type that holds all protocol registration information.
   *
   * The external API does not reveal the internal structure of the object.
   *
   * \internal Implementations should verify that protocol registration objects
   *           need less space than sizeof(DCMF_Protocol_t).
   *
   * \note     Need to adjust size to optimal level (currently 512 bytes).
   */
  typedef DCQuad DCMF_Protocol_t[DCMF_PROTOCOL_NQUADS];

  /**
   * \brief Opaque data type that holds request message state information.
   *
   * The external API does not reveal the internal structure of the object.
   *
   * \internal Implementations should verify that request message objects
   *           need less space than \c sizeof(DCMF_Request_t).
   */
  typedef DCQuad DCMF_Request_t [DCMF_REQUEST_NQUADS];

  /**
   * \brief Opaque data type that holds error callback context 
   *
   * The external API does not reveal the internal structure of the object.
   *
   * \internal Implementations should verify that request message objects
   *           need less space than \c sizeof(DCMF_ErrReq_t).
   */
  typedef DCQuad DCMF_Error_request_t [DCMF_ERROR_NQUADS];

  /**
   * \brief Opaque user data type that holds control message information.
   *
   * The external API does not reveal the internal structure of the object.
   *
   * \ingroup CONTROL 
   */
  typedef DCQuad DCMF_Control_t[DCMF_CONTROL_NQUADS];

  /**
   * \brief Opaque data type that holds all memory region registration information.
   *
   * The external API does not reveal the internal structure of the object.
   *
   * \internal Implementations should verify that memory region registration objects
   *           need less space than sizeof(DCMF_MemRegion_t).
   *
   * \ingroup ONESIDED
   */
  typedef DCQuad DCMF_Memregion_t[DCMF_MEMREGION_NQUADS];

  /**
   * \brief Initialize messager singleton. Not thread safe. This
   * function can be called multiple times.
   *
   * \retval 1 when called the first time
   * \retval 0 for every subsequent call.
   */

  unsigned DCMF_Messager_initialize ();

  /**
   * \brief Finalize messager singleton. Not thread safe. This
   * function can be called multiple times.
   *
   * \retval 1 when called the first time
   * \retval 0 for every subsequent call.
   */

  unsigned DCMF_Messager_finalize ();

  /**
   * \defgroup CRITICAL_SECTION Critical Section management
   * \brief These functions relate to the mutual-exclusion code in the message layer.
   *
   * The critical section code allows the message layer to be made
   * thead-safe.  Most functions will require that a DCMF library
   * client enter a critical section before calling one of the
   * functions.
   *
   * It is safe to use members of this group even before initializing
   * the messager.  In fact, this may be required: in interrupt mode,
   * ALL calls to the DCMF layer are required to be wrapped in CS
   * enter/exit calls.  Of course, several such calls may be made from
   * within the critical section.  Another limitation of interrupt
   * mode relates to CS enter nesting.  The interrupt thread may not
   * support nesting (as is the case on BG), so dispatch functions
   * (anything called from advance in any way) must not call CS enter
   * in interrupt mode.
   *
   * Some implementations may choose to allow for fine-grain locking.
   * To this end, an argument has been added to the critical section
   * functions.  Passing "0" is considered to be a request for the
   * global lock and can always be used.
   */

  /**
   * \ingroup CRITICAL_SECTION
   * \brief Enter a critical section
   * \param[in] x Used by those implementations doing fine-grained locking
   */

  extern int32_t DCMF_CriticalSection_flag;
  void DCMF_CriticalSection_enter_func(uint32_t x);
  void DCMF_CriticalSection_exit_func (uint32_t x);
  void DCMF_CriticalSection_cycle_func(uint32_t x);

#define DCMF_CriticalSection_enter(a)           \
({                                              \
  if (DCMF_CriticalSection_flag)                \
    DCMF_CriticalSection_enter_func(a);         \
})

  /**
   * \ingroup CRITICAL_SECTION
   * \brief Exit a critical section
   * \param[in] x Used by those implementations doing fine-grained locking
   */

#define DCMF_CriticalSection_exit(a)            \
({                                              \
  if (DCMF_CriticalSection_flag)                \
    DCMF_CriticalSection_exit_func(a);          \
})

  /**
   * \ingroup CRITICAL_SECTION
   * \brief Exit and Enter a critical section
   * \param[in] x Used by those implementations doing fine-grained locking
   */

#define DCMF_CriticalSection_cycle(a)           \
({                                              \
  if (DCMF_CriticalSection_flag)                \
    DCMF_CriticalSection_cycle_func(a);         \
})


  #define DCMF_CHANNEL_ALL   ((unsigned)-1)
  #define DCMF_PROTOCOL_HAS_NO_CHANNELS ((unsigned)-2)
  #define DCMF_PROTOCOL_NULL NULL
  DCMF_Result DCMF_Channel_info   (DCMF_Protocol_t* p, unsigned* base, unsigned* count, unsigned* current);
  DCMF_Result DCMF_Channel_acquire(unsigned channel);
  DCMF_Result DCMF_Channel_release();
  DCMF_Result DCMF_Channel_handoff(unsigned channel, DCMF_Callback_t cb);

  /**
   * \brief Retrieve DMA channel being used by this thread
   *
   * \return currently used DMA channel
   */
  unsigned DCMF_Channel_current();

  /**
   * \brief Set DMA channel to be used by this thread
   *
   * \return DCMF_SUCCESS on success
   */
  #define DCMF_MESSAGER_USE_CHANNEL
  int DCMF_Messager_use_channel( unsigned channel );


  typedef struct
  {
    unsigned channel;
//    unsigned block;
//    unsigned timeout;
  } DCMF_Messager_advance_options;

  /**
   * \brief Messager advance loop.
   *
   * \todo add more doxygen here. This is an important API and needs more information
   *
   * \return Number of events processed
   */

  unsigned DCMF_Messager_advance ();

  /**
   * \brief Messager advance loop with extra options.
   *
   * \return Number of events processed
   */

  unsigned DCMF_Messager_advance_expert (DCMF_Messager_advance_options opt);

  /**
   * \brief Retrieve the linear rank of current node. Reentrant.
   *
   * \return Node rank
   */

  size_t DCMF_Messager_rank ();

  /**
   * \brief Retrieve the number of processors running in the current job.
   * Reentrant.
   *
   * \return Number of processors
   */

  size_t DCMF_Messager_size ();

  /**
   * \brief Retrieve the hardware information about the block.
   *
   * \param[out] hw Hardware information
   */

  DCMF_Result DCMF_Hardware (DCMF_Hardware_t *hw);

  /**
   * \brief Map a network coordinate to a global rank. Thread safe.
   *
   * \note Output parameters are only valid on \c DCMF_SUCCESS.
   *
   * \param[in]  address  Network address to query
   * \param[out] rank     Global rank
   * \param[out] network  The type of network
   */

  DCMF_Result DCMF_Messager_network2rank(const DCMF_NetworkCoord_t * address,
                                         size_t                    * rank,
                                         DCMF_Network              * network);

  /**
   * \brief Retrieve network coordinates for a global rank. Thread safe.
   *
   * \note Output parameters are only valid on \c DCMF_SUCCESS.
   *
   * \param[in]  rank    Global rank to query
   * \param[in]  network Type of network to query
   * \param[out] address Network address structure result
   */

  DCMF_Result DCMF_Messager_rank2network(size_t                rank,
                                         DCMF_Network          network,
                                         DCMF_NetworkCoord_t * address);

  /**
   * \defgroup CONFIGURE Messager configure information
   */

  /**
   * \ingroup CONFIGURE
   * \brief Select a thread mode
   * \note These have the same meaning at the thread levels defined in MPI2
   */
  typedef enum
  {
    DCMF_THREAD_SINGLE     = 0, /**< Single     */
    DCMF_THREAD_FUNNELED   = 1, /**< Funneled   */
    DCMF_THREAD_SERIALIZED = 2, /**< Serialized */
    DCMF_THREAD_MULTIPLE   = 3  /**< Multiple   */
  }
  DCMF_Thread;

  /**
   * \ingroup CONFIGURE
   * \brief Select an Interrupt mode
   */
  typedef enum
  {
    DCMF_INTERRUPTS_OFF = 0,    /**< Do not request interrupt enablement */
    DCMF_INTERRUPTS_ON  = 1     /**< Demand that interrupts be enabled   */
  }
  DCMF_Interrupts;

  /**
   * \ingroup CONFIGURE
   * \brief The object used to pass around API Configuration info
   */
  typedef struct
  {
    DCMF_Thread     thread_level;   /**< The requested/existing thread level */
    DCMF_Interrupts interrupts;     /**< The requested/existing thread level */
  }
  DCMF_Configure_t;

  /**
   * \ingroup CONFIGURE
   * \brief Set and/or Fetch the configuration information
   *
   * \note It is vital that the configuration not be changed inside a
   *       DCMF_CriticalSection region.  Performance enhancements may
   *       disable the nest-counting used for critical sections until
   *       threads and interrupts are enabled.  This would be
   *       problematic if one attempted to leave a critical section
   *       after turning on the nest-counter.
   *
   * It is acceptable to pass NULL for either of the parameters.
   * It is acceptable to pass the same address for both of the
   * parameters (the input data is read out before the config data is
   * written back).
   *
   * \param[in]  config  The reuqested configuration
   * \param[out] out     The actual configuration after updating
   * \return             DCMF_SUCCESS
   */

  DCMF_Result DCMF_Messager_configure (DCMF_Configure_t * config, DCMF_Configure_t * out);

  /**
   * \brief  Returns an elapsed time on the calling processor.
   * \note   This has the same definition as MPI_Wtime
   * \return Time in seconds since an arbitrary time in the past.
   */

  double DCMF_Timer();

  /**
   * \brief  Computes the smallest clock resolution theoretically possible
   * \note   This has the same definition as MPI_Wtick
   * \return The duration of a single timebase clock increment in seconds
   */

  double DCMF_Tick();

  /**
   * \brief  Returns the number of cycles elapsed on the calling processor.
   * \return Number of cycles since an arbitrary time in the past.
   */

  unsigned long long DCMF_Timebase();


  /** ********************************************************************* **/
  /**
   * \defgroup ERRORHANDLER Error handling functions
   */
  /** ********************************************************************* **/

  /**
   * \brief Function signature of the error callback.
   *
   * The error callback is invoked when a global error condition occurs.
   *
   * \param[in] error This error result of the error event
   * \param[in] peer  Remote rank associated with this error event
   *
   */

  typedef void (*DCMF_GlobalError) (DCMF_Error_t *error,
                                    size_t        peer);

  /**
   * \brief Register a global error handler with the message layer.
   *
   * \param[in] req   Opaque storage for the error request
   * \param[in] func  function to register
   *
   *  This function is called asynchronously when error events occur
   *  The typical example is if a network is disconnected
   */

  DCMF_Result DCMF_Error_register (DCMF_Error_request_t *req,
                                   DCMF_GlobalError      func);

  /** ********************************************************************* **/
  /**
   * \defgroup PT2PT Point-to-point transfer operations
   */
  /** ********************************************************************* **/

  /** ********************************************************************* **/
  /**
   * \defgroup TWOSIDED Two-sided point-to-point transfer operations
   * \ingroup PT2PT
   */
  /** ********************************************************************* **/

  /** ********************************************************************* **/
  /**
   * \brief 2-sided point-to-point send
   *
   * This is a description of the send API
   *
   * \defgroup SEND Send
   * \ingroup TWOSIDED
   */
  /** ********************************************************************* **/

  /**
   * \brief Function signature of the receive callback which is invoked when a
   *        single-packet point-to-point send message arrives at the target node.
   *
   * DCMF_Send() operation receive short callback implementations must process
   * the data buffer sent from the origin node. After this callback returns the
   * data in the \c src buffer will be discarded.
   *
   * This callback is used to receive the source buffer for a send operation
   * which has already been completely transfered to the receive node messager.
   * Receive events are processed, and callbacks invoked, during
   * DCMF_Messager_advance().
   *
   * Typical receive short callback implementations will copy the data from the
   * \c src buffer into another receive buffer.
   *
   * \note The implementation \b will \b always invoke the \e short callback
   *       when zero bytes of source data are sent from the origin rank. In all
   *       other cases the implementation may invoke the DCMF_RecvSend()
   *       callback instead.
   *
   * \param[in]  clientdata Opaque client data specified when the send operation
   *                        was registered.
   * \param[in]  msginfo    Array of opaque application control information
   *                        quads sent in this operation.
   * \param[in]  count      Number of quads of opaque application control
   *                        information sent in this operation. \b Must be
   *                        greater than or equal to zero and less than or
   *                        equal to 7.
   * \param[in]  peer       Rank of the node that originated the send operation.
   * \param[in]  src        Data buffer received from the origin node.
   * \param[in]  bytes      Number of bytes received from the origin node.
   *
   * \see DCMF_RecvSend
   * \see DCMF_Messager_advance
   * \see DCMF_Send_register
   * \see DCMF_Send
   *
   * \ingroup SEND
   */

  typedef void (*DCMF_RecvSendShort) (void           * clientdata,
                                      const DCQuad   * msginfo,
                                      unsigned         count,
                                      size_t           peer,
                                      const char     * src,
                                      size_t           bytes);

  /**
   * \brief Function signature of the receive callback which is invoked when a
   *        point-to-point send message data stream arrives at the target node.
   *
   * DCMF_Send() operation receive callback implementations must allocate and
   * return an opaque request object to maintain the internal receive state of
   * the send operation, and must provide a callback, specified by the \c cb_done
   * output parameter, to deallocate the request object when the send operation
   * is complete.
   *
   * Implementations must also provide a buffer, specified by the \c rcvbuf and
   * \c rcvlen parameters, to receive the data streaming from the origin node.
   * If the \c sndlen is greater than the \c rcvlen the transfer will complete,
   * and invoke the \c cb_done callback, when the first \c rcvlen number of
   * bytes have been received into \c rcvbuf.
   *
   * \note This callback is used to initialize an asynchronous receive of a
   *       source buffer for a send operation. Consequently, the node must
   *       actively participate, via callbacks, at the beginning and at the end
   *       of the receive of the send operation. Receive events are processed,
   *       and callbacks invoked, during DCMF_Messager_advance().
   *
   * \param[in]  clientdata Opaque client data specified when the send operation
   *                        was registered.
   * \param[in]  msginfo    Array of opaque application control information
   *                        quads sent in this operation.
   * \param[in]  count      Number of quads of opaque application control
   *                        information sent in this operation. \b Must be
   *                        greater than or equal to zero and less than or
   *                        equal to 7.
   * \param[in]  peer       Rank of the node that originated the send operation.
   * \param[in]  sndlen     Number of bytes the sender wishes to transfer.
   * \param[out] rcvlen     Number of bytes the receiver is willing to accept.
   * \param[out] rcvbuf     Location of the buffer to receive the data.
   * \param[out] cb_done    Callback to invoke when the transfer has completed.
   *                        The callback must deallocate the request object
   *                        returned by this function to avoid memory leaks.
   *
   * \return                Pointer to allocated memory for request object.
   *
   * \see DCMF_Messager_advance
   * \see DCMF_Send_register
   * \see DCMF_Send
   *
   * \ingroup SEND
   */

  typedef DCMF_Request_t * (*DCMF_RecvSend) (void             * clientdata,
                                             const DCQuad     * msginfo,
                                             unsigned           count,
                                             size_t             peer,
                                             size_t             sndlen,
                                             size_t           * rcvlen,
                                             char            ** rcvbuf,
                                             DCMF_Callback_t  * cb_done);

  /**
   * \brief Send protocol implementations.
   *
   * This information is used when a send protocol is registered using
   * DCMF_Send_register().
   *
   * \see DCMF_Send_Configuration_t
   * \see DCMF_Send_register
   * \ingroup SEND
   */

  typedef enum
  {
    /**
     * DCMF_Send_register() with the default send protocol is \b guaranteed
     * to return DCMF_SUCCESS. Registration of all other protocols may return
     * DCMF_UNIMPL or another error.
     */
    DCMF_DEFAULT_SEND_PROTOCOL,

    /**
     * \brief The eager protocol is a latency optimized point-to-point send.
     *
     * Typically, an envelope message is sent to the target node followed by a
     * data stream which is processed in strict sequential order by the
     * target node.
     */
    DCMF_EAGER_SEND_PROTOCOL,

    /**
     * \brief The rendezvous protocol is a bandwidth optimized point-to-point send.
     *
     * Typically, a request-to-send (rts) message is sent to the target node
     * which may respond with a clear-to-send (cts) message such that the
     * origin node may transfer the source buffer in a bandwidth-optimized
     * fashion, or, on systems with a DMA, the target node may issue a get
     * message to retrieve the source buffer from the origin node.
     */
    DCMF_RZV_SEND_PROTOCOL,

    DCMF_USER0_SEND_PROTOCOL, /**< Implementation-specific send protocol. */
    DCMF_USER1_SEND_PROTOCOL, /**< Implementation-specific send protocol. */
    DCMF_USER2_SEND_PROTOCOL, /**< Implementation-specific send protocol. */
    DCMF_USER3_SEND_PROTOCOL, /**< Implementation-specific send protocol. */

    DCMF_SEND_PROTOCOL_COUNT  /**< Number of send protocols defined. */
  }
  DCMF_Send_Protocol;

  /**
   * \brief Point-to-point send configuration.
   *
   * \see DCMF_Send_register
   * \ingroup SEND
   */

  typedef struct
  {
    DCMF_Send_Protocol protocol;                 /**< The send protocol implementation to register. */
    DCMF_Network       network;                  /**< The network fabric to be used for communication. */
    DCMF_RecvSendShort cb_recv_short;            /**< Callback to invoke to receive a single-packet send message. */
    void             * cb_recv_short_clientdata; /**< client data. */
    DCMF_RecvSend      cb_recv;                  /**< Callback to invoke to receive a multi-packet send message. */
    void             * cb_recv_clientdata;       /**< client data. */
  }
  DCMF_Send_Configuration_t;


  /**
   * \brief Register the point-to-point send protocol implementation.
   *
   * \warning After registering the protocol implementation it is illegal to
   *          deallocate the registration object.
   *
   * \param[out] registration  Opaque memory to maintain registration information.
   * \param[in]  configuration Point-to-point send configuration information.
   *
   * \see DCMF_Send
   *
   * \ingroup SEND
   */

  DCMF_Result DCMF_Send_register (DCMF_Protocol_t           * registration,
                                  DCMF_Send_Configuration_t * configuration);

  /**
   * \brief Start a point-to-point send operation.
   *
   * The send operation transfers data from the local node to the remote node.
   * The appropriate registered receive callback function is invoked on the
   * remote node to receive the data.
   *
   * To make progress on the send operation the local node must invoke the
   * DCMF_Messager_advance() function until the \c cb_done callback is invoked.
   *
   * \note The messager implementation \b must invoke the registered \e short
   *       receive callback, DCMF_RecvSendShort(), when zero bytes of source
   *       data are transfered. In all other cases the messager implementation
   *       \b may invoke the registered \e asynchronous receieve callback,
   *       DCMF_RecvSend().
   *
   * \warning Until the message done callback is invoked, it is illegal to
   *          restart the operation, touch the attached buffers, or deallocate
   *          the request object.
   *
   * \param[in]  registration Protocol registration.
   * \param[in]  request      Opaque memory to maintain internal message state.
   * \param[in]  cb_done      Callback to invoke when message is complete.
   *                          Completion notification will not occur if a
   *                          \c NULL function pointer is specified. \c NULL
   *                          completion callbacks are dangerous and their use
   *                          is not recommended.
   * \param[in]  consistency  Required consistency level.
   * \param[in]  rank         Remote node rank.
   * \param[in]  bytes        Number of bytes to send.
   * \param[in]  src          Source data buffer to send. It is \b illegal to touch
   *                          or deallocate the source data buffer until the
   *                          message done callback is invoked. The source data
   *                          buffer is refreshed for each subsequent DCMF_Restart().
   * \param[in]  msginfo      Array of opaque application control information
   *                          quads to transfer. Control information is copied
   *                          internally and is safe to access or deallocate
   *                          immediately after the DCMF_Send() function returns.
   *                          The control information is \b not refreshed when the
   *                          request is restarted with DCMF_Restart().
   * \param[in]  count        Number of quads of opaque application control
   *                          information to transfer. \b Must be greater than or
   *                          equal to zero and less than or equal to 7.
   *
   * \see DCMF_Messager_advance
   * \see DCMF_Send_register
   * \see DCMF_RecvSendShort
   * \see DCMF_RecvSend
   * \see DCMF_Restart
   *
   * \ingroup SEND
   */

  DCMF_Result DCMF_Send (DCMF_Protocol_t * registration,
                         DCMF_Request_t  * request,
                         DCMF_Callback_t   cb_done,
                         DCMF_Consistency  consistency,
                         size_t            rank,
                         size_t            bytes,
                         char            * src,
                         DCQuad          * msginfo,
                         unsigned          count);

  /** ********************************************************************* **/
  /**
   * \brief 2-sided point-to-point control
   *
   * This is a description of the control API
   *
   * \defgroup CONTROL Control
   * \ingroup TWOSIDED
   */
  /** ********************************************************************* **/

  /**
   * \brief Function signature of the receive callback which is invoked when 
   *        an \e out-of-band point-to-point control message arrives at the
   *        target node.
   *
   * \note This callback is used to receive control information which has been
   *       completely transferred to the receive node messager. Receive events
   *       are processed, and callbacks invoked, during DCMF_Messager_advance().
   *
   * \param[in]  clientdata Opaque client data specified when the control
   *                        operation was registered.
   * \param[in]  info       Opaque control message information received from
   *                        the peer node.
   * \param[in]  peer       Rank of the node that originated the control operation.
   *
   * \see DCMF_Control_register
   * \see DCMF_Control
   * \see DCMF_Messager_advance
   *
   * \ingroup CONTROL
   */

  typedef void (*DCMF_RecvControl) (void                 * clientdata,
                                    const DCMF_Control_t * info,
                                    size_t                 peer);

  /**
   * \brief Control Protocols that can be registered.
   *
   * \see DCMF_Control_Configuration_t
   * \see DCMF_Control_register
   *
   * \ingroup CONTROL
   */
  typedef enum
  {
    /**
     * DCMF_Control_register() with the default control protocol is \b guaranteed
     * to return DCMF_SUCCESS. Registration of all other protocols may return
     * DCMF_UNIMPL or another error.
     */
    DCMF_DEFAULT_CONTROL_PROTOCOL,
    DCMF_OVERSEND_CONTROL_PROTOCOL,    /**< Using DCMF_Send - always available. */

    DCMF_USER0_CONTROL_PROTOCOL, /**< Implementation-specific control protocol. */
    DCMF_USER1_CONTROL_PROTOCOL, /**< Implementation-specific control protocol. */
    DCMF_USER2_CONTROL_PROTOCOL, /**< Implementation-specific control protocol. */
    DCMF_USER3_CONTROL_PROTOCOL, /**< Implementation-specific control protocol. */

    DCMF_CONTROL_PROTOCOL_COUNT  /**< Number of control protocols defined. */
  }
  DCMF_Control_Protocol;

  /**
   * \brief Control Protocol Configuration Information
   *
   * This information is used when a control protocol is registered using
   * DCMF_Control_register().
   *
   * \see DCMF_Control_register
   *
   * \ingroup CONTROL
   */
  typedef struct
  {
    DCMF_Control_Protocol protocol; /**< The control protocol being registered */
    DCMF_Network          network;                  /**< The network fabric to be used for communication. */
    DCMF_RecvControl      cb_recv;  /**< The function invoked on the destination
                                         when a control message arrives using
                                         the registered control protocol.      */
    void                * cb_recv_clientdata; /**< client data */
  }
  DCMF_Control_Configuration_t;

  /**
   * \brief Register the point-to-point control protocol implementation.
   *
   * \warning After registering the protocol implementation it is illegal to
   *          deallocate the registration object.
   *
   * \param[out] registration  Opaque memory to maintain registration information.
   * \param[in]  configuration Point-to-point control configuration information.
   *
   * \see DCMF_Control
   *
   * \ingroup CONTROL
   */

  DCMF_Result DCMF_Control_register (DCMF_Protocol_t              * registration,
                                     DCMF_Control_Configuration_t * configuration);

  /**
   * \brief Send an \e out-of-band point-to-point control message.
   *
   * The control message operation transfers a fixed amount of control
   * information from the local node to the remote node. The transfer is
   * completed \e out-of-band with respect to regular operations and no message
   * ordering is guaranteed between control and regular operations.
   *
   * Control operations still have an associated consistency model which
   * specifies how the control message is ordered relative to other control
   * messages.
   *
   * \note This is a blocking operation. Once the function returns, the
   *       user may safely access all associated buffers. DCMF_Messager_advance()
   *       does not need to be called in order to make progress on control
   *       operation messages.
   *
   * \param[in]  registration Protocol registration.
   * \param[in]  consistency  Required consistency level.
   * \param[in]  rank         Destination rank.
   * \param[in]  info         Opaque control message information.
   *
   * \see DCMF_RecvControl
   * \see DCMF_Control_register
   *
   * \ingroup CONTROL
   */

  DCMF_Result DCMF_Control (DCMF_Protocol_t * registration,
                            DCMF_Consistency  consistency,
                            size_t            rank,
                            DCMF_Control_t  * info);


  /** ********************************************************************* **/
  /**
   * \defgroup ONESIDED One-sided point-to-point transfer operations
   * \ingroup PT2PT
   */
  /** ********************************************************************* **/

  /**
   * \brief Create a memory region for 1-sided operations
   *
   * \warning After creating the memory region it is illegal to write into or
   *          deallocate the opaque memory region object. Additionally, it is
   *          illegal to free or otherwise deallocate the memory buffer managed
   *          by the memory region.
   *
   * \param[out] memregion Memory region opaque data type to be initialized
   * \param[out] bytes_out Actual number of bytes pinned
   * \param[in]  bytes_in  Requested number of bytes to be pinned
   * \param[in]  base      Requested base virtual address
   * \param[in]  options   ???
   *
   * \see DCMF_Memregion_destroy
   * \see DCMF_Memregion_query
   *
   * \retval DCMF_SUCCESS The entire memory region, or a portion of the memory
   *                      region was pinned. The actual number of bytes pinned
   *                      from the start of the buffer is returned in the
   *                      \c bytes_out field. The memory region must be free'd
   *                      with DCMF_Memregion_destroy().
   *
   * \retval DCMF_EAGAIN  The memory region was not pinned due to an unavailable
   *                      resource. The memory region does not need to be freed
   *                      with DCMF_Memregion_destroy().
   *
   * \retval DCMF_INVAL   An invalid parameter value was specified.
   *
   * \retval DCMF_ERROR   The memory region was not pinned and does not need to
   *                      be freed with DCMF_Memregion_destroy().
   *
   * \ingroup ONESIDED
   */

  DCMF_Result DCMF_Memregion_create (DCMF_Memregion_t               * memregion,
                                     size_t                         * bytes_out,
                                     size_t                           bytes_in,
                                     void                           * base,
                                     uint64_t                         options);

  /**
   * \brief Destroy a memory region
   *
   * After a memory region is destroyed it is again legal to write into or
   * deallocate the memory used by the memory region object.
   *
   * \warning It is illegal to query the contents of the memory region with the
   *          DCMF_Memregion_query() function after destroying the memory region.
   *
   * \param[in] memregion Memory region opaque data type to be destroyed
   *
   * \see DCMF_Memregion_create
   * \see DCMF_Memregion_query
   *
   * \ingroup ONESIDED
   */

  DCMF_Result DCMF_Memregion_destroy (DCMF_Memregion_t * memregion);

  /**
   * \brief Query a memory region
   *
   * \warning This function may fail in an unpredictable way if an uninitialized
   *          memory region object is queried.
   *
   * \param[in]  memregion Memory region opaque data type to be queried
   * \param[out] bytes     Number of bytes in the memory region
   * \param[out] base      Base virtual address of the memory region
   *
   * \see DCMF_Memregion_create
   * \see DCMF_Memregion_destroy
   *
   * \ingroup ONESIDED
   */

  DCMF_Result DCMF_Memregion_query (DCMF_Memregion_t *  memregion,
                                    size_t           *  bytes,
                                    void             ** base);

  /** ********************************************************************* **/
  /**
   * \brief one-sided point-to-point put
   *
   * This is a description of the put API
   *
   * \defgroup PUT Put
   * \ingroup ONESIDED
   */
  /** ********************************************************************* **/

  /**
   * \brief Put protocol implementations.
   *
   * \ingroup PUT
   */

  typedef enum
  {
    /**
     * DCMF_Put_register() with the default put protocol is \b guaranteed
     * to return DCMF_SUCCESS. Registration of all other protocols may return
     * DCMF_UNIMPL or another error.
     */
    DCMF_DEFAULT_PUT_PROTOCOL,
    DCMF_OVERSEND_PUT_PROTOCOL, /**< Uses DCMF_Send - always available. */

    DCMF_USER0_PUT_PROTOCOL, /**< Implementation-specific put protocol. */
    DCMF_USER1_PUT_PROTOCOL, /**< Implementation-specific put protocol. */
    DCMF_USER2_PUT_PROTOCOL, /**< Implementation-specific put protocol. */
    DCMF_USER3_PUT_PROTOCOL, /**< Implementation-specific put protocol. */

    DCMF_PUT_PROTOCOL_COUNT  /**< Number of put protocols defined. */
  }
  DCMF_Put_Protocol;

  /**
   * \brief 1-sided point-to-point put configuration.
   *
   * \ingroup PUT
   */

  typedef struct
  {
    DCMF_Put_Protocol protocol;      /**< The put protocol implementation to register. */
    DCMF_Network       network;                  /**< The network fabric to be used for communication. */
  }
  DCMF_Put_Configuration_t;


  /**
   * \brief Register the point-to-point put protocol implementation specified by the
   *        point-to-point put configuration.
   *
   * \warning After registering the protocol implementation it is illegal to
   *          deallocate the registration object.
   *
   * \param[out] registration  Opaque memory to maintain registration information.
   * \param[in]  configuration Point-to-point put configuration information.
   *
   * \see DCMF_Put
   *
   * \ingroup PUT
   */

  DCMF_Result DCMF_Put_register (DCMF_Protocol_t          * registration,
                                 DCMF_Put_Configuration_t * configuration);


  /**
   * \brief Start a point-to-point put operation.
   *
   * The put operation transfers data from the local node to the remote node.
   *
   * Two callbacks may be specified for a put operation. The first callback to
   * be invoked is the cb_done callback when the entire source buffer has been
   * injected onto the network. At this point the source buffer may be accessed
   * or deallocated.
   *
   * The second callback to be invoked is the cb_ack callback when the entire
   * source buffer has been received by the remote node.
   *
   * The request object may not be touched, deallocated, or restarted until
   * all callbacks have been invoked.
   *
   * \warning Completion notification will not occur if a \c NULL function
   *          pointer is specified for both callbacks. \c NULL completion
   *          callbacks are dangerous and their use is not recommended.
   *
   * \param[in]  registration  Protocol registration.
   * \param[in]  request       Opaque memory to maintain internal message state.
   * \param[in]  cb_done       Callback to invoke when the message is locally
   *                           complete and the source buffer may be touched.
   * \param[in]  consistency   Required consistency level.
   * \param[in]  rank          Remote node rank.
   * \param[in]  bytes         Number of bytes to put to the remote node.
   * \param[in]  src_memregion Memory region on the local node.
   * \param[in]  dst_memregion Memory region on the remote node.
   * \param[in]  src_offset    Offset from the base address of the local
   *                           memory region to the start of the local
   *                           source buffer.
   * \param[in]  dst_offset    Offset from the base address of the remote
   *                           memory region to the start of the remote
   *                           destination buffer.
   * \param[in]  cb_ack        Callback to invoke when the source buffer has
   *                           been completely received by the remote node.
   *
   * \see DCMF_Put_register
   *
   * \ingroup PUT
   */

  DCMF_Result DCMF_Put (DCMF_Protocol_t  * registration,
                        DCMF_Request_t   * request,
                        DCMF_Callback_t    cb_done,
                        DCMF_Consistency   consistency,
                        size_t             rank,
                        size_t             bytes,
                        DCMF_Memregion_t * src_memregion,
                        DCMF_Memregion_t * dst_memregion,
                        size_t             src_offset,
                        size_t             dst_offset,
                        DCMF_Callback_t    cb_ack);


  /** ********************************************************************* **/
  /**
   * \brief One-sided point-to-point get
   *
   * This is a description of the get API
   *
   * \defgroup GET Get
   * \ingroup ONESIDED
   */
  /** ********************************************************************* **/

  /**
   * \brief Get protocol implementations.
   * \ingroup GET
   */

  typedef enum
  {
    /**
     * DCMF_Get_register() with the default get protocol is \b guaranteed
     * to return DCMF_SUCCESS. Registration of all other protocols may return
     * DCMF_UNIMPL or another error.
     */
    DCMF_DEFAULT_GET_PROTOCOL,
    DCMF_OVERSEND_GET_PROTOCOL, /**< Uses DCMF_Send - always available. */

    DCMF_USER0_GET_PROTOCOL, /**< Implementation-specific get protocol. */
    DCMF_USER1_GET_PROTOCOL, /**< Implementation-specific get protocol. */
    DCMF_USER2_GET_PROTOCOL, /**< Implementation-specific get protocol. */
    DCMF_USER3_GET_PROTOCOL, /**< Implementation-specific get protocol. */

    DCMF_GET_PROTOCOL_COUNT  /**< Number of get protocols defined. */
  }
  DCMF_Get_Protocol;

  /**
   * \brief Point-to-point get configuration.
   * \ingroup GET
   */

  typedef struct
  {
    DCMF_Get_Protocol protocol;      /**< The get protocol implementation to register. */
    DCMF_Network       network;                  /**< The network fabric to be used for communication. */
  }
  DCMF_Get_Configuration_t;


  /**
   * \brief Register the point-to-point get protocol implementation.
   *
   * \warning After registering the protocol implementation it is illegal to
   *          deallocate the registration object.
   *
   * \param[out] registration  Opaque memory to maintain registration information.
   * \param[in]  configuration Point-to-point get configuration information.
   *
   * \see DCMF_Get
   *
   * \ingroup GET
   */

  DCMF_Result DCMF_Get_register (DCMF_Protocol_t          * registration,
                                 DCMF_Get_Configuration_t * configuration);


  /**
   * \brief Start a point-to-point get operation.
   *
   * The get operation transfers data from the remote node to the local node.
   *
   * To make progress on the get operation the local node must invoke the
   * DCMF_Messager_advance() function until the \c cb_done callback is invoked.
   *
   * \warning Until the message done callback is invoked, it is illegal to
   *          restart the operation, touch the attached buffers, or deallocate
   *          the request object.
   *
   * \param[in]  registration  Protocol registration.
   * \param[in]  request       Opaque memory to maintain internal message state.
   * \param[in]  cb_done       Callback to invoke when message is complete.
   *                           Completion notification will not occur if a
   *                           \c NULL function pointer is specified. \c NULL
   *                           completion callbacks are dangerous and thier use
   *                           is not recommended.
   * \param[in]  consistency   Required consistency level.
   * \param[in]  rank          Remote node rank.
   * \param[in]  bytes         Number of bytes to get from the remote node.
   * \param[in]  src_memregion Memory region on the \b remote node.
   * \param[in]  dst_memregion Memory region on the \b local node.
   * \param[in]  src_offset    Offset from the base address of the remote
   *                           memory region to the start of the remote
   *                           source buffer.
   * \param[in]  dst_offset    Offset from the base address of the local
   *                           memory region to the start of the local
   *                           destination buffer.
   *
   * \see DCMF_Get_register
   * \see DCMF_Messager_advance
   * \see DCMF_Restart
   *
   * \ingroup GET
   */

  DCMF_Result DCMF_Get (DCMF_Protocol_t  * registration,
                        DCMF_Request_t   * request,
                        DCMF_Callback_t    cb_done,
                        DCMF_Consistency   consistency,
                        size_t             rank,
                        size_t             bytes,
                        DCMF_Memregion_t * src_memregion,
                        DCMF_Memregion_t * dst_memregion,
                        size_t             src_offset,
                        size_t             dst_offset);

 /**
   * \brief Restart an existing request.
   *
   * This allows a request to be sent multiple times. The request is reset and
   * posted into the communication send queue.
   *
   * \warning Until the message done callback is invoked, it is illegal to
   *          restart the operation, touch the attached buffers, or deallocate
   *          the request object.
   *
   * \param[in]  request      Opaque memory to maintain internal message state.
   */

  DCMF_Result DCMF_Restart (DCMF_Request_t * request);

  /**
   *  \brief Suspend The Message Layer
   *
   *  This function suspends the message layer.
   *  To resume it, invoke DCMF_Messager_initialize followed by
   *  DCMF_Messager_resume.
   *
   *  \see DCMF_Messager_resume
   *  \see DCMF_Messager_initialize
   */
  DCMF_Result DCMF_Messager_suspend ();

  /**
   *  \brief Resume The Message Layer
   *
   *  This function resumes the message layer after it was suspended
   *  via DCMF_Messager_suspend and re-initialized via 
   *  DCMF_Messager_initialize.
   *
   *  \see DCMF_Messager_suspend
   *  \see DCMF_Messager_initialize
   */
  DCMF_Result DCMF_Messager_resume ();


  /* ********************************************************************* */
  /*                                                                       */
  /*          Examples                                                     */
  /*                                                                       */
  /* ********************************************************************* */

  /**
   * \ example bg1s_put.cc
   * The following is an example implementation a one-sided put operation.
   */

  /**
   * \ example bg1s_get.cc
   * The following is an example implementation a one-sided get operation.
   */

  /**
   * \ example bg1s_accumulate.cc
   * The following is an example implementation a one-sided double complex
   * accumulate operation.
   */

  /**
   * \ example bg1s_rmw.cc
   * The following is an example implementation the one-sided
   * "Read-Modify-Write" operations "swap" and "fetch and add".
   */


  /**
   * \ example waitproc.cc
   * This example demonstrates the use of the non-blocking DCMF_WaitProc()
   * function to implement a \b blocking "wait proc" function.
   */

  /**
   * \ example waitall.cc
   * This example demonstrates the use of the non-blocking DCMF_WaitAll()
   * function to implement a \b blocking "wait" function.
   */


#ifdef __cplusplus
};
#endif

#endif
