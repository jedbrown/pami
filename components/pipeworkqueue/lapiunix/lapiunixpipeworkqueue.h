///
/// \file components/pipeworkqueue/lapiunix/lapiunixpipeworkqueue.h
/// \brief XMI LAPI specific PipeWorkQueue implementation.
///
#ifndef   __xmi_lapi_lapipipeworkqueue_h__
#define   __xmi_lapi_lapipipeworkqueue_h__

#include <stdlib.h>
#include <string.h>

#include "components/pipeworkqueue/PipeWorkQueue.h"

#define XMI_PIPEWORKQUEUE_CLASS XMI::PipeWorkQueue::LAPI

namespace XMI
{
    namespace PipeWorkQueue
    {
        class LAPI : public PipeWorkQueue<XMI::PipeWorkQueue::LAPI>
        {
        public:
            inline LAPI():
                PipeWorkQueue<XMI::PipeWorkQueue::LAPI>()
                {
                }
            inline LAPI(XMI::PipeWorkQueue::LAPI &obj):
                PipeWorkQueue<XMI::PipeWorkQueue::LAPI>(obj)
                {
                }
            inline ~LAPI()
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

            inline xmi_result_t exp_impl(xmi_pipeworkqueue_ext_t *exp)
                {
		  return XMI_UNIMPL;
                }

            inline xmi_result_t import_impl(xmi_pipeworkqueue_ext_t *import)
                {
		  return XMI_UNIMPL;
                }

            inline void setConsumerWakeup_impl(void *vec)
                {
                }

            inline void setProducerWakeup_impl(void *vec)
                {
                }

            inline size_t bytesAvailableToProduce_impl()
                {
		  return -1;
                }

            inline size_t bytesAvailableToConsume_impl()
                {
		  return -1;
                }

            inline size_t getBytesProduced_impl()
                {
		  return -1;
                }

            inline size_t getBytesConsumed_impl()
                {
		  return -1;
                }

            inline char *bufferToProduce_impl()
                {
		  return NULL;
                }

            inline void produceBytes_impl(size_t bytes)
                {
                }

            inline char *bufferToConsume_impl()
                {
		  return NULL;
                }

            inline void consumeBytes_impl(size_t bytes)
                {
                }

            inline bool available_impl()
                {
		  return false;
                }

        }; // end XMI::PipeWorkQueue::LAPI
    }; // end namespace PipeWorkQueue
}; // end namespace XMI

#endif // __xmi_lapi_lapipipeworkqueue_h__
