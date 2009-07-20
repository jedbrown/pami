/* --------------------------------------------------------------- */
/* (C) Copyright 2001,2006,                                        */
/* International Business Machines Corporation,                    */
/*                                                                 */
/* All Rights Reserved.                                            */
/* --------------------------------------------------------------- */
/* PROLOG END TAG zYx                                              */

// ident = "@(#) internal-src/lib/dacs/include/dacs.h v%I% - %H% %T% @(#)"

#ifndef _DACS_H_
#define _DACS_H_

/*--------------------------------------------------------------------*/
/*  Version                                                           */
/*--------------------------------------------------------------------*/

#define DACS_VERSION_MAJOR  4
#define DACS_VERSION_MINOR  0

/*--------------------------------------------------------------------*/
/*  Includes                                                          */
/*--------------------------------------------------------------------*/

#include <stdint.h>
#include <unistd.h>

/*--------------------------------------------------------------------*/
/*  User Types                                                        */
/*--------------------------------------------------------------------*/

// 64-bit address, aligned on 128-bit boundaries
typedef uint64_t dacs_addr_64_t;

// dacs_remote_mem_t is a handle to a remote memory object
typedef uint64_t dacs_remote_mem_t;

// dacs_mem_t is a handle to a memory object
typedef uint64_t dacs_mem_t;

// A handle to a mutex.  This is an opaque type.
typedef uint64_t dacs_mutex_t;

// 4-byte unsigned int
typedef uint32_t uint4;

// 4-byte signed int
typedef int32_t int4;

// 8-byte unsigned int
typedef uint64_t uint8;

// 8-byte signed int
typedef int64_t int8;

// octet
typedef uint8_t dacs_byte_t;

typedef uint32_t dacs_wid_t;

// Group descriptor handle 
typedef uint64_t  dacs_group_t;

// Dacs Error
typedef void *dacs_error_t;

// Error handler 
typedef uint32_t(*dacs_error_handler_t)(dacs_error_t error);

// Representation of the dacs_id 
typedef uint32_t de_id_t;

// Representation of a process running on a DE
typedef uint64_t dacs_process_id_t;

// Byte-swap flags
typedef enum DACS_BYTE_SWAP
{
    DACS_BYTE_SWAP_DISABLE      = 0,    // no byte-swapping
    DACS_BYTE_SWAP_HALF_WORD    = 1,    // byte-swapping for halfword. 
                                        //  A halfword is a two-byte value.
    DACS_BYTE_SWAP_WORD         = 2,    // byte-swapping for word. 
                                        //  A word is a four-byte value.
    DACS_BYTE_SWAP_DOUBLE_WORD  = 3,    // byte-swapping for doubleword. 
                                        //  A double word is a eight-byte value.
}   DACS_BYTE_SWAP_T ;

// DACS_MEMORY_ACCESS_MODE_T specifies the mem access modes for 
// dacs_shared_mem_t types
typedef enum DACS_MEMORY_ACCESS_MODE
{
    DACS_READ_ONLY  = 0x20,   // remote proc has only read access
    DACS_WRITE_ONLY = 0x40,   // remote proc has only write access
    DACS_READ_WRITE = 0x60,   // remote proc has both read/write access
}   DACS_MEMORY_ACCESS_MODE_T;

// Used when creating a dacs_mem_t memory region. Mode is specified for
// both remote and local access
typedef enum DACS_MEM_ACCESS_MODE
{
    DACS_MEM_READ_ONLY  = 0x20, ///< Read-only region
    DACS_MEM_WRITE_ONLY = 0x40, ///< Write-only region
    DACS_MEM_READ_WRITE = 0x60, ///< Read-write region
    DACS_MEM_NONE       = 0x00, ///< No access 
}   DACS_MEM_ACCESS_MODE_T;

// DACS_REMOTE_MEM_ATTR_T specifies the attributes that can be read from a 
// dacs_remote_mem_t
typedef enum DACS_REMOTE_MEM_ATTR
{
    DACS_REMOTE_MEM_ADDR,
    DACS_REMOTE_MEM_SIZE,
    DACS_REMOTE_MEM_PERM,
}   DACS_REMOTE_MEM_ATTR_T;

