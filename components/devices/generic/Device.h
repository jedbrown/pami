/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __components_devices_generic_Device_h__
#define __components_devices_generic_Device_h__

///  \file components/devices/generic/Device.h
///  \brief Generic Device
///
///  The Generic classes implement a BaseDevice and a Message object
///  to post into the queueing system.  The GI device is currently
///  used to implement barriers, so the Generic device posts a message
///  and uses a interprocess communication sysdep to signal the Generic wire
///

/// \page use_gendev How to use the Generic Device
///
/// This chapter explains the basic requirements and features of the generic device.
///
/// \section use_gendev_basic Basic requirements and features of the Generic Device
///
/// The generic device implements a pair of queues for each context.
///
/// One queue holds thread objects, which each represent a until of work.
/// Each thread on the queue will have it's work function called when
/// the generic device slice (context) is advanced. Depending on the
/// thread status and work function return code, the thread may persist
/// on the queue or be removed. Thread objects are posted using
/// \ref PAMI::Device::Generic::Device::postThread "postThread()".
///
/// The other queue holds message objects. These objects are queued only
/// to be checked for completion. during advance, each object on this queue
/// has it's status checked, and if Done will be dequeued and the completion
/// callback invoked. Message are posted using
/// \ref PAMI::Device::Generic::Device::postMsg "postMsg()".
///
/// A user of the generic device may employ both message and threads,
/// or either one alone.
///
/// A typical usage is to create a message object with common data
/// and the completion callback, and then create one or more thread objects
/// with data members for the specific work assigned to the thread.
/// As each thread completes it's work it updates the message and when
/// last thread completes it will set the message status to Done.
/// The thread objects are posted to different contexts thereby achieving
/// parallelism.
///
/// All devices are given a pointer to the array of generic devices during
/// init(). By saving the pointer, any device may later post work to
/// any slice of the generic device.
///
/// \subsection use_gendev_syn SYNOPSIS
/// <div style="margin-left: 3em">
///
/// \#include "components/devices/generic/Device.h"
///
/// </div>
/// \subsection use_gendev_thr THREAD
/// <div style="margin-left: 3em">
///
/// All objects posted to the generic device via postThread() must
/// inherit from GenericThread. Each thread has a status which determines
/// how it is handled when on a generic device queue. The thread status
/// may change as a result of calling the work fuinction, but also may
/// be changed by third-parties, for example to terminate a thread without
/// the work function being called (again).
///
/// \ref PAMI::Device::ThreadStatus "GenericThread status values"
///
/// \ref PAMI::Device::Generic::GenericThread "class GenericThread"
///
/// \b Provides:
/// <div style="margin-left: 3em">
///
/// \ref PAMI::Device::Generic::GenericThread::getStatus "ThreadStatus getStatus()"
///
/// \ref PAMI::Device::Generic::GenericThread::setStatus "void setStatus(ThreadStatus stat)"
///
/// \ref PAMI::Device::Generic::GenericThread::setFunc "void setFunc(pami_work_function func, void *cookie)"
///
/// </div>
/// </div>
/// \subsection use_gendev_msg MESSAGE
/// <div style="margin-left: 3em">
///
/// All objects posted to the generic device via postMsg() must
/// inherit from GenericMessage.
///
/// \ref PAMI::Device::MessageStatus "GenericMessage status values"
///
/// \ref PAMI::Device::Generic::GenericMessage "class GenericMessage"
///
/// The generic device will check the status of each message on it's queue
/// and if a status is Done then will dequeue the message and call the
/// message completion callback. This also requires that the message support
/// the getQS() method to return the send queue on which the message is
/// (may be) queued. Note, this queue may never be used (always empty) but
/// it must be a valid queue. If the generic device finds another message
/// on this queue, then it will start that message using the postNext() method.
///
/// The message has a private interface known by the model and thread,
/// and possibly device.
///
/// \b Requires:
/// <div style="margin-left: 3em">
///
/// \ref GenericDeviceMessageQueue "typedef GenericDeviceMessageQueue"
///
/// \ref PAMI::Device::Generic::GenericMessage::postNext "virtual pami_context_t postNext(bool devQueued)"
///
/// </div>
/// \b Provides:
/// <div style="margin-left: 3em">
///
/// \ref PAMI::Device::Generic::GenericMessage::setStatus "void setStatus(MessageStatus status)"
///
/// \ref PAMI::Device::Generic::GenericMessage::getStatus "MessageStatus getStatus()"
///
/// \ref PAMI::Device::Generic::GenericMessage::getQS "GenericDeviceMessageQueue *getQS()"
///
/// \ref PAMI::Device::Generic::GenericMessage::getClientId "size_t getClientId()"
///
/// \ref PAMI::Device::Generic::GenericMessage::getContextId "size_t getContextId()"
///
/// \ref PAMI::Device::Generic::GenericMessage::executeCallback "void executeCallback(pami_context_t ctx, pami_result_t err = PAMI_SUCCESS)"
///
/// </div>
/// </div>
/// \subsection use_gendev_dev DEVICE
/// <div style="margin-left: 3em">
///
/// All devices that post work to the generic device must ensure that the
/// messages posted have a QS pointer (in GenericMessage ctor) which
/// points to a valid send queue. The send queue may never be used (always empty)
/// but it must exist.
///
/// The device has a private interface known by the model and message
/// (and possibly thread). The device must, however, implement the
/// \ref PAMI::Device::Interface::FactoryInterface "FactoryInterface"
/// and be instanciated in the
/// \ref PAMI::PlatformDeviceList "PlatformDeviceList"
/// of the client (defined in the context).
///
/// \b Requires:
/// <div style="margin-left: 3em">
///
/// \ref GenericDeviceMessageQueue "typedef GenericDeviceMessageQueue"
///
/// </div>
/// </div>
/// \subsection use_gendev_mdl MODEL
/// <div style="margin-left: 3em">
///
/// The model is typically driven by the defined interface for the model type.
/// For example, a multicombine model will implement the postMulticombine()
/// interface. The model constructor takes a device reference, client ID, and
/// context ID. The device reference may be for a single global device, a
/// device specific to the context, or some other arangement. Since the model
/// is intimately tied to the device, the model knows how to interact with
/// the device.
///
/// All details of the Message, Device, and Thread are hidden from the user of
/// the model, except that a reference to a (meaningful) device must be passed
/// to the model ctor, and the model must expose an integer constant "sizeof_msg"
/// which is the number of bytes required for the Message object.
///
/// </div>
/// \subsection use_gendev_use HOW TO
/// <div style="margin-left: 3em">
///
/// Steps to create a new Model/Device/Message/Thread set.
///
/// TBD...
///
/// </div>
/// <HR>

