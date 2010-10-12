/**
 * \file components/devices/shmemcoll/ShmemCollDesc.h
 * \brief ???
 */
#ifndef __components_devices_shmemcoll_ShmemCollDesc_h__
#define __components_devices_shmemcoll_ShmemCollDesc_h__

#include "components/devices/MulticastModel.h"
#include "components/devices/MultisyncModel.h"
#include "components/devices/MulticombineModel.h"

#include "Arch.h"
#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif


namespace PAMI
{
  namespace Device
  {
    namespace Shmem
    {

#define DESCRIPTOR_FIFO_SIZE    16
//#define DESCRIPTOR_FIFO_SIZE   32
//#define	BUFFER_SIZE_PER_TASK	256
#define	BUFFER_SIZE_PER_TASK	512
#define NUM_LOCAL_TASKS			16


		typedef enum
		{
			MULTICAST=0,
			MULTICOMBINE,
			MULTISYNC
		}CollType_t;

		typedef enum
		{
			FREE=0,
			INIT,
			ACTIVE,
			DONE
		}DescState_t;

		template < class T_Atomic >
		class ShmemCollDesc
		{

			private:

			uint16_t        		_dispatch_id; 	/* Invoke the dispatch after match */
			unsigned 				_master;		/* Master of the collective */
			CollType_t				_type;
			void*					_storage;
			int						_connid;		/* Initialize with -1		*/
			T_Atomic				_synch_counter;	/* Whether everyone has arrived */
			T_Atomic				_done_counter; 	/* Whether everyone finished 	*/
			DescState_t				_my_state;		/* Any of FREE, INIT, ACTIVE, DONE */
			//T_Atomic				_seq_num;
			volatile uint64_t		_seq_num __attribute__((__aligned__(8)));;
			unsigned				_num_consumers;
			volatile unsigned		_flag;
			//union
			//{
				pami_multicast_t	_mcast;
				pami_multicombine_t	_mcomb;

			//}						_coll_params;
			char					_buffer[NUM_LOCAL_TASKS*BUFFER_SIZE_PER_TASK] __attribute__((__aligned__(128)));

			public:

			inline ShmemCollDesc():_master(0),_storage(NULL),_my_state(FREE),_seq_num(0),_num_consumers(0),_flag(0){};
			inline ~ShmemCollDesc() {}

			inline void init(Memory::MemoryManager *mm)
			{
				_synch_counter.init (mm);
          		_synch_counter.fetch_and_clear ();
				_done_counter.init (mm);
          		_done_counter.fetch_and_clear ();
				//_seq_num.init (mm);
          		//_seq_num.fetch_and_clear ();
				_num_consumers = 0;
				_connid = -1;
			}

			inline void reset()
			{
				_synch_counter.fetch_and_clear ();
				_done_counter.fetch_and_clear ();
				_flag = 0;
				_num_consumers = 0;
			}

			inline void set_mcast_params(pami_multicast_t* mcast)
			{
				//memcpy((void*)&_coll_params.mcast, (void*)mcast, sizeof(pami_multicast_t));
				memcpy((void*)&_mcast, (void*)mcast, sizeof(pami_multicast_t));
				//_connid = mcast->connection_id;

			}

			inline void set_mcomb_params(pami_multicombine_t* mcomb)
			{
				assert(mcomb != NULL);
				//memcpy((void*)&_coll_params.mcomb, (void*)mcomb, sizeof(pami_multicombine_t));
				memcpy((void*)&_mcomb, (void*)mcomb, sizeof(pami_multicombine_t));
				//_connid = mcomb->connection_id;
			}

			inline pami_multicast_t& get_mcast_params()
			{
				//return _coll_params.mcast;
				return _mcast;
			}

			inline pami_multicombine_t& get_mcomb_params()
			{
				//return _coll_params.mcomb;
				return _mcomb;
			}

			inline	void set_type(CollType_t type)
			{
				_type = type;
			}

			inline	CollType_t get_type()
			{
				return _type;
			}

			inline void set_dispatch_id(uint16_t dispatch_id)
			{
				 _dispatch_id = dispatch_id;
			}

			inline uint16_t get_dispatch_id()
			{
				 return _dispatch_id;
			}

			inline void set_storage(void* storage)
			{
				_storage = storage;
			}

			inline void signal_done()
			{
				_done_counter.fetch_and_inc();
				TRACE_ERR((stderr, "Done counter:%zu\n",_done_counter.fetch() ));
			}


			//after using the descriptor, each consumer updates the done counter of the descriptor
			inline unsigned in_use()
			{
				if (_done_counter.fetch() == _num_consumers) { return 0;}
				return 1;
			}

			inline void signal_flag()
			{
				++_flag;
			}

			inline unsigned get_flag()
			{
				return _flag;
			}

			inline void* get_buffer(unsigned index){
				return (void*)(_buffer+index*BUFFER_SIZE_PER_TASK);
			}

