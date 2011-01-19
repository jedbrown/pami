!
!   \file sys/fort_inc/pamif.h
!   \brief Common external 32-bit FORTRAN interface for IBM's PAMI message layer.
!

external PAMI_ADDR_NULL

!
!   \brief PAMI result status codes
!
    integer PAMI_SUCCESS
    integer PAMI_NERROR
    integer PAMI_ERROR
    integer PAMI_INVAL
    integer PAMI_UNIMPL
    integer PAMI_EAGAIN
    integer PAMI_ENOMEM
    integer PAMI_SHUTDOWN
    integer PAMI_CHECK_ERRNO
    integer PAMI_OTHER
    integer PAMI_RESULT_EXT
    parameter (PAMI_SUCCESS=0)
    parameter (PAMI_NERROR=-1)
    parameter (PAMI_ERROR=1)
    parameter (PAMI_INVAL=2)
    parameter (PAMI_UNIMPL=3)
    parameter (PAMI_EAGAIN=4)
    parameter (PAMI_ENOMEM=5)
    parameter (PAMI_SHUTDOWN=6)
    parameter (PAMI_CHECK_ERRNO=7)
    parameter (PAMI_OTHER=8)
    parameter (PAMI_RESULT_EXT=1000)

!    
!   \brief The sizes (in byte) of PAMI basic types
!
    integer PAMI_CLIENT_T
    integer PAMI_CONTEXT_T
    integer PAMI_TYPE_T
    integer PAMI_TASK_T
    integer PAMI_ENDPOINT_T
    integer PAMI_USER_KEY_T
    integer PAMI_USER_VALUE_T
    integer PAMI_CLIENT_MEMREGION_SIZE_STATIC ! //TODO: this value differs from BG and power
    integer PAMI_GEOMETRY_T
    integer PAMI_ALGORITHM_T
    integer PAMI_DT_T
    integer PAMI_OP_T
    integer PAMI_SIZE_T
    integer PAMI_VOID_T
    integer PAMI_INT_T
    integer PAMI_DBL_T
    integer PAMI_EXTENSION_T
    ! different for 32/64 bit
    parameter (PAMI_CLIENT_T=4)
    parameter (PAMI_CONTEXT_T=4)
    parameter (PAMI_TYPE_T=4)
    parameter (PAMI_USER_KEY_T=4)
    parameter (PAMI_USER_VALUE_T=4)
    parameter (PAMI_GEOMETRY_T=4)
    parameter (PAMI_ALGORITHM_T=4)
    parameter (PAMI_SIZE_T=4)
    parameter (PAMI_VOID_T=4)
    parameter (PAMI_EXTENSION_T=4)
    ! same for 32/64 bit
    parameter (PAMI_CLIENT_MEMREGION_SIZE_STATIC=64)
    parameter (PAMI_TASK_T=4)
    parameter (PAMI_ENDPOINT_T=4)
    parameter (PAMI_DT_T=4)
    parameter (PAMI_OP_T=4)
    parameter (PAMI_INT_T=4)
    parameter (PAMI_DBL_T=8)

!
!   \brief Message layer operation types
!
    integer PAMI_UNDEFINED_OP
    integer PAMI_NOOP
    integer PAMI_MAX
    integer PAMI_MIN
    integer PAMI_SUM
    integer PAMI_PROD
    integer PAMI_LXOR
    integer PAMI_BAND
    integer PAMI_BOR
    integer PAMI_BXOR
    integer PAMI_MAXLOC
    integer PAMI_MINLOC
    integer PAMI_USERDEFINED_OP
    integer PAMI_OP_COUNT
    integer PAMI_OP_EXT
    parameter (PAMI_UNDEFINED_OP=0)
    parameter (PAMI_NOOP=1)
    parameter (PAMI_MAX=2)
    parameter (PAMI_MIN=3)
    parameter (PAMI_SUM=4)
    parameter (PAMI_PROD=5)
    parameter (PAMI_LXOR=6)
    parameter (PAMI_BAND=7)
    parameter (PAMI_BOR=8)
    parameter (PAMI_BXOR=9)
    parameter (PAMI_MAXLOC=10)
    parameter (PAMI_MINLOC=11)
    parameter (PAMI_USERDEFINED_OP=12)
    parameter (PAMI_OP_COUNT=13)
    parameter (PAMI_OP_EXT=1000)

