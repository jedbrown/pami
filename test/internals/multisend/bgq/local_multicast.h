/**
 * \file test/internals/multisend/bgq/local_multicast.h
 * \brief ???
 */

#ifndef __test_internals_multisend_bgq_local_multicast_h__
#define __test_internals_multisend_bgq_local_multicast_h__

#include <stdio.h>
#include <pami.h>

#include "PipeWorkQueue.h"
#include "Topology.h"
#include "Global.h"
#undef USE_COMMTHREADS
#include "components/devices/generic/Device.h"

#include "components/devices/MulticastModel.h"

//#define DATA_CHECK

namespace PAMI {
namespace Test {
namespace Multisend {

template <class T_MulticastModel, class T_MulticastDevice, int T_BufSize>
class Multicast {
private:
        uint8_t _mdlbuf[sizeof(T_MulticastModel)];
        T_MulticastModel *_model;
        uint8_t _msgbuf[T_MulticastModel::sizeof_msg];
        PAMI::Device::Generic::Device *_generics;
        T_MulticastDevice *_dev;

        char* _source;
        char* _result;

        PAMI::PipeWorkQueue _ipwq;
        PAMI::PipeWorkQueue _opwq;

        pami_result_t _status;
        int _done;
        const char *_name;
    pami_quad_t     _msginfo;
    size_t _dispatch_id;

        static void _done_cb(pami_context_t context, void *cookie, pami_result_t result) {
                PAMI::Test::Multisend::Multicast<T_MulticastModel,T_MulticastDevice,T_BufSize> *thus =
                        (PAMI::Test::Multisend::Multicast<T_MulticastModel,T_MulticastDevice,T_BufSize> *)cookie;
        //fprintf(stderr, "... completion callback for %s, done %d ++\n", thus->_name, thus->_done);
                ++thus->_done;
        }

        static void dispatch_multicast_fn(const pami_quad_t     *msginfo,
                                          unsigned              msgcount,
                                          unsigned              connection_id,
                                          size_t                root,
                                          size_t                sndlen,
                                          void                 *clientdata,
                                          size_t               *rcvlen,
                                          pami_pipeworkqueue_t **rcvpwq,
                                          pami_callback_t       *cb_done)
        {
          //fprintf(stderr,"%s:%s msgcount %d, connection_id %d, root %d, sndlen %d, cookie %s\n",
          //        __FILE__,__PRETTY_FUNCTION__,msgcount, connection_id, root, sndlen, (char*) clientdata);
          PAMI::Test::Multisend::Multicast<T_MulticastModel,T_MulticastDevice,T_BufSize> *test =
            (PAMI::Test::Multisend::Multicast<T_MulticastModel,T_MulticastDevice,T_BufSize> *) clientdata;
          PAMI_assertf(sndlen <= T_BufSize,"sndlen %zu\n",sndlen);
          PAMI_assertf(msgcount == 1,"msgcount %d",msgcount);
          PAMI_assertf(msginfo->w0 == test->_msginfo.w0,"msginfo->w0=%d\n",msginfo->w0);
          PAMI_assertf(msginfo->w1 == test->_msginfo.w1,"msginfo->w1=%d\n",msginfo->w1);
          PAMI_assertf(msginfo->w2 == test->_msginfo.w2,"msginfo->w2=%d\n",msginfo->w2);
          PAMI_assertf(msginfo->w3 == test->_msginfo.w3,"msginfo->w3=%d\n",msginfo->w3);

          if(connection_id == 0) // no data being sent
          {
            *rcvlen = 0;
            *rcvpwq = (pami_pipeworkqueue_t*) NULL;
            if(sndlen == 0)
              fprintf(stderr, "PASS: msgdata received with no data\n");
            else
              fprintf(stderr, "FAIL: no data expected\n");
          }
          else
          {
            *rcvlen = sndlen;
            *rcvpwq = (pami_pipeworkqueue_t*) &test->_opwq;
          }

          *cb_done = (pami_callback_t)
          {
            _done_cb, (void *)test
          };

        }

public:

