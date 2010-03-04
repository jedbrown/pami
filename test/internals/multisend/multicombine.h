/**
 * \file test/internals/multisend/multicombine.h
 * \brief ???
 */

#ifndef __test_internals_multisend_multicombine_h__
#define __test_internals_multisend_multicombine_h__

#include <stdio.h>
#include "sys/xmi.h"

#include "PipeWorkQueue.h"
#include "Topology.h"
#include "Global.h"

#include "memorymanager.h"

namespace XMI {
namespace Test {
namespace Multisend {

template <class T_MulticombineModel, class T_MulticombineDevice, int T_BufSize>
class Multicombine {
private:
	uint8_t _mdlbuf[sizeof(T_MulticombineModel)];
	T_MulticombineModel *_model;
	XMI::SysDep _sd;
	XMI::Device::Generic::Device *_generics;
	T_MulticombineDevice *_dev;
	uint8_t _msgbuf[T_MulticombineModel::sizeof_msg];

	char _source[T_BufSize];
	char _result[T_BufSize];

	XMI::PipeWorkQueue _ipwq;
	XMI::PipeWorkQueue _opwq;

	xmi_result_t _status;
	int _done;
	const char *_name;

	static void _done_cb(xmi_context_t context, void *cookie, xmi_result_t result) {
		XMI::Test::Multisend::Multicombine<T_MulticombineModel,T_MulticombineDevice,T_BufSize> *thus =
			(XMI::Test::Multisend::Multicombine<T_MulticombineModel,T_MulticombineDevice,T_BufSize> *)cookie;
		//fprintf(stderr, "... completion callback for %s, done %d ++\n", thus->_name, thus->_done);
		++thus->_done;
	}

public:

	Multicombine(const char *test, XMI::Memory::MemoryManager &mm) :
	_sd(mm),
	_name(test)
	{
		_generics = XMI::Device::Generic::Device::Factory::generate(0, 1, _sd.mm);
		_dev = T_MulticombineDevice::Factory::generate(0, 1, _sd.mm);

		XMI::Device::Generic::Device::Factory::init(_generics, 0, 0, NULL, (xmi_context_t)1, &_sd, _generics);
		T_MulticombineDevice::Factory::init(_dev, 0, 0, NULL, (xmi_context_t)1, &_sd, _generics);
		_model = new (_mdlbuf) T_MulticombineModel(T_MulticombineDevice::Factory::getDevice(_dev, 0, 0), _status);
	}

	~Multicombine() { }

	inline xmi_result_t perform_test(size_t task_id, size_t num_tasks,
					xmi_context_t ctx, xmi_multicombine_t *mcomb) {
		xmi_result_t rc;
		size_t x;

		if (_status != XMI_SUCCESS) {
			fprintf(stderr, "Failed to register multicombine \"%s\"\n", _name);
			return XMI_ERROR;
		}

		bool root = (mcomb->results_participants == NULL ||
			((XMI::Topology *)mcomb->results_participants)->isRankMember(task_id));
		_ipwq.configure(NULL, _source, sizeof(_source), sizeof(_source));
		_ipwq.reset();
		_opwq.configure(NULL, _result, sizeof(_result), 0);
		_opwq.reset();

		mcomb->cb_done = (xmi_callback_t){_done_cb, (void *)this};
		mcomb->data = (xmi_pipeworkqueue_t *)&_ipwq;
		mcomb->results = (xmi_pipeworkqueue_t *)&_opwq;

		//mcomb->count = TEST_BUF_SIZE / sizeof(unsigned);
		// temporary, until this gets smarter...
		if (mcomb->dtype != XMI_UNSIGNED_INT || mcomb->optor != XMI_SUM) {
			fprintf(stderr, "unsupported test case operator/datatype\n");
			return XMI_ERROR;
		}
		for (x = 0; x < mcomb->count; ++x) {
			((unsigned *)_source)[x] = 1;
			((unsigned *)_result)[x] = -1;
		}
		_done = 0;
		//fprintf(stderr, "... before %s.postMulticombine\n", _name);
		rc = _model->postMulticombine(_msgbuf, mcomb);
		if (rc != XMI_SUCCESS) {
			fprintf(stderr, "Failed to post multicombine \"%s\"\n", _name);
			return XMI_ERROR;
		}

		//fprintf(stderr, "... before advance loop for %s.postMulticombine\n", _name);
		while (!_done) {
			XMI::Device::Generic::Device::Factory::advance(_generics, 0, 0);
			T_MulticombineDevice::Factory::advance(_dev, 0, 0);
		}
		for (x = 0; x < mcomb->count; ++x) {
			if (((unsigned *)_source)[x] != 1) {
				fprintf(stderr, "Corrupted source buffer at index %zu. stop.\n", x);
				break;
			}
			if (root) {
				if (((unsigned *)_result)[x] != num_tasks) {
					fprintf(stderr, "Incorrect result at index %zu. stop.\n", x);
					break;
				}
			} else {
				if (((unsigned *)_result)[x] != (unsigned)-1) {
					fprintf(stderr, "Corrupted result buffer at index %zu. stop.\n", x);
					break;
				}
			}
		}
		if (x < mcomb->count) {
			return XMI_ERROR;
		}
		return XMI_SUCCESS;
	}

private:

}; // class Multicombine
}; // namespace Multisend
}; // namespace Test
}; // namespace XMI

#endif // __xmi_test_internals_multisend_multicombine_h__