!
!   \brief Message layer data types
!
    integer PAMI_UNDEFINED_DT
    integer PAMI_SIGNED_CHAR
    integer PAMI_UNSIGNED_CHAR
    integer PAMI_SIGNED_SHORT
    integer PAMI_UNSIGNED_SHORT
    integer PAMI_SIGNED_INT
    integer PAMI_UNSIGNED_INT
    integer PAMI_SIGNED_LONG_LONG
    integer PAMI_UNSIGNED_LONG_LONG
    integer PAMI_FLOAT
    integer PAMI_DOUBLE
    integer PAMI_LONG_DOUBLE
    integer PAMI_LOGICAL
    integer PAMI_SINGLE_COMPLEX
    integer PAMI_DOUBLE_COMPLEX
    integer PAMI_LOC_2INT
    integer PAMI_LOC_SHORT_INT
    integer PAMI_LOC_FLOAT_INT
    integer PAMI_LOC_DOUBLE_INT
    integer PAMI_LOC_2FLOAT
    integer PAMI_LOC_2DOUBLE
    integer PAMI_USERDEFINED_DT
    integer PAMI_DT_COUNT
    integer PAMI_DT_EXT
    parameter (PAMI_UNDEFINED_DT=0)
    parameter (PAMI_SIGNED_CHAR=1)
    parameter (PAMI_UNSIGNED_CHAR=2)
    parameter (PAMI_SIGNED_SHORT=3)
    parameter (PAMI_UNSIGNED_SHORT=4)
    parameter (PAMI_SIGNED_INT=5)
    parameter (PAMI_UNSIGNED_INT=6)
    parameter (PAMI_SIGNED_LONG_LONG=7)
    parameter (PAMI_UNSIGNED_LONG_LONG=8)
    parameter (PAMI_FLOAT=9)
    parameter (PAMI_DOUBLE=10)
    parameter (PAMI_LONG_DOUBLE=11)
    parameter (PAMI_LOGICAL=12)
    parameter (PAMI_SINGLE_COMPLEX=13)
    parameter (PAMI_DOUBLE_COMPLEX=14)
    parameter (PAMI_LOC_2INT=15)
    parameter (PAMI_LOC_SHORT_INT=16)
    parameter (PAMI_LOC_FLOAT_INT=17)
    parameter (PAMI_LOC_DOUBLE_INT=18)
    parameter (PAMI_LOC_2FLOAT=19)
    parameter (PAMI_LOC_2DOUBLE=20)
    parameter (PAMI_USERDEFINED_DT=21)
    parameter (PAMI_DT_COUNT=22)
    parameter (PAMI_DT_EXT=1000)

    type pami_user_config_t
        sequence
        integer(PAMI_USER_KEY_T)    :: key
        integer(PAMI_USER_VALUE_T)  :: value
    end type pami_user_config_t

!
!   \brief All possible attributes for all hardware  
!

    integer PAMI_CLIENT_CLOCK_MHZ
    integer PAMI_CLIENT_CONST_CONTEXTS
    integer PAMI_CLIENT_HWTHREADS_AVAILABLE
    integer PAMI_CLIENT_MEMREGION_SIZE
    integer PAMI_CLIENT_MEM_SIZE
    integer PAMI_CLIENT_NUM_TASKS
    integer PAMI_CLIENT_NUM_CONTEXTS
    integer PAMI_CLIENT_PROCESSOR_NAME
    integer PAMI_CLIENT_TASK_ID
    integer PAMI_CLIENT_WTIMEBASE_MHZ
    integer PAMI_CLIENT_WTICK
    integer PAMI_CONTEXT_DISPATCH_ID_MAX
    integer PAMI_DISPATCH_RECV_IMMEDIATE_MAX
    integer PAMI_DISPATCH_SEND_IMMEDIATE_MAX
    integer PAMI_DISPATCH_ATOM_SIZE_MAX
    integer PAMI_GEOMETRY_OPTIMIZE
    integer PAMI_TYPE_DATA_SIZE
    integer PAMI_TYPE_DATA_EXTENT
    integer PAMI_TYPE_ATOM_SIZE
    integer PAMI_ATTRIBUTE_NAME_EXT
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

    integer PAMI_EXT_ATTR
    parameter (PAMI_EXT_ATTR=1000)

!
!   \brief General purpose configuration structure: 3 types
!
    type pami_configuration_int_t
        sequence
        integer(PAMI_INT_T)  :: name
        integer(PAMI_INT_T)  :: value
    end type pami_configuration_int_t

    type pami_configuration_double_t
        sequence
        integer(PAMI_INT_T)  :: name
        integer(PAMI_INT_T)  :: paddings
        integer(PAMI_DBL_T)  :: value
    end type pami_configuration_double_t

    type pami_configuration_addr_t
        sequence
        integer(PAMI_INT_T)  :: name
        integer(PAMI_VOID_T) :: value
    end type pami_configuration_addr_t

