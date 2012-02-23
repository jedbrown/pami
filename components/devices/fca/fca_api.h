/**
* Copyright (C) Mellanox Technologies Ltd. 2001-2011.  ALL RIGHTS RESERVED.
* This software product is a proprietary product of Mellanox Technologies Ltd.
* (the "Company") and all right, title, and interest and to the software product,
* including all associated intellectual property rights, are and shall
* remain exclusively with the Company.
*
* This software product is governed by the End User License Agreement
* provided with the software product.
* $COPYRIGHT$
* $HEADER$
*/





#ifndef __FCA_API_H__
#define __FCA_API_H__

#include "components/devices/fca/core/fca_errno.h"
#include "components/devices/fca/fca_version.h"

/* Element types */
enum fca_element_type_t {
	FCA_ELEMENT_ANY         = 0x00,
	FCA_ELEMENT_RANK        = 0x01,   /* MPI rank process */
	FCA_ELEMENT_ICPU        = 0x02,   /* ICPU kernel mode poller */
	FCA_ELEMENT_FMM         = 0x04    /* FMM process */
};


/* 
 * Reduce operation data type
 * We encode the types to enable quickly getting the size of the type  
 * The type is 4 bits, 2 top bits are reserved for the size:
 * sizeof(type) = type & 0x0C
 */
enum fca_reduce_dtype_t {

	FCA_DTYPE_CHAR          = 1,  /* 8 bit signed integer */
	FCA_DTYPE_SHORT         = 2,  /* 16 bit signed integer */
	FCA_DTYPE_INT           = 3,  /* 32 bit signed integer */
	FCA_DTYPE_LONG          = 4,  /* 64 bit signed integer */

	FCA_DTYPE_UNSIGNED_CHAR = 5,  /* 8 bit unsigned integer */
	FCA_DTYPE_UNSIGNED_SHORT= 6,  /* 16 bit unsigned integer */
	FCA_DTYPE_UNSIGNED      = 7,  /* 32 bit unsigned integer */
	FCA_DTYPE_UNSIGNED_LONG = 8,  /* 64 bit unsigned integer */

	FCA_DTYPE_FLOAT         = 9,  /* 32 bit floating point */
	FCA_DTYPE_DOUBLE        = 10, /* 64 bit floating point */

	FCA_DTYPE_SHORT_INT     = 11, /* 16 bit signed integer + 32 bit signed integer */
	FCA_DTYPE_2INT          = 12, /* two 32 bit signed integers */
	FCA_DTYPE_FLOAT_INT     = 13, /* 32 bit floating point + 32 bit signed integer */
	FCA_DTYPE_LONG_INT      = 14, /* 64 bit signed integer + 32 bit signed integer */
	FCA_DTYPE_DOUBLE_INT    = 15, /* 64 bit floating point + 32 bit signed integer */

	FCA_DTYPE_LAST          = 15  /* Marks the last data type */
};


/* Reduce binary operator */
enum fca_reduce_op_t {
	FCA_OP_MAX        = 2,           /* Maximum */
	FCA_OP_MIN        = 3,           /* Minimum */
	FCA_OP_SUM        = 4,           /* Sum */
	FCA_OP_PROD       = 5,           /* Product */
	FCA_OP_LAND       = 6,           /* Logical AND */
	FCA_OP_BAND       = 7,           /* Bitwise AND */
	FCA_OP_LOR        = 8,           /* Logical OR */
	FCA_OP_BOR        = 9,           /* Bitwise OR */
	FCA_OP_LXOR       = 10,          /* Logical XOR */
	FCA_OP_BXOR       = 11,          /* Bitwise XOR */
	FCA_OP_MAXLOC     = 12,          /* Maximum with location */
	FCA_OP_MINLOC     = 13,          /* Minimum with location */

	FCA_OP_LAST       = 14           /* Marks the last data operation */
};


/**
 * Closure which specifies a callback.
 */
