/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/tree/VnDualShortTreeAllreduce.cc
 * \brief ???
 */
#ifndef __bgp__
#warning DCMF specific protocol - needs work to make it generic
#endif
#include "util/ccmi_util.h"
#include "algorithms/ccmi.h"
#include "util/ccmi_debug.h"

#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include "math/math_coremath.h"
#include "VnDualShortTreeAllreduce.h"
#include "msgr.h"

extern DCMF::Messager *_g_messager; /// \todo dcmf specific

extern XMI::Topology *_g_topology_local;

#define SHM_FILE "/unique-ccmi-shmem"

extern int dcmf_dt_shift[XMI_DT_COUNT];

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {
      namespace Tree
      {
	/// \todo Why is all this 'static' instead of instantiated in the registration?!
	// Is it all *really* protected (mutual exclusion) from different instances?
	CCMI::MultiSend::CCMI_Multicombine_t VnDualShortTreeAllreduce::_mcombArgs;
        VnDualShortTreeAllreduce::SharedData *VnDualShortTreeAllreduce::_shared =
					(VnDualShortTreeAllreduce::SharedData *)-1;
	char *VnDualShortTreeAllreduce::_swq_buf;
	char *VnDualShortTreeAllreduce::_rwq_buf;
	XMI::PipeWorkQueue VnDualShortTreeAllreduce::_swq;
	XMI::PipeWorkQueue VnDualShortTreeAllreduce::_rwq;
	XMI::Topology VnDualShortTreeAllreduce::_root;
        unsigned VnDualShortTreeAllreduce::_numPeers = 0;
        unsigned VnDualShortTreeAllreduce::_myPeer = 0;
        unsigned VnDualShortTreeAllreduce::_isMasterCore = 0;

        void VnDualShortTreeAllreduce::init(CCMI::TorusCollectiveMapping * mapping)
        {
          //CCMI_assert(_shared == NULL);
          if (_shared != ((SharedData *)-1)) return; // this should run only once
          int fd = -1, rc = -1;

          // allocate shared memory region
          fd = shm_open(SHM_FILE, O_CREAT | O_RDWR, 0600);
          CCMI_assert(fd != -1);

          rc = ftruncate(fd, sizeof(SharedData));
          CCMI_assert(rc != -1);

          void * loc = NULL;
          loc = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
          CCMI_assert(loc != MAP_FAILED);

          _shared = (SharedData *)loc;

#if 0
#warning "Remove this hack when shm_open() limit is fixed"
          _shared =  (SharedData *)_g_sysdep->memoryManager().scratchpad_dynamic_area_malloc(sizeof(SharedData));
#endif

	_swq.configure(_g_messager->sysdep(), MaxDataBytes);
	_swq.reset();
	_swq_buf = _swq.bufferToConsume();
	_rwq.configure(_g_messager->sysdep(), MaxDataBytes);
	_rwq.reset();
	_rwq_buf = _rwq.bufferToConsume();

	_mcombArgs.setData((XMI_PipeWorkQueue_t *)&_swq, 0);
	_mcombArgs.setResults((XMI_PipeWorkQueue_t *)&_rwq, 0);
	_mcombArgs.setDataRanks(NULL);

          // find out who is the master core (to access the tree)
          unsigned i;

	  _numPeers = _g_topology_local->size();
	  size_t me = mapping->rank();
	  _myPeer = _g_topology_local->rank2Index(me);
	  _isMasterCore = (_myPeer == 0);

          for (i = 1; i < MAX_NUM_CORES; ++i) {
		// everybody does the same, so trampling eachother isn't a problem
		// this wasy we don't have to coordinate a "join" afterwards.
		_shared->srcs[i] = _shared->client[i - 1].src;
		if (i >= _numPeers && _isMasterCore) {
			memset(_shared->client[i - 1].src, 0, MaxDataBytes);
		}
	  }

          TRACE_ADAPTOR((stderr,"<        >Allreduce::Tree::VnDualShortTreeAllreduce::init "
                         "minTCoord:%d numPeers:%d t:%d srcs[0]:%#.8X srcs[1]:%#.8X "
                         "srcs[2]:%#.8X srcs[3]:%#.8X\n",
                         _minTCoord,
                         _numPeers,
                         mapping->GetCoord(CCMI_T_DIM),
                         (int)_shared->srcs[0],
                         (int)_shared->srcs[1],
                         (int)_shared->srcs[2],
                         (int)_shared->srcs[3]));

        }

          unsigned VnDualShortTreeAllreduce::restart(XMI_CollectiveRequest_t  *request,
                                       XMI_Callback_t           & cb_done,
                                       CCMI_Consistency            consistency,
                                       char                      * srcbuf,
                                       char                      * dstbuf,
                                       size_t                      count,
                                       XMI_Dt                     dtype,
                                       XMI_Op                     op,
                                       size_t                      root)
          {
            TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::Tree::VnDualShortTreeAllreduce::restart() "
                           "srcbuf[0]:%d srcbuf:%#.8X\n", (int)this,
                           ((int*)srcbuf)[0],(int)srcbuf));

            _cb_done.function = cb_done.function;
            _cb_done.clientdata = cb_done.clientdata;
            if ((root == (size_t)-1) || (root == _mapping->rank())) { // allreduce or reduce root wants results
              _dstbuf = dstbuf;
            } else { // we don't want the results if we're non-root reduce.
              _dstbuf = NULL;
	    }
            if (_op != op || _dt != dtype)
            {
              _op = op;

              _mcombArgs.setReduceInfo(op, dtype);

              if (_op == XMI_SUM  &&  dtype == XMI_SIGNED_INT)
              {
                _reduceOpSelect = INT_SUM;
              }
              else if (_op == XMI_SUM  && dtype == XMI_DOUBLE)
              {
                _reduceOpSelect = DBL_SUM;
              }
              else
              {
                _reduceOpSelect = OPT_FUN;
                _reduceFunc = (coremath)MATH_OP_FUNCS( (XMI_Dt)dtype,
                                                              (XMI_Op)op,
                                                              _numPeers );
              }

              _sizeOfType = dcmf_dt_shift[dtype];
              _dt         = dtype;
              _count      = 0;
              _bytes      = 0;
            }

            if (_count != count)
            {
              _bytes  = count << _sizeOfType;
              _count = count;

              if (_bytes > MaxDataBytes)// || (root != -1)) // short allreduce only
              {
                TRACE_ALERT((stderr, "<%#.8X>Allreduce::VnDualShortTreeAllreduce::Composite::restart() ALERT: "
                             "XMI_INVAL op %#X, type %#X, count %#X!\n", (int)this, op, dtype, count));
                TRACE_ADAPTOR((stderr, "<%#.8X>Allreduce::VnDualShortTreeAllreduce::Composite::restart():"
                               "XMI_INVAL op %#X, type %#X, count %#X!\n",(int)this, op, dtype, count));
                return XMI_INVAL;
              }

              _mcombArgs.setData((XMI_PipeWorkQueue_t *)&_swq, _count);
              _mcombArgs.setResults((XMI_PipeWorkQueue_t *)&_rwq, _count);
            }

            if (!_isMasterCore)
            {
              // (busy) wait for the shared src buffer being available
              while ((volatile unsigned)_shared->client[_myPeer - 1].isSrcReady == 1);

              TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::Tree::VnDualShortTreeAllreduce slave1\n", (int)this));

              // copy local contribution into the shared area
              memcpy(_shared->client[_myPeer - 1].src, srcbuf, _bytes);
              asm volatile ("mbar" ::: "memory");
              _shared->client[_myPeer - 1].isSrcReady = 1;

              TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::Tree::VnDualShortTreeAllreduce "
                             "slave2 srcbuf[0]:%d shared.src[0]:%d tcoord:%d\n", (int)this,
                             ((int*)srcbuf)[0],((int*)_shared->client[tCoord - 1].src)[0],tCoord ));

              // (busy) wait for the result
              while ((volatile unsigned)_shared->client[_myPeer - 1].isDstReady == 0);

              TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::Tree::VnDualShortTreeAllreduce slave3\n", (int)this));

              // reset result ready flag for the next run
              _shared->client[_myPeer - 1].isDstReady = 0;
              // local completion
              done();
            }
            else // master core doing local reduction and writing/reading the tree
            {
              // register master's srcbuf for the reduction
              _shared->srcs[0] = srcbuf;
	      _rwq.reset();
	      _swq.reset();


              // wait for peers to copy  src data into the shared buffer
              volatile unsigned num;
              do
              {
                num =
                (volatile unsigned)_shared->client[0].isSrcReady +
                (volatile unsigned)_shared->client[1].isSrcReady +
                (volatile unsigned)_shared->client[2].isSrcReady;
              } while (num < _numPeers - 1);

              if (_dstbuf) {
	        new (&_root) XMI::Topology(_mapping->rank());
              } else { // we don't want the results if we're non-root reduce.
	        new (&_root) XMI::Topology();
	      }
              _mcombArgs.setResultsRanks((XMI_Topology_t *)&_root);

              TRACE_ADAPTOR((stderr,"<%#.8X>Allreduce::Tree::VnDualShortTreeAllreduce "
                             "master1 dst[0]:%d srcs[0,0]:%d srcs[1,0]:%d "
                             "srcs[2,0]:%d num:%d count:%d bytes:%d\n", (int)this,
                             ((int*)_shared->dst)[0],
                             ((int*)_shared->client[0].src)[0],
                             ((int*)_shared->client[1].src)[0],
                             ((int*)_shared->client[2].src)[0],
                             _numPeers,
                             _count,
                             _bytes));

              // do local reduction
              if (_reduceOpSelect == INT_SUM)
              {
                for (unsigned i = 0; i < _count; i++)
                {
                  ((int *)(_swq_buf))[i] =
                  	((int *)(_shared->srcs[0]))[i] +
                  	((int *)(_shared->srcs[1]))[i] +
                  	((int *)(_shared->srcs[2]))[i] +
                  	((int *)(_shared->srcs[3]))[i];
                }
              }
              else if (_reduceOpSelect == DBL_SUM)
              {
                for (unsigned i = 0; i < _count; i++)
                {
                  ((double *)(_swq_buf))[i] =
                  	((double *)(_shared->srcs[0]))[i] +
                  	((double *)(_shared->srcs[1]))[i] +
                  	((double *)(_shared->srcs[2]))[i] +
                  	((double *)(_shared->srcs[3]))[i];
                }
              }
              else
              {
                if (_numPeers > 1)
                  _reduceFunc(_swq_buf, _shared->srcs, _numPeers, _count);
                else
                  memcpy(_swq_buf, _shared->srcs[0], _bytes);
              }
	      _swq.produceBytes(_bytes);

              // reset shared src buffer ready flags for the next run
              _shared->client[0].isSrcReady = 0;
              _shared->client[1].isSrcReady = 0;
              _shared->client[2].isSrcReady = 0;

              // call tree multisend directly
              _mcombArgs.setRequestBuffer((XMI_Request_t *)request, sizeof(XMI_Request_t));
              _mcomb->generate(&_mcombArgs);
            }

            return XMI_SUCCESS;
          }

      } /* Tree */
    } /* Allreduce */
  } /* Adaptor */
} /* CCMI */
