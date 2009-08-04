/**
 * \file xmi_dgs.h
 * \brief XMI Data Gather Scatter functions
 * \todo This is meant to be a component that has no dependency on XMI.
 *       Need to clean up the syntax.
 */
#ifndef __xmi_dgs_h__
#define __xmi_dgs_h__

#include <string.h> // for size_t

/********************************************************************/
/*
 * Data Type
 */
typedef void *        xmi_type_t;

/* Create a new type */
int XMI_Type_create(xmi_type_t *type);

/* Add simple buffers */
int XMI_Type_add_simple(xmi_type_t type, size_t bytes, 
        size_t offset, size_t reps, size_t stride);

/* Add typed buffers */
int XMI_Type_add_typed(xmi_type_t type, xmi_type_t sub_type, 
        size_t offset, size_t reps, size_t stride);

/* Commit the type, which can no longer be modified afterwards */
int XMI_Type_commit(xmi_type_t type);

/* Destroy the type */
/*
   What if some in-flight messages are still using it?
   What if some other types have references to it?
 */
int XMI_Type_destroy(xmi_type_t type);


/********************************************************************/
/*
 * Data Gather Scatter Machine
 */
typedef void *        xmi_dgsm_t;

typedef void (*xmi_dgsm_callback_t) (
    void               * dst,          /* IN: destination buffer */
    void               * src,          /* IN: source data */
    size_t               size,         /* IN: size of data to handle */
    void               * cookie);      /* IN: cookie to the callback */

/* Create a DGSM to be associated with a data buffer and a type */
int XMI_Dgsm_create(
    void               * address,      /* IN: address of data buffer */
    xmi_type_t           type,         /* IN: type of the data */
    xmi_dgsm_t         * dgsm);        /* OUT: DGSM created */

/* Destroy a DGSM */
int XMI_Dgsm_destroy(
    xmi_dgsm_t           dgsm);        /* IN: DGSM to destroy */

/*
 * Associate a data-processing callback to a DGSM.
 *
 * By default, a DGSM just copies data. The callback provides a mechanism
 * for the user to further manipulate the data.
 */
int XMI_DGSM_set_callback(
    xmi_dgsm_t           dgsm,         /* IN: DGSM to get the callback */
    xmi_dgsm_callback_t  callback,     /* IN: callback to handle data chunk */
    void               * cookie);      /* IN: cookie to the callback */

/* Move the DGSM cursor to a specified offset */
int XMI_Dgsm_move_cursor(
    xmi_dgsm_t           dgsm,         /* IN: DGSM for data buffer */
    size_t               offset);      /* IN: offset to move to */

/* Gather DGSM data to a contiguous buffer */
int XMI_Dgsm_gather(
    xmi_dgsm_t           dgsm,         /* IN: DGSM for source buffer */
    void               * address,      /* IN: address of destination buffer */
    size_t               size);        /* IN: size of destination buffer */

/* Scatter contiguous data to DGSM data buffer */
int XMI_Dgsm_scatter(
    xmi_dgsm_t           dgsm,         /* IN: DGSM for destination buffer */
    void               * address,      /* IN: address of source buffer */
    size_t               size);        /* IN: size of source buffer */

#endif
