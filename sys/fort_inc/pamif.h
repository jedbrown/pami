!
!   \file sys/fort_inc/pamif.h
!   \brief Common external 32-bit FORTRAN interface for IBM's PAMI message layer.
!

external PAMI_ADDR_NULL

!
!   \brief PAMI result status codes.
!
    integer PAMI_SUCCESS,PAMI_NERROR,PAMI_ERROR,PAMI_INVAL,PAMI_UNIMPL
    integer PAMI_EAGAIN,PAMI_ENOMEM,PAMI_SHUTDOWN,PAMI_CHECK_ERRNO
    integer PAMI_OTHER,PAMI_RESULT_EXT
    parameter (PAMI_SUCCESS=0,PAMI_NERROR=-1,PAMI_ERROR=1,PAMI_INVAL=2)
    parameter (PAMI_UNIMPL=3,PAMI_EAGAIN=4,PAMI_ENOMEM=5,PAMI_SHUTDOWN=6)
    parameter (PAMI_CHECK_ERRNO=7,PAMI_OTHER=8,PAMI_RESULT_EXT=1000)

!   typedef void*    pami_client_t;   /**< Client of communication contexts */
!   typedef void*    pami_context_t;  /**< Context for data transfers       */
!   typedef void*    pami_type_t;     /**< \ingroup datatype Description for data layout  */
!   typedef uint32_t pami_task_t;     /**< Identity of a process            */
!   typedef uint32_t pami_endpoint_t; /**< Identity of a context            */

    integer PAMI_CLIENT_T
    integer PAMI_CONTEXT_T
    integer PAMI_TYPE_T
    integer PAMI_TASK_T
    integer PAMI_ENDPOINT_T
    parameter (PAMI_CLIENT_T=4)
    parameter (PAMI_CONTEXT_T=4)
    parameter (PAMI_TYPE_T=4)
    parameter (PAMI_TASK_T=4)
    parameter (PAMI_ENDPOINT_T=4)

!
!   \brief Callback to handle message events
!
!   \param[in] context   PAMI communication context that invoked the callback
!   \param[in] cookie    Event callback application argument
!   \param[in] result    Asynchronous result information
!  
! typedef void (*pami_event_function) ( pami_context_t   context,
!                                       void          *  cookie,
!                                       pami_result_t    result );

!
!   \brief Prototype for function used for context-queued work (post)
!  
!   Posted function will continue to run or stop according to the return value
!   described below.
!  
!   \param[in] context   PAMI communication context running function
!   \param[in] cookie    Application argument
!   \return PAMI_SUCCESS causes function to dequeue (stop running)
!           PAMI_EAGAIN causes function to remain queued and is called on next advance
!           (any other value) causes function to dequeue and (optionally) report error
! 
! typedef pami_result_t (*pami_work_function)(pami_context_t context,void *cookie);
!
! typedef uintptr_t pami_work_t[8];

!
!   \brief Message layer operation types
!
    integer PAMI_UNDEFINED_OP,PAMI_NOOP,PAMI_MAX,PAMI_MIN,PAMI_SUM,PAMI_PROD
    integer PAMI_LXOR,PAMI_BAND,PAMI_BOR,PAMI_BXOR,PAMI_MAXLOC,PAMI_MINLOC
    integer PAMI_USERDEFINED_OP,PAMI_OP_COUNT,PAMI_OP_EXT
    parameter (PAMI_UNDEFINED_OP=0,PAMI_NOOP=1,PAMI_MAX=2,PAMI_MIN=3,PAMI_SUM=4)
    parameter (PAMI_PROD=5,PAMI_LXOR=6,PAMI_BAND=7,PAMI_BOR=8,PAMI_BXOR=9)
    parameter (PAMI_MAXLOC=10,PAMI_MINLOC=11,PAMI_USERDEFINED_OP=12)
    parameter (PAMI_OP_COUNT=13,PAMI_OP_EXT=1000)


!
!   \brief Message layer data types
!
    integer PAMI_UNDEFINED_DT,PAMI_SIGNED_CHAR,PAMI_UNSIGNED_CHAR
    integer PAMI_SIGNED_SHORT,PAMI_UNSIGNED_SHORT,PAMI_SIGNED_INT
    integer PAMI_UNSIGNED_INT,PAMI_SIGNED_LONG_LONG
    integer PAMI_UNSIGNED_LONG_LONG,PAMI_FLOAT,PAMI_DOUBLE
    integer PAMI_LONG_DOUBLE,PAMI_LOGICAL,PAMI_SINGLE_COMPLEX
    integer PAMI_DOUBLE_COMPLEX,PAMI_LOC_2INT,PAMI_LOC_SHORT_INT
    integer PAMI_LOC_FLOAT_INT,PAMI_LOC_DOUBLE_INT,PAMI_LOC_2FLOAT
    integer PAMI_LOC_2DOUBLE,PAMI_USERDEFINED_DT
    integer PAMI_DT_COUNT,PAMI_DT_EXT
    parameter (PAMI_UNDEFINED_DT=0,PAMI_SIGNED_CHAR=1,PAMI_UNSIGNED_CHAR=2)
    parameter (PAMI_SIGNED_SHORT=3,PAMI_UNSIGNED_SHORT=4,PAMI_SIGNED_INT=5)
    parameter (PAMI_UNSIGNED_INT=6,PAMI_SIGNED_LONG_LONG=7)
    parameter (PAMI_UNSIGNED_LONG_LONG=8,PAMI_FLOAT=9,PAMI_DOUBLE=10)
    parameter (PAMI_LONG_DOUBLE=11,PAMI_LOGICAL=12,PAMI_SINGLE_COMPLEX=13)
    parameter (PAMI_DOUBLE_COMPLEX=14,PAMI_LOC_2INT=15,PAMI_LOC_SHORT_INT=16)
    parameter (PAMI_LOC_FLOAT_INT=17,PAMI_LOC_DOUBLE_INT=18,PAMI_LOC_2FLOAT=19)
    parameter (PAMI_LOC_2DOUBLE=20,PAMI_USERDEFINED_DT=21)
    parameter (PAMI_DT_COUNT=22,PAMI_DT_EXT=1000)


!
!   \brief Query the data type
!  
!   \param[in]  dt   PAMI reduction data type
!   \param[out] size The size of the data operand
!  
!   \retval PAMI_SUCCESS  The query was successful
!   \retval PAMI_INVAL    The query was rejected due to invalid parameters.
! 
! pami_result_t PAMI_Dt_query (pami_dt dt,size_t *size);


! /*****************************************************************************/
! /**
!  * \defgroup configuration pami configuration interface
!  *
!  * Some brief documentation on configuration stuff ...
!  * \{
!  */
! /*****************************************************************************/

! typedef char* pami_user_key_t;   /**< Char string as configuration key */
! typedef char* pami_user_value_t; /**< Char string as configuration value */

    type pami_user_config_t
        sequence
        integer(4)  :: key      ! /**< The configuration key   */
        integer(4)  :: value    ! /**< The configuration value */
    end type pami_user_config_t

! /**
!  * This enum contains ALL possible attributes for all hardware
!  */

!   /* PAMI attribute name                   Query / Update */
    integer PAMI_CLIENT_CLOCK_MHZ           !/**< Q : size_t : Frequency of the CORE clock,in units of 10^6/seconds.  This can be used to approximate the performance of the current task. */
    integer PAMI_CLIENT_CONST_CONTEXTS      !/**< Q : size_t : All processes will return the same PAMI_CLIENT_NUM_CONTEXTS */
    integer PAMI_CLIENT_HWTHREADS_AVAILABLE !/**< Q : size_t : The number of HW threads available to a process without over-subscribing (at least 1) */
    integer PAMI_CLIENT_MEMREGION_SIZE      !/**< Q : size_t : Size of the pami_memregion_t handle in this implementation,in units of Bytes. */
    integer PAMI_CLIENT_MEM_SIZE            !/**< Q : size_t : Size of the core main memory,in units of 1024^2 Bytes    */
    integer PAMI_CLIENT_NUM_TASKS           !/**< Q : size_t : Total number of tasks        */
    integer PAMI_CLIENT_NUM_CONTEXTS        !/**< Q : size_t : The maximum number of contexts allowed on this process */
    integer PAMI_CLIENT_PROCESSOR_NAME      !/**< Q : char[] : A unique name string for the calling process,and should be suitable for use by MPI_Get_processor_name(). The storage should *not* be freed by the caller. */
    integer PAMI_CLIENT_TASK_ID             !/**< Q : size_t : ID of this task (AKA "rank") */
    integer PAMI_CLIENT_WTIMEBASE_MHZ       !/**< Q : size_t : Frequency of the WTIMEBASE clock,in units of 10^6/seconds.  This can be used to convert from PAMI_Wtimebase to PAMI_Timer manually. */
    integer PAMI_CLIENT_WTICK               !/**< Q : double : This has the same definition as MPI_Wtick(). */
    integer PAMI_CONTEXT_DISPATCH_ID_MAX    !/**< Q : size_t : Maximum allowed dispatch id,see PAMI_Dispatch_set() */
    integer PAMI_DISPATCH_RECV_IMMEDIATE_MAX!/**< Q : size_t : Maximum number of bytes that can be received,and provided to the application,in a dispatch function. */
    integer PAMI_DISPATCH_SEND_IMMEDIATE_MAX!/**< Q : size_t : Maximum number of bytes that can be transfered with the PAMI_Send_immediate() function. */
    integer PAMI_DISPATCH_ATOM_SIZE_MAX     !/**< Q : size_t : Maximum atom size that can be used with PAMI_Send_typed() function. */
    integer PAMI_GEOMETRY_OPTIMIZE          !/**< Q : bool : Populate the geometry algorithms list with hardware optimized algorithms.  If the algorithms list is not optimized,point-to-point routines only will be present,and hardware resources will be released */
    integer PAMI_TYPE_DATA_SIZE             !/**< Q : size_t : The data size of a type */
    integer PAMI_TYPE_DATA_EXTENT           !/**< Q : size_t : The data extent of a type */
    integer PAMI_TYPE_ATOM_SIZE             !/**< Q : size_t : The atom size of a type */
    integer PAMI_ATTRIBUTE_NAME_EXT         !/**< Begin extension-specific values */
    parameter (PAMI_CLIENT_CLOCK_MHZ=100)
    parameter (PAMI_CLIENT_CONST_CONTEXTS=101)
    parameter (PAMI_CLIENT_HWTHREADS_AVAILABLE=102)
    parameter (PAMI_CLIENT_MEMREGION_SIZE=103)
    parameter (PAMI_CLIENT_MEM_SIZE=104)
    parameter (PAMI_CLIENT_NUM_TASKS=105)
    parameter (PAMI_CLIENT_NUM_CONTEXTS=106)
    parameter (PAMI_CLIENT_PROCESSOR_NAME=107)
    parameter (PAMI_CLIENT_TASK_ID=108)
    parameter (PAMI_CLIENT_WTIMEBASE_MHZ=109)
    parameter (PAMI_CLIENT_WTICK=110)
    parameter (PAMI_CONTEXT_DISPATCH_ID_MAX = 200)
    parameter (PAMI_DISPATCH_RECV_IMMEDIATE_MAX = 300)
    parameter (PAMI_DISPATCH_SEND_IMMEDIATE_MAX=301)
    parameter (PAMI_DISPATCH_ATOM_SIZE_MAX=302)
    parameter (PAMI_GEOMETRY_OPTIMIZE=400)
    parameter (PAMI_TYPE_DATA_SIZE=500)
    parameter (PAMI_TYPE_DATA_EXTENT=501)
    parameter (PAMI_TYPE_ATOM_SIZE=502)
    parameter (PAMI_ATTRIBUTE_NAME_EXT=1000)

!  typedef union
!  {
!    size_t      intval;
!    double      doubleval;
!    const char* chararray;
!  } pami_attribute_value_t;

    integer PAMI_EXT_ATTR           !/**< starting value for extended attributes */
    parameter (PAMI_EXT_ATTR=1000)

! /** \} */ /* end of "configuration" group */

! /**
!  * \brief General purpose configuration structure.
!  */
    type pami_configuration_t
        sequence
        integer(4)  :: name     ! /**< Attribute type */
        integer(4)  :: value(2) ! /**< Attribute value */
    end type pami_configuration_t

! /*****************************************************************************/
! /**
!  * \defgroup PAMI Collectives Metadata
!  * \{
!  */
! /*****************************************************************************/

! /**
!  *  This enum contains the "reason" codes that indicate why a particular collective
!  *  operation could fail.  Upon querying the metadata of a particular collective
!  *  the user may have a function pointer populated in the "check_fn" field of the
!  *  metadata struct.  The user can pass in the call site parameters (the pami_xfer_t)
!  *  of the collective into this function and receive a list of failures.
!  */
    type metadata_result_t
        sequence
        integer(4)  :: align_send_buffer        ! /*  Send buffer must be aligned */
        integer(4)  :: align_send_recv_buffer   ! /*  Receive buffer must be aligned */
        integer(4)  :: datatype                 ! /*  Datatype not valid for this op */
        integer(4)  :: op                       ! /*  Operation not valid for this op */
        integer(4)  :: contiguous_send          ! /*  Send data must be contiguous */
        integer(4)  :: contiguous_recv          ! /*  Receive data must be contigous */
        integer(4)  :: continuous_send          ! /*  Send data must be continuous */
        integer(4)  :: continuous_recv          ! /*  Receive data must be continuous */
    end type metadata_result_t
  
! /*  Forward declaration of pami_xfer_t */
! typedef struct pami_xfer_t *xfer_p;
  
! /**
!  * \brief Function signature for metadata queries
!  *
!  * \param[in] in A "call-site"   collecitive query parameters
!  * \retval    pami_metata_check  failure code,0=success,nonzero
!  *                               bits are OR'd from pami_metadata_check_t
!  */
! typedef metadata_result_t (*pami_metadata_function) (struct pami_xfer_t *in);

! /**
!  * \brief A metadata structure to describe a collective protocol
!  */
    type pami_metadata_t
        sequence
        CHARACTER   :: name(32)     ! /**<  A character string describing the collective */
        integer(4)  :: version      ! /**<  A version number of the collective */
        ! /* Correctness Parameters */
        integer(4)  :: check_fn     ! /**<  A function pointer to validate parameters for the collective operation.  Can be NULL if no correctness check is required */
        integer(4)  :: range_lo     ! /**<  This protocol has minumum bytes to send/recv requirements */
        integer(4)  :: range_hi     ! /**<  This protocol has maximum bytes to send/recv requirements */
        integer(4)  :: mustquery    ! /**<  A flag indicating whether or not the user MUST query the metadata or not.  0:query is not necessary.  >0: the user must call the check_fn to determine if the call site parameters are correct. */
        integer(4)  :: nonlocal     ! /**<  The protocol associated with this metadata requires "nonlocal" knowledge,meaning that the result code from the check_fn must be allreduced to determine if the operation will work 0:no,>0:  requires nonlocal data */
        integer(4)  :: sendminalign ! /**<  This protocol requires a minimum address alignment of sendminalign bytes */
        integer(4)  :: recvminalign ! /**<  This protocol requires a minimum address alignment of recvminalign bytes */
        integer(4)  :: alldt        ! /**<  This protocol works for all datatypes for reduction. 0:not for all dt 1:for all dt */
        integer(4)  :: allop        ! /**<  This protocol works for all operations for reduction. 0: not for all op,1:for all op */
        integer(4)  :: contigsflags ! /**<  This protocol requires contiguous data(send) contiguous:  data type must be PAMI_BYTE */
        integer(4)  :: contigrflags ! /**<  This protocol requires contiguous data(recv) contiguous:  data type must be PAMI_BYTE */
        integer(4)  :: continsflags ! /**<  This protocol requires continuous data(send) continuous:  data type must be PAMI_BYTE and for vector collectives,the target buffers of the vectors must be adjacent in memory */
        integer(4)  :: continrflags ! /**<  This protocol requires continuous data(recv) continuous:  data type must be PAMI_BYTE and for vector collectives,the target buffers of the vectors must be adjacent in memory */
        ! /* Performance Parameters */
        integer(4)  :: hw_accel     ! /**<  This collective is using special purpose hardware to accelerate the collective 0:  no 1:  yes */
        integer(4)  :: range_lo_perf! /**<  Estimated performance range in bytes(low) */
        integer(4)  :: range_hi_perf! /**<  Estimated performance range in bytes(high) */
    end type pami_metadata_t
