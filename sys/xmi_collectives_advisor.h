/**
 * \file xmi_collectives_advisor.h
 * \brief XMI client advisor layer interface.
 */

#ifndef __xmi_collective_advisor_h__
#define __xmi_collective_advisor_h__

#include "xmi_types.h"

#ifdef __cplusplus
extern "C"
{
#endif
  
  typedef struct
  {
    xmi_algorithm_t algorithm;
    xmi_metadata_t meta;
    unsigned order;
  } xmi_repository_t;

  xmi_result_t XMI_Collective_Advisor_init (xmi_context_t context);
  xmi_result_t XMI_Collective_Advisor_finalize (xmi_context_t context);
  xmi_result_t XMI_Collective_via_advisor (xmi_context_t context,
                                           xmi_xfer_t *cmd,
                                           xmi_metadata_t cs_meta);  


  /**
   * \brief Invokes a given collective operation and utilizes the XMI advisor
   *        client to execute the most efficient algorithm based on the state
   *        (meta data) of the collective call site.
   * \param[in]   context   xmi context
   * \param[in]   cmd       collective paramaters
   * \param[in]   cs_meta   call site meta data
   *
   * \retval     0            Success
   *
   * \todo doxygen
   */
  xmi_result_t XMI_Collective_via_advisor (xmi_context_t context,
                                           xmi_xfer_t *cmd,
                                           xmi_metadata_t cs_meta);  

#ifdef __cplusplus
};
#endif

#endif /* __xmi_collective_advisor_h__ */
