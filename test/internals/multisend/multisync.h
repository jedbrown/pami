/**
 * \file test/internals/multisend/multisync.h
 * \brief ???
 */

#ifndef __test_internals_multisend_multisync_h__
#define __test_internals_multisend_multisync_h__

#include <stdio.h>
#include "sys/xmi.h"

#include "Topology.h"
#include "Global.h"

namespace XMI {
namespace Test {
namespace Multisend {

template <class T_MultisyncModel>
class Multisync {
private:
	T_MultisyncModel _model;
	char _msgbuf[T_MultisyncModel::sizeof_msg];
	xmi_result_t _status;
	int _done;
	const char *_name;

	static void _done_cb(xmi_context_t context, void *cookie, xmi_result_t result) {
		XMI::Test::Multisend::Multisync<T_MultisyncModel> *thus = (XMI::Test::Multisend::Multisync<T_MultisyncModel> *)cookie;
		// printf skews timing too much...
		//fprintf(stderr, "... completion callback for %s, done %d++\n", thus->_name, thus->_done);
		++thus->_done;
	}

public:
	unsigned long long delay;
	unsigned long long raw_time;
	unsigned long long total_time;
	unsigned long long barrier_time;

	Multisync(const char *test) :
	_model(_status),
	_name(test)
	{
	}

	~Multisync() {}

	/// msync is partially filled-in.
	///
	inline xmi_result_t perform_test(size_t task_id, size_t num_tasks,
					xmi_multisync_t *msync) {
		unsigned long long t0, t1, t2;
		xmi_result_t rc;
		bool res;

		if (_status != XMI_SUCCESS) {
			fprintf(stderr, "Failed to register multisync \"%s\"\n", _name);
			return XMI_ERROR;
		}

		msync->cb_done = (xmi_callback_t){_done_cb, (void *)this};
		msync->request = &_msgbuf;

		// Do three barriers: first is to synchronize the ranks so that
		// the timing of the follwing barriers are cleaner. Also, no printfs
		// in the barriers because that messes up timing.  So, if the barrier
		// needs debug then the printfs will have to be re-enabled.

		// Basic timing profile: Each rank will delay prior to the start
		// by the rank multiplied by 1000 (cycles). The idea being, if the
		// barrier actually does it's job then the total time spent by each
		// rank should be (relatively) close to the others. NOTE: these times
		// are not to be used as performance numbers! They intentionally introduce
		// extra time in order to show that the barrier is functional.

		// first barrier: get everyone together
		_done = 0;
		//fprintf(stderr, "... before %s.postMultisync\n", _name);
		res = _model.postMultisync(msync);
		if (!res) {
			fprintf(stderr, "Failed to post first multisync \"%s\"\n", _name);
			return XMI_ERROR;
		}
		// printf skews timing too much...
		//fprintf(stderr, "... before advance loop for %s.postMultisync\n", _name);
		while (!_done) {
			rc = XMI_Context_advance(XMI_Client_getcontext(msync->client,msync->context), 100);
			if (rc != XMI_SUCCESS) {
				fprintf (stderr, "Failed advance first \"%s\"\n", _name);
				return rc;
			}
		}

		// second barrier: get an accurate time
		++msync->connection_id;
		_done = 0;
		t0 = __global.time.timebase();
		res = _model.postMultisync(msync);
		if (!res) {
			fprintf(stderr, "Failed to post second multisync \"%s\"\n", _name);
			return XMI_ERROR;
		}
		while (!_done) {
			rc = XMI_Context_advance(XMI_Client_getcontext(msync->client,msync->context), 100);
			if (rc != XMI_SUCCESS) {
				fprintf (stderr, "Failed advance second \"%s\"\n", _name);
				return rc;
			}
		}
		raw_time = __global.time.timebase() - t0;
		delay = raw_time * task_id;

		// In order to get meaningful results below, we need to vary each rank's
		// arrival time at the barrier by a "significant" amount. We use the
		// time it took for the initial barrier, multiplied by our task ID, to
		// create noticable differences in arrival time (we hope).
		//
		// The resulting timings should show that barrier times (in parens)
		// vary a lot while the overall times should be more closely aligned.
		//
		// +--------------------------------------------------------------+
		// |                                        |    barrier          |
		// +--------------------------------------------------------------+
		// |                         |    barrier                         |
		// +--------------------------------------------------------------+
		// |               |    barrier                                   |
		// +--------------------------------------------------------------+
		// |                                             |    barrier     |
		// +--------------------------------------------------------------+
		// t0            (t1[x] ...         ...      ... )                t2
		//

		// third barrier: check that it really synchronizes participants
		++msync->connection_id;
		_done = 0;
		t0 = __global.time.timebase();
		while ((t1 = __global.time.timebase()) - t0 < delay);
		res = _model.postMultisync(msync);
		if (!res) {
			fprintf(stderr, "Failed to post third multisync \"%s\"\n", _name);
			return XMI_ERROR;
		}

		// printf skews timing too much...
		//fprintf(stderr, "... before advance loop for %s.postMultisync\n", _name);
		while (!_done) {
			rc = XMI_Context_advance(XMI_Client_getcontext(msync->client,msync->context), 100);
			if (rc != XMI_SUCCESS) {
				fprintf (stderr, "Failed advance last \"%s\"\n", _name);
				return rc;
			}
		}
		t2 = __global.time.timebase();
		total_time = t2 - t0;
		barrier_time = t2 - t1;

		return XMI_SUCCESS;
	}

private:

}; // class Multisync
}; // namespace Multisend
}; // namespace Test
}; // namespace XMI

#endif // __xmi_test_internals_multisend_multisync_h__
