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
 * \file api/extension/c/collsel/AdvisorTable.h
 */
#ifndef __api_extension_c_collsel_AdvisorTable_h__
#define __api_extension_c_collsel_AdvisorTable_h__

#include "api/extension/c/collsel/CollselExtension.h"

namespace PAMI{

class AdvisorTable
{
public:
  AdvisorTable(Advisor &advisor);
  pami_result_t generate(char             *filename,
                         advisor_params_t *params,
                         int               mode);
  pami_result_t load(char* filename);
  pami_result_t unload();
  ~AdvisorTable();
private:
  Advisor &_advisor;
};


inline AdvisorTable::AdvisorTable(Advisor &advisor):
  _advisor(advisor)
{
  
  
  
}

inline AdvisorTable::~AdvisorTable()
{
  
  
}

inline pami_result_t AdvisorTable::generate(char             *filename,
                                            advisor_params_t *params,
                                            int               mode)
{
  
  return PAMI_SUCCESS;
}

inline pami_result_t AdvisorTable::load(char* filename)
{
  
  return PAMI_SUCCESS;
}

inline pami_result_t AdvisorTable::unload()
{
  
  return PAMI_SUCCESS;
}
}

#endif // __api_extension_c_collsel_Extension_h__
