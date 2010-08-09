/**
 * \file sys/pami_ext.h
 * \brief platform-specific messaging interface
 */

#include "pami.h"

#ifndef __pami_ext_h__
#define __pami_ext_h__

#ifndef PAMI_EXT_ATTR
#define PAMI_EXT_ATTR 1000 /**< starting value for extended attributes */
#endif

typedef struct pami_quad_t
{
    unsigned w0; /**< Word[0] */
    unsigned w1; /**< Word[1] */
    unsigned w2; /**< Word[2] */
    unsigned w3; /**< Word[3] */
}
pami_quad_t __attribute__ ((__aligned__ (16)));

typedef struct {
  int x; /**< X value */
  int y; /**< Y value */
  int z; /**< Z value */
} pami_coordinates_t;

typedef enum {
  /* Attribute                    Init / Query / Update    */
  PAMI_COORDINATES = PAMI_EXT_ATTR, /** Q : pami_coordinates_t: coordinates of this task */
} pami_attribute_ext_t;



  /**
   * \brief Network type
   */
  typedef enum
  {
    PAMI_DEFAULT_NETWORK = 0, /**< Default network type. \b Guaranteed to work. */
    PAMI_N_TORUS_NETWORK,     /**< nD-Torus / nD-SMP network type.
                              * mapping->torusDims() for torus dim,
                              * mapping->globalDims() for all (torus+SMP) dim.
                              */
    PAMI_SOCKET_NETWORK,      /**< Unix socket network type. */
    PAMI_SHMEM_NETWORK,       /**< local shared memory "network" for smp nodes. */
    PAMI_NETWORK_COUNT        /**< Number of network types defined. */
  }
    pami_network;