!
!   \brief PAMI Collectives Metadata
!
    integer(PAMI_INT_T) PAMI_ALIGN_SEND_BUFFER
    integer(PAMI_INT_T) PAMI_ALIGN_SEND_RECV_BUFFER
    integer(PAMI_INT_T) PAMI_DATATYPE_NOT_VALID
    integer(PAMI_INT_T) PAMI_OP_NOT_VALID
    integer(PAMI_INT_T) PAMI_CONTIGUOUS_SEND
    integer(PAMI_INT_T) PAMI_CONTIGUOUS_RECV
    integer(PAMI_INT_T) PAMI_CONTINUOUS_SEND
    integer(PAMI_INT_T) PAMI_CONTINUOUS_RECV    
    parameter (PAMI_ALIGN_SEND_BUFFER=-2147483648)      ! 0x80000000
    parameter (PAMI_ALIGN_SEND_RECV_BUFFER=1073741824)  ! 0x40000000
    parameter (PAMI_DATATYPE_NOT_VALID=536870912)       ! 0x20000000
    parameter (PAMI_OP_NOT_VALID=268435456)             ! 0x10000000
    parameter (PAMI_CONTIGUOUS_SEND=134217728)          ! 0x08000000
    parameter (PAMI_CONTIGUOUS_RECV=67108864)           ! 0x04000000
    parameter (PAMI_CONTINUOUS_SEND=33554432)           ! 0x02000000
    parameter (PAMI_CONTINUOUS_RECV=16777216)           ! 0x01000000

    type metadata_result_t
        sequence
        integer(PAMI_INT_T)  :: bitmask
    end type metadata_result_t
  
!
!   \brief A metadata structure to describe a collective protocol
!
    integer(PAMI_INT_T) PAMI_MUST_QUERY
    integer(PAMI_INT_T) PAMI_NONLOGICAL
    integer(PAMI_INT_T) PAMI_SEND_MIN_ALIGN
    integer(PAMI_INT_T) PAMI_RECV_MIN_ALIGN
    integer(PAMI_INT_T) PAMI_ALL_DT
    integer(PAMI_INT_T) PAMI_ALL_OP
    integer(PAMI_INT_T) PAMI_CONTIG_S_FLAGS
    integer(PAMI_INT_T) PAMI_CONTIG_R_FLAGS
    integer(PAMI_INT_T) PAMI_CONTIN_S_FLAGS
    integer(PAMI_INT_T) PAMI_CONTIN_R_FLAGS 
    parameter (PAMI_MUST_QUERY=-2147483648)     ! 0x80000000
    parameter (PAMI_NONLOGICAL=1073741824)      ! 0x40000000
    parameter (PAMI_SEND_MIN_ALIGN=536870912)   ! 0x20000000
    parameter (PAMI_RECV_MIN_ALIGN=268435456)   ! 0x10000000
    parameter (PAMI_ALL_DT=134217728)           ! 0x08000000
    parameter (PAMI_ALL_OP=67108864)            ! 0x04000000
    parameter (PAMI_CONTIG_S_FLAGS=33554432)    ! 0x02000000
    parameter (PAMI_CONTIG_R_FLAGS=16777216)    ! 0x01000000
    parameter (PAMI_CONTIN_S_FLAGS=8388608)     ! 0x00800000
    parameter (PAMI_CONTIN_R_FLAGS=4194304)     ! 0x00400000

    integer(PAMI_INT_T) PAMI_HW_ACCEL
    parameter (PAMI_HW_ACCEL=-2147483648)       ! 0x80000000

    type pami_metadata_t
        sequence
        CHARACTER                   :: name(64)
        integer(PAMI_INT_T)         :: version
        integer(PAMI_VOID_T)        :: check_fn
        integer(PAMI_SIZE_T)        :: range_lo
        integer(PAMI_SIZE_T)        :: range_hi
        integer(PAMI_INT_T)         :: bitmask_correct
        integer(PAMI_INT_T)         :: bitmask_perf
        integer(PAMI_SIZE_T)        :: range_lo_perf
        integer(PAMI_SIZE_T)        :: range_hi_perf
    end type pami_metadata_t

!
!   \brief Options for bi-state dispatch hints
!
    integer PAMI_HINT2_OFF
    integer PAMI_HINT2_ON
    parameter (PAMI_HINT2_OFF=0)
    parameter (PAMI_HINT2_ON=1)

!
!   \brief Options for tri-state dispatch hints
!
    integer PAMI_HINT3_DEFAULT
    integer PAMI_HINT3_FORCE_ON
    integer PAMI_HINT3_FORCE_OFF
    parameter (PAMI_HINT3_DEFAULT=0)
    parameter (PAMI_HINT3_FORCE_ON=1)
    parameter (PAMI_HINT3_FORCE_OFF=2)

