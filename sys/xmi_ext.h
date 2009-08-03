/**
 * \file xmi_ext.h
 * \brief platform-specific messaging interface
 */
#ifndef __xmi_ext_h__
#define __xmi_ext_h__

#ifndef XMI_EXT_ATTR
#define XMI_EXT_ATTR 1000
#endif

typedef struct {
    int x, y, z;
} xmi_coordinates_t;

typedef enum {
    // Attribute        // Init / Query / Update
    XMI_COORDINATES     // Q : xmi_coordinates_t: coordinates of this task
        = XMI_EXT_ATTR,
} xmi_attribute_ext_t;

#endif
