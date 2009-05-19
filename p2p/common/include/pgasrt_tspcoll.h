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

/* ************************************************************************* */
/*                         transport collectives                             */
/* ************************************************************************* */

#ifndef __pgasrt_tspcoll_comm_h__
#define __pgasrt_tspcoll_comm_h__

#include "pgasrt.h"

/* *********************************************************************** */
/** @file pgasrt_tspcoll.h                                                 */
/* *********************************************************************** */

#ifdef __cplusplus
extern "C" {
#endif

  /* ******************************************************************** */
  /** \brief initialize collective communication subsystem                */
  /* ******************************************************************** */

  void                 __pgasrt_tspcoll_comm_init  (void);

  /* ******************************************************************** */
  /** \brief return my rank in current communicator                       */
  /* ******************************************************************** */

  __pgasrt_thread_t    __pgasrt_tspcoll_comm_rank  (int               commID);

  /* ******************************************************************** */
  /** \brief calculate absolute rank of any rank in a communicator
   *  \param commID        communicator ID
   *  \param rank          rank in communicator
   *  \returns absolute rank (rank in COMM_WORLD)
   */
  /* ******************************************************************** */

  __pgasrt_thread_t    __pgasrt_tspcoll_comm_rankof(int               commID,
						    __pgasrt_thread_t rank);

  /* ******************************************************************** */
  /** \brief return communicator size                                     */
  /* ******************************************************************** */

  __pgasrt_thread_t    __pgasrt_tspcoll_comm_size  (int               commID);

  /* ******************************************************************** */
  /** \brief split a communicator according to colors.
   *  \param commID          communicator ID
   *  \param newID           ID of new communicator to be created
   *  \param color           which new communicator current node will belong to
   *  \param rank            rank of current node in new communicator
   */
  /* ******************************************************************** */

  void                 __pgasrt_tspcoll_comm_split (int               commID,
						    int               newID,
						    int               color,
						    int               rank);
  
  /* ******************************************************************** */
  /** \brief split COMM_WORLD into new communicators with blocking
   *  \param newID          new communicator's ID
   *  \param BF             blocking factor (number of nodes in each block)
   *  \param ncomms         number of communicators to split into
   */
  /* ******************************************************************** */

  void                 __pgasrt_tspcoll_comm_block (int               newID,
						    int               BF,
						    int               ncomms);
  
  /* ******************************************************************** */
  /*                             collectives                              */
  /* ******************************************************************** */

  /** \brief barrier
   *  \param commID communicator ID
   */

  void                 __pgasrt_tspcoll_barrier     (int               commID);

  /** \brief allgather
   *  \param commID communicator ID
   *  \param sbuf   send buffer
   *  \param dbuf   receive buffer
   *  \param nbytes each node's contribution (bytes)
   */

  void                 __pgasrt_tspcoll_allgather   (int               commID,
						     const void      * sbuf,
						     void            * dbuf,
						     size_t            nbytes);
  
  /** \brief allgatherv
   *  \param commID communicator ID
   *  \param sbuf   send buffer
   *  \param dbuf   receive buffer
   *  \param llengs array of each node's contribution lengths
   */

  void                 __pgasrt_tspcoll_allgatherv  (int               commID,
						     const void      * sbuf,
						     void            * dbuf,
						     size_t          * llengs);
  
  /** \brief scatter
   *  \param commID communicator ID
   *  \param root   originator node ID
   *  \param sbuf   send buffer
   *  \param dbuf   receive buffer
   *  \param nbytes # bytes to send to each node
   */

  void                 __pgasrt_tspcoll_scatter     (int               commID, 
						     int               root,
						     const void      * sbuf,
						     void            * dbuf,
						     size_t            nbytes);

  /** \brief scatterv
   *  \param commID communicator ID
   *  \param root   originator node ID
   *  \param sbuf   send buffer
   *  \param dbuf   receive buffer
   *  \param llens  array of # bytes to send to each node
   */

  void                 __pgasrt_tspcoll_scatterv    (int               commID, 
						     int               root,
						     const void      * sbuf,
						     void            * dbuf,
						     size_t          * llens);
  /** \brief gather
   *  \param commID communicator ID
   *  \param root   node ID gathering all data
   *  \param sbuf   send buffer
   *  \param dbuf   receive buffer
   *  \param nbytes # bytes sent by each node
   */

  void                 __pgasrt_tspcoll_gather      (int               commID, 
						     int               root,
						     const void      * sbuf,
						     void            * dbuf,
						     size_t            nbytes);

  /** \brief gatherv
   *  \param commID communicator ID
   *  \param root   node ID gathering all data
   *  \param sbuf   send buffer
   *  \param dbuf   receive buffer
   *  \param llens  array of lengths sent by each node
   */

  void                 __pgasrt_tspcoll_gatherv     (int               commID, 
						     int               root,
						     const void      * sbuf,
						     void            * dbuf,
						     size_t          * llens);

  /** \brief broadcast
   *  \param commID communicator ID
   *  \param root   broadcast source node ID
   *  \param sbuf   send buffer
   *  \param dbuf   receive buffer
   *  \param nbytes length of send buffer
   */

  void                 __pgasrt_tspcoll_bcast       (int              commID, 
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

  void                 __pgasrt_tspcoll_allreduce   (int               commID,
						     const void      * sbuf,
						     void            * rbuf,
						     __pgasrt_ops_t    op,
						     __pgasrt_dtypes_t dtype,
						     unsigned          nelems);


  /* one sided collectives - draft */


  void *     __pgasrt_tsp_ambcast  (int                            commID,
				    __pgasrt_AMHeader_t          * header,
				    const __pgasrt_local_addr_t    data,
				    size_t                         datalen,
				    __pgasrt_LCompHandler_t        comp_h,
				    void                         * info);


  void *    __pgasrt_tsp_amscatter (int                            commID,
				    __pgasrt_AMHeader_t          * headers,
				    const __pgasrt_local_addr_t  * data,
				    size_t                       * datalengths,
				    __pgasrt_LCompHandler_t        comp_h,
				    void                         * info);

  void *    __pgasrt_tsp_amgather  (int                            commID,
				    __pgasrt_AMHeader_t          * headers,
				    const __pgasrt_local_addr_t  * data,
				    size_t                       * datalengths,
				    __pgasrt_LCompHandler_t        comp_h,
				    void                         * info);


  void *    __pgasrt_tsp_amreduce  (int                            commID,
				    __pgasrt_AMHeader_t          * headers,
                                    const __pgasrt_local_addr_t    data,
				    __pgasrt_ops_t                 op,
				    __pgasrt_dtypes_t              dtype,
				    unsigned                       nelems,
                                    __pgasrt_LCompHandler_t        comp_h,
                                    void                         * info);





#ifdef __cplusplus
}
#endif

#endif