!
!   \brief Hints for sending a message
!
    integer(PAMI_INT_T) PAMI_BUFFER_REGISTERED 
    integer(PAMI_INT_T) PAMI_CONSISTENCY
    integer(PAMI_INT_T) PAMI_INTERRUPT_ON_RECV
    integer(PAMI_INT_T) PAMI_NO_LOCAL_COPY
    integer(PAMI_INT_T) PAMI_NO_LONG_HEADER
    integer(PAMI_INT_T) PAMI_RECV_IMMEDIATE
    integer(PAMI_INT_T) PAMI_RDMA_FORCE_OFF
    integer(PAMI_INT_T) PAMI_RDMA_FORCE_ON
    integer(PAMI_INT_T) PAMI_SHMEM_FORCE_OFF
    integer(PAMI_INT_T) PAMI_SHMEM_FORCE_ON
    integer(PAMI_INT_T) PAMI_MULTICONTEXT_FORCE_OFF
    integer(PAMI_INT_T) PAMI_MULTICONTEXT_FORCE_ON
    parameter (PAMI_BUFFER_REGISTERED=-2147483648)  ! 0x80000000
    parameter (PAMI_CONSISTENCY=1073741824)         ! 0x40000000
    parameter (PAMI_INTERRUPT_ON_RECV=536870912)    ! 0x20000000
    parameter (PAMI_NO_LOCAL_COPY=268435456)        ! 0x10000000
    parameter (PAMI_NO_LONG_HEADER=134217728)       ! 0x08000000
    parameter (PAMI_RECV_IMMEDIATE=67108864)        ! 0x04000000
    parameter (PAMI_RDMA_FORCE_OFF=33554432)        ! 0x02000000
    parameter (PAMI_RDMA_FORCE_ON=16777216)         ! 0x01000000
    parameter (PAMI_SHMEM_FORCE_OFF=8388608)        ! 0x00800000
    parameter (PAMI_SHMEM_FORCE_ON=4194304)         ! 0x00400000
    parameter (PAMI_MULTICONTEXT_FORCE_OFF=2097152) ! 0x00200000
    parameter (PAMI_MULTICONTEXT_FORCE_ON=1048576)  ! 0x00100000

    type pami_send_hint_t
        sequence
        integer(PAMI_INT_T)  :: hints
    end type pami_send_hint_t

    integer(PAMI_INT_T) PAMI_COLLECTIVE_MULTICONTEXT_FORCE_OFF
    integer(PAMI_INT_T) PAMI_COLLECTIVE_MULTICONTEXT_FORCE_ON
    parameter (PAMI_COLLECTIVE_MULTICONTEXT_FORCE_OFF=-2147483648)  ! 0x80000000
    parameter (PAMI_COLLECTIVE_MULTICONTEXT_FORCE_ON=1073741824)    ! 0x40000000

    type pami_collective_hint_t
        sequence
        integer(PAMI_INT_T)  :: multicontext
    end type pami_collective_hint_t

!
!   \brief Active message send common parameters structure
!
    type pami_iovec_t
        sequence
        integer(PAMI_VOID_T)  :: iov_base
        integer(PAMI_SIZE_T)  :: iov_len
    end type pami_iovec_t

    type pami_send_immediate_t
        sequence
        type(pami_iovec_t)          :: header
        type(pami_iovec_t)          :: data
        integer(PAMI_SIZE_T)        :: dispatch
        type(pami_send_hint_t)      :: hints
        integer(PAMI_ENDPOINT_T)    :: dest
    end type pami_send_immediate_t

!
!   \brief Structure for event notification
!
    type pami_send_event_t
        sequence
        integer(PAMI_VOID_T)  :: cookie
        integer(PAMI_VOID_T)  :: local_fn
        integer(PAMI_VOID_T)  :: remote_fn
    end type pami_send_event_t

!
!   \brief Structure for send parameters unique to a simple active message send
!
    type pami_send_t
        sequence
        type(pami_send_immediate_t) :: send
        type(pami_send_event_t)     :: events
    end type pami_send_t

!
!   \brief Structure for send parameters of a typed active message send
!
    type pami_send_typed_t
        sequence
        type(pami_send_immediate_t) :: send
        type(pami_send_event_t)     :: events
        integer(PAMI_TYPE_T)        :: type
        integer(PAMI_SIZE_T)        :: offset
        integer(PAMI_VOID_T)        :: data_fn
        integer(PAMI_VOID_T)        :: data_cookie
    end type pami_send_typed_t

!
!   \brief Receive message structure
!
    type pami_recv_t
        sequence
        integer(PAMI_VOID_T)        :: cookie
        integer(PAMI_VOID_T)        :: local_fn
        integer(PAMI_VOID_T)        :: addr
        integer(PAMI_TYPE_T)        :: type
        integer(PAMI_SIZE_T)        :: offset
        integer(PAMI_VOID_T)        :: data_fn
        integer(PAMI_VOID_T)        :: data_cookie
    end type pami_recv_t

!
!   \brief Input parameters common to all rma transfers
!
    type pami_rma_t
        sequence
        integer(PAMI_ENDPOINT_T)    :: dest
        type(pami_send_hint_t)      :: hints
        integer(PAMI_SIZE_T)        :: bytes
        integer(PAMI_VOID_T)        :: cookie
        integer(PAMI_VOID_T)        :: done_fn
    end type pami_rma_t

!
!   \brief Input parameters for rma simple transfers
!
    type pami_rma_addr_t
        sequence
        integer(PAMI_VOID_T)        :: local
        integer(PAMI_VOID_T)        :: remote
    end type pami_rma_addr_t

!
!   \brief Input parameters for rma typed transfers
!
    type pami_rma_typed_t
        sequence
        integer(PAMI_VOID_T)        :: local
        integer(PAMI_VOID_T)        :: remote
    end type pami_rma_typed_t

