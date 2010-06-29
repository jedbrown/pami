/**
 * \file sys/examples/pami_examples.h
 * \brief PAMI code examples
 */

#ifndef __sys_examples_pami_examples_h__
#define __sys_examples_pami_examples_h__

/**
 * \example endpoint_table.c
 *
 * <b>Alternative endpoint table addressing example</b>
 *
 * This example demonstrates how to construct an endpoint table containing
 * all endpoints in the system. The endpoint table can then be used, in
 * concert with a wrapper function, as a way to address all endpoints as
 * monotonically increasing integers instead of directly using the
 * pami_endpoint_t opaque type.
 *
 * \dontinclude endpoint_table.c
 *
 * The endpoint table must be initialized for each task after the client
 * and contexts are created.
 *
 * In this example the global endpoint table, \c _endpoint, is initialized
 * in the \b createEndpointTable() function. The endpoint table is allocated from
 * memory to contain a pami_endpoint_t element for each endpoint in the system.
 *
 * \note Tasks that are located on the same node and have access to the
 *       same shared memory segment may place this table in shared memory
 *       to reduce the amount of memory consumed by each process.
 *
 * In this example each task in the client creates an identical number of
 * contexts (in this case 4). This allows the endpoint table to be created
 * without communication with any other endpoints.
 *
 * The PAMI_Endpoint_createv() function is used to initialize the endpoint
 * table in a loop over all tasks in the system.  The pointer to the next
 * location in the endpoint table to initialized is incremented by the number
 * of endpoints created for each task.  This allows for the case when the
 * tasks in the client may create different number of contexts.
 *
 * \par
 * \skip _endpoint;
 * \until };
 *
 * After the endpoint table has been initialized the task may send to any
 * endpoint in the system by specifying the \em index of the destination
 * endpoint in the endpoint table to a wrapper send function.
 *
 * In this example the wrapper send function simply accesses the global
 * pointer to the endpoint table in order to determine the actual destination
 * endpoint for the send operation.
 *
 * \par
 * \skip send_endpoint
 * \until };
 *
 * The client application software may now send to an endpoint using an
 * integer identifier instead of the pami_endpoint_t opaque type identifier.
 * All other PAMI configuration parameters remain standard.
 *
 * \par
 * \skip _lock
 * \until _unlock
 *
 * <b>Full example source code</b>
 *
 */

/**
 * \example barrier.c
 * \dontinclude barrier.c
 *
 * <b>A simple barrier test case</b>
 *
 *  This example demonstrates how to issue a barrier collective call
 *  on the world geometry.  This simple test only uses one context
 *  and one client.
 *
 * We must first initialize PAMI clients and contexts for communication
 *
 * \par
 * \skipline Docs1
 * \until Docs2
 *
 * Next, we query our task id, which is the unique task number for this job
 * The task number can be used for geometry and endpoint creation.  This
 * will be used later in this example to control which nodes print results.
 * In the case of barrier, we may only want one node to print the results
 * of the test, so we will save the task id to print only on rank 0.
 *
 * \par
 * \skipline Docs3
 * \until Docs4
 *
 * The next step in any of the geometry creation routines is to create a
 * geometry object.  In this particular example, we will create a geometry
 * that is the "world" object, that is, a geometry that contains all the nodes
 * in the current client.  The geometry object is scoped to the client, but
 * and conceptually contains a list of all the tasks within the client.
 *
 * \par
 * \skipline Docs5
 * \until Docs6
 *
 * Now that we have a "world" geometry object, we need to query it
 * for a set of available algorithms.  In this example , we want to obtain
 * first the number of available algorithms.
 * memory to contain a pami_endpoint_t element for each endpoint in the system.
 * At the end of the call to \b PAMI_Geometry_algorithms_num, the num_algorithm
 * array will contain two entries, the "always works" list, and the "must query"
 * list.  In this example, we will only use the "always works" list of
 * collectives.
 *
 * \note To use collectives in the "must query" list, additional
 *       steps must be taken to determine if the collective will work for
 *       this particular geometry and call site parameters.
 *
 * \par
 * \skipline Docs7
 * \until Docs8
 *
 * \par
 * \skipline Docs9
 * \until Docs10
 *
 *
 * \par
 * \skipline Docs10
 * \until Docs11
 *
 * \note Tasks that are located on the same node and have access to the
 *       same shared memory segment may place this table in shared memory
 *       to reduce the amount of memory consumed by each process.
 *
 * In this example each task in the client creates an identical number of
 * contexts (in this case 4). This allows the endpoint table to be created
 * without communication with any other endpoints.
 *
 * The PAMI_Endpoint_createv() function is used to initialize the endpoint
 * table in a loop over all tasks in the system.  The pointer to the next
 * location in the endpoint table to initialized is incremented by the number
 * of endpoints created for each task.  This allows for the case when the
 * tasks in the client may create different number of contexts.
 *
 * \par
 * \skip _endpoint;
 * \until };
 *
 * After the endpoint table has been initialized the task may send to any
 * endpoint in the system by specifying the \em index of the destination
 * endpoint in the endpoint table to a wrapper send function.
 *
 * In this example the wrapper send function simply accesses the global
 * pointer to the endpoint table in order to determine the actual destination
 * endpoint for the send operation.
 *
 * \par
 * \skip send_endpoint
 * \until };
 *
 * The client application software may now send to an endpoint using an
 * integer identifier instead of the pami_endpoint_t opaque type identifier.
 * All other PAMI configuration parameters remain standard.
 *
 * \par
 * \skip _lock
 * \until _unlock
 *
 * <b>Full example source code</b>
 *
 */




#endif /* __sys_examples_pami_examples_h__ */
