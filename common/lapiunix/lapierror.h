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
/**
 * \file common/lapiunix/lapierror.h
 * \brief Error Handling Classes for LAPI routines.
 */
/****************************************************************************
 Classes: LapiError

 Description: Error Handling Classes for LAPI routines.

 Author: Alan Jea

 History:
   Date     Who ID    Description
   -------- --- ---   -----------
   07/16/09 tjea      Initial code

****************************************************************************/

#ifndef __common_lapiunix_lapierror_h__
#define __common_lapiunix_lapierror_h__

#include <iostream>
#include <string>

using namespace std;

class LapiError
{
    public:
        int          err_code; // LAPI error code
        string       err_msg;  // error message
        string       file;     // file name where the error happened
        int          line;     // line number where the error happened
        string       function; // function name where the error happened

        LapiError(int rc, string file, int line, string func);
        ~LapiError();
};

inline ostream& operator<< (ostream& out, LapiError& e)
{
    out << "LAPI function failed in " << e.function << " ("
        << e.file << ":" << e.line << ") with rc=" << e.err_code << "\n";
    return out;
}

#define CheckLapiRC(stmt) \
{ \
    int rc = (stmt); \
    if (LAPI_SUCCESS != rc) { \
        LapiError e(rc, __FILE__, __LINE__, __func__); \
        cerr << e; \
    PAMI_abort(); \
    } \
}

#define CheckNULL(val,func_and_args)                                     \
    {                                                                     \
if ((val = (func_and_args)) == NULL) {                            \
    fprintf(stderr,"Aborting in %s : %s returns NULL\n",          \
    __FUNCTION__, #func_and_args );                       \
    abort();                                                      \
}                                                                 \
    }



#endif
