/**
 * \file xmi_config.h
 * \brief messaging interface
 * \example config/xmi_config_test.c
 */
#ifndef __xmi_config_h__
#define __xmi_config_h__

#include "xmi.h"

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
    // Attribute        // Init / Query / Update
    XMI_PROTOCOL_NAME=1,// IQ  : char *            : name of the protocol
    XMI_RELIABILITY,    // IQ  : xmi_bool_t        : guaranteed reliability
    XMI_ATTRIBUTES,     //  Q  : xmi_attribute_t[] : attributes on the platform terminated with NULL
    XMI_TASK_ID,        //  Q  : size_t            : ID of this task
    XMI_NUM_TASKS,      //  Q  : size_t            : total number of tasks
    XMI_RECV_INTR_MASK, //  QU : xmi_intr_mask_t   : receive interrupt mask
    XMI_CHECK_PARAMS,   //  QU : xmi_bool_t        : check parameters
    XMI_USER_KEYS,      //  Q  : xmi_user_key_t[]  : user-defined keys terminated with NULL
    XMI_USER_CONFIG,    //  QU : xmi_user_config_t : user-defined configuration key and value are shallow-copied for update
} xmi_attribute_t;

#define XMI_EXT_ATTR 1000 // starting value for extended attributes
#include "xmi_ext.h"    // platform-specific

typedef struct {
    xmi_attribute_t  attr;
    void *           value;
} xmi_config_t;

#if 0
/* Replaced by XMI_Context_create */
xmi_result_t XMI_Init(xmi_context_t *context,
                      int num_configs, xmi_config_t config[]);
#endif
char * XMI_Last_error();

/*
 * \brief NULL_CONTEXT to allow queries outside a context
 */

#define NULL_CONTEXT  ((xmi_context_t)0)

/*
 * \brief Query the value of an attribute
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
 * \brief Update the value of an attribute
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
