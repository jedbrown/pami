/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file sys/pami_counter.h
 * \brief ???
 */

#ifndef __pami_counter_h__
#define __pami_counter_h__

#include <stdlib.h>
#include <stdint.h>
#include "pami.h"
#ifdef __cplusplus
extern "C"
{
#endif

/* \brief PAMI counter */
    typedef struct {
        size_t opaque[2];
    } pami_counter_t;

    /* \brief Set counter value */
    pami_result_t PAMI_Counter_set(pami_context_t context, pami_counter_t *counter, size_t value);

    /* \brief Get counter value */
    pami_result_t  PAMI_Counter_get(pami_context_t context, pami_counter_t *counter, size_t *value);
    /* \brief Wait for the counter to reach/exceed the specified the value
       then the specified value is substracted from the counter and the new
       value is returned. CPU is released after poll_count expires. */
    pami_result_t PAMI_Counter_wait(pami_context_t context, pami_counter_t *counter,
                     size_t target_value, size_t *current_value, size_t poll_count);

    /* \brief Simple send command using counters for notifications */
    typedef struct
    {
        pami_send_t             send;     /**< Common send parameters */
        struct
        {
            size_t               bytes;    /**< Number of bytes of data */
            void               * addr;     /**< Address of the buffer */
            pami_counter_t      * local_done;/**< Local message completion event */
            pami_counter_t      * remote_done;/**< Remote message completion event */
        } simple;                        /**< Simple send parameters */
    } pami_send_simple_counter_t;

    /* \brief Typed send command using counters for notifications */
    typedef struct
    {
        pami_send_t             send;     /**< Common send parameters */
        struct
        {
            size_t               bytes;    /**< Number of bytes of data */
            void               * addr;     /**< Starting address of the buffer */
            size_t               offset;   /**< Starting offset */
            pami_type_t           type;     /**< Datatype */
            pami_counter_t      * local_done;/**< Local message completion event */
            pami_counter_t      * remote_done;/**< Remote message completion event */
        } typed;                         /**< Typed send parameters */
    } pami_send_typed_counter_t;

    /* \brief Simple send function using counters for notifications */
    pami_result_t PAMI_Send_simple_counter (pami_context_t       context,
                                          pami_send_simple_counter_t * parameters);

    /* \brief Typed send function using counters for notifications */
    pami_result_t PAMI_Send_typed_counter (pami_context_t       context,
                                         pami_send_typed_counter_t * parameters);
#ifdef __cplusplus
};
#endif
#endif /* __pami_p2p_h__ */
