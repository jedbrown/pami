/**
 * \file sys/examples/config/xmi_config_test.c
 * \brief Example XMI_Send source code.
 *
 * ***** need more description of this example ***
 */
#include <stdio.h>
#include "xmi.h"

xmi_client_t  client;
xmi_context_t context;

void Init()
{
    xmi_result_t rc = XMI_Client_initialize ("MPI", &client);
    if (rc)
        printf("XMI_Client_initialize returned %d due to %s\n", rc, XMI_Last_error());

    char     protocol_name[] = "MPI";
    unsigned reliability = 0;
    unsigned intr_mask = 0;
    xmi_configuration_t  config[] = {
        { XMI_PROTOCOL_NAME,  protocol_name },
        { XMI_RELIABILITY,    &reliability  },
        { XMI_RECV_INTR_MASK, &intr_mask    },
    };

    int num_configs = sizeof(config)/sizeof(config[0]);
    rc = XMI_Context_create (client, config, num_configs, &context);
    if (rc)
        printf("XMI_Context_create returned %d due to %s\n", rc, XMI_Last_error());
}

/*
 * \brief Update config
 */
void Update()
{
    xmi_intr_mask_t  intr_mask = 2;
    XMI_Configuration_update(context, XMI_RECV_INTR_MASK, &intr_mask);

    xmi_user_config_t  config;
    config.key   = "some key";
    config.value = "some value";
    XMI_Configuration_update(context, XMI_USER_CONFIG, &config);
    // The XMI implementation can check user config
    // but it is not part of the API specification
}

/*
 * \brief Query option 1: Everything pointing to XMI managed memory
 */
void Query1()
{
    // Query attribute with a simple value
    size_t  *num_tasks;
    XMI_Configuration_query(context, XMI_NUM_TASKS, &num_tasks);
    printf("num tasks = %d\n", *num_tasks);

    // Query attribute with a struct value
    // (need to cast extended attribute type to attribute type)
    xmi_coordinates_t  *coors;
    XMI_Configuration_query(context, (xmi_attribute_t)XMI_COORDINATES, &coors);
    printf("(%d, %d, %d)\n", coors->x, coors->y, coors->z);

    // Query XMI attributes
    // It's not clear how the list of attributes is going to be used
    // All attributes are defined in xmi.h + xmi_ext.h already
    xmi_attribute_t *xmi_attrs;
    XMI_Configuration_query(context, XMI_ATTRIBUTES, &xmi_attrs);
    for (; *xmi_attrs; xmi_attrs++)
        printf("%d\n", *xmi_attrs);

    // Query user defined keys and values
    xmi_user_key_t *user_keys;
    XMI_Configuration_query(context, XMI_USER_KEYS, &user_keys);
    for (; *user_keys; user_keys++) {
        xmi_user_config_t  config;
        config.key   = *user_keys;
        XMI_Configuration_query(context, XMI_USER_CONFIG, &config);
        printf("%s = %s\n", config.key, config.value);
    }
}

/*
 * \brief Query option 2: Everything allocated by user
 */
void Query2()
{
    // Query attribute with a simple value
    size_t  num_tasks;
    XMI_Configuration_query(context, XMI_NUM_TASKS, &num_tasks);
    printf("num tasks = %d\n", num_tasks);

    // Query attribute with a struct value
    // (need to cast extended attribute type to attribute type)
    xmi_coordinates_t  coors;
    XMI_Configuration_query(context, (xmi_attribute_t)XMI_COORDINATES, &coors);
    printf("(%d, %d, %d)\n", coors.x, coors.y, coors.z);

    // Query XMI attributes
    // Need an extra query for number of attributes
    // ...

    // Query user defined keys and values
    // Need an extra query for number of keys
    // ...
}