! /** \} */ /* end of "PAMI Collectives Metadata" group */


! /*****************************************************************************/
! /**
!  * \defgroup activemessage pami active messaging interface
!  *
!  * Some brief documentation on active message stuff ...
!  * \{
!  */
! /*****************************************************************************/

! /**
!  * \brief Options for bi-state dispatch hints
!  */
    integer PAMI_HINT2_OFF      !/**< This turns the option off. */
    integer PAMI_HINT2_ON       !/**< This turns the option on. */
    parameter (PAMI_HINT2_OFF=0)
    parameter (PAMI_HINT2_ON=1)

! /**
!  * \brief Options for tri-state dispatch hints
!  *
!  * Several dispatch hints accept have a yes/no/maybe sort of
!  * approach.
!  */
    integer PAMI_HINT3_DEFAULT      !/**< This hint leaves the option up to the PAMI implementation to choose. */
    integer PAMI_HINT3_FORCE_ON     !/**< This allows the user to force an option to be used. */
    integer PAMI_HINT3_FORCE_OFF    !/**< The user can force the implementation to not use this option. */
    parameter (PAMI_HINT3_DEFAULT=0)
    parameter (PAMI_HINT3_FORCE_ON=1)
    parameter (PAMI_HINT3_FORCE_OFF=2)

! /**
!  * \brief Hints for sending a message
!  *
!  * \todo better names for the hints
!  * \todo better documentation for the hints
!  */
    type pami_send_hint_t
        sequence
        integer(4)  :: buffer_registered ! /**< Send and receive buffers are ready for RDMA operations */
        integer(4)  :: consistency       ! /**< Force match ordering semantics */
        integer(4)  :: interrupt_on_recv ! /**< Interrupt the remote task when the first packet arrives */
        integer(4)  :: no_local_copy     ! /**< Disable PAMI making a local copy of data */
        integer(4)  :: no_long_header    ! /**< Disable long header support */
        integer(4)  :: recv_immediate    ! /**< Assert that sends will result in an 'immediate' receive */
        ! /* The following hints use the PAMI_HINT3_* values */
        integer(4)  :: use_rdma          ! /**< Enable/Disable rdma operations */
        integer(4)  :: use_shmem         ! /**< Enable/Disable shared memory optimizations */
        integer(4)  :: multicontext      ! /**< Parallelize communication across multiple contexts */
    end type pami_send_hint_t

    type pami_collective_hint_t
        sequence
        integer(4)  :: multicontext      ! /**< Parallelize communication across multiple contexts */
    end type pami_collective_hint_t

! /**
!  * \brief Active message send common parameters structure
!  */
    type pami_send_immediate_t
        sequence
        integer(4)  :: header(2)    ! /**< Header buffer address and size in bytes */
        integer(4)  :: data(2)      ! /**< Data buffer address and size in bytes */
        integer(4)  :: dispatch     ! /**< Dispatch identifier */
        integer(4)  :: hints(9)     ! /**< Hints for sending the message */
        integer(4)  :: dest         ! /**< Destination endpoint */
    end type pami_send_immediate_t

! /**
!  * \brief Structure for event notification
!  */
    type pami_send_event_t
        sequence
        integer(4)  :: cookie       ! /**< Argument to \b all event callbacks */
        integer(4)  :: local_fn     ! /**< Local message completion event */
        integer(4)  :: remote_fn    ! /**< Remote message completion event */
    end type pami_send_event_t

! /**
!  * \brief Structure for send parameters unique to a simple active message send
!  */
    type pami_send_t
        sequence
        integer(4)  :: send(15)     ! /**< Common send parameters */
        integer(4)  :: events(3)    ! /**< Non-blocking event parameters */
    end type pami_send_t

! /**
!  * \brief Function to produce data at send side or consume data at receive side
!  *
!  * The default operation with a typed transfer is to copy data. A data
!  * function,when associated with a type,allows a different way of handling
!  * data. For example,one can write a data function to perform data reduction
!  * instead of data copy.
!  *
!  * Unlike memory copy,a data function may not be able to handle byte data.
!  * Atom size is defined as the minimum unit size that a data function
!  * can accepts. For example,a data function for summing up doubles may
!  * require the input to be an integral number of doubles,thus the atom
!  * size for this data function is sizeof(double).
!  *
!  * When a data function is used with a typed transfer,one must make sure
!  * the atom size of the data function divides the atom size of the type.
!  *
!  * \param [in] target  The address of a contiguous target buffer
!  * \param [in] source  The address of a contiguous source buffer
!  * \param [in] bytes   The number of bytes to handle
!  * \param [in] cookie  A user-specified value
!  *
!  */
!  typedef void (*pami_data_function) (void   * target,
!                                      void   * source,
!                                      size_t   bytes,
!                                      void   * cookie);

! /**
!  * \brief Data function that performs data copy
!  *
!  */
!  extern pami_data_function PAMI_DATA_COPY;

! /**
!  * \brief Structure for send parameters of a typed active message send
!  *
!  * \c data_fn takes the same cookie for events
!  */
    type pami_send_typed_t
        sequence
        integer(4)  :: send(15)     ! /**< Common send parameters */ 
        integer(4)  :: events(3)    ! /**< Non-blocking event parameters */
        integer(4)  :: type         ! /**< Datatype */
        integer(4)  :: offset       ! /**< Starting offset in \c datatype */
        integer(4)  :: data_fn      ! /**< Function to produce data */
    end type pami_send_typed_t

! /**
!  * \brief Non-blocking active message send for contiguous data
!  *
!  * A low-latency send operation may be enhanced by using a dispatch id which
!  * was set with the \c recv_immediate hint bit enabled. This hint asserts
!  * that all receives with the dispatch id will not exceed a certain limit.
!  *
!  * The implementation configuration attribute \c PAMI_DISPATCH_RECV_IMMEDIATE_MAX
!  * defines the maximum size of data buffers that can be completely received
!  * with a single dispatch callback. Typically this limit is associated with
!  * a network resource attribute,such as a packet size.
!  *
!  * \see pami_send_hint_t
!  * \see PAMI_Dispatch_query
!  *
!  * \param[in] context    PAMI communication context
!  * \param[in] parameters Send simple parameter structure
!  *
!  * \retval PAMI_SUCCESS  The request has been accepted.
!  * \retval PAMI_INVAL    The request has been rejected due to invalid parameters.
!  */
!  pami_result_t PAMI_Send (pami_context_t context, pami_send_t * parameters);

! /**
!  * \brief Immediate active message send for small contiguous data
!  *
!  * The blocking send is only valid for small data buffers. The implementation
!  * configuration attribute \c PAMI_DISPATCH_SEND_IMMEDIATE_MAX defines the upper
!  * bounds for the size of data buffers,including header data,that can be
!  * sent with this function. This function will return an error if a data
!  * buffer larger than the \c PAMI_DISPATCH_SEND_IMMEDIATE_MAX is attempted.
!  *
!  * This function provides a low-latency send that can be optimized by the
!  * specific pami implementation. If network resources are immediately
!  * available the send data will be injected directly into the network. If
!  * resources are not available the specific pami implementation may internally
!  * buffer the send parameters and data until network resource are available
!  * to complete the transfer. In either case the send will immediately return,
!  * no doce callback is invoked,and is considered complete.
!  *
!  * The low-latency send operation may be further enhanced by using a dispatch
!  * id which was set with the \c recv_immediate hint bit enabled. This hint
!  * asserts that all receives with the dispatch id will not exceed a certain
!  * limit.
!  *
!  * The implementation configuration attribute \c PAMI_DISPATCH_RECV_IMMEDIATE_MAX
!  * defines the maximum size of data buffers that can be completely received
!  * with a single dispatch callback. Typically this limit is associated with
!  * a network resource attribute,such as a packet size.
!  *
!  * \see pami_send_hint_t
!  * \see PAMI_Dispatch_query
!  *
!  * \todo Better define send parameter structure so done callback is not required
!  * \todo Define configuration attribute for the size limit
!  *
!  * \param[in] context    PAMI communication context
!  * \param[in] parameters Send parameter structure
!  *
!  * \retval PAMI_SUCCESS  The request has been accepted.
!  * \retval PAMI_INVAL    The request has been rejected due to invalid parameters.
!  */
!  pami_result_t PAMI_Send_immediate (pami_context_t context, pami_send_immediate_t * parameters);

! /**
!  * \brief Non-blocking active message send for non-contiguous typed data
!  *
!  * Transfers data according to a predefined data memory layout,or type,to
!  * the remote task.
!  *
!  * Conceptually,the data is transfered as a byte stream which may be
!  * received by the remote task into a different format,such as a contiguous
!  * buffer or the same or different predefined type.
!  *
!  * \param[in] context    PAMI communication context
!  * \param[in] parameters Send typed parameter structure
!  *
!  * \retval PAMI_SUCCESS  The request has been accepted.
!  * \retval PAMI_INVAL    The request has been rejected due to invalid parameters.
!  */
!  pami_result_t PAMI_Send_typed (pami_context_t context, pami_send_typed_t * parameters);

! /**
!  * \brief Active message receive hints
!  */
    type pami_recv_hint_t
        sequence
        integer(4)  :: inline_completion! /**< The receive completion callback \b must be invoked by the thread that receives the dispatch notification. */
        integer(4)  :: reserved         ! /**< Reserved. Do not use. */
    end type pami_recv_hint_t

! /**
!  * \brief Receive message structure
!  *
!  * This structure is initialized and then returned as an output parameter from
!  * the active message dispatch callback to direct the pami runtime how to
!  * receive the data stream.
!  *
!  * When \c type is \c PAMI_BYTE,the receive buffer is contiguous and it
!  * must be large enough to hold the entire message.
!  *
!  * With non-contiguous \c type,the receive buffer in general must be large
!  * enough for the incoming message as well but \c type can be constructed
!  * in such a way that unwanted portions of the incoming are disposed into
!  * a circular junk buffer.
!  *
!  * \see pami_dispatch_p2p_function
!  */
    type pami_recv_t
        sequence
        integer(4)  :: hints(2)     ! /**< Hints for receiving the message */
        integer(4)  :: cookie       ! /**< Argument to \b all event callbacks */
        integer(4)  :: local_fn     ! /**< Local message completion event */
        integer(4)  :: addr         ! /**< Starting address of the buffer */
        integer(4)  :: type         ! /**< Datatype */
        integer(4)  :: offset       ! /**< Starting offset of the type */
        integer(4)  :: data_fn      ! /**< Function to consume data */
    end type pami_recv_t

! /**
!  * \brief Dispatch callback
!  *
!  * This single dispatch function type supports two kinds of receives:
!  * "immediate" and "asynchronous".
!  *
!  * An immediate receive occurs when the dispatch function is invoked and all
!  * of the data sent is \em immediately available in the buffer. In this case
!  * \c pipe_addr will point to a valid memory location - even when the number
!  * of bytes sent is zero,and the \c recv output structure will be \c NULL.
!  *
!  * An asynchronous receive occurs when the dispatch function is invoked and
!  * all of the data sent is \b not immediately available. In this case the
!  * application must provide information to direct how the receive will
!  * complete. The \c recv output structure will point to a valid memory
!  * location for this purpose,and the \c pipe_addr pointer will be \c NULL.
!  * The \c data_size parameter will contain the number of bytes that are being
!  * sent from the remote endpoint.
!  *
!  * \note A zero-byte send will \b always result in an immediate receive.
!  *
!  * \note A zero-byte receive does not require any changes to \c recv in the dispatch fn
!  *
!  * \note The maximum number of bytes that may be immediately received can be
!  *       queried with the \c PAMI_RECV_IMMEDIATE configuration attribute.
!  *
!  * \see PAMI_Dispatch_query
!  *
!  * "pipe" has nothing to do with "PipeWorkQueue"s
!  */
!  typedef void (*pami_dispatch_p2p_function) (
!       pami_context_t    context,     /**< IN:  communication context which invoked the dispatch function */
!       void            * cookie,      /**< IN:  dispatch cookie */
!       const void      * header_addr, /**< IN:  header address  */
!       size_t            header_size, /**< IN:  header size     */
!       const void      * pipe_addr,   /**< IN:  address of PAMI pipe  buffer,valid only if non-NULL        */
!       size_t            data_size,   /**< IN:  number of byts of message data,valid regardless of message type */
!       pami_endpoint_t   origin,      /**< IN:  Endpoint that originated the transfer */
!       pami_recv_t     * recv);        /**< OUT: receive message structure,only needed if addr is non-NULL */

! /** \} */ /* end of "active message" group */


! /*****************************************************************************/
! /**
!  * \defgroup rma pami remote memory access data transfer interface
!  *
!  * Some brief documentation on rma stuff ...
!  * \{
!  */
! /*****************************************************************************/

! /**
!  * \brief Input parameters common to all rma transfers
!  **/
    type pami_rma_t
        sequence
        integer(4)  :: dest         ! /**< Destination endpoint */
        integer(4)  :: hints(9)     ! /**< Hints for sending the message */
        integer(4)  :: bytes        ! /**< Data transfer size in bytes */
        integer(4)  :: cookie       ! /**< Argument to \b all event callbacks */
        integer(4)  :: done_fn      ! /**< Local completion event */ 
    end type pami_rma_t

! /**
!  * \brief Input parameters for rma simple transfers
!  **/
    type pami_rma_addr_t
        sequence
        integer(4)  :: local    ! /**< Local transfer virtual address */
        integer(4)  :: remote   ! /**< Remote transfer virtual address */
    end type pami_rma_addr_t

! /**
!  * \brief Input parameters for rma typed transfers
!  */
    type pami_rma_typed_t
        sequence
        integer(4)  :: local    ! /**< Data type of local buffer */
        integer(4)  :: remote   ! /**< Data type of remote buffer */
    end type pami_rma_typed_t

! /*****************************************************************************/
! /**
!  * \defgroup put pami remote memory access put transfer interface
!  *
!  * Some brief documentation on put stuff ...
!  * \{
!  */
! /*****************************************************************************/

! /**
!  * \brief Input parameters for rma put transfers
!  */
    type pami_rma_put_t
        sequence
        integer(4)  :: rdone_fn     ! /**< Remote completion event - all local data has been received by remote task */
    end type pami_rma_put_t

! /**
!  * \brief Input parameters for simple put transfers
!  * \code
!  * pami_put_simple_t parameters;
!  * parameters.rma.dest     = ;
!  * parameters.rma.hints    = ;
!  * parameters.rma.bytes    = ;
!  * parameters.rma.cookie   = ;
!  * parameters.rma.done_fn  = ;
!  * parameters.addr.local   = ;
!  * parameters.addr.remote  = ;
!  * parameters.put.rdone_fn = ;
!  * \endcode
!  */
    type pami_put_simple_t
        sequence
        integer(4)  :: rma(13)  ! /**< Common rma parameters */
        integer(4)  :: addr(2)  ! /**< Simple rma address parameters */
        integer(4)  :: put      ! /**< Common put parameters */
    end type pami_put_simple_t

! /**
!  * \brief One-sided put operation for simple contiguous data transfer
!  *
!  * \param[in] context    PAMI communication context
!  * \param[in] parameters Simple put input parameters
!  *
!  * \retval PAMI_SUCCESS  The request has been accepted.
!  * \retval PAMI_INVAL    The request has been rejected due to invalid parameters.
!  */
!  pami_result_t PAMI_Put (pami_context_t context, pami_put_simple_t * parameters);

! /**
!  * \brief Input parameters for simple typed put transfers
!  * \code
!  * pami_put_typed_t parameters;
!  * parameters.rma.dest     = ;
!  * parameters.rma.hints    = ;
!  * parameters.rma.bytes    = ;
!  * parameters.rma.cookie   = ;
!  * parameters.rma.done_fn  = ;
!  * parameters.addr.local   = ;
!  * parameters.addr.remote  = ;
!  * parameters.type.local   = ;
!  * parameters.type.remote  = ;
!  * parameters.put.rdone_fn = ;
!  * \endcode
!  */
    type pami_put_typed_t
        sequence
        integer(4)  :: rma(13)  ! /**< Common rma parameters */
        integer(4)  :: addr(2)  ! /**< Simple rma address parameters */
        integer(4)  :: type(2)  ! /**< Simple rma typed parameters */
        integer(4)  :: put      ! /**< Common put parameters */
    end type pami_put_typed_t