!
!   \brief Input parameters for rma put transfers
!
    type pami_rma_put_t
        sequence
        integer(PAMI_VOID_T)        :: rdone_fn
    end type pami_rma_put_t

! 
!   \brief Input parameters for simple put transfers
!
    type pami_put_simple_t
        sequence
        type(pami_rma_t)        :: rma
        type(pami_rma_addr_t)   :: addr
        type(pami_rma_put_t)    :: put
    end type pami_put_simple_t

!
!   \brief Input parameters for simple typed put transfers
!
    type pami_put_typed_t
        sequence
        type(pami_rma_t)        :: rma
        type(pami_rma_addr_t)   :: addr
        type(pami_rma_typed_t)  :: type
        type(pami_rma_put_t)    :: put
    end type pami_put_typed_t

!
!   \brief Input parameter structure for simple get transfers
!
    type pami_get_simple_t
        sequence
        type(pami_rma_t)        :: rma
        type(pami_rma_addr_t)   :: addr
    end type pami_get_simple_t

!
!   \brief Input parameter structure for typed get transfers
!
    type pami_get_typed_t
        sequence
        type(pami_rma_t)        :: rma
        type(pami_rma_addr_t)   :: addr
        type(pami_rma_typed_t)  :: type
    end type pami_get_typed_t

!
!   \brief Atomic rmw data type
!
    integer PAMI_RMW_KIND_UINT32
    integer PAMI_RMW_KIND_UINT64
    parameter (PAMI_RMW_KIND_UINT32=1)
    parameter (PAMI_RMW_KIND_UINT64=2)

!    
!   \brief Atomic rmw assignment type
!
    integer PAMI_RMW_ASSIGNMENT_SET
    integer PAMI_RMW_ASSIGNMENT_ADD
    integer PAMI_RMW_ASSIGNMENT_OR
    integer PAMI_RMW_ASSIGNMENT_AND
    parameter (PAMI_RMW_ASSIGNMENT_SET=16)
    parameter (PAMI_RMW_ASSIGNMENT_ADD=32)
    parameter (PAMI_RMW_ASSIGNMENT_OR=64)
    parameter (PAMI_RMW_ASSIGNMENT_AND=128)

!
!   \brief Atomic rmw comparison type
!
    integer PAMI_RMW_COMPARISON_NOOP
    integer PAMI_RMW_COMPARISON_EQUAL
    parameter (PAMI_RMW_COMPARISON_NOOP=256)
    parameter (PAMI_RMW_COMPARISON_EQUAL=512)

    type pami_rmw_t
        sequence
        type(pami_rma_t)        :: rma
        type(pami_rma_addr_t)   :: addr
        integer(PAMI_INT_T)     :: compare
        integer(PAMI_INT_T)     :: assign
        integer(PAMI_INT_T)     :: kind
        integer(PAMI_SIZE_T)    :: value
        integer(PAMI_SIZE_T)    :: test
    end type pami_rmw_t

!
!   \brief Memory region type
!
    type pami_rma_mr_t
        sequence
        CHARACTER(PAMI_CLIENT_MEMREGION_SIZE_STATIC)    :: mr
        integer(PAMI_SIZE_T)                            :: offset
    end type pami_rma_mr_t

    type pami_rdma_t
        sequence
        type(pami_rma_mr_t) :: local
        type(pami_rma_mr_t) :: remote
    end type pami_rdma_t

!
!   \brief Input parameter structure for simple rdma put transfers
!
    type pami_rput_simple_t
        sequence
        type(pami_rma_t)        :: rma
        type(pami_rdma_t)       :: rdma
        type(pami_rma_put_t)    :: put
    end type pami_rput_simple_t

!
!   \brief Input parameter structure for typed rdma put transfers
!
    type pami_rput_typed_t
        sequence
        type(pami_rma_t)        :: rma
        type(pami_rdma_t)       :: rdma
        type(pami_rma_typed_t)  :: type
        type(pami_rma_put_t)    :: put
    end type pami_rput_typed_t

!
!   \brief Input parameter structure for simple rdma get transfers
!
    type pami_rget_simple_t
        sequence
        type(pami_rma_t)        :: rma
        type(pami_rdma_t)       :: rdma
    end type pami_rget_simple_t

!
!   \brief Input parameter structure for typed rdma get transfers
!
    type pami_rget_typed_t
        sequence
        type(pami_rma_t)        :: rma
        type(pami_rdma_t)       :: rdma
        type(pami_rma_typed_t)  :: type
    end type pami_rget_typed_t

!
!   \brief Transfer Types,used by geometry and xfer routines
!
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
    integer PAMI_XFER_TYPE_EXT
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

!
!   \brief Geometry (like groups/communicators)
!
    type pami_geometry_range_t
        sequence
        integer(PAMI_SIZE_T)  :: lo
        integer(PAMI_SIZE_T)  :: hi
    end type pami_geometry_range_t

