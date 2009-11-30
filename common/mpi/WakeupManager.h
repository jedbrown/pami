
///
/// \file common/mpi/WakeupManager.h
/// \brief XMI wakeup manager implementation.
///
#ifndef __common_mpi_WakeupManager_h__
#define __common_mpi_WakeupManager_h__

#include "common/WakeupManagerInterface.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#undef USE_WAKEUP_VECTORS

namespace XMI {

        class WakeupManager : public Interface::WakeupManager<XMI::WakeupManager> {
		// everything must fit in a (void *)
		#define SEMID_BITS	((sizeof(void *) / 2) * 8)
		#define SEMNO_BITS	SEMID_BITS

		#define MAX_SEMID	((1 << SEMID_BITS) - 1)
		#define MAX_SEMNO	((1 << SEMNO_BITS) - 1)
        public:
		inline WakeupManager() :
		Interface::WakeupManager<XMI::WakeupManager>(),
		_semKey(IPC_PRIVATE),
		_semSet(0),
		_semNum(0)
		{
		}

		inline xmi_result_t init_impl(int num, int setKey) {
			if (_semKey != IPC_PRIVATE) {
				return XMI_ERROR;
			}
			if (num > MAX_SEMNO) {
				return XMI_ERROR;
			}
			key_t key = (key_t)setKey;
			XMI_assertf(key != IPC_PRIVATE, "WakeupManager called with illegal key value %d\n", key);
			int rc = semget(key, num, IPC_CREAT);
			if (rc < 0) {
				return XMI_ERROR;
			}
			if (rc > MAX_SEMID) {
				return XMI_ERROR;
			}
			_semSet = rc;
			_semKey = key;
			_semNum = num;
			return XMI_SUCCESS;
		}

		inline void *getWakeupVec_impl(int num) {
			if (num >= _semNum || num < 0) {
				return NULL;
			}
			size_t z = ((num + 1) << SEMNO_BITS) | _semSet;
			return (void *)z;
		}

		inline xmi_result_t wakeup_impl(void *v) {
			struct sembuf op;
			size_t z = (size_t)v;
			op.sem_num = ((z >> SEMNO_BITS) & MAX_SEMNO) - 1;
			op.sem_op = 1;	// "up" the semaphore
			op.sem_flg = IPC_NOWAIT; // just in case...
			// assert(_semSet == (z & MAX_SEMID));
			int err = semop(_semSet, &op, 1);
			if (err < 0) {
				return XMI_ERROR;
			}
			return XMI_SUCCESS;
		}

		inline xmi_result_t clear_impl(void *v) {
			union semun arg;
			size_t z = (size_t)v;
			int sem_num = ((z >> SEMNO_BITS) & MAX_SEMNO) - 1;
			arg.val = 0;
			// assert(_semSet == (z & MAX_SEMID));
			int err = semctl(_semSet, sem_num, SETVAL, &arg);
			if (err < 0) { 
				return XMI_ERROR;
			}
			return XMI_SUCCESS;
		}

		inline xmi_result_t sleep_impl(void *v) {
			struct sembuf op;
			size_t z = (size_t)v;
			op.sem_num = ((z >> SEMNO_BITS) & MAX_SEMNO) - 1;
			op.sem_op = -1; // 'down' the semaphore
			op.sem_flg = 0;
			// assert(_semSet == (z & MAX_SEMID));
			int err = semop(_semSet, &op, 1);
			if (err < 0) { 
				return XMI_ERROR;
			}
			clear_impl(v);
			return XMI_SUCCESS;
		}

		inline xmi_result_t try_impl(void *v) {
			struct sembuf op;
			size_t z = (size_t)v;
			op.sem_num = ((z >> SEMNO_BITS) & MAX_SEMNO) - 1;
			op.sem_op = -1; // 'down' the semaphore
			op.sem_flg = IPC_NOWAIT;
			// assert(_semSet == (z & MAX_SEMID));
			int err = semop(_semSet, &op, 1);
			if (err == EAGAIN) { 
				return XMI_EAGAIN;
			}
			if (err < 0) { 
				return XMI_ERROR;
			}
			clear_impl(v);
			return XMI_SUCCESS;
		}

		inline xmi_result_t poll_impl(void *v) {
			size_t z = (size_t)v;
			int s = ((z >> SEMNO_BITS) & MAX_SEMNO) - 1;
			// assert(_semSet == (z & MAX_SEMID));
			int err = semctl(_semSet, s, GETVAL);
			if (err < 0) { 
				return XMI_ERROR;
			}
			if ((short)err <= 0) { 
				return XMI_EAGAIN;
			}
			return XMI_SUCCESS;
		}

	private:
		key_t _semKey;
		int _semSet;
		int _semNum;

        }; // class WakeupManager
}; // namespace XMI

#endif // __common_mpi_WakeupManager_h__
