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
#define TRACE_ERR(x) //fprintf(stderr,"%s:%d\n",__FILE__,__LINE__); fprintf x
#endif


namespace PAMI
{
  namespace Device
  {
    namespace Shmem
    {

#define DESCRIPTOR_FIFO_SIZE    4 
//#define DESCRIPTOR_FIFO_SIZE   64
//#define DESCRIPTOR_FIFO_SIZE  128
//#define	BUFFER_SIZE_PER_TASK	256
#define	BUFFER_SIZE_PER_TASK  1024	
#define NUM_LOCAL_TASKS		16


      typedef enum
      {
        MULTICAST = 0,
        MULTICOMBINE,
        MULTISYNC
      } CollType_t;

      typedef enum
      {
        FREE = 0,
        INIT,
        ACTIVE,
        DONE
      } DescState_t;

      struct  ShmemRegion //only master should initialize these fields
      {
        volatile  char	      buffer[NUM_LOCAL_TASKS*BUFFER_SIZE_PER_TASK];// __attribute__((__aligned__(128)));
        volatile  uint64_t      seq_num;                                     // __attribute__((__aligned__(8)));
        volatile  unsigned      num_consumers;
        volatile  unsigned      flag;
        volatile  unsigned      master_done;
        volatile  DescState_t   state;	/* Any of FREE, INIT, ACTIVE, DONE */

        public:

        inline ShmemRegion(uint64_t seq_id)
        { 
          seq_num = seq_id;
          //num_consumers = 0;
          num_consumers = __global.topology_local.size();
          flag = 0;
          master_done = 0;
          //state = FREE;
        }
        inline ~ShmemRegion()
        {}
      }__attribute__((__aligned__(64))); 

     static ShmemRegion*  _shmem_region = NULL;

      template < class T_Atomic >
        class ShmemCollDesc
        {

          public:

            struct  atomics_region
            {
              T_Atomic		synch_counter;	/* Whether everyone has arrived */
              T_Atomic		done_counter; 	/* Whether everyone finished 	*/
              //T_Atomic		_seq_num;
            };



          private:

            atomics_region        _atomics;
            ShmemRegion         *_shared;
            uint16_t    	_dispatch_id; 	/* Invoke the dispatch after match */
            unsigned 		_master;	/* Master of the collective */
            CollType_t		_type;
            void*		_storage;
            int			_connid;	/* Initialize with -1		*/
            uint64_t            _my_seq_num;    // __attribute__((__aligned__(8)));
            DescState_t		_my_state;	/* Any of FREE, INIT, ACTIVE, DONE */
            union
            {
              pami_multicast_t	mcast;
              pami_multicombine_t	mcomb;
            }			_coll_params;
            /* pami_multicast_t	_mcast;
               pami_multicombine_t	_mcomb;*/

          public:


            inline ShmemCollDesc() {}

            inline ShmemCollDesc(Memory::MemoryManager &mm, char * unique_device_string, size_t usageid, size_t index): _shared(NULL),_master(0), _storage(NULL), _my_seq_num(0),  _my_state(FREE)
          {
            char key[PAMI::Memory::MMKEYSIZE];
            sprintf(key, "/ShmemCollDesc-synch-%s-%zd-%zd", unique_device_string, usageid, index);
            _atomics.synch_counter.init(&mm, key);
            sprintf(key, "/ShmemCollDesc-done-%s-%zd-%zd", unique_device_string, usageid, index);
            _atomics.done_counter.init(&mm, key);

            _shared = _shmem_region + index;
            _my_seq_num = (uint64_t)index;

          };
            inline ~ShmemCollDesc() {}

            inline void reset()
            {
              _atomics.synch_counter.fetch_and_clear ();
              _atomics.done_counter.fetch_and_clear ();
              _shared->flag = 0;
              _shared->num_consumers = __global.topology_local.size();;
            }

            inline void set_mcast_params(pami_multicast_t* mcast)
            {
              memcpy((void*)&_coll_params.mcast, (void*)mcast, sizeof(pami_multicast_t));
              //memcpy((void*)&_mcast, (void*)mcast, sizeof(pami_multicast_t));
              //_connid = mcast->connection_id;

            }

            inline void set_mcomb_params(pami_multicombine_t* mcomb)
            {
              //assert(mcomb != NULL);
              memcpy((void*)&_coll_params.mcomb, (void*)mcomb, sizeof(pami_multicombine_t));
              //memcpy((void*)&_mcomb, (void*)mcomb, sizeof(pami_multicombine_t));
              //_connid = mcomb->connection_id;
            }

            inline pami_multicast_t& get_mcast_params()
            {
              return _coll_params.mcast;
              //return _mcast;
            }

