/**
 * \file xmi_dispatch.h
 * \brief xmi active messaging interface
 * \defgroup dispatch Dispatch stuff
 * \{
 */
#ifndef __xmi_dispatch_h__
#define __xmi_dispatch_h__

#include "xmi.h"
#include "xmi_am.h"

typedef enum
{
  XMI_OPTION_RDMA = 0x01,
  XMI_OPTION_SMP  = 0x02
} xmi_option_t;


/**
 * \brief Initialize the dispatch functions for a dispatch id.
 *
 * This is a local, non-collective operation. There is no communication
 * between tasks.
 *
 * \param[in] context    XMI communication context
 * \param[in] dispatch   Dispatch identifier to initialize
 * \param[in] fn         Dispatch receive function
 * \param[in] clientdata Dispatch function clientdata
 * \param[in] options    Dispatch registration options - bitmask?
 *
 */
xmi_result_t XMI_Dispatch_set (xmi_context_t           * context,
                               xmi_dispatch_t            dispatch,
                               xmi_dispatch_callback_t   fn,
                               void                    * cookie,
                               xmi_option_t              options);

/**
 * \}
 * \addtogroup dispatch
 *
 * More documentation for dispatch stuff....
 */

#endif /* __xmi_dispatch_h__ */
