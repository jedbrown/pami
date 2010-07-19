/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2010, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

////////////////////////////////////////////////////////////////////////////////
///
/// \file common/bgq/ResourceManager.h
///
/// \brief PAMI Resource Manager Definitions
///
/// The PAMI Resource Manager is a common class that controls resources
/// common to all PAMI components.
///
/// Current implementation uses environment variables to specify the resource
/// configuration.  Future implementation may use a file or some other means.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef __common_bgq_ResourceManager_h__
#define __common_bgq_ResourceManager_h__

#ifdef __FWEXT__

#include <firmware/include/fwext/fwext.h>
#include <firmware/include/Firmware.h>
#include <hwi/include/bqc/A2_core.h>
#include <hwi/include/bqc/A2_inlines.h>
#include <firmware/include/fwext/fwext_lib.h>
#include <firmware/include/fwext/fwext_nd.h>
#include <firmware/include/fwext/fwext_mu.h>

#endif // __FWEXT__

#include <stdio.h>
#include <stdlib.h>
#include <hwi/include/bqc/MU_Macros.h>
#include <spi/include/kernel/process.h>

#ifdef TRACE
#undef TRACE
#endif
#define TRACE(x) //fprintf x


namespace PAMI
{
  const size_t numSpiUserInjFifosPerProcessDefault = 0;
  const size_t numSpiUserRecFifosPerProcessDefault = 0;
  const size_t numClientsDefault         = 1;
  const char   defaultClientName[]       = "MPI";
  const size_t rgetInjFifoSizeDefault    = 65536;
  const size_t injFifoSizeDefault        = 65536;
  const size_t recFifoSizeDefault        = 1024*1024;

  class ResourceManager
  {
    public:

      //////////////////////////////////////////////////////////////////////////
      ///
      /// \brief PAMI Resource Manager Default Constructor
      ///
      /////////////////////////////////////////////////////////////////////////
      ResourceManager ()
	{
	  getConfig();

	} // End: ResourceManager Default Constructor


      inline size_t getNumSpiUserInjFifosPerProcess() { return _numSpiUserInjFifosPerProcess; }

      inline size_t getNumSpiUserRecFifosPerProcess() { return _numSpiUserRecFifosPerProcess; }

      inline size_t getNumClients()         { return _numClients; }

      inline const char *getClientName( size_t clientId ) { return _clientNamesPtrs[clientId]; }

      inline size_t getClientWeight( size_t clientId )    { return _clientWeights[clientId]; }

      inline size_t getRgetInjFifoSize() { return _rgetInjFifoSize; }

      inline size_t getInjFifoSize() { return _injFifoSize; }

      inline size_t getRecFifoSize() { return _recFifoSize; }

      inline size_t getNumProcessesPerNode() { return _numProcessesPerNode; }

      inline size_t getNumCoresPerProcess() { return _numCoresPerProcess; }

      inline size_t getNumProcessesPerCore() { return _numProcessesPerCore; }

      inline size_t getNumHWThreadsPerProcess() { return _numHWThreadsPerProcess; }

      inline size_t getNumHWThreadsPerProcessPerCore() { return _numHWThreadsPerProcessPerCore; }

    private:

      inline unsigned long getConfigValueSize_t( const char *configName,
						 size_t      defaultValue );

      inline char * getConfigValueString( const char *configName );


      inline void   getSubStringsFromConfigValueString( const char   *configName,
							const char   *configValueString,
							char        **configValueStringCopy,
							const char ***subStringConfigValuePtrs,
							size_t        numSubStringConfigValues,
							const char   *defaultSubStringConfigValue );
      
      inline void   getClientNames( const char *clientNamesConfigValueString );

      inline void   getClientWeights( const char *clientWeightsConfigValueString );

      inline void   getConfig();

      inline void   getProcessInfo();

      //////////////////////////////////////////////////////////////////////////
      ///
      /// Member data:
      ///
      /////////////////////////////////////////////////////////////////////////

