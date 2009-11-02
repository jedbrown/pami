#ifndef __test_internals_multisend_generic_multicast_h__
#define __test_internals_multisend_generic_multicast_h__

#include <stdio.h>
#include "sys/xmi.h"

#include "PipeWorkQueue.h"
#include "Topology.h"
#include "Global.h"

namespace XMI {
namespace Test {
namespace Multisend {

template <class T_MulticastModel, int T_BufSize>
class Multicast {
private:
	T_MulticastModel _model;
	char _msgbuf[T_MulticastModel::sizeof_msg];

	char _source[T_BufSize];
	char _result[T_BufSize];

	XMI::PipeWorkQueue _ipwq;
	XMI::PipeWorkQueue _opwq;

	xmi_result_t _status;
	int _done;
	const char *_name;

	static void _done_cb(xmi_context_t context, void *cookie, xmi_result_t result) {
		XMI::Test::Multisend::Multicast<T_MulticastModel,T_BufSize> *thus =
			(XMI::Test::Multisend::Multicast<T_MulticastModel,T_BufSize> *)cookie;
		//fprintf(stderr, "... completion callback for %s, done %d ++\n", thus->_name, thus->_done);
		++thus->_done;
	}

public:

	Multicast(const char *test) :
	_model(_status),
	_name(test)
	{
	}

	~Multicast() { }

	inline xmi_result_t perform_test(size_t task_id, size_t num_tasks,
						xmi_multicast_t *mcast) {
		xmi_result_t rc;
		bool res;
		size_t x, root;

		if (_status != XMI_SUCCESS) {
			fprintf(stderr, "Failed to register multicast \"%s\"\n", _name);
			return XMI_ERROR;
		}

		root = ((XMI::Topology *)mcast->src_participants)->index2Rank(0);

		_ipwq.configure(NULL, _source, sizeof(_source), sizeof(_source));
		_ipwq.reset();
		_opwq.configure(NULL, _result, sizeof(_result), 0);
		_opwq.reset();

		// simple allreduce on the local ranks...
		mcast->request = &_msgbuf;
		mcast->cb_done = (xmi_callback_t){_done_cb, (void *)this};
		mcast->src = (xmi_pipeworkqueue_t *)&_ipwq;
		mcast->dst = (xmi_pipeworkqueue_t *)&_opwq;
		size_t count = mcast->bytes / sizeof(unsigned);
        unsigned value = 0;
		for (x = 0; x < count; ++x) {
			((unsigned *)_source)[x] = (task_id == root)? value++ : -1;
			((unsigned *)_result)[x] = -1;
		}
		_done = 0;
		//fprintf(stderr, "... before %s.postMulticast\n", _name);
		res = _model.postMulticast(mcast);
		if (!res) {
			fprintf(stderr, "Failed to post multicast \"%s\"\n", _name);
			return XMI_ERROR;
		}

		//fprintf(stderr, "... before advance loop for %s.postMulticast\n", _name);
		while (!_done) {
			rc = XMI_Context_advance(mcast->context, 100);
			if (rc != XMI_SUCCESS) {
				fprintf (stderr, "Error. Unable to advance xmi context. result = %d\n", rc);
				return rc;
			}
		}
        unsigned errors = 0;
        value = 0;
		if (task_id == root) {
			for (x = 0; x < count && errors < 5 ; ++x) {
				if (((unsigned *)_source)[x] != value++) {
					fprintf(stderr, "Corrupted source buffer at index %zd. stop.\n", x);
					errors++;//break;
				}
				if (((unsigned *)_result)[x] != (unsigned)-1) {
                  fprintf(stderr, "Corrupted result buffer(%d) at index %zd. stop.\n",((unsigned *)_result)[x], x);
					errors++;//break;
				}
			}
		} else {
			for (x = 0; x < count  && errors < 5 ; ++x) {
				if (((unsigned *)_source)[x] != (unsigned)-1) {
					fprintf(stderr, "Corrupted source buffer at index %zd. stop.\n", x);
					errors++;//break;
				}
				if (((unsigned *)_result)[x] != value++) {
					fprintf(stderr, "Incorrect result buffer(%d) at index %zd. stop.\n",((unsigned *)_result)[x], x);
					errors++;//break;
				}
			}
		}
		if ((x - errors) < count) {
			return XMI_ERROR;
		}
		return XMI_SUCCESS;
	}

private:

}; // class Multicast
}; // namespace Multisend
}; // namespace Test
}; // namespace XMI

#endif // __xmi_test_internals_multisend_generic_multicast_h__