#include "components/memory/MemoryManager.h"
#include "WakeupManager.h"
#include "Global.h"
#include "components/devices/BaseDevice.h"
#include "components/devices/generic/AdvanceThread.h"
#include "components/devices/generic/Message.h"
#include "components/devices/FactoryInterface.h"
#include "components/atomic/Mutex.h"
#include <pami.h>

#ifndef PAMI_MAX_NUM_CLIENTS
/** \todo PAMI_MAX_NUM_CLIENTS needs to be setup by pami.h */
#define PAMI_MAX_NUM_CLIENTS  4
#endif // !PAMI_MAX_NUM_CLIENTS

#include "GenericDevicePlatform.h"

namespace PAMI {
namespace Device {
namespace Generic {

///  \brief A Device implmentation of a Generic Work Queuesystem
///
///  This class implements a queue system. The user posts work to the queue
///  and calls to advance will execute the work.
///
class Device {

public:
        /// \brief standard Device::Factory API
        class Factory : public Interface::FactoryInterface<Factory,Device,Device> {
        public:
                /// \brief Generate an array of devices for a client
                /// \param[in] client Client ID
                /// \param[in] num_ctx  Number of contexts being created in client
                /// \param[in] mm Memory manager (for shmem alloc, if needed)
                /// \return Array of devices
                static inline Device *generate_impl(size_t client, size_t num_ctx, Memory::MemoryManager & mm, PAMI::Device::Generic::Device *devices) {
                        size_t x;
                        Device *gds;
                        int rc = posix_memalign((void **)&gds, 16, sizeof(*gds) * num_ctx);
                        PAMI_assertf(rc == 0, "posix_memalign failed for generics[%zu], errno=%d\n", num_ctx, errno);
                        for (x = 0; x < num_ctx; ++x) {
                                new (&gds[x]) PAMI::Device::Generic::Device(client, x, num_ctx);
                        }
                        return gds;
                }
                /// \brief Initialize a specific device for client/context
                /// \param[in] devs   Device array returned by generate call
                /// \param[in] client   Client ID
                /// \param[in] contextId  Context ID
                /// \param[in] clt    Client
                /// \param[in] ctx    Context
                /// \param[in] sd   SysDep
                /// \param[in] devices    Generic Device array (same as devs in this case)
                /// \return Error code
                static inline pami_result_t init_impl(Device *devs, size_t client, size_t contextId, pami_client_t clt, pami_context_t ctx, PAMI::Memory::MemoryManager *mm, PAMI::Device::Generic::Device *devices) {
                        return getDevice_impl(devs, client, contextId).init(ctx, client, contextId, mm, devices);
                }
                /// \brief Advance this device for client/context
                /// \param[in] devs Device array returned by generate call
                /// \param[in] client Client ID
                /// \param[in] context  Context ID
                /// \return Events processed
                static inline size_t advance_impl(Device *devs, size_t client, size_t context) {
                        return getDevice_impl(devs, client, context).advance();
                }
                /// \brief Get reference to specific device given client and context
                /// \param[in] devs Device array returned by generate call
                /// \param[in] client Client ID
                /// \param[in] context  Context ID
                /// \return Reference to a device
                static inline Device & getDevice_impl(Device *devs, size_t client, size_t context) {
                        return devs[context];
                }
        }; // class Factory

