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
  AdvisorTable(Advisor &advisor)
  {
  }
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
  
  
}

inline pami_result_t AdvisorTable::load(char* filename)
{
  
  
}

inline pami_result_t AdvisorTable::unload()
{
  
  
}


#endif // __api_extension_c_collsel_Extension_h__
