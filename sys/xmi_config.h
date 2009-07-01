#ifndef XMI_H
#define XMI_H

typedef int           xmi_result_t;
typedef void *        xmi_context_t;
typedef unsigned int  xmi_task_t;
typedef unsigned int  xmi_intr_mask_t;
typedef unsigned int  xmi_bool_t;

typedef char *        xmi_user_key_t;
typedef char *        xmi_user_value_t;

typedef struct {
    xmi_user_key_t    key;
    xmi_user_value_t  value;
} xmi_user_config_t;

/*
 * This enum contains ALL possible attributes for all hardware
 */
typedef enum {
    // Attribute        // Query / Update
    XMI_ATTRIBUTES,     // Q : xmi_attribute_t[]: attributes on the platform
                        //     terminated with NULL
    XMI_TASK_ID,        // Q : xmi_task_t: ID of this task
    XMI_NUM_TASKS,      // Q : xmi_task_t: total number of tasks
    XMI_RECV_INTR_MASK, // QU: xmi_intr_mask_t: receive interrupt mask
    XMI_CHECK_PARAMS,   // QU: xmi_bool_t: check parameters 
    XMI_USER_KEYS,      // Q : xmi_user_key_t[]: user-defined keys
                        //     terminated with NULL
    XMI_USER_CONFIG,    // QU: xmi_user_config_t: user-defined configuration
                        //     key and value are shallow-copied for update
} xmi_attribute_t;

#include <xmi_ext.h>    // platform-specific

/*
 * Query the value of an attribute
 *
 * \param [in]  context    The XMI context
 * \param [in]  attribute  The attribute of interest
 * \param [out] value      Pointer to the retrieved value
 *
 * \note 
 * \returns 
 *   XMI_SUCCESS
 *   XMI_ERR_CONTEXT
 *   XMI_ERR_ATTRIBUTE
 *   XMI_ERR_VALUE
 */
xmi_result_t XMI_Config_query(xmi_context_t context, xmi_attribute_t attribute,
        void* value_out);

/*
 * Update the value of an attribute
 *
 * \param [in]  context    The XMI context
 * \param [in]  attribute  The attribute of interest
 * \param [in]  value      Pointer to the new value 
 *
 * \note 
 * \returns 
 *   XMI_SUCCESS
 *   XMI_ERR_CONTEXT
 *   XMI_ERR_ATTRIBUTE
 *   XMI_ERR_VALUE
 */
xmi_result_t XMI_Config_update(xmi_context_t context, xmi_attribute_t attribute,
        void* value_in);

#endif