!
!   \brief Common fields for PAMI collective operations
!
    type pami_xfer_comm_t
        sequence
        integer(PAMI_VOID_T)        :: cb_done
        integer(PAMI_VOID_T)        :: cookie
        integer(PAMI_ALGORITHM_T)   :: algorithm
        type(pami_collective_hint_t):: options
    end type pami_xfer_comm_t

!
!   \brief Alltoall vector type
!
    type pami_xfer_alltoallv_t
        sequence
        type(pami_xfer_comm_t):: xfer
        integer(PAMI_VOID_T)  :: sndbuf
        integer(PAMI_TYPE_T)  :: stype
        integer(PAMI_SIZE_T)  :: stypecounts
        integer(PAMI_SIZE_T)  :: sdispls
        integer(PAMI_VOID_T)  :: rcvbuf
        integer(PAMI_TYPE_T)  :: rtype
        integer(PAMI_SIZE_T)  :: rtypecounts
        integer(PAMI_SIZE_T)  :: rdispls
    end type pami_xfer_alltoallv_t

!
!   \brief Non-blocking alltoall vector type
!
    type pami_xfer_alltoallv_int_t
        sequence  
        type(pami_xfer_comm_t):: xfer
        integer(PAMI_VOID_T)  :: sndbuf
        integer(PAMI_TYPE_T)  :: stype
        integer(PAMI_SIZE_T)  :: stypecounts
        integer(PAMI_SIZE_T)  :: sdispls
        integer(PAMI_VOID_T)  :: rcvbuf
        integer(PAMI_TYPE_T)  :: rtype
        integer(PAMI_SIZE_T)  :: rtypecounts
        integer(PAMI_SIZE_T)  :: rdispls
    end type pami_xfer_alltoallv_int_t

!
!   \brief Non-blocking alltoall type
!
    type pami_xfer_alltoall_t
        sequence
        type(pami_xfer_comm_t):: xfer
        integer(PAMI_VOID_T)  :: sndbuf
        integer(PAMI_TYPE_T)  :: stype
        integer(PAMI_SIZE_T)  :: stypecounts
        integer(PAMI_SIZE_T)  :: sdispls
        integer(PAMI_VOID_T)  :: rcvbuf
        integer(PAMI_TYPE_T)  :: rtype
        integer(PAMI_SIZE_T)  :: rtypecounts
    end type pami_xfer_alltoall_t

!
!   \brief Non-blocking reduce type
!
    type pami_xfer_reduce_t
        sequence
        type(pami_xfer_comm_t):: xfer
        integer(PAMI_SIZE_T)  :: root
        integer(PAMI_VOID_T)  :: sndbuf
        integer(PAMI_TYPE_T)  :: stype
        integer(PAMI_SIZE_T)  :: stypecount
        integer(PAMI_VOID_T)  :: rcvbuf
        integer(PAMI_TYPE_T)  :: rtype
        integer(PAMI_SIZE_T)  :: rtypecount
        integer(PAMI_DT_T)    :: dt
        integer(PAMI_OP_T)    :: op
    end type pami_xfer_reduce_t

!
!   \brief Non-blocking reduce_scatter type
!
    type pami_xfer_reduce_scatter_t
        sequence
        type(pami_xfer_comm_t):: xfer
        integer(PAMI_VOID_T)  :: sndbuf
        integer(PAMI_TYPE_T)  :: stype
        integer(PAMI_SIZE_T)  :: stypecount
        integer(PAMI_VOID_T)  :: rcvbuf
        integer(PAMI_TYPE_T)  :: rtype
        integer(PAMI_SIZE_T)  :: rtypecount
        integer(PAMI_SIZE_T)  :: rcounts
        integer(PAMI_DT_T)    :: dt
        integer(PAMI_OP_T)    :: op
    end type pami_xfer_reduce_scatter_t

!
!   \brief Non-blocking broadcast type
!
    type pami_xfer_broadcast_t
        sequence
        type(pami_xfer_comm_t):: xfer
        integer(PAMI_SIZE_T)  :: root
        integer(PAMI_VOID_T)  :: buf
        integer(PAMI_TYPE_T)  :: type
        integer(PAMI_SIZE_T)  :: typecount
    end type pami_xfer_broadcast_t

!
!   \brief Non-blocking allgather type
!
    type pami_xfer_allgather_t
        sequence
        type(pami_xfer_comm_t):: xfer
        integer(PAMI_VOID_T)  :: sndbuf
        integer(PAMI_TYPE_T)  :: stype
        integer(PAMI_SIZE_T)  :: stypecount
        integer(PAMI_VOID_T)  :: rcvbuf
        integer(PAMI_TYPE_T)  :: rtype
        integer(PAMI_SIZE_T)  :: rtypecount
    end type pami_xfer_allgather_t

