/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file p2p/protocols/send/datagram/DTimer.h
/// \brief Datagram timer
///
/// The DTimer class defined in this file defines a timer which works
/// with the Generic device progress function
///
#ifndef __p2p_protocols_send_datagram_DTimer_h__
#define __p2p_protocols_send_datagram_DTimer_h__

#include "components/devices/generic/ProgressFunctionMsg.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

namespace XMI {
namespace Protocol {
namespace Send {
class DTimer {
public:
	DTimer() :
		_start_time(0), _interval(0), _timeout_cb(NULL), _timeout_cookie(NULL),
				_max_timeouts(0), _max_cb(NULL), _max_cookie(NULL),
				_running(false), _closed(false) {
	}

	xmi_result_t start(XMI::Device::ProgressFunctionMdl progfmodel,
			unsigned long long interval, xmi_event_function timeout_cb, void * timeout_cookie,
			unsigned long max_timeouts, xmi_event_function max_cb, void * max_cookie) {
		TRACE_ERR((stderr,"DTimer(%p) start() ... Starting timer with interval = %llu \n", this, interval));
		XMI_ProgressFunc_t progf;

		_start_time = 0;
		_interval = interval;
		_timeout_cb = timeout_cb;
		_timeout_cookie = timeout_cookie;
		_max_timeouts = max_timeouts;
		_max_timeouts_left = max_timeouts;
		_max_cb = max_cb;
		_max_cookie = max_cookie;
		_running = true;
		_closed = false;

		progf.request = &pmsgbuf;
		progf.clientdata = this;
#warning fix this progress fnction stuff
		//progf.client = client;
		//progf.context = contextid;
		progf.func = advance_timer_cb;  // xmi_work_function
		progf.cb_done = (xmi_callback_t) {NULL};
		bool rc = progfmodel.postWork(&progf);
		if (!rc) {
			TRACE_ERR((stderr,"DTimer(%p) advance_timer() ... Failed to generateMessage on progress function\n",this));
			return XMI_ERROR;
		}
		return XMI_SUCCESS;
	}

	static xmi_result_t advance_timer_cb(xmi_context_t context, void * me) {
		DTimer *dt = (DTimer *)me;
		return dt->advance_timer(context);
	}

	xmi_result_t advance_timer(xmi_context_t context) {
		if (_running) {
			// If initial time not set, set it
			if (_start_time == 0) {
				_start_time = (unsigned long long)__global.time.time();TRACE_ERR((stderr,
								"	DTimer(%p) advance_timer().. Starting Timer at tick %llu, waiting until %llu\n",
								this, _start_time, _start_time + _interval));
			}

			// See if timed out
			unsigned long long t1 = (unsigned long long)__global.time.time();
			if (t1 - _start_time >= _interval) {
				// Timed out
				TRACE_ERR((stderr,
								"	DTimer(%p) advance_timer().. Timeout at tick %llu (%ld calls)\n",
								this, t1, _max_timeouts));
				_start_time = 0; //reset timer
				--_max_timeouts; //decrement number of timeouts left
				if (_max_timeouts == 0) {
					// out of timeouts
					TRACE_ERR((stderr, "   DTimer(%p) advance_timer() ...  Maxed Out\n", this));
					if (_max_cb) {
						_max_cb(context, _max_cookie, XMI_ERROR);
					}
					return XMI_NERROR;
				} else {
					// normal timeout processing
					TRACE_ERR((stderr, "   DTimer(%p) advance_timer() ...  Normal timeout processing\n", this));
					if (_timeout_cb) {
						_timeout_cb(context, _timeout_cookie, XMI_EAGAIN);
					}
				}
			}
			return XMI_EAGAIN;
		}
		if (_closed) return XMI_SUCCESS;  // Remove from advance
		return XMI_EAGAIN; // Keep advancing, but don't do anything
	}

	void reset() {
		if (_closed) abort(); // can't reset
		_start_time = 0;
		_max_timeouts_left = _max_timeouts;
		_running = true;
	}

	void stop() {
		_running = false;
	}

	void close() {
		_closed = true;
	}

protected:
private:
	char pmsgbuf[XMI::Device::ProgressFunctionMdl::sizeof_msg];

	unsigned long long _start_time; ///starting time
	unsigned long long _interval; ///define interval
	xmi_event_function _timeout_cb;
	void * _timeout_cookie;
	unsigned long _max_timeouts; ///time out condition
	unsigned long _max_timeouts_left;
	xmi_event_function _max_cb;
	void * _max_cookie;
	bool _running; ///running flag
	bool _closed;  ///closed flag


}; // XMI::Protocol::Send::DTimer class
}
; // XMI::Protocol::Send namespace
}
; // XMI::Protocol namespace
}
; // XMI namespace

#undef TRACE_ERR
#endif // __xmi_p2p_protocol_send_datagram_DTimer_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
