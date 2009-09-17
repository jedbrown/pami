/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __components_devices_generic_thread_h__
#define __components_devices_generic_thread_h__

#include "util/queue/Queue.h"
#include "logging/Logging.h"
#include "components/devices/BaseDevice.h"
#include "SysDep.h"

////////////////////////////////////////////////////////////////////////
///  \file components/devices/generic/AdvanceThread.h
///  \brief Generic Device Threads implementation
///
////////////////////////////////////////////////////////////////////////
namespace XMI {
namespace Device {
namespace Generic {

//////////////////////////////////////////////////////////////////////
///  \brief A Device implmentation of a Queuesystem
///  This class implements a queue system. The user posts to the queue
///  and the interprocess communications are called in order until all of them
///  have been executed.
//////////////////////////////////////////////////////////////////////
class GenericMessage;
class GenericAdvanceThread : public QueueElem {

public:
	GenericAdvanceThread() :
	QueueElem(),
	_msg(NULL),
	_dev_wake(NULL),
	_dev_chan(0),
	_polled(true),
	_done(false)
	{
	}

	inline void setMsg(GenericMessage *msg) { _msg = msg; }
	inline GenericMessage *getMsg() { return _msg; }

	inline void setWakeVec(void *v) { _dev_wake = v; }
	inline void *getWakeVec() { return _dev_wake; }

	inline void setChannel(int c) { _dev_chan = c; }
	inline int getChannel() { return _dev_chan; }

	inline void setDone(bool f) { _done = f; }
	inline bool isDone() { return _done; }

	inline void setPolled(bool f) { _polled = f; }
	inline bool isPolled() { return _polled; }

protected:
	GenericMessage *_msg;
	void *_dev_wake;
	int _dev_chan;
	bool _polled;
	bool _done;
}; /* class GenericAdvanceThread */

class SimpleAdvanceThread : public GenericAdvanceThread {
public:
	SimpleAdvanceThread() :
	GenericAdvanceThread(),
	_bytesLeft(0)
	{
	}
public:
	size_t _bytesLeft;
}; /* class SimpleAdvanceThread */

}; /* namespace Generic */
}; /* namespace Device */
}; /* namespace XMI */

#endif /* __components_devices_generic_thread_h__ */