!
!   \brief Non-blocking allgather type
!
    type pami_xfer_gather_t
        sequence
        type(pami_xfer_comm_t):: xfer
        integer(PAMI_SIZE_T)  :: root
        integer(PAMI_VOID_T)  :: sndbuf
        integer(PAMI_TYPE_T)  :: stype
        integer(PAMI_SIZE_T)  :: stypecount
        integer(PAMI_VOID_T)  :: rcvbuf
        integer(PAMI_TYPE_T)  :: rtype
        integer(PAMI_SIZE_T)  :: rtypecount
    end type pami_xfer_gather_t

!
!   \brief Non-blocking gatherv type
!
    type pami_xfer_gatherv_t
        sequence
        type(pami_xfer_comm_t):: xfer
        integer(PAMI_SIZE_T)  :: root
        integer(PAMI_VOID_T)  :: sndbuf
        integer(PAMI_TYPE_T)  :: stype
        integer(PAMI_SIZE_T)  :: stypecount
        integer(PAMI_VOID_T)  :: rcvbuf
        integer(PAMI_TYPE_T)  :: rtype
        integer(PAMI_SIZE_T)  :: rtypecounts
        integer(PAMI_SIZE_T)  :: rdispls
    end type pami_xfer_gatherv_t

!
!   \brief Non-blocking gatherv type
!
    type pami_xfer_gatherv_int_t
        sequence
        type(pami_xfer_comm_t):: xfer
        integer(PAMI_SIZE_T)  :: root
        integer(PAMI_VOID_T)  :: sndbuf
        integer(PAMI_TYPE_T)  :: stype
        integer(PAMI_SIZE_T)  :: stypecount
        integer(PAMI_VOID_T)  :: rcvbuf
        integer(PAMI_TYPE_T)  :: rtype
        integer(PAMI_SIZE_T)  :: rtypecounts
        integer(PAMI_SIZE_T)  :: rdispls
    end type pami_xfer_gatherv_int_t

!
!   \brief Non-blocking allgatherv type
!
    type pami_xfer_allgatherv_t
        sequence
        type(pami_xfer_comm_t):: xfer
        integer(PAMI_VOID_T)  :: sndbuf
        integer(PAMI_TYPE_T)  :: stype
        integer(PAMI_SIZE_T)  :: stypecount
        integer(PAMI_VOID_T)  :: rcvbuf
        integer(PAMI_TYPE_T)  :: rtype
        integer(PAMI_SIZE_T)  :: rtypecounts
        integer(PAMI_SIZE_T)  :: rdispls
    end type pami_xfer_allgatherv_t

!
!   \brief Non-blocking allgatherv type
!
    type pami_xfer_allgatherv_int_t
        sequence
        type(pami_xfer_comm_t):: xfer
        integer(PAMI_VOID_T)  :: sndbuf
        integer(PAMI_TYPE_T)  :: stype
        integer(PAMI_SIZE_T)  :: stypecount
        integer(PAMI_VOID_T)  :: rcvbuf
        integer(PAMI_TYPE_T)  :: rtype
        integer(PAMI_SIZE_T)  :: rtypecounts
        integer(PAMI_SIZE_T)  :: rdispls
    end type pami_xfer_allgatherv_int_t

!
!   \brief Non-blocking scatter type
!
    type pami_xfer_scatter_t
        sequence
        type(pami_xfer_comm_t):: xfer
        integer(PAMI_SIZE_T)  :: root
        integer(PAMI_VOID_T)  :: sndbuf
        integer(PAMI_TYPE_T)  :: stype
        integer(PAMI_SIZE_T)  :: stypecount
        integer(PAMI_VOID_T)  :: rcvbuf
        integer(PAMI_TYPE_T)  :: rtype
        integer(PAMI_SIZE_T)  :: rtypecount
        integer(PAMI_SIZE_T)  :: rdispls
    end type pami_xfer_scatter_t

!
!   \brief Non-blocking scatterv type
!
    type pami_xfer_scatterv_t
        sequence
        type(pami_xfer_comm_t):: xfer
        integer(PAMI_SIZE_T)  :: root
        integer(PAMI_VOID_T)  :: sndbuf
        integer(PAMI_TYPE_T)  :: stype
        integer(PAMI_SIZE_T)  :: stypecounts
        integer(PAMI_VOID_T)  :: rcvbuf
        integer(PAMI_TYPE_T)  :: rtype
        integer(PAMI_SIZE_T)  :: rtypecount
    end type pami_xfer_scatterv_t

!
!   \brief Non-blocking scatterv type
!
    type pami_xfer_scatterv_int_t
        sequence
        type(pami_xfer_comm_t):: xfer
        integer(PAMI_SIZE_T)  :: root
        integer(PAMI_VOID_T)  :: sndbuf
        integer(PAMI_TYPE_T)  :: stype
        integer(PAMI_SIZE_T)  :: stypecounts
        integer(PAMI_SIZE_T)  :: sdispls
        integer(PAMI_VOID_T)  :: rcvbuf
        integer(PAMI_TYPE_T)  :: rtype
        integer(PAMI_SIZE_T)  :: rtypecount
    end type pami_xfer_scatterv_int_t