            inline pami_multicombine_t& get_mcomb_params()
            {
              return _coll_params.mcomb;
              //return _mcomb;
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
              _atomics.done_counter.fetch_and_inc();
              TRACE_ERR((stderr, "Done counter:%zu\n", _atomics.done_counter.fetch() ));
            }


            //after using the descriptor, each consumer updates the done counter of the descriptor
            inline unsigned in_use()
            {
              if (_atomics.done_counter.fetch() == _shared->num_consumers) { return 0;}
              return 1;
            }

            inline void signal_flag()
            {
              ++_shared->flag;
            }

            inline unsigned get_flag()
            {
              return _shared->flag;
            }

            inline void reset_master_done()
            {
              _shared->master_done = 0;
            }

            inline void signal_master_done()
            {
              _shared->master_done = 1;
            }

            inline unsigned get_master_done()
            {
              return _shared->master_done;
            }


            inline void* get_buffer(unsigned index)
            {
              return (void*)(_shared->buffer + index*BUFFER_SIZE_PER_TASK);
            }

            inline void* get_buffer()
            {
              return (void*)_shared->buffer;
            }
            inline unsigned arrived_peers()
            {
              return _atomics.synch_counter.fetch();
            }
            inline unsigned done_peers()
            {
              return _atomics.done_counter.fetch();
            }

            inline void signal_arrived()
            {
              _atomics.synch_counter.fetch_and_inc();
              TRACE_ERR((stderr, "arrival counter:%zu\n", _atomics.synch_counter.fetch() ));
            }


            inline void set_master(unsigned master)
            {
              _master = master;
            }

/*
            inline void set_state(DescState_t state)
            {
              _shared.state = state;
            }

            inline DescState_t get_state()
            {
              return _shared.state;
            }
*/
            inline void set_my_state(DescState_t state)
            {
              _my_state = state;
            }

            inline DescState_t get_my_state()
            {
              return _my_state;
            }

            inline unsigned get_master()
            {
              return _master;
            }

            inline unsigned get_conn_id()
            {
              return _connid;
            }

            inline void* get_storage()
            {
              return _storage;
            }

            inline uint64_t get_seq_id()
            {
              return _shared->seq_num;
            }

            inline void set_seq_id(uint64_t seq_num)
            {
              _shared->seq_num = seq_num;
            }
            inline uint64_t get_my_seq_id()
            {
              return _my_seq_num;
            }

            inline void set_my_seq_id(uint64_t seq_num)
            {
              _my_seq_num = seq_num;
            }

            inline void set_consumers(unsigned consumers)
            {
              _shared->num_consumers = consumers;
              TRACE_ERR((stderr, "num_consumers:%u\n", consumers ));
            }

            inline void set_shmem_region(ShmemRegion* shmemreg)
            {
              _shared = shmemreg;
            }
            
        };

      template < class T_Atomic >
        class ShmemCollDescFifo
        {

          private:
            ShmemCollDesc<T_Atomic>      _desc[DESCRIPTOR_FIFO_SIZE];
            uint64_t    _head;
            uint64_t    _tail;
            uint64_t    _next_pending_match;
            uint64_t    _fifo_end;
            size_t      _local_rank;

          public:

            inline ShmemCollDescFifo(): _head(0), _tail(0), _next_pending_match(0), _fifo_end(DESCRIPTOR_FIFO_SIZE), _local_rank(__global.topology_local.rank2Index(__global.mapping.task()))
          {
            TRACE_ERR((stderr, "ShmemCollDescFifo constructor called\n"));
          }


            void init (Memory::MemoryManager &mm, char * unique_device_string)
            {
            char key[PAMI::Memory::MMKEYSIZE];
            sprintf(key, "/ShmemCollDescFifo-%s", unique_device_string);


            size_t total_size = sizeof(ShmemRegion)*DESCRIPTOR_FIFO_SIZE ; 
            pami_result_t rc;
            rc = mm.memalign ((void **) & _shmem_region,
                64,
                total_size,
                key,
                ShmemCollDescFifo::shmem_region_initialize,
                NULL);
            PAMI_assertf(rc == PAMI_SUCCESS, "Failed to allocate shared memory resources for collective descriptors");

            for (size_t i = 0; i < DESCRIPTOR_FIFO_SIZE; i++)
            {
              new (&_desc[i]) ShmemCollDesc<T_Atomic>(mm, unique_device_string, 0, i );
            }
            };

            inline ~ShmemCollDescFifo()
            { }

            static void shmem_region_initialize (void       * memory,
                size_t       bytes,
                const char * key,
                unsigned     attributes,
                void       * cookie)
            {
              ShmemRegion *shm_region = (ShmemRegion*) memory;

              size_t  i;
              for (i =0; i < DESCRIPTOR_FIFO_SIZE;i++)
              {
                new (&shm_region[i]) ShmemRegion((uint64_t)i); 
              }

            }