! /**
!  * \brief One-sided put operation for typed non-contiguous data transfer
!  *
!  * \param[in] context    PAMI communication context
!  * \param[in] parameters Typed put input parameters
!  *
!  * \retval PAMI_SUCCESS  The request has been accepted.
!  * \retval PAMI_INVAL    The request has been rejected due to invalid parameters.
!  */
!  pami_result_t PAMI_Put_typed (pami_context_t context, pami_put_typed_t * parameters);


! /** \} */ /* end of "put" group */

! /*****************************************************************************/
! /**
!  * \defgroup get pami remote memory access get transfer interface
!  *
!  * Some brief documentation on get stuff ...
!  * \{
!  */
! /*****************************************************************************/

! /**
!  * \brief Input parameter structure for simple get transfers
!  * \code
!  * pami_get_simple_t parameters;
!  * parameters.rma.dest    = ;
!  * parameters.rma.hints   = ;
!  * parameters.rma.bytes   = ;
!  * parameters.rma.cookie  = ;
!  * parameters.rma.done_fn = ;
!  * parameters.addr.local  = ;
!  * parameters.addr.remote = ;
!  * \endcode
!  */
    type pami_get_simple_t
        sequence
        integer(4)  :: rma(13)  ! /**< Common rma parameters */
        integer(4)  :: addr(2)  ! /**< Simple rma address parameters */
    end type pami_get_simple_t

! /**
!  * \brief One-sided get operation for simple contiguous data transfer
!  *
!  * \param[in] context    PAMI communication context
!  * \param[in] parameters Simple get input parameters
!  *
!  * \retval PAMI_SUCCESS  The request has been accepted.
!  * \retval PAMI_INVAL    The request has been rejected due to invalid parameters.
!  */
!  pami_result_t PAMI_Get (pami_context_t context, pami_get_simple_t * parameters);

! /**
!  * \brief Input parameter structure for typed get transfers
!  * \code
!  * pami_get_typed_t parameters;
!  * parameters.rma.dest    = ;
!  * parameters.rma.hints   = ;
!  * parameters.rma.bytes   = ;
!  * parameters.rma.cookie  = ;
!  * parameters.rma.done_fn = ;
!  * parameters.addr.local  = ;
!  * parameters.addr.remote = ;
!  * parameters.type.local  = ;
!  * parameters.type.remote = ;
!  * \endcode
!  */
    type pami_get_typed_t
        sequence
        integer(4)  :: rma(13)  ! /**< Common rma parameters */
        integer(4)  :: addr(2)  ! /**< Simple rma address parameters */
        integer(4)  :: type(2)  ! /**< Simple rma typed parameters */
    end type pami_get_typed_t

! /**
!  * \brief One-sided get operation for typed non-contiguous data transfer
!  *
!  * \param[in] context    PAMI communication context
!  * \param[in] parameters Typed get input parameters
!  *
!  * \retval PAMI_SUCCESS  The request has been accepted.
!  * \retval PAMI_INVAL    The request has been rejected due to invalid parameters.
!  */
!  pami_result_t PAMI_Get_typed (pami_context_t context, pami_get_typed_t * parameters);

! /** \} */ /* end of "get" group */

! /*****************************************************************************/
! /**
!  * \defgroup rmw pami remote memory access read-modify-write interface
!  *
!  * Some brief documentation on rmw stuff ...
!  * \{
!  */
! /*****************************************************************************/

! /** \brief Atomic rmw data type */
    integer PAMI_RMW_KIND_UINT32        ! /**< 32-bit unsigned integer operation */
    integer PAMI_RMW_KIND_UINT64        ! /**< 64-bit unsigned integer operation */
    parameter (PAMI_RMW_KIND_UINT32=1)
    parameter (PAMI_RMW_KIND_UINT64=2)

! /** \brief Atomic rmw assignment type */
    integer PAMI_RMW_ASSIGNMENT_SET     ! /**< =  operation */
    integer PAMI_RMW_ASSIGNMENT_ADD     ! /**< += operation */
    integer PAMI_RMW_ASSIGNMENT_OR      ! /**< |= operation */
    integer PAMI_RMW_ASSIGNMENT_AND     ! /**< &= operation */
    parameter (PAMI_RMW_ASSIGNMENT_SET=16)
    parameter (PAMI_RMW_ASSIGNMENT_ADD=32)
    parameter (PAMI_RMW_ASSIGNMENT_OR=64)
    parameter (PAMI_RMW_ASSIGNMENT_AND=128)

! /** \brief Atomic rmw comparison type */
    integer PAMI_RMW_COMPARISON_NOOP    ! /**< No comparison operation */
    integer PAMI_RMW_COMPARISON_EQUAL   ! /**< Equality comparison operation */
    parameter (PAMI_RMW_COMPARISON_NOOP=256)
    parameter (PAMI_RMW_COMPARISON_EQUAL=512)

    type pami_rmw_t
        sequence
        integer(4)  :: rma(13)      ! /**< Common rma parameters */
        integer(4)  :: addr(2)      ! /**< Rma addresses parameters */
        integer(4)  :: compare      ! /**< read-modify-write comparison type */
        integer(4)  :: assign       ! /**< read-modify-write assignment type */
        integer(4)  :: kind         ! /**< read-modify-write variable type */
        integer(4)  :: value        ! /**< 32-bit data value */
        integer(4)  :: test         ! /**< 32-bit test value */
    end type pami_rmw_t

! /**
!  * \brief Atomic read-modify-write operation to a remote memory location
!  *
!  * The specific operation is determined by the combination of the three rmw
!  * identifier enumeration types. All operations will perform the same
!  * generic logical atomic operation:
!  *
!  * \code
!  * *local = *remote; (*remote COMPARISON test) ? *remote ASSIGNMENT value;
!  * \endcode
!  *
!  * \warning All read-modify-write operations are \b unordered relative
!  *          to all other data transfer operations - including other
!  *          read-modify-write operations.
!  *
!  * Example read-modify-write operations include:
!  *
!  * \par PAMI_RMW_KIND_UINT32 | PAMI_RMW_COMPARISON_NOOP | PAMI_RMW_ASSIGNMENT_ADD
!  *      "32-bit unsigned integer fetch-and-add operation"
!  * \code
!  * uint32_t *local,*remote,value,test;
!  * *local = *remote; *remote += value;
!  * \endcode
!  *
!  * \par PAMI_RMW_KIND_UINT32 | PAMI_RMW_COMPARISON_NOOP | PAMI_RMW_ASSIGNMENT_OR
!  *      "32-bit unsigned integer fetch-and-or operation"
!  * \code
!  * uint32_t *local,*remote,value,test;
!  * *local = *remote; *remote |= value;
!  * \endcode
!  *
!  * \par PAMI_RMW_KIND_UINT32 | PAMI_RMW_COMPARISON_NOOP | PAMI_RMW_ASSIGNMENT_AND
!  *      "32-bit unsigned integer fetch-and-and operation"
!  * \code
!  * uint32_t *local,*remote,value,test;
!  * *local = *remote; *remote &= value;
!  * \endcode
!  *
!  * \par PAMI_RMW_KIND_UINT64 | PAMI_RMW_COMPARISON_NOOP | PAMI_RMW_ASSIGNMENT_SET
!  *      "64-bit unsigned integer swap (fetch-and-set) operation"
!  * \code
!  * uint64_t *local,*remote,value,test;
!  * *local = *remote; *remote = value;
!  * \endcode
!  *
!  * \par PAMI_RMW_KIND_UINT64 | PAMI_RMW_COMPARISON_EQUAL | PAMI_RMW_ASSIGNMENT_SET
!  *      "64-bit unsigned integer compare-and-swap operation"
!  * \code
!  * uint64_t *local,*remote,value,test;
!  * *local = *remote; (*remote == test) ? *remote = value;
!  * \endcode
!  *
!  * \param[in] context    PAMI communication context
!  * \param[in] parameters read-modify-write input parameters
!  *
!  * \retval PAMI_SUCCESS  The request has been accepted.
!  * \retval PAMI_INVAL    The request has been rejected due to invalid parameters.
!  */
!  pami_result_t PAMI_Rmw (pami_context_t context, pami_rmw_t * parameters);

! /** \} */ /* end of "rmw" group */

! /*****************************************************************************/
! /**
!  * \defgroup rdma pami remote memory access rdma interfaces
!  *
!  * Some brief documentation on rdma stuff ...
!  * \{
!  */
! /*****************************************************************************/

! /**
!  * \brief Memory region opaque object
!  *
!  * A memory region must be created before it may be used. The opaque
!  * object may be directly transfered to a remote endpoint using the
!  * PAMI_Send() or PAMI_Send_immediate() interface.
!  *
!  * \see PAMI_Memregion_create
!  * \see PAMI_Memregion_destroy
!  */
! typedef uint8_t pami_memregion_t[PAMI_CLIENT_MEMREGION_SIZE_STATIC];

! /**
!  * \brief Create a local memory region for one sided operations
!  *
!  * The local memregion may be transfered,via a send message,to a remote task
!  * to allow the remote task to perform one-sided operations with this local
!  * task
!  *
!  * \param[in]  context   PAMI application context
!  * \param[in]  address   Base virtual address of the memory region
!  * \param[in]  bytes_in  Number of bytes requested
!  * \param[out] bytes_out Number of bytes granted
!  * \param[out] memregion Memory region object to initialize
!  *
!  * \retval PAMI_SUCCESS The entire memory region,or a portion of
!  *                      the memory region was pinned. The actual
!  *                      number of bytes pinned from the start of the
!  *                      buffer is returned in the \c bytes_out
!  *                      parameter. The memory region must be free'd with
!  *                      with PAMI_Memregion_destroy().
!  *
!  * \retval PAMI_EAGAIN  The memory region was not pinned due to an
!  *                      unavailable resource. The memory region does not
!  *                      need to be free'd with PAMI_Memregion_destroy().
!  *
!  * \retval PAMI_INVAL   An invalid parameter value was specified. The memory
!  *                      region does not need to be free'd with
!  *                      PAMI_Memregion_destroy().
!  *
!  * \retval PAMI_ERROR   The memory region was not pinned and does not need to
!  *                      be free'd with PAMI_Memregion_destroy().
!  */
!  pami_result_t PAMI_Memregion_create (pami_context_t     context,
!                                       void             * address,
!                                       size_t             bytes_in,
!                                       size_t           * bytes_out,
!                                       pami_memregion_t * memregion);

! /**
!  * \brief Destroy a local memory region for one sided operations
!  *
!  * \param[in] context   PAMI application context
!  * \param[in] memregion Memory region object
!  *
!  * The memregion object will be changed to an invalid value so that
!  * it is clearly destroyed.
!  */
!  pami_result_t PAMI_Memregion_destroy (pami_context_t     context,pami_memregion_t * memregion);

    type pami_rma_mr_t
        sequence
        integer(4)  :: mr       ! /**< Memory region */
        integer(4)  :: offset   ! /**< Offset from beginning of memory region */
    end type pami_rma_mr_t

    type pami_rdma_t
        sequence
        integer(4)  :: local(2) ! /**< Local memory region information */
        integer(4)  :: remote(2)! /**< Remote memory region information */
    end type pami_rdma_t

! /**
!  * \brief Input parameter structure for simple rdma put transfers
!  * \code
!  * pami_rput_simple_t parameters;
!  * parameters.rma.dest           = ;
!  * parameters.rma.hints          = ;
!  * parameters.rma.bytes          = ;
!  * parameters.rma.cookie         = ;
!  * parameters.rma.done_fn        = ;
!  * parameters.rdma.local.mr      = ;
!  * parameters.rdma.local.offset  = ;
!  * parameters.rdma.remote.mr     = ;
!  * parameters.rdma.remote.offset = ;
!  * parameters.put.rdone_fn       = ;
!  * \endcode
!  */
    type pami_rput_simple_t
        sequence
        integer(4)  :: rma(13)  ! /**< Common rma parameters */
        integer(4)  :: rdma(4)  ! /**< Common rdma memregion parameters */
        integer(4)  :: put      ! /**< Common put parameters */
    end type pami_rput_simple_t

! /**
!  * \brief Input parameter structure for typed rdma put transfers
!  * \code
!  * pami_rput_typed_t parameters;
!  * parameters.rma.dest           = ;
!  * parameters.rma.hints          = ;
!  * parameters.rma.bytes          = ;
!  * parameters.rma.cookie         = ;
!  * parameters.rma.done_fn        = ;
!  * parameters.rdma.local.mr      = ;
!  * parameters.rdma.local.offset  = ;
!  * parameters.rdma.remote.mr     = ;
!  * parameters.rdma.remote.offset = ;
!  * parameters.type.local         = ;
!  * parameters.type.remote        = ;
!  * parameters.put.rdone_fn       = ;
!  * \endcode
!  */
    type pami_rput_typed_t
        sequence
        integer(4)  :: rma(13)  ! /**< Common rma parameters */
        integer(4)  :: rdma(6)  ! /**< Common rdma memregion parameters */
        integer(4)  :: type(2)  ! /**< Common rma typed parameters */
        integer(4)  :: put      ! /**< Common put parameters */
    end type pami_rput_typed_t

! /**
!  * \brief Simple put operation for one-sided contiguous data transfer.
!  *
!  * \param[in] context    PAMI application context
!  * \param[in] parameters Input parameters structure
!  */
!  pami_result_t PAMI_Rput (pami_context_t context, pami_rput_simple_t * parameters);

! /**
!  * \brief Put operation for data type specific one-sided data transfer.
!  *
!  * \param[in] context    PAMI application context
!  * \param[in] parameters Input parameters structure
!  */
!  pami_result_t PAMI_Rput_typed (pami_context_t context, pami_rput_typed_t * parameters);

! /**
!  * \brief Input parameter structure for simple rdma get transfers
!  * \code
!  * pami_rget_simple_t parameters;
!  * parameters.rma.dest           = ;
!  * parameters.rma.hints          = ;
!  * parameters.rma.bytes          = ;
!  * parameters.rma.cookie         = ;
!  * parameters.rma.done_fn        = ;
!  * parameters.rdma.local.mr      = ;
!  * parameters.rdma.local.offset  = ;
!  * parameters.rdma.remote.mr     = ;
!  * parameters.rdma.remote.offset = ;
!  * \endcode
!  */
    type pami_rget_simple_t
        sequence
        integer(4)  :: rma(13)  ! /**< Common rma parameters */
        integer(4)  :: rdma(6)  ! /**< Common rdma memregion parameters */
    end type pami_rget_simple_t

! /**
!  * \brief Simple get operation for one-sided contiguous data transfer.
!  *
!  * \param[in] context    PAMI application context
!  * \param[in] parameters Input parameters structure
!  */
!  pami_result_t PAMI_Rget (pami_context_t context, pami_rget_simple_t * parameters);

! /**
!  * \brief Input parameter structure for typed rdma get transfers
!  * \code
!  * pami_rget_typed_t parameters;
!  * parameters.rma.dest           = ;
!  * parameters.rma.hints          = ;
!  * parameters.rma.bytes          = ;
!  * parameters.rma.cookie         = ;
!  * parameters.rma.done_fn        = ;
!  * parameters.rdma.local.mr      = ;
!  * parameters.rdma.local.offset  = ;
!  * parameters.rdma.remote.mr     = ;
!  * parameters.rdma.remote.offset = ;
!  * parameters.type.local         = ;
!  * parameters.type.remote        = ;
!  * \endcode
!  */
    type pami_rget_typed_t
        sequence
        integer(4)  :: rma(13)  ! /**< Common rma parameters */
        integer(4)  :: rdma(6)  ! /**< Common rdma memregion parameters */
        integer(4)  :: type(2)  ! /**< Common rma typed parameters */
    end type pami_rget_typed_t

! /**
!  * \brief Get operation for data type specific one-sided data transfer.
!  *
!  * \param[in] context    PAMI application context
!  * \param[in] parameters Input parameters structure
!  */
!  pami_result_t PAMI_Rget_typed (pami_context_t        context,
!                                 pami_rget_typed_t *   parameters);

! /** \} */ /* end of "rdma" group */
! /** \} */ /* end of "rma" group */