      size_t       _numSpiUserInjFifosPerProcess;
      size_t       _numSpiUserRecFifosPerProcess;
      size_t       _numClients;
      char        *_clientNamesConfigValueStringCopy;
      const char **_clientNamesPtrs;
      size_t      *_clientWeights;
      size_t       _rgetInjFifoSize;
      size_t       _injFifoSize;
      size_t       _recFifoSize;
      size_t       _numProcessesPerNode;
      size_t       _numCoresPerProcess;
      size_t       _numProcessesPerCore;
      size_t       _numHWThreadsPerProcess;
      size_t       _numHWThreadsPerProcessPerCore;

  }; // ResourceManager class

};       // PAMI namespace


size_t PAMI::ResourceManager::getConfigValueSize_t( const char    *configName,
						    size_t  defaultValue )
{
  char *configValueString;
  unsigned long outputValue;

#ifdef __FWEXT__

  configValueString = fwext_getenv( configName );

#else // Not firmware extension

  configValueString = getenv( configName );

#endif

  if (configValueString)
    {

#ifdef __FWEXT__

      outputValue = fwext_strtoul( configValueString, 0, 10 );

#else

      outputValue = strtoul( configValueString, 0, 10 );

#endif

      TRACE((stderr,"PAMI ResourceManager: Overriding %s = %lu\n",configName,outputValue));
    }
  else
    {
      outputValue = defaultValue;
      TRACE((stderr,"PAMI ResourceManager: Using default for %s = %lu\n",configName,outputValue));
    }

  return (size_t)outputValue;

} // End: getConfigValueSize_t()

  
char * PAMI::ResourceManager::getConfigValueString( const char *configName )
{
  char *outputString;

#ifdef __FWEXT__

  outputString = fwext_getenv( configName );

#else // Not firmware extension

  outputString = getenv( configName );

#endif
  
  return outputString;

} // End: getConfigValueString()


void PAMI::ResourceManager::getSubStringsFromConfigValueString( 
						 const char   *configName,
						 const char   *configValueString,
						 char        **configValueStringCopy,
						 const char ***subStringConfigValuePtrs,
						 size_t        numSubStringConfigValues,
						 const char   *defaultSubStringConfigValue )
{
  char        *myConfigValueStringCopy = NULL;
  const char **mySubStringConfigValuePtrs;
  size_t       i;

  // Allocate space for _numClients pointers, to point to the client names.
  mySubStringConfigValuePtrs = (const char**)malloc( numSubStringConfigValues * sizeof( char* ) );
  PAMI_assertf( mySubStringConfigValuePtrs != NULL, "The heap is full.\n" );

  // If the env var is not specified, fill the pointers with the default value.
  if ( configValueString == NULL )
    {
      for (i=0; i<numSubStringConfigValues; i++)
	mySubStringConfigValuePtrs[i] = defaultSubStringConfigValue;
    }
  else // Env var specifies numSubStringConfigValues values
    {
      // Find out how long the env var string is, so we can make a copy of it
      unsigned int configValueStringLen = 0;
      while ( configValueString[configValueStringLen++] != '\0' );

      // Allocate space for a copy of the comma-delimited list of values
      myConfigValueStringCopy = (char*)malloc( configValueStringLen );
      PAMI_assertf( myConfigValueStringCopy != NULL, "The heap is full.\n");

      // Copy the env var string into our writeable copy
      for ( i=0; i<configValueStringLen; i++ )
	myConfigValueStringCopy[i] = configValueString[i];

      // First name is the start of the env var value
      mySubStringConfigValuePtrs[0] = myConfigValueStringCopy;

      char *currentChar = myConfigValueStringCopy;

      for ( i=1; i<=numSubStringConfigValues; i++ )
	{
	  // Scan the env var value for the next name (after the comma)
	  while ( ( *currentChar != ','  ) &&
		  ( *currentChar != '\0' ) )
	    currentChar++;

	  if ( *currentChar != '0' )
	    {
	      *currentChar = '\0'; // Change delimiter to NULL
	      currentChar++; // Skip past delimiter
	    }

	  if ( i<numSubStringConfigValues ) // Still expecting a name?
	    {
	      PAMI_assertf( *currentChar != '\0', "%s does not have enough names specified\n",configName );
	      mySubStringConfigValuePtrs[i] = currentChar;
	    }
	}
      *configValueStringCopy = myConfigValueStringCopy;
    }
  *subStringConfigValuePtrs = mySubStringConfigValuePtrs;

} // End: getSubStringsFromConfigValueString()


