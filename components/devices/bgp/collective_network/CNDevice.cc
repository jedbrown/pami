/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgp/collective_network/CNDevice.cc
 * \brief Collective Network Device implementation
 */
#include "config.h"
#include "sys/xmi.h"
#include "util/common.h"
#include "SysDep.h"
#include "components/atomic/bgp/LockBoxBarrier.h"
#include "components/devices/bgp/collective_network/CNDevice.h"

size_t _g_num_active_nodes;
size_t _g_min_peers;
size_t _g_max_peers;

// exported to other modules...

/**
 * \brief Bitmap to enable helper thread(s), 1 = send, 2 = recv
 */
int XMI_THREADED_CN = 3;

/**
 * \brief Actual persistence for advance (cycles)
 */
unsigned XMI_PERSISTENT_ADVANCE = 1000;

/**
 * \brief Maximum persistence for advance, when computed
 */
unsigned XMI_PERSIST_MAX = 5000;

/**
 * \brief Minimum persistence for advance, when computed
 */
unsigned XMI_PERSIST_MIN = 1000;

/**
 * \brief Number of bytes at which to start using 2-Pass
 */
unsigned XMI_CN_DBLSUM_THRESH = 2 * sizeof(double);

/**
 * \brief Number of bytes at which to start using helper thread(s)
 */
unsigned XMI_CN_HELPER_THRESH = 16384; // a guess

/**
 * \brief Boolean, whether to use Deep Advance for VN mode
 */
int XMI_CN_VN_DEEP = 1;

namespace XMI {
namespace Device {
namespace BGP {

	typedef XMI::Barrier::BGP::LockBoxNodeProcBarrier<XMI::SysDep> CNDeviceInitBarrier;