! /*****************************************************************************/
! /**
!  * \defgroup dynamictasks pami dynamic task interface
!  *
!  * Some brief documentation on dynamic task stuff ...
!  * \{
!  */
! /*****************************************************************************/

! /**
!  * \brief Clean up local resources to an endpoint in preparation for
!  *        task shutdown or checkpoint
!  *
!  *        It is the user of this API's responsibility to ensure
!  *        that all communication has been quiesced to and from
!  *        the destination via a fence call and synchronization
!  *
!  * \param[in] context    PAMI communication context
!  * \param[in] dest       Array of destination endpoints to close connections to
!  * \param[in] count      Number of endpoints in the array dest
!  */

!  pami_result_t PAMI_Purge (pami_context_t    context,
!                            pami_endpoint_t * dest,
!                            size_t           count);

! /**
!  * \brief Setup local resources to an endpoint in preparation for
!  *        task restart or creation
!  *
!  * \param[in] context    PAMI communication context
!  * \param[in] dest       Array of destination endpoints to resume connections to
!  * \param[in] count      Number of endpoints in the array dest
!  */
!  pami_result_t PAMI_Resume (pami_context_t    context,
!                             pami_endpoint_t * dest,
!                             size_t           count);

! /** \} */ /* end of "dynamic tasks" group */


! /*****************************************************************************/
! /**
!  * \defgroup sync pami memory synchronization and data fence interface
!  *
!  * Some brief documentation on sync stuff ...
!  * \{
!  */
! /*****************************************************************************/

! /**
!  * \brief Begin a memory synchronization region
!  *
!  * A fence region is defined as an area of program control on the local task
!  * bounded by the PAMI_Fence_begin() and PAMI_Fence_end() functions.
!  *
!  * \warning It is considered \b illegal to invoke a fence operation outside of
!  *          a fence region.
!  *
!  * \warning It is considered \b illegal to begin a fence region inside an
!  *          existing fence region. Fence regions can not be nested.
!  *
!  * \param[in] context PAMI communication context
!  */
! pami_result_t PAMI_Fence_begin (pami_context_t context);

! /**
!  * \brief End a memory synchronization region
!  *
!  * A fence region is defined as an area of program control on the local task
!  * bounded by the PAMI_Fence_begin() and PAMI_Fence_end() functions.
!  *
!  * \warning It is considered \b illegal to invoke a fence operation outside of
!  *          a fence region.
!  *
!  * \warning It is considered \b illegal to end a fence region outside of an
!  *          existing fence region.
!  *
!  * \param[in] context PAMI communication context
!  */
! pami_result_t PAMI_Fence_end (pami_context_t context);


! /**
!  * \brief Syncronize all transfers between all endpoints on a context.
!  *
!  * \param[in] context PAMI communication context
!  * \param[in] done_fn Event callback to invoke when the fence is complete
!  * \param[in] cookie  Event callback argument
!  */
!  pami_result_t PAMI_Fence_all (pami_context_t        context,
!                                pami_event_function   done_fn,
!                                void                * cookie);

! /**
!  * \brief Syncronize all transfers to an endpoints.
!  *
!  * \param[in] context PAMI communication context
!  * \param[in] done_fn Event callback to invoke when the fence is complete
!  * \param[in] cookie  Event callback argument
!  * \param[in] target  Endpoint to synchronize
!  */
!  pami_result_t PAMI_Fence_endpoint (pami_context_t        context,
!                                     pami_event_function   done_fn,
!                                     void                * cookie,
!                                     pami_endpoint_t       target);

! /** \} */ /* end of "sync" group */


! /* ************************************************************************* */
! /* ********* Transfer Types,used by geometry and xfer routines ************ */
! /* ************************************************************************* */
    integer PAMI_XFER_BROADCAST
    integer PAMI_XFER_ALLREDUCE
    integer PAMI_XFER_REDUCE
    integer PAMI_XFER_ALLGATHER
    integer PAMI_XFER_ALLGATHERV
    integer PAMI_XFER_ALLGATHERV_INT
    integer PAMI_XFER_SCATTER
    integer PAMI_XFER_SCATTERV
    integer PAMI_XFER_SCATTERV_INT
    integer PAMI_XFER_GATHER
    integer PAMI_XFER_GATHERV
    integer PAMI_XFER_GATHERV_INT
    integer PAMI_XFER_BARRIER
    integer PAMI_XFER_FENCE
    integer PAMI_XFER_ALLTOALL
    integer PAMI_XFER_ALLTOALLV
    integer PAMI_XFER_ALLTOALLV_INT
    integer PAMI_XFER_SCAN
    integer PAMI_XFER_REDUCE_SCATTER
    integer PAMI_XFER_AMBROADCAST
    integer PAMI_XFER_AMSCATTER
    integer PAMI_XFER_AMGATHER
    integer PAMI_XFER_AMREDUCE
    integer PAMI_XFER_COUNT
    integer PAMI_XFER_TYPE_EXT      ! /**< Begin extension-specific values */
    parameter (PAMI_XFER_BROADCAST=0)
    parameter (PAMI_XFER_ALLREDUCE=1)
    parameter (PAMI_XFER_REDUCE=2)
    parameter (PAMI_XFER_ALLGATHER=3)
    parameter (PAMI_XFER_ALLGATHERV=4)
    parameter (PAMI_XFER_ALLGATHERV_INT=5)
    parameter (PAMI_XFER_SCATTER=6)
    parameter (PAMI_XFER_SCATTERV=7)
    parameter (PAMI_XFER_SCATTERV_INT=8)
    parameter (PAMI_XFER_GATHER=9)
    parameter (PAMI_XFER_GATHERV=10)
    parameter (PAMI_XFER_GATHERV_INT=11)
    parameter (PAMI_XFER_BARRIER=12)
    parameter (PAMI_XFER_FENCE=13)
    parameter (PAMI_XFER_ALLTOALL=14)
    parameter (PAMI_XFER_ALLTOALLV=15)
    parameter (PAMI_XFER_ALLTOALLV_INT=16)
    parameter (PAMI_XFER_SCAN=17)
    parameter (PAMI_XFER_REDUCE_SCATTER=18)
    parameter (PAMI_XFER_AMBROADCAST=19)
    parameter (PAMI_XFER_AMSCATTER=20)
    parameter (PAMI_XFER_AMGATHER=21)
    parameter (PAMI_XFER_AMREDUCE=22)
    parameter (PAMI_XFER_COUNT=23)
    parameter (PAMI_XFER_TYPE_EXT=1000)

! /* ************************************************************************* */
! /* **************     Geometry (like groups/communicators)  **************** */
! /* ************************************************************************* */

! typedef void*   pami_geometry_t;
! typedef size_t  pami_algorithm_t;
    
    integer PAMI_GEOMETRY_T
    integer PAMI_ALGORITHM_T
    parameter (PAMI_GEOMETRY_T=4)
    parameter (PAMI_ALGORITHM_T=4)

  type pami_geometry_range_t
    sequence
    integer(4)  :: lo
    integer(4)  :: hi
  end type pami_geometry_range_t


! extern pami_geometry_t PAMI_NULL_GEOMETRY;
! /**
!  * \brief Initialize the geometry
!  *        A synchronizing operation will take place during geometry_initialize
!  *        on the parent geometry
!  *        If the output geometry "geometry" pointer is NULL,then no geometry will be
!  *        created,however,all nodes in the parent must participate in the
!  *        geometry_initialize operation,even if they do not create a geometry
!  *
!  *        If a geometry is created without a parent geometry (parent is set to
!  *        PAMI_NULL_GEOMETRY),then an "immediate" geometry will be created.
!  *        In this case,the new geometry will be created and synchronized.
!  *        However,the new geometry cannot take advantage of optimized collectives
!  *        from the parent in the creation of the new geometry.  This kind of
!  *        geometry create may not be as optimal as when a parent has been provided
!  *
!  * \param[in]  client          pami client
!  * \param[in]  configuration   List of configurable attributes and values
!  * \param[in]  num_configs     The number of configuration elements
!  * \param[out] geometry        Opaque geometry object to initialize
!  * \param[in]  parent          Parent geometry containing all the nodes in the task list
!  * \param[in]  id              Identifier for this geometry
!  *                             which uniquely represents this geometry(if tasks overlap)
!  * \param[in]  task_slices     Array of node slices participating in the geometry
!  *                             User must keep the array of slices in memory for the
!  *                             duration of the geometry's existence
!  * \param[in]  slice_count     Number of task slices participating in the geometry
!  * \param[in]  context         context to deliver async callback to
!  * \param[in]  fn              event function to call when geometry has been created
!  * \param[in]  cookie          user cookie to deliver with the callback
!  */

!  pami_result_t PAMI_Geometry_create_taskrange (pami_client_t           client,
!                                                pami_configuration_t    configuration[],
!                                                size_t                  num_configs,
!                                                pami_geometry_t       * geometry,
!                                                pami_geometry_t         parent,
!                                                unsigned                id,
!                                                pami_geometry_range_t * task_slices,
!                                                size_t                  slice_count,
!                                                pami_context_t          context,
!                                                pami_event_function     fn,
!                                                void                  * cookie);

! /**
!  * \brief Initialize the geometry
!  *        A synchronizing operation will take place during geometry_initialize
!  *        on the parent geometry
!  *        If the output geometry "geometry" pointer is NULL,then no geometry will be
!  *        created,however,all nodes in the parent must participate in the
!  *        geometry_initialize operation,even if they do not create a geometry
!  *
!  *        If a geometry is created without a parent geometry (parent is set to
!  *        PAMI_NULL_GEOMETRY),then an "immediate" geometry will be created.
!  *        In this case,the new geometry will be created and synchronized,
!  *        However,the new geometry cannot take advantage of optimized collectives
!  *        from the parent in the creation of the new geometry.  This kind of
!  *        geometry create may not be as optimal as when a parent has been provided
!  *
!  *
!  * \param[in]  client          pami client
!  * \param[in]  configuration   List of configurable attributes and values
!  * \param[in]  num_configs     The number of configuration elements
!  * \param[out] geometry        Opaque geometry object to initialize
!  * \param[in]  parent          Parent geometry containing all the nodes in the task list
!  * \param[in]  id              Identifier for this geometry
!  *                             which uniquely represents this geometry(if tasks overlap)
!  * \param[in]  tasks           Array of tasks to build the geometry list
!  *                             User must keep the task list in memory
!  *                             duration of the geometry's existence
!  * \param[in]  task_count      Number of tasks participating in the geometry
!  * \param[in]  context         context to deliver async callback to
!  * \param[in]  fn              event function to call when geometry has been created
!  * \param[in]  cookie          user cookie to deliver with the callback
!  */

!  pami_result_t PAMI_Geometry_create_tasklist (pami_client_t               client,
!                                               pami_configuration_t        configuration[],
!                                               size_t                      num_configs,
!                                               pami_geometry_t           * geometry,
!                                               pami_geometry_t             parent,
!                                               unsigned                    id,
!                                               pami_task_t               * tasks,
!                                               size_t                      task_count,
!                                               pami_context_t              context,
!                                               pami_event_function         fn,
!                                               void                      * cookie);

! /**
!  * \brief Initialize the geometry
!  *
!  * \param[in]  client          pami client
!  * \param[in]  world_geometry  world geometry object
!  */
!  pami_result_t PAMI_Geometry_world (pami_client_t                client,
!                                     pami_geometry_t            * world_geometry);

! /**
!  * \brief determines the number of algorithms available for a given op
!  *        in the two different lists (always work list,
!  *        under-cetain conditions list).
!  *
!  * \param[in]     context       pami context
!  * \param[in]     geometry      An input geometry to be analyzed.
!  * \param[in]     coll_type     type of collective op.
!  * \param[in,out] lists_lengths array of 2 numbers representing all valid
!  *                              algorithms and optimized algorithms.
!  *
!  * \retval        PAMI_SUCCESS  number of algorithms is determined.
!  * \retval        PAMI_INVAL    There is an error with input parameters
!  */
!  pami_result_t PAMI_Geometry_algorithms_num (pami_context_t   context,
!                                              pami_geometry_t  geometry,
!                                              pami_xfer_type_t coll_type,
!                                              size_t          *lists_lengths);

! /**
!  * \brief fills in the protocols and attributes for a set of algorithms
!  *        The first lists are used to populate collectives that work under
!  *        any condidtion.  The second lists are used to populate
!  *        collectives that the metadata must be checked before use
!  *
!  * \param[in]     context        pami context
!  * \param[in]     coll_type      type of collective op.
!  * \param[in,out] algs0          array of algorithms to query
!  * \param[in,out] mdata0         metadata array to be filled in if algorithms
!  *                               are applicable,can be NULL.
!  * \param[in]     num0           number of algorithms to fill in.
!  * \param[in,out] algs1          array of algorithms to query
!  * \param[in,out] mdata1         metadata array to be filled in if algorithms
!  *                               are applicable,can be NULL.
!  * \param[in]     num1           number of algorithms to fill in.
!  *
!  * \retval        PAMI_SUCCESS   algorithm is applicable to geometry.
!  * \retval        PAMI_INVAL     Error in input arguments or not applicable.
!  */
!  pami_result_t PAMI_Geometry_algorithms_query (pami_context_t     context,
!                                               pami_geometry_t     geometry,
!                                               pami_xfer_type_t    colltype,
!                                               pami_algorithm_t *  algs0,
!                                               pami_metadata_t  *  mdata0,
!                                               size_t              num0,
!                                               pami_algorithm_t *  algs1,
!                                               pami_metadata_t  *  mdata1,
!                                               size_t              num1);

! /**
!  * \brief Free any memory allocated inside of a geometry.
!  * \param[in] client   pami client
!  * \param[in] geometry The geometry object to free
!  * \retval PAMI_SUCCESS Memory free didn't fail
!  *
!  * The geometry handle will be changed to an invalid value so that
!  * it is clearly destroyed.
!  */
!  pami_result_t PAMI_Geometry_destroy(pami_client_t     client,
!                                      pami_geometry_t * geometry);

! /**
!  * \brief Create and post a non-blocking alltoall vector operation.
!  *
!  * The alltoallv operation ...
!  *
!  * \param[in]  cb_done      Callback to invoke when message is complete.
!  * \param[in]  geometry     Geometry to use for this collective operation.
!  * \param[in]  sndbuf       The base address of the buffers containing data to be sent
!  * \param[in]  stype        A single type datatype
!  * \param[in]  stypecounts  An array of type replication counts.  Size of geometry length
!  * \param[in]  sdispls      Array of offsets into the sndbuf.  Size of geometry length
!  * \param[out] rcvbuf       The base address of the buffer for data reception
!  * \param[in]  rtype        A single type datatype
!  * \param[in]  rtypecounts  Array of type replication counts.  Size of geometry length
!  * \param[in]  rdispls      Array of offsets into the rcvbuf.  Size of geometry length
!  *
!  * \retval     0            Success
!  *
!  */
    type pami_alltoallv_t
        sequence
        integer(4)  :: sndbuf
        integer(4)  :: stype
        integer(4)  :: stypecounts
        integer(4)  :: sdispls
        integer(4)  :: rcvbuf
        integer(4)  :: rtype
        integer(4)  :: rtypecounts
        integer(4)  :: rdispls
    end type pami_alltoallv_t

! /**
!  * \brief Create and post a non-blocking alltoall vector operation.
!  *
!  * The alltoallv_int operation ...
!  *
!  * \param[in]  cb_done      Callback to invoke when message is complete.
!  * \param[in]  geometry     Geometry to use for this collective operation.
!  * \param[in]  sndbuf       The base address of the buffers containing data to be sent
!  * \param[in]  stype        A single type datatype
!  * \param[in]  stypecounts  An array of type replication counts.  Size of geometry length
!  * \param[in]  sdispls      Array of offsets into the sndbuf.  Size of geometry length
!  * \param[out] rcvbuf       The base address of the buffer for data reception
!  * \param[in]  rtype        A single type datatype
!  * \param[in]  rtypecounts  Array of type replication counts.  Size of geometry length
!  * \param[in]  rdispls      Array of offsets into the rcvbuf.  Size of geometry length
!  *
!  * \retval     0            Success
!  *
!  */
    type pami_alltoallv_int_t
        sequence
        integer(4)  :: sndbuf
        integer(4)  :: stype
        integer(4)  :: stypecounts
        integer(4)  :: sdispls
        integer(4)  :: rcvbuf
        integer(4)  :: rtype
        integer(4)  :: rtypecounts
        integer(4)  :: rdispls
    end type pami_alltoallv_int_t