/** \todo Remove this platform-specific #define */
#define PAMI_MAX_DIMS 7
/* #define PAMI_MAX_DIMS	4 */

  /**
   * \brief A structure to describe a network coordinate
   */
  typedef struct
  {
    pami_network network; /**< Network type for the coordinates */
    union
    {
      struct
      {
        size_t coords[PAMI_MAX_DIMS];
      } n_torus;
      struct
      {
        int recv_fd;   /**< Receive file descriptor */
        int send_fd;   /**< Send file descriptor    */
      } socket;   /**< PAMI_SOCKET_NETWORK coordinates */
      struct
      {
        size_t rank;   /**< Global task id of process */
        size_t peer;   /**< Local task id of process */
      } shmem;    /**< PAMI_SHMEM_NETWORK coordinates */
    } u;
  } pami_coord_t;


  typedef pami_quad_t pami_pipeworkqueue_t[8];
  typedef pami_quad_t pami_pipeworkqueue_ext_t[2];

  /**
   * \brief Map a task to a network address expressed as coordinates.
   *
   * \param[in] task	Id of the rank or task.
   * \param[out] ntw	coordinate object to be filled out.
   */
  pami_result_t PAMI_Task2Network(pami_task_t task,
                                  pami_coord_t *ntw);

  /**
   * \brief Map a network address expressed as coordinates into a task.
   *
   * \param[in] ntw	coordinate object to map to a task.
   * \param[out] task	Id of the rank or task to be assigned.
   */
  pami_result_t PAMI_Network2Task(pami_coord_t ntw,
                                  pami_task_t *task);

  /**
   * \brief Configure for Shared Circular Buffer variety.
   *
   * Only one consumer and producer are allowed.
   * Creates a circular buffer of specified size in shared memory.
   * Buffer size must be power-of-two.
   *
   * \param[out] wq	Opaque memory for PipeWorkQueue
   * \param[in] bufsize	Size of buffer to allocate
   */
  void PAMI_PipeWorkQueue_config_circ(pami_pipeworkqueue_t *wq, size_t bufsize);

  /**
   * \brief Configure for User-supplied Circular Buffer variety.
   *
   * Only one consumer and producer are allowed.
   * Uses the provided buffer as a circular buffer of specified size.
   * Buffer size must be power-of-two.
   * Assumes the caller has placed buffer and (this) in appropriate memory
   * for desired use - i.e. all in shared memory if to be used beyond this process.
   *
   * NOTE: details need to be worked out. The buffer actually needs to include the
   * WQ header, so the caller must somehow know how much to allocate memory -
   * and how to ensure desired alignment.
   *
   * \param[out] wq		Opaque memory for PipeWorkQueue
   * \param[in] buffer		Buffer to use
   * \param[in] bufsize	Size of buffer
   */
  void PAMI_PipeWorkQueue_config_circ_usr(pami_pipeworkqueue_t *wq, char *buffer, size_t bufsize);

  /**
   * \brief Configure for Memory (flat buffer) variety.
   *
   * Only one consumer and producer are allowed. Still supports pipelining.
   * Sets up a flat buffer of specified maximum size with an arbitrary "initial fill".
   * Assumes the caller has placed buffer and (this) in appropriate memory
   * for desired use - i.e. all in shared memory if to be used beyond this process.
   *
   * \param[out] wq		Opaque memory for PipeWorkQueue
   * \param[in] buffer		Buffer to use
   * \param[in] bufsize	Size of buffer
   * \param[in] bufinit	Amount of data initially in buffer
   */
  void PAMI_PipeWorkQueue_config_flat(pami_pipeworkqueue_t *wq, char *buffer, size_t bufsize, size_t bufinit);

  /**
   * \brief PROPOSAL: Configure for Non-Contig Memory (flat buffer) variety.
   *
   * Only one consumer and producer are allowed. Still supports pipelining.
   * Sets up a flat buffer of specified maximum size with an arbitrary "initial fill".
   * Assumes the caller has placed buffer and (this) in appropriate memory
   * for desired use - i.e. all in shared memory if to be used beyond this process.
   *
   * This is typically only used for the application buffer, either input or output,
   * and so would not normally have both producer and consumer (only one or the other).
   * The interface is the same as for contiguous data except that "bytesAvailable" will
   * only return the number of *contiguous* bytes available. The user must consume those
   * bytes before it can see the next contiguous chunk.
   *
   * \param[out] wq            Opaque memory for PipeWorkQueue
   * \param[in] buffer         Buffer to use
   * \param[in] type           Memory layout of a buffer unit
   * \param[in] typecount      Number of repetitions of buffer units
   * \param[in] typeinit       Number of units initially in buffer
   */
  void PAMI_PipeWorkQueue_config_noncontig(pami_pipeworkqueue_t *wq, char *buffer, pami_type_t *type, size_t typecount, size_t typeinit);

  /**
   * \brief Export
   *
   * Produces information about the PipeWorkQueue into the opaque buffer "export".
   * This info is suitable for sharing with other processes such that those processes
   * can then construct a PipeWorkQueue which accesses the same data stream.
   *
   * This only has value in the case of a flat buffer PipeWorkQueue and a platform
   * that supports direct mapping of memory from other processes. Circular buffer
   * PipeWorkQueues use shared memory and are inherently inter-process in nature.
   *
   * The exporting process is the only one that can produce to the PipeWorkQueue.
   * All importing processes are restricted to consuming. There is no feedback
   * from the consumers to the producer - no flow control.
   *
   * \param[in] wq             Opaque memory for PipeWorkQueue
   * \param[out] export        Opaque memory to export into
   * \return	success of the export operation
   */
  pami_result_t PAMI_PipeWorkQueue_export(pami_pipeworkqueue_t *wq, pami_pipeworkqueue_ext_t *t_exp);

  /**
   * \brief Import
   *
   * Takes the results of an export of a PipeWorkQueue on a different process and
   * constructs a new PipeWorkQueue which the local process may use to access the
   * data stream.
   *
   * The resulting PipeWorkQueue may consume data, but that is a local-only operation.
   * The producer has no knowledge of data consumed. There can be only one producer.
   * There may be multiple consumers, but the producer does not know about them.
   * An importing processes cannot be the producer.
   *
   * TODO: can this work for circular buffers? does it need to, since those are
   * normally shared memory and thus already permit inter-process communication.
   *
   * \param[in] import        Opaque memory into which an export was done.
   * \param[out] wq           Opaque memory for new PipeWorkQueue
   * \return	success of the import operation
   */
  pami_result_t PAMI_PipeWorkQueue_import(pami_pipeworkqueue_ext_t *import, pami_pipeworkqueue_t *wq);

  /**
   * \brief Clone constructor.
   *
   * Used to create a second local memory wrapper object of the same
   * shared memory resource.
   *
   * \see WorkQueue(WorkQueue &)
   *
   * \param[out] wq	Opaque memory for new PipeWorkQueue
   * \param[in] obj	old object, to be cloned
   */
  void PAMI_PipeWorkQueue_clone(pami_pipeworkqueue_t *wq, pami_pipeworkqueue_t *obj);

  /**
   * \brief Destructor
   *
   * \param[out] wq	Opaque memory for PipeWorkQueue
   */
  void PAMI_PipeWorkQueue_destroy(pami_pipeworkqueue_t *wq);

  /**
   * \brief Reset this pipe work queue.
   *
   * All PipeWorkQueues must be reset() at least once after configure and before using.
   * (TODO: should this simply be part of configure?)
   *
   * Sets the number of bytes produced and the number of bytes
   * consumed to zero (or to "bufinit" as appropriate).
   *
   * This is typically required by circular PipeWorkQueues that are re-used.
   * Flat PipeWorkQueues are usually configured new for each instance
   * and thus do not require resetting. Circular PipeWorkQueues should be
   * reset by only one entity, and at a point when it is known that no other
   * entity is still using it (it must be idle). For example, in a multisend pipeline
   * consisting of:
   *
   *        [barrier] ; local-reduce -(A)-&gt; global-allreduce -(B)-&gt; local-broadcast
   *
   * the PipeWorkQueue "B" would be reset by the root of the reduce when starting the
   * local-reduce operation (when it is known that any prior instances have completed).
   *
   * One reason that a reset may be needed is to preserve buffer alignment. Another is
   * to prevent problems when, say, a consumer requires a full packet of data. In this
   * case, a circular PipeWorkQueue may have been left in a state from the previous
   * operation where the final chunk of data has left less than a packet length before
   * the wrap point. This would create a hang because the consumer would never see a full
   * packet until it consumes those bytes and passes the wrap point.
   *
   * Since resets are performed by the protocol code, it understands the context and
   * whether the PipeWorkQueue represents a flat (application) buffer or an intermediate
   * (circular) one.
   *
   * \param[out] wq	Opaque memory for PipeWorkQueue
   */
  void PAMI_PipeWorkQueue_reset(pami_pipeworkqueue_t *wq);

  /**
   * \brief Dump shared memory work queue statistics to stderr.
   *
   * \param[in] wq	Opaque memory for PipeWorkQueue
   * \param[in] prefix Optional character string to prefix.
   */
  void PAMI_PipeWorkQueue_dump(pami_pipeworkqueue_t *wq, const char *prefix);

  /**
   * \brief register a wakeup for the consumer side of the PipeWorkQueue
   *
   * The 'vec' parameter is typically obtained from some platform authority,
   * which is the same used by the PipeWorkQueue to perform the wakeup.
   * A consumer wishing to be awoken when data is available would call the
   * system to get their 'vec' value, and pass it to this method to register
   * for wakeups. When the produceBytes method is called, it will use this
   * consumer wakeup 'vec' to tell the system to wake up the consumer process or thread.
   *
   * \param[in] wq	Opaque memory for PipeWorkQueue
   * \param[in] vec	Opaque wakeup vector parameter
   */
  void PAMI_PipeWorkQueue_setConsumerWakeup(pami_pipeworkqueue_t *wq, void *vec);

  /**
   * \brief register a wakeup for the producer side of the PipeWorkQueue
   *
   * See setConsumerWakeup() for details.
   *
   * \param[in] wq	Opaque memory for PipeWorkQueue
   * \param[in] vec	Opaque wakeup vector parameter
   */
  void PAMI_PipeWorkQueue_setProducerWakeup(pami_pipeworkqueue_t *wq, void *vec);

  /**
   * \brief register user-defined info for producer
   *
   * Cooperating consumer/producers may share information using
   * these interfaces. The information is static, i.e. after a
   * call to setConsumerUserInfo() with values A and B, all subsequent
   * calls to getConsumerUserInfo() will return values A and B
   * (until another call to setConsumerUserInfo() is made).
   *
   * "Producer" and "Consumer" here are just convenience references,
   * either side (or potentially third-parties) may use these
   * methods as desired to set/get the info.
   *
   * \param[in] word1    First piece of info
   * \param[in] word2    Second piece of info
   */
  void PAMI_PipeWorkQueue_setProducerUserInfo(pami_pipeworkqueue_t *wq, void *word1, void *word2);

  /**
   * \brief register user-defined info for consumer
   * \param[in] word1    First piece of info
   * \param[in] word2    Second piece of info
   */
  void PAMI_PipeWorkQueue_setConsumerUserInfo(pami_pipeworkqueue_t *wq, void *word1, void *word2);

  /**
   * \brief get user-defined info for producer
   * \param[in] word1    First piece of info
   * \param[in] word2    Second piece of info
   */
  void PAMI_PipeWorkQueue_getProducerUserInfo(pami_pipeworkqueue_t *wq, void **word1, void **word2);

  /**
   * \brief get user-defined info for consumer
   * \param[in] word1    First piece of info
   * \param[in] word2    Second piece of info
   */
  void PAMI_PipeWorkQueue_getConsumerUserInfo(pami_pipeworkqueue_t *wq, void **word1, void **word2);

  /**
   * \brief Return the number of contiguous bytes that can be produced into this work queue.
   *
   * Bytes must be produced into the memory location returned by bufferToProduce() and then
   * this work queue \b must be updated with produceBytes().
   *
   * \see bufferToProduce
   * \see produceBytes
   *
   * \param[in] wq	Opaque memory for PipeWorkQueue
   * \return Number of bytes that may be produced.
   */
  size_t PAMI_PipeWorkQueue_bytesAvailableToProduce(pami_pipeworkqueue_t *wq);

  /**
   * \brief Return the number of contiguous bytes that can be consumed from this work queue.
   *
   * Bytes must be consumed into the memory location returned by bufferToConsume() and then
   * this work queue \b must be updated with consumeBytes().
   *
   * \see bufferToConsume
   * \see consumeBytes
   *
   * \param[in] wq	Opaque memory for PipeWorkQueue
   * \param[in] consumer Consumer id for work queues with multiple consumers
   *
   * \param[in] wq	Opaque memory for PipeWorkQueue
   * \return Number of bytes that may be consumed.
   */
  size_t PAMI_PipeWorkQueue_bytesAvailableToConsume(pami_pipeworkqueue_t *wq);

  /**
   * \brief raw accessor for total number of bytes produced since reset()
   *
   * \param[in] wq	Opaque memory for PipeWorkQueue
   * \return	number of bytes produced
   */
  size_t PAMI_PipeWorkQueue_getBytesProduced(pami_pipeworkqueue_t *wq);

  /**
   * \brief raw accessor for total number of bytes consumed since reset()
   *
   * \param[in] wq	Opaque memory for PipeWorkQueue
   * \return	number of bytes consumed
   */
  size_t PAMI_PipeWorkQueue_getBytesConsumed(pami_pipeworkqueue_t *wq);

  /**
   * \brief current position for producing into buffer
   *
   * \param[in] wq	Opaque memory for PipeWorkQueue
   * \return	location in buffer to produce into
   */
  char *PAMI_PipeWorkQueue_bufferToProduce(pami_pipeworkqueue_t *wq);

  /**
   * \brief notify workqueue that bytes have been produced
   *
   * \param[in] wq	Opaque memory for PipeWorkQueue
   * \return	number of bytes that were produced
   */
  void PAMI_PipeWorkQueue_produceBytes(pami_pipeworkqueue_t *wq, size_t bytes);

  /**
   * \brief current position for consuming from buffer
   *
   * \param[in] wq	Opaque memory for PipeWorkQueue
   * \return	location in buffer to consume from
   */
  char *PAMI_PipeWorkQueue_bufferToConsume(pami_pipeworkqueue_t *wq);

  /**
   * \brief notify workqueue that bytes have been consumed
   *
   * \param[in] wq	Opaque memory for PipeWorkQueue
   * \return	number of bytes that were consumed
   */
  void PAMI_PipeWorkQueue_consumeBytes(pami_pipeworkqueue_t *wq, size_t bytes);

  /**
   * \brief is workqueue ready for action
   *
   * \param[in] wq	Opaque memory for PipeWorkQueue
   * \return	boolean indicate workqueue readiness
   */
  int PAMI_PipeWorkQueue_available(pami_pipeworkqueue_t *wq);


  /** \brief The various types a Topology can be */
  typedef enum {
    PAMI_EMPTY_TOPOLOGY = 0, /**< topology represents no (zero) tasks    */
    PAMI_SINGLE_TOPOLOGY,    /**< topology is for one task               */
    PAMI_RANGE_TOPOLOGY,     /**< topology is a simple range of tasks    */
    PAMI_LIST_TOPOLOGY,      /**< topology is an unordered list of tasks */
    PAMI_COORD_TOPOLOGY,     /**< topology is a rectangular segment
                               represented by coordinates               */
    PAMI_AXIAL_TOPOLOGY,     /**< topology is a axial neighborhood --
                               represented by a rectangular seqment, a
                               reference task, and optional torus flags */
    PAMI_TOPOLOGY_COUNT
  } pami_topology_type_t;

  typedef pami_quad_t pami_topology_t[16];

  /**
   * \brief default constructor (PAMI_EMPTY_TOPOLOGY)
   *
   * \param[out] topo	Opaque memory for topology
   */
  void PAMI_Topology_create(pami_topology_t *topo);

  /**
   * \brief rectangular segment with torus (PAMI_COORD_TOPOLOGY)
   *
   * Assumes no torus links if no 'tl' param.
   *
   * \param[out] topo	Opaque memory for topology
   * \param[in] ll	lower-left coordinate
   * \param[in] ur	upper-right coordinate
   * \param[in] tl	optional, torus links flags
   */
  void PAMI_Topology_create_rect(pami_topology_t *topo,
                                pami_coord_t *ll, pami_coord_t *ur, unsigned char *tl);

  /**
   * \brief Axial topology (PAMI_AXIAL_TOPOLOGY)
   *
   * Assumes no torus links if 'tl' param = NULL.
   *
   * \param[out] topo	Opaque memory for topology
   * \param[in] ll	lower-left coordinate
   * \param[in] ur	upper-right coordinate
   * \param[in] ref	coordinates of the reference task where axes cross.
   * \param[in] tl	optional, torus links flags
   */
  void PAMI_Topology_create_axial(pami_topology_t *topo,
                                 pami_coord_t *ll,
                                 pami_coord_t *ur,
                                 pami_coord_t *ref,
                                 unsigned char *tl);

  /**
   * \brief single task constructor (PAMI_SINGLE_TOPOLOGY)
   *
   * \param[out] topo	Opaque memory for topology
   * \param[in] task	The task
   */
  void PAMI_Topology_create_task(pami_topology_t *topo, pami_task_t task);

  /**
   * \brief task range constructor (PAMI_RANGE_TOPOLOGY)
   *
   * \param[out] topo	Opaque memory for topology
   * \param[in] task0	first task in range
   * \param[in] taskn	last task in range
   */
  void PAMI_Topology_create_range(pami_topology_t *topo, pami_task_t task0, pami_task_t taskn);

  /**
   * \brief task list constructor (PAMI_LIST_TOPOLOGY)
   *
   * caller must not free tasks[]!
   *
   * \param[out] topo	Opaque memory for topology
   * \param[in] tasks	array of tasks
   * \param[in] ntasks	size of array
   *
   * \todo create destructor to free list, or establish rules
   */
  void PAMI_Topology_create_list(pami_topology_t *topo, pami_task_t *tasks, size_t ntasks);

  /**
   * \brief destructor
   *
   * For PAMI_LIST_TOPOLOGY, would free the tasks list array... ?
   *
   * \param[out] topo	Opaque memory for topology
   */
  void PAMI_Topology_destroy(pami_topology_t *topo);

  /**
   * \brief accessor for size of a Topology object
   *
   * \param[in] topo	Opaque memory for topology
   * \return	size of PAMI::Topology
   */
  unsigned PAMI_Topology_size_of(pami_topology_t *topo);

  /**
   * \brief number of tasks in topology
   * \param[in] topo	Opaque memory for topology
   * \return	number of tasks
   */
  size_t PAMI_Topology_size(pami_topology_t *topo);

  /**
   * \brief type of topology
   * \param[out] topo	Opaque memory for topology
   * \return	topology type
   */
  pami_topology_type_t pami_topology_type(pami_topology_t *topo);

  /**
   * \brief Nth task in topology
   *
   * \param[in] topo	Opaque memory for topology
   * \param[in] ix	Which task to select
   * \return	Nth task or (pami_task_t)-1 if does not exist
   */
  pami_task_t PAMI_Topology_index2TaskID(pami_topology_t *topo, size_t ix);

  /**
   * \brief determine index of task in topology
   *
   * This is the inverse function to task(ix) above.
   *
   * \param[in] topo	Opaque memory for topology
   * \param[in] task	Which task to get index for
   * \return	index of task (task(ix) == task) or (size_t)-1
   */
  size_t PAMI_Topology_taskID2Index(pami_topology_t *topo, pami_task_t task);

  /**
   * \brief return range
   *
   * \param[in] topo	Opaque memory for topology
   * \param[out] first	Where to put first task in range
   * \param[out] last	Where to put last task in range
   * \return	PAMI_SUCCESS, or PAMI_UNIMPL if not a range topology
   */
  pami_result_t PAMI_Topology_taskRange(pami_topology_t *topo, pami_task_t *first, pami_task_t *last);

  /**
   * \brief return task list
   *
   * \param[in] topo	Opaque memory for topology
   * \param[out] list	pointer to list stored here
   * \return	PAMI_SUCCESS, or PAMI_UNIMPL if not a list topology
   */
  pami_result_t PAMI_Topology_taskList(pami_topology_t *topo, pami_task_t **list);

  /**
   * \brief return rectangular segment coordinates
   *
   * This method copies data to callers buffers. It is safer
   * as the caller cannot directly modify the topology.
   *
   * \param[in] topo	Opaque memory for topology
   * \param[out] ll	lower-left coord pointer storage
   * \param[out] ur	upper-right coord pointer storage
   * \param[out] tl	optional, torus links flags
   * \return	PAMI_SUCCESS, or PAMI_UNIMPL if not a coord topology
   */
  pami_result_t PAMI_Topology_rectSeg(pami_topology_t *topo,
                                    pami_coord_t *ll, pami_coord_t *ur,
                                    unsigned char *tl);

  /**
   * \brief does topology consist entirely of tasks local to self
   *
   * \param[in] topo	Opaque memory for topology
   * \return boolean indicating locality of tasks
   */
  int PAMI_Topology_isLocalToMe(pami_topology_t *topo);

  /**
   * \brief is topology a rectangular segment
   * \param[in] topo	Opaque memory for topology
   * \return	boolean indicating rect seg topo
   */
  int PAMI_Topology_isRectSeg(pami_topology_t *topo);

  /**
   * \brief extract Nth dimensions from coord topology
   *
   * \param[in] topo	Opaque memory for topology
   * \param[in] n	Which dim to extract
   * \param[out] c0	lower value for dim range
   * \param[out] cn	upper value for dim range
   * \param[out] tl	optional, torus link flag
   */
  void PAMI_Topology_getNthDims(pami_topology_t *topo, unsigned n,
                               unsigned *c0, unsigned *cn, unsigned char *tl);

  /**
   * \brief is task in topology
   *
   * \param[in] topo	Opaque memory for topology
   * \param[in] task	Task to test
   * \return	boolean indicating task is in topology
   */
  int PAMI_Topology_isTaskMember(pami_topology_t *topo, pami_task_t task);

  /**
   * \brief is coordinate in topology
   *
   * \param[in] topo	Opaque memory for topology
   * \param[in] c0	Coord to test
   * \return	boolean indicating coord is a member of topology
   */
  int PAMI_Topology_isCoordMember(pami_topology_t *topo, pami_coord_t *c0);

  /**
   * \brief create topology of tasks local to self
   *
   * \param[out] _new	Where to build topology
   * \param[in] topo	Opaque memory for topology
   */
  void PAMI_Topology_sub_LocalToMe(pami_topology_t *_new, pami_topology_t *topo);

  /**
   * \brief create topology from all Nth tasks globally
   *
   * \param[out] _new	Where to build topology
   * \param[in] topo	Opaque memory for topology
   * \param[in] n	Which local task to select on each node
   */
  void PAMI_Topology_sub_NthGlobal(pami_topology_t *_new, pami_topology_t *topo, int n);

  /**
   * \brief reduce dimensions of topology (cube -> plane, etc)
   *
   * The 'fmt' param is a pattern indicating which dimensions
   * to squash, and what coord to squash into. A dim in 'fmt'
   * having "-1" will be preserved, while all others will be squashed
   * into a dimension of size 1 having the value specified.
   *
   * \param[out] _new	where to build new topology
   * \param[in] topo	Opaque memory for topology
   * \param[in] fmt	how to reduce dimensions
   */
  void PAMI_Topology_sub_ReduceDims(pami_topology_t *_new, pami_topology_t *topo, pami_coord_t *fmt);

  /**
   * \brief Return list of tasks representing contents of topology
   *
   * This always returns a list regardless of topology type.
   * Caller must allocate space for list, and determine an
   * appropriate size for that space. Note, there might be a
   * number larger than 'max' returned in 'ntasks', but there
   * are never more than 'max' tasks put into the array.
   * If the caller sees that 'ntasks' exceeds 'max' then it
   * should assume it did not get the whole list, and could
   * allocate a larger array and try again.
   *
   * \param[in] topo	Opaque memory for topology
   * \param[in] max	size of caller-allocated array
   * \param[out] tasks	array where task list is placed
   * \param[out] ntasks	actual number of tasks put into array
   */
  void PAMI_Topology_getTaskList(pami_topology_t *topo, size_t max, pami_task_t *tasks, size_t *ntasks);

  /**
   * \brief check if task range or list can be converted to rectangle
   *
   * Since a rectangular segment is consider the optimal state, no
   * other analysis is done. A PAMI_SINGLE_TOPOLOGY cannot be optimized,
   * either. Optimization levels:
   *
   * 	PAMI_SINGLE_TOPOLOGY (most)
   * 	PAMI_COORD_TOPOLOGY
   * 	PAMI_RANGE_TOPOLOGY
   * 	PAMI_LIST_TOPOLOGY (least)
   *
   * \param[in,out] topo	Opaque memory for topology
   * \return	'true' if topology was changed
   */
  int PAMI_Topology_analyze(pami_topology_t *topo);

  /**
   * \brief check if topology can be converted to type
   *
   * Does not differentiate between invalid conversions and
   * 'null' conversions (same type).
   *
   * \param[in,out] topo	Opaque memory for topology
   * \param[in] new_type	Topology type to try and convert into
   * \return	'true' if topology was changed
   */
  int PAMI_Topology_convert(pami_topology_t *topo, pami_topology_type_t new_type);

  /**
   * \brief produce the intersection of two topologies
   *
   * produces: _new = this ./\. other
   *
   * \param[out] _new	New topology created there
   * \param[in] topo	Opaque memory for topology
   * \param[in] other	The other topology
   */
  void PAMI_Topology_intersect(pami_topology_t *_new, pami_topology_t *topo, pami_topology_t *other);

  /**
   * \brief produce the difference of two topologies
   *
   * produces: _new = this .-. other
   *
   * \param[out] _new	New topology created there
   * \param[in] topo	Opaque memory for topology
   * \param[in] other	The other topology
   */
  void PAMI_Topology_subtract(pami_topology_t *_new, pami_topology_t *topo, pami_topology_t *other);

