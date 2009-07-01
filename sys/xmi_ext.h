/**
 * \file xmi_ext.h
 * \brief platform-specific messaging interface
 */
#ifndef __xmi_ext_h__
#define __xmi_ext_h__

typedef struct {
    int x, y, z;
} xmi_coordinates_t;

typedef enum {
    XMI_COORDINATES,    //  O: xmi_coordinates_t: coordinates of this task
} xmi_attribute_ext_t;

#endif