! /**
!  * \brief Create and post a non-blocking alltoall operation.
!  * The alltoall operation ...
!  *
!  * \param[in]  cb_done     Callback to invoke when message is complete.
!  * \param[in]  geometry    Geometry to use for this collective operation.
!  * \param[in]  sndbuf      The base address of the buffers containing data to be sent
!  * \param[in]  stype       Single datatype of the send buffer
!  * \param[in]  stypecount  Single type replication count
!  * \param[out] rbuf        The base address of the buffer for data reception
!  * \param[in]  rtype       Single datatype of the receive buffer
!  * \param[in]  rtypecount  Single type replication count
!  *
!  * \retval     0            Success
!  *
!  * \todo doxygen
!  */
    type pami_alltoall_t
        sequence
        integer(4)  :: sndbuf
        integer(4)  :: stype
        integer(4)  :: stypecount
        integer(4)  :: rcvbuf
        integer(4)  :: rtype
        integer(4)  :: rtypecount
    end type pami_alltoall_t

! /**
!  * \brief Create and post a non-blocking reduce operation.
!  * The reduce operation ...
!  *
!  * \param[in]  cb_done      Callback to invoke when message is complete.
!  * \param[in]  geometry     Geometry to use for this collective operation.
!  * \param[in]  root         Task ID of the reduce root node.
!  * \param[in]  sbuffer      Source buffer.
!  * \param[in]  stype        Source buffer type
!  * \param[in]  stypecount   Source buffer type count
!  * \param[in]  rbuffer      Receive buffer.
!  * \param[in]  rtype        Receive buffer layout
!  * \param[in]  rtypecount   Receive buffer type count
!  * \param[in]  dt           Element data type
!  * \param[in]  op           Reduce operation
!  *
!  * \retval     0            Success
!  */
    type pami_reduce_t
        sequence
        integer(4)  :: root
        integer(4)  :: sndbuf
        integer(4)  :: stype
        integer(4)  :: stypecount
        integer(4)  :: rcvbuf
        integer(4)  :: rtype
        integer(4)  :: rtypecount
        integer(4)  :: dt
        integer(4)  :: op
    end type pami_reduce_t

! /**
!  * \brief Create and post a non-blocking reduce_scatter operation.
!  *
!  * The reduce_scatter operation ...
!  *
!  * \param[in]  cb_done      Callback to invoke when message is complete.
!  * \param[in]  geometry     Geometry to use for this collective operation.
!  * \param[in]  sbuffer      Source buffer.
!  * \param[in]  stype        Source buffer type
!  * \param[in]  stypecount   Source buffer type count
!  * \param[in]  rbuffer      Receive buffer.
!  * \param[in]  rtype        Receive buffer layout
!  * \param[in]  rtypecount   Receive buffer type count
!  * \param[in]  rcounts      number of elements to receive from the destinations(common on all nodes)
!  * \param[in]  dt           Element data type
!  * \param[in]  op           Reduce operation
!  *
!  * \retval     0            Success
!  *
!  */
    type pami_reduce_scatter_t
        sequence
        integer(4)  :: sndbuf
        integer(4)  :: stype
        integer(4)  :: stypecount
        integer(4)  :: rcvbuf
        integer(4)  :: rtype
        integer(4)  :: rtypecount
        integer(4)  :: rcounts
        integer(4)  :: dt
        integer(4)  :: op
    end type pami_reduce_scatter_t

! /**
!  * \brief Create and post a non-blocking broadcast operation.
!  *
!  * The broadcast operation ...
!  *
!  * \param[in]  cb_done      Callback to invoke when message is complete.
!  * \param[in]  geometry     Geometry to use for this collective operation.
!  * \param[in]  root         Task ID of the node performing the broadcast.
!  * \param[in]  buf          Source buffer to broadcast on root,dest buffer on non-root
!  * \param[in]  type         data type layout,may be different on root/destinations
!  * \param[in]  count        Single type replication count
!  *
!  * \retval     0            Success
!  *
!  */
    type pami_broadcast_t
        sequence
        integer(4)  :: root
        integer(4)  :: buf
        integer(4)  :: type
        integer(4)  :: typecount
    end type pami_broadcast_t


! /**
!  * \brief Create and post a non-blocking allgather
!  *
!  * The allgather
!  *
!  * \param[in]  cb_done      Callback to invoke when message is complete.
!  * \param[in]  geometry     Geometry to use for this collective operation.
!  * \param[in]  src          Source buffer to send
!  * \param[in]  stype        data layout of send buffer
!  * \param[in]  stypecount   replication count of the type
!  * \param[in]  rcv          Source buffer to receive the data
!  * \param[in]  rtype        data layout of each receive buffer
!  * \param[in]  rtypecount   replication count of the type
!  *
!  * \retval     0            Success
!  *
!  */
    type pami_allgather_t
        sequence
        integer(4)  :: sndbuf
        integer(4)  :: stype
        integer(4)  :: stypecount
        integer(4)  :: rcvbuf
        integer(4)  :: rtype
        integer(4)  :: rtypecount
    end type pami_allgather_t


! /**
!  * \brief Create and post a non-blocking allgather
!  *
!  * The gather
!  *
!  * \param[in]  cb_done      Callback to invoke when message is complete.
!  * \param[in]  geometry     Geometry to use for this collective operation.
!  * \param[in]  root         The root node of the gather operation
!  * \param[in]  src          Source buffer to send
!  * \param[in]  stype        data layout of send buffer
!  * \param[in]  stypecount   replication count of the type
!  * \param[in]  rcv          Source buffer to receive the data
!  * \param[in]  rtype        data layout of each receive buffer
!  * \param[in]  rtypecount   replication count of the type
!  *
!  * \retval     0            Success
!  *
!  * \todo doxygen
!  */
    type pami_gather_t
        sequence
        integer(4)  :: root
        integer(4)  :: sndbuf
        integer(4)  :: stype
        integer(4)  :: stypecount
        integer(4)  :: rcvbuf
        integer(4)  :: rtype
        integer(4)  :: rtypecount
    end type pami_gather_t

! /**
!  * \brief Create and post a non-blocking gatherv
!  *
!  * The gatherv
!  *
!  * \param[in]  request      Opaque memory to maintain internal message state.
!  * \param[in]  cb_done      Callback to invoke when message is complete.
!  * \param[in]  geometry     Geometry to use for this collective operation.
!  * \param[in]  root         The root node for the gatherv operation
!  * \param[in]  sndbuf       The base address of the buffers containing data to be sent
!  * \param[in]  stype        A single type datatype
!  * \param[in]  stypecount   type replication count.
!  * \param[out] rcvbuf       The base address of the buffer for data reception
!  * \param[in]  rtype        A single type datatype
!  * \param[in]  rtypecounts  Array of type replication counts.  Size of geometry length
!  * \param[in]  rdispls      Array of offsets into the rcvbuf.  Size of geometry length
!  *
!  * \retval     0            Success
!  *
!  * \todo doxygen
!  */
    type pami_gatherv_t
        sequence
        integer(4)  :: root
        integer(4)  :: sndbuf
        integer(4)  :: stype
        integer(4)  :: stypecount
        integer(4)  :: rcvbuf
        integer(4)  :: rtype
        integer(4)  :: rtypecounts
        integer(4)  :: rdispls
    end type pami_gatherv_t

! /**
!  * \brief Create and post a non-blocking gatherv
!  *
!  * The gatherv_int routine
!  *
!  * \param[in]  request      Opaque memory to maintain internal message state.
!  * \param[in]  cb_done      Callback to invoke when message is complete.
!  * \param[in]  geometry     Geometry to use for this collective operation.
!  * \param[in]  root         The root node for the gatherv operation
!  * \param[in]  sndbuf       The base address of the buffers containing data to be sent
!  * \param[in]  stype        A single type datatype
!  * \param[in]  stypecount   type replication count.
!  * \param[out] rcvbuf       The base address of the buffer for data reception
!  * \param[in]  rtype        A single type datatype
!  * \param[in]  rtypecounts  Array of type replication counts.  Size of geometry length
!  * \param[in]  rdispls      Array of offsets into the rcvbuf.  Size of geometry length
!  *
!  * \retval     0            Success
!  *
!  * \todo doxygen
!  */
    type pami_gatherv_int_t
        sequence
        integer(4)  :: root
        integer(4)  :: sndbuf
        integer(4)  :: stype
        integer(4)  :: stypecount
        integer(4)  :: rcvbuf
        integer(4)  :: rtype
        integer(4)  :: rtypecounts
        integer(4)  :: rdispls
    end type pami_gatherv_int_t

! /**
!  * \brief Create and post a non-blocking allgatherv
!  *
!  * The allgatherv
!  *
!  * \param[in]  cb_done      Callback to invoke when message is complete.
!  * \param[in]  geometry     Geometry to use for this collective operation.
!  * \param[in]  sndbuf       The base address of the buffers containing data to be sent
!  * \param[in]  stype        A single type datatype
!  * \param[in]  stypecount   type replication count.
!  * \param[out] rcvbuf       The base address of the buffer for data reception
!  * \param[in]  rtype        A single type datatype
!  * \param[in]  rtypecounts  Array of type replication counts.  Size of geometry length
!  * \param[in]  rdispls      Array of offsets into the rcvbuf.  Size of geometry length
!  *
!  * \retval     0            Success
!  *
!  * \todo doxygen
!  */
    type pami_allgatherv_t
        sequence
        integer(4)  :: sndbuf
        integer(4)  :: stype
        integer(4)  :: stypecount
        integer(4)  :: rcvbuf
        integer(4)  :: rtype
        integer(4)  :: rtypecounts
        integer(4)  :: rdispls
    end type pami_allgatherv_t

! /**
!  * \brief Create and post a non-blocking allgatherv
!  *
!  * The allgatherv_int
!  *
!  * \param[in]  cb_done      Callback to invoke when message is complete.
!  * \param[in]  geometry     Geometry to use for this collective operation.
!  * \param[in]  sndbuf       The base address of the buffers containing data to be sent
!  * \param[in]  stype        A single type datatype
!  * \param[in]  stypecount   type replication count.
!  * \param[out] rcvbuf       The base address of the buffer for data reception
!  * \param[in]  rtype        A single type datatype
!  * \param[in]  rtypecounts  Array of type replication counts.  Size of geometry length
!  * \param[in]  rdispls      Array of offsets into the rcvbuf.  Size of geometry length
!  *
!  * \retval     0            Success
!  *
!  * \todo doxygen
!  */
    type pami_allgatherv_int_t
        sequence
        integer(4)  :: sndbuf
        integer(4)  :: stype
        integer(4)  :: stypecount
        integer(4)  :: rcvbuf
        integer(4)  :: rtype
        integer(4)  :: rtypecounts
        integer(4)  :: rdispls
    end type pami_allgatherv_int_t


! /**
!  * \brief Create and post a non-blocking scatter
!  *
!  * The scatter
!  *
!  * \param[in]  cb_done      Callback to invoke when message is complete.
!  * \param[in]  geometry     Geometry to use for this collective operation.
!  * \param[in]  root         Task ID of the reduce root node.
!  * \param[in]  sbuffer      Source buffer.
!  * \param[in]  stype        Source buffer type
!  * \param[in]  stypecount   Source buffer type count
!  * \param[in]  rbuffer      Receive buffer.
!  * \param[in]  rtype        Receive buffer layout
!  * \param[in]  rtypecount   Receive buffer type count
!  *
!  * \retval     0            Success
!  *
!  * \todo doxygen
!  */
    type pami_scatter_t
        sequence
        integer(4)  :: root
        integer(4)  :: sndbuf
        integer(4)  :: stype
        integer(4)  :: stypecount
        integer(4)  :: rcvbuf
        integer(4)  :: rtype
        integer(4)  :: rtypecount
    end type pami_scatter_t

! /**
!  * \brief Create and post a non-blocking scatterv
!  *
!  * The scatterv
!  *
!  * \param[in]  cb_done      Callback to invoke when message is complete.
!  * \param[in]  geometry     Geometry to use for this collective operation.
!  * \param[in]  sndbuf       The base address of the buffers containing data to be sent
!  * \param[in]  stype        A single type datatype
!  * \param[in]  stypecounts  An array of type replication counts.  Size of geometry length
!  * \param[in]  sdispls      Array of offsets into the sndbuf.  Size of geometry length
!  * \param[in]  rbuffer      Receive buffer.
!  * \param[in]  rtype        A single type datatype
!  * \param[in]  rtypecount   Receive buffer type replication count
!  *
!  * \retval     0            Success
!  *
!  * \todo doxygen
!  */
    type pami_scatterv_t
        integer(4)  :: root
        integer(4)  :: sndbuf
        integer(4)  :: stype
        integer(4)  :: stypecounts
        integer(4)  :: sdispls
        integer(4)  :: rcvbuf
        integer(4)  :: rtype
        integer(4)  :: rtypecount
    end type pami_scatterv_t

! /**
!  * \brief Create and post a non-blocking scatterv
!  *
!  * The scatterv_int
!  *
!  * \param[in]  cb_done      Callback to invoke when message is complete.
!  * \param[in]  geometry     Geometry to use for this collective operation.
!  * \param[in]  sndbuf       The base address of the buffers containing data to be sent
!  * \param[in]  stype        A single type datatype
!  * \param[in]  stypecounts  An array of type replication counts.  Size of geometry length
!  * \param[in]  sdispls      Array of offsets into the sndbuf.  Size of geometry length
!  * \param[in]  rbuffer      Receive buffer.
!  * \param[in]  rtype        A single type datatype
!  * \param[in]  rtypecount   Receive buffer type replication count
!  *
!  * \retval     0            Success
!  *
!  * \todo doxygen
!  */
    type pami_scatterv_int_t
        sequence
        integer(4)  :: root
        integer(4)  :: sndbuf
        integer(4)  :: stype
        integer(4)  :: stypecounts
        integer(4)  :: sdispls
        integer(4)  :: rcvbuf
        integer(4)  :: rtype
        integer(4)  :: rtypecount
    end type pami_scatterv_int_t


! /**
!  * \brief Create and post a non-blocking allreduce operation.
!  *
!  * The allreduce operation ...
!  *
!  * \param[in]  cb_done      Callback to invoke when message is complete.
!  * \param[in]  geometry     Geometry to use for this collective operation.
!  * \param[in]  sbuffer      Source buffer.
!  * \param[in]  stype        Source buffer type
!  * \param[in]  stypecount   Source buffer type count
!  * \param[in]  rbuffer      Receive buffer.
!  * \param[in]  rtype        Receive buffer layout
!  * \param[in]  rtypecount   Receive buffer type count
!  * \param[in]  dt           Element data type
!  * \param[in]  op           Reduce operation
!  *
!  * \retval     0            Success
!  *
!  * \todo doxygen
!  * \todo discuss collapsing pami_dt into type type.
!  */
    type pami_allreduce_t
        sequence
        integer(4)  :: sndbuf
        integer(4)  :: stype
        integer(4)  :: stypecount
        integer(4)  :: rcvbuf
        integer(4)  :: rtype
        integer(4)  :: rtypecount
        integer(4)  :: dt
        integer(4)  :: op
    end type pami_allreduce_t

! /**
!  * \brief Create and post a non-blocking scan operation.
!  *
!  * The scan operation ...
!  *
!  * \param[in]  cb_done      Callback to invoke when message is complete.
!  * \param[in]  geometry     Geometry to use for this collective operation.
!  * \param[in]  sbuffer      Source buffer.
!  * \param[in]  stype        Source buffer type
!  * \param[in]  stypecount   Source buffer type count
!  * \param[in]  rbuffer      Receive buffer.
!  * \param[in]  rtype        Receive buffer layout
!  * \param[in]  rtypecount   Receive buffer type count
!  * \param[in]  dt           Element data type
!  * \param[in]  op           Reduce operation
!  * \param[in]  exclusive    scan operation is exclusive of current node
!  *
!  * \retval     0            Success
!  *
!  * \todo doxygen
!  * \todo discuss collapsing pami_dt into type type.
!  */
    type pami_scan_t
        sequence
        integer(4)  :: sndbuf
        integer(4)  :: stype
        integer(4)  :: stypecount
        integer(4)  :: rcvbuf
        integer(4)  :: rtype
        integer(4)  :: rtypecount
        integer(4)  :: dt
        integer(4)  :: op
        integer(4)  :: exclusive
    end type pami_scan_t

