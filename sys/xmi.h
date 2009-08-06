/**
 * \file xmi.h
 * \brief messaging interface
 * \todo API style - all lowercase types? mixed-case? all uppercase?
 * \todo API style - abbreviated or explicit method/type/variable names ?
 * \todo API style - use MPI convention to name function pointer typedefs
 *       and function pointer parameters. see -> https://svn.mpi-forum.org/trac/mpi-forum-web/ticket/7
 * \todo API style - define parameter list convention. Most common parameters first?
 */
#ifndef __xmi_h__
#define __xmi_h__

#include <stdlib.h>
#include <stdint.h>


typedef int   xmi_result_t;
typedef void* xmi_context_t;
typedef void* xmi_error_t;
typedef void* xmi_hint_t;
typedef void* xmi_configuration_t;
typedef void* xmi_dispatch_t;
typedef void* xmi_data_type_t;


/**
 * \brief Callback to handle message events
 *
 * \todo Move this to a common header file
 *
 * \param[in] context   XMI communication context that invoked the callback
 * \param[in] cookie    Event callback application argument
 * \param[in] result    Asynchronous result information (was error information)
 */
typedef void (*xmi_event_callback_t) ( xmi_context_t   context,
                                       void          * cookie,
                                       xmi_result_t    result );

#if 0
#include "xmi_config.h"
#include "xmi_context.h"
#include "xmi_lock.h"
#endif


#endif /* __xmi_h__ */
