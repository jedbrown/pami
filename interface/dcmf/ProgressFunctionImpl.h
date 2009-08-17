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

#ifndef __ll_cpp_progressfuncimpl_h__
#define __ll_cpp_progressfuncimpl_h__

namespace LL {

///
/// \brief Generic Progress Engine Function
///
class _ProgressFunctionImpl {
private:
	DCMF::CDI::ProgressFunctionMdl _model;
public:

	///
	/// \brief Simple default constructor
	///
	/// for a progress function that will call ->start() later.
	/// 
	_ProgressFunctionImpl() {}

	///
	/// \brief Full start constructor
	///
	/// Construct a progress function and start it (queue it).
	/// 
	_ProgressFunctionImpl(LL_ProgressFunc_t *pf, CM_Result &status) {
		status = start(pf);
	}

	///
	/// \brief Start a generic progress function
	///
	/// \param[in] pf		Function to be invoked from progress engine
	/// \return	Results of start.
	/// \note	Actual error code/status would have been delivered to completion
	///		callback. This function might return success while the actual
	///		execution of the progress function might return an error.
	///
	inline CM_Result start(LL_ProgressFunc_t *pf) {
		bool rc = _model.generateMessage(pf);
		return (rc ? CM_SUCCESS : CM_ERROR);
	}

}; /* class _ProgressFunctionImpl */

}; /* namespace LL */

#endif /* __ll_cpp_progressfuncimpl_h__ */
