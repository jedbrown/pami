/**
 * \file api/extension/c/collsel/Advisor.h
 */
#ifndef __api_extension_c_collsel_Advisor_h__
#define __api_extension_c_collsel_Advisor_h__

#include "api/extension/c/collsel/CollselExtension.h"

namespace PAMI{
class Advisor
{
public:
  Advisor(pami_client_t            client,
          advisor_configuration_t  configuration[],
          size_t                   num_configs,
          pami_context_t           contexts[],
          size_t                   num_contexts);
  ~Advisor();
private:
  pami_client_t            _client;
  advisor_configuration_t *_configuration;
  size_t                   _num_configs;
  pami_context_t          *_contexts;
  size_t                   _num_contexts;

};


inline Advisor::Advisor(pami_client_t            client,
                        advisor_configuration_t  configuration[],
                        size_t                   num_configs,
                        pami_context_t           contexts[],
                        size_t                   num_contexts):
  _client(client),
  _configuration(configuration),
  _num_configs(num_configs),
  _contexts(contexts),
  _num_contexts(num_contexts)
{
  
  
  
  
}

inline Advisor::~Advisor()
{
  
  
}

} // Nnamespace pami
#endif // __api_extension_c_collsel_Extension_h__
