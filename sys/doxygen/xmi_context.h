/**
 * \file sys/doxygen/xmi_context.h
 * \brief Detailed doxygen documentation for xmi multi-context interface
 */
#ifndef __doxygen_xmi_context_h__
#define __doxygen_xmi_context_h__



/**
 * \addtogroup contexts_and_endpoints Contexts and Endpoints
 *
 * Contexts are parallel "threading points" that an application may use to
 * optimize concurrent communications.
 *
 * \todo define a "threading point"
 *
 * Contexts have the following features:
 * - Contexts are a local resource for, and created by, each task
 * - Every context within a client has equivalent connectivity, functionality,
 *   and semantics
 * - Communication initiated by the local task will use a \ref context
 *   "context object" to identify the specific threading point that will
 *   complete the operation
 * - Communication that addresses a destination task will use an \ref endpoint
 *   "endpoint object" to identify the destination thread point that will
 *   receive the dispatch
 *
 * <b>Context initialization</b>
 *
 * The maximum number of allowed contexts should be queried after the client
 * is initialized with the XMI_Client_initialize() function. The contexts
 * are created with the XMI_Context_createv() function. This function will
 * initialize an array of context opaque objects and will set the
 * "global endpoints" output parameter.  The "global endpoints" is the number
 * of destination endpoints for all tasks in a client.
 *
 * XMI_Context_createv() is a
 * synchronizing operation. Each task may create a different number of local
 * endpoints. However, the special attribute BLAH can be used to assert that
 * every task in the client is creating the same number of contexts
 *
 * <b>Endpoint usage</b>
 *
 * All communication functions require a context to perform the
 * operation on the dedicated hardware resource(s) and an endpoint
 * to address the destination. The array of context opaque objects is initialized
 * when the contexts are created. To obtain an endpoint opaque
 * object the application must invoke one of the two endpoint functions.
 *
 * - The application must invoke XMI_Client_endpoint() to obtain an endpoint to
 *   address a specific context on a destination task.
 * - The application must invoke XMI_Client_endpointv() to initialize an array
 *   of endpoint opaque objects that address the local contexts on a destination task.
 *
 * <b>Endpoint identification</b>
 * Applications may prefer to use a unique integer to identify all endpoints
 * for a client. This can be accomplished by creating an array of
 * endpoint opaque objects and using the index into the array as the endpoint
 * identifier. The XMI_Client_endpointv() function can be used to create the
 * array.
 *
 * \dontinclude examples/endpoint_table.c
 * \until }
 *
 * Endpoints may be identified by a monotonically increasing integer
 * value after the endpoint opaque object array has been initialized.
 *
 * \until }
 *
 * \until XMI_Context_unlock (context[0]);
 *
 * <b>Dispatch</b>
 *
 * A dispatch is specific to a context and contexts are specific
 * to a client. The dispatch function does not need the client as an input
 * parameter because the local endpoint opaque object may contain back-pointers
 * to client information.
 *
 * Contexts and endpoints are both opaque object types and not integers.
 * Applications may prefer to use a unique, monotonically increasing integer to
 * identify a context associated with an endpoint for the client in a registered
 * dispatch function. This can be
 * accomplished by storing the application-defined endpoint identifier in the
 * dispatch function cookie for each dispatch registration.
 */
/**
 * \defgroup context Context
 * \ingroup contexts_and_endpoints
 * \brief Contexts specify a local operation thread point.
 *
 * Contexts are defined as an opaque object type. The application must not
 * directly read or write the value of the object.
 *
 * \par Advice to implementors
 * The opaque context type may contain a back-pointer to the internal
 * client object associated with the context, flow control information,
 * etc.
 */
/**
 * \defgroup endpoint Endpoints
 * \ingroup contexts_and_endpoints
 * \brief Endpoints address a destination thread point on a destination task.
 *
 * Endpoints are defined as an opaque object type. The application must not
 * directly read or write the value of the object.
 *
 * \note The endpoint type may be written by one task and read by another task.
 *       To save storage in an environment where multiple tasks reside on the
 *       same physical node the endpoint table may be stored in a shared memory
 *       area.
 *
 * \par Advice to implementors
 * The opaque endpoint type may contain the task id and local context
 * index, etc.
 *
 */

#endif /* __doxygen_xmi_context_h__ */