            inline ShmemCollDesc<T_Atomic>* fetch_descriptor()
            {
              if (_tail < _fifo_end)
              {
                unsigned index = _tail % DESCRIPTOR_FIFO_SIZE;
                _tail++;
                TRACE_ERR((stderr, "fetch successful: head:%ld tail:%ld fifoend:%ld\n", _head, _tail, _fifo_end));
                return &_desc[index];
              }

              return NULL;
            }

            inline ShmemCollDesc<T_Atomic>* next_free_descriptor(unsigned &index)
            {
              if (_tail < _fifo_end)
              {
                index = _tail % DESCRIPTOR_FIFO_SIZE;
                //TRACE_ERR((stderr, "fetch successful: head:%ld tail:%ld fifoend:%ld\n", _head, _tail, _fifo_end));
                return &_desc[index];
              }

              return NULL;
            }

            inline ShmemCollDesc<T_Atomic>* get_descriptor_by_idx(unsigned index)
            {
              return &_desc[index];
            }

            //master has to be set for every descriptor
            inline void release_done_descriptors()
            {
              uint64_t seq_id;
              while (_head < _tail)
              {
                if (_desc[_head%DESCRIPTOR_FIFO_SIZE].get_my_state() == DONE)
                {

                  //only the master sets the shared state and seq number
                  if (_desc[_head%DESCRIPTOR_FIFO_SIZE].get_master() == _local_rank) 
                  {
                    if (!_desc[_head%DESCRIPTOR_FIFO_SIZE].in_use())
                    {
                    TRACE_ERR((stderr, "releasing descriptor:%d\n", (unsigned)_head % DESCRIPTOR_FIFO_SIZE));
                    _desc[_head%DESCRIPTOR_FIFO_SIZE].reset();
                    mem_sync();
                    seq_id = _desc[_head%DESCRIPTOR_FIFO_SIZE].get_seq_id();
                    _desc[_head%DESCRIPTOR_FIFO_SIZE].set_seq_id(seq_id + DESCRIPTOR_FIFO_SIZE);
                    seq_id = _desc[_head%DESCRIPTOR_FIFO_SIZE].get_my_seq_id();
                    _desc[_head%DESCRIPTOR_FIFO_SIZE].set_my_seq_id(seq_id + DESCRIPTOR_FIFO_SIZE);
                    _desc[_head%DESCRIPTOR_FIFO_SIZE].set_master(0); //default
                    _desc[_head%DESCRIPTOR_FIFO_SIZE].set_my_state(FREE);
                    _head++;
                    _fifo_end++;
                    }
                    else return;
                  }
                  else
                  {
                    TRACE_ERR((stderr, "releasing descriptor:%d\n", (unsigned)_head % DESCRIPTOR_FIFO_SIZE));
                    //set individual sequence ids and states as well
                    seq_id = _desc[_head%DESCRIPTOR_FIFO_SIZE].get_my_seq_id();
                    _desc[_head%DESCRIPTOR_FIFO_SIZE].set_my_seq_id(seq_id + DESCRIPTOR_FIFO_SIZE);
                    _desc[_head%DESCRIPTOR_FIFO_SIZE].set_master(0); //default
                    _desc[_head%DESCRIPTOR_FIFO_SIZE].set_my_state(FREE);
                    _head++;
                    _fifo_end++;
                  }
                }
                else return;
              }
            }

            inline unsigned is_empty()
            {
              return (_head == _tail);
            }

            inline unsigned head()
            {
              return _head;
            }

            inline unsigned tail()
            {
              return _tail;
            }


            //TODO..fix this
            inline void advance_next_match()
            {
              ++_next_pending_match;
            }

            //TODO..fix this
            inline ShmemCollDesc<T_Atomic>* next_desc_pending_match()
            {
              if (_desc[_next_pending_match % DESCRIPTOR_FIFO_SIZE].get_my_state() == INIT)
              {
                return &_desc[_next_pending_match % DESCRIPTOR_FIFO_SIZE];
              }

              return NULL;
            }

            //TODO..fix this
            inline ShmemCollDesc<T_Atomic>* match_descriptor(unsigned conn_id)
            {
              while (_head < _tail)
              {
                if ((_desc[_head%DESCRIPTOR_FIFO_SIZE].get_conn_id() == conn_id) &&
                    (_desc[_head%DESCRIPTOR_FIFO_SIZE].get_my_state() == INIT))
                {
                  return &_desc[_head%DESCRIPTOR_FIFO_SIZE];
                }
              }

              return NULL;
            }

            inline void incr_head()
            {
              _head++;
            }
        };

    }
  }
}

#endif