typedef struct fca_callback {
	void                       (*func)(void *);          /* Function pointer */
	void                       *arg; /* user argument passed to the function */
} fca_callback_t;


/* Forward declarations */
typedef struct fca_context fca_t;
typedef struct fca_rank_comm fca_comm_t;


/* Specifies a fabric element multicast address */
typedef struct fca_mcast_addr {
	int                        type;
	unsigned char              data[32];
} fca_mcast_addr_t;


/* Specifies how to create a new fabric communicator (rank0 only) */
typedef struct fca_comm_new_spec {
	void                       *rank_info;
	int                        rank_count;
	int                        is_comm_world;
} fca_comm_new_spec_t;


/* Communicator description, passed in COMM_NEW stage from rank0 to other ranks */
typedef struct fca_comm_desc {
	fca_mcast_addr_t           comm_maddr;
	int                        comm_id;
	unsigned long              job_id;
} fca_comm_desc_t;


/* Communicator capabilities */
typedef struct fca_comm_caps {
	int                        max_payload;                 /* Max payload */
	int                        max_nofrag_payload; /* Max for single packet */
} fca_comm_caps_t;


/* Specifies how to create a communicator (all ranks) */
typedef struct fca_comm_init_spec {
	fca_comm_desc_t            desc;   /* Return value from fca_comm_new() */
	int                        rank;                         /* Rank index */
	int                        size;             /* Communicator size */
	int                        proc_idx;   /* Index of local process */
	int                        num_procs; /* Number of local processes */
} fca_comm_init_spec_t;


/* FCA collectives for fules */
enum fca_collective_id {
	FCA_COLL_ID_REDUCE,
	FCA_COLL_ID_ALLREDUCE,
	FCA_COLL_ID_BCAST,
	FCA_COLL_ID_ALLGATHER,
	FCA_COLL_ID_ALLGATHERV,
	FCA_COLL_ID_BARRIER,

	FCA_COLL_ID_MAX
};


/* FCA offload type */
enum fca_offload_type {
	FCA_OFFLOAD_UD = 0,     /* Use UD messaging */
	FCA_OFFLOAD_CD = 1,     /* Use Core-direct offload */
	FCA_OFFLOAD_NONE = 2   /* Don't use FCA - return -EUSEMPI */
};


/* Rule Struct */
typedef struct fca_rule {
	enum fca_collective_id  coll_id;         /* collective type this rule applies to */
	int                     rule_id;         /* rule id */
	int                     comm_size_min;   /* minimum communicator size to activate this rule */
	int                     comm_size_max;   /* maximum communicator size to activate this rule */
	int                     msg_size_min;    /* minimum message size to use this rule for */
	int                     msg_size_max;    /* maximum message size to use this rule for */
	enum fca_offload_type   offload_type;    /* offload type to use for action */
	struct fca_rule         *next;           /* link to the next rule */
} fca_rule_t;


/*
 * FCA configuration
 * All time values are in milliseconds.
 */
