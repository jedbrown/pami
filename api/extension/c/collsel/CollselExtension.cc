#include "api/extension/c/collsel/CollselExtension.h"
#include "api/extension/c/collsel/Advisor.h"
#include "api/extension/c/collsel/AdvisorTable.h"

namespace PAMI{

pami_result_t Collsel_init_fn(pami_client_t            client,
                              advisor_configuration_t  configuration[],
                              size_t                   num_configs,
                              pami_context_t           contexts[],
                              size_t                   num_contexts,
                              advisor_t               *advisor)
{
  advisor = (advisor_t*)new Advisor(client,configuration,num_configs,
                                    contexts,num_contexts);
  return PAMI_SUCCESS;
}

pami_result_t Collsel_destroy_fn(advisor_t *advisor)
{
  Advisor *deleteme = (Advisor*)advisor;
  delete deleteme;
  return PAMI_SUCCESS;
}

pami_result_t Collsel_table_generate_fn(advisor_t         advisor,
                                        char             *filename,
                                        advisor_params_t *params,
                                        int               mode)
{
  Advisor       *a  = (Advisor*) advisor;
  AdvisorTable  *at = new AdvisorTable(*a);
  return at->generate(filename,params, mode);
}


pami_result_t Collsel_table_load_fn(advisor_t        advisor,
                                    char            *filename,
                                    advisor_table_t *advisor_table)
{
  Advisor      *a  = (Advisor*) advisor;
  AdvisorTable *at = new AdvisorTable(*a);
  *advisor_table   = (advisor_table_t) at;
  return at->load(filename);
}


pami_result_t Collsel_table_unload_fn(advisor_table_t *advisor_table)
{
  AdvisorTable *at = (AdvisorTable*) *advisor_table;
  *advisor_table = NULL;
  return   at->unload();
}

};//namespace PAMI