/**
 * \brief Multisend interfaces.
 *
 * A multisend operation allows many message passing transactions to
 * be performed in the same call to amortize software overheads.  It
 * has two flavors
 *
 *   - multicast, which sends the same buffer to a list of processors
 *     and also supports depost-bit torus line broadcasts
 *
 *   - manytomany, which sends different offsets from the same buffer
 *     to a list of processors
 *
 * As reductions have a single source buffer and destination buffer,
 * we have extended the multicast call to support reductions.
 *
 * Each multisend operation carries a connection id to identify the
 * data on the reciever.
 *
 * In a multisend operation the cores are typically involved at the
 * end-points. Moreover the processor lists can be created on the fly.
 */

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
   * \param[in] connection_id  Stream ID of data
   * \param[in] root        Sending task
   * \param[in] sndlen      Length of data sent
   * \param[in] clientdata  Opaque arg
   * \param[out] rcvlen     Length of data to receive
   * \param[out] rcvpwq     Where to put recv data
   * \param[out] cb_done    Completion callback to invoke when data received
   * \return   void
   */
  typedef void (*pami_dispatch_multicast_fn)(const pami_quad_t        *msginfo,
                                            unsigned              msgcount,
                                            unsigned              connection_id,
                                            size_t                root,
                                            size_t                sndlen,
                                            void                 *clientdata,
                                            size_t               *rcvlen,
                                            pami_pipeworkqueue_t **rcvpwq,
                                            pami_callback_t       *cb_done);

  /**
   * \brief The new structure to pass parameters for the multisend multicast operation.
   *
   * The PAMI_Multicast_t object is re-useable immediately, but objects referred to
   * (src, etc) cannot be re-used until cb_done.
   *
   * client and context types should not be pami_client_t and pami_context_t,
   * the implementations need offsets/index and can't access the opaque types
   * to get that because of circular dependencies.
   */
  typedef struct
  {
    size_t               client;	   /**< client to operate within */
    size_t               context;	   /**< primary context to operate within */
    size_t               dispatch;         /**< Dispatch identifier */
    pami_callback_t       cb_done;          /**< Completion callback */
    unsigned             connection_id;    /**< A connection is a distinct stream of
                                              traffic. The connection id identifies the
                                              connection */
    unsigned             roles;            /**< bitmap of roles to perform */
    size_t               bytes;            /**< size of the message*/
    pami_pipeworkqueue_t *src;              /**< source buffer */
    pami_topology_t      *src_participants; /**< root */
    pami_pipeworkqueue_t *dst;              /**< dest buffer (ignored for one-sided) */
    pami_topology_t      *dst_participants; /**< destinations to multicast to*/
    const pami_quad_t    *msginfo;	       /**< A extra info field to be sent with the message.
                                                  This might include information about
                                                  the data being sent, for one-sided. */
    unsigned            msgcount;          /**< info count*/
  } pami_multicast_t;
  

  /**  Deprecated Multicast:  To be deleted soon!!! */
  /**********************************************************************/
  typedef enum
  {
    PAMI_UNDEFINED_CONSISTENCY = -1,
    PAMI_RELAXED_CONSISTENCY,
    PAMI_MATCH_CONSISTENCY,
    PAMI_WEAK_CONSISTENCY,
    PAMI_CONSISTENCY_COUNT
  } pami_consistency_t;