typedef struct fca_config {

	char                       *element_name;/* Element name for debugging */

	/*
	 * Log settings
	 */
	struct {
		int                level;               /* Initial debug level */
		int                filename_cache;          /* Log buffer size */
		char               *filename;                  /* Log filename */
		char               *layout;                     /* Layout name */
		char               *membuf_filename;   /* Circular buffer file */
		int                membuf_size;        /* Circular buffer size */
		int                enable_stdout;            /* Log to stdout? */
		int                log_file_max_size; /* Max size of every log file. 0 means unlimited */
		int                log_max_backup_files;
	} log;

	/*
	 * Collective operations
	 */
	struct {
		int                tree_flags;

		/* Resends are done in the following intervals:
		 * start, start*backoff, start*backoff*backoff, ...., max
		 */
		int                resend_start;    /* Initial resend interval */
		float              resend_backoff; /* Scale of resend interval */
		int                resend_max;      /* Maximal resend interval */

		/* Threshold for sending NACKs */
		int                resend_thresh;

		/* Timeout for collective operation */
		int                timeout;

		/* Enable mathematical consistency for floating point allreduce sum */
		int                fp_sum_fixedpoint;

		/* Force floating point allreduce sum rank order */
		int                fp_sum_forceorder;

		/* Local reduce above this size will involve all ranks */
		int                smp_reduce_thresh;

		/* Maximal data for each route, TODO const max */
		int                route_max_data[3];

		/* Offload type for flow control barrier */
		enum fca_offload_type  flow_control_offload;

		/* Use UD QPs in CoreDirect.
		 * Make it disabled until CX2 HCA correctly supports it */
		int                core_direct_ud;

		/* Number of shmem polls before going slow */
		int                slow_num_polls;

		/* Number of microseconds to sleep in slow phase */
		int				   slow_sleep;
	} coll;

	/*
	 * Protocols
	 */
	struct {
		int                interval;      /* Interval between retries */
		int                retries;                   /* Max. retries */
	} comm_new, comm_init, comm_release, comm_end;

	/*
	 * Network device
	 */
	struct {
		int                recv_queue_len;    /* Receive queue length */
		int                send_queue_len;       /* Send queue length */
		int                recv_mtu;                   /* Receive MTU */
		int                send_mtu;                      /* Send MTU */
		int                fast_send_mtu;  /* Send smaller than this will be faster */
		int                fabric_timeout;  /* Timeout for fabric operations */
		int                recv_drop_rate;  /* drop one of X received packets (0: disabled) */
		int                send_drop_rate;  /* drop one of X send packets (0: disabled) */
		int                service_level;  /* QOS level for outgoing packets */
		int                sr_retries; /* Number of retries when writing/quering service records */
	} device;

	/*
	 * Statistics
	 */
	struct {
		 int                enable;               /* Enable statistics */
		 int                max_ops;    /* How many operations to save */
		 char               *filename;         /* Statistics file name */
	} stats;
	char                        *fmm_mcast_addr;  /* Use mcast to find FMM */

	/*
	 * Rules
	 */	                                                                                         
	struct {
		int                 enable;  /* Enable/disable rules mechanism */
		fca_rule_t          *rules[FCA_COLL_ID_MAX];   /* Rules' Array */
	} rules;
	
} fca_config_t;


/*
 * Specifies how to create the FCA engine.
 * 
 * NOTE
 * 1. 'element_name' is used for logging purposes. Host name is a good choice
 *    for element name.
 * 2. If 'dev_selector' is NULL, default device / port are used.
 */
typedef struct fca_init_spec {
	enum fca_element_type_t  element_type;      /* Type of fabric element */
	int                      job_id;                        /* MPI Job ID */
	int                      rank_id;                      /* MPI Rank ID */
	fca_callback_t           progress;  /* Argument for progress callback */
	void                     *dev_selector;  /* Transport device selector */
	fca_config_t             config;   /* FCA Configuration. See `fca_default_config'. */
} fca_init_spec_t;


/*
 * Specifies a reduce/allreduce operation
 */
typedef struct fca_reduce_spec {
	int                      root;     /* root rank number (ignored for allreduce) */
	void                     *sbuf;    /* data to submit */
	void                     *rbuf;    /* buffer to receive the result */
	int                      buf_size; /* Deprecated - ignored */
	enum fca_reduce_dtype_t  dtype;    /* data type */
	int                      length;   /* operation size */
	enum fca_reduce_op_t     op;       /* operator */
} fca_reduce_spec_t;


/*
 * Specifies a bcast operation
 */
typedef struct fca_bcast_spec {
	int                      root;     /* root rank number */
	void                     *buf;     /* buffer to send/receive the result */
	int                      size;     /* buffer size */
} fca_bcast_spec_t;


/*
 * Specifies a gather/allgather operation
 */
typedef struct fca_gather_spec {
	int                      root;     /* root rank number (ignored for allgather) */
	void                     *sbuf;    /* data to submit */
	void                     *rbuf;    /* buffer to receive the result */
	int                      size;     /* how much to distribute from each rank */
} fca_gather_spec_t;