void PAMI::ResourceManager::getClientNames( const char *clientNamesConfigValueString )
{
  // If PAMI_CLIENTNAMES is not specified, then there must be only one client.
  if ( clientNamesConfigValueString == NULL )
    {
      PAMI_assertf( _numClients == 1, "Multiple clients requires PAMI_CLIENTNAMES to be specified\n" );
    }

  getSubStringsFromConfigValueString ( "PAMI_CLIENTNAMES",
				       clientNamesConfigValueString,
				       &_clientNamesConfigValueStringCopy,
				       &_clientNamesPtrs,
				       _numClients,
				       defaultClientName );
} // End: getClientNames()


void PAMI::ResourceManager::getClientWeights( const char *clientWeightsConfigValueString )
{
  size_t i;
  size_t sum;

  // Allocate space for _numClients weights.
  _clientWeights = (size_t*)malloc( _numClients * sizeof( size_t ) );
  PAMI_assertf( _clientWeights != NULL, "The heap is full.\n" );

  // If PAMI_CLIENTWEIGHTS is not specified, then the weights are evenly
  // distributed among the clients.  For example, 3 clients would be 34,33,33,
  // since the weights add up to 100, and the first few clients get 1 extra.
  if ( clientWeightsConfigValueString == NULL )
    {
      size_t defaultWeight = 100 / _numClients;
      sum = 0;
      for ( i=0; i<_numClients; i++ )
	{
	  _clientWeights[i] = defaultWeight;
	  sum += defaultWeight;
	}
      if ( sum < 100 )
	{
	  sum = 100 - sum; // Calculate amount left over
	  for ( i=0; (sum > 0) && (i<_numClients); i++ )
	    {
	      _clientWeights[i] += 1;
	      sum--;
	    }
	}
    }
  else
    {
      char *configValueStringCopy; // Temporary...not needed afterwards.
      const char **subStringConfigValuePtrs; // Temporary strings of weights.
                                             // Not needed afterwards.

      // Get substrings containing weights, one for each client.
      getSubStringsFromConfigValueString ( "PAMI_CLIENTWEIGHTS",
					   clientWeightsConfigValueString,
					   &configValueStringCopy,
					   &subStringConfigValuePtrs,
					   _numClients,
					   NULL );

      // Convert the substrings into size_t values.
      sum = 0;
      for (i=0; i<_numClients; i++)
	{
	  TRACE((stderr,"PAMI ResourceManager: Weight[%zu] = %s\n",i,subStringConfigValuePtrs[i]));
#ifdef __FWEXT__

	  _clientWeights[i] = (size_t)fwext_strtoul( subStringConfigValuePtrs[i], 0, 10 );

#else

	  _clientWeights[i] = (size_t)strtoul( subStringConfigValuePtrs[i], 0, 10 );

#endif
	  sum += _clientWeights[i];
	}

      PAMI_assertf ( sum == 100, "The PAMI_CLIENTWEIGHTS do not add up to 100.\n" );

      // Free temporary arrays.
      free( subStringConfigValuePtrs );
      free( configValueStringCopy    );
    }
} // End: getClientWeights()