// DACS_MEM_ATTR_T specifies the attributes that can be read from a
// dacs_mem_t
typedef enum DACS_MEM_ATTR
{
    DACS_MEM_ADDR,        ///< Base address of memory region
    DACS_MEM_SIZE,        ///< Size of memory region
    DACS_RMT_MEM_PERM, ///< Remote permissions of memory region
    DACS_LCL_MEM_PERM,  ///< Local permissions of memory region
}   DACS_MEM_ATTR_T;

// DACS_MEM_LIMITS_T are values that can queried to find the limits of resources
// available for memory regions
typedef enum DACS_MEM_LIMITS
{
    DACS_MEM_REGION_MAX_NUM  = 1, ///< Maximum number of memory regions available
    DACS_MEM_REGION_MAX_SIZE,     ///< Maximum memory region size (in bytes)
    DACS_MEM_REGION_AVAIL,        ///< Current number of available memory regions
} DACS_MEM_LIMITS_T;

// DACS_ORDER_ATTR_T describes the memory ordering for put, get, and list 
// operations. Some implementations rely on barrier being equal to 1 and
// fence being equal to 2, so these values should not be changed
typedef enum DACS_ORDER_ATTR
{
    DACS_ORDER_ATTR_NONE    = 0,
    DACS_ORDER_ATTR_BARRIER = 1,
    DACS_ORDER_ATTR_FENCE   = 2,
}   DACS_ORDER_ATTR_T;

// Types of DEs in the topology
typedef enum    DACS_DE_TYPE
{
    DACS_DE_INVALID=0,              // INVALID TYPE
    DACS_DE_SYSTEMX,                // Opteron - supervises 4 Cell Blades
    DACS_DE_CELL_BLADE,             // Cell Blade - contains 2 CBE's
    DACS_DE_CBE,                    // Cell Broadband Engine
    DACS_DE_SPE,                    // Cell SPE
    DACS_DE_MAX_TYPE                // Always last - max DE type
}   DACS_DE_TYPE_T ;

// DACS error code convention:
// - Some DACS "error" codes are actually status returned from API's.  These
// will be 0 or positive numbers.
// - All DACS error codes will start with DACS_ERR prefix.
// - All DACS codes indicating true errors will be negative numbers.

typedef enum
{
    DACS_SUCCESS                        =        0,
    DACS_WID_READY                      =        0,
    DACS_WID_BUSY                       =        1,
    DACS_STS_PROC_RUNNING               =        2,
    DACS_STS_PROC_FINISHED              =        3,
    DACS_STS_PROC_FAILED                =        4,
    DACS_STS_PROC_ABORTED               =        5,
    DACS_STS_PROC_KILLED                =        6,
    DACS_LAST_STATUS                    =        7,

    //  errcodes should start at -35000 and count UP from there.
    //  therefore DACS_FIRST_ERROR will be the most negative #
    DACS_ERR_FIRST_ERROR                =   -35000, //< first errorcode
    DACS_ERR_INTERNAL                   =   -34999,
    DACS_ERR_SYSTEM                     =   -34998,
    DACS_ERR_INVALID_ARGV               =   -34997,
    DACS_ERR_INVALID_ENV                =   -34996,
    DACS_ERR_INVALID_HANDLE             =   -34995,
    DACS_ERR_INVALID_ADDR               =   -34994,
    DACS_ERR_INVALID_ATTR               =   -34993,
    DACS_ERR_INVALID_DE                 =   -34992,
    DACS_ERR_INVALID_PID                =   -34991,
    DACS_ERR_INVALID_TARGET             =   -34990,
    DACS_ERR_BUF_OVERFLOW               =   -34989,
    DACS_ERR_NOT_ALIGNED                =   -34988,
    DACS_ERR_INVALID_SIZE               =   -34987,
    DACS_ERR_BYTESWAP_MISMATCH          =   -34986,
    DACS_ERR_NO_RESOURCE                =   -34985,
    DACS_ERR_PROC_LIMIT                 =   -34984,
    DACS_ERR_NO_PERM                    =   -34983,
    DACS_ERR_OWNER                      =   -34982,
    DACS_ERR_NOT_OWNER                  =   -34981,
    DACS_ERR_RESOURCE_BUSY              =   -34980,
    DACS_ERR_GROUP_CLOSED               =   -34979,
    DACS_ERR_GROUP_OPEN                 =   -34978,
    DACS_ERR_GROUP_DUPLICATE            =   -34977,
    DACS_ERR_INVALID_WID                =   -34976,
    DACS_ERR_INVALID_STREAM             =   -34975,
    DACS_ERR_NO_WIDS                    =   -34974,
    DACS_ERR_WID_ACTIVE                 =   -34973,
    DACS_ERR_WID_NOT_ACTIVE             =   -34972,
    DACS_ERR_INITIALIZED                =   -34971,
    DACS_ERR_NOT_INITIALIZED            =   -34970,
    DACS_ERR_MUTEX_BUSY                 =   -34969,
    DACS_ERR_NOT_SUPPORTED_YET          =   -34968,
    DACS_ERR_VERSION_MISMATCH           =   -34967,
    DACS_ERR_DACSD_FAILURE              =   -34966,
    DACS_ERR_INVALID_PROG               =   -34965,
    DACS_ERR_ARCH_MISMATCH              =   -34964,
    DACS_ERR_INVALID_USERNAME		=   -34963,
    DACS_ERR_INVALID_CWD		=   -34962,
    DACS_ERR_NOT_FOUND			=   -34961,
    DACS_ERR_TOO_LONG 			=   -34960,
    DACS_ERR_DE_TERM			=   -34959,

    // Add new error codes above here
    DACS_ERR_LAST_ERROR

}   DACS_ERR_T;