/*
 * Specifies a gatherv/allgatherv operation
 */
typedef struct fca_gatherv_spec {
	int                      root;     /* root rank number (ignored for allgatherv) */
	void                     *sbuf;    /* data to submit */
	void                     *rbuf;    /* buffer to receive the result */
	int                      sendsize; /* how much to send from each rank */
	int                      *recvsizes; /* array of how much to receive from each rank
	                                      must have one element per rank. */
	int                      *displs;  /* array of displacements to place messages from
	                                      each rank. */
} fca_gatherv_spec_t;


/**
 * Default FCA configuration.
 */
extern const fca_config_t fca_default_config;


/**
 * Get library version.
 * 
 * @return: Numerical library version.  
 */
unsigned long fca_get_version(void);

/**
 * Get library version as string
 * 
 * @return: Numerical library version.  
 */
const char *fca_get_version_string(void);

/**
 * Initialize FCA library and create FCA context.
 * Typically done once, during process startup.
 * 
 * @dev_spec: Defines which device to use for communications. 
 *            Passed as-is to fca_dev_open().
 * @mng_maddr: Specifies a multicast address for management.
 * @element_name: Fabric element name, for debugging purposes.
 * @element_type: Fabric element type.
 * @packet_handler: Handler function for unsupported packet types.
 * @handler_arg: Argument for packet handler.
 * @fca_context: Context structure, NULL on error
 * @return 0 if ok, or < 0 if error.
 */
int fca_init(struct fca_init_spec *fca_spec, fca_t **fca_context);

/**
 * Cleanup FCA.
 * Typically done once, just before the process ends.
 * 
 * @context: FCA context to cleanup.
 */
void fca_cleanup(fca_t *context);

/**
 * Progress FCA operations.
 * Typically called from MPI progress context.
 *
 * @context: FCA context to progress.
 */
void fca_progress(fca_t *context);

/**
 * Get rank information.
 * Fills a buffer with information that can be passed to fca_ask_comm().
 * Typically called from each MPI rank that wants to create a new communicator,
 * but only one selected rank will talk to the FMM. This information is passed
 * to this selected rank.
 * 
 * @context: FCA context.
 * @buf_len: Buffer length
 * @return: Buffer with rank information.
 * 
 * NOTE: The caller MUST call fca_free_rank_info() to free the returned data.
 */
void *fca_get_rank_info(fca_t *context, int *buf_len);

/**
 * Free rank information block returned by 'fca_get_rank_info'.
 * 
 * @rank_info: Rank information block to free.
 */
void fca_free_rank_info(void *rank_info);

/**
 * Ask the FMM for new communicator.
 * Typically called by one selected MPI rank to create a communicator on behalf
 * of all other ranks in the group.
 * 
 * @context: FCA context.
 * @spec: comm_new parameters.
 * @comm_desc: Filled with communicator description, to pass to fca_comm_init().
 *
 * @return 0 if OK, or < 0 if error.
 */
int fca_comm_new(fca_t *context, fca_comm_new_spec_t *spec,
                 fca_comm_desc_t *comm_desc);

/**
 * Ask the FMM to close the communicator.
 * Typically called by one selected MPI rank to close the communicator on behalf
 * of all other ranks.
 * Each call to 'fca_comm_new' should match a previous call to 'fca_comm_end'.
 * 
 * @context: FCA context.
 * @fca_comm: FCA Communicator.
 * @fmm_maddr: FMM discovery multicast.
 *
 * @return 0 if OK, or < 0 if error.
 */
int fca_comm_end(fca_t *context, int comm_id);

/**
 * Create new communicator according to information received from FMM.
 * Should be called by every rank in the communicator group, using the 'comm_desc'
 * returned from 'fca_comm_new'. The caller should make sure that the 'comm_desc'
 * structure is distributed from the rank which called 'fca_comm_new' to the other
 * ranks in the group.
 * 
 * @context: FCA context.
 * @spec: Specifies how to create the communicator.
 * @fca_comm: Filled with communicator object.
 *
 * @return 0 if OK, or < 0 if error.
 */
