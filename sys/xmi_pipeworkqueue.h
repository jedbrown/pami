/**
 * \file xmi_pipeworkqueue.h
 * \brief PipeWorkQueue interface.
 */
#ifndef __xmi_pipeworkqueue_h__
#define __xmi_pipeworkqueue_h__

#include "xmi_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

  typedef XMIQuad XMI_PipeWorkQueue_t[4];
  typedef XMIQuad XMI_PipeWorkQueue_ext[2];

  ///
  /// \brief Configure for Shared Circular Buffer variety.
  ///
  /// Only one consumer and producer are allowed.
  /// Creates a circular buffer of specified size in shared memory.
  /// Buffer size must be power-of-two.
  ///
  /// \param[out] wq	Opaque memory for PipeWorkQueue
  /// \param[in] bufsize	Size of buffer to allocate
  ///
  void XMI_PipeWorkQueue_config_circ(XMI_PipeWorkQueue_t *wq, size_t bufsize);

  ///
  /// \brief Configure for User-supplied Circular Buffer variety.
  ///
  /// Only one consumer and producer are allowed.
  /// Uses the provided buffer as a circular buffer of specified size.
  /// Buffer size must be power-of-two.
  /// Assumes the caller has placed buffer and (this) in appropriate memory
  /// for desired use - i.e. all in shared memory if to be used beyond this process.
  ///
  /// NOTE: details need to be worked out. The buffer actually needs to include the
  /// WQ header, so the caller must somehow know how much to allocate memory -
  /// and how to ensure desired alignment.
  ///
  /// \param[out] wq		Opaque memory for PipeWorkQueue
  /// \param[in] buffer		Buffer to use
  /// \param[in] bufsize	Size of buffer
  ///
  void XMI_PipeWorkQueue_config_circ_usr(XMI_PipeWorkQueue_t *wq, char *buffer, size_t bufsize);

  ///
  /// \brief Configure for Memory (flat buffer) variety.
  ///
  /// Only one consumer and producer are allowed. Still supports pipelining.
  /// Sets up a flat buffer of specified maximum size with an arbitrary "initial fill".
  /// Assumes the caller has placed buffer and (this) in appropriate memory
  /// for desired use - i.e. all in shared memory if to be used beyond this process.
  ///
  /// \param[out] wq		Opaque memory for PipeWorkQueue
  /// \param[in] buffer		Buffer to use
  /// \param[in] bufsize	Size of buffer
  /// \param[in] bufinit	Amount of data initially in buffer
  ///
  void XMI_PipeWorkQueue_config_flat(XMI_PipeWorkQueue_t *wq, char *buffer, size_t bufsize, size_t bufinit);

  ///
  /// \brief PROPOSAL: Configure for Non-Contig Memory (flat buffer) variety.
  ///
  /// Only one consumer and producer are allowed. Still supports pipelining.
  /// Sets up a flat buffer of specified maximum size with an arbitrary "initial fill".
  /// Assumes the caller has placed buffer and (this) in appropriate memory
  /// for desired use - i.e. all in shared memory if to be used beyond this process.
  ///
  /// This is typically only used for the application buffer, either input or output,
  /// and so would not normally have both producer and consumer (only one or the other).
  /// The interface is the same as for contiguous data except that "bytesAvailable" will
  /// only return the number of *contiguous* bytes available. The user must consume those
  /// bytes before it can see the next contiguous chunk.
  ///
  /// \param[out] wq            Opaque memory for PipeWorkQueue
  /// \param[in] buffer         Buffer to use
  /// \param[in] type           Memory layout of a buffer unit
  /// \param[in] typecount      Number of repetitions of buffer units
  /// \param[in] typeinit       Number of units initially in buffer
  ///
  void XMI_PipeWorkQueue_config_noncontig(XMI_PipeWorkQueue_t *wq, char *buffer, xmi_type_t *type, size_t typecount, size_t typeinit); 

  ///
  /// \brief Export
  ///
  /// Produces information about the PipeWorkQueue into the opaque buffer "export".
  /// This info is suitable for sharing with other processes such that those processes
  /// can then construct a PipeWorkQueue which accesses the same data stream.
  ///
  /// This only has value in the case of a flat buffer PipeWorkQueue and a platform
  /// that supports direct mapping of memory from other processes. Circular buffer
  /// PipeWorkQueues use shared memory and are inherently inter-process in nature.
  ///
  /// The exporting process is the only one that can produce to the PipeWorkQueue.
  /// All importing processes are restricted to consuming. There is no feedback
  /// from the consumers to the producer - no flow control.
  ///
  /// \param[in] wq             Opaque memory for PipeWorkQueue
  /// \param[out] export        Opaque memory to export into
  /// \return	success of the export operation
  ///
  xmi_result_t XMI_PipeWorkQueue_export(XMI_PipeWorkQueue_t *wq, XMI_PipeWorkQueue_ext *exp); 

  ///
  /// \brief Import
  ///
  /// Takes the results of an export of a PipeWorkQueue on a different process and
  /// constructs a new PipeWorkQueue which the local process may use to access the
  /// data stream.
  ///
  /// The resulting PipeWorkQueue may consume data, but that is a local-only operation.
  /// The producer has no knowledge of data consumed. There can be only one producer.
  /// There may be multiple consumers, but the producer does not know about them.
  /// An importing processes cannot be the producer.
  ///
  /// TODO: can this work for circular buffers? does it need to, since those are
  /// normally shared memory and thus already permit inter-process communication.
  ///
  /// \param[in] import        Opaque memory into which an export was done.
  /// \param[out] wq           Opaque memory for new PipeWorkQueue
  /// \return	success of the import operation
  ///
  xmi_result_t XMI_PipeWorkQueue_import(XMI_PipeWorkQueue_ext *import, XMI_PipeWorkQueue_t *wq); 
 
  ///
  /// \brief Clone constructor.
  ///
  /// Used to create a second local memory wrapper object of the same
  /// shared memory resource.
  ///
  /// \see WorkQueue(WorkQueue &)
  ///
  /// \param[out] wq	Opaque memory for new PipeWorkQueue
  /// \param[in] obj	old object, to be cloned
  ///
  void XMI_PipeWorkQueue_clone(XMI_PipeWorkQueue_t *wq, XMI_PipeWorkQueue_t *obj);

  ///
  /// \brief Destructor
  ///
  /// \param[out] wq	Opaque memory for PipeWorkQueue
  ///
  void XMI_PipeWorkQueue_destroy(XMI_PipeWorkQueue_t *wq);

  ///
  /// \brief Reset this pipe work queue.
  ///
  /// All PipeWorkQueues must be reset() at least once after configure and before using.
  /// (TODO: should this simply be part of configure?)
  ///
  /// Sets the number of bytes produced and the number of bytes
  /// consumed to zero (or to "bufinit" as appropriate).
  ///
  /// This is typically required by circular PipeWorkQueues that are re-used. 
  /// Flat PipeWorkQueues are usually configured new for each instance
  /// and thus do not require resetting. Circular PipeWorkQueues should be
  /// reset by only one entity, and at a point when it is known that no other
  /// entity is still using it (it must be idle). For example, in a multisend pipeline
  /// consisting of:
  ///
  ///        [barrier] ; local-reduce -(A)-&gt; global-allreduce -(B)-&gt; local-broadcast
  ///
  /// the PipeWorkQueue "B" would be reset by the root of the reduce when starting the
  /// local-reduce operation (when it is known that any prior instances have completed).
  ///
  /// One reason that a reset may be needed is to preserve buffer alignment. Another is
  /// to prevent problems when, say, a consumer requires a full packet of data. In this
  /// case, a circular PipeWorkQueue may have been left in a state from the previous
  /// operation where the final chunk of data has left less than a packet length before
  /// the wrap point. This would create a hang because the consumer would never see a full
  /// packet until it consumes those bytes and passes the wrap point.
  ///
  /// Since resets are performed by the protocol code, it understands the context and
  /// whether the PipeWorkQueue represents a flat (application) buffer or an intermediate
  /// (circular) one.
  ///
  /// \param[out] wq	Opaque memory for PipeWorkQueue
  ///
  void XMI_PipeWorkQueue_reset(XMI_PipeWorkQueue_t *wq);

  ///
  /// \brief Dump shared memory work queue statistics to stderr.
  ///
  /// \param[in] wq	Opaque memory for PipeWorkQueue
  /// \param[in] prefix Optional character string to prefix.
  ///
  void XMI_PipeWorkQueue_dump(XMI_PipeWorkQueue_t *wq, const char *prefix);

  /// \brief register a wakeup for the consumer side of the PipeWorkQueue
  ///
  /// The 'vec' parameter is typically obtained from some platform authority,
  /// which is the same used by the PipeWorkQueue to perform the wakeup.
  /// A consumer wishing to be awoken when data is available would call the
  /// system to get their 'vec' value, and pass it to this method to register
  /// for wakeups. When the produceBytes method is called, it will use this
  /// consumer wakeup 'vec' to tell the system to wake up the consumer process or thread. 
  ///
  /// \param[in] wq	Opaque memory for PipeWorkQueue
  /// \param[in] vec	Opaque wakeup vector parameter
  ///
  void XMI_PipeWorkQueue_setConsumerWakeup(XMI_PipeWorkQueue_t *wq, void *vec);

  /// \brief register a wakeup for the producer side of the PipeWorkQueue
  ///
  /// See setConsumerWakeup() for details.
  ///
  /// \param[in] wq	Opaque memory for PipeWorkQueue
  /// \param[in] vec	Opaque wakeup vector parameter
  ///
  void XMI_PipeWorkQueue_setProducerWakeup(XMI_PipeWorkQueue_t *wq, void *vec);

  ///
  /// \brief Return the number of contiguous bytes that can be produced into this work queue.
  ///
  /// Bytes must be produced into the memory location returned by bufferToProduce() and then
  /// this work queue \b must be updated with produceBytes().
  ///
  /// \see bufferToProduce
  /// \see produceBytes
  ///
  /// \param[in] wq	Opaque memory for PipeWorkQueue
  /// \return Number of bytes that may be produced.
  ///
  size_t XMI_PipeWorkQueue_bytesAvailableToProduce(XMI_PipeWorkQueue_t *wq);

  ///
  /// \brief Return the number of contiguous bytes that can be consumed from this work queue.
  ///
  /// Bytes must be consumed into the memory location returned by bufferToConsume() and then
  /// this work queue \b must be updated with consumeBytes().
  ///
  /// \see bufferToConsume
  /// \see consumeBytes
  ///
  /// \param[in] wq	Opaque memory for PipeWorkQueue
  /// \param[in] consumer Consumer id for work queues with multiple consumers
  ///
  /// \param[in] wq	Opaque memory for PipeWorkQueue
  /// \return Number of bytes that may be consumed.
  ///
  size_t XMI_PipeWorkQueue_bytesAvailableToConsume(XMI_PipeWorkQueue_t *wq);

  /// \brief raw accessor for total number of bytes produced since reset()
  ///
  /// \param[in] wq	Opaque memory for PipeWorkQueue
  /// \return	number of bytes produced
  ///
  size_t XMI_PipeWorkQueue_getBytesProduced(XMI_PipeWorkQueue_t *wq);

  /// \brief raw accessor for total number of bytes consumed since reset()
  ///
  /// \param[in] wq	Opaque memory for PipeWorkQueue
  /// \return	number of bytes consumed
  ///
  size_t XMI_PipeWorkQueue_getBytesConsumed(XMI_PipeWorkQueue_t *wq);

  /// \brief current position for producing into buffer
  ///
  /// \param[in] wq	Opaque memory for PipeWorkQueue
  /// \return	location in buffer to produce into
  ///
  char *XMI_PipeWorkQueue_bufferToProduce(XMI_PipeWorkQueue_t *wq);

  /// \brief notify workqueue that bytes have been produced
  ///
  /// \param[in] wq	Opaque memory for PipeWorkQueue
  /// \return	number of bytes that were produced
  ///
  void XMI_PipeWorkQueue_produceBytes(XMI_PipeWorkQueue_t *wq, size_t bytes);

  /// \brief current position for consuming from buffer
  ///
  /// \param[in] wq	Opaque memory for PipeWorkQueue
  /// \return	location in buffer to consume from
  ///
  char *XMI_PipeWorkQueue_bufferToConsume(XMI_PipeWorkQueue_t *wq);

  /// \brief notify workqueue that bytes have been consumed
  ///
  /// \param[in] wq	Opaque memory for PipeWorkQueue
  /// \return	number of bytes that were consumed
  ///
  void XMI_PipeWorkQueue_consumeBytes(XMI_PipeWorkQueue_t *wq, size_t bytes);

  /// \brief is workqueue ready for action
  ///
  /// \param[in] wq	Opaque memory for PipeWorkQueue
  /// \return	boolean indicate workqueue readiness
  ///
  int XMI_PipeWorkQueue_available(XMI_PipeWorkQueue_t *wq);

#ifdef __cplusplus
};
#endif

#endif /* __xmi_pipeworkqueue_h__ */
