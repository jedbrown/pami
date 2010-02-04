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