// Flags to test read or write mailbox
typedef enum DACS_TEST_MAILBOX
{
    DACS_TEST_MAILBOX_READ,
    DACS_TEST_MAILBOX_WRITE,
}   DACS_TEST_MAILBOX_T;

/* Flags for dacs_de_kill*/
typedef enum DACS_KILL_TYPE
{
    DACS_KILL_TYPE_ASYNC
}   DACS_KILL_TYPE_T;


// Implementation-specific flag that influences process creation.
//
//      DACS_PROC_LOCAL_FILE -  the local file format:  
//                              A fully-qualified POSIX-compliant pathname of 
//                              the executable file that can be accessed from 
//                              the local file system.  
//      DACS_PROC_LOCAL_FILE_LIST - the local file list format: 
//                              A fully-qualified POSIX-compliant pathname of a 
//                              text file that can be accessed from the local 
//                              file system.  This file contains a (text) list 
//                              of the executable file and all dependant 
//                              libraries. The file and all the libraries is 
//                              loaded onto the accelerator and executed. 
//                              The executable file must be the first thing in 
//                              the list.  
//      DACS_PROC_REMOTE_FILE - the remote file format: 
//                              A fully-qualified POSIX-compliant pathname of 
//                              the executable file that can be accessed from 
//                              the remote accelerator.  
//      DACS_PROC_EMBEDDED is the embedded image format: 
//                              A pointer to an executable image that is 
//                              embedded in the executable image. 
typedef enum DACS_PROC_CREATION_FLAG_T
{
    DACS_PROC_LOCAL_FILE,
    DACS_PROC_LOCAL_FILE_LIST,
    DACS_PROC_REMOTE_FILE,
    DACS_PROC_EMBEDDED,
} DACS_PROC_CREATION_FLAG_T;

typedef struct dacs_program_handle {
    unsigned int data[8];
} dacs_program_handle_t;

// dacs_dma_list_t is a dma_list_element in 64 bit addressing mode
typedef struct dacs_dma_list
{
    dacs_addr_64_t offset;          // address of the buffer on DE memory
    uint64_t size;                  // byte count to be transfered
}   dacs_dma_list_t;

/*--------------------------------------------------------------------*/
/*  Constants                                                         */
/*--------------------------------------------------------------------*/

#define DACSI_MAX_ERROR_STRINGS   50

// Invalid group #, used to initialize vars
#define DACS_NULL_GROUP     0

// Constant used as a process_id to refer to all running processes on a node
#define DACS_PROCESS_ALL (0xFFFFFFFFFFFFFFFFULL)

//  invalid de_id
#define NULL_DE_ID  0
#define DACS_DE_SELF  -1U
#define DACS_PID_SELF -1ULL

