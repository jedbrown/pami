/**
 * \file xmi_types.h
 * \brief Common external collectives message layer types.
 */
#ifndef __xmi_types_h__
#define __xmi_types_h__

#include <stdint.h>
#include <stddef.h>
#include "xmi_config.h"
/* #include "xmi_impl.h" */

#ifdef __cplusplus
extern "C"
{
#endif
  /**
   * \brief XMI result status codes.
   */
  typedef enum
  {
    XMI_SUCCESS = 0,  /**< Successful execution        */
    XMI_NERROR  = -1, /**< Generic error (-1)          */
    XMI_ERROR   = 1,  /**< Generic error (+1)          */
    XMI_INVAL,        /**< Invalid argument            */
    XMI_UNIMPL,       /**< Function is not implemented */
    XMI_EAGAIN,       /**< Not currently availible     */
    XMI_SHUTDOWN,     /**< Rank has shutdown           */
    XMI_CHECK_ERRNO,  /**< Check the errno val         */
    XMI_OTHER,        /**< Other undefined error       */
  }
  xmi_result_t;

  typedef void* xmi_client_t;   /**< ??? */
  typedef void* xmi_context_t;  /**< ??? */
  typedef void* xmi_hint_t;     /**< ??? */
  typedef void* xmi_dispatch_t; /**< ??? */
  typedef void* xmi_type_t;     /**< ??? */

  /**
   * \brief Callback to handle message events
   *
   * \param[in] context   XMI communication context that invoked the callback
   * \param[in] cookie    Event callback application argument
   * \param[in] result    Asynchronous result information (was error information)
   */
  typedef void (*xmi_event_function) ( xmi_context_t   context,
                                       void          * cookie,
                                       xmi_result_t    result );

  typedef struct xmi_callback_t
  {
    xmi_event_function  function;
    void               *clientdata;
  }xmi_callback_t;




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
  xmi_op;

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
  xmi_dt;


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
  xmi_network;

  #define XMI_MAX_DIMS	4

  /**
   * \brief A structure to describe a network coordinate
   */
  typedef struct xmi_coord_t
  {
    xmi_network network; /**< Network type for the coordinates */
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
  } xmi_coord_t;



#ifdef __cplusplus
};
#endif

#endif /* __xmi_types_h__ */
