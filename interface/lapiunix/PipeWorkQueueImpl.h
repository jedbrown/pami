/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file generic/PipeWorkQueueImpl.h
 * \brief Implementation of PipeWorkQueue
 */

#ifndef __xmi_cpp_pipeworkqueueimpl_h__
#define __xmi_cpp_pipeworkqueueimpl_h__



namespace XMI 
{
  class _PipeWorkQueueImpl 
  {
  public:
    _PipeWorkQueueImpl()
    {
    }
    ///
    /// \brief Configure for Shared Circular Buffer variety.
    ///
    /// Only one consumer and producer are allowed.
    /// Creates a circular buffer of specified size in shared memory.
    /// Buffer size must be power-of-two.
    ///
    /// \param[in] sysdep	System dependent methods
    /// \param[in] bufsize	Size of buffer to allocate
    ///
    inline void configure(void *sysdep, size_t bufsize) 
    {
    }

    ///
    /// \brief Configure for User-supplied Circular Buffer variety.
    ///
    /// Only one consumer and producer are allowed.
    /// Uses the provided buffer as a circular buffer of specified size.
    /// Buffer size must be power-of-two.
    /// Assumes the caller has placed buffer and (this) in appropriate memory
    /// for desired use - i.e. all in shared memory if to be used beyond this process.
    ///
    /// \param[in] sysdep   System dependent methods
    /// \param[in] buffer	Buffer to use
    /// \param[in] bufsize	Size of buffer
    ///
    inline void configure(void *sysdep, char *buffer, size_t bufsize) 
    {
    }

    ///
    /// \brief Configure for Memeory (flat buffer) variety.
    ///
    /// Only one consumer and producer are allowed. Still supports pipelining.
    /// Sets up a flat buffer of specified maximum size with an arbitrary "initial fill".
    /// Buffer size must be power-of-two.
    /// Assumes the caller has placed buffer and (this) in appropriate memory
    /// for desired use - i.e. all in shared memory if to be used beyond this process.
    ///
    /// \param[in] sysdep   System dependent methods
    /// \param[in] buffer	Buffer to use
    /// \param[in] bufsize	Size of buffer
    /// \param[in] bufinit	Amount of data initially in buffer
    ///
    inline void configure(void *sysdep, char *buffer, size_t bufsize, size_t bufinit) 
    {
    }


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
    /// \param[in] dgsp           Memory layout of a buffer unit
    /// \param[in] dgspcount      Number of repetitions of buffer units
    /// \param[in] dgspinit       Number of units initially in buffer
    ///
    inline void configure(void *sysdep, char *buffer, XMI_dgsp_t *dgsp, size_t dgspcount, size_t dgspinit)
    {

    }

    ///
    /// \brief Clone constructor.
    ///
    /// Used to create a second local memory wrapper object of the same
    /// shared memory resource.
    ///
    /// \see WorkQueue(WorkQueue &)
    ///
    /// \param[in] obj     Shared work queue object
    ///
    _PipeWorkQueueImpl(_PipeWorkQueueImpl &obj)
    {
      // need ref count so we know when to free...
      reset();
    }
    ///
    /// \brief Virtual destructors make compilers happy.
    ///
    ~_PipeWorkQueueImpl() 
    {
    }

    ///
    /// \brief Reset this shared memory work queue.
    ///
    /// Sets the number of bytes produced and the number of bytes
    /// consumed by each consumer to zero.
    ///
    inline void reset() 
    {
    }

    ///
    /// \brief Dump shared memory work queue statistics to stderr.
    ///
    /// \param[in] prefix Optional character string to prefix.
    ///
    inline void dump(const char *prefix = NULL) 
    {
    }

