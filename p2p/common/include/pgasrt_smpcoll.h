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

#ifndef __pgasrt_smpcoll_comm_h__
#define __pgasrt_smpcoll_comm_h__

#include "pgasrt.h"

/* *********************************************************************** */
/** @file pgasrt_smpcoll.h                                                 */
/* *********************************************************************** */

#ifdef __cplusplus
extern "C" {
#endif

  /* ******************************************************************** */
  /** \brief initialize collective communication subsystem                */
  /* ******************************************************************** */

  void                 __pgasrt_smpcoll_comm_init  (void);

  /* ******************************************************************** */
  /** \brief return my rank in current communicator                       */
  /* ******************************************************************** */

  __pgasrt_thread_t    __pgasrt_smpcoll_comm_rank  (int               commID);

  /* ******************************************************************** */
  /** \brief calculate absolute rank of any rank in a communicator
   *  \param commID        communicator ID
   *  \param rank          rank in communicator
   *  \returns absolute rank (rank in COMM_WORLD)
   */
  /* ******************************************************************** */

  __pgasrt_thread_t    __pgasrt_smpcoll_comm_rankof(int               commID,
						    __pgasrt_thread_t rank);

  /* ******************************************************************** */
  /** \brief return communicator size                                     */
  /* ******************************************************************** */

  __pgasrt_thread_t    __pgasrt_smpcoll_comm_size  (int               commID);

  /* ******************************************************************** */
  /** \brief split a communicator according to colors.
   *  \param commID          communicator ID
   *  \param newID           ID of new communicator to be created
   *  \param color           which new communicator current thread will belong to
   *  \param rank            rank of current thread in new communicator
   */
  /* ******************************************************************** */

  void                 __pgasrt_smpcoll_comm_split (int               commID,
						    int               newID,
						    int               color,
						    int               rank);

  /* ******************************************************************** */
  /** \brief split COMM_WORLD into new communicators with blocking
   *  \param newID          new communicator's ID
   *  \param BF             blocking factor (number of threads in each block)
   *  \param ncomms         number of communicators to split into
   */
  /* ******************************************************************** */

  void                 __pgasrt_smpcoll_comm_block (int               newID,
						    int               BF,
						    int               ncomms);
  
  /* ******************************************************************** */
  /*                             collectives                              */
  /* ******************************************************************** */

  /** \brief barrier
   *  \param commID communicator ID
   */

  void                 __pgasrt_smpcoll_barrier     (int               commID);

  /** \brief allgather
   *  \param commID communicator ID
   *  \param sbuf   send buffer
   *  \param dbuf   receive buffer
   *  \param nbytes each thread's contribution (bytes)
   */

  void                 __pgasrt_smpcoll_allgather   (int               commID,
						     const void      * sbuf,
						     void            * dbuf,
						     size_t            nbytes);
  
  /** \brief allgatherv
   *  \param commID communicator ID
   *  \param sbuf   send buffer
   *  \param dbuf   receive buffer
   *  \param llengs array of each thread's contribution lengths
   */

  void                 __pgasrt_smpcoll_allgatherv  (int               commID,
						     const void      * sbuf,
						     void            * dbuf,
						     size_t          * llengs);
  
  /** \brief scatter
   *  \param commID communicator ID
   *  \param root   originator thread ID
   *  \param sbuf   send buffer
   *  \param dbuf   receive buffer
   *  \param nbytes # bytes to send to each thread
   */

  void                 __pgasrt_smpcoll_scatter     (int               commID, 
						     int               root,
						     const void      * sbuf,
						     void            * dbuf,
						     size_t            nbytes);

  /** \brief scatterv
   *  \param commID communicator ID
   *  \param root   originator thread ID
   *  \param sbuf   send buffer
   *  \param dbuf   receive buffer
   *  \param llens  array of # bytes to send to each thread
   */

  void                 __pgasrt_smpcoll_scatterv    (int               commID, 
						     int               root,
						     const void      * sbuf,
						     void            * dbuf,
						     size_t          * llens);

  /** \brief gather
   *  \param commID communicator ID
   *  \param root   thread ID gathering all data
   *  \param sbuf   send buffer
   *  \param dbuf   receive buffer
   *  \param nbytes # bytes sent by each thread
   */

  void                 __pgasrt_smpcoll_gather      (int               commID, 
						     int               root,
						     const void      * sbuf,
						     void            * dbuf,
						     size_t            nbytes);

  /** \brief gatherv
   *  \param commID communicator ID
   *  \param root   thread ID gathering all data
   *  \param sbuf   send buffer
   *  \param dbuf   receive buffer
   *  \param llens  array of lengths sent by each thread
   */

  void                 __pgasrt_smpcoll_gatherv     (int               commID, 
						     int               root,
						     const void      * sbuf,
						     void            * dbuf,
						     size_t          * llens);

  /** \brief broadcast
   *  \param commID communicator ID
   *  \param root   broadcast source thread ID
   *  \param sbuf   send buffer
   *  \param dbuf   receive buffer
   *  \param nbytes length of send buffer
   */

  void                 __pgasrt_smpcoll_bcast       (int              commID, 
						     int               root,
						     const void      * sbuf,
						     void            * dbuf,
						     size_t            nbytes);
  
  /** \brief allreduce
   *  \param commID communicator ID
   *  \param sbuf   send buffer
   *  \param rbuf   receive buffer
   *  \param op     operation to execute: \ref __pgasrt_ops_t
   *  \param dtype  element type in operation: \ref __pgasrt_dtypes_t
   *  \param nelems number of elements reduced
   */

  void                 __pgasrt_smpcoll_allreduce   (int               commID,
						     const void      * sbuf,
						     void            * rbuf,
						     __pgasrt_ops_t    op,
						     __pgasrt_dtypes_t dtype,
						     unsigned          nelems);

#ifdef __cplusplus
}
#endif

#endif