// current number of SPE's per CBE
#define NUM_SPES_PER_CBE        8
#define NUM_SPES_PER_BLADE      (2*NUM_SPES_PER_CBE)

// This allows sending to indirect streams
enum {
        DACS_STREAM_ALL = 0xffffffff,
        DACS_STREAM_UB  = 0xffffff00
};

// Number of Processes allowed to run on a DE
#define MAX_PROCESSES_PER_DE    1

// Refers to the parent de_id in the topology.
#define DACS_DE_PARENT  -2U

// Refers to the parent process in the topology.
#define DACS_PID_PARENT -2ULL

// Refers to the number of PPEs that a PPE can talk to directly
#define MAX_PPE_PEERS 2

#define DACS_INVALID_WID   0xDAC5DEAD

// Bit-significant flags used for init configuration
#define DACS_INIT_FLAGS_NONE        0
#define DACS_INIT_SINGLE_THREADED 0x1

/*--------------------------------------------------------------------*/
/*  Function Prototypes                                               */
/*--------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern DACS_ERR_T dacs_test(dacs_wid_t wid);

extern DACS_ERR_T dacs_wait(dacs_wid_t wid);

extern DACS_ERR_T dacs_remote_mem_create(void                      *addr,
                                         uint64_t                  size,
                                         DACS_MEMORY_ACCESS_MODE_T access_mode,
                                         dacs_remote_mem_t         *mem);


extern DACS_ERR_T dacs_remote_mem_share(de_id_t           dst_de,
                                        dacs_process_id_t dst_pid,
                                        dacs_remote_mem_t mem);


extern DACS_ERR_T dacs_remote_mem_accept(de_id_t           src_de,
                                         dacs_process_id_t src_pid,
                                         dacs_remote_mem_t *mem);

extern DACS_ERR_T dacs_remote_mem_release(dacs_remote_mem_t *mem);

extern DACS_ERR_T dacs_remote_mem_destroy(dacs_remote_mem_t *mem);

extern DACS_ERR_T dacs_remote_mem_query(dacs_remote_mem_t      mem,
                                        DACS_REMOTE_MEM_ATTR_T attr,
                                        uint64_t               *value);

extern DACS_ERR_T dacs_put(dacs_remote_mem_t dst_remote_mem,
                           uint64_t          dst_remote_mem_offset,
                           void              *src_addr,
                           uint64_t          size,
                           dacs_wid_t        wid,
                           DACS_ORDER_ATTR_T order_attr,
                           DACS_BYTE_SWAP_T  swap);

extern DACS_ERR_T dacs_get(void              *dst_addr,
                           dacs_remote_mem_t src_remote_mem,
                           uint64_t          src_remote_mem_offset,
                           uint64_t          size,
                           dacs_wid_t        wid,
                           DACS_ORDER_ATTR_T order_attr,
                           DACS_BYTE_SWAP_T  swap);

extern DACS_ERR_T dacs_put_list(dacs_remote_mem_t dst_remote_mem,
                                dacs_dma_list_t   *dst_dma_list,
                                uint32_t          dst_list_size,
                                void              *src_addr,
                                dacs_dma_list_t   *src_dma_list,
                                uint32_t          src_list_size,
                                dacs_wid_t        wid,
                                DACS_ORDER_ATTR_T order_attr,
                                DACS_BYTE_SWAP_T  swap);

extern DACS_ERR_T dacs_get_list(void              *dst_addr,
                                dacs_dma_list_t   *dst_dma_list,
                                uint32_t          dst_list_size,
                                dacs_remote_mem_t src_remote_mem,
                                dacs_dma_list_t   *src_dma_list,
                                uint32_t          src_list_size,
                                dacs_wid_t        wid,
                                DACS_ORDER_ATTR_T order_attr,
                                DACS_BYTE_SWAP_T  swap);

extern DACS_ERR_T dacs_errhandler_reg(dacs_error_handler_t handler,
                                      uint32_t             flags);

extern const char *dacs_strerror(DACS_ERR_T errcode);
extern DACS_ERR_T dacs_error_num(dacs_error_t error);
extern DACS_ERR_T dacs_error_code(dacs_error_t error, 
                                  uint32_t *code);
extern DACS_ERR_T dacs_error_str(dacs_error_t error, 
                                 const char **errstr);
extern DACS_ERR_T dacs_error_de(dacs_error_t error, 
                                de_id_t *de);
extern DACS_ERR_T dacs_error_pid(dacs_error_t error, 
                                 dacs_process_id_t *pid);

extern DACS_ERR_T dacs_group_init(dacs_group_t *group,
                                  uint32_t     flags);

extern DACS_ERR_T dacs_group_add_member(de_id_t           de,
                                        dacs_process_id_t pid,
                                        dacs_group_t      group);

extern DACS_ERR_T dacs_group_close(dacs_group_t group);

extern DACS_ERR_T dacs_group_destroy(dacs_group_t *group);

extern DACS_ERR_T dacs_group_accept(de_id_t           de,
                                    dacs_process_id_t pid,
                                    dacs_group_t      *group);

extern DACS_ERR_T dacs_group_leave(dacs_group_t *group);

extern DACS_ERR_T dacs_mailbox_read(uint32_t          *msg,
                                    de_id_t           src_de,
                                    dacs_process_id_t src_pid);


extern DACS_ERR_T dacs_mailbox_write(uint32_t          *msg,
                                     de_id_t           dst_de,
                                     dacs_process_id_t dst_pid);


extern DACS_ERR_T dacs_mailbox_test(DACS_TEST_MAILBOX_T rw_flag,
                                    de_id_t             de,
                                    dacs_process_id_t   pid,
                                    int32_t             *mbox_status);

extern DACS_ERR_T dacs_de_start (de_id_t                   de,
                                 void                      *text,
                                 char const                **argv,
                                 char const                **envv,
                                 DACS_PROC_CREATION_FLAG_T creation_flags,
                                 dacs_process_id_t         *pid);


extern DACS_ERR_T dacs_num_processes_supported(de_id_t  de,
                                               uint32_t *num_processes);


extern DACS_ERR_T dacs_num_processes_running(de_id_t  de,
                                             uint32_t *num_processes);

extern DACS_ERR_T dacs_de_wait(de_id_t           de,
                               dacs_process_id_t pid,
                               int32_t           *exit_status);

extern DACS_ERR_T dacs_de_test(de_id_t           de,
                               dacs_process_id_t pid,
                               int32_t           *exit_status);


extern DACS_ERR_T dacs_de_kill(de_id_t           de,
                               dacs_process_id_t process_id,DACS_KILL_TYPE_T kill_type);

extern DACS_ERR_T dacs_mutex_init (dacs_mutex_t *mutex);

extern DACS_ERR_T dacs_mutex_share (de_id_t           dst_de,
                                    dacs_process_id_t dst_pid,
                                    dacs_mutex_t      mutex);

extern DACS_ERR_T dacs_mutex_accept (de_id_t           remote_de,
                                     dacs_process_id_t remote_pid,
                                     dacs_mutex_t      *received_mutex);

extern DACS_ERR_T dacs_mutex_lock (dacs_mutex_t mutex);

extern DACS_ERR_T dacs_mutex_try_lock (dacs_mutex_t mutex);

extern DACS_ERR_T dacs_mutex_unlock (dacs_mutex_t mutex);

extern DACS_ERR_T dacs_mutex_release (dacs_mutex_t *mutex);

extern DACS_ERR_T dacs_mutex_destroy (dacs_mutex_t *mutex);

extern DACS_ERR_T dacs_barrier_wait (dacs_group_t group);

extern DACS_ERR_T dacs_runtime_init(void *, void *) __attribute__ ((deprecated));

extern DACS_ERR_T dacs_init(int config_flags);

extern DACS_ERR_T dacs_runtime_exit(void) __attribute__ ((deprecated));

extern DACS_ERR_T dacs_exit(void);

extern DACS_ERR_T dacs_recv(void              *dst_data,
                            uint32_t          len,
                            de_id_t           src_de,
                            dacs_process_id_t src_pid,
                            uint32_t          stream,
                            dacs_wid_t        wid,
                            DACS_BYTE_SWAP_T  swap);

extern DACS_ERR_T dacs_send(void              *src_data,
                            uint32_t          len,
                            de_id_t           dst_de,
                            dacs_process_id_t dst_pid,
                            uint32_t          stream,
                            dacs_wid_t        wid,
                            DACS_BYTE_SWAP_T  swap);

extern DACS_ERR_T dacs_get_num_avail_children( DACS_DE_TYPE_T  type,
                                                 uint32_t       *num_children);

extern DACS_ERR_T dacs_reserve_children(DACS_DE_TYPE_T type,
                                        uint32_t       *num_children,
                                        de_id_t        *de_list);

extern DACS_ERR_T dacs_release_de_list(uint32_t num_des,
                                       de_id_t  *de_list);
    
extern DACS_ERR_T dacs_wid_reserve(dacs_wid_t *wid);

extern DACS_ERR_T dacs_wid_release(dacs_wid_t *wid);

extern DACS_ERR_T dacs_mem_create(void *   addr,
                                  uint64_t size,
                                  DACS_MEM_ACCESS_MODE_T rmt_access_mode,
                                  DACS_MEM_ACCESS_MODE_T lcl_access_mode,
                                  dacs_mem_t * mem);

extern DACS_ERR_T dacs_mem_share(de_id_t dst_de,
                                 dacs_process_id_t dst_pid,
                                 dacs_mem_t mem);

extern DACS_ERR_T dacs_mem_register(de_id_t dst_de,
                                    dacs_process_id_t dst_pid,
                                    dacs_mem_t mem);

extern DACS_ERR_T dacs_mem_deregister(de_id_t dst_de,
                                      dacs_process_id_t dst_pid,
                                      dacs_mem_t mem);

extern DACS_ERR_T dacs_mem_accept(de_id_t src_de,
                                  dacs_process_id_t src_pid,
                                  dacs_mem_t * mem);

extern DACS_ERR_T dacs_mem_release(dacs_mem_t * mem);

extern DACS_ERR_T dacs_mem_destroy(dacs_mem_t * mem);

extern DACS_ERR_T dacs_mem_query(dacs_mem_t mem,
                                 DACS_MEM_ATTR_T attr,
                                 uint64_t * value);

extern DACS_ERR_T dacs_mem_limits_query(DACS_MEM_LIMITS_T attr,
                                        de_id_t tgt_de,
                                        dacs_process_id_t tgt_pid,
                                        uint64_t *value);

extern DACS_ERR_T dacs_mem_put(dacs_mem_t        dst_remote_mem,
                               uint64_t          dst_remote_mem_offset,
                               dacs_mem_t        src_local_mem,
                               uint64_t          src_local_mem_offset,
                               uint64_t          size,
                               dacs_wid_t        wid,
                               DACS_ORDER_ATTR_T order_attr,
                               DACS_BYTE_SWAP_T  swap);

extern DACS_ERR_T dacs_mem_get(dacs_mem_t        dst_local_mem,
                               uint64_t          dst_local_mem_offset,
                               dacs_mem_t        src_remote_mem,
                               uint64_t          src_remote_mem_offset,
                               uint64_t          size,
                               dacs_wid_t        wid,
                               DACS_ORDER_ATTR_T order_attr,
                               DACS_BYTE_SWAP_T  swap);

extern DACS_ERR_T dacs_mem_put_list(dacs_mem_t       dst_remote_mem,
                                    dacs_dma_list_t   *dst_dma_list,
                                    uint32_t          dst_list_size,
                                    dacs_mem_t        src_local_mem,
                                    dacs_dma_list_t   *src_dma_list,
                                    uint32_t          src_list_size,
                                    dacs_wid_t        wid,
                                    DACS_ORDER_ATTR_T order_attr,
                                    DACS_BYTE_SWAP_T  swap);

extern DACS_ERR_T dacs_mem_get_list(dacs_mem_t       dst_local_mem,
                                    dacs_dma_list_t   *dst_dma_list,
                                    uint32_t          dst_list_size,
                                    dacs_mem_t        src_remote_mem,
                                    dacs_dma_list_t   *src_dma_list,
                                    uint32_t          src_list_size,
                                    dacs_wid_t        wid,
                                    DACS_ORDER_ATTR_T order_attr,
                                    DACS_BYTE_SWAP_T  swap);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //_DACS_H_