    ///
    /// \brief Export
    ///
    /// Produces information about the PipeWorkQueue into the opaque buffer "export".
    /// This info is suitable for sharing with other processes such that those processes
    /// can then construct a PipeWorkQueue which accesses the same data stream.
    ///
    /// \param[in] wq             Opaque memory for PipeWorkQueue
    /// \param[out] export        Opaque memory to export into
    /// \return   success of the export operation
    ///
    inline XMI_Result exp(XMI_PipeWorkQueue_ext *exp)
    {

    }
	
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
    ///
    /// TODO: can this work for circular buffers? does it need to, since those are
    /// normally shared memory and thus already permit inter-process communication.
    ///
    /// \param[in] import        Opaque memory into which an export was done.
    /// \param[out] wq           Opaque memory for new PipeWorkQueue
    /// \return   success of the import operation
    ///
    inline XMI_Result import(XMI_PipeWorkQueue_ext *import)
    {

    }


    /// \brief register a wakeup for the consumer side of the PipeWorkQueue
    ///
    /// \param[in] vec	Opaque wakeup vector parameter
    ///
    inline void setConsumerWakeup(void *vec) 
    {
    }

    /// \brief register a wakeup for the producer side of the PipeWorkQueue
    ///
    /// \param[in] vec	Opaque wakeup vector parameter
    ///
    inline void setProducerWakeup(void *vec) 
    {
    }

    ///
    /// \brief Return the maximum number of bytes that can be produced into this work queue.
    ///
    /// Bytes must be produced into the memory location returned by bufferToProduce() and then
    /// this work queue \b must be updated with produceBytes().
    ///
    /// The number of bytes that may be produced is calculated by determining
    /// the difference between the \b minimum number of bytes consumed by all
    /// consumers and the number of bytes produced.
    ///
    /// This must only be called with serious intent to produce! If this
    /// routine returns non-zero, the caller MUST produceBytes() in a timely
    /// manner. Never returns more than one packet.
    ///
    /// \see bufferToProduce
    /// \see produceBytes
    ///
    /// \return Number of bytes that may be produced.
    ///
    inline size_t bytesAvailableToProduce() 
    {
    }

    ///
    /// \brief Return the maximum number of bytes that can be consumed from this work queue.
    ///
    /// Consuming from work queues with multiple consumers must specify the consumer id.
    ///
    /// Bytes must be consumed into the memory location returned by bufferToConsume() and then
    /// this work queue \b must be updated with consumeBytes().
    ///
    /// \see bufferToConsume
    /// \see consumeBytes
    ///
    /// \param[in] consumer Consumer id for work queues with multiple consumers
    ///
    /// \return Number of bytes that may be consumed.
    ///
    inline size_t bytesAvailableToConsume() 
    {
    }

    /// \brief raw accessor for total number of bytes produced since reset()
    ///
    /// \return	number of bytes produced
    ///
    inline size_t getBytesProduced() 
    {
    }

    /// \brief raw accessor for total number of bytes consumed since reset()
    ///
    /// \return	number of bytes consumed
    ///
    inline size_t getBytesConsumed() 
    {
    }

    /// \brief current position for producing into buffer
    ///
    /// \return	location in buffer to produce into
    ///
    inline char *bufferToProduce() 
    {
    }

    /// \brief notify workqueue that bytes have been produced
    ///
    /// \return	number of bytes that were produced
    ///
    inline void produceBytes(size_t bytes) 
    {
    }

    /// \brief current position for consuming from buffer
    ///
    /// \return	location in buffer to consume from
    ///
    inline char *bufferToConsume() 
    {
    }

    /// \brief notify workqueue that bytes have been consumed
    ///
    /// \return	number of bytes that were consumed
    ///
    inline void consumeBytes(size_t bytes) 
    {
    }

    /// \brief is workqueue ready for action
    ///
    /// \return	boolean indicate workqueue readiness
    ///
    inline bool available() 
    {
    }

    /// \brief is workqueue buffer 16-byte aligned
    ///
    /// \return	boolean indicate workqueue buffer alignment
    ///
    inline bool aligned() 
    {
    }

  private:
  }; // class _PipeWorkQueueImpl

}; /* namespace XMI */

#endif /* __xmi_cpp_pipeworkqueueimpl_h__ */
