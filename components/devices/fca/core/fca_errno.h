/**
* Copyright (C) Mellanox Technologies Ltd. 2001-2011.  ALL RIGHTS RESERVED.
* This software product is a proprietary product of Mellanox Technologies Ltd.
* (the "Company") and all right, title, and interest and to the software product,
* including all associated intellectual property rights, are and shall
* remain exclusively with the Company.
*
* This software product is governed by the End User License Agreement
* provided with the software product.
* $COPYRIGHT$
* $HEADER$
*/





#ifndef __FCA_ERRNO_H__
#define __FCA_ERRNO_H__

#define	ENOFMM                      257 /* FMM not found */
#define	EJOIN                       258 /* Failed to join multicast */
#define	EDTYPE                      259 /* Unsupported MPI dtype */
#define	EOP                         260 /* Unsupported MPI coll operation */
#define ESHMEM_GET                  261 /* Shmem get failed */
#define EINSMALLMTU                 262 /* Device MTU is too small */
#define ECOMM_INIT_FAILED           263 /* comm_init failed */
#define EOSM_GENERAL_FAIL           264 /* opensm general failure to complete request. */
#define EOSM_GRP_NO_EXIST           265 /* mcast group deleted not by soap API (probably bug in OSM) */
#define EOSM_NO_JOB                 266 /* No job exists in OSM */
#define EOSM_JOB_NOT_READY          267 /* Job's tree still not ready. try again later. */
#define EOSM_NO_MLIDS               268 /* opensm is out of mlids. */
#define ELOGINIT                    269 /* Failed to initialize logger. */
#define EMNOTSUP                    270 /* Method not supported (SM) */
#define EBADVER                     271 /* Bad version (SM)  */
#define EINVMATTR                   272 /* Invalid combination of method/attributes (SM) */
#define ELOGTREE                    273 /* Couldn't create new logical tree */
#define ENOELEM                     274 /* No such element found */
#define ETOOMANYELEMS               275 /* Too many elements */
#define EELEMTBDELETED              276 /* Element is about to be deleted */
#define ENEEDICPUROOT               277 /* Couldn't use icpu as master root */
#define ENOLOGTREE                  278 /* Logical tree doesn't exist */
#define EDBLMSG                     279 /* Double message - already got it before */
#define ENOROUTE                    280 /* Cannot find a suitable route */
#define ELOGTREENOICPU              281 /* Couldn't build logical tree because no icpu found in the fabric */
#define EDBLDATA                    282 /* Already contain this data */
#define EMULFCAMGR                  283 /* Multiple FCA managers are active */
#define EFMMBADVER                  284 /* Bad FCA manager version */
#define ENOTREADY                   285 /* In not ready state */
#define ENOLICENSE                  286 /* Not licensed to run */
#define EUSEMPI                     287 /* Dynamic rules check failed - use MPI fallback */
#define ENOUFM                      288 /* UFM is not available */
#define ELIBVTOPO                   289 /* Error in libvtopo library */
#define ECLI                        290 /* Error in cli library */
#define EOUTSTANDING                291 /* Can't be handled yet, will be when possible */
#define ERULEPARSE                  292 /* Error parsing rules' section */

/**
 * Convert FCA error code to a message string.
 * NOTE: Standard Linux error codes are a subset of possible FCA error codes.
 *
 * @error: Error code.
 * @return: Error string.
 */
const char *fca_strerror(int error);

#endif
