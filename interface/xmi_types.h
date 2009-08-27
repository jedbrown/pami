/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file xmi_types.h
 * \brief Common external collectives message layer types.
 */
#ifndef __xmi_types_h__
#define __xmi_types_h__

#include <stdint.h>
#include <stddef.h>

#include "xmi_impl.h"

#ifdef __cplusplus
extern "C"
{
#endif

  // this probably should be platform-dependent... need PLATFORM/xmi_types_impl.h
  typedef struct XMIQuad
  {
    unsigned w0; /**< Word[0] */
    unsigned w1; /**< Word[1] */
    unsigned w2; /**< Word[2] */
    unsigned w3; /**< Word[3] */
  }
  XMIQuad __attribute__ ((__aligned__ (16)));

  typedef enum
  {
    XMI_SUCCESS = 0,  /**< Successful execution        */
    XMI_NERROR  =-1,  /**< Generic error (-1)          */
    XMI_ERROR   = 1,  /**< Generic error (+1)          */
    XMI_INVAL,        /**< Invalid argument            */
    XMI_UNIMPL,       /**< Function is not implemented */
    XMI_EAGAIN,       /**< Not currently availible     */
    XMI_SHUTDOWN,     /**< Rank has shutdown           */
    XMI_CHECK_ERRNO,  /**< Check the errno val         */
    XMI_OTHER,        /**< Other undefined error       */
  }
  XMI_Result;

  /**
   * \brief Core Messaging Error callback results
   */
  typedef struct XMI_Error_t
  {
    XMI_Result result;
  }
  XMI_Error_t;

  /**
   * \brief Message layer operation types
   */

  typedef enum
  {
    XMI_UNDEFINED_OP = 0,
    XMI_NOOP,
    XMI_MAX,
    XMI_MIN,
    XMI_SUM,
    XMI_PROD,
    XMI_LAND,
    XMI_LOR,
    XMI_LXOR,
    XMI_BAND,
    XMI_BOR,
    XMI_BXOR,
    XMI_MAXLOC,
    XMI_MINLOC,
    XMI_USERDEFINED_OP,
    XMI_OP_COUNT
  }
  XMI_Op;

  /**
   * \brief Message layer data types
   */

  typedef enum
  {
    /* Standard/Primative DT's */
    XMI_UNDEFINED_DT = 0,
    XMI_SIGNED_CHAR,
    XMI_UNSIGNED_CHAR,
    XMI_SIGNED_SHORT,
    XMI_UNSIGNED_SHORT,
    XMI_SIGNED_INT,
    XMI_UNSIGNED_INT,
    XMI_SIGNED_LONG_LONG,
    XMI_UNSIGNED_LONG_LONG,
    XMI_FLOAT,
    XMI_DOUBLE,
    XMI_LONG_DOUBLE,
    XMI_LOGICAL,
    XMI_SINGLE_COMPLEX,
    XMI_DOUBLE_COMPLEX,
    /* Max/Minloc DT's */
    XMI_LOC_2INT,
    XMI_LOC_SHORT_INT,
    XMI_LOC_FLOAT_INT,
    XMI_LOC_DOUBLE_INT,
    XMI_LOC_2FLOAT,
    XMI_LOC_2DOUBLE,
    XMI_USERDEFINED_DT,
    XMI_DT_COUNT
  }
  XMI_Dt;


  /**
   * \brief Network type
   */
  typedef enum
  {
    XMI_DEFAULT_NETWORK = 0, /**< Default network type. \b Guaranteed to work. */
    XMI_TORUS_NETWORK,       /**< DEPRECATED! 3D-Torus / 1D-SMP network type. */
    XMI_N_TORUS_NETWORK,     /**< nD-Torus / nD-SMP network type.
			     * mapping->numGlobalDims() for torus dim,
			     * mapping->numDims() for all (torus+SMP) dim.
			     */
    XMI_SOCKET_NETWORK,      /**< Unix socket network type. */
    XMI_SHMEM_NETWORK,       /**< local shared memory "network" for smp nodes. */
    XMI_NETWORK_COUNT        /**< Number of network types defined. */
  }
  XMI_Network;

  /**
   * \brief A structure to describe a network coordinate
   */
  typedef struct XMI_Coord_t
  {
    XMI_Network network; /**< Network type for the coordinates */
    union
    {
      struct
      {
        size_t x; /**< Torus network x coordinate */
        size_t y; /**< Torus network y coordinate */
        size_t z; /**< Torus network z coordinate */
        size_t t; /**< Torus network t coordinate */
      } torus;    /**< obsolete: XMI_TORUS_NETWORK coordinates */
      struct
      {
        size_t coords[XMI_MAX_DIMS];
      } n_torus;
      struct
      {
        int recv_fd;   /**< Receive file descriptor */
        int send_fd;   /**< Send file descriptor    */
      } socket;   /**< XMI_SOCKET_NETWORK coordinates */
      struct
      {
        size_t rank;   /**< Global rank of process */
        size_t peer;   /**< Local rank of process */
      } shmem;    /**< XMI_SHMEM_NETWORK coordinates */
    };
  } XMI_Coord_t;

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

  typedef void (*XMI_Callback) (void         * clientdata,
                                 XMI_Error_t * error);

  /**
   * \brief Completion callback information descriptor.
   *
   * Commonly used to signal completion of a particular operation to the
   * world outside the messaging library. These callbacks are invoked
   * during a call to Messager advance()
   */

  typedef struct XMI_Callback_t
  {
    XMI_Callback   function;    /**< Function to invoke */
    void          * clientdata;  /**< Argument to function */
  }
  XMI_Callback_t;

typedef XMIQuad XMI_Protocol_t[XMI_PROTOCOL_NQUADS];

typedef XMIQuad XMI_Request_t[XMI_REQUEST_NQUADS];

typedef void XMI_dgsp_t;	// temporary...

#ifdef __cplusplus
};
#endif

#endif // __xmi_types_h__
