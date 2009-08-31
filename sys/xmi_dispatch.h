/**
 * \file xmi_dispatch.h
 * \brief messaging interface
 */
#ifndef __xmi_dispatch_h__
#define __xmi_dispatch_h__

#include "xmi_types.h"
#include "xmi_p2p.h"
#include "xmi_collectives.h"

#ifdef __cplusplus
extern "C"
{
#endif
    typedef union
    {
        xmi_dispatch_p2p_fn         p2p;
        xmi_dispatch_ambroadcast_fn ambroadcast;
        xmi_dispatch_amscatter_fn   amscatter;
        xmi_dispatch_amreduce_fn    amreduce;
    }xmi_dispatch_callback_fn;

    /*****************************************************************************/
    /**
     * \defgroup dispatch xmi dispatch interface
     *
     * Some brief documentation on dispatch stuff ...
     * \{
     */
    /*****************************************************************************/
    /**
     * \brief Initialize the dispatch functions for a dispatch id.
     *
     * This is a local, non-collective operation. There is no communication
     * between tasks.
     *
     * \param[in] context    XMI communication context
     * \param[in] dispatch   Dispatch identifier to initialize
     * \param[in] fn         Dispatch receive function
     * \param[in] cookie     Dispatch function cookie
     * \param[in] options    Dispatch registration assertions
     *
     */
    xmi_result_t XMI_Dispatch_set (xmi_context_t            * context,
                                   xmi_dispatch_t             dispatch,
                                   xmi_dispatch_callback_fn   fn,
                                   void                     * cookie,
                                   xmi_send_hint_t            options);
    /** \} */ /* end of "dispatch" group */

#ifdef __cplusplus
};
#endif

#endif /* __xmi_dispatch_h__ */
