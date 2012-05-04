/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing,  DO NOT MODIFY OR MOVE                       */
/*  --------------------------------------------------------------- */
/* Licensed Materials - Property of IBM                             */
/* Blue Gene/Q 5765-PER 5765-PRP                                    */
/*                                                                  */
/* (C) Copyright IBM Corp. 2011, 2012 All Rights Reserved           */
/* US Government Users Restricted Rights -                          */
/* Use, duplication, or disclosure restricted                       */
/* by GSA ADP Schedule Contract with IBM Corp.                      */
/*                                                                  */
/*  --------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file common/lapiunix/lapierror.cc
/// \brief Error Handling Classes for LAPI routines.
///
/****************************************************************************
 Classes: LapiError

 Description: Error Handling Classes for LAPI routines.

 Author: Alan Jea

 History:
   Date     Who ID    Description
   -------- --- ---   -----------
   07/16/09 tjea      Initial code

****************************************************************************/

#include "lapierror.h"
#include <stdio.h>
#include <stdlib.h>
#include "lapifunc.h"

LapiError::LapiError(int rc, string file, int line, string func)
{
    char buf[LAPI_MAX_ERR_STRING];

    lapi_msg_string(rc, buf);
    this->err_code = rc;
    this->err_msg  = buf;
    this->file     = file;
    this->line     = line;
    this->function = func;
}

LapiError::~LapiError()
{
}