	void CNDevice::init(XMI::SysDep &sd, XMI::Device::Generic::Device *device) {
		__init(sd, device);
		char *s;
		/*
		 * Measure the collective network tramsit time,
		 * i.e. time between injecting a packet and
		 * receiving that packet's results. To do this, we
		 * perform a single-packet dummy tree operation. We
		 * have to do a global barrier beforehand in order
		 * to ensure accurate timings.
		 *
		 * Each node in a block may see a different time,
		 * because each node experiences a different number
		 * of hops for the round-trip of the packet, so we
		 * will have to do another collective operation
		 * afterwards in order to compute the global
		 * maximum value.
		 *
		 * We must ensure that only one core per chip
		 * accesses the tree.  This will always be core
		 * 0. If more than one core is active per chip,
		 * then we'll have to do a (lightweight) broadcast
		 * to share the results of the measurements.
		 */
		unsigned *loc = NULL;
		CNDeviceInitBarrier lbb;
		size_t peers;
		__global.mapping.nodePeers(peers);
		/*
		 * This is used by TreeMessage objects pre/post
		 * math routines to account for changes to
		 * bitfields in the data.
		 */
		_g_num_active_nodes = __global.mapping.size();
		if (peers > 1) {
			lbb.init(&sd, peers);
			sd.mm.memalign((void **)&loc, sizeof(loc), BGPCN_PKT_SIZE);
		}
		if (sd.lockboxFactory.isMasterRank()) {
			static char pkt[BGPCN_PKT_SIZE]__attribute__((__aligned__(16)));
			_BGP_TreeHwHdr hdr;
			register unsigned hc, dc, ih, id;
			unsigned t0;
			// Assume nothing else is going on on the tree...
			hdr.CtvHdr.Class = 3;
			hdr.CtvHdr.Ptp = 0;
			hdr.CtvHdr.Irq = 0;
			hdr.CtvHdr.OpCode = _BGP_TREE_OPCODE_MAX;
			hdr.CtvHdr.OpSize = _BGP_TREE_OPSIZE_BIT32;
			hdr.CtvHdr.Tag = 0;
			hdr.CtvHdr.CsumMode = _BGP_TREE_CSUM_NONE;
			GlobInt_Barrier(0, NULL, NULL);
			// send "_peers" packet, get MIN (using MAX func), and also measure time
			((unsigned *)pkt)[0] = ~ peers;
			((unsigned *)pkt)[1] = peers;
			t0 = __tsc();
			CollectiveRawSendPacket(VIRTUAL_CHANNEL, &hdr, pkt);
			do {
				CollectiveFifoStatus(VIRTUAL_CHANNEL, &hc, &dc, &ih, &id);
			} while (hc == 0 && dc == 0);
			CollectiveRawReceivePacket(VIRTUAL_CHANNEL, &hdr, pkt);
			__cn_times[0] = __tsc() - t0;
			_g_min_peers = ~ ((unsigned *)pkt)[0];
			_g_max_peers = ((unsigned *)pkt)[1];
			// perform a global max operation to get the longest time
			*((unsigned *)pkt) = __cn_times[0];
			CollectiveRawSendPacket(VIRTUAL_CHANNEL, &hdr, pkt);
			do {
				CollectiveFifoStatus(VIRTUAL_CHANNEL, &hc, &dc, &ih, &id);
			} while (hc == 0 && dc == 0);
			CollectiveRawReceivePacket(VIRTUAL_CHANNEL, &hdr, pkt);
			// retrieve the MAX xmit time for all nodes
			__cn_times[1] = *((unsigned *)pkt);
			if (peers > 1) {
				// "broadcast" results to other cores
				loc[0] = __cn_times[0];
				loc[1] = __cn_times[1];
				loc[2] = _g_min_peers;
				loc[3] = _g_max_peers;
			}
		}
		if (peers > 1) {
			// complete the "broadcast" by barriering and
			// picking up the results from shared memory.
			lbb.enter();
			if (!sd.lockboxFactory.isMasterRank()) {
				__cn_times[0] = loc[0];
				__cn_times[1] = loc[1];
				_g_min_peers = loc[2];
				_g_max_peers = loc[3];
			}
		}
		//fprintf(stderr, "Tree transmit time %d (%d)\n", __cn_times[1], __cn_times[0]);
		s = getenv("XMI_CN_VN_DEEP");
		if (s) XMI_CN_VN_DEEP = atoi(s);
		s = getenv("XMI_CN_DBLSUM_THRESH");
		if (s) {
			XMI_CN_DBLSUM_THRESH = atoi(s);
			if (XMI_CN_DBLSUM_THRESH < (unsigned)-1) {
				XMI_CN_DBLSUM_THRESH *= sizeof(double);
			}
		} else {
			/// \todo compute XMI_CN_DBLSUM_THRESH
		}
		s = getenv("XMI_CN_HELPER_THRESH");
		if (s) {
			XMI_CN_HELPER_THRESH = atoi(s);
		} else {
			/// \todo compute XMI_CN_HELPER_THRESH
		}
		s = getenv("XMI_THREADED_CN");
		if (s) XMI_THREADED_CN = atoi(s);
		s = getenv("XMI_PERSIST_MAX");
		if (s) XMI_PERSIST_MAX = atoi(s);
		s = getenv("XMI_PERSIST_MIN");
		if (s) XMI_PERSIST_MIN = atoi(s);
		s = getenv("XMI_PERSISTENT_ADVANCE");
		if (s) {
			XMI_PERSISTENT_ADVANCE = atoi(s);
		} else {
			unsigned i = 0;
			// use 1.5x for persistent advance timeout
			i = __cn_times[1] +
					(__cn_times[1] >> 1);
			if (i > 2 * XMI_PERSIST_MAX) {
				// Not worth even trying
				i = 0;
			}
			if (i < XMI_PERSIST_MIN) {
				i = XMI_PERSIST_MIN;
			}
			if (i > XMI_PERSIST_MAX) {
				i = XMI_PERSIST_MAX;
			}
			XMI_PERSISTENT_ADVANCE = i;
		}
		// this will need to be more generic if number of
		// cores becomes larger.
		switch (__global.mapping.tSize()) {
		case 1: // SMP mode
			if (XMI_THREADED_CN) {
				_threadRoles = 2; // two roles may use comm_threads
			}
			break;
		case 2: // DUAL mode
			if (XMI_THREADED_CN) {
				_threadRoles = 1; // only one comm_thread
			}
			break;
		case 4: // VN mode
			// Can't use comm threads, but can divide
			// work between virt nodes calling advance()...
			// But that requires different nodes using
			// different roles...
			break;
		default:
			XMI_abort();
		}
	}

}; // namespace BGP
}; // namespace Device
}; // namespace XMI