#define  LINE_BCAST_MASK    (PAMI_LINE_BCAST_XP|PAMI_LINE_BCAST_XM|	\
                             PAMI_LINE_BCAST_YP|PAMI_LINE_BCAST_YM|	\
                             PAMI_LINE_BCAST_ZP|PAMI_LINE_BCAST_ZM)
  typedef enum
  {
    PAMI_PT_TO_PT_SUBTASK           =  0,      /**< Send a pt-to-point message */
    PAMI_LINE_BCAST_XP              =  0x20,   /**< Bcast along x+ */
    PAMI_LINE_BCAST_XM              =  0x10,   /**< Bcast along x- */
    PAMI_LINE_BCAST_YP              =  0x08,   /**< Bcast along y+ */
    PAMI_LINE_BCAST_YM              =  0x04,   /**< Bcast along y- */
    PAMI_LINE_BCAST_ZP              =  0x02,   /**< Bcast along z+ */
    PAMI_LINE_BCAST_ZM              =  0x01,   /**< Bcast along z- */
    PAMI_COMBINE_SUBTASK            =  0x0100, /**< Combine the incoming message */
    /* with the local state */
    PAMI_GI_BARRIER                 =  0x0200,
    PAMI_LOCKBOX_BARRIER            =  0x0300,
    PAMI_TREE_BARRIER               =  0x0400,
    PAMI_TREE_BCAST                 =  0x0500,
    PAMI_TREE_ALLREDUCE             =  0x0600,
    PAMI_REDUCE_RECV_STORE          =  0x0700,
    PAMI_REDUCE_RECV_NOSTORE        =  0x0800,
    PAMI_BCAST_RECV_STORE           =  0x0900,
    PAMI_BCAST_RECV_NOSTORE         =  0x0a00,
    PAMI_LOCALCOPY                  =  0x0b00,
    PAMI_UNDEFINED_SUBTASK          =  (~LINE_BCAST_MASK),
  } pami_subtask_t;


  typedef pami_quad_t * (*pami_olddispatch_multicast_fn) (const pami_quad_t   * info,
                                                        unsigned             count,
                                                        unsigned             peer,
                                                        unsigned             sndlen,
                                                        unsigned             conn_id,
                                                        void               * arg,
                                                        unsigned           * rcvlen,
                                                        char              ** rcvbuf,
                                                        unsigned           * pipewidth,
                                                        pami_callback_t     * cb_done);
  typedef struct
  {
    pami_quad_t        * request;
    pami_callback_t      cb_done;
    unsigned            connection_id;
    unsigned            bytes;
    const char        * src;
    unsigned            ntasks;
    unsigned          * tasks;
    pami_subtask_t     * opcodes;
    const pami_quad_t  * msginfo;
    unsigned            count;
    unsigned            flags;
    pami_op              op;
    pami_dt              dt;
  } pami_oldmulticast_t;

  typedef struct
  {
    pami_quad_t        * request;
    pami_callback_t      cb_done;
    unsigned            connection_id;
    unsigned            bytes;
    char              * rcvbuf;
    unsigned            pipelineWidth;
    pami_subtask_t       opcode;
    pami_op              op;
    pami_dt              dt;
  } pami_oldmulticast_recv_t;


  typedef pami_quad_t * (*pami_olddispatch_manytomany_fn) (unsigned         conn_id,
                                                         void           * arg,
                                                         char          ** rcvbuf,
                                                         size_t        ** rcvdispls,
                                                         size_t        ** rcvlens,
                                                         size_t        ** rcvcounters,
                                                         size_t         * ntasks,
                                                         pami_callback_t * cb_done);


  /**********************************************************************/



  /**
   * \brief Sub-structure used to represent a vectored buffer for many-to-many.
   *
   * This is used for both send and receive parameters.
   *
   * This structure is likely to change as decisions are made about how
   * much of the vector to embed inside PipeWorkQueue and if Topology
   * contains permutation information.
   */
  typedef struct
  {
    pami_pipeworkqueue_t *buffer;       /**< Memory used for data (buffer)            */
    pami_topology_t      *participants; /**< Tasks that are vectored in buffer        */
    size_t              *lengths;      /**< Array of lengths in buffer for each task */
    size_t              *offsets;      /**< Array of offsets in buffer for each task */
    size_t               num_vecs;     /**< The number of entries in
                                            "lengths" and "offsets".
                                            May be a flag: either "1"
                                            or participants->size(). */
  } pami_manytomanybuf_t;

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
   * The myIndex parameter is the receiving task's index in the recv arrays
   * (lengths and offsets) and is used by the manytomany as an optimization
   * for handling reception data and completion.
   *
   * \param[in] arg		Client Data
   * \param[in] conn_id		Instance ID
   * \param[in] metadata	Pointer to metadata, if any, in message header.
   * \param[in] metacount	Number of pami_quad_ts of metadata.
   * \param[out] recv		Receive parameters for this connection (instance)
   * \param[out] myIndex	Index of Recv Task in the receive parameters.
   * \param[out] cb_done	Completion callback when message complete
   * \return	Request object opaque storage for message.
   */
  typedef void (*pami_dispatch_manytomany_fn)(void                 *arg,
                                             unsigned              conn_id,
                                             pami_quad_t           *metadata,
                                             unsigned              metacount,
                                             pami_manytomanybuf_t **recv,
                                             size_t               *myIndex,
                                             pami_callback_t       *cb_done);

  /**
   * \brief Structure of parameters used to initiate a ManyToMany
   *
   * The taskIndex parameter is transmitted to the receiver for use by cb_recv
   * for indexing into the recv parameter arrays (lengths and offsets).
   */
  typedef struct
  {
    size_t               client;	     /**< client to operate within */
    size_t               context;	     /**< primary context to operate within */
    pami_callback_t       cb_done;	     /**< User's completion callback */
    unsigned             connection_id;      /**< differentiate data streams */
    unsigned             roles;              /**< bitmap of roles to perform */
    size_t              *taskIndex;	     /**< Index of send in recv parameters */
    size_t               num_index;          /**< Number of entries in "taskIndex".
                                                should be multiple of send.participants->size()?
                                             */
    pami_manytomanybuf_t  send;               /**< send data parameters */
    const pami_quad_t    *metadata;	     /**< A extra info field to be sent with the message.
                                                This might include information about
                                                the data being sent, for one-sided. */
    unsigned             metacount;	     /**< metadata count*/
  } pami_manytomany_t;


  /******************************************************************************
   *       Multisync Personalized synchronization/coordination
   ******************************************************************************/

  /**
   * \brief structure defining interface to Multisync
   */
  typedef struct
  {
    size_t             client;	        /**< client to operate within */
    size_t             context;	        /**< primary context to operate within */
    pami_callback_t     cb_done;		/**< User's completion callback */
    unsigned           connection_id;	/**< (remove?) differentiate data streams */
    unsigned           roles;		/**< bitmap of roles to perform */
    pami_topology_t    *participants;	/**< Tasks involved in synchronization */
  } pami_multisync_t;

  /******************************************************************************
   *       Multicombine Personalized reduction
   ******************************************************************************/
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
  typedef struct
  {
    size_t               client;	      /**< client to operate within */
    size_t               context;	      /**< primary context to operate within */
    pami_callback_t       cb_done;             /**< User's completion callback */
    unsigned             connection_id;    /**< A connection is a distinct stream of
                                              traffic. The connection id identifies the
                                              connection */
    unsigned             roles;		      /**< bitmap of roles to perform */
    pami_pipeworkqueue_t *data;		      /**< Data source */
    pami_topology_t      *data_participants;   /**< Tasks contributing data */
    pami_pipeworkqueue_t *results;	      /**< Results destination */
    pami_topology_t      *results_participants;/**< Tasks receiving results */
    pami_op               optor;		      /**< Operation to perform on data */
    pami_dt               dtype;		      /**< Datatype of elements */
    size_t               count;		      /**< Number of elements */
  } pami_multicombine_t;


  /*****************************************************************************/
  /**
   * \defgroup datatype pami non-contiguous datatype interface
   *
   * Some brief documentation on active message stuff ...
   * \{
   */
  /*****************************************************************************/

  extern int pami_dt_shift[PAMI_DT_COUNT]; /// \todo what is this and is it really an extension?


#define PAMI_DISPATCH_EXTEND          pami_dispatch_multicast_fn   multicast;\
                                      pami_dispatch_manytomany_fn  manytomany; 


#define PAMI_DISPATCH_TYPE_EXTEND

#define PAMI_HINT_EXTEND


#endif
