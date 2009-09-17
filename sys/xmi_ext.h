/**
 * \file sys/xmi_ext.h
 * \brief platform-specific messaging interface
 */
#ifndef __xmi_ext_h__
#define __xmi_ext_h__

#ifndef XMI_EXT_ATTR
#define XMI_EXT_ATTR 1000 /**< starting value for extended attributes */
#endif

typedef struct {
  int x; /**< X value */
  int y; /**< Y value */
  int z; /**< Z value */
} xmi_coordinates_t;

typedef enum {
  /* Attribute                    Init / Query / Update    */
  XMI_COORDINATES = XMI_EXT_ATTR, /** Q : xmi_coordinates_t: coordinates of this task */
} xmi_attribute_ext_t;

#endif