! /**
!  * \brief Create and post a non-blocking barrier operation.
!  * The barrier operation ...
!  * \param      geometry     Geometry to use for this collective operation.
!  * \param[in]  cb_done      Callback to invoke when message is complete.
!  * \retval  0            Success
!  *
!  * \see PAMI_Barrier_register
!  *
!  * \todo doxygen
!  */
    type pami_barrier_t
    end type pami_barrier_t


! /**
!  * \brief Create and post a non-blocking fence operation.
!  * The fence operation
!  *
!  * Use this subroutine to enforce order on PAMI calls.  It can also be used for
!  * "batch message completion"  This is a one sided collective call,meaning that the
!  * collective can be issued from a single node issuing the fence.
!  * On completion of this operation,it is assumed that all PAMI communication associated
!  * with the current context from all participants in the geometry have quiesced.
!  * Although the context is local,the geometry represents a set of tasks
!  * that were associated with it at geometry_create  time,all of which
!  * must participate in this operation for it to complete. This is a
!  * data fence,which means that the data movement is complete. This is not an operation
!  * fence,which would need to include active message completion handlers
!  * completing on the target.  A common operation to implement "complete geometry fence"
!  * operation would be for all tasks to call a fence operation on a geometry,
!  * followed by a barrier.
!  *
!  * Any point-to-point transfers issued to one of the endpoints covered by the geometry cannot
!  * start until all point-to-point transfers to the endpoints in the geometry complete.
!  *
!  * *  "Task based" geometry fence will fence point-to-point and collective
!  *    operations to all addressable endpoints for the respective outstanding
!  *    point-to-point and collective operations.
!  *
!  * *  For point-to-point,all communication outstanding endpoints will be fenced
!  *    for the context
!  *
!  * *  For collectives,all collectives pending on the current context will be
!  *    fenced. These collectives only address endpoints on the same context offset
!  *    as the issuing context ("cross talk" does not apply to collectives) on
!  *    all remote nodes.
!  *
!  * *  The user must use the "done function" of the pami_fence to ensure completion
!  *    of outstanding operations,before issuing the next operation.  The user
!  *    can post subsequent operations in the done function.
!  *    This is an example of an ordered,correct fence flow:
!  *    1)  put
!  *    2)  fence --> advance until done callback is called
!  *    3)  get
!  *    Note that the done callback on the fence must have completed to guarantee
!  *    proper data consistency.
!  */
    type pami_fence_t
    end type pami_fence_t


! /**
!  * \brief Create and post a non-blocking active message broadcast operation.
!  * The Active Message broadcast operation ...
!  *
!  * This differs from AMSend in only one particular: it takes geometry/team
!  * as an argument. The semantics are as follows: the included header and data
!  * are broadcast to every place in the team. The completion handler is invoked
!  * on the sender side as soon as send buffers can be reused. On the receive
!  * side the usual two-phase reception protocol is executed: a header handler
!  * determines the address to which to deposit the data and sets the address
!  * of a receive completion hander to be invoked once the data has arrived.
!  *
!  * \param[in]  dispatch     registered dispatch id to use
!  * \param[in]  user_header  single metadata to send to destination in the header
!  * \param[in]  headerlen    length of the metadata (can be 0)
!  * \param[in]  src          Base source buffer to broadcast.
!  * \param[in]  stype        Datatype of the send buffer
!  * \param[in]  stypecount   replication count of the send buffer data type
!  *
!  * \retval     0            Success
!  *
!  * \todo doxygen
!  */
    type pami_ambroadcast_t
        sequence
        integer(4)  :: dispatch
        integer(4)  :: user_header
        integer(4)  :: headerlen
        integer(4)  :: sndbuf
        integer(4)  :: stype
        integer(4)  :: stypecount
    end type pami_ambroadcast_t

! /**
!  * \brief The active message callback function,delivered to the user
!  * \param[in]   root       system defined metadata:  root initiating the broadcast
!  * \param[in]   comm       system defined metadata:  geometry id of the broadcast
!  * \param[in]   sndlen     system defined metadata:  total number of bytes of the send
!  * \param[in]   user_header user defined metadata:  active message header
!  * \param[in]   headerlen  length of the user defined header (may be 0 bytes)
!  * \param[out]  rcvbuf     target buffer for the incoming collective
!  * \param[out]  rtype      data layout of the incoming collective
!  * \param[out]  rtypecount replication count of the incoming collective
!  * \param[out]  cb_info    data done callback to call on completion
!  *
!  * \retval     0            Success
!  *
!  * \todo doxygen
!  */
!  typedef void (*pami_dispatch_ambroadcast_function) (pami_context_t         context,
!                                                      size_t                 root,
!                                                      pami_geometry_t        geometry,
!                                                      const size_t           sndlen,
!                                                      const void           * user_header,
!                                                      const size_t           headerlen,
!                                                      void                ** rcvbuf,
!                                                      pami_type_t          * rtype,
!                                                      size_t               * rtypecount,
!                                                      pami_event_function  * const cb_info,
!                                                      void                ** cookie);


! /**
!  * \brief Create and post a non-blocking active message scatter operation.
!  * The Active Message scatter operation ...
!  *
!  * This is slightly more complicated than an AMBroadcast,because it allows
!  * different headers and data buffers to be sent to everyone in the team.
!  *
!  * \param[in]  dispatch     registered dispatch id to use
!  * \param[in]  headers      array of  metadata to send to destination
!  * \param[in]  headerlength length of every header in the headers array
!  * \param[in]  src          Base source buffer to scatter (size of geometry)
!  * \param[in]  stype        single Datatype of the send buffer
!  * \param[in]  stypecount   replication count of the send buffer data type
!  *
!  * \retval     0            Success
!  *
!  * \todo doxygen
!  */
    type pami_amscatter_t
        sequence
        integer(4)  :: dispatch
        integer(4)  :: headers
        integer(4)  :: headerlen
        integer(4)  :: sndbuf
        integer(4)  :: stype
        integer(4)  :: stypecount
    end type pami_amscatter_t

! /**
!  * \brief The active message callback function,delivered to the user
!  * \param[in]   root       system defined metadata:  root initiating the scatter
!  * \param[in]   comm       system defined metadata:  geometry id of the scatter
!  * \param[in]   sndlen     system defined metadata:  total number of bytes of the send
!  * \param[in]   user_header user defined metadata:  active message header
!  * \param[in]   headerlen  length of the user defined header (may be 0 bytes)
!  * \param[out]  rcvbuf     target buffer for the incoming collective
!  * \param[out]  rtype      data layout of the incoming collective
!  * \param[out]  rtypecount replication count of the incoming collective
!  * \param[out]  cb_info    data done callback to call on completion
!  *
!  * \retval     0            Success
!  *
!  * \todo doxygen
!  */
!  typedef void (*pami_dispatch_amscatter_function) (size_t                 root,
!                                                    pami_geometry_t        geometry,
!                                                    const unsigned         sndlen,
!                                                    const void           * user_header,
!                                                    const size_t           headerlen,
!                                                    void                ** rcvbuf,
!                                                    pami_type_t          * rtype,
!                                                    size_t               * rtypecount,
!                                                    pami_event_function  * const cb_info);

! /**
!  * \brief Create and post a non-blocking active message gather operation.
!  * The Active Message gather operation ...
!  *
!  * This is the reverse of amscatter. It works as follows. The header only,
!  * no data,is broadcast to the team. Each place in the team executes the
!  * header handler and points to a data buffer in local space. A reverse transfer
!  * then takes place (the buffer is sent from the receiver back to the sender,
!  * and deposited in one of the buffers provided as part of the original call
!  * (the "data" parameter).
!  *
!  * \param[in]  dispatch     registered dispatch id to use
!  * \param[in]  headers      array of metadata to send to destination
!  * \param[in]  headerlen    length of every header in headers array
!  * \param[in]  rcvbuf       target buffer of the gather operation (size of geometry)
!  * \param[in]  rtype        data layout of the incoming gather
!  * \param[in]  rtypecount   replication count of the incoming gather
!  * \param[in]  cb_info      data done callback to call on completion
!  *
!  * \retval     0            Success
!  *
!  * \todo doxygen
!  */
    type pami_amgather_t
        sequence
        integer(4)  :: dispatch
        integer(4)  :: headers
        integer(4)  :: headerlen
        integer(4)  :: rcvbuf
        integer(4)  :: rtype
        integer(4)  :: rtypecount
    end type pami_amgather_t

! /**
!  * \brief The active message callback function,delivered to the user
!  * \param[in]   root       system defined metadata:  root initiating the gather
!  * \param[in]   comm       system defined metadata:  geometry id of the gather
!  * \param[in]   sndlen     system defined metadata:  total number of bytes of the send
!  * \param[in]   user_header user defined metadata:  active message header
!  * \param[in]   headerlen  length of the user defined header (may be 0 bytes)
!  * \param[out]  sbuf       source buffer for the incoming collective
!  * \param[out]  stype      data layout of the send buffer
!  * \param[out]  stypecount replication count of the send buffer
!  * \param[out]  cb_info    data done callback to call on completion
!  *
!  * \retval     0            Success
!  *
!  * \todo doxygen
!  */
!  typedef void (*pami_dispatch_amgather_function) (size_t                 root,
!                                                   pami_geometry_t        geometry_id,
!                                                   const unsigned         sndlen,
!                                                   const void           * user_header,
!                                                   const size_t           headerlen,
!                                                   void                ** sndbuf,
!                                                   pami_type_t          * stype,
!                                                   size_t               * stypecount,
!                                                   pami_event_function  * const cb_info);

! /**
!  * \brief Create and post a non-blocking active message reduce operation.
!  * The Active Message reduce operation ...
!  *
!  * This is fairly straightforward given how amgather works. Instead of
!  * collecting the data without processing,all buffers are reduced using the
!  * operation and data type provided by the sender. The final reduced data is
!  * deposited in the original buffer provided by the initiator. On the receive
!  * side the algorithm has the right to change the buffers provided by the header
!  * handler (this may avoid having the implementor allocate more memory for
!  * internal buffering)
!  *
!  * \param[in]  dispatch     registered dispatch id to use
!  * \param[in]  geometry     Geometry to use for this collective operation.
!  *                          \c NULL indicates the global geometry.
!  * \param[in]  headers      metadata to send to destinations in the header
!  * \param[in]  rcvbuf       target buffer of the reduce operation (size of geometry)
!  * \param[in]  rtype        data layout of the incoming reduce
!  * \param[in]  rtypecount   replication count of the incoming reduce
!  * \param[in]  dt           datatype of reduction operation
!  * \param[in]  op           operation type
!  *
!  * \retval     0            Success
!  *
!  * \todo doxygen
!  */
    type pami_amreduce_t
        sequence
        integer(4)  :: dispatch
        integer(4)  :: user_header
        integer(4)  :: headerlen
        integer(4)  :: rcvbuf
        integer(4)  :: rtype
        integer(4)  :: rtypecount
        integer(4)  :: dt
        integer(4)  :: op
    end type pami_amreduce_t

! /**
!  * \brief The active message callback function,delivered to the user
!  *        Note that the user does no math in these routines,just provides the buffer.
!  *        The system provided metadata is information that the underlying messaging stack
!  *        must transmit anyways,so it is delivered to the user for extra information about
!  *        the incoming message
!  * \param[in]   root       system defined metadata:  root initiating the reduce
!  * \param[in]   comm       system defined metadata:  geometry id of the reduce
!  * \param[in]   dt         system defined metadata:  datatype of reduction operation
!  * \param[in]   op         system defined metadata:  operation type
!  * \param[in]   sndlen     system defined metadata:  total number of bytes of the send
!  * \param[in]   user_header user defined metadata:  active message header
!  * \param[in]   headerlen  length of the user defined header (may be 0 bytes)
!  * \param[out]  sbuf       source buffer for the incoming collective
!  * \param[out]  stype      data layout of the send buffer
!  * \param[out]  stypecount replication count of the send buffer
!  * \param[out]  cb_info    data done callback to call on completion
!  *
!  * \retval     0            Success
!  *
!  * \todo doxygen
!  */
!  typedef void (*pami_dispatch_amreduce_function) (size_t                 root,
!                                                   pami_geometry_t        geometry_id,
!                                                   const unsigned         sndlen,
!                                                   pami_dt                dt,
!                                                   pami_op                op,
!                                                   const void           * user_header,
!                                                   const size_t           headerlen,
!                                                   void                ** sndbuf,
!                                                   pami_type_t          * stype,
!                                                   size_t               * stypecount,
!                                                   pami_event_function  * const cb_info);


!  typedef union
!  {
!    pami_allreduce_t        xfer_allreduce;
!    pami_broadcast_t        xfer_broadcast;
!    pami_reduce_t           xfer_reduce;
!    pami_allgather_t        xfer_allgather;
!    pami_allgatherv_t       xfer_allgatherv;
!    pami_allgatherv_int_t   xfer_allgatherv_int;
!    pami_scatter_t          xfer_scatter;
!    pami_scatterv_t         xfer_scatterv;
!    pami_scatterv_int_t     xfer_scatterv_int;
!    pami_gather_t           xfer_gather;
!    pami_gatherv_t          xfer_gatherv;
!    pami_gatherv_int_t      xfer_gatherv_int;
!    pami_alltoall_t         xfer_alltoall;
!    pami_alltoallv_t        xfer_alltoallv;
!    pami_alltoallv_int_t    xfer_alltoallv_int;
!    pami_ambroadcast_t      xfer_ambroadcast;
!    pami_amscatter_t        xfer_amscatter;
!    pami_amgather_t         xfer_amgather;
!    pami_amreduce_t         xfer_amreduce;
!    pami_scan_t             xfer_scan;
!    pami_barrier_t          xfer_barrier;
!    pami_fence_t            xfer_fence;
!    pami_reduce_scatter_t   xfer_reduce_scatter;
!  } pami_collective_t;

    type pami_xfer_t
        sequence
        integer(4)  :: cb_done
        integer(4)  :: cookie
        integer(4)  :: algorithm
        integer(4)  :: options
        integer(4)  :: cmd(8)
    end type pami_xfer_t

!  pami_result_t PAMI_Collective (pami_context_t context,pami_xfer_t *cmd);

! /*****************************************************************************/
! /**
!  * \defgroup datatype pami datatype interface
!  *
!  * Some brief documentation on datatype stuff ...
!  * \{
!  */
! /*****************************************************************************/

! /**
!  * \brief A PAMI Datatype that represents a contigous data layout
!  *
!  *  This is a contiguous type object that does not need to be
!  *  explicitly created using PAMI_Type_create.  It can be used
!  *  in transfers where a pami_type_t is accepted to specify contiguous
!  *  bytes.  This can be used for both collective and point-to-point
!  *  communication.
!  */
!  extern pami_type_t PAMI_BYTE;
! /**
!  * \brief Create a new type for noncontiguous transfers
!  *
!  * \todo provide example code
!  *
!  * \param[out] type Type identifier to be created
!  *
!  * \retval PAMI_SUCCESS  The type is created.
!  * \retval PAMI_ENOMEM   Out of memory.
!  */
!  pami_result_t PAMI_Type_create (pami_type_t* type);

! /**
!  * \brief Append a simple contiguous buffer to an existing type identifier
!  *
!  * \todo doxygen for offset parameter
!  * \todo provide example code
!  *
!  * \param[in,out] type   Type identifier to be modified
!  * \param[in]     bytes  Number of contiguous bytes to append
!  * \param[in]     offset Offset from the end of the type to place the buffer
!  * \param[in]     count  Number of buffers
!  * \param[in]     stride Data stride
!  *
!  * \retval PAMI_SUCCESS  The buffer is added to the type.
!  * \retval PAMI_INVAL    The type is complete.
!  * \retval PAMI_ENOMEM   Out of memory.
!  */
!  pami_result_t PAMI_Type_add_simple (pami_type_t type,
!                                      size_t      bytes,
!                                      size_t      offset,
!                                      size_t      count,
!                                      size_t      stride);