!
!   \brief Non-blocking allreduce type
!
    type pami_xfer_allreduce_t
        sequence
        type(pami_xfer_comm_t):: xfer
        integer(PAMI_VOID_T)  :: sndbuf
        integer(PAMI_TYPE_T)  :: stype
        integer(PAMI_SIZE_T)  :: stypecount
        integer(PAMI_VOID_T)  :: rcvbuf
        integer(PAMI_TYPE_T)  :: rtype
        integer(PAMI_SIZE_T)  :: rtypecount
        integer(PAMI_DT_T)    :: dt
        integer(PAMI_OP_T)    :: op
    end type pami_xfer_allreduce_t

!
!   \brief Non-blocking scan type
!
    type pami_xfer_scan_t
        sequence
        type(pami_xfer_comm_t):: xfer
        integer(PAMI_VOID_T)  :: sndbuf
        integer(PAMI_TYPE_T)  :: stype
        integer(PAMI_SIZE_T)  :: stypecount
        integer(PAMI_VOID_T)  :: rcvbuf
        integer(PAMI_TYPE_T)  :: rtype
        integer(PAMI_SIZE_T)  :: rtypecount
        integer(PAMI_DT_T)    :: dt
        integer(PAMI_OP_T)    :: op
        integer(PAMI_INT_T)   :: exclusive
    end type pami_xfer_scan_t

!
!   \brief Non-blocking barrier type
!
    type pami_xfer_barrier_t
        sequence
        type(pami_xfer_comm_t):: xfer
    end type pami_xfer_barrier_t

!
!   \brief Non-blocking fence type
!
    type pami_xfer_fence_t
        sequence
        type(pami_xfer_comm_t):: xfer
    end type pami_xfer_fence_t

!
!   \brief Non-blocking active message broadcast type
!
    type pami_xfer_ambroadcast_t
        sequence
        type(pami_xfer_comm_t):: xfer
        integer(PAMI_SIZE_T)  :: dispatch
        integer(PAMI_VOID_T)  :: user_header
        integer(PAMI_SIZE_T)  :: headerlen
        integer(PAMI_VOID_T)  :: sndbuf
        integer(PAMI_TYPE_T)  :: stype
        integer(PAMI_SIZE_T)  :: stypecount
    end type pami_xfer_ambroadcast_t

!
!   \brief Non-blocking active message scatter type
!
    type pami_xfer_amscatter_t
        sequence
        type(pami_xfer_comm_t):: xfer
        integer(PAMI_SIZE_T)  :: dispatch
        integer(PAMI_VOID_T)  :: headers
        integer(PAMI_SIZE_T)  :: headerlen
        integer(PAMI_VOID_T)  :: sndbuf
        integer(PAMI_TYPE_T)  :: stype
        integer(PAMI_SIZE_T)  :: stypecount
    end type pami_xfer_amscatter_t

!
!   \brief Non-blocking active message gather type
!
    type pami_xfer_amgather_t
        sequence
        type(pami_xfer_comm_t):: xfer
        integer(PAMI_SIZE_T)  :: dispatch
        integer(PAMI_VOID_T)  :: headers
        integer(PAMI_SIZE_T)  :: headerlen
        integer(PAMI_VOID_T)  :: rcvbuf
        integer(PAMI_TYPE_T)  :: rtype
        integer(PAMI_SIZE_T)  :: rtypecount
    end type pami_xfer_amgather_t

!
!   \brief Non-blocking active message reduce type
!
    type pami_xfer_amreduce_t
        sequence
        type(pami_xfer_comm_t):: xfer
        integer(PAMI_SIZE_T)  :: dispatch
        integer(PAMI_VOID_T)  :: user_header
        integer(PAMI_SIZE_T)  :: headerlen
        integer(PAMI_VOID_T)  :: rcvbuf
        integer(PAMI_TYPE_T)  :: rtype
        integer(PAMI_SIZE_T)  :: rtypecount
        integer(PAMI_DT_T)    :: dt
        integer(PAMI_OP_T)    :: op
    end type pami_xfer_amreduce_t

!
!   \brief PAMI datatype that represents a contigous data layout
!
    integer(PAMI_TYPE_T) PAMI_TYPE_CONTIGUOUS
    integer(PAMI_VOID_T) PAMI_DATA_COPY
    integer(PAMI_GEOMETRY_T) PAMI_NULL_GEOMETRY
    common/PAMI_FORT_GLOBALS/ PAMI_TYPE_CONTIGUOUS, PAMI_DATA_COPY, PAMI_NULL_GEOMETRY

!
!   \brief PAMI type of dispatch
!
    integer PAMI_P2P_SEND
    parameter (PAMI_P2P_SEND=0)

!
!   \brief Hints for dispatch
!
    type pami_dispatch_hint_t
        sequence
        integer(PAMI_INT_T)     :: type
        type(pami_send_hint_t)  :: hint
        integer(PAMI_VOID_T)    :: config
    end type pami_dispatch_hint_t