        /// \brief  Constructor for generic device
        ///
        /// \param[in] client   Client ID
        /// \param[in] contextId  Context ID
        /// \param[in] num_ctx    Number of contexts for client
        ///
        inline Device(size_t client, size_t contextId, size_t num_ctx) :
#if 0
        __GenericQueue(),
        __Threads(),
#endif
#ifndef QUEUE_NO_ITER
        __ThrIter(),
#endif // !QUEUE_NO_ITER
        __clientId(client),
        __contextId(contextId),
        __nContexts(num_ctx)
        {
        }

        /// \brief Initialize the generic device slice
        /// \param[in] ctx  Context
        /// \param[in] client Client ID
        /// \param[in] context  Context ID
        /// \return Error code
        inline pami_result_t init(pami_context_t ctx, size_t client, size_t context, PAMI::Memory::MemoryManager *mm, PAMI::Device::Generic::Device *devices) {
                __context = ctx;
    __mm = mm;
    __allGds = devices;

    __queues = NULL;
#if defined(__pami_target_bgq__) && defined(USE_COMMTHREADS)
    size_t myix = __global.topology_local.rank2Index(__global.mapping.task());
    __global._wuRegion_mms[client][myix].memalign((void **)&__queues, sizeof(void *), sizeof(*__queues));
#else // ! __pami_target_bgq__
    int rc = posix_memalign((void **)&__queues, sizeof(void *), sizeof(*__queues));
    rc = rc; // until we decide what to do with error
#endif // ! __pami_target_bgq__
    PAMI_assertf(__queues, "Out of memory allocating generic device queues");
                __queues->__Threads.init(mm);
                __queues->__GenericQueue.init(mm);
#ifndef QUEUE_NO_ITER
    __queues->__Threads.iter_init(&__ThrIter);
#endif // !QUEUE_NO_ITER
                return PAMI_SUCCESS;
        }