int fca_comm_init(fca_t *context, fca_comm_init_spec_t *spec, fca_comm_t **fca_comm);

/**
 * Destroy communicator returned from 'fca_comm_init'.
 * Each call to 'fca_comm_destroy' should match a prevoius call to fca_comm_new.
 * 
 * @fabric_comm: Communicator to destroy.
 */
void fca_comm_destroy(fca_comm_t *comm);

/**
 * Returns communicator capabilities.
 * 
 * @fabric_comm: FCA communicator.
 * @caps: Filled with communicator capabilities.
 * @return: number on success, < 0 on failure.
 */
int fca_comm_get_caps(fca_comm_t *comm, fca_comm_caps_t *caps);

/**
 * Run a fabric REDUCE operation.
 * Typically called from MPI collective operation context.
 * 
 * @comm: Communicator to perform the operation on.
 * @spec: Operation specification.
 * @return: 0 on success, < 0 on failure.
 */
int fca_do_reduce(fca_comm_t *comm, fca_reduce_spec_t *spec);

/**
 * Run a fabric ALLREDUCE operation.
 * Typically called from MPI collective operation context.
 * 
 * @spec: Operation specification.
 * @return: 0 on success, < 0 on failure.
 */
int fca_do_all_reduce(fca_comm_t *comm, fca_reduce_spec_t *spec);

/**
 * Run a fabric BROADCAST operation.
 * Typically called from MPI collective operation context.
 *
 * @spec: Broadcast operation.
 * @return: 0 on success, < 0 on failure.
 */
int fca_do_bcast(fca_comm_t *comm, fca_bcast_spec_t *spec);

/**
 * Run a fabric ALLGATHER operation.
 *
 * @comm: Communicator to run the allgather on.
 * @spec: Operation specification. See its documentation above.
 * @return: 0 on success, < 0 on failure.
 */
int fca_do_allgather(fca_comm_t *comm, fca_gather_spec_t *spec);

/**
 * Run a fabric ALLGATHERV operation.
 *
 * @comm: Communicator to run the allgatherv on.
 * @spec: Operation specification. See its documentation above.
 * @return: 0 on success, < 0 on failure.
 */
int fca_do_allgatherv(fca_comm_t *comm, fca_gatherv_spec_t *spec);

/**
 * Run a fabric BARRIER operation.
 * Typically called from MPI collective operation context.
 *
 * @comm: Communicator to run the barrier on.
 * @return: 0 on success, < 0 on failure.
 */
int fca_do_barrier(fca_comm_t *comm);

/**
 * Check if collective operation is supported, and if it is - return the FCA
 * operation ID.
 * Typically called from MPI context.
 * 
 * @mpi_op_str: MPI Operation name.
 * @return: fca_reduce_op_t on success, < 0 on failure.
 */
int fca_translate_mpi_op(char *mpi_op_str);

/**
 * Check if a data type  is supported, and if it is return the FCA data type ID.
 * Typically called from MPI context.
 * 
 * @spec: Operation specification.
 * @return: fca_reduce_dtype_t on success, < 0 on failure.
 */
int fca_translate_mpi_dtype(char *mpi_dtype_str);

/**
 * Get the size of the data type in FCA internal representation.
 * This can be used to determine whether an array of elements will fit into a
 * communicator max. data:
 *
 *       fca_comm_get_caps(comm, &caps);
 *       if (fca_get_dtype_size(fca_dtype) * length <= caps.max_nofrag_payload)
 *             ... use FCA for collective operation ...
 *       else
 *             ... fall back to original implementation ...
 *
 * @dtype: FCA data type.
 * @return: Datatype size, of -EINVAL if data type is invalid.
 */
int fca_get_dtype_size(enum fca_reduce_dtype_t dtype);

#endif
