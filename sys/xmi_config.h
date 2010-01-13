/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file sys/xmi_config.h
 * \brief ???
 */

#ifndef __xmi_config_h__
#define __xmi_config_h__

#define PLATFORM_DUMMY
#ifdef PLATFORM_DUMMY

// Todo:  remove this from the interface
// It is not needed by the

#define XMI_PROTOCOL_NQUADS	48
#define XMI_REQUEST_NQUADS	512
typedef struct xmi_quad_t
{
    unsigned w0; /**< Word[0] */
    unsigned w1; /**< Word[1] */
    unsigned w2; /**< Word[2] */
    unsigned w3; /**< Word[3] */
}
xmi_quad_t __attribute__ ((__aligned__ (16)));
#endif

#ifdef PLATFORM_SOCKETS

#endif

#ifdef PLATFORM_BGP

#endif

#ifdef PLATFORM_BGQ

#endif

#ifdef PLATFORM_LAPIUNIX

#endif

#ifdef PLATFORM_MPI

#endif


#endif /* __xmi_config_h__ */