void PAMI::ResourceManager::getProcessInfo()
{
  // Get number of processes per node.  This is the same on all nodes.
  _numProcessesPerNode = Kernel_ProcessCount();

  // Get number of cores per process.  This is the same on all processes.
  _numCoresPerProcess = 16 /* cores per node */ / _numProcessesPerNode;
  if ( _numCoresPerProcess == 0 ) _numCoresPerProcess = 1;

  // Get number of hardware threads per process.  This is the same on all processes.
  _numHWThreadsPerProcess = Kernel_ProcessorCount();

  _numProcessesPerCore = 4 /* HW threads per core */ / _numHWThreadsPerProcess;
  if ( _numProcessesPerCore == 0) _numProcessesPerCore = 1;

  // Get number of hardware threads per process per core.  This is the same on all cores
  // within a process.
  _numHWThreadsPerProcessPerCore = _numHWThreadsPerProcess / _numCoresPerProcess;

  TRACE((stderr,"PAMI ResourceManager: numProcessesPerNode = %zu, numCoresPerProcess = %zu, numProcessesPerCore = %zu, numHWThreadsPerProcess = %zu, numHWThreadsPerProcessPerCore = %zu\n",_numProcessesPerNode, _numCoresPerProcess, _numProcessesPerCore, _numHWThreadsPerProcess, _numHWThreadsPerProcessPerCore));

} // End: getProcessInfo()


void PAMI::ResourceManager::getConfig()
{
  char *clientNamesConfigValueString;
  char *clientWeightsConfigValueString;
  size_t i;

  // Get MUSPI_NUMINJFIFOS 
  _numSpiUserInjFifosPerProcess    = getConfigValueSize_t( "MUSPI_NUMINJFIFOS",
							   numSpiUserInjFifosPerProcessDefault );

  // Get MUSPI_NUMRECFIFOS
  _numSpiUserRecFifosPerProcess    = getConfigValueSize_t( "MUSPI_NUMRECFIFOS",
							   numSpiUserRecFifosPerProcessDefault );

  // Get PAMI_NUMCLIENTS
  _numClients            = getConfigValueSize_t( "PAMI_NUMCLIENTS",
						 numClientsDefault );

  TRACE((stderr,"PAMI ResourceManager: numSpiUserInjFifosPerProcess=%zu, numSpiUserRecFifosPerProcess=%zu, numClients=%zu\n",_numSpiUserInjFifosPerProcess,_numSpiUserRecFifosPerProcess,_numClients));

  PAMI_assertf( _numClients > 0, "PAMI_NUMCLIENTS must be greater than zero.\n" );

  // Get PAMI_CLIENTNAMES
  clientNamesConfigValueString = getConfigValueString( "PAMI_CLIENTNAMES" );

  getClientNames( clientNamesConfigValueString );

  // Get PAMI_CLIENTWEIGHTS
  clientWeightsConfigValueString = getConfigValueString( "PAMI_CLIENTWEIGHTS" );

  getClientWeights( clientWeightsConfigValueString );

  for (i=0; i<_numClients; i++)
    {
      TRACE((stderr,"PAMI ResourceManager: ClientName[%zu]=%s, Weight=%lu\n",i,_clientNamesPtrs[i],_clientWeights[i]));
    }

  // Get PAMI_RGETINJFIFOSIZE 
  _rgetInjFifoSize = getConfigValueSize_t( "PAMI_RGETINJFIFOSIZE",
					   rgetInjFifoSizeDefault );

  // Get PAMI_INJFIFOSIZE 
  _injFifoSize = getConfigValueSize_t( "PAMI_INJFIFOSIZE",
				       injFifoSizeDefault );

  // Get PAMI_RECFIFOSIZE 
  _recFifoSize = getConfigValueSize_t( "PAMI_RECFIFOSIZE",
				       recFifoSizeDefault );

  // Get Process Info
  getProcessInfo();

} // End: getConfig()


#undef TRACE

#endif   // __common_bgq_ResourceManager_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//