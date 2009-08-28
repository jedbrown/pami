///
/// \file xmi/mpi/mpiPipeWorkQueue.h
/// \brief XMI MPI specific PipeWorkQueue implementation.
///
#ifndef   __xmi_mpi_mpipipeworkqueue_h__
#define   __xmi_mpi_mpipipeworkqueue_h__

#include <stdlib.h>
#include <string.h>

#include "components/pipeworkqueue/PipeWorkQueue.h"

#define XMI_PIPEWORKQUEUE_CLASS XMI::PipeWorkQueue::MPI

namespace XMI
{
    namespace PipeWorkQueue
    {
        class MPI : public PipeWorkQueue<XMI::PipeWorkQueue::MPI>
        {
        public:
            inline MPI():
                PipeWorkQueue<XMI::PipeWorkQueue::MPI>()
                {
                }
            inline MPI(XMI::PipeWorkQueue::MPI &obj):
                PipeWorkQueue<XMI::PipeWorkQueue::MPI>(obj)
                {
                }
            inline ~MPI()
                {
                }
            
            inline void configure_impl(void *sysdep, size_t bufsize)
                {
                }

            inline void configure_impl(void *sysdep, char *buffer, size_t bufsize)
                {
                }

            inline void configure_impl(void *sysdep, char *buffer, size_t bufsize, size_t bufinit)
                {
                }

            inline void configure_impl(void *sysdep, char *buffer, xmi_type_t *type, size_t typecount, size_t typeinit)
                {
                }

            inline void reset_impl()
                {
                }

            inline void dump_impl(const char *prefix = NULL)
                {
                }

            inline xmi_result_t exp_impl(XMI_PipeWorkQueue_ext *exp)
                {
                }

            inline xmi_result_t import_impl(XMI_PipeWorkQueue_ext *import)
                {
                }

            inline void setConsumerWakeup_impl(void *vec)
                {
                }

            inline void setProducerWakeup_impl(void *vec)
                {
                }

            inline size_t bytesAvailableToProduce_impl()
                {
                }

            inline size_t bytesAvailableToConsume_impl()
                {
                }

            inline size_t getBytesProduced_impl()
                {
                }

            inline size_t getBytesConsumed_impl()
                {
                }

            inline char *bufferToProduce_impl()
                {
                }

            inline void produceBytes_impl(size_t bytes)
                {
                }

            inline char *bufferToConsume_impl()
                {
                }

            inline void consumeBytes_impl(size_t bytes)
                {
                }

            inline bool available_impl()
                {
                }
            
        }; // end XMI::PipeWorkQueue::MPI
    }; // end namespace PipeWorkQueue
}; // end namespace XMI

#endif // __xmi_mpi_mpipipeworkqueue_h__
