/*  
    Low Level Standalone Collectives API 
    Features requested:

    1)  Non-blocking API:                Done
    2)  Not Network Specific(abstract):  Done
    3)  Open Source:                     Not Yet
    4)  Host Multiple High Level Libs:   Done
    5)  Topology support (geometries):   Done


    ToDo:

    1)  Define Progress Model/Threading policy
    2)  Missing function:  allgather/scan/others?  (is multicast sufficient)
    3)  Fencing on onesided operations:  is consistency sufficient
    Do we want to use an AllFence operation?
    Pros:  Allfence might be more clear/easy to understand
    Cons:  consistency doesn't require a separate call to fence
    4)  Process model:  multiple instances per thread?

*/

/**
 * \file ll_collectives.h
 * \brief Common external collective layer interface.
 */

#ifndef __ll_collective_h__
#define __ll_collective_h__

#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif
 
    /* ************************************************************************* */
    /* **************     TYPES   ********************************************** */
    /* ************************************************************************* */
    typedef enum
    {
        LL_UNDEFINED_OP = 0,
        LL_NOOP,
        LL_MAX,
        LL_MIN,
        LL_SUM,
        LL_PROD,
        LL_LAND,
        LL_LOR,
        LL_LXOR,
        LL_BAND,
        LL_BOR,
        LL_BXOR,
        LL_MAXLOC,
        LL_MINLOC,
        LL_USERDEFINED_OP,
        LL_OP_COUNT
    }LL_Op;
    typedef enum
    {
        /* Standard/Primative DT's */
        LL_UNDEFINED_DT = 0,
        LL_SIGNED_CHAR,
        LL_UNSIGNED_CHAR,
        LL_SIGNED_SHORT,
        LL_UNSIGNED_SHORT,
        LL_SIGNED_INT,
        LL_UNSIGNED_INT,
        LL_SIGNED_LONG_LONG,
        LL_UNSIGNED_LONG_LONG,
        LL_FLOAT,
        LL_DOUBLE,
        LL_LONG_DOUBLE,
        LL_LOGICAL,
        LL_SINGLE_COMPLEX,
        LL_DOUBLE_COMPLEX,
        /* Max/Minloc DT's */
        LL_LOC_2INT,
        LL_LOC_SHORT_INT,
        LL_LOC_FLOAT_INT,
        LL_LOC_DOUBLE_INT,
        LL_LOC_2FLOAT,
        LL_LOC_2DOUBLE,
        LL_USERDEFINED_DT,
        LL_DT_COUNT
    }LL_Dt;

    typedef enum LL_Config_t
	{
	    LL_CFG_MULTISYNC = 0,
	    LL_CFG_MULTICAST,
	    LL_CFG_MANY2MANY,
	    LL_CFG_MULTICOMBINE,
	    LL_CFG_COUNT
	}LL_Config_t;

    typedef enum
    {
        LL_SUCCESS = 0,  /**< Successful execution        */
        LL_NERROR  =-1,  /**< Generic error (-1)          */
        LL_ERROR   = 1,  /**< Generic error (+1)          */
        LL_INVAL,        /**< Invalid argument            */
        LL_UNIMPL,       /**< Function is not implemented */
        LL_EAGAIN,       /**< Not currently availible     */
        LL_SHUTDOWN,     /**< Rank has shutdown           */
        LL_CHECK_ERRNO,  /**< Check the errno val         */
        LL_OTHER,        /**< Other undefined error       */
    }LL_Result;

    typedef struct LL_Error_t
    {
        LL_Result result;
    }LL_Error_t;

    typedef void (*LL_Callback) (void         * clientdata,
                                 LL_Error_t   * error);

    typedef struct LL_Callback_t
    {
        LL_Callback     function;    /**< Function to invoke */
        void          * clientdata;  /**< Argument to function */
    }LL_Callback_t;

#define LL_REQUEST_NQUADS 32

    typedef struct Quad
    {
        unsigned w0; /**< Word[0] */
        unsigned w1; /**< Word[1] */
        unsigned w2; /**< Word[2] */
        unsigned w3; /**< Word[3] */
    }Quad __attribute__ ((__aligned__ (16)));

    typedef Quad LL_Request_t [LL_REQUEST_NQUADS];
    typedef enum {
        LL_EMPTY_TOPOLOGY = 0,	///< topology represents no (zero) ranks
        LL_SINGLE_TOPOLOGY,		///< topology is for one rank
        LL_RANGE_TOPOLOGY,		///< topology is a simple range of ranks
        LL_LIST_TOPOLOGY,		///< topology is an unordered list of ranks
        LL_COORD_TOPOLOGY,		///< topology is a rectangular segment
        ///  represented by coordinates
        LL_TOPOLOGY_COUNT
    } LL_TopologyType_t;


    typedef Quad LL_NetworkCoord_t[2];
    typedef Quad LL_PipeWorkQueue_t[4];
    typedef Quad LL_CollectiveProtocol_t [64*2];   /**< Opaque datatype for collective protocols */
    typedef Quad LL_CollectiveRequest_t  [32*8*4]; /**< Opaque datatype for collective requests */


    /* High level types */
    typedef Quad HL_CollectiveProtocol_t [64*2];   /**< Opaque datatype for collective protocols */
    typedef Quad HL_CollectiveRequest_t  [32*8*4]; /**< Opaque datatype for collective requests */

    typedef enum
    {
        HL_SUCCESS = 0,  /**< Successful execution        */
        HL_NERROR  =-1,  /**< Generic error (-1)          */
        HL_ERROR   = 1,  /**< Generic error (+1)          */
        HL_INVAL,        /**< Invalid argument            */
        HL_UNIMPL,       /**< Function is not implemented */
        HL_EAGAIN,       /**< Not currently availible     */
        HL_SHUTDOWN,     /**< Rank has shutdown           */
        HL_CHECK_ERRNO,  /**< Check the errno val         */
        HL_OTHER,        /**< Other undefined error       */
    }HL_Result;

    typedef LL_Callback_t HL_Callback_t;
    typedef LL_Dt HL_Dt;
    typedef LL_Op HL_Op;
    

#ifdef __cplusplus
};
#endif

#endif