        Multicast(const char *test, PAMI::Memory::MemoryManager &mm) :
        _name(test)
        {
		pami_result_t rc;
		uint64_t my_alignment;
		my_alignment = 128;
		rc = __global.heap_mm->memalign((void **)&_source, my_alignment, T_BufSize);
		if (rc != PAMI_SUCCESS) printf("malloc failed\n");

		rc = __global.heap_mm->memalign((void **)&_result, my_alignment, T_BufSize);
		if (rc != PAMI_SUCCESS) printf("malloc failed\n");

                _generics = PAMI::Device::Generic::Device::Factory::generate(0, 1, __global.mm, NULL);
                _dev = T_MulticastDevice::Factory::generate(0, 1, mm, _generics);

                PAMI::Device::Generic::Device::Factory::init(_generics, 0, 0, NULL, (pami_context_t)1, &__global.mm, _generics);
                T_MulticastDevice::Factory::init(_dev, 0, 0, NULL, (pami_context_t)1, &mm, _generics);
                _model = new (_mdlbuf) T_MulticastModel(T_MulticastDevice::Factory::getDevice(_dev, 0, 0), _status);
        }

        Multicast(const char *test, size_t dispatch_id, PAMI::Memory::MemoryManager &mm) :
        _name(test),
    _dispatch_id(dispatch_id)
        {

                _generics = PAMI::Device::Generic::Device::Factory::generate(0, 1, __global.mm, NULL);
                _dev = T_MulticastDevice::Factory::generate(0, 1, mm, _generics);

                PAMI::Device::Generic::Device::Factory::init(_generics, 0, 0, NULL, (pami_context_t)1, &__global.mm, _generics);
                T_MulticastDevice::Factory::init(_dev, 0, 0, NULL, (pami_context_t)1, &mm, _generics);
                _model = new (_mdlbuf) T_MulticastModel(T_MulticastDevice::Factory::getDevice(_dev, 0, 0), _status);

      _msginfo.w0 = 0;
      _msginfo.w1 = 1;
      _msginfo.w2 = 2;
      _msginfo.w3 = 3;
      _model->registerMcastRecvFunction (dispatch_id,&dispatch_multicast_fn, (void*)this);
        }


        ~Multicast() { }

        inline unsigned long long perform_test(size_t task_id, size_t num_tasks,
                                        pami_context_t ctx, pami_multicast_t *mcast) {
                pami_result_t rc;
                size_t  root;

                if (_status != PAMI_SUCCESS) {
                        fprintf(stderr, "Failed to register multicast \"%s\"\n", _name);
                        return PAMI_ERROR;
                }

                root = ((PAMI::Topology *)mcast->src_participants)->index2Rank(0);

                //_ipwq.configure(_source, T_BufSize, T_BufSize);
                _ipwq.configure(_source, mcast->bytes, mcast->bytes);
                _ipwq.reset();
                //_opwq.configure(_result, T_BufSize, 0);
                _opwq.configure(_result, mcast->bytes, 0);
                _opwq.reset();

                // simple allreduce on the local ranks...
                mcast->dispatch = _dispatch_id;
                mcast->msgcount = 1;
                mcast->msginfo = &_msginfo;
                mcast->cb_done = (pami_callback_t){_done_cb, (void *)this};
                mcast->src = (pami_pipeworkqueue_t *)&_ipwq;
                mcast->dst = (pami_pipeworkqueue_t *)&_opwq;
#ifdef DATA_CHECK
				size_t x;
                size_t count = mcast->bytes / sizeof(unsigned);
                for (x = 0; x < count; ++x) {
                        ((unsigned *)_source)[x] = 1;
                        ((unsigned *)_result)[x] = -1;
                }
#endif
                _done = 0;

				 unsigned long long t1 = PAMI_Wtimebase();
                rc = _model->postMulticast(_msgbuf, mcast);
                if (rc != PAMI_SUCCESS) {
                        fprintf(stderr, "Failed to post multicast \"%s\"\n", _name);
                        return PAMI_ERROR;
                }
                while (!_done) {
                        PAMI::Device::Generic::Device::Factory::advance(_generics, 0, 0);
                        T_MulticastDevice::Factory::advance(_dev, 0, 0);
                }
				 unsigned long long t2 = PAMI_Wtimebase();

#ifdef DATA_CHECK
                if (task_id == root) {
                        for (x = 0; x < count; ++x) {
                                if (((unsigned *)_source)[x] != 1) {
                                        fprintf(stderr, "Corrupted source buffer at index %zu. stop.\n", x);
                                        break;
                                }
                                if (((unsigned *)_result)[x] != (unsigned)-1) {
                                        fprintf(stderr, "Corrupted result buffer at index %zu. stop.\n", x);
                                        break;
                                }
                        }
                } else {
                        for (x = 0; x < count; ++x) {
                                if (((unsigned *)_source)[x] != 1) {
                                        fprintf(stderr, "Corrupted source buffer at index %zu. stop.\n", x);
                                        break;
                                }
                                if (((unsigned *)_result)[x] != 1) {
                                        fprintf(stderr, "Incorrect result at index %zu. stop.\n", x);
                                        break;
                                }
                        }
                }
                if (x < count) {
                        return PAMI_ERROR;
                }
#endif
				return (t2-t1);
        }