! /**
!  * \brief Append a typed buffer to an existing type identifier
!  *
!  * \todo doxygen for offset parameter
!  * \todo provide example code
!  *
!  * \warning It is considered \b illegal to append an incomplete type to
!  *          another type.
!  *
!  * \param[in,out] type    Type identifier to be modified
!  * \param[in]     subtype Subtype to append
!  * \param[in]     offset  Offset from the end of the type to place the buffer
!  * \param[in]     count   Number of buffers
!  * \param[in]     stride  Data stride
!  *
!  * \retval PAMI_SUCCESS  The subtype is added to the type.
!  * \retval PAMI_INVAL    The type is complete or the subtype is incomplete.
!  * \retval PAMI_ENOMEM   Out of memory.
!  */
!  pami_result_t PAMI_Type_add_typed (pami_type_t type,
!                                     pami_type_t subtype,
!                                     size_t      offset,
!                                     size_t      count,
!                                     size_t      stride);

! /**
!  * \brief Complete the type identifier
!  *
!  * The atom size of a type must divide the size of any contiguous buffer
!  * that's described by the type. An atom size of one is valid for any type.
!  *
!  * \warning It is considered \b illegal to modify a type layout after it
!  *          has been completed.
!  *
!  * \param[in] type       Type identifier to be completed
!  * \param[in] atom_size  Atom size of the type
!  *
!  * \retval PAMI_SUCCESS  The type is complete.
!  * \retval PAMI_INVAL    The atom size in invalid.
!  */
!  pami_result_t PAMI_Type_complete (pami_type_t type, size_t atom_size);

! /**
!  * \brief Destroy the type
!  *
!  * The type handle will be changed to an invalid value so that it is
!  * clearly destroyed.
!  *
!  * \param[in] type Type identifier to be destroyed
!  */
!  pami_result_t PAMI_Type_destroy (pami_type_t * type);

! /**
!  * \brief Serialize a type
!  *
!  * Serialize a type and retrieve the address and the size of a serialized
!  * type object,which can be copied or transferred like normal data.
!  * A serialized type object can be reconstructed into a type with
!  * \c PAMI_Type_deserialize.
!  *
!  * The serialization is internal to PAMI and not into user-allocated
!  * memory. Serializing an already-serialized type retrieves the address
!  * and the size of the serialized type object.
!  *
!  * \note A PAMI implementation can choose to keep the internal representation
!  * of a type always serialized. Otherwise,it needs to handle serialization
!  * while a type is in use.
!  *
!  * \param[in]  type      Type identifier to be serialized
!  * \param[out] address   Address of the serialized type object
!  * \param[out] size      Size of the serialized type object
!  *
!  * \retval PAMI_SUCCESS  The serialization is successful.
!  * \retval PAMI_INVAL    The type is invalid.
!  * \retval PAMI_ENOMEM   Out of memory.
!  */
!  pami_result_t PAMI_Type_serialize (pami_type_t type, void** address, size_t* size);

! /**
!  * \brief Reconstruct a new type from a serialized type object
!  *
!  * Successful reconstruction completes the new type and the new type does
!  * not depend on the memory of the serialized type object.
!  *
!  * A reconstructed type can be destroyed by \c PAMI_Type_destroy.
!  *
!  * \param[out] type      Type identifier to be created
!  * \param[in]  address   Address of the serialized type object
!  * \param[in]  size      Size of the serialized type object
!  *
!  * \retval PAMI_SUCCESS  The reconstruction is successful.
!  * \retval PAMI_INVAL    The serialized type object is corrupted.
!  * \retval PAMI_ENOMEM   Out of memory.
!  */
!  pami_result_t PAMI_Type_deserialize (pami_type_t* type, void* address, size_t size);

! /**
!  * \brief Query the attributes of a type
!  *
!  * The type being queried must have completed.
!  *
!  * \param [in] type           The type to query
!  * \param [in] configuration  The configuration attributes to query
!  * \param [in] num_configs    The number of configuration elements
!  *
!  * \retval PAMI_SUCCESS  The update has completed successfully.
!  * \retval PAMI_INVAL    The update has failed due to invalid parameters.
!  */
!  pami_result_t PAMI_Type_query (pami_type_t           type,
!                                 pami_configuration_t  configuration[],
!                                 size_t                num_configs);

! /**
!  * \brief Transform typed data between buffers in the same address space
!  *
!  * \param [in] src_addr       Source buffer address
!  * \param [in] src_type       Source data type
!  * \param [in] src_offset     Starting offset of source data type
!  * \param [in] dst_addr       Destination buffer address
!  * \param [in] dst_type       Destination data type
!  * \param [in] dst_offset     Starting offset of destination data type
!  * \param [in] size           Amount of data to transform
!  * \param [in] data_fn        Function to transform the data
!  * \param [in] cookie         Argument to data function
!  *
!  * \retval PAMI_SUCCESS  The operation has completed successfully.
!  * \retval PAMI_INVAL    The operation has failed due to invalid parameters,
!  *                       e.g. incomplete types.
!  */
!  pami_result_t PAMI_Type_transform_data (void               * src_addr,
!                                          pami_type_t          src_type,
!                                          size_t               src_offset,
!                                          void               * dst_addr,
!                                          pami_type_t          dst_type,
!                                          size_t               dst_offset,
!                                          size_t               size,
!                                          pami_data_function   data_fn,
!                                          void               * cookie);

! /** \} */ /* end of "datatype" group */

!  typedef union
!  {
!    pami_dispatch_p2p_function   p2p;
!    pami_dispatch_ambroadcast_function ambroadcast;
!    pami_dispatch_amscatter_function amscatter;
!    pami_dispatch_amreduce_function amreduce;
!  } pami_dispatch_callback_function;

! /*****************************************************************************/
! /**
!  * \defgroup dispatch pami dispatch interface
!  *
!  * Some brief documentation on dispatch stuff ...
!  * \{
!  */
! /*****************************************************************************/

! /**
!  * \brief PAMI type of dispatch
!  */
    integer PAMI_P2P_SEND
    parameter (PAMI_P2P_SEND=0)

! /**
!  * \brief Hints for dispatch
!  *
!  */
    type pami_dispatch_hint_t
        sequence
        integer(4)  :: type     ! /**< Type of dispatch reqistered    */
        integer(4)  :: send(9)  ! /**< Type-specific hints            */
        integer(4)  :: config   ! /**< Type-specific additional config*/
    end type pami_dispatch_hint_t

! /**
!  * \brief Initialize the dispatch function for a dispatch identifier.
!  *
!  * This is a local,non-collective operation. There is no communication
!  * between tasks.
!  *
!  * It is \b illegal for the user to specify different hint assertions for the
!  * same client,context offset,and dispatch identifier on different tasks.
!  * However,there is no specific error check that will prevent specifying
!  * different hint assertions. The result of a communication operation using
!  * mismatched hint assertions is \em undefined.
!  *
!  * \note The maximum allowed dispatch identifier attribute,
!  *       \c PAMI_CONTEXT_DISPATCH_ID_MAX,can be queried with the
!  *       configuration interface
!  *
!  * \see PAMI_Context_query
!  *
!  * \param[in] context    PAMI communication context
!  * \param[in] dispatch   Dispatch identifier to initialize
!  * \param[in] fn         Dispatch receive function
!  * \param[in] cookie     Dispatch function cookie
!  * \param[in] options    Dispatch registration assertions
!  */
!  pami_result_t PAMI_Dispatch_set (pami_context_t              context,
!                                   size_t                      dispatch,
!                                   pami_dispatch_callback_function fn,
!                                   void                      * cookie,
!                                   pami_send_hint_t            options);

! /**
!  * \brief Initialize the dispatch functions for a dispatch id.
!  *
!  * This is a local,non-collective operation. There is no communication
!  * between tasks.
!  *
!  * \param[in] context    PAMI communication context
!  * \param[in] algorithm  The AM collective to set the dispatch
!  * \param[in] dispatch   Dispatch identifier to initialize
!  * \param[in] fn         Dispatch receive function
!  * \param[in] cookie     Dispatch function cookie
!  * \param[in] options    Dispatch registration assertions
!  *
!  */
!  pami_result_t PAMI_AMCollective_dispatch_set(pami_context_t             context,
!                                               pami_algorithm_t           algorithm,
!                                               size_t                     dispatch,
!                                               pami_dispatch_callback_function fn,
!                                               void                     * cookie,
!                                               pami_collective_hint_t      options);

! /** \} */ /* end of "dispatch" group */

! /**
!  * \brief Query the value of an attribute
!  *
!  * \param [in]     dispatch       The PAMI dispatch
!  * \param [in,out] configuration  The configuration attribute of interest
!  * \param [in]     num_configs    The number of configuration elements
!  *
!  * \retval PAMI_SUCCESS  The query has completed successfully.
!  * \retval PAMI_INVAL    The query has failed due to invalid parameters.
!  */
!  pami_result_t PAMI_Dispatch_query (pami_context_t        context,
!                                     size_t                dispatch,
!                                     pami_configuration_t  configuration[],
!                                     size_t                num_configs);

! /**
!  * \brief Update the value of an attribute
!  *
!  * \param [in] dispatch       The PAMI dispatch
!  * \param [in] configuration  The configuration attribute to update
!  * \param [in] num_configs    The number of configuration elements
!  *
!  * \retval PAMI_SUCCESS  The update has completed successfully.
!  * \retval PAMI_INVAL    The update has failed due to invalid parameters.
!  *                       For example,trying to update a read-only attribute.
!  */
!  pami_result_t PAMI_Dispatch_update (pami_context_t        context,
!                                      size_t                dispatch,
!                                      pami_configuration_t  configuration[],
!                                      size_t                num_configs);

! /**
!  * \brief Query the value of an attribute
!  *
!  * \param [in]     client         The PAMI client
!  * \param [in,out] configuration  The configuration attribute of interest
!  * \param [in]     num_configs    The number of configuration elements
!  *
!  * \retval PAMI_SUCCESS  The query has completed successfully.
!  * \retval PAMI_INVAL    The query has failed due to invalid parameters.
!  */
!  pami_result_t PAMI_Client_query (pami_client_t         client,
!                                   pami_configuration_t  configuration[],
!                                   size_t                num_configs);

! /**
!  * \brief Update the value of an attribute
!  *
!  * \param [in] client         The PAMI client
!  * \param [in] configuration  The configuration attribute to update
!  * \param [in] num_configs    The number of configuration elements
!  *
!  * \retval PAMI_SUCCESS  The update has completed successfully.
!  * \retval PAMI_INVAL    The update has failed due to invalid parameters.
!  *                       For example,trying to update a read-only attribute.
!  */
!  pami_result_t PAMI_Client_update (pami_client_t         client,
!                                    pami_configuration_t  configuration[],
!                                    size_t                num_configs);

!    /**
!  * \brief Query the value of an attribute
!  *
!  * \param [in]     context        The PAMI context
!  * \param [in,out] configuration  The configuration attribute of interest
!  * \param [in]     num_configs    The number of configuration elements
!  *
!  * \retval PAMI_SUCCESS  The query has completed successfully.
!  * \retval PAMI_INVAL    The query has failed due to invalid parameters.
!  */
!  pami_result_t PAMI_Context_query (pami_context_t        context,
!                                    pami_configuration_t  configuration[],
!                                    size_t                num_configs);

! /**
!  * \brief Update the value of an attribute
!  *
!  * \param [in] context        The PAMI context
!  * \param [in] configuration  The configuration attribute to update
!  * \param [in] num_configs    The number of configuration elements
!  *
!  * \retval PAMI_SUCCESS  The update has completed successfully.
!  * \retval PAMI_INVAL    The update has failed due to invalid parameters.
!  *                       For example,trying to update a read-only attribute.
!  */
!  pami_result_t PAMI_Context_update (pami_context_t        context,
!                                     pami_configuration_t  configuration[],
!                                     size_t                num_configs);

! /**
!  * \brief Query the value of an attribute
!  *
!  * \param [in]     geometry       The PAMI geometry
!  * \param [in,out] configuration  The configuration attribute of interest
!  * \param [in]     num_configs    The number of configuration elements
!  *
!  * \retval PAMI_SUCCESS  The query has completed successfully.
!  * \retval PAMI_INVAL    The query has failed due to invalid parameters.
!  */
!  pami_result_t PAMI_Geometry_query (pami_geometry_t       geometry,
!                                     pami_configuration_t  configuration[],
!                                     size_t                num_configs);

! /**
!  * \brief Update the value of an attribute
!  *
!  * WARNING - Changing a Geometry configuration attribute may fundamentally
!  * alter the Geometry. Any saved knownledge (for example,algorithm lists)
!  * must be discarded and re-queried after a call to PAMI_Geometry_update().
!  *
!  * \param [in] geometry      The PAMI geometry
!  * \param [in] configuration The configuration attribute to update
!  * \param [in] num_configs   The number of configuration elements
!  * \param[in]  context       context to deliver async callback to
!  * \param[in]  fn            event function to call when geometry has been created
!  * \param[in]  cookie        user cookie to deliver with the callback
!  *
!  * \note This is a collective call,and the configuration variable
!  *       must be set collectively
!  *
!  * \retval PAMI_SUCCESS  The update has completed successfully.
!  * \retval PAMI_INVAL    The update has failed due to invalid parameters.
!  *                       For example,trying to update a read-only attribute.
!  */
!  pami_result_t PAMI_Geometry_update (pami_geometry_t       geometry,
!                                      pami_configuration_t  configuration[],
!                                      size_t                num_configs,
!                                      pami_context_t        context,
!                                      pami_event_function   fn,
!                                      void                 *cookie);


! /**
!  * \brief Provides the detailed description of the most recent pami result.
!  *
!  * The "most recent pami result" is specific to each thread.
!  *
!  * \note  PAMI implementations may provide translated (i18n) text.
!  *
!  * \param[in] string Character array to write the descriptive text
!  * \param[in] length Length of the character array
!  *
!  * \return Number of characters written into the array
!  */
! size_t PAMI_Error_text (char * string,size_t length);


! /*****************************************************************************/
! /**
!  * \defgroup Time Timer functions required by MPI
!  *
!  * \{
!  */
! /*****************************************************************************/

! /**
!  * \brief  Returns an elapsed time on the calling processor.
!  * \note   This has the same definition as MPI_Wtime
!  * \return Time in seconds since an arbitrary time in the past.
!  */
! double PAMI_Wtime();

! /**
!  * \brief  Returns the number of "cycles" elapsed on the calling processor.
!  * \return Number of "cycles" since an arbitrary time in the past.
!  *
!  * "Cycles" could be any quickly and continuously increasing counter
!  * if true cycles are unavailable.
!  */
! unsigned long long PAMI_Wtimebase();

! /** \} */ /* end of "Time" group */


! /*****************************************************************************/
! /**
!  * \defgroup contexts_and_endpoints multi-context messaging interface
!  *
!  * Some brief documentation on context stuff ...
!  * \{
!  */
! /*****************************************************************************/

! /**
!  * \brief Initialize the PAMI runtime for a client program
!  *
!  * An PAMI client represents a collection of resources to enable network
!  * communications. Each PAMI client that is initialized is unque and does not
!  * directly communicate with other clients. This allows middleware to be
!  * developed independently and each middleware can be used concurrently by an
!  * application. Resources are allocated and assigned at client creation time.
!  *
!  * An PAMI client \em program is any software that invokes an PAMI function.
!  * This includes applications,libraries,and other middleware. Some example
!  * client names may include: "MPI","UPC","OpenSHMEM",and "ARMCI"
!  *
!  * \note Client creation may be a synchronizing event,but is not required
!  *       to be implemented as a synchonizing event. Application code must
!  *       not make any assumption about synchronization during client
!  *       creation,and therefore must create clients in the same order in
!  *       all processes of the job.
!  *
!  * A communication context must be created before any data transfer functions
!  * may be invoked.
!  *
!  * \see PAMI_Context_createv
!  *
!  * \param[in]  name           PAMI client unique name
!  * \param[out] client         Opaque client object
!  * \param[in]  configuration  objects for the client
!  * \param[in]  num_configs    The number of configuration elements
!  *
!  * \retval PAMI_SUCCESS  The client has been successfully created.
!  * \retval PAMI_INVAL    The client name has been rejected by the runtime.
!  *                       It happens when a job scheduler requires the client
!  *                       name to match what's in the job description.
!  */
!  pami_result_t PAMI_Client_create (const char           *name,
!                                    pami_client_t        *client,
!                                    pami_configuration_t  configuration[],
!                                    size_t                num_configs);