			inline void* get_buffer(){
				return (void*)_buffer;
			}
			inline unsigned arrived_peers()
			{
				return _synch_counter.fetch();
			}

			inline void signal_arrived(){
				_synch_counter.fetch_and_inc();
			}


			inline void set_master(unsigned master){
				_master = master;
			}

			inline void set_state(DescState_t state){
				_my_state = state;
			}

			inline DescState_t get_state(){
				return _my_state;
			}

			inline unsigned get_master(){
				return _master;
			}

			inline unsigned get_conn_id(){
				return _connid;
			}

			inline void* get_storage()
			{
				return _storage;
			}

			inline uint64_t get_seq_id(){
				return _seq_num;
			}

			inline void set_seq_id(uint64_t seq_num){
				_seq_num = seq_num;
			}

			inline void set_consumers(unsigned consumers){
				_num_consumers = consumers;
			}
		};

		template < class T_Desc >
		class ShmemCollDescFifo{

			private:
			T_Desc _desc[DESCRIPTOR_FIFO_SIZE];
			uint64_t _head;
			uint64_t _tail;
			uint64_t _next_pending_match;
			uint64_t _fifo_end;

			public:

			inline ShmemCollDescFifo(Memory::MemoryManager *mm ):_head(0), _tail(0), _next_pending_match(0), _fifo_end(DESCRIPTOR_FIFO_SIZE)
			{
				for(unsigned i=0; i < DESCRIPTOR_FIFO_SIZE; i++){
					_desc[i].init(mm);
					_desc[i].set_seq_id((uint64_t)i);
//					new((void*)&_desc[i]) T_Desc(mm);
				}

			}

			inline ~ShmemCollDescFifo()
			{
				//printf("releasing done descriptors\n");
				while (_head < _tail){
					if ((_desc[_head%DESCRIPTOR_FIFO_SIZE].get_state() == DONE)&& !(_desc[_head%DESCRIPTOR_FIFO_SIZE].in_use()))
						_head++;
				}
				//printf("done releasing descriptors\n");
			}

			inline T_Desc* fetch_descriptor() {
				if (_tail < _fifo_end) {
					unsigned index = _tail % DESCRIPTOR_FIFO_SIZE;
					_tail++;
					TRACE_ERR((stderr,"fetch successful: head:%ld tail:%ld fifoend:%ld\n",_head, _tail,_fifo_end));
					return &_desc[index];
				}
				return NULL;
			}

			inline T_Desc* next_free_descriptor(unsigned &index) {
				if (_tail < _fifo_end) {
					index = _tail % DESCRIPTOR_FIFO_SIZE;
					TRACE_ERR((stderr,"fetch successful: head:%ld tail:%ld fifoend:%ld\n",_head, _tail,_fifo_end));
					return &_desc[index];
				}
				return NULL;
			}

			inline T_Desc* get_descriptor_by_idx(unsigned index) {
				return &_desc[index];
			}

			inline void release_done_descriptors(){
				uint64_t seq_id;

				while (_head < _tail){
					if ((_desc[_head%DESCRIPTOR_FIFO_SIZE].get_state() == DONE)&& !(_desc[_head%DESCRIPTOR_FIFO_SIZE].in_use()))
					{
						TRACE_ERR((stderr,"releasing descriptor:%d\n",(unsigned)_head%DESCRIPTOR_FIFO_SIZE));

						_desc[_head%DESCRIPTOR_FIFO_SIZE].set_state(FREE);
						_desc[_head%DESCRIPTOR_FIFO_SIZE].reset();
						seq_id = _desc[_head%DESCRIPTOR_FIFO_SIZE].get_seq_id();
						mem_sync();
						_desc[_head%DESCRIPTOR_FIFO_SIZE].set_seq_id(seq_id + DESCRIPTOR_FIFO_SIZE);
						_head++;
						_fifo_end++;

					}
					else return;
				}
			}

			inline unsigned is_empty(){
				return (_head == _tail);
			}

			inline unsigned head(){
				return _head;
			}

			inline unsigned tail(){
				return _tail;
			}

			inline void advance_next_match(){
				++_next_pending_match;
			}

			inline T_Desc* next_desc_pending_match(){
				if (_desc[_next_pending_match % DESCRIPTOR_FIFO_SIZE].get_state() == INIT){
					return &_desc[_next_pending_match % DESCRIPTOR_FIFO_SIZE];
				}
				return NULL;
			}

			inline T_Desc* match_descriptor(unsigned conn_id){
				while (_head < _tail){
					if ((_desc[_head%DESCRIPTOR_FIFO_SIZE].get_conn_id() == conn_id) &&
						(_desc[_head%DESCRIPTOR_FIFO_SIZE].get_state() == INIT))
						{
							return &_desc[_head%DESCRIPTOR_FIFO_SIZE];
						}
				}
				return NULL;
			}

			inline void incr_head(){
				_head++;
			}
		};

	}
  }
}

#endif