        /// \brief Advance routine for (one channel of) the generic device.
        ///
        /// This advances all units of work on this context's queue, and
        /// checks the message queue for completions. It also calls the
        /// advanceRecv routine for all devices.
        ///
        /// \return number of events processed
        ///
        inline size_t advance() {
                int events = 0;
                //+ Need to ensure only one of these runs per core
                //+ (even if multi-threads per core)
                //+ if (core_mutex.tryAcquire()) {

                // could check the queues here and return if empty, but it
                // probably takes just as much as the for loops would, and
                // just further delay the advance of real work when present.

                //if (!__Threads.mutex()->tryAcquire()) continue;
#ifndef QUEUE_NO_ITER
                GenericThread *thr;
    __queues->__Threads.iter_begin(&__ThrIter);
    for (; __queues->__Threads.iter_check(&__ThrIter); __queues->__Threads.iter_end(&__ThrIter)) {
      thr = (GenericThread *)__queues->__Threads.iter_current(&__ThrIter);
#else // QUEUE_NO_ITER
    GenericThread *thr, *nxtthr;
    for (thr = (GenericThread *)__queues->__Threads.peekHead(); thr; thr = nxtthr) {
      nxtthr = (GenericThread *)__queues->__Threads.nextElem(thr);
#endif // QUEUE_NO_ITER
                        if (thr->getStatus() == PAMI::Device::Ready) {
                                ++events;
                                pami_result_t rc = thr->executeThread(__context);
                                if (rc != PAMI_EAGAIN) {
                                        // thr->setStatus(PAMI::Device::Complete);
#ifndef QUEUE_NO_ITER
                                        __queues->__Threads.iter_remove(&__ThrIter);
#else // QUEUE_NO_ITER
          __queues->__Threads.deleteElem(thr);
#endif // QUEUE_NO_ITER
                                        continue;
                                }
                        } else if (thr->getStatus() == PAMI::Device::OneShot) {
                                ++events;
                                // thread is like completion callback, dequeue first.
#ifndef QUEUE_NO_ITER
                                __queues->__Threads.iter_remove(&__ThrIter);
#else // QUEUE_NO_ITER
        __queues->__Threads.deleteElem(thr);
#endif // QUEUE_NO_ITER
                                thr->executeThread(__context);
                                continue;
                        }
                        // This allows a thread to be "completed" by something else...
                        if (thr->getStatus() == PAMI::Device::Complete) {
#ifndef QUEUE_NO_ITER
                                __queues->__Threads.iter_remove(&__ThrIter);
#else // QUEUE_NO_ITER
        __queues->__Threads.deleteElem(thr);
#endif // QUEUE_NO_ITER
                                continue;
                        }
                }
                //__queues->__Threads.mutex()->release();

                //+ core_mutex.release();

                // Now check everything on the completion queue...
                GenericMessage *msg, *nxtmsg, *nxt;
                for (msg = (GenericMessage *)__queues->__GenericQueue.peekHead(); msg; msg = nxtmsg) {
                        nxtmsg = (GenericMessage *)__queues->__GenericQueue.nextElem(msg);
                        if (msg->getStatus() == Done) {
                                ++events;
                                __queues->__GenericQueue.deleteElem(msg);
                                GenericDeviceMessageQueue *qs = msg->getQS();
                                // replace dequeue by remove to support multiple active messages in queue
                                // qs->dequeue(); // assert return == msg
                                qs->remove(msg);
                                nxt = (PAMI::Device::Generic::GenericMessage *)qs->peek();
                                if (nxt) nxt->postNext(true); // virtual function
                                msg->executeCallback(__context);
                        }
                }
                return events;
        }

        /// \brief     Advance routine for the generic device.
        ///
        /// Currently not used, since subdevices have to be polled for recvs.
        ///
        /// \return Boolean indicating if device needs advancing
        ///
        inline bool isAdvanceNeeded();

        /// \brief     Post a thread object on a generic device slice's queue
        ///
        /// Used this to post a thread of work.
        ///
        /// \param[in] thr  Thread object to post for advance work
        ///
        inline void postThread(GenericThread *thr) {
                __queues->__Threads.enqueue((GenericDeviceWorkQueue::Element *)thr);
        }

        /// \brief Post a message to the generic-device queuing system
        ///
        /// \param[in] msg  Message to be queued/completed.
        ///
        inline void postMsg(GenericMessage *msg) {
                __queues->__GenericQueue.enqueue((GenericDeviceCompletionQueue::Element *)msg);
        }

        /// \brief accessor for the context-id associated with generic device slice
        /// \return context ID
        inline size_t contextId() { return __contextId; }
        inline size_t clientId() { return __clientId; }

        /// \brief accessor for the total number of contexts in this client
        /// \return number of contexts/generic device slices
        inline size_t nContexts() { return __nContexts; }

        /// \brief accessor for the context associated with generic device slice
        /// \return context handle
        inline pami_context_t getContext() { return __context; }

        inline Memory::MemoryManager *getMM() { return __mm; }

        inline PAMI::Device::Generic::Device *getAllDevs() { return __allGds; }

private:
  struct GenericDeviceQueues {
          /// \brief Storage for the queue for message completion
          ///
          /// Queue[1] is used by the Generic::Device to enqueue messages for completion.
          /// By convention, queue[0] is used for attaching messages to a sub-device.
          ///
          GenericDeviceCompletionQueue __GenericQueue;

          /// \brief Storage for the queue of threads (a.k.a. work units)
          GenericDeviceWorkQueue __Threads;
  };

  GenericDeviceQueues *__queues;
#ifndef QUEUE_NO_ITER
        GenericDeviceWorkQueue::Iterator __ThrIter;
#endif // !QUEUE_NO_ITER

        pami_context_t __context; ///< context handle for this generic device
        size_t __clientId;    ///< client ID for context
        size_t __contextId;   ///< context ID
        size_t __nContexts;   ///< number of contexts in client
  Memory::MemoryManager *__mm;
  Device *__allGds;
}; /* class Device */

}; /* namespace Generic */
}; /* namespace Device */
}; /* namespace PAMI */

#endif /* __components_devices_generic_device_h__ */