! /**
!  * \brief Finalize the PAMI runtime for a client program
!  *
!  * \warning It is \b illegal to invoke any PAMI functions using the client
!  *          handle from any thread after the finalize function.
!  *
!  * \param[in] client PAMI client handle
!  * \retval PAMI_SUCCESS  The client has been successfully destroyed.
!  * \retval PAMI_INVAL    The client is invalid,e.g. already destroyed.
!  *
!  * The client handle will be changed to an invalid value so that it
!  * is clearly destroyed.
!  */
!  pami_result_t PAMI_Client_destroy (pami_client_t * client);

! /**
!  * \brief Construct an endpoint to address communication destinations
!  *
!  * Endpoints are opaque objects that are used to address a destination
!  * in a client and are constructed from a client,task,and context offset.
!  * - The client is required to disambiguate the task and context offset
!  *   identifiers,as these identifiers may be the same for multiple clients
!  * - The task is required to construct an endpoint to address the specific
!  *   process that contains the destination context
!  * - The context offset is required to identify the specific context on the
!  *   destination task. Recall that a context identifies a specific threading
!  *   point on a task. The context offset identifies which threading point
!  *   will process the communication operation.
!  *
!  * Point-to-point communication operations,such as send,put,and get,will
!  * address a destination with the opaque endpoint object. Collective
!  * communication operations are addressed by an opaque geometry object.
!  *
!  * The application may choose to write an endpoint table in shared memory to
!  * save storage in an environment where multiple tasks of a client have
!  * access to the same shared memory area  It is the responsibility of the
!  * application to allocate this shared memory area and coordinate the
!  * initialization and access of any shared data structures. This includes
!  * any opaque endpoint objects which may be created by one task and read by
!  * another task.
!  *
!  * \internal The endpoint opaque object should not contain any pointers to
!  *           the local address space of a particular process,as doing so will
!  *           prevent the application from placing an array of endpoints in a
!  *           shared memory area to be used,read-only,by all tasks with
!  *           access to the shared memory area.
!  *
!  * \note This function may be replaced with a generated macro specific to the
!  *       install platform if needed for performance reasons.
!  *
!  * \param[in]  client   Opaque destination client object
!  * \param[in]  task     Opaque destination task object
!  * \param[in]  offset   Destination context offset
!  * \param[out] endpoint Opaque endpoint object
!  */
!  pami_result_t PAMI_Endpoint_create (pami_client_t     client,
!                                      pami_task_t       task,
!                                      size_t            offset,
!                                      pami_endpoint_t * endpoint);

! /**
!  * \brief Construct an endpoint for each context in a task
!  *
!  * This function may be used to initialize an array of all endpoints for a
!  * client by looping over all tasks.
!  *
!  * \param[in]     client    Opaque destination client object
!  * \param[in]     task      Opaque destination task object
!  * \param[out]    endpoints Array of opaque endpoint objects to initialize
!  * \param[in,out] count     Length of the endpoint array available/initialized
!  *
!  * \retval PAMI_INVAL The number endpoints on the task is larger than the
!  *                    size of endpoint array
!  */
!  pami_result_t PAMI_Endpoint_createv (pami_client_t     client,
!                                       pami_task_t       task,
!                                       pami_endpoint_t * endpoints,
!                                       size_t          * count);

! /**
!  * \brief Retrieve the client,task,and context offset associated with an endpoint
!  *
!  * The endpoint must have been previously initialized with the
!  * PAMI_Endpoint_create or PAMI_Endpoint_createv function.
!  *
!  * \note This function may be replaced with a generated macro specific to the
!  *       install platform if needed for performance reasons.
!  *
!  * \param[in]  endpoint Opaque endpoint object
!  * \param[out] task     Opaque destination task object
!  * \param[out] offset   Destination context offset
!  */
!  pami_result_t PAMI_Endpoint_query (pami_endpoint_t   endpoint,
!                                     pami_task_t     * task,
!                                     size_t          * offset);



! /**
!  * \brief Create new independent communication contexts for a client
!  *
!  * Contexts are local "threading points" that an application may use to
!  * optimize concurrent communcation operations. A context handle is an
!  * opaque object type that the application must not directly read or write
!  * the value of the object.
!  *
!  * Communication contexts have these features:
!  * - Each context is a partition of the local resources assigned to the
!  *   client object for each task
!  * - Every context within a client has equivalent functionality and
!  *   semantics
!  * - Communcation operations initiated by the local task will use the
!  *   opaque context object to identify the specific threading point that
!  *   will be used to issue the communication independent of communication
!  *   occuring in other contexts
!  * - All local event callbacks(s) associated with a communication operation
!  *   will be invoked by the thread which advances the context that was used
!  *   to initiate the operation
!  * - A context is a local object and is not used to directly address a
!  *   communication destination
!  * - Progress is driven independently among contexts
!  * - Progress may be driven concurrently among contexts,by using multiple
!  *   threads,as desired by the application
!  * - <b>All contexts created by a client must be advanced by the application
!  *   to prevent deadlocks.  This is the "all advance" rule</b>
!  * - The rationale for the "all-advance" rule is that for a point-to-point
!  *   send or a collective operation,a communication is posted to a
!  *   context,and delivered to a context on a remote task. The internals
!  *   of the messaging layer could implement "horizontal" parallelism by
!  *   injecting data or processing across multiple contexts associated with
!  *   the client. Consequently,data can be received across multiple contexts.
!  *   To guarantee progress of a single operation,every context must be advanced
!  *   by the user.
!  * - The user application/client of pami may have more knowledge about
!  *   the communication patterns and the "all advance" rule can be relaxed.
!  *   To do this the user can specify special hints to disable "horizontal",
!  *   or cross context parallelism. Refer to pami_send_hint_t,and the
!  *   "multicontext" option.  This option must be switched "off" to disable
!  *   parallelization and the "all advance rule".
!  * - The task based geometry constructor implies all contexts are included
!  *   in the geometry,with a single participant per task. All contexts must
!  *   be advanced during a collective operation.  However,the user can specify
!  *   special hints to disable "horizontal",or cross context parallelsm.
!  *   Refer to pami_collective_hint_t,and the
!  *   "multicontext" option.  This option must be switched "off" to disable
!  *   parallelization and the "all advance rule".
!  *
!  * \par Thread considerations
!  *       Applications map,or "apply",threading resources to contexts.
!  *       Operations on contexts are critical sections and not thread-safe.
!  *       The application must ensure that critical sections are protected
!  *       from re-entrant use. PAMI provides mechanisms for controlling access
!  *       to critical sections
!  *
!  * The context configuration attributes may include:
!  * - Context optimizations,such as shared memory,collective acceleration,etc
!  *
!  * Context creation is a local operation and does not involve communication or
!  * syncronization with other tasks.
!  *
!  * \param[in]  client        Client handle
!  * \param[in]  configuration List of configurable attributes and values
!  * \param[in]  num_configs   Number of configurations,may be zero
!  * \param[out] context       Array of communication contexts to initialize
!  * \param[in]  ncontexts     Number of contexts to be created
!  *
!  * \retval PAMI_SUCCESS  Contexts have been created.
!  * \retval PAMI_INVAL    Configuration could not be satisified or there were
!  *                       errors in other parameters.
!  */
!  pami_result_t PAMI_Context_createv (pami_client_t          client,
!                                      pami_configuration_t   configuration[],
!                                      size_t                 num_configs,
!                                      pami_context_t       * context,
!                                      size_t                 ncontexts);


! /**
!  * \brief Destroy the communication context
!  *
!  * \warning It is \b illegal to invoke any PAMI functions using the
!  *          communication context from any thread after the context is
!  *          destroyed.
!  *
!  * \param[in,out] contexts  PAMI communication context list
!  * \param[in]     ncontexts The number of contexts in the list.
!  * \retval PAMI_SUCCESS  The contexts have been destroyed.
!  * \retval PAMI_INVAL    Some context is invalid,e.g. already destroyed.
!  *
!  * The context handles will be changed to an invalid value so that
!  * they are clearly destroyed.
!  */
!  pami_result_t PAMI_Context_destroyv (pami_context_t * contexts,
!                                       size_t           ncontexts);

! /**
!  * \brief Post work to a context,thread-safe
!  *
!  * It is \b not required that the target context is locked,or otherwise
!  * reserved,by an external atomic operation to ensure thread safety. The PAMI
!  * runtime will internally perform any neccessary atomic operations in order
!  * to post the work to the context.
!  *
!  * The callback function will be invoked in the thread that advances the
!  * \em work context. There is no implicit completion notification provided
!  * to the \em posting thread when the thread advancing the \em work context
!  * returns from the callback event function.  If the posting thread desires
!  * a completion notification it must explicitly program such notifications,
!  * via the PAMI_Context_post() interface,from the target thread back to the
!  * origin thread
!  *
!  * \todo Needs some opaque storage to enqueue on to the work queue. This is
!  *       neccesary to improve the performance for the MMPS benchmark. In
!  *       other words,latency may degrade if the internal implementation
!  *       must allocate memory.
!  *
!  * \param[in] context PAMI communication context
!  * \param[in] work_fn Event callback function to post to the context
!  * \param[in] cookie  Opaque data pointer to pass to the event function
!  *
!  * \retval PAMI_SUCCESS  The work has been posted.
!  * \retval PAMI_INVAL    There were errors in the parameters.
!  */
!  pami_result_t PAMI_Context_post (pami_context_t       context,
!                                   pami_work_t        * work,
!                                   pami_work_function   fn,
!                                   void               * cookie);


! /**
!  * \brief Advance the progress engine for a single communication context
!  *
!  * May complete zero,one,or more outbound transfers. May invoke dispatch
!  * handlers for incoming transfers. May invoke work event callbacks previously
!  * posted to the communication context.
!  *
!  * This polling advance function will return after the first poll iteration
!  * that results in a processed event or if,no events are processed,after
!  * polling for the maximum number of iterations.
!  *
!  * \warning This function is \b not \b threadsafe and the application must
!  *          ensure that only one thread advances a context at any time.
!  *
!  * \see PAMI_Context_lock
!  * \see PAMI_Context_trylock
!  *
!  * \todo Define return code
!  *
!  * \param[in] context PAMI communication context
!  * \param[in] maximum Maximum number of internal poll iterations
!  *
!  * \retval PAMI_SUCCESS  An event has occurred and been processed.
!  * \retval PAMI_EAGAIN   No event has occurred.
!  */
!  pami_result_t PAMI_Context_advance (pami_context_t context,size_t maximum);

! /**
!  * \brief Advance the progress engine for multiple communication contexts
!  *
!  * May complete zero,one,or more outbound transfers. May invoke dispatch
!  * handlers for incoming transfers. May invoke work event callbacks previously
!  * posted to a communication context.
!  *
!  * This polling advance function will return after the first poll iteration
!  * that results in a processed event on any context,or if,no events are
!  * processed,after polling for the maximum number of iterations.
!  *
!  * \warning This function is \b not \b threadsafe and the application must
!  *          ensure that only one thread advances the contexts at any time.
!  *
!  * \note It is possible to define a set of communication contexts that are
!  *       always advanced together by any pami client thread.  It is the
!  *       responsibility of the pami client to atomically lock the context set,
!  *       perhaps by using the PAMI_Context_lock() function on a designated
!  *       \em leader context,and to manage the pami client threads to ensure
!  *       that only one thread ever advances the set of contexts.
!  *
!  * \todo Define return code
!  *
!  * \see PAMI_Context_lock
!  * \see PAMI_Context_trylock
!  *
!  * \param[in] context Array of PAMI communication contexts
!  * \param[in] count   Number of communication contexts
!  * \param[in] maximum Maximum number of internal poll iterations on each context
!  * \retval PAMI_SUCCESS  An event has occurred and been processed.
!  * \retval PAMI_EAGAIN   No event has occurred.
!  */
!  pami_result_t PAMI_Context_advancev (pami_context_t context[],
!                                       size_t        count,
!                                       size_t        maximum);

! /**
!  * \brief Acquire an atomic lock on a communication context
!  *
!  * \warning This function will block until the lock is aquired.
!  *
!  * \param[in] context PAMI communication context
!  *
!  * \retval PAMI_SUCCESS  The lock has been acquired.
!  * \retval PAMI_INVAL    The context is invalid.
!  */
!  pami_result_t PAMI_Context_lock (pami_context_t context);

! /**
!  * \brief Attempt to acquire an atomic lock on a communication context
!  *
!  * \param[in] context PAMI communication context
!  *
!  * \retval PAMI_SUCCESS  The lock has been acquired.
!  * \retval PAMI_EAGAIN   The lock has not been acquired. Try again later.
!  * \retval PAMI_INVAL    The context is invalid.
!  */
!  pami_result_t PAMI_Context_trylock (pami_context_t context);

! /**
!  * \brief Release an atomic lock on a communication context
!  *
!  * \param[in] context PAMI communication context
!  *
!  * \retval PAMI_SUCCESS  The lock has been released.
!  * \retval PAMI_INVAL    The context is invalid.
!  */
!  pami_result_t PAMI_Context_unlock (pami_context_t context);

! /** \} */ /* end of "context" group */

! /*****************************************************************************/
! /**
!  * \defgroup extensions PAMI Extensions
!  *
!  * A PAMI extension may contain one or more extended functions,using the
!  * PAMI_Extension_function() interface.
!  *
!  * For example,extension-specific functions may provide a mechanism to query
!  * and set configuration attributes,similar to PAMI_Context_query() and
!  * PAMI_Context_update(). The extension would define new extension-specific
!  * configuration functions,structs,and enums to accomplish this.
!  *
!  * Refer to the extension-specific documentation for more information on a
!  * particular extension.
!  *
!  * Refer to the \ref pami_extension_framework for more information on how to
!  * define,implement,and register a new extension.
!  * \{
!  */
! /*****************************************************************************/
!
!  typedef void * pami_extension_t;

    integer PAMI_EXTENSION_T
    parameter (PAMI_EXTENSION_T=4)

! /**
!  * \brief Open an extension for use by a client
!  *
!  * The extension may also be queried during the compile pre-processing using
!  * an \c ##ifdef of the form \c __pami_extension_{name}__
!  *
!  * \code
!  * #ifdef __pami_extension_1234__
!  *
!  * // Use the "1234" extension
!  *
!  * #endif
!  * \endcode
!  *
!  * \param [in]  client    Client handle
!  * \param [in]  name      Unique extension name
!  * \param [out] extension Extension handle
!  *
!  * \retval PAMI_SUCCESS The named extension is available and implemented by the PAMI runtime.
!  * \retval PAMI_UNIMPL  The named extension is not implemented by the PAMI runtime.
!  * \retval PAMI_ERROR   The named extension was not initialized by the PAMI runtime.
!  */
!  pami_result_t PAMI_Extension_open (pami_client_t      client,
!                                     const char       * name,
!                                     pami_extension_t * extension);

! /**
!  * \brief Close an extension
!  *
!  * \param [in] extension Extension handle
!  */
!  pami_result_t PAMI_Extension_close (pami_extension_t extension);

! /**
!  * \brief Query an extension function
!  *
!  * If the named extension is available and implemented by the PAMI runtime
!  * the function pointer to the extension function is returned. This
!  * function pointer can be used to invoke the extension operation.
!  *
!  * \code
!  * typedef void (*pami_extension_1234_foo_fn) (pami_context_t context,size_t foo);
!  * typedef void (*pami_extension_1234_bar_fn) (pami_context_t context,struct iovec ** iov);
!  *
!  * pami_extension_1234_foo_fn pami_1234_foo =
!  *   (pami_extension_1234_foo_fn) PAMI_Extension_function ("pami_extension_1234","foo");
!  * pami_extension_1234_bar_fn pami_1234_bar =
!  *   (pami_extension_1234_bar_fn) PAMI_Extension_function ("pami_extension_1234","bar");
!  *
!  * pami_context_t context = ...;
!  * pami_extension_1234_foo (context,0);
!  *
!  * struct iovec iov[1024];
!  * pami_extension_1234_bar (context,&iov);
!  * \endcode
!  *
!  * \param [in] extension Extension handle
!  * \param [in] fn        Extension function name
!  *
!  * \retval NULL Request PAMI extension is not available
!  * \return PAMI extension function pointer
!  */
!  void * PAMI_Extension_function (pami_extension_t extension,const char * fn);

! /** \} */ /* end of "extensions" group */