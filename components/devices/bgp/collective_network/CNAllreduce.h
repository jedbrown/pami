/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/**
 * \file components/devices/bgp/collective_network/CNAllreduce.h
 * \brief Default collective network allreduce interface.
 */
#ifndef __components_devices_bgp_cnallreduce_h__
#define __components_devices_bgp_cnallreduce_h__

#include "Util.h"
#include "components/devices/MulticombineModel.h"
#include "components/devices/bgp/collective_network/Packet.h"
#include "workqueue/WorkQueue.h"
#include "xmi_bg_math.h"
#include "FloatUtil.h"

extern int xmi_dt_shift[XMI_DT_COUNT];

namespace XMI {
namespace Device {
namespace BGP {

typedef coremath1 preprocess;
typedef coremath vnmprocess;
typedef coremath1 postprocess;

extern void postprocSum1PDouble(void *out, void *in, int c);
extern void preprocSum1PDouble(void *out, void *in, int c);
extern void preprocSum1PFloat(void *out, void *in, int c);
extern void postprocSum1PFloat(void *out, void *in, int c);

static inline int allreduce_setup(XMI_Dt            dt,
			XMI_Op            op,
			preprocess        *pre,
			postprocess       *post,
			vnmprocess        *vnm,
			unsigned          *hhfunc,
			unsigned          *opshift);

/**
 * \brief Object to hold info about a Collective Network Allreduce
 *
 * Creates an object that contains all pertinent information
 * about an Allreduce on the Collective Network, based on datatype
 * and operator.
 */
class CNAllreduceSetup {
	static CNAllreduceSetup CNAllreduceSetupCache[XMI_OP_COUNT][XMI_DT_COUNT];
public:
       
        CNAllreduceSetup() {}
        CNAllreduceSetup(XMI_Dt dt, XMI_Op op) 
		{

		int rc = allreduce_setup(dt, op, &_pre, &_post,
					NULL, &_hhfunc, &_logopsize);
		if (rc != 0)
			_hhfunc = XMI::Device::BGP::COMBINE_OP_NONE;

		_dbl_sum = (dt == XMI_DOUBLE && op == XMI_SUM);
		//_flt_sum = (dt == XMI_FLOAT && op == XMI_SUM);
		if (_dbl_sum) {
			_logopsize = 8; // 2^8 == 256 == BGPCN_PKT_SIZE
		//} else if (_flt_sum) {
			// TBD...
		}
		_logbytemult = _logopsize - dcmf_dt_shift[dt];
		_opsize = (1 << _logopsize);
	}
	static void initCNAS();
	static CNAllreduceSetup &getCNAS(XMI_Dt dt, XMI_Op op) {
		return CNAllreduceSetupCache[op][dt];
	}
	preprocess         _pre;
	postprocess        _post;
	//vnmprocess         _vnm;
	unsigned           _hhfunc;
	unsigned           _opsize;
	unsigned           _logopsize;
	unsigned           _logbytemult;
	bool		   _dbl_sum;
	//bool		   _flt_sum;
};	// class CNAllreduceSetup

	/**
	 * \brief Function to set up function pointers for collective processing
	 *
	 * Based on datatype and operator, chooses pre- and post-processing routines
	 * (when Collective Network hardware is used), virtual node mode routine (to
	 * implement operator on datatype), hardware header operator code (for
	 * Collective Network hardware), and size of operands in datatype.
	 *
	 * Note, the hardware header operator code may not be equivalent to 'op'
	 * as pre- and post-processing may dictate.
	 *
	 * \param[in]	dt	Datatype of operands
	 * \param[in]	op	Operator
	 * \param[out]	pre	Pre-processing routine required for CN
	 * \param[out]	post	Post-processing routine required for CN
	 * \param[out]	vnm	Implementation of operator on datatype, for VNM
	 * \param[out]	hhfunc	Hardware code for operator used on CN
	 * \param[out]	opsize	Size of operands in datatype
	 * \return	Zero on success, -1 if dt/op pair is not supported.
	 */
	static inline int allreduce_setup(XMI_Dt	dt,
					XMI_Op		op,
					preprocess	*pre,
					postprocess	*post,
					vnmprocess	*vnm,
					unsigned	*hhfunc,
					unsigned	*opshift) {
		//*vnm =NULL;
		*pre =NULL;
		*post=NULL;
		*opshift = xmi_dt_shift[dt];
		switch(op) {
		case XMI_SUM:
			*hhfunc =  XMI::Device::BGP::COMBINE_OP_ADD;
			break;
		case XMI_MAX:
		case XMI_MAXLOC:
			*hhfunc =  XMI::Device::BGP::COMBINE_OP_MAX;
			break;
		case XMI_MIN:
		case XMI_MINLOC:
			*hhfunc =  XMI::Device::BGP::COMBINE_OP_MAX;
			break;
		case XMI_BAND:
		case XMI_LAND:
			*hhfunc =  XMI::Device::BGP::COMBINE_OP_AND;
			break;
		case XMI_BOR:
		case XMI_LOR:
			*hhfunc =  XMI::Device::BGP::COMBINE_OP_OR;
			break;
		case XMI_BXOR:
		case XMI_LXOR:
			*hhfunc =  XMI::Device::BGP::COMBINE_OP_XOR;
			break;
		default:
			return -1;
		}
		*pre = BGPCN_PRE_OP_FUNCS(dt,op,1);
		if (*pre == (void *)XMI_UNIMPL) {
			return -1;
		}
		*post = BGPCN_POST_OP_FUNCS(dt,op,1);
		//*vnm = BGPCN_MARSHALL_OP_FUNCS(dt,op,0);
		return 0;
	}

};      // namespace BGP
};      // namespace Device
};	// XMI

#endif // __components_devices_bgp_cnallreduce_h__
