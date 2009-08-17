/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file PipeWorkQueue.h
 * \brief ???
 */

#ifndef __ll_cpp_progressfunc_h__
#define __ll_cpp_progressfunc_h__

#include "ProgressFunctionImpl.h"

namespace LL {

///
/// \brief Generic Progress Engine Function
///
class ProgressFunction : public _ProgressFunctionImpl {
private:
public:

	///
	/// \brief Simple default constructor
	///
	/// Construct a progress function that will call ->start() later.
	/// 
	ProgressFunction() : _ProgressFunctionImpl() {}

	///
	/// \brief Full start constructor
	///
	/// Construct a progress function and start it (queue it).
	///
	/// \param[in] pf	Progress Function parameters
	/// \param[out] status	Results of operation (CM_SUCCESS, ...)
	///
	ProgressFunction(LL_ProgressFunc_t *pf, CM_Result &status) :
		_ProgressFunctionImpl(pf, status) {}

	///
	/// \brief Start a generic progress function
	///
	/// \param[in] pf	Progress Function parameters
	/// \return	Results of operation (CM_SUCCESS, ...)
	///
	inline CM_Result start(LL_ProgressFunc_t *pf);

}; /* class ProgressFunction */

}; /* namespace LL */

// This is ugly - but it never has to change...
inline CM_Result LL::PipeWorkQueue::start(LL_ProgressFunc_t *pf) {
	return _PipeWorkQueueImpl::start(pf);
}

#endif /* __ll_cpp_progressfunc_h__ */