        inline pami_result_t perform_test_active_message(size_t task_id, size_t num_tasks,
                                        pami_context_t ctx, pami_multicast_t *mcast) {
                pami_result_t rc;
                bool res;
                size_t x, root;

                if (_status != PAMI_SUCCESS) {
                        fprintf(stderr, "Failed to register multicast \"%s\"\n", _name);
                        return PAMI_ERROR;
                }

                root = ((PAMI::Topology *)mcast->src_participants)->index2Rank(0);

                _ipwq.configure(_source, sizeof(_source), sizeof(_source));
                _ipwq.reset();
                _opwq.configure(_result, sizeof(_result), 0);
                _opwq.reset();

                // simple allreduce on the local ranks...
                mcast->dispatch = _dispatch_id;
                mcast->msgcount = 1;
                mcast->msginfo = &_msginfo;
                mcast->cb_done = (pami_callback_t){_done_cb, (void *)this};
                mcast->src = (pami_pipeworkqueue_t *)&_ipwq;
                mcast->dst = (pami_pipeworkqueue_t *)&_opwq;
                size_t count = mcast->bytes / sizeof(unsigned);
                for (x = 0; x < count; ++x) {
                        ((unsigned *)_source)[x] = 1;
                        ((unsigned *)_result)[x] = -1;
                }
                _done = 0;
                if (task_id == root) {
                        //fprintf(stderr, "... before %s.postMulticast\n", _name);
                        rc = _model->postMulticast(_msgbuf,mcast);
                        if (rc != PAMI_SUCCESS) {
                                fprintf(stderr, "Failed to post multicast \"%s\"\n", _name);
                                return PAMI_ERROR;
                        }
                }
                //fprintf(stderr, "... before advance loop for %s.postMulticast\n", _name);
                while (!_done) {
                        PAMI::Device::Generic::Device::Factory::advance(_generics, 0, 0);
                        T_MulticastDevice::Factory::advance(_dev, 0, 0);
                }
                if (task_id == root) {
                        for (x = 0; x < count; ++x) {
                                if (((unsigned *)_source)[x] != 1) {
                                        fprintf(stderr, "Corrupted source buffer at index %zu. stop.\n", x);
                                        break;
                                }
                                if (((unsigned *)_result)[x] != (unsigned)-1) {
                                        fprintf(stderr, "Corrupted result buffer at index %zu. stop.\n", x);
                                        break;
                                }
                        }
                } else {
                        for (x = 0; x < count; ++x) {
                                if (((unsigned *)_source)[x] != 1) {
                                        fprintf(stderr, "Corrupted source buffer at index %zu. stop.\n", x);
                                        break;
                                }
                                if (((unsigned *)_result)[x] != 1) {
                                        fprintf(stderr, "Incorrect result at index %zu. stop.\n", x);
                                        break;
                                }
                        }
                }
                if (x < count) {
                        return PAMI_ERROR;
                }
                return PAMI_SUCCESS;
        }

private:

}; // class Multicast
}; // namespace Multisend
}; // namespace Test
}; // namespace PAMI

#endif // __pami_test_internals_multisend_multicast_h__
