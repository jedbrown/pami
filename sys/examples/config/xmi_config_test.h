#include <stdio.h>
#include <xmi_config.h>

xmi_context_t  context;

/*
 * \brief Update config
 */
void Update()
{
    unsigned int  intr_mask = 2;
    XMI_Config_update(context, XMI_RECV_INTR_MASK, &intr_mask);

    xmi_user_config_t  config;
    config.key   = "some key";
    config.value = "some value";
    XMI_Config_update(context, XMI_USER_CONFIG, &config);
    // The XMI implementation can check user config 
    // but it is not part of the API specification
}

/*
 * \brief Query option 1: Everything pointing to XMI managed memory
 */
void Query1()
{
    // Query attribute with a simple value
    xmi_task_t  *num_tasks;
    XMI_Config_query(context, XMI_NUM_TASKS, &num_tasks);
    printf("num tasks = %d\n", *num_tasks);

    // Query attribute with a struct value
    // (need to cast extended attribute type to attribute type)
    xmi_coordinates_t  *coors;
    XMI_Config_query(context, (xmi_attribute_t)XMI_COORDINATES, &coors);
    printf("(%d, %d, %d)\n", coors->x, coors->y, coors->z);

    // Query XMI attributes
    // It's not clear how the list of attributes is going to be used
    // All attributes are defined in xmi.h + xmi_ext.h already
    xmi_attribute_t *xmi_attrs;
    XMI_Config_query(context, XMI_ATTRIBUTES, &xmi_attrs);
    for (; *xmi_attrs; xmi_attrs++) 
        printf("%d\n", *xmi_attrs);

    // Query user defined keys and values
    xmi_user_key_t *user_keys;
    XMI_Config_query(context, XMI_USER_KEYS, &user_keys);
    for (; *user_keys; user_keys++) {
        xmi_user_config_t  config;
        config.key   = *user_keys;
        XMI_Config_query(context, XMI_USER_CONFIG, &config);
        printf("%s = %s\n", config.key, config.value);
    }
}

/*
 * \brief Query option 2: Everything allocated by user
 */
void Query2()
{
    // Query attribute with a simple value
    xmi_task_t  num_tasks;
    XMI_Config_query(context, XMI_NUM_TASKS, &num_tasks);
    printf("num tasks = %d\n", num_tasks);

    // Query attribute with a struct value
    // (need to cast extended attribute type to attribute type)
    xmi_coordinates_t  coors;
    XMI_Config_query(context, (xmi_attribute_t)XMI_COORDINATES, &coors);
    printf("(%d, %d, %d)\n", coors.x, coors.y, coors.z);

    // Query XMI attributes
    // Need an extra query for number of attributes
    // ...

    // Query user defined keys and values
    // Need an extra query for number of keys
    // ...
}


