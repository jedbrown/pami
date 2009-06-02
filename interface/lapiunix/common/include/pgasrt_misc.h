/* ************************************************************************* */
/*                            IBM Confidential                               */
/*                          OCO Source Materials                             */
/*                      IBM XL UPC Alpha Edition, V0.9                       */
/*                                                                           */
/*                      Copyright IBM Corp. 2005, 2007.                      */
/*                                                                           */
/* The source code for this program is not published or otherwise divested   */
/* of its trade secrets, irrespective of what has been deposited with the    */
/* U.S. Copyright Office.                                                    */
/* ************************************************************************* */

#ifndef __PGASRT_MISC_H__
#define __PGASRT_MISC_H__

#undef EXTERN
#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN extern
#endif

#include "pgasrt_config.h"
#include "pgasrt_types.h"

#include <stdlib.h>         /* for size_t */
#include <limits.h>
#include <stddef.h>

/* ************************************************************************ */
/** @file pgasrt_misc.h                                                     */
/* ************************************************************************ */

/* *********************************************************************** */
/*              NOT CATEGORIZED !!!!!!!!!!!!!!!!!                          */
/* *********************************************************************** */

EXTERN void               __pgasrt_trace_set      (int val);

/* *********************************************************************** */
/** \brief fatal error: print error message, stop everything dead.         */
/* *********************************************************************** */

EXTERN void               __pgasrt_fatalerror     (int errcode, 
						   const char * strg,
						   ...);

/* *********************************************************************** */
/** \brief cpu binding: bind hybrid thread ID to a particular
 * processor.  If the PGASRT_CPUMAP environment variable is set, it
 * names a file specifying the file name specifying processor IDs, one
 * line for each hybrid thread ID.
 *
 * \param myID            hybrid ID of current thread.
 */
/* *********************************************************************** */

EXTERN void               __pgasrt_cpubind        (int myID);

/* *********************************************************************** */
/*                         registry                                        */
/* *********************************************************************** */

EXTERN void               __pgasrt_call_init_modules ();
EXTERN void               __pgasrt_call_fini_modules ();
EXTERN void               __pgasrt_register          (void *init, void *fini);

/* *********************************************************************** */
/*                          SVD                                            */
/* *********************************************************************** */

/* *********************************************************************** */
/** \brief initializes the SVD. Return 0 if OK, error code otherwise.      */
/* *********************************************************************** */

EXTERN int
__pgasrt_svd_init                  (unsigned                        numparts);

/* *********************************************************************** */
/** \brief free the SVD table                                              */
/* *********************************************************************** */

EXTERN void 
__pgasrt_svd_free ();

/* *********************************************************************** */
/** \brief return the next free index in the specified partition           */
/* *********************************************************************** */

EXTERN unsigned short 
__pgasrt_svd_get_next_index        (unsigned                       partition);

/* *********************************************************************** */
/** \brief set a directory entry                                           */
/* *********************************************************************** */

EXTERN void 
__pgasrt_svd_set_entry             (__pgasrt_svd_handle_t          handle,
				    __pgasrt_shared_var_types      var_type,
				    void                         * shr_ptr);

/* *********************************************************************** */
/** \brief return a directory entry                                        */
/* *********************************************************************** */

EXTERN __pgasrt_shared_var_ctrl_block *
__pgasrt_svd_get_entry             (__pgasrt_svd_handle_t          handle);

/* *********************************************************************** */
/** \brief free a directory entry                                          */
/* *********************************************************************** */

EXTERN void 
__pgasrt_svd_free_entry            (__pgasrt_svd_handle_t          handle);

/* *********************************************************************** */
/** \brief debugging: print an SVD entry                                   */
/* *********************************************************************** */

EXTERN void
__pgasrt_svd_print_entry           (unsigned int                   part,
				    unsigned int                   index);

/* *********************************************************************** */
/** \brief Figure out the local address of an SVD entity                   */
/* *********************************************************************** */

EXTERN __pgasrt_local_addr_t
__pgasrt_svd_addrof                (__pgasrt_svd_handle_t          handle);

/* *********************************************************************** */
/** \brief Dump SVD table                                                  */
/* *********************************************************************** */

EXTERN void 
__pgasrt_svd_print                 ();


#undef UPDATE
/* *********************************************************************** */
/** \brief update operator for multiple data types                         */
/* *********************************************************************** */
#define UPDATE(type,name_ext) \
EXTERN void                                                          \
__pgasrt_update_##name_ext         (type                   * dst,    \
                                    type                   * src,    \
                                    __pgasrt_ops_t           op)

UPDATE(unsigned char, byte);
UPDATE(char, chr);
UPDATE(short, srt);
UPDATE(unsigned short, hwrd);
UPDATE(double, dbl);
UPDATE(float, flt);
UPDATE(int, int);
UPDATE(unsigned, word);
UPDATE(long long, llg);
UPDATE(unsigned long long, dwrd);

#undef UPDATE

EXTERN void 
__pgasrt_update                    (__pgasrt_local_addr_t    dst,
				    __pgasrt_local_addr_t    src,
				    __pgasrt_dtypes_t        type,
				    __pgasrt_ops_t           op);



#endif

