/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file cm_types.h
 * \brief Common external collectives message layer types.
 */
#ifndef __cm_types_h__
#define __cm_types_h__

#include <stdint.h>
#include <stddef.h>

#include "cm_impl.h"

#ifdef __cplusplus
extern "C"
{
#endif

  // this probably should be platform-dependent... need PLATFORM/cm_types_impl.h
  typedef struct CMQuad
  {
    unsigned w0; /**< Word[0] */
    unsigned w1; /**< Word[1] */
    unsigned w2; /**< Word[2] */
    unsigned w3; /**< Word[3] */
  }
  CMQuad __attribute__ ((__aligned__ (16)));

  typedef enum
  {
    CM_SUCCESS = 0,  /**< Successful execution        */
    CM_NERROR  =-1,  /**< Generic error (-1)          */
    CM_ERROR   = 1,  /**< Generic error (+1)          */
    CM_INVAL,        /**< Invalid argument            */
    CM_UNIMPL,       /**< Function is not implemented */
    CM_EAGAIN,       /**< Not currently availible     */
    CM_SHUTDOWN,     /**< Rank has shutdown           */
    CM_CHECK_ERRNO,  /**< Check the errno val         */
    CM_OTHER,        /**< Other undefined error       */
  }
  CM_Result;

  /**
   * \brief Core Messaging Error callback results
   */
  typedef struct CM_Error_t
  {
    CM_Result result;
  }
  CM_Error_t;

  /**
   * \brief Message layer operation types
   */

  typedef enum
  {
    CM_UNDEFINED_OP = 0,
    CM_NOOP,
    CM_MAX,
    CM_MIN,
    CM_SUM,
    CM_PROD,
    CM_LAND,
    CM_LOR,
    CM_LXOR,
    CM_BAND,
    CM_BOR,
    CM_BXOR,
    CM_MAXLOC,
    CM_MINLOC,
    CM_USERDEFINED_OP,
    CM_OP_COUNT
  }
  CM_Op;

  /**
   * \brief Message layer data types
   */

  typedef enum
  {
    /* Standard/Primative DT's */
    CM_UNDEFINED_DT = 0,
    CM_SIGNED_CHAR,
    CM_UNSIGNED_CHAR,
    CM_SIGNED_SHORT,
    CM_UNSIGNED_SHORT,
    CM_SIGNED_INT,
    CM_UNSIGNED_INT,
    CM_SIGNED_LONG_LONG,
    CM_UNSIGNED_LONG_LONG,
    CM_FLOAT,
    CM_DOUBLE,
    CM_LONG_DOUBLE,
    CM_LOGICAL,
    CM_SINGLE_COMPLEX,
    CM_DOUBLE_COMPLEX,
    /* Max/Minloc DT's */
    CM_LOC_2INT,
    CM_LOC_SHORT_INT,
    CM_LOC_FLOAT_INT,
    CM_LOC_DOUBLE_INT,
    CM_LOC_2FLOAT,
    CM_LOC_2DOUBLE,
    CM_USERDEFINED_DT,
    CM_DT_COUNT
  }
  CM_Dt;


  /**
   * \brief Network type
   */
  typedef enum
  {
    CM_DEFAULT_NETWORK = 0, /**< Default network type. \b Guaranteed to work. */
    CM_TORUS_NETWORK,       /**< DEPRECATED! 3D-Torus / 1D-SMP network type. */
    CM_N_TORUS_NETWORK,     /**< nD-Torus / nD-SMP network type.
			     * mapping->numGlobalDims() for torus dim,
			     * mapping->numDims() for all (torus+SMP) dim.
			     */
    CM_SOCKET_NETWORK,      /**< Unix socket network type. */
    CM_SHMEM_NETWORK,       /**< local shared memory "network" for smp nodes. */
    CM_NETWORK_COUNT        /**< Number of network types defined. */
  }
  CM_Network;

  #define CM_MAX_DIMS	4

  /**
   * \brief A structure to describe a network coordinate
   */
  typedef struct CM_Coord_t
  {
    CM_Network network; /**< Network type for the coordinates */
    union
    {
      struct
      {
        size_t x; /**< Torus network x coordinate */
        size_t y; /**< Torus network y coordinate */
        size_t z; /**< Torus network z coordinate */
        size_t t; /**< Torus network t coordinate */
      } torus;    /**< obsolete: CM_TORUS_NETWORK coordinates */
      struct
      {
        size_t coords[CM_MAX_DIMS];
      } n_torus;
      struct
      {
        int recv_fd;   /**< Receive file descriptor */
        int send_fd;   /**< Send file descriptor    */
      } socket;   /**< CM_SOCKET_NETWORK coordinates */
      struct
      {
        size_t rank;   /**< Global rank of process */
        size_t peer;   /**< Local rank of process */
      } shmem;    /**< CM_SHMEM_NETWORK coordinates */
    };
  } CM_Coord_t;

  /* ********************************************************************* */
  /*                                                                       */
  /*               Data structures                                         */
  /*                                                                       */
  /* ********************************************************************* */

  /**
   * \brief Function signature of the completion callback function.
   *
   * \param[in] clientdata  Registered client data.
   * \param[in] error       Error result of the error event.
   */

  typedef void (*CM_Callback) (void         * clientdata,
                                 CM_Error_t * error);

  /**
   * \brief Completion callback information descriptor.
   *
   * Commonly used to signal completion of a particular operation to the
   * world outside the messaging library. These callbacks are invoked
   * during a call to Messager advance()
   */

  typedef struct CM_Callback_t
  {
    CM_Callback   function;    /**< Function to invoke */
    void          * clientdata;  /**< Argument to function */
  }
  CM_Callback_t;

typedef CMQuad CM_Protocol_t[CM_PROTOCOL_NQUADS];

typedef CMQuad CM_Request_t[CM_REQUEST_NQUADS];

#ifdef __cplusplus
};
#endif

#endif // __cm_types_h__
