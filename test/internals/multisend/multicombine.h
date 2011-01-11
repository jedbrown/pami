/**
 * \file test/internals/multisend/multicombine.h
 * \brief ???
 */

#ifndef __test_internals_multisend_multicombine_h__
#define __test_internals_multisend_multicombine_h__

#include <stdio.h>
#include <pami.h>

#include "PipeWorkQueue.h"
#include "Topology.h"
#include "Global.h"
#undef USE_COMMTHREADS
#include "components/devices/generic/Device.h"

#include "memorymanager.h"

#include "components/devices/MulticombineModel.h"

namespace PAMI {
namespace Test {
namespace Multisend {

template <class T_MulticombineModel, class T_MulticombineDevice, int T_BufSize>
class Multicombine {
private:
        uint8_t _mdlbuf[sizeof(T_MulticombineModel)];
        T_MulticombineModel *_model;
        PAMI::Device::Generic::Device *_generics;
        T_MulticombineDevice *_dev;
        uint8_t _msgbuf[T_MulticombineModel::sizeof_msg];

        char _source[T_BufSize];
        char _result[T_BufSize];

        PAMI::PipeWorkQueue _ipwq;
        PAMI::PipeWorkQueue _opwq;

        pami_result_t _status;
        int _done;
        const char *_name;

        static void _done_cb(pami_context_t context, void *cookie, pami_result_t result) {
                PAMI::Test::Multisend::Multicombine<T_MulticombineModel,T_MulticombineDevice,T_BufSize> *thus =
                        (PAMI::Test::Multisend::Multicombine<T_MulticombineModel,T_MulticombineDevice,T_BufSize> *)cookie;
                //fprintf(stderr, "... completion callback for %s, done %d ++\n", thus->_name, thus->_done);
                ++thus->_done;
        }

public:

        Multicombine(const char *test, PAMI::Memory::MemoryManager &mm) :
        _name(test)
        {
                _generics = PAMI::Device::Generic::Device::Factory::generate(0, 1, mm, NULL);
                _dev = T_MulticombineDevice::Factory::generate(0, 1, mm, _generics);

                PAMI::Device::Generic::Device::Factory::init(_generics, 0, 0, NULL, (pami_context_t)1, &mm, _generics);
                T_MulticombineDevice::Factory::init(_dev, 0, 0, NULL, (pami_context_t)1, &mm, _generics);
                _model = new (_mdlbuf) T_MulticombineModel(T_MulticombineDevice::Factory::getDevice(_dev, 0, 0), _status);
        }

        ~Multicombine() { }

        inline pami_result_t perform_test(size_t task_id, size_t num_tasks,
					  pami_context_t ctx, pami_multicombine_t *mcomb) {
                pami_result_t rc;
                size_t x;

                if (_status != PAMI_SUCCESS) {
                        fprintf(stderr, "Failed to register multicombine \"%s\"\n", _name);
                        return PAMI_ERROR;
                }

                bool root = (mcomb->results_participants == NULL ||
                        ((PAMI::Topology *)mcomb->results_participants)->isRankMember(task_id));
                _ipwq.configure(_source, sizeof(_source), sizeof(_source));
                _ipwq.reset();
                _opwq.configure(_result, sizeof(_result), 0);
                _opwq.reset();

                mcomb->cb_done = (pami_callback_t){_done_cb, (void *)this};
                mcomb->data = (pami_pipeworkqueue_t *)&_ipwq;
                mcomb->results = (pami_pipeworkqueue_t *)&_opwq;

                //mcomb->count = TEST_BUF_SIZE / sizeof(unsigned);
                // temporary, until this gets smarter...
                if (mcomb->dtype != PAMI_UNSIGNED_INT || mcomb->optor != PAMI_SUM) {
                        fprintf(stderr, "unsupported test case operator/datatype\n");
                        return PAMI_ERROR;
                }
                for (x = 0; x < mcomb->count; ++x) {
                        ((unsigned *)_source)[x] = 1;
                        ((unsigned *)_result)[x] = -1;
                }
                _done = 0;
                //fprintf(stderr, "... before %s.postMulticombine\n", _name);
                rc = _model->postMulticombine(_msgbuf, 0, 0, mcomb);
                if (rc != PAMI_SUCCESS) {
                        fprintf(stderr, "Failed to post multicombine \"%s\"\n", _name);
                        return PAMI_ERROR;
                }

                //fprintf(stderr, "... before advance loop for %s.postMulticombine\n", _name);
                while (!_done) {
                        PAMI::Device::Generic::Device::Factory::advance(_generics, 0, 0);
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
                        return PAMI_ERROR;
                }
                return PAMI_SUCCESS;
        }

private:

}; // class Multicombine
}; // namespace Multisend
}; // namespace Test
}; // namespace PAMI

#endif // __pami_test_internals_multisend_multicombine_h__
