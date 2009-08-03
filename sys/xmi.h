/**
 * \file xmi.h
 * \brief messaging interface
 */
#ifndef __xmi_h__
#define __xmi_h__

#include <stdlib.h>
#include <stdint.h>


typedef int           xmi_result_t;
typedef void *        xmi_context_t;
typedef unsigned int  xmi_task_t;
typedef size_t        xmi_size_t;
typedef void *        xmi_error_t;
typedef void *        xmi_hint_t;
typedef void *        xmi_configuration_t;
typedef void *        xmi_dispatch_t;
typedef void *        xmi_data_type_t;


/**
 * \brief Callback to handle message events
 *
 * \todo Move this to a common header file
 *
 * \param[in] context   XMI application context
 * \param[in] cookie    Event callback application argument
 * \param[in] result    Asynchronous result information (was error information)
 */
typedef void (*xmi_event_callback_t) ( xmi_context_t   context,
                                       void          * cookie,
                                       xmi_result_t    result );

/**
 * \todo Define common return type.
 *
 * \todo Determine if need support for a separate "configure" function, or if
 *       configuration is only done during xmi context initialization
 *
 * \todo API style - all lowercase types? mixed-case? all uppercase?
 * \todo API style - abbreviated or explicit method/type/variable names ?
 * \todo API style - use MPI convention to name function pointer typedefs
 *       and function pointer parameters. see -> https://svn.mpi-forum.org/trac/mpi-forum-web/ticket/7
 *
 * \todo Define parameter list convention. Most common parameters first?
 *
 * \todo Define critical section interface
 *
 * \todo How to target specific devices? As it is below, BGQ would need to
 *       advance two xmi contexts - one for the torus and another for shmem.
 */

/**
 *
 * 1. Set the device/fabric/network for pt2pt (dma, shmem, etc)
 * 2. Set the sync and async recv callbacks and clientdata
 */
xmi_result_t XMI_Initialize (xmi_context_t * context);

xmi_result_t XMI_Finalize (xmi_context_t * context);


/**
 * \brief Singular advance of the progress engine.
 *
 * May complete zero, one, or more outbound transfers. May
 * invoke dispatch handlers for incoming transfers.
 *
 * If interrupt mode is enabled it is not neccesary to explicitly
 * advance the progress engine.
 *
 * \todo Define return code - any events occured?
 */
xmi_result_t XMI_Advance (xmi_context_t * context);




/**
 * \brief Retrieve the configuration structure for an XMI context
 *
 * Specifies XMI attributes such as interrupt mode (default is enabled on Power
 * and disabled on Blue Gene). Configuration structure is used as input to the
 * XMI_Initialize() function.
 *
 * This function may be invoked before XMI_Initialize()
 *
 * \todo Define configuration attriutes (interrupt mode, send/recv buffer space, etc)
 *
 * \param[in]     context       Pointer to an initialized XMI application.
 *                              context. If \c NULL then the default
 *                              configuration is returned
 * \param[in,out] configuration Pointer to the configuration structure to update
 */
xmi_result_t XMI_Configuration (xmi_context_t       * context,
                                xmi_configuration_t * configuration);

#if 0
xmi_result_t XMI_Configure (...);
#endif


#endif /* __xmi_h__ */
