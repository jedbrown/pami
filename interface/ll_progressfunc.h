/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file ll_progressfunckqueue.h
 * \brief Generic Progress Function interface.
 */
#ifndef __ll_progressfunc_h__
#define __ll_progressfunc_h__

#include "cm_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

	///
	/// \brief Signature of generic progress function
	///
	/// \param[in] clientdata	Opaque data
	/// \return	0 if done (CM_SUCCESS),
	///		-CM_Result on error,
	///		1 (any + value) if more progress is needed.
	///
	typedef int LL_ProgressFunc(void *clientdata);

	typedef struct {
		void		*request;	///< Opaque memory for object
		size_t		req_size;	///< length of Opaque memory
		LL_ProgressFunc	*func;		///< Function to invoke for progress
		void		*clientdata;	///< Opaque data to pass to func
		CM_Callback_t	cb_done;	///< Completion callback
	} LL_ProgressFunc_t;

	///
	/// \brief Start calling a function from the progress engine
	///
	/// Arrange for 'func(clientdata)' to be called from the progress engine,
	/// and invoke 'cb_done' when 'func' returns <= 0.
	///
	/// Depending on factors such as platform type and run mode, the function
	/// may be invoked on a separate thread and may make progress independently
	/// of the calling thread. However, the calling thread cannot assume that.
	///
	/// If 'func' returns a value less than or equal to zero, the 'cb_done' will
	/// be invoked and 'func' will be removed from the progress engine queue.
	///
	/// If 'func' returns a negative value, the 'cb_done' will be invoked with
	/// an error parameter including the absolute value of the return value
	/// as the CM_Result code.
	///
	/// If 'clientdata' is a pointer, the caller must ensure that the pointer's
	/// contents are preserved until 'cb_done' is invoked.
	///
	/// \param[in] pf	Function to be called in progress loop
	/// \return	CM_SUCCESS or error code
	///
	CM_Result LL_ProgressFunc_start(LL_ProgressFunc_t *pf);

#ifdef __cplusplus
};
#endif

#endif /* __ll_progressfunc_h__ */
