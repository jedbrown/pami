#!/bin/bash

#-------------------------------------------------------------------------------
# Prerequisites:
#-------------------------------------------------------------------------------
# Source code for all exes was checked out/updated manually
# See SST Wiki:
#    https://bgweb.rchland.ibm.com/bgq/wiki/index.php/HOWTO_use_Subversion#Access_From_Rochester_Servers
# Run ../bgq/scripts.configure
#
# Source code for bgtools was checked out/updated manually:
#    From bgq dir:  svn update system_tests/tools
# source bgtools setup file (see Installation in ../bgq/system_tests/tools/bgtools/README)
# Additional bgtools documentation can be found at the following wiki:
#    BGQ: https://bgweb.rchland.ibm.com/bgq/wiki/index.php/BlueGene_User_Utilities
#-------------------------------------------------------------------------------

# HASH TABLE INIT
#hinit ()
#{
#    if [ -e /tmp/hashmap.$1 ]
#	then
#	rm -f /tmp/hashmap.$1
#    fi
#}

# HASH ADD KEY/VALUE PAIR
hput ()
{

    # $1 = file name
    # $2 = key
    # $3 = value

    hvalue=''

    # If file exists, see if key already exists
    if [ -e $1 ] 
	then 
#	hvalue=\"$( grep --max-count=1 "^${2} " $1 | awk '{ print substr($0, (index($0," ")+1)) }' )\"
	hget $1 $2 hvalue
	hvalue=$( echo $hvalue | sed 's/"//g' )

    else # create new hash file
	echo ${2} ${3} > $1
	return 0
    fi

    # Append or replace key/value pair
    if [ "${hvalue}" == '' ] # key doesn't exist 
	then 
	echo ${2} ${3} >> $1
    else
	# replace old value with new one
	# Use "," as the delimiter so we don't have to escape all "/"
	sed -i "s,${2} ${hvalue},${2} ${3}," $1
    fi

    return 0
}

# HASH GET VALUE 
hget () 
{
    # $1 = file name
    # $2 = key
    # $3 = variable to hold value

    eval $3=\"$( grep --max-count=1 "^${2} " $1 | awk '{ print substr($0, (index($0, " ")+1)) }' )\"

    if [ $? -ne 0 ]
	then
	echo "ERROR (E)::hget:  ${2} key DNE in ${1}"
	return -1
    fi
}

hdelete ()
{
    sed -i "/${2}/d" $1
}

hdestroy ()
{
    if [ -e $1 ]
	then
	rm -f $1
    else
	echo "ERROR (E)::hdestroy: ${1} DNE !!"
	return 1
    fi

    return 0
}

cleanExit ()
{
    if [ -e $compileHash ]
	then
	rm -f $compileHash
    fi

    if [ -e $copyHash ]
	then
	rm -f $copyHash
    fi

    if [ -e $exeHash ]
	then
	rm -f $exeHash
    fi

    exit $1
}

#-------------------------------------------------------------------------------
# Translate BGP mode input parms from Text values to number values
# $1 - mode text 
# $2 - var to hold numerical mode value
#-------------------------------------------------------------------------------
bgp_mode_TexttoNum ()
{

    alphaMode=$1
    numModeVar=$2

    TtN_rc=0

    # Turn on case-insensitive matching (-s set nocasematch)
    shopt -s nocasematch

    case $alphaMode in
	smp )
	    eval $numModeVar=1
	    ;;
	dual )
	    eval $numModeVar=2
	    ;;
	vn )
	    eval $numModeVar=4
	    ;;
	* )                     
	    echo "ERROR (E)::bgp_mode_TexttoNum: Unrecognized mode: ${alphaMode}"
	    echo "ERROR (E)::bgp_mode_TexttoNum: Valid values are:  SMP, DUAL & VN"
	    TtN_rc=1
    esac

    # Turn off case-insensitive matching (-u unset nocasematch)
    shopt -u nocasematch

    return $TtN_rc
}

#-------------------------------------------------------------------------------
# Translate BGP mode input parms from Text values to number values
# $1 - numerical mode 
# $2 - var to hold text mode value
#-------------------------------------------------------------------------------
bgp_mode_NumtoText ()
{

    digiMode=$1
    textModeVar=$2

    NtT_rc=0

    case $digiMode in
	1 )
	    eval $textModeVar='SMP'
	    ;;
	2 )
	    eval $textModeVar='DUAL'
	    ;;
	4 )
	    eval $textModeVar='VN'
	    ;;
	* )                     
	    echo "ERROR (E)::bgp_mode_NumtoText: Unrecognized mode: ${digiMode}"
	    echo "ERROR (E)::bgp_mode_NumtoText: Valid values are:  1, 2 & 4"
	    NtT_rc=1
    esac

    return $NtT_rc
}

#-------------------------------------------------------------------------------
# runHW input parms
# $1 - The working directory for the test
# $2 - The executable/binary file to run
# $3 - Test scenario:  nodes, mode, NP
# $4 - The options to pass to mpirun/runjob
# $5 - The arguments to pass to the executable
# $6 - The log file for this run
# $7 - The variable to store the exe return code for this test
# $8 - The variable to store the elapsed time for this test
#-------------------------------------------------------------------------------
runHW ()
{
    # Get the arguments
    cwd=$1
    exe=$2
    HWNodes=$( echo $3 | awk '{print $1}' )
    HWMode=$( echo $3 | awk '{print $2}' )
    HWProcs=$( echo $3 | awk '{print $3}' )
    opts=$4
    args=$5
    logFile=$6
    HWSignalVar=$7
    elapsedTimeVar=$8

    runfctest="${run_floor}/scripts/runfctest.sh"
    type='MMCS'
    
    if [[ "${exe}" =~ 'threadTest_omp' ]]
	then
	threads=16
    else
	threads=1
    fi

    runHW_rc=0

    declare -a ENV_VAR_ARRAY          # array of ENV VAR names
    declare -a ENV_VAL_ARRAY          # array of new ENV VAR values
    declare -a ENV_PREVAL_ARRAY       # array of PREvious ENV VAR VALues

    element=0
    env_name=""
    env_val=""

    # Parse mpirun/runjob options
    # Determine which ENV vars need to be saved, updated and documented
    set - $(echo "${opts}")

    while [ "${1}" != "" ]; do
	case $1 in
	    -env | --envs )           shift                  
		                      while [[ "${1}" =~ '=' ]];do
					  env_name=$( echo $1 | cut -d '=' -f1 | sed 's/"//g' )
					  env_val=$( echo $1 | cut -d '=' -f2 | sed 's/"//g' )
					  
					  # Update environment array
					  if [ "${env_name}" != 'BG_PROCESSESPERNODE' ] && [ "${env_name}" != 'MPIRUN_MODE' ]
					      then

					      ENV_VAR_ARRAY[$element]=$env_name
					      ENV_VAL_ARRAY[$element]=$env_val
					      ENV_PREVAL_ARRAY[$element]=$( eval echo \$${env_name} )

                                              # Look for OMP threads
					      if [ "${env_name}" == 'OMP_NUM_THREADS' ]
						  then
						  threads=$env_val
					      fi

                                              # Increment element 
					      element=$(( $element + 1 ))
					  fi                                          
					  shift
				      done
				      ;;
	    * )                       # Goto the next value
		                      shift
	esac
#	shift
    done

    # Look for number of threads in exe args
    set - $(echo "${args}" | sed 's/"//g')

    while [ "${1}" != "" ]; do
	case $1 in
	    --numPthreads )                  shift 
		                             threads=$((1 + $1)) # total num threads = main thread + $1 Pthreads = 1 + $1
					     ;;
	    -n )                             shift
		                             if [[ "${exe}" =~ 'threadTest' ]]
						 then
						 threads=$1 # for threadTests, total num threads = main thread + ($1 - 1) Pthreads = $1   
					     fi
					     ;;
        * ) #continue                    
	esac
	shift
    done

    # Check for mpirun (BGP) parms that we don't want to fill the input file with: 
    if [ "${platform}" == 'BGP' ]
	then
        # nofree
	if [[ ! "{$opts}" =~ '-nofree' ]]
	    then
	    opts="${opts} -nofree"
	fi

        # label
	if [[ ! "${opts}" =~ '-label' ]]
	    then
	    opts="${opts} -label"
	fi
    fi

    # Check for runjob (BGQ) parms that we don't want to fill the input file with: 
    if [ "${platform}" == 'BGQ' ]
	then    
        # ttype
	if [[ ! "${opts}" =~ '--ttype' ]]
	    then
	    opts="${opts} --ttype cnk"
	fi

        # noobjdump
	if [[ ! "${opts}" =~ '--noobjdump' ]]
	    then
	    opts="${opts} --noobjdump"
	fi

        # numnodes
	if [[ ! "${opts}" =~ '--numnodes' ]]
	    then
	    opts="${opts} --numnodes ${HWNodes}"
	fi

        # svchost
	if [[ ! "${opts}" =~ '--svchost' ]] 
	    then
	    opts="${opts} --svchost ${abs_test_dir}/svchost.pers"
	fi
    fi

    # Save off the current directory
    curDir=`pwd`

    # Echo useful info and ENV VARs to logfile
    {
	echo "NP = ${HWProcs}"
	echo "THREADS = ${threads}"
	echo "NODES = ${HWNodes}"
	if [ "${platform}" == 'BGP' ]
	    then
	    echo "MODE = ${HWMode}"
	else
	    #????
	    echo "BG_PROCESSESPERNODE = ${BG_PROCESSESPERNODE}"
	    echo "PAMI_DEVICE = ${PAMI_DEVICE}"
	    echo "BG_MEMSIZE = ${BG_MEMSIZE}"
	    echo "BG_SHAREDMEMSIZE = ${BG_SHAREDMEMSIZE}"
	fi
    } >> $logFile

    for ((var=0; var < ${#ENV_VAR_ARRAY[@]}; var++))
      do
      #????
      if [ "${ENV_VAR_ARRAY[$var]}" != 'BG_PROCESSESPERNODE' ] && [ "${ENV_VAR_ARRAY[$var]}" != 'PAMI_DEVICE' ] && [ "${ENV_VAR_ARRAY[$var]}" != 'BG_MEMSIZE' ] && [ "${ENV_VAR_ARRAY[$var]}" != 'BG_SHAREDMEMSIZE' ]
	  then
	  echo "${ENV_VAR_ARRAY[$var]} = ${ENV_VAL_ARRAY[$var]}" >> $logFile 
      fi
    done

    echo "SERVER ID = ${serverID}" >> $logFile

    echo ""

    # Change to the working directory and run the test
    changeDir $cwd

    if [ $? -ne 0 ]
	then
	echo "ERROR (E)::runHW: cd to exe dir FAILED!! Skipping to the next test ..."
	return 1
    fi

    # Create platform-specific run command
    if [ "${platform}" == 'BGP' ]
	then # Create BGP run command
	runCmd="mpirun -partition ${block} -np ${HWProcs} -mode ${HWMode} -cwd ${cwd} -exe ${exe} ${opts} -args \"${args}\""
    else # Create BGQ run command
        # Remove "'s from args string if they exist
#         args=$( echo $args | sed 's/"//g' ) 
	runCmd="${runfctest} --script ${type} ${opts} --location ${block} --program ${exe} -- ${args}"
    fi

    # Start time in Epoch time
    before=$(date +%s)

    # Make sure we don't lose any error return codes due to piping
#    set -o pipefail

    if [ $quietly -eq 1 ]
	then
	echo $runCmd >> $logFile
	if [ $debug -eq 0 ]
	    then
	    eval $runCmd >> $logFile
	    runStatus=($( echo ${PIPESTATUS[*]} ))
	fi
    else
	echo $runCmd | tee -a $logFile
	if [ $debug -eq 0 ]
	    then
	    eval $runCmd | tee -a $logFile
	    runStatus=($( echo ${PIPESTATUS[*]} ))
	fi
    fi

    # End time in Epoch time
    after=$(date +%s)

    # Run time in readable format
    elapsed_seconds="$((${after} - ${before}))" # Needs to be a string
    eval $elapsedTimeVar=$(date -d "1970-01-01 ${elapsed_seconds seconds}" +%T)

    if [ $debug -eq 1 ]
	then
	if [ "${platform}" == 'BGQ' ]
	    then
	    for ((i=0; i < ${HWNodes}; i++))
	      do
	      echo 'Software Test PASS. 0' >> $logFile
	    done
	fi

	declare -a runStatus; runStatus[0]=0; runStatus[1]=0
    fi

    eval $HWSignalVar=${runStatus[0]}

    # Check status of runfctest
    if [ ${runStatus[0]} -ne 0 ]
	then
	echo "ERROR (E)::runHW: Execution of ${exe} FAILED!!"
	runHW_rc=${runStatus[0]}
    else # runCmd passed, let's check the logFile
	if [ $quietly -eq 0 ]
	    then
	    if [ ${runStatus[1]} -ne 0 ] || [ ! -e $logFile ]
		then
		echo "ERROR (E)::runHW: FAILED to tee output into ${logFile}!!"
		runHW_rc=${runStatus[1]}
	    fi
	fi
    fi

    # Verify success/failure of binary itself for runMambo and runMmcsLite runs    
    if [ "${platform}" == 'BGQ' ] && [ $runHW_rc -eq 0 ]
	then
	$(tail -10 $logFile | grep -q 'Software Test PASS')
       
	runHW_rc=$?
    fi

    # Restore ENV VARs
    for ((var=0; var < ${#ENV_VAR_ARRAY[@]}; var++))
      do
      export ${ENV_VAR_ARRAY[$var]}=${ENV_PREVAL_ARRAY[$var]}
    done

    # Switch back to the previous directory
    changeDir $curDir

    if [ $? -ne 0 ]
	then
	echo "ERROR (E)::runHW: cd back to original dir FAILED!! Exiting."
	cleanExit 1
    fi

    # Return
    return $runHW_rc
}

#-------------------------------------------------------------------------------
# runHW input parms
# $1 - The working directory for the test
# $2 - The executable/binary file to run
# $3 - The options to pass to runjob
# $4 - The arguments to pass to the executable
# $5 - The log file
#-------------------------------------------------------------------------------
runHWRoy ()
{
    # Get the arguments
    cwd=$1
    exe=$2
    opts=$3
    args=$4
    logFile=$5

    runHW_rc=0

    # runjob Options:
    #  --exe arg                  executable to run
    #  --args arg                 arguments
    #  --envs arg                 environment variables in key=value form
    #  --exp_env arg              export a specific environment variable
    #  --env_all                  export all environment variables
    #  --cwd arg                  current working directory
    #  --timeout arg              number of seconds to wait after job starts before 
    #                                it is killed
    #  --block arg                block ID
    #  --location arg             specific nodes for midplane or smaller jobs, see 
    #                                below
    #  --ranks arg (=1)           number of ranks per node: 1, 2, 4, 8, 16, 32, or 
    #                                64
    #  --np arg                   number of ranks in the entire job
    #  --mapping arg              ABCDET permutation or path to mapping file
    #  --label                    prefix job output with stdout, stderr, and rank  
    #                                labels
    #  --strace arg               specify all, none, or n where n is a rank to 
    #                                enable system call tracing
    #  --start_tool arg           path to tool to start with the job
    #  --tool_args arg            arguments for the tool
    #  --socket arg (=runjob_mux) runjobmux listen socket
    #  --enable_tty_reporting     disable default line buffering of standard I/O if 
    #                                not attached to a tty
    #  --stdinrank arg (=0)       rank to send stdin to
    #  --raise                    if the job dies with a signal, raise it
    #  -h [ --help ]              this help text
    #  -v [ --version ]           display version information
    #  --properties arg           Blue Gene configuration file
    #  --verbose arg              Logging configuration

    # --- Set any misc runjob options ---
#    opts=""
#    opts="$opts --ranks 1"	# ranks per node { 1, 2, 4, 8, 16, 32, or 64 }
#    opts="$opts --np 1"	# total ranks 
#    opts="$opts --label"	# 
    #opts="$opts --mapping EDCBA" 
    #opts="$opts --timeout 30"
    #opts="$opts --start_tool /bgsys/drivers/ppcfloor/bin/gdbserver-bgp"
#    opts="$opts ${MPIOPTS}"

    if [[ ! "${opts}" =~ '--envs' ]] || [[ ! "${opts}" =~ '--exp_env' ]] || [[ ! "${opts}" =~ '--env_all' ]]
	then 
	opts="${opts} --env_all"
    fi

    # --- Set any environment variables ---
#    env=""
#    env="$env OMP_NUM_THREADS=1"
    #env="$env BG_COREDUMPONEXIT=1"
    #env="$env BG_MAPPING=TXYZ"
    #env="$env LD_DEBUG=files,libs"        # prints the starting address of each library 
                                           #    as they are dynamically loaded.
    #env="$env BG_STATICTLB=0"
    #env="$env DCMF_COLLECTIVES=0"         # use this to disable optimized collectives
    #env="$env DCMF_BCAST=B"               # broadcast: B=binomial R=retangular
    #env="$env DCMF_EAGER=1000000000       # Eager/Rendezvous protocol threashold.. 
    #env="$env DCMF_INTERRUPTS=1"          # =1 to enable message passing interrupt mode 
    #env="$env DCMF_STATISTICS=1 DCMF_VERBOSE=2"   # print receive queue stats.   
    #env="$env TRACE_ALL_TASKS=yes"        # mpitrace option 
    #env="$env TRACE_SEND_PATTERN=yes"     # mpitrace option

#    if [[ A"" != A"$env" ]]; then env="--envs \"$env\""; fi

    # include some utility functions
    if [ A"" = A`type -P bgutils` ]; then         
	echo "ERROR (E):  The bgutils file is not found in your PATH."  
	echo "ERROR (E):  Please install the bgtools package available from SVN bgq/system_tests/tools/bgtools."

	#ding, ding, ding
	if [[ A1 = A$ding ]]; then echo -en "\007"; sleep 1; echo -en "\007"; sleep 1; echo -en "\007"; fi
	
	cleanExit 1
    fi

    source bgutils

    # Save off the current directory
    curDir=`pwd`

    # Make sure we don't lose any error return codes due to piping
    set -o pipefail

    # Change to the working directory and run the test
    changeDir $cwd

    if [ $? -ne 0 ]
	then
	echo "ERROR (E)::runHW: cd to exe dir FAILED!!"
	return 1
    fi

    # assemble a runjob command 
    if [ A != A$sub_block ]; then sbloc="--location $block_name"; fi

    #  

    rjcmd="runjob --block $block $sbloc --verbose ibm.runjob=INFO $opts --cwd `pwd -P` --exe $exe --args $args"

    # define a name that will help identify this run, use exe name, input args, modes etc.
    run_name=$exe	

    if [ $debug -eq 1 ]
	then
	echo "runjob_wrapper ${exe} $rjcmd"
	jobid="dummy"
	ExitStatus=0
    else
	runjob_wrapper ${run_name} $rjcmd
    fi

    runHW_rc=$ExitStatus

    # Verify output from runjob_wrapper exists
    runjob_out="run.${run_name}.$jobid.out"

    if [ $debug -eq 1 ]
	then
	echo "dummy runjob of $exe PASSED" > $runjob_out
    fi

    if [ -e "${runjob_out}" ]
	then
	cp "${runjob_out}" "${logFile}" 

	if [ $? -ne 0 ] || [ ! -e "${logFile}" ]
	    then
	    echo "ERROR (E)::runHW: Copy of $runjob_out to $logFile FAILED!!"
	    runHW_rc=1
	fi
    else
	echo "ERROR (E)::runHW: $runjob_out DNE!!"
	runHW_rc=1
    fi	

    # Verify pass/fail of binary

    # Restore ENV VARs
    for ((var=0; var < ${#ENV_VAR_ARRAY[@]}; var++))
      do
      export ${ENV_VAR_ARRAY[$var]}=${ENV_VAL_ARRAY[$var]}
    done

    # Switch back to the previous directory
    changeDir $curDir

    if [ $? -ne 0 ]
	then
	echo "ERROR (E)::runHW: cd back to original dir FAILED!! Exiting."
	cleanExit 1
    fi

    # Return code
    return $runHW_rc
}

#-------------------------------------------------------------------------------
# exe_preProcessing input parms
# $1 - Num nodes based on commandline values
# $2 - Mode based on commandline values
# $3 - NP based on commandline values
# $4 - The options to pass to runjob
# $5 - Variable name to hold final node value to run with
# $6 - Variable name to hold final mode valued to run with
# $7 - Variable name to hold final NP value to run with
# $8 - Variable to hold final opts string
# $9 - Variable name to hold override status
#-------------------------------------------------------------------------------
exe_preProcessing ()
{
    orgNodes=$1
    orgMode=$2
    orgNP=$3
    opts=$4
    finalNodesVar=$5
    finalModeVar=$6
    finalNPVar=$7
    finalOptsVar=$8
    overrideVar=$9

    eppNodes=$orgNodes
    eppMode=$orgMode
    eppNP=$orgNP
    npOverride=0
    eppOverride='N'

    runfctest="${run_floor}/scripts/runfctest.sh"

    # Ensure that user didn't set -mode and MPIRUN_MODE for BGP runs
    if [[ "${opts}" =~ "-mode" ]] && [[ "${opts}" =~ "MPIRUN_MODE" ]]
	then 
	echo "ERROR (E)::exe_preProcessing: use either -mode OR -envs MPIRUN_MODE to set mode, but not both."
	return 1
    fi

    # Ensure that user didn't set --ranks-per-node and BG_PROCESSESPERNODE for BGQ runs
    if [[ "${opts}" =~ "--ranks-per-node" ]] && [[ "${opts}" =~ "BG_PROCESSESPERNODE" ]]
	then 
	echo "ERROR (E)::exe_preProcessing: use either --ranks-per-node OR -envs BG_PROCESSESPERNODE to set \"mode\", but not both."
	return 1
    fi

    env_name=""
    env_val=""
    temp_opts=""

    # Parse "runjob" options
    # Replace command-line positional parameters with $opts values
    set - $(echo "${opts}")

    while [ "$1" != "" ]; do
	case $1 in
	    -env | --envs )           shift                  
		                      while [[ "$1" =~ '=' ]]; do
					  env_name=$( echo $1 | cut -d '=' -f1 | sed 's/"//g' )
					  env_val=$( echo $1 | cut -d '=' -f2 | sed 's/"//g' )
					  
					  # Update environment array
					  if [ "${env_name}" == 'BG_PROCESSESPERNODE' ] || [ "${env_name}" == 'MPIRUN_MODE' ]
					      then
					      if ! ([ $forceScaling -eq 1 ] && [ $cmdLineMode -eq 1 ])
						  then
						  eppMode=$env_val

						  if [ $eppMode -ne $orgMode ] && [ $cmdLineMode -eq 1 ]
						      then
						      eppOverride='Y'
						  fi
					      fi
					  fi                               
					  shift
				      done
				      ;;
	    --ranks-per-node )        shift
		                      if ! ([ $forceScaling -eq 1 ] && [ $cmdLineMode -eq 1 ])
					  then
					  eppMode=$1

					  if [ $eppMode -ne $orgMode ] && [ $cmdLineMode -eq 1 ]
					      then
					      eppOverride='Y'
					  fi
				      fi

                                      # Remove --ranks-per-node from $opts (unsupported by runfctest.sh)
				      # Save off everything after "--ranks-per-node"
				      temp_opts=${opts##*--ranks-per-node}

				      # Set opts = everything before "--ranks-per-node"
				      opts=${opts%%--ranks-per-node*}

				      # Final opts string with --ranks_per_node val removed
				      if [[ "${temp_opts}" =~ '--' ]]
					  then
					  opts="${opts}--${temp_opts#*--}"
				      fi

				      shift
				      ;;
	    -mode )                   shift
		                      if ! ([ $forceScaling -eq 1 ] && [ $cmdLineMode -eq 1 ])
					  then
					  eppMode=$1

					  if [ "${eppMode}" != "${orgMode}" ] && [ $cmdLineMode -eq 1 ]
					      then
					      eppOverride='Y'
					  fi
				      fi

                                      # Remove -mode from $opts (so we control value using mpirun)
				      # Save off everything after "-mode"
				      temp_opts=${opts##*-mode}

				      # Set opts = everything before "-mode"
				      opts=${opts%%-mode*}

				      # Final opts string with --ranks_per_node val removed
				      if [[ "${temp_opts}" =~ '-' ]]
					  then
					  opts="${opts}-${temp_opts#*-}"
				      fi

				      shift
				      ;;
	    -n )                      shift
		                      # Alternate to -np
		                      if ! ([ $forceScaling -eq 1 ] && [ $forceNP -gt 0 ])
					  then
					  npOverride=$1
				      fi

                                      # Remove -n from $opts (so we control value using mpirun)
				      # Save off everything after "-n"
				      temp_opts=${opts##*-n }

				      # Set opts = everything before "-n"
				      opts=${opts%%-n *}

				      # Final opts string with -n val removed
				      if [[ "${temp_opts}" =~ '-' ]]
					  then					 
					  opts="${opts}-${temp_opts#*-}"
				      fi
				      
				      shift
				      ;;
	    -nodes )                  shift
                                      # Alternate to -np
		                      if ! ([ $forceScaling -eq 1 ] && [ $forceNP -gt 0 ])
					  then
					  npOverride=$1
				      fi

                                      # Remove -nodes from $opts (so we control value using mpirun)
				      # Save off everything after "-nodes"
				      temp_opts=${opts##*-nodes}

				      # Set opts = everything before "-nodes"
				      opts=${opts%%-nodes*}

				      # Final opts string with -nodes val removed
				      if [[ "${temp_opts}" =~ '-' ]]
					  then					 
					  opts="${opts}-${temp_opts#*-}"
				      fi
				      
				      shift
				      ;;
	    -np )                     shift
		                      if ! ([ $forceScaling -eq 1 ] && [ $forceNP -gt 0 ])
					  then
					  npOverride=$1
				      fi

                                      # Remove -np from $opts (so we control value using mpirun)
				      # Save off everything after "-np"
				      temp_opts=${opts##*-np}

				      # Set opts = everything before "-np"
				      opts=${opts%%-np*}

				      # Final opts string with -np val removed
				      if [[ "${temp_opts}" =~ '-' ]]
					  then					 
					  opts="${opts}-${temp_opts#*-}"
				      fi
				      
				      shift
				      ;;
	    --np )                    shift
		                      if ! ([ $forceScaling -eq 1 ] && [ $forceNP -gt 0 ])
					  then
					  npOverride=$1
				      fi

                                      # Remove --np from $opts (unsupported by runfctest.sh)
				      # Save off everything after "--np"
				      temp_opts=${opts##*--np}

				      # Set opts = everything before "--np"
				      opts=${opts%%--np*}

				      # Final opts string with --np val removed
				      if [[ "${temp_opts}" =~ '--' ]]
					  then					 
					  opts="${opts}--${temp_opts#*--}"
				      fi

				      shift
				      ;;
	    --numnodes )              shift
		                      if ! ([ $forceScaling -eq 1 ] && [ $cmdLineNode -eq 1 ])
					  then
					  eppNodes=$1

					  if [ $eppNodes -ne $orgNodes ] && [ $cmdLineNode -eq 1 ]
					      then
					      eppOverride='Y'
					  fi
				      fi

                                      # Remove --numnodes from $opts (we'll add it back in later)
				      # Save off everything after "--numnodes"
				      temp_opts=${opts##*--numnodes}

				      # Set opts = everything before "--numnodes"
				      opts=${opts%%--numnodes*}

				      # Final opts string with --numnodes val removed
				      if [[ "${temp_opts}" =~ '--' ]]
					  then					 
					  opts="${opts}--${temp_opts#*--}"
				      fi

				      shift
				      ;;
	    --twinstarfloor )         shift # go to twinstarfloor arg
		                      shift # go to next option
				      # Have to do this since this option is valid, but not in the runfctest.sh help text
		                      ;;
	    * )                       # See if this is an option or value
		                      if [[ "$1" =~ '-' ]] # option
					  then
	
                                          # Check against runfctest help text
					  found=$($runfctest --help | grep -c -e "$1")

					  if [ $found -eq 0 ]
					      then 
					      echo "ERROR (E)::exe_preProcessing: Unsupported option $1 for ${runfctest}!!"
					      echo "ERROR (E)::exe_preProcessing: Type ${runfctest} --help to view supported options"
					      return 1
					  fi
 
					  shift
				      else # value
					  shift
				      fi
	esac
#	shift
    done

    # Determine NP to run with
    if [ $npOverride -gt 0 ]
	then
	eppNP=$npOverride
    elif [ $forceNP -gt 0 ]
	then
	eppNP=$forceNP
    else
	# Get numerical version of mode for BGP
	if [ "${platform}" == 'BGP' ]
	    then
	    bgp_mode_TexttoNum $eppMode eppMode
	    if [ $? -ne 0 ]
		then
		echo "ERROR (E)::exe_preProcessing: bgp_mode_TexttoNum subroutine FAILED!!"
		return 1
	    fi
	fi

	eppNP=$(( $eppNodes * $eppMode ))
    fi

    if [ $eppNP -ne $orgNP ] && [ $forceNP -gt 0 ] 
	then
	eppOverride='Y'
    fi

    eval $finalNodesVar=$eppNodes
    eval $finalModeVar=$eppMode
    eval $finalNPVar=$eppNP
    eval $overrideVar=$eppOverride
    eval $finalOptsVar=\"$opts\"

    return 0
}

#-------------------------------------------------------------------------------
# runSim input parms
# $1 - type of sim (runMambo, runFpga or runMmcsLite) 
# $2 - The working directory for the test
# $3 - The executable/binary file to run
# $4 - Test scenario:  nodes, mode, NP
# $5 - The options to pass to runjob
# $6 - The arguments to pass to the executable
# $7 - The log file for this run
# $8 - The variable to store the exe return code for this test
# $9 - The variable to store the elapsed time for this test
#-------------------------------------------------------------------------------
runSim ()
{
    # Get the arguments
    type=$1
    cwd=$2
    exe=$3
    simNodes=$( echo $4 | awk '{print $1}' )
    simMode=$( echo $4 | awk '{print $2}' )
    simProcs=$( echo $4 | awk '{print $3}' )
    opts=$5
    args=$6
    logFile=$7
    simSignalVar=$8
    elapsedTimeVar=$9

    runfctest="${run_floor}/scripts/runfctest.sh"
    
    if [[ "${exe}" =~ 'threadTest_omp' ]]
	then
	threads=16
    else
	threads=1
    fi

    runSim_rc=0

    declare -a ENV_VAR_ARRAY          # array of ENV VAR names
    declare -a ENV_VAL_ARRAY          # array of new ENV VAR values
    declare -a ENV_PREVAL_ARRAY       # array of PREvious ENV VAR VALues

    element=0
    env_name=""
    env_val=""
    temp_opts=""

    # Parse "runjob" options
    # Determine which ENV vars need to be saved, updated and documented
    set - $(echo "${opts}")

    while [ "$1" != "" ]; do
	case $1 in
	    --envs )                  shift                  
		                      while [[ "$1" =~ '=' ]];do
					  env_name=$( echo $1 | cut -d '=' -f1 | sed 's/"//g' )
					  env_val=$( echo $1 | cut -d '=' -f2 | sed 's/"//g' )
					  
					  # Update environment array
					  if [ "${env_name}" != 'BG_PROCESSESPERNODE' ]
					      then

					      ENV_VAR_ARRAY[$element]=$env_name
					      ENV_VAL_ARRAY[$element]=$env_val
					      ENV_PREVAL_ARRAY[$element]=$( eval echo \$${env_name} )

                                              # Look for OMP threads
					      if [ "${env_name}" == 'OMP_NUM_THREADS' ]
						  then
						  threads=$env_val
					      fi

                                              # Increment element 
					      element=$(( $element + 1 ))
					  fi                                          
					  shift
				      done

				      # Remove --env from $opts (unsupported by runMambo.sh and runfctest.sh)
				      # Save off everything after "--envs"
				      temp_opts=${opts##*--envs}

				      # Set opts = everything before "--envs"
				      opts=${opts%%--envs*}

				      # New opts string with --env arg1 arg 2 ... removed
				      if [[ "${temp_opts}" =~ '--' ]]
					  then
					  opts="${opts}--${temp_opts#*--}"
				      fi
				      ;;
	    * )                       # Goto the next value
		                      shift
	esac
#	shift
    done

    # Parse exe args looking for number of threads
    # Replace command-line positional parameters with $args values

    set - $(echo "${args}" | sed 's/"//g')

    while [ "$1" != "" ]; do
	case $1 in
	    --numPthreads )                  shift 
		                             threads=$((1 + $1)) # total num threads = main thread + $1 Pthreads = 1 + $1
					     ;;
	    -n )                             shift
		                             if [[ "${exe}" =~ 'threadTest' ]]
						 then
						 threads=$1 # for threadTests, total num threads = main thread + ($1 - 1) Pthreads = $1   
					     fi
					     ;;
        * ) #continue                    
	esac
	shift
    done

    # Append mode from command line input to arrays
    ENV_VAL_ARRAY[${#ENV_VAR_ARRAY[@]}]=$simMode
    ENV_PREVAL_ARRAY[${#ENV_VAR_ARRAY[@]}]=$BG_PROCESSESPERNODE

    # Have to do this last or arrays won't match up
    ENV_VAR_ARRAY[${#ENV_VAR_ARRAY[@]}]='BG_PROCESSESPERNODE'

    # Update shell with ENV vars
    for ((var=0; var < ${#ENV_VAR_ARRAY[@]}; var++))
      do
      if [ "${ENV_VAL_ARRAY[${var}]}" == 'NULL' ]
	  then
	  unset ${ENV_VAR_ARRAY[${var}]}
      else
	  export ${ENV_VAR_ARRAY[${var}]}=${ENV_VAL_ARRAY[${var}]}
      fi
    done

    # Check for common parms that we don't want to fill the input file with: 
    # ttype
    if [[ ! "${opts}" =~ "--ttype" ]]
	then
	opts="${opts} --ttype cnk"
    fi

    # noobjdump
    if [[ ! "${opts}" =~ "--noobjdump" ]]
	then
	opts="${opts} --noobjdump"
    fi

    # numcores
    if [ "${type}" != "runMmcsLite" ] && [[ ! "${opts}" =~ "--numcores" ]]
	then
	opts="${opts} --numcores 17"
    fi

    # svchost (for MMCS-Lite and FPGA)
    if [ "${type}" != "runMambo" ] && [[ ! "${opts}" =~ "--svchost" ]] 
	then
	opts="${opts} --svchost ${abs_test_dir}/svchost.pers"
    fi

    # Check for MMCS-Lite specific parms that we don't want to fill the input file with:
    if [ "${type}" == 'runMmcsLite' ]
	then

        # numnodes
	if [[ ! "${opts}" =~ "--numnodes" ]]
	    then
	    opts="${opts} --numnodes $simNodes"
	fi
    fi

    # Check for fpga specific parms that we don't want to fill the input file with:
    if [ "${type}" == 'runFpga' ]
	then

	# twinstar path
	if [[ ! "${opts}" =~ "--twinstarfloor" ]]
	    then
	    opts="${opts} --twinstarfloor /gsa/yktgsa/projects/t/twinstar_dd2/release/oct_11_2010_dd2_mc/"
	fi

        # llwall_clock_limit
	if [[ ! "${opts}" =~ "--llwall_clock_limit" ]]
	    then
	    opts="${opts} --llwall_clock_limit 00:30:00"
	fi

        # maxtime
	if [[ ! "${opts}" =~ "--maxtime" ]]
	    then
	    opts="${opts} --maxtime 8000000000"
	fi
    fi

    # Save off the current directory
    curDir=`pwd`

    # Make sure we don't lose any error return codes due to piping
#    set -o pipefail

    # Echo ENV VARs to logfile
    {
	echo "NP = ${simProcs}"
	echo "THREADS = ${threads}"
	echo "NODES = ${simNodes}"
	echo "BG_PROCESSESPERNODE = ${BG_PROCESSESPERNODE}"
	echo "PAMI_DEVICE = ${PAMI_DEVICE}"
	echo "BG_MEMSIZE = ${BG_MEMSIZE}"
	echo "BG_SHAREDMEMSIZE = ${BG_SHAREDMEMSIZE}"
    } >> $logFile

    for ((var=0; var < ${#ENV_VAR_ARRAY[@]}; var++))
      do
      if [ "${ENV_VAR_ARRAY[$var]}" != 'BG_PROCESSESPERNODE' ] && [ "${ENV_VAR_ARRAY[$var]}" != 'PAMI_DEVICE' ] && [ "${ENV_VAR_ARRAY[$var]}" != 'BG_MEMSIZE' ] && [ "${ENV_VAR_ARRAY[$var]}" != 'BG_SHAREDMEMSIZE' ]
	  then
	  echo "${ENV_VAR_ARRAY[$var]} = ${ENV_VAL_ARRAY[$var]}" >> $logFile 
      fi
    done

    echo "SERVER ID = ${serverID}" >> $logFile

    echo ""

    # Change to the working directory and run the test
    changeDir $cwd

    if [ $? -ne 0 ]
	then
	echo "ERROR (E)::runSim: cd to exe dir FAILED!! Skipping to the next test ..."
	return 1
    fi

    # Remove "'s from args string if they exist
#    args=$( echo $args | sed 's/"//g' )  

    if [ "${type}" == 'runMmcsLite' ]
	then
	runCmd="${runfctest} --script ${type} ${opts} --location ${block} --program ${exe} -- ${args}"
    else # runMambo or runFpga
	runCmd="${runfctest} --script ${type} ${opts} --program ${exe} -- ${args}"
    fi

    # Start time in Epoch time
    before=$(date +%s)

    if [ $quietly -eq 1 ]
	then
	echo $runCmd >> $logFile
	if [ $debug -eq 0 ]
	    then
	    eval $runCmd >> $logFile
	    runStatus=($( echo ${PIPESTATUS[*]} ))
	fi
    else
	echo $runCmd | tee -a $logFile
	if [ $debug -eq 0 ]
	    then
	    eval $runCmd | tee -a $logFile
	    runStatus=($( echo ${PIPESTATUS[*]} ))
	fi
    fi

    # End time in Epoch time
    after=$(date +%s)

    # Run time in readable format
    elapsed_seconds="$(($after - $before))" # Needs to be a string
    eval $elapsedTimeVar=$(date -d "1970-01-01 $elapsed_seconds seconds" +%T)

    if [ $debug -eq 1 ]
	then

	if [ "${type}" == 'runFpga' ]
	    then

	    fctestDir="${cwd}/FCTest.dummy.${exe%.*}.$(date +%Y%m%d-%H%M%S)"

	    while [ -e "${fctestDir}" ]; do
		fctestDir="${cwd}/FCTest.dummy.${exe%.*}.$(date +%Y%m%d-%H%M%S)"
	    done

	    mkdir -p "${fctestDir}"
	    if [ $? -ne 0 ] || [ ! -d "${fctestDir}" ]
		then
		echo "Creation of ${fctestDir} FAILED!!"
		cleanExit 1
	    fi

	    echo "Test directory: ${fctestDir}" >> $logFile	
	    echo 'Software Test PASS. 0' >> "${fctestDir}/std.out"
	else
	    for ((i=0; i < ${simNodes}; i++))
	      do
	      echo 'Software Test PASS. 0' >> $logFile
	    done
	fi

	declare -a runStatus; runStatus[0]=0; runStatus[1]=0
    fi

    eval $simSignalVar=${runStatus[0]}

    # Check status of runfctest
    if [ ${runStatus[0]} -ne 0 ]
	then
	echo "ERROR (E)::runSim: Execution of $exe FAILED!!"
	runSim_rc=${runStatus[0]}
    else # runCmd passed, let's check the logFile
	if [ $quietly -eq 0 ]
	    then
	    if [ ${runStatus[1]} -ne 0 ] || [ ! -e $logFile ]
		then
		echo "ERROR (E)::runSim: FAILED to tee output into $logFile!!"
		runSim_rc=${runStatus[1]}
	    fi
	fi
    fi

    # Verify success/failure of binary itself for runMambo and runMmcsLite runs
    if [ "${runSim_rc}" -eq 0 ] && [ "${type}" != 'runFpga' ]
	then
	$(tail -10 $logFile | grep -q 'Software Test PASS')
       
	runSim_rc=$?
    fi

    # Restore ENV VARs
    for ((var=0; var < ${#ENV_VAR_ARRAY[@]}; var++))
      do
      export ${ENV_VAR_ARRAY[$var]}=${ENV_PREVAL_ARRAY[$var]}
    done

    # Switch back to the previous directory
    changeDir $curDir

    if [ $? -ne 0 ]
	then
	echo "ERROR (E)::runSim: cd back to original dir FAILED!! Exiting."
	cleanExit 1
    fi

    # Return
    return $runSim_rc
}

llJobs ()
{
    action=$1
    old_queue=$fpga_queue   # Use this to see if we've made any progress

    llJobs_rc=0

    if [ "${action}" != 'query' ] && [ "${action}" != 'cancel' ]
	then
	echo -e "\nERROR (E):llJobs:  Parameter $action is undefined.  Defined values are: query and cancel.\n"
	return 1
    fi	

    # Find out how many jobids in the queue are from this program
    # Reset fpga_queue
    if [ "${action}" == 'query' ]
	then
	fpga_queue=0
    fi

    # Save and change IFS
    OLDIFS=$IFS
    IFS=$'\n'

    # Read all jobids into an array
    jobidArray=($(llq | grep $USER | awk '{print $1}'))

    # Read all job stati into an array
    jobSTArray=($(llq | grep $USER | awk '{print $5}'))

    # Restore IFS
    IFS=$OLDIFS

    for ((jobid=0; jobid < ${#jobidArray[@]}; jobid++))
      do

      # Chop off the step name from the llq jobid
      jobidArray[$jobid]=${jobidArray[$jobid]%.*}

      for ((test=0; test < ${#TEST_ARRAY[@]}; test++))
	do
	for numNodes in ${nodeArray[@]}
	  do
	  for mode in ${modeArray[@]}
	    do
	    
	    if [ $forceNP -eq 0 ]
		then
		numProcs=$(( $numNodes * $mode ))
	    else
		numProcs=$forceNP
	    fi

	    # Compare known jobids to jobids in queue
	    hget $exeHash "${TEST_ARRAY[$test]}:$test:FPGAjobID_n${numNodes}_m${mode}_p${numProcs}" fpgaJobID
	    
	    if [ "${fpgaJobID}" == "${jobidArray[$jobid]}" ]
		then 
		if [ "${action}" == 'query' ]
		    then
		    if [ "${jobSTArray[$jobid]}" == "H" ] || [ "${jobSTArray[$jobid]}" == "NR" ] || [ "${jobSTArray[$jobid]}" == "S" ] || [ "${jobSTArray[$jobid]}" == "SH" ] || [ "${jobSTArray[$jobid]}" == "V" ] || [ "${jobSTArray[$jobid]}" == "XP" ]
			then
		    
		        # Cancel this job
			hput $exeHash "${TEST_ARRAY[$test]}:$test:status_n${numNodes}_m${mode}_p${numProcs}" 'Cancelled'
			
			echo -e "\nERROR (E): ${TEST_ARRAY[$test]} llq status = ${jobSTArray[$jobid]}. Cancelling jobid ${jobidArray[$jobid]}"
			llcancel ${jobidArray[$jobid]}
			continue
		    else # Wait for this job to complete
			fpga_queue=$(( $fpga_queue + 1 ))
		    fi
		else # cancel
		    hput $exeHash "${TEST_ARRAY[$test]}:$test:status_n${numNodes}_m${mode}_p${numProcs}" 'Cancelled'
		    echo "Cancelling ${TEST_ARRAY[$test]} (jobid ${jobidArray[$jobid]})"
		    llcancel ${jobidArray[$jobid]}
		    continue
		fi
	    fi
	  done # end mode loop
	done # end node loop
      done # end test loop
    done # end jobid loop

    if [ $fpga_queue -ne 0 ] && [ $fpga_queue -lt $old_queue ]
	then
	echo -e -n "\n $fpga_queue job(s) remaining in the queue." 
    fi

    return $llJobs_rc
}

changeDir ()
{
    dir=$1
    cDir_rc=0

    if [ -e $dir ]
	then
	cd $dir

	if [ $? -ne 0 ]
	    then
	    echo "ERROR (E)::changeDir: cd to $dir FAILED!!"
	    cDir_rc=1
	fi
    else
	echo "ERROR (E)::changeDir: $dir DNE!!"
	cDir_rc=1
    fi

    return $cDir_rc
}

reConfig ()
{
    tar=$1
    level=$2
    type=$3
    new_floor=
    config_rc=0

    # Save current dir
    curDir=`pwd`

    # cd to bgq dir
    changeDir $abs_build_dir
    if [ $? -ne 0 ]
	then
	echo "ERROR (E)::reConfig: changeDir FAILED!!"
	config_rc=1
    fi

    echo "INFO (I)::reConfig: Reconfiguring ${abs_build_dir} tree to: ${level}"

    # exe the configure script
    config_cmd="${abs_buildtools_dir}/configure --with-target=${tar} --with-bgfloor=${level}"

    if [ "${type}" == 'hw' ] || [ "${type}" == 'runMmcsLite' ]
	then
	config_cmd="${config_cmd} --without-mambo-workarounds"
    fi
    
    echo $config_cmd
    eval $config_cmd

    # Verify tree floor changed successfully
    if [ $? -ne 0 ]
	then
	echo "ERROR (E)::reConfig: configure script FAILED!!"
	config_rc=1
    else # command passed, check value
	new_floor=$(fgrep 'BGQ_FLOOR_DIR=' "${abs_build_dir}/Make.rules" | cut -d '=' -f2)
	new_floor=$(readlink -e "${new_floor}" )

	# Remove floor test until we have something to compare to
#	if [ "${new_floor}" != "${level}" ]
#	    then
#	    echo "ERROR (E)::reConfig: Floor mismatch:"
#	    echo "ERROR (E)::reConfig: BGQ_FLOOR_DIR => ${new_floor}"
#	    echo "ERROR (E)::reConfig: Desired       => ${level}"
#	    config_rc=1
#	else
	    echo "BGQ_FLOOR_DIR is now set to: ${level}"
#	fi
    fi

    # Restore original dir
    changeDir $curDir

    return $config_rc
}

usage ()
{
    echo "pami_verif.sh compiles, copies and executes binaries it reads in from a file."
    echo "By default, binaries will be copied to a bgusr exe dir and run on hw." 
    echo "Separate make log files and runtime log files are made for each binary."
    echo "pami_verif.sh can be called from any dir, but must reside in ../pami/test"
    echo ""
    echo "Prerequisites:"
    echo "   Source code for all exes and this script was checked out/updated in both the compile and runtime domains:"
    echo "      See SST Wiki:"
    echo "         https://bgweb.rchland.ibm.com/bgq/wiki/index.php/HOWTO_use_Subversion#Access_From_Rochester_Servers"
    echo ""
    echo "   Source code for bgtools was checked out/updated and installed in the runtime domain:"
    echo "      From bgq dir:  svn update system_tests/tools"
    echo "      source bgtools setup file (see Installation in ../bgq/system_tests/tools/bgtools/README)"
    echo "      Additional bgtools documentation can be found at the following wiki:"
    echo "         BGQ: https://bgweb.rchland.ibm.com/bgq/wiki/index.php/BlueGene_User_Utilities"
    echo ""
    echo "Format:  pami_verif.sh [options]"
    echo "Options:"
    echo ""
    echo "-ag | --autogroup            Automatically generates a group name for tests using input file, PAMI_DEVICE, number of nodes and mode and passes it to logXML.sh."
    echo "                             autogroup will create groups that scale with your run."
    echo "                             -g | --group overrides autogroup."
    echo ""
    echo "                             ex: -nn \"1 2\" -rpn \"4 8\" -ag"
    echo "                                 input file = /somedir/anotherdir/debug.in"
    echo "                                 PAMI_DEVICE = M"
    echo ""
    echo "                                 4 groups would be created and populated:"
    echo "                                    debug_MU_1n_4m"
    echo "                                    debug_MU_1n_8m"
    echo "                                    debug_MU_2n_4m"
    echo "                                    debug_MU_2n_8m"
    echo ""
    echo "-b | --block <block>         Block name for runjob." 
    echo ""
    echo "-c | --compile               Only compile binaries. Can be combined with -cp and/or -r."
    echo ""
    echo "-clean | --clean             Run make clean before building binaries. MUST be combined with -c"
    echo ""
    echo "-comment | --comment         Used with -email option to give short unique description of tests being run"
    echo "                             Text is prepended to summary results."
    echo "                                      ex: -comment \"This is a test\""
    echo ""
    echo "-cp | --copy                 Only copy binaries to bgusr exe dir. Can be combined with -c and/or -r."
    echo ""
    echo "-d | --debug                 Debug enabled, major commands (make, runjob, etc) are only echoed."
    echo ""
    echo "-db | --dbhost <arg>         Select which DB to add results to."
    echo "                             Valid values: sst or msg (PAMI)."
    echo "                             default: sst"
    echo ""
    echo "-dn | --display-name <length>Specify how much of the test name to display on the SST Test History web page."
    echo "                             Valid values: short or rel."
    echo "                             default: rel              (displays /exercisers/bgqmem/bgqmem.elf)"
    echo "                             ex:  --display-name short (displays bgqmem.elf)"
    echo ""
    echo "-e | --exebase <path>        Specify trunk for binaries to be copied to and run from."
    echo "                              hw default: /bgusr/<USER>/<sandbox>/pami/<platform>/exe/test"
    echo "                                      ex: /bgusr/alderman/pami_sb_bgq/pami/bgq/exe/test" 
    echo "                             sim default: /gsa/rchgsa/home/<USER0:1>/<USER1:1>/<USER>/<sandbox>/pami/<platform>/exe/test"                                 
    echo "                                      ex: /gsa/rchgsa/home/a/l/alderman/pami_sb_bgq/pami/bgq/exe/test"
    echo ""
    echo "-email | --email <addr>      Email results summary to recipient(s). Can be combined with -comment option."
    echo "                                      ex: -email foo@us.ibm.com"
    echo "                                      ex: --email \"foo@us.ibm.com manchu@us.ibm.com\""
    echo ""
    echo "-f | --fpga                  Directs copies to gsa exe dir and runs from gsa exe dir on fpga simulator."
    echo ""
    echo "-fs | --force-scaling        Forces run to use command line values for parms that exist in both the command line and input file.  Otherwise, input file values trump command line values."
    echo ""
    echo "-fw | --force-web )          Force compile results to the SST DB. Default is to only document compile fails."
    echo ""
    echo "-g | --group <name>          Provide group name for logXml.sh. Time stamp will atomatically be appended by this script. Overrides -ag | --autogroup. Can be used with -gs | --group-scale."
    echo "                                      ex: -g foo"
    echo "" 
    echo "-gs | --group-scale          Appends nodes and mode to group name provided with -g | --group parm.  For scaling runs, multiple groups will be created and populated"
    echo "                                      ex: -g foo --group-scale -nn 2"
    echo "                                          passes -g foo_2n_1m to logXML.sh"
    echo ""
    echo "                                      ex: -g bar --gs -nn \"1 2\" -rpn \"4 8\""
    echo "                                          creates and populates 4 groups instead of 1:"
    echo "                                          bar_1n_4m"
    echo "                                          bar_1n_8m"
    echo "                                          bar_2n_4m"
    echo "                                          bar_2n_8m"
    echo "" 
    echo "-i | --input <file>          Specify input file."
    echo "                              hw default: ../pami/test/bvt.in"
    echo "                             sim default: ../pami/test/bvt.in.sim"
    echo ""
    echo "                             NOTE:  to specify a binary as \"compile only\" type --compile-only on the same line as the binary (after the binary name, but before --args (if present))" 
    echo "" 
    echo "-l | --lite )                Directs copies to bgusr exe dir and runs from bgusr exe dir on hw using MMCS-Lite."
    echo ""
    echo "-m | --mambo )               Directs copies to gsa exe dir and runs from gsa exe dir on mambo simulator."
    echo ""
    echo "-mode <arg>                  Specify BGP mode to run in. Can be given as a space separated range for scaling purposes.  Command line value(s) will be trumped by -mode or -env MPIRUN_MODE in the input file unless -fs|--force-scaling is also used on the command line."
    echo "                             Valid values: SMP, DUAL & VN."
    echo "                             default: 1 rank per node"
    echo "                             ex:  -mode DUAL"
    echo "                             ex:  -mode \"SMP VN\" (scaling)"
    echo ""
    echo "-noweb | --noweb )           Skips updating SST Test History web page using logXML.sh."
    echo ""
    echo "-nn | --numnodes <arg>       Specify number of nodes to boot (required for multi-node runs with MMCS-Lite). Can be given as a space separated range for scaling purposes.  Command line value(s) will be trumped by --numnodes in the input file unless -fs|--force-scaling is also used on the command line."
    echo "                             default: 1 node"
    echo "                             ex:  --numnodes 2"
    echo "                             ex:  --nn \"4 16\" (scaling)"
    echo "" 
    echo "-o | --outdir <path>         Specify dir for runtime output logs."
    echo "                             default:  <exedir>/results"
    echo ""
    echo "-p | --platform <arg>        Specify the platform that tests were run on (used with logXML.sh)."
    echo "                             Valid values: bgp, bgq, linux, mpi & percs."
    echo "                             default:  bgq"
    echo ""
    echo "-q | --quiet )               Redirect runtime output (>>) instead of tee."
    echo ""
    echo "-r | --run )                 Only run binaries from bgusr exe dir on hardware using MMCS. Can be combined with -cp and/or -c."
    echo ""
    echo "-rc | --reconfigure <floor>  Cds to ../bgq dir and runs ./script/configure --with-floor=<floor>"
    echo "                             ex:  -rc latest automatically sets <floor> to current floor"
    echo "                             ex:  -rc /bgsys/drivers/DRV240_2010-100617Q-dd1-floor/ppc64-rhel53"
    echo ""                             
    echo "-rf | --runfloor <path>      Specify floor used for running binaries."
    echo "                             default = BGQ_FLOOR_DIR from ../bgq/Make.rules"
    echo ""
    echo "-rpn | --ranks-per-node <arg> Specify BGQ \"mode\" to run in. Can be given as a space separated range for scaling purposes.  Command line value(s) will be trumped by --ranks-per-node or --envs BG_PROCESSESPERNODE in the input file unless -fs|--force-scaling is also used on the command line."
    echo "                             Valid values: 1 - 64 in powers of 2:  1, 2, 4, 8, 16, 32 & 64."
    echo "                             default: 1 rank per node"
    echo "                             ex:  --ranks-per-node 2"
    echo "                             ex:  --rpn \"8 64\" (scaling)"
    echo ""
    echo "-s | --serverid )            Allows user to specify server ID of service node"
    echo "                             ex: -s bgts3sn.rchland.ibm.com"
    echo "                             ex: --serverid bgts3sn"
    echo ""
    echo "-sb | --sandbox <path>       Specify alternate sandbox dir"
    echo "                             default: everything before /pami/test"
    echo "                             ex: /bghome/alderman/test"
    echo ""
    echo "-v | --verbose               Enable extra info to be printed to output."
    echo ""
    echo "-h | --help                  This help text."
}

# --- Global variables ---
abs_test_dir=$(dirname $(readlink -f $0))  # my dir
abs_pami_dir=${abs_test_dir%/*}
abs_sandbox_dir=${abs_pami_dir%/*}
abs_buildtools_dir="${abs_sandbox_dir}/buildtools"
abs_build_dir="${abs_sandbox_dir}/build"
file=""
verbose=0
quietly=0
ding=${DING:-0};        # 1 = issue audible "ding" for error or completion messages.
debug=0
reconfigure=0
cur_floor=
timestamp="$(date +%Y%m%d-%H%M%S)"
exeHash="/tmp/hashmap.exe.${timestamp}"         # store data for each test

# Compile vars
make_log='make.log'
checkout=0
update=0
compile=2          # 0 = off, 1 = on, 2 = unset
make_clean=0
compile_floor=""
copy=2             # 0 = off, 1 = on, 2 = unset
compileHash="/tmp/hashmap.compile.${timestamp}" # avoid recompiles
copyHash="/tmp/hashmap.copy.${timestamp}"       # avoid recopies

# Exe vars
gsa_base="/gsa/rchgsa/home/${USER:0:1}/${USER:1:1}/$USER"
bgusr_base="/bgusr/${USER}"
sandbox=""
common_exe_dir=""
exe_base=""
out_dir=""
user_outdir=0
run_type='hw'
run=2              # 0 = off, 1 = on, 2 = unset
run_floor=""
serverID="unknown"
cmdLineNode=0           # User specified node value on command line 
cmdLineMode=0           # User specified mode value on command line
cmdLineNP=0             # User specified NP value on command line
numNodes=0              # total num of nodes
forceNP=0               # force np to this value instead of ranks/node * nodes
numProcs=0              # total num of ranks/tasks = ranks/node * nodes
forceScaling=0
needBlock=0             # 0 = can run w/out block, 1 = fail if no block given
block_name="missing"
block=""
sub_block=""
fpga_queue=0
temp_jobid=""      # used to build common jobid between llsubmit and llq
jobid=""           # used by FPGA and HW runs

# Logging/Documenting vars
summaryFile="pami_verif_summary.${timestamp}"
web_update=1
logXml="${abs_test_dir}/logXML.sh"
db_host='msg'
platform='BGQ'
testNameLength='rel'
groupDev='MU'
autoGroup=0
logGroup=""
groupScale=0
logDisabled=0           # set to 1 when logXML has rc = 2 (can't create xml file)
emailAddr=""
summaryEmail="pami_verif_summary_email.${timestamp}"
comment=""

# =========================================================
# PARSE INPUT PARMS
# =========================================================
while [ "$1" != "" ]; do
    case $1 in
        -ag | --autogroup )     autoGroup=1
                                ;;
        -b | --block )          shift
	                        block_name=$1
                                ;;
        -c | --compile )        compile=1
	                        if [ $copy -eq 2 ]; then copy=0; fi
	                        if [ $run -eq 2 ]; then run=0; fi
                                ;;
        -clean | --clean )      make_clean=1
                                ;;
        -comment | --comment )  shift
	                        comment=$1
                                ;;
        -cp | --copy )          copy=1
	                        if [ $compile -eq 2 ]; then compile=0; fi
	                        if [ $run -eq 2 ]; then run=0; fi
                                ;;
        -d | --debug )          debug=1
                                ;;
        -db | --db )            shift
	                        db_host=$1
                                ;;
        -dn | --display-name )  shift
	                        testNameLength=$1
				if [ "${testNameLength}" != 'short' ] &&  [ "${testNameLength}" != 'rel' ]
				    then
				    echo "ERROR (E):  Unknown display-name arg:  $1"
				    usage
				    cleanExit 1
				fi
                                ;;
        -e | --exebase )        shift
	                        exe_base=$1
                                ;;
        -email | --email )      shift
	                        emailAddr=$1
                                ;;
        -g | --group )          shift
	                        logGroup=$1
                                ;;
        -gs | --group-scale )   groupScale=1
                                ;;
        -f | --fpga )           run_type='runFpga'
                                ;;
        -fs | --force-scaling ) forceScaling=1
                                ;;        
	-fw | --force-web )     forceWeb=1
                                ;;
        -i | --input )          shift
	                        file=$1
                                ;;
	-l | --lite )           run_type='runMmcsLite'
	                        ;;
        -m | --mambo )          run_type='runMambo'
                                ;;
        -nn | --numnodes )      shift
	                        declare -a nodeArray=($( echo $1 | sed 's/"//g' )) 
				cmdLineNode=1
                                ;;
        -noweb | --noweb )      web_update=0
                                ;;
	-np | --np )            shift
	                        forceNP=$1
				;;
        -o | --outdir )         shift
	                        out_dir=$1
				user_outdir=1
                                ;;
	-p | --platform )       shift
	                        platform=$(echo $1 | tr '[:lower:]' '[:upper:]' )
				;;
        -q | --quiet )          quietly=1
                                ;;
        -r | --run )            run=1
	                        if [ $compile -eq 2 ]; then compile=0; fi
	                        if [ $copy -eq 2 ]; then copy=0; fi
                                ;;
        -rc | --reconfigure )   shift
	                        compile_floor=$1
	                        reconfigure=1
                                ;;
        -rf | --runfloor )      shift
	                        run_floor=$1
                                ;;
	-mode | -rpn | --ranks-per-node ) shift
	                        declare -a modeArray=($( echo $1 | sed 's/"//g' | tr '[:lower:]' '[:upper:]' ))
				cmdLineMode=1
				;;
	-s | --serverid )       shift
	                        serverID=$1
				;;
        -sb | --sandbox )       shift
	                        abs_sanbox_dir="${1}"
				abs_pami_dir="${abs_sandbox_dir}/pami"
				abs_test_dir="${abs_pami_dir}/test"
				abs_buildtools_dir="${abs_sandbox_dir}/buildtools"
				abs_build_dir="${abs_sandbox_dir}/build"
                                ;;
        -v | --verbose )        verbose=1
                                ;;
        -h | --help )           usage
                                cleanExit 0
                                ;;
        * )                     echo "ERROR (E):  Unknown option $1"
                                usage
                                cleanExit 1
    esac
    shift
done

# =========================================================
# DEFAULTS
# =========================================================

# Default is to run all sections:  compile, copy & execute
if [ $compile -eq 2 ]; then compile=1; fi
if [ $copy -eq 2 ]; then copy=1; fi
if [ $run -eq 2 ]; then run=1; fi

# Verify platform
# Turn on case-insensitive matching (-s set nocasematch)
shopt -s nocasematch

case $platform in
    bgp | bgq | linux | mpi | percs )
	logXml="${logXml} --platform ${platform}"
	;;
    * )                     
	echo "ERROR (E):  Unrecognized platform: ${platform}"
	echo "ERROR (E):  Valid values are:  bgp, bgq, linux, mpi & percs"
	cleanExit 1
esac

# Turn off case-insensitive matching (-u unset nocasematch)
shopt -u nocasematch

# Set platform
if [ "${platform}" == "" ]
    then
    if [ ! -e "${abs_build_dir}/Make.rules" ]
	then 
	echo "ERROR (E): ${abs_build_dir}/Make.rules DNE!! Cannot determine platform."
	echo "ERROR (E): Run ../scripts/configure manually from: $abs_build_dir"
	echo "ERROR (E):    or"
	echo "ERROR (E): Rerun pami_verif.sh with -t <platform> and <-co | -rc <floor>> options."
	echo "ERROR (E): Exiting."
	cleanExit 1
    else
	platform=$(grep "^TARGET" "${abs_build_dir}/Make.rules" | awk '{print $3}')
	if [ $? -ne 0 ] || [ "${platform}" == "" ]
	    then
	    echo "ERROR (E): FAILED to determine platform. Exiting."
	    cleanExit 1
	fi
	
    fi
fi

# Set file to default based on input parms
if [ "${file}" == "" ]
    then
    if [ "${run_type}" == 'runMambo' ] || [ "${run_type}" == 'runFpga' ]
	then
	file="${abs_test_dir}/testfile.in.sim"
    else
	file="${abs_test_dir}/testfile.in"
    fi
fi
    
# Ensure input file exists
if [ ! -e "${file}" ]
    then
    echo "ERROR (E): Input file: $file DNE!! Exiting."

    #ding ding ding
    if [[ A1 = A$ding ]]; then echo -en "\007"; sleep 1; echo -en "\007"; sleep 1; echo -en "\007"; fi

    cleanExit 1
else
    file=$(readlink -e "${file}" )
fi

# Create make log
make_log="${abs_build_dir}/${make_log}"

# Default to current host if server ID wasn't provided
if [ "${serverID}" == 'unknown' ]
    then
    serverID=$HOSTNAME
fi

# exe_base default algorithm
# Set exe_base to default
common_exe_dir="pami/${platform}/exe/test"
temp=${abs_pami_dir%/*}
sandbox=${temp#*${USER}/}

if [ "${exe_base}" == "" ]
    then
    if [ "${run_type}" == 'runMambo' ] || [ "${run_type}" == 'runFpga' ]
	then
	exe_base="${gsa_base}/${sandbox}/${common_exe_dir}"
    else
	exe_base="${bgusr_base}/${sandbox}/${common_exe_dir}"
    fi
fi

# --- Create the exe base dir if needed ---
# Look for the exe base
if [ ! -d "${exe_base}" ] 
    then
    # Create if we planned to copy binarires during this run
    if [ $copy -eq 1 ]
	then 
	echo "Creating exe base ..."
	mkdir -p "${exe_base}"

	if [ $? -ne 0 ] || [ ! -d "${exe_base}" ]
	    then
	    echo "Creation of ${exe_base} FAILED!!"
	    cleanExit 1
	fi
    elif [ $run -eq 1 ] # Fail if we only planned to run
	then
	echo "ERROR (E): Runtime dir: $exe_base DNE!! Exiting."
	cleanExit 1
    fi
fi


# Create the results dir specified by the user
if [ $user_outdir -eq 1 ] && [ ! -d "${out_dir}" ] && [ $run -eq 1 ]
    then
    echo "Creating results dir ..."
    mkdir -p "${out_dir}"

    if [ $? -ne 0 ] || [ ! -d "${out_dir}" ]
	then
	echo "Creation of ${out_dir} FAILED!!"
	cleanExit 1
    fi
fi

# Set "mode" ENV variable to default value
if [ "${platform}" == 'BGQ' ]
    then
    if [ "${BG_PROCESSESPERNODE}" == "" ] 
	then
	echo "Setting BG_PROCESSESPERNODE to 1"
	export BG_PROCESSESPERNODE=1
    else
	echo "BG_PROCESSESPERNODE set to ${BG_PROCESSESPERNODE}"
    fi
else # BGP
    if [ "${MPIRUN_MODE}" == "" ] 
	then
	echo "Setting MPIRUN_MODE to 1"
	export MPIRUN_MODE='SMP'
    else
	echo "MPIRUN_MODE set to ${MPIRUN_MODE}"
    fi
fi

# Default ranks per node (mode)
if [ ${#modeArray[@]} -eq 0 ]
    then
    if [ "${platform}" == 'BGQ' ]
	then
	modeArray[0]=$BG_PROCESSESPERNODE
    else
	modeArray[0]=$MPIRUN_MODE
    fi
else 
    # Convert BGP text mode values to numbers
    if [ "${platform}" == 'BGP' ]
	then  
	for ((pmode=0; pmode < ${#modeArray[@]}; pmode++))
	  do
	  # Get numerical version of mode
	  bgp_mode_TexttoNum ${modeArray[$pmode]} numMode

	  if [ $? -ne 0 ]
	      then
	      echo "ERROR (E): bgp_mode_TexttoNum subroutine FAILED!!"
	      cleanExit 1
	  else
	      modeArray[$pmode]=$numMode
	  fi
	done
    fi
fi

# Default number of nodes = 1
if [ ${#nodeArray[@]} -eq 0 ]
    then
    nodeArray[0]=1
fi

# Runtime only defaults (BGQ)
if [ $run -eq 1 ] && [ "${platform}" == 'BGQ' ]
    then
    # Set up memory ENV variables to default values
    if [ "${BG_MEMSIZE}" == "" ] && [ "${run_type}" != 'runFpga' ]
	then 
#	if [ "${run_type}" == 'runFpga' ]
#	    then
#	    echo "Setting BG_MEMSIZE to 2048"
#	    export BG_MEMSIZE=2048 
#	else
	    echo "Setting BG_MEMSIZE to 4096"
	    export BG_MEMSIZE=4096 
#	fi
    else # When BG_MEMSIZE env var is set
#	if [ "${run_type}" == 'runFpga' ] && [ $BG_MEMSIZE -ne 2048 ]
#	    then
#	    echo "Forcing BG_MEMSIZE to 2048 for FPGA"
#	    export BG_MEMSIZE=2048
#	else
	    echo "BG_MEMSIZE set to ${BG_MEMSIZE}"
#	fi
    fi

    if [ "${BG_SHAREDMEMSIZE}" == "" ]
	then 
	echo "Setting BG_SHAREDMEMSIZE to 32"
	export BG_SHAREDMEMSIZE=32
    else
	echo "BG_SHAREDMEMSIZE set to ${BG_SHAREDMEMSIZE}"
    fi

    # PAMI_DEVICE default
    # When PAMI_DEVICE env var is NULL
    if [ "${PAMI_DEVICE}" == "" ]
	then 
	if [ "${run_type}" == 'runFpga' ]
	    then
	    echo "Setting PAMI_DEVICE to S"
	    export PAMI_DEVICE='S' # No MU unit in FPGA model
	    groupDev='SHMem'
	else
	    groupDev='Both'
	fi
    else # When PAMI_DEVICE env var is set
	if [ "${run_type}" == 'runFpga' ] && [ "${PAMI_DEVICE}" != 'S' ]
	    then
	    echo "Forcing PAMI_DEVICE to S for FPGA" # No MU unit in FPGA model
	    export PAMI_DEVICE='S'
	    groupDev='SHMem'
	else
	    echo "PAMI_DEVICE set to ${PAMI_DEVICE}"

	    if [ "${PAMI_DEVICE}" == 'M' ]
		then
		groupDev='MU'
	    elif [ "${PAMI_DEVICE}" == 'S' ]
		then
		groupDev='SHMem'
	    elif [ "${PAMI_DEVICE}" == 'B' ]
		then
		groupDev='Both'
	    else
		echo "ERROR (E):  Unknown PAMI_DEVICE setting:  ${PAMI_DEVICE}"
		echo "ERROR (E):  Valid options for PAMI_DEVICE are: B, M or S"
		cleanExit 1
	    fi
	fi
    fi
fi

# Verify DB host
if [ $web_update -eq 1 ]
    then
    
    # Verify DB host
    if [ "${db_host}" == 'msg' ]
	then
	logXml="${logXml} --messaging"
    else # better be sst then
	if [ "${db_host}" != 'sst' ]
	    then
	    echo "ERROR (E):  Unrecognizd DB host: ${db_host} !!"
	    echo "ERROR (E):  Valid values are:  sst & msg"
	    echo "ERROR (E):  Exiting."
	    cleanExit 1
	fi
    fi
fi

# --- Set and verify floor vars ---
# Set cur_floor
arch=$(uname -p)
cur_base='/bgsys/drivers'

if [ "${arch}" == 'ppc64' ]
    then
    cur_floor=$(readlink -e "${cur_base}/ppcfloor" )
elif [ "${arch}" == 'x86_64' ]
    then
    cur_floor=$(readlink -e "${cur_base}/x86_64.floor" )
else
    echo "ERROR (E): Unknown arch (${arch})!! Can't determine current floor. Exiting."
    cleanExit 1
fi

# Exit if readlink failed
if [ $? -ne 0 ]
    then
    echo "ERROR (E): readlink command FAILED!! Can't determine current floor. Exiting."
    cleanExit 1
fi

# Reconfigure tree to desired floor if specified
if [ $reconfigure -eq 1 ]
    then
    if [ "${compile_floor}" == 'latest' ]
	then
	compile_floor=$cur_floor
    else # Ensure path passed to me is legit
	if [ ! -e "${compile_floor}" ]
	    then
	    echo "ERROR (E): $compile_floor DNE!! Exiting."
	    cleanExit 1
	fi
    fi
	
    reConfig $platform $compile_floor $run_type
    if [ $? -ne 0 ]
	then
	echo "ERROR (E): reConfig FAILED!! Exiting."
	cleanExit 1
    fi
# Remove this until we have a floor in a file to check against
#else # Set compile floor to existing floor in Make.rules
#    compile_floor=$(grep 'BGQ_FLOOR_DIR=' "${abs_build_dir}/Make.rules" | cut -d '=' -f2 )
#    compile_floor=$(readlink -e $compile_floor)
fi


# Set run_floor
if [ $run -eq 1 ]
    then 
    if [ "${run_floor}" == "" ] || [ "${run_floor}" == 'latest' ]
	then
	run_floor=$cur_floor
    else 
        # Ensure path passed to me is legit and expand it
	run_floor=$(readlink -e "${run_floor}" )
	if [ $? -ne 0 ]
	    then
	    echo "ERROR (E): ${run_floor} DNE!! Exiting."
	    cleanExit 1
	fi
    fi
fi

# =========================================================
# PROCESS INPUT FILE
# =========================================================
element=0

echo "Processsing input file: ${file}"

declare -a TEST_ARRAY      # array of individual binaries

while read xtest xopts
  do

  # Skip blank lines and commented lines (lines that start with #)
  if [[ "${xtest}" =~ ^$ ]] || [[ "${xtest}" =~ ^# ]]
      then
      continue
  fi

  # Store binary name
  TEST_ARRAY[$element]=$(echo ${xtest##*/} | tr -d '\n' | tr -d '\r')
  hput $compileHash ${TEST_ARRAY[$element]} 0
  hput $copyHash ${TEST_ARRAY[$element]} 0

  # Store build and exe dirs
  if [[ $xtest =~ '/' ]]
      then
      hput $exeHash "${TEST_ARRAY[$element]}:$element:stub" "${xtest%/*}"
      hput $exeHash "${TEST_ARRAY[$element]}:$element:buildDir" "${abs_build_dir}/pami/test/${xtest%/*}"
      hput $exeHash "${TEST_ARRAY[$element]}:$element:exeDir" "${exe_base}/${xtest%/*}"
  else
      hput $exeHash "${TEST_ARRAY[$element]}:$element:stub" ""
      hput $exeHash "${TEST_ARRAY[$element]}:$element:buildDir" "${abs_build_dir}/pami/test"
      hput $exeHash "${TEST_ARRAY[$element]}:$element:exeDir" "${exe_base}"
  fi

  # Enable test
  hput $exeHash "${TEST_ARRAY[$element]}:$element:exe" 1
  hput $exeHash "${TEST_ARRAY[$element]}:$element:runTotal" $(( ${#nodeArray[@]} * ${#modeArray[@]} ))
  hput $exeHash "${TEST_ARRAY[$element]}:$element:runPass" 0

  # Create Overall Status element
  hput $exeHash "${TEST_ARRAY[$element]}:$element:status" 'Skipped'

  # Create Compile log element
  hput $exeHash "${TEST_ARRAY[$element]}:$element:logCompile" 'N/A'

  # Store "runjob" options
  tempOpts="$(echo ${xopts%%--args*} | tr -d '\n' | tr -d '\r')"

  # Does the user plan to compile only?
  if [[ "${tempOpts}" =~ 'compile-only' ]]
      then
      if [ $compile -eq 1 ]
	  then
	  hput $exeHash "${TEST_ARRAY[$element]}:$element:compileOnly" 1
      else # Disable this test and tell user why
	  hput $exeHash "${TEST_ARRAY[$element]}:$element:exe" 0
	  hput $exeHash "${TEST_ARRAY[$element]}:$element:status" 'Compile Only'
      fi
  else
      hput $exeHash "${TEST_ARRAY[$element]}:$element:compileOnly" 0
      hput $exeHash "${TEST_ARRAY[$element]}:$element:runOpts" "${tempOpts}"
      needBlock=1
  fi

#  hput $exeHash "${TEST_ARRAY[$element]}:$element:runOpts" "$(echo ${xopts%%--args*} | tr -d '\n' | tr -d '\r')"

  # Store exe args
  if [[ ${xopts} =~ '--args' ]]
      then
      hput $exeHash "${TEST_ARRAY[$element]}:$element:exeArgs" "$(echo ${xopts##*--args } | tr -d '\n' | tr -d '\r' | sed 's/\"//g' )"
  fi

  # Increment element 
  element=$(( $element + 1 ))

done < $file

# Ensure block was given if executing binaries on hw
if [ $run -eq 1 ] && [ $needBlock -eq 1 ]     
    then 
    if [ "${run_type}" == "hw" ] || [ "${run_type}" == "runMmcsLite" ]
	then
	if [ "${block_name}" == "missing" ]
	    then
	    echo -e "ERROR (E): Block parameter missing. Specify with -b <block> or --block <block>.\nExiting." 

            #ding ding ding
	    if [[ A1 = A$ding ]]; then echo -en "\007"; sleep 1; echo -en "\007"; sleep 1; echo -en "\007"; fi

	    cleanExit 1
	else
	    sub_block=`echo $block_name | sed "s|:| |" | cut -d " " -s -f2`
	    block=`echo $block_name | sed "s|:.*||g"`
	fi
    fi
fi

# =========================================================
# COMPILE
# =========================================================
if [ $compile -eq 1 ]
    then

    echo -e "\n******************************" 
    echo "***                        ***"
    echo "***   Compiling binaries   ***"
    echo "***                        ***"
    echo -e "******************************\n"

    # Only do this check if we did a reconfigure. Otherwise we can't determine compile_floor.
    if [ $reconfigure -eq 1 ]
	then
	if [ "${cur_floor}" == "${compile_floor}" ]
	    then
	    echo "Compiling with floor:  ${cur_floor}"
	else
	    echo "WARNING (W): Floor mismatch:"
	    echo "WARNING (W): Latest floor  => ${cur_floor}"
	    echo "WARNING (W): Compile floor => ${compile_floor}"
	fi
    fi

    changeDir $abs_build_dir

    if [ $? -ne 0 ]
	then
	echo "ERROR (E): changeDir FAILED!! Exiting."
	cleanExit 1
    fi

    # Perform make clean if requested
    if [ $make_clean == 1 ]
	then
	echo -e "\nINFO (I):  Performing make clean in:  $abs_build_dir\n"

	if [ $debug -eq 1 ]
	    then
	    echo "/bglhome/jratt/install-sles10/bin/make clean"
	else
	    /bglhome/jratt/install-sles10/bin/make clean
	fi
    fi

    # --- Build binaries ---
    # Split up the make work
    MAKECPUS=$(grep -c '^processor' /proc/cpuinfo)

    if [ $debug -eq 1 ]
	then
	echo "/bglhome/jratt/install-sles10/bin/make -j $MAKECPUS | tee $make_log.dummy"
    else
	if [ $quietly -eq 1 ]
	    then	
	    /bglhome/jratt/install-sles10/bin/make -j $MAKECPUS >> "${make_log}"
	else
	    /bglhome/jratt/install-sles10/bin/make -j $MAKECPUS | tee "${make_log}"
	fi

	makeStatus=($( echo ${PIPESTATUS[*]} ))

	hput $exeHash "${TEST_ARRAY[$test]}:$test:compSignal" ${makeStatus[0]}

        # Check status of make
	if [ ${makeStatus[0]} -ne 0 ]
	    then
	    echo "ERROR (E): make FAILED!!"
	    cleanExit 1
	else # make passed, let's check the log
	    if [ ${makeStatus[1]} -ne 0 ] || [ ! -e "${make_log}" ]
		then
		echo "ERROR (E): FAILED to tee output into ${make_log}!!"
		cleanExit 1
	    fi
	fi

        # See which tests built. Disable tests that failed or are compile only
	for ((test=0; test < ${#TEST_ARRAY[@]}; test++))
	  do

	  hget $exeHash "${TEST_ARRAY[$test]}:$test:buildDir" buildDir

	  if [ ! -e "${buildDir}/${TEST_ARRAY[$test]}" ]
	      then
	      hput $exeHash "${TEST_ARRAY[$test]}:$test:status" 'Compile FAILED'
	      echo "Compile of ${buildDir}/${TEST_ARRAY[$test]} FAILED!!"
	 
      	      # Disable this test for the rest of this run
	      hput $exeHash "${TEST_ARRAY[$test]}:$test:exe" 0
	  else
	      hput $exeHash "${TEST_ARRAY[$test]}:$test:status" 'Compiled'
	      hget $exeHash "${TEST_ARRAY[$test]}:$test:compileOnly" compileOnly
	      if [ $compileOnly -eq 1 ]
		  then
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:exe" 0
	      fi
	  fi

	  # Update compile log
	  hput $exeHash "${TEST_ARRAY[$test]}:$test:logCompile" "${make_log}"
	done
    fi
fi # end compile section

# =========================================================
# COPY
# =========================================================
if [ $copy -eq 1 ]
    then

    echo -e "\n****************************" 
    echo "***                      ***"
    echo "***   Copying binaries   ***"
    echo "***                      ***"
    echo -e "****************************\n"

    # Remove reconfigure req, if we ever get a way to determine the compile floor without reconfiguring
    if [ $reconfigure -eq 1 ] && [ $compile -ne 1 ] # Already know this if we compiled during this run
	then
	if [ "${cur_floor}" != "${compile_floor}" ]
	    then
	    echo "WARNING (W): Floor mismatch:"
	    echo "WARNING (W): Latest floor      => ${cur_floor}"
	    echo "WARNING (W): Compiled w/ floor => ${compile_floor}"
	fi
    fi

    # --- Copy each binary to the exe dir ---
    echo "Copying binaries to exe dirs relative to: $exe_base"

    for ((test=0; test < ${#TEST_ARRAY[@]}; test++))
      do

      # Don't bother copying tests that are compile only
      hget $exeHash "${TEST_ARRAY[$test]}:$test:exe" hashExe
      if [ ${hashExe} -eq 0 ]
	  then
	  echo "Skipping copy of ${TEST_ARRAY[$test]} ..."
	  continue
      fi

      # Don't recopy tests
      hget $copyHash ${TEST_ARRAY[$test]} alreadyCopied
      if [ $alreadyCopied -eq 1 ]
	  then
	  echo "Already copied ${TEST_ARRAY[${test}]} ..."
	  hput $exeHash "${TEST_ARRAY[$test]}:$test:status" 'Copied'
	  continue
      fi

      hget $exeHash "${TEST_ARRAY[$test]}:$test:buildDir" buildDir
      hget $exeHash "${TEST_ARRAY[$test]}:$test:exeDir" exeDir

      
      # Create exe dir if it doesn't exist
      if [ ! -d "${exeDir}" ] 
	  then
	  echo "Creating exe dir: ${exeDir}"
	  mkdir -p "${exeDir}"

	  if [ $? -ne 0 ] || [ ! -d "${exeDir}" ]
	      then
	      echo "Creation of ${exeDir} FAILED!!"
	      cleanExit 1
	  fi
      fi

      cp "${buildDir}/${TEST_ARRAY[$test]}" "${exeDir}"
      if [ $? -ne 0 ] || [ ! -e "${exeDir}/${TEST_ARRAY[$test]}" ]
	  then
	  hput $exeHash "${TEST_ARRAY[$test]}:$test:status" 'Copy FAILED'
	  echo "Copy of ${buildDir}/${TEST_ARRAY[$test]} to ${exeDir} FAILED!!"
	  echo "Skipping to next test ..."

      	  # Disable this test for the rest of this run
	  hput $exeHash "${TEST_ARRAY[$test]}:$test:exe" 0
	  continue
      else
	  hput $exeHash "${TEST_ARRAY[$test]}:$test:status" 'Copied'
	  hput $copyHash ${TEST_ARRAY[$test]} 1 # Don't copy this test again
      fi
    done

    echo "Finished copying binaries."
fi

# =========================================================
# EXECUTE
# =========================================================
if [ $run -eq 1 ]
    then

    echo -e "\n******************************" 
    echo "***                        ***"
    echo "***    Running binaries    ***"
    echo "***                        ***"
    echo -e "******************************\n"

    if [ "${cur_floor}" == "${run_floor}" ]
	then 
	echo "Running with floor:  ${cur_floor}"
    else
	echo "WARNING (W): Floor mismatch:"
	echo "WARNING (W): Latest floor  => ${cur_floor}"
	echo "WARNING (W): Runtime floor => ${run_floor}"
    fi

    # Verify the binary exists if we didn't copy it during this run
    if [ $copy -ne 1 ]
	then
	for ((test=0; test < ${#TEST_ARRAY[@]}; test++))
	  do

          # Ensure test is enabled
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:exe" hashExe
	  if [ $hashExe -eq 1 ]
	      then

	      # Ensure file exists in exe dir
	      hget $exeHash "${TEST_ARRAY[$test]}:$test:exeDir" exeDir

	      if [ ! -e "${exeDir}/${TEST_ARRAY[$test]}" ]
		  then
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:status" 'DNE'
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:exe" 0
		  echo -e "\nERROR (E): ${exeDir}/${TEST_ARRAY[$test]} DNE!!\n"
		  echo "Skipping to next test ..."
	      fi
	  fi
	done
    fi

    # Convert overall status of enabled tests to a pass/total ratio
    for ((test=0; test < ${#TEST_ARRAY[@]}; test++))
      do

      # Ensure test is enabled
      hget $exeHash "${TEST_ARRAY[$test]}:$test:exe" hashExe
      if [ $hashExe -eq 1 ]
	  then # update overall status
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:runPass" runPass
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:runTotal" runTotal
	  
	  hput $exeHash "${TEST_ARRAY[$test]}:$test:status" "${runPass}/${runTotal} Passed"
      fi
    done

    # --- Run selected binaries ---
    for numNodes in ${nodeArray[@]}
      do
      for mode in ${modeArray[@]}
	do
	for ((test=0; test < ${#TEST_ARRAY[@]}; test++))
	  do

	  preProcRC=0
	  exeRC=0

	  # "Clear" runtime 
	  hput $exeHash "${TEST_ARRAY[$test]}:$test:runtime_n${numNodes}_m${mode}_p${numProcs}" ""

	  hget $exeHash "${TEST_ARRAY[$test]}:$test:status" overallStatus
	  if [ "${overallStatus}" == 'Bad Input Parm' ] || [ "${overallStatus}" == 'Compile Only' ]
	      then
	      continue # to next test
	  fi

	  # Determine NP
	  if [ $forceNP -eq 0 ]
	      then
	      numProcs=$(( $numNodes * $mode ))
	  else
	      numProcs=$forceNP
	  fi
	      
	  # Set default test scenario values
	  exeNodes=$numNodes # Numeric value of # of nodes sent to run* subroutine
	  eppInputMode=$mode # Alpha-numeric value of mode sent to exe_preProcessing subroutine
	  exeMode=$mode      # Numerical value of final mode used for math and documentation 
	  exeInputMode=$mode # Alpha-numeric value of final mode sent to run* subroutine
	  exeNP=$numProcs    # Numeric value of # of procs sent to run* subroutine

	  # Get parms for this test
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:exeDir" exeDir
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:runOpts" runOpts
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:exeArgs" exeArgs

	  # Remove until PAMI inserts driver level into binaries
          # Compare binary floor with runtime floor
#          bin_floor=$( strings "${exeDir}/${TEST_ARRAY[$test]}" | grep "SST_COMPILE_DRIVER" )
#          if [ "${bin_floor}" != "${run_floor}" ]
#	       then
#	       echo "WARNING (W): Floor mismatch:"
#	       echo "WARNING (W): Binary floor  => ${bin_floor}"
#	       echo "WARNING (W): Runtime floor => ${run_floor}"
#          fi

	  # Determine values for nodes, mode & NP
	  # Get text version of BGP mode
	  if [ "${platform}" == 'BGP' ]
	      then
	      bgp_mode_NumtoText $mode eppInputMode
	      if [ $? -ne 0 ]
		  then
		  echo "ERROR (E): bgp_mode_NumtoText subroutine FAILED!!"

		  # Amend overall status
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:status" 'Bad Input Parm'

	          # Disable test
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:exe" 0
		  continue
	      fi
	  fi

	  exe_preProcessing $numNodes $eppInputMode $numProcs "${runOpts}" exeNodes exeMode exeNP exeOpts exeOverride

	  preProcRC=$?
	  
	  if [ $preProcRC -ne 0 ]
	      then
	      echo -e "\nERROR (E):  exe_preProcessing sub FAILED!! Disabling ${TEST_ARRAY[$test]} test ...\n"
	      
	      # Amend overall status
	      hput $exeHash "${TEST_ARRAY[$test]}:$test:status" 'Bad Input Parm'

	      # Disable test
	      hput $exeHash "${TEST_ARRAY[$test]}:$test:exe" 0
	      continue
	  fi

	  exeInputMode=$exeMode
	  # Get text version of BGP exe mode
	  if [ "${platform}" == 'BGP' ]
	      then
	      bgp_mode_NumtoText $exeMode exeInputMode
	      if [ $? -ne 0 ]
		  then
		  echo "ERROR (E): bgp_mode_NumtoText subroutine FAILED!!"

		  # Amend overall status
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:status" 'Bad Input Parm'

	          # Disable test
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:exe" 0
		  continue
	      fi
	  fi	  

          # Document if final values contain override(s)
	  hput $exeHash "${TEST_ARRAY[$test]}:$test:override_n${numNodes}_m${mode}_p${numProcs}" $exeOverride

	  # Define output dir path
	  if [ $user_outdir -eq 0 ] 
	      then
	      # Use default output dir
	      out_dir="${exeDir}/results"

              # Create output dir if it doesn't exist
	      if [ ! -d "${out_dir}" ] 
		  then
		  echo "Creating results dir: ${out_dir}"
		  mkdir -p "${out_dir}"

		  if [ $? -ne 0 ] || [ ! -d "${out_dir}" ]
		      then
		      echo "Creation of ${out_dir} FAILED!!"
		      cleanExit 1
		  fi
	      fi
	  fi

          # Generate a unique logfile that contains the actual test scenario
	  if [ $debug -eq 1 ]
	      then
	      runLog="${out_dir}/${TEST_ARRAY[$test]%\.*}_$(date +%Y%m%d-%H%M%S).log.n${exeNodes}_m${exeMode}_p${exeNP}.$run_type.dummy"
	  else
	      runLog="${out_dir}/${TEST_ARRAY[$test]%\.*}_$(date +%Y%m%d-%H%M%S).log.n${exeNodes}_m${exeMode}_p${exeNP}.$run_type"
	  fi

	  while [ -e "${runLog}" ]; do
	      if [ $debug -eq 1 ]
		  then
		  runLog="${out_dir}/${TEST_ARRAY[$test]%\.*}_$(date +%Y%m%d-%H%M%S).log.n${exeNodes}_m${exeMode}_p${exeNP}.$run_type.dummy"
	      else
		  runLog="${out_dir}/${TEST_ARRAY[$test]%\.*}_$(date +%Y%m%d-%H%M%S).log.n${exeNodes}_m${exeMode}_p${exeNP}.$run_type"
	      fi
	  done

	  hput $exeHash "${TEST_ARRAY[$test]}:$test:log_n${numNodes}_m${mode}_p${numProcs}" $runLog

          # See if this is a rerun
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:combo_n${exeNodes}_m${exeMode}_p${exeNP}" combo

	  if [ "${combo}" == "1" ]
	      then
	      echo -e "Skipping rerun of ${TEST_ARRAY[$test]} (nodes = ${exeNodes}, mode = ${exeInputMode}, NP = ${exeNP})"
                 
	      # Set individual test status to 'Rerun'
	      hput $exeHash "${TEST_ARRAY[${test}]}:$test:status_n${numNodes}_m${mode}_p${numProcs}" 'Skipped (Rerun)'

	      # Ensure we report this right in the summary
	      {
		  echo "NP = $exeNP"
		  echo "NODES = $exeNodes"
		  if [ "${platform}" == 'BGP' ]
		      then
		      echo "MODE = $exeInputMode"
		  else
		      echo "BG_PROCESSESPERNODE = $exeInputMode"
		  fi
	      } >> $runLog

	      continue # to next test
	  fi

          # Update hash with final node, mode and NP values to compare with next iteration
	  hput $exeHash "${TEST_ARRAY[$test]}:$test:combo_n${exeNodes}_m${exeMode}_p${exeNP}" 1

          # See if test has been disabled since we last checked
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:exe" hashExe
	  if [ $hashExe -eq 0 ]
	      then
	      echo "Skipping execution of disabled test: ${TEST_ARRAY[$test]} (nodes = ${exeNodes}, mode = ${exeInputMode}, np = ${exeNP}) ..."

	      hput $exeHash "${TEST_ARRAY[$test]}:$test:status_n${numNodes}_m${mode}_p${numProcs}" 'Skipped (DOA)'

	      # Ensure we report this right in the summary
	      {
		  echo "NP = ${exeNP}"
		  echo "NODES = ${exeNodes}"
		  if [ "${platform}" == 'BGP' ]
		      then
		      echo "MODE = ${exeInputMode}"
		  else
		      echo "BG_PROCESSESPERNODE = ${exeInputMode}"
		  fi
	      } >> $runLog

	      continue # to next test
	  fi

          # Print test scenario to screen
	  if [ $exeOverride == 'Y' ]
	      then
	      echo -e "\nWARNING (W): Command line value(s) have been over-written by input file values for ${TEST_ARRAY[$test]}:"
	      echo "WARNING (W): Expected values: nodes = ${numNodes}, mode = ${eppInputMode}, NP = ${numProcs}"
	      echo "WARNING (W):   Actual values: nodes = ${exeNodes}, mode = ${exeInputMode}, NP = ${exeNP}"
	  else
	      echo -e "\nRunning test ${TEST_ARRAY[$test]} (nodes = ${exeNodes}, mode = ${exeInputMode}, np = ${exeNP}) ..."
	  fi

          # Call correct run script
	  if [ "${run_type}" != 'hw' ] 
	      then
	      runSim $run_type "${exeDir}" "${TEST_ARRAY[$test]}" "${exeNodes} ${exeMode} ${exeNP}" "${exeOpts}" "${exeArgs}" "${runLog}" exeSignal exeRuntime
	  else 
	      runHW "${exeDir}" "${TEST_ARRAY[$test]}" "${exeNodes} ${exeInputMode} ${exeNP}" "${exeOpts}" "${exeArgs}" "${runLog}" exeSignal exeRuntime
	  fi

	  exeRC=$?
	 
	  # Document signal and elapsed time for this run
	  if [ "${run_type}" != 'runFpga' ]
	      then
	      hput $exeHash "${TEST_ARRAY[$test]}:$test:signal_n${numNodes}_m${mode}_p${numProcs}" $exeSignal
	      hput $exeHash "${TEST_ARRAY[$test]}:$test:runtime_n${numNodes}_m${mode}_p${numProcs}" $exeRuntime

	      # Append run time to log file
	      echo "Elapsed time:  $exeRuntime" >> $runLog
	  else # FPGA run
	      hput $exeHash "${TEST_ARRAY[$test]}:$test:runtime_n${numNodes}_m${mode}_p${numProcs}" 'Funknown' # Threw an 'F' in there since I cut off the first runtime character when printing summary to preserve tabs

	      # Also document dummy fctest dir for FPGA debug runs
	      if [ $debug -eq 1 ]
		  then
		  temp_fctest=$(fgrep "Test directory:" "${runLog}" | awk '{print $3}')
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:FPGAfctestDir_n${numNodes}_m${mode}_p${numProcs}" $temp_fctest
	      fi
	  fi

          # Verify that the test "passed"
	  if [ $exeRC -eq 0 ]
	      then
	      if [ "${run_type}" == 'runFpga' ]
		  then 
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:status_n${numNodes}_m${mode}_p${numProcs}" 'Submitted'
		  fpga_queue=$(( $fpga_queue + 1 )) # total number of jobs submitted
	      else
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:status_n${numNodes}_m${mode}_p${numProcs}" 'Passed'
		  hget $exeHash "${TEST_ARRAY[$test]}:$test:runPass" runPass
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:runPass" $(( $runPass + 1 ))
	      fi

	      hget $exeHash "${TEST_ARRAY[$test]}:$test:status_n${numNodes}_m${mode}_p${numProcs}" hashStatus
	      echo "${TEST_ARRAY[$test]} ${hashStatus}."
	  else
	      hput $exeHash "${TEST_ARRAY[$test]}:$test:status_n${numNodes}_m${mode}_p${numProcs}" 'FAILED'
	      
	      # Disable this test if failure is NOT due to "block is not booted"
	      hget $exeHash "${TEST_ARRAY[$test]}:$test:log_n${numNodes}_m${mode}_p${numProcs}" runLog

	      if [ $( tail ${runLog} | grep -c "boot failed" ) -eq 0 ]
	          then
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:exe" 0
	      fi

	      echo "${TEST_ARRAY[$test]} FAILED!!"
	  fi # end status check   
	done # end test loop
      done # end mode loop
    done # end node loop
fi # end EXECUTE section

# --- Monitor/Control FPGA Loadleveler Jobs ---
if [ "${run_type}" == 'runFpga' ] && [ $fpga_queue -ne 0 ] && [ $debug -eq 0 ]
    then

    # Gather job ids and FCTest dirs of submitted jobs
    for ((test=0; test < ${#TEST_ARRAY[@]}; test++))
      do
      for numNodes in ${nodeArray[@]}
	do
	for mode in ${modeArray[@]}
	  do

	  if [ $forceNP -eq 0 ]
	      then
	      numProcs=$(( $numNodes * $mode ))
	  else
	      numProcs=$forceNP
	  fi

	  hget $exeHash "${TEST_ARRAY[$test]}:$test:status_n${numNodes}_m${mode}_p${numProcs}" hashStatus
	  if [ "${hashStatus}" == 'Submitted' ]
	      then
	  
	      hget $exeHash "${TEST_ARRAY[$test]}:$test:log_n${numNodes}_m${mode}_p${numProcs}" runLog

	      jobLine=$(fgrep "llsubmit" "${runLog}" | cut -d '"' -f2)
	      jobID=${jobLine%%.*} # set to everything up to first "."
	      jobID="${jobID}.${jobLine##*.}" # add everything after last "."
	      hput $exeHash "${TEST_ARRAY[$test]}:$test:FPGAjobID_n${numNodes}_m${mode}_p${numProcs}" $jobID


	      temp_fctest=$(fgrep "Test directory:" "${runLog}" | awk '{print $3}')
	      hput $exeHash "${TEST_ARRAY[$test]}:$test:FPGAfctestDir_n${numNodes}_m${mode}_p${numProcs}" $temp_fctest
	  fi
	done # end test loop
      done # end mode loop
    done # end node loop

    # See if any submitted jobs are still in the queue
    llJobs 'query'

    if [ $? -ne 0 ]
 	then
	echo -e "\nERROR (E):  ll jobs query FAILED!!\n"
    fi

    # Wait for any remaining jobs to complete
    if [ $fpga_queue -gt 0 ]
	then

	echo -e -n "\nWaiting for ${fpga_queue} job(s) to complete "

	while [ $fpga_queue -gt 0 ]
	  do
	  sleep 60
	  echo -n '.'
	  
          # See if any jobs are still in the queue
	  llJobs 'query'
	  if [ $? -ne 0 ]
	      then
	      echo -e "\nERROR (E):  ll jobs query FAILED!!\n"
	      break
	  fi
	done

	echo -e "\n"
    fi # end wait for jobs still in queue
fi # end monitor/control FPGA ll jobs

# =========================================================
# EVALUATE
# =========================================================
if [ $run -eq 1 ]
    then

    # Evaluate FPGA runs
    if [ $run -eq 1 ] && [ "${run_type}" == "runFpga" ]
	then

        # Append runtime data and determine results
	for ((test=0; test < ${#TEST_ARRAY[@]}; test++))
	  do
	  for numNodes in ${nodeArray[@]}
	    do
	    for mode in ${modeArray[@]}
	      do

	      if [ $forceNP -eq 0 ]
		  then
		  numProcs=$(( $numNodes * $mode ))
	      else
		  numProcs=$forceNP
	      fi  
	      
	      hget $exeHash "${TEST_ARRAY[$test]}:$test:status_n${numNodes}_m${mode}_p${numProcs}" hashStatus
	      if [ "${hashStatus}" == 'Submitted' ] || [ "${hashStatus}" == 'Cancelled' ]
		  then
		  hget $exeHash "${TEST_ARRAY[$test]}:$test:FPGAfctestDir_n${numNodes}_m${mode}_p${numProcs}" hashFctestDir
		  outfile="${hashFctestDir}/std.out"
	      else
		  continue
	      fi

              # Append FPGA runtime data to log file
	      if  [ -e "${outfile}" ]
		  then

		  hget $exeHash "${TEST_ARRAY[$test]}:$test:log_n${numNodes}_m${mode}_p${numProcs}" runLog

		  {
		      echo ""
		      echo "**************************"
		      echo "***   Runtime Output   ***"
		      echo "**************************"
		      echo "From ${outfile}:"
		      echo ""
		      cat "${outfile}"
		  } >> "${runLog}"

                  # Get rid of dummy output dir if it exists
		  if [ $debug -eq 1 ] && [ -e ${hashFctestDir} ]
		      then
		      rm -fr ${hashFctestDir}
		  fi
	      else
		  echo "ERROR (E): Runtime output file: ${outfile} DNE!! Skipping to the next test."
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:status_n${numNodes}_m${mode}_p${numProcs}" 'Output DNE'
		  continue
	      fi

              # Determine pass or fail of finished run
	      if [ "${hashStatus}" == 'Submitted' ]
		  then
	      
		  $(tail -10 $runLog | grep -q 'Software Test PASS')

		  if [ $? -eq 0 ]
		      then
		      hput $exeHash "${TEST_ARRAY[$test]}:$test:status_n${numNodes}_m${mode}_p${numProcs}" 'Passed'
		      hget $exeHash "${TEST_ARRAY[$test]}:$test:runPass" runPass
		      hput $exeHash "${TEST_ARRAY[$test]}:$test:runPass" $(( $runPass + 1 ))
		  else
		      hput $exeHash "${TEST_ARRAY[$test]}:$test:status_n${numNodes}_m${mode}_p${numProcs}" 'FAILED'
		  fi
	      fi
	    done # mode loop
	  done # node loop
	done # test loop
    fi # end FGPA eval
   
    # Update overall status of all executed tests
    for ((test=0; test < ${#TEST_ARRAY[@]}; test++))
      do
  
      hget $exeHash "${TEST_ARRAY[$test]}:$test:status" hashStatus
      hget $exeHash "${TEST_ARRAY[$test]}:$test:runTotal" runTotal
      hget $exeHash "${TEST_ARRAY[$test]}:$test:runPass" runPass

      # Overall status indicates this test reached the execution stage
      if [[ "${hashStatus}" =~ ^[0-9] ]]    
	  then
	  hput $exeHash "${TEST_ARRAY[$test]}:$test:status" "${runPass}/${runTotal} Passed"
      fi
    done
fi

# =========================================================  
# Log to Web
# =========================================================
if [ $web_update -eq 1 ]
    then
    echo -e "\nLogging results to PAMI Test History webpage ..."

    for ((test=0; test < ${#TEST_ARRAY[@]}; test++))
      do

      logSummary=""
      comp_drv=""

      # Create test name to be displayed in DB
      testName=""

      # Grab rel and abs exe paths
      hget $exeHash "${TEST_ARRAY[$test]}:$test:stub" relPath
      hget $exeHash "${TEST_ARRAY[$test]}:$test:exeDir" exeDir

      if [ "${testNameLength}" == 'short' ]
	  then
	  testName="${TEST_ARRAY[$test]}"
      else
	  testName="${relPath}/${TEST_ARRAY[$test]}"
      fi

      # Grab overall status of this test      
      hget $exeHash "${TEST_ARRAY[$test]}:$test:status" hashStatus

      # Overall status indicates this test reached the execution stage
      if [[ "${hashStatus}" =~ ^[0-9] ]] 
	  then # Determine exe parms

	  # Determine compile driver level from binary in exe dir
#	  comp_drv=$( strings "${exeDir}/${TEST_ARRAY[$test]}" | grep "SST_COMPILE_DRIVER" )
#	  comp_drv=${comp_drv##*=}

	  # Determine run driver level (2nd to last dir in floor)
	  run_drv=${run_floor%/*} # Get rid of last dir
	  run_drv=${run_drv##*/} # only keep newest last dir (driver)

	  for numNodes in ${nodeArray[@]}
	    do
	    for mode in ${modeArray[@]}
	      do
	      
	      if [ $forceNP -eq 0 ]
		  then
		  numProcs=$(( $numNodes * $mode ))
	      else
		  numProcs=$forceNP
	      fi

	      # Grab individual run status
	      hget $exeHash "${TEST_ARRAY[$test]}:$test:status_n${numNodes}_m${mode}_p${numProcs}" runStatus
	      if [[ "${runStatus}" =~ 'Skipped' ]]
		  then 
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${numNodes}_m${mode}_p${numProcs}" 'N/A'
		  continue # to next node/mode combo
	      else
		  # Grab individual run log
		  hget $exeHash "${TEST_ARRAY[$test]}:$test:log_n${numNodes}_m${mode}_p${numProcs}" runLog
	      fi

	      # Determine np
	      np=$( grep "^NP =" ${runLog} | awk '{print $3}')

              # Determine procs/node
	      if [ "${platform}" == 'BGP' ]
		  then
		  ppn=$( grep "^MODE =" ${runLog} | awk '{print $3}')
		  	  
                  # Get numerical version of mode
		  bgp_mode_TexttoNum $ppn ppn

		  if [ $? -ne 0 ]
		      then
		      echo "ERROR (E): bgp_mode_TexttoNum subroutine FAILED!!"
		      hput $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${numNodes}_m${mode}_p${numProcs}" 'FAILED'
		      continue # to next node/mode combo
		  fi
	      else
		  ppn=$( grep "^BG_PROCESSESPERNODE =" ${runLog} | awk '{print $3}')
	      fi

              # Determine number of threads
	      threads=$( grep "^THREADS =" ${runLog} | awk '{print $3}')

              # Determine exe return code
	      if [ "${platform}" == 'BGP' ]
		  then
		  hget $exeHash "${TEST_ARRAY[$test]}:$test:signal_n${numNodes}_m${mode}_p${numProcs}" signal
	      else 
		  # Get parms needed to determine passage
		  nodes=$( grep "^NODES =" ${runLog} | awk '{print $3}')
		  passCount=$( grep -c 'Software Test PASS' ${runLog} )
	      
	          # Use -ge since FPGA runs return more "passes" than nodes
		  if [ $passCount -ge $nodes ]  
		      then 
		      signal=0
		  else
		      # Look for failing tests
		      signal=$( grep -m 1 'Software Test FAIL' ${runLog} | awk '{print $NF}' )
		      if [ $? -ne 0 ] || [ "${signal}" == "" ]
			  then # This test didn't exit normally, check for timeout or boot fail
			  if [ $( tail ${runLog} | grep -c "^Timeout reached" ) -eq 1 ]
			      then 
			      signal="9 -s \"MMCS-Lite Timeout\"" # SIGKILL
			  elif [ $( tail ${runLog} | grep -c "boot failed" ) -gt 0 ]
			      then 
			      signal="9 -s \"Boot FAILED for $block\"" # SIGKILL
			  else # I give up
			      signal=10 # SIGUSR1
			  fi
		      fi
		  fi
	      fi   

	      # Create logXML command
	      logCmd="${logXml} -e ${testName} -n ${np} -p ${ppn} -t ${threads} -r ${signal} -u ${USER} -m ${serverID} -o ${runLog}"

	      if [ "${comp_drv}" != "" ]
		  then
		  logCmd="${logCmd} -c ${comp_drv}"
	      fi

	      if [ "${run_drv}" != "" ]
		  then
		  logCmd="${logCmd} -d ${run_drv}"
	      fi

	      if [ "${logSummary}" != "" ]
		  then
		  logCmd="${logCmd} -s \"${logSummary}\""
	      fi

	      # Format group name 
	      groupMode="${mode}m" # Default

	      # Get text version of BGP mode
	      if [ "${platform}" == 'BGP' ]
		  then
		  bgp_mode_NumtoText $mode groupMode
		  if [ $? -ne 0 ]
		      then
		      echo "ERROR (E): bgp_mode_NumtoText subroutine FAILED!!"
		      echo "ERROR (E): Using numeric mode value in group name."
		  fi
	      fi

	      # Group name was specified
	      if [ "${logGroup}" != "" ]
		  then
		  if [ $groupScale -eq 1 ] # Add scaling info to given group name
		      then
		      tempGroup="${logGroup}_${numNodes}n_${groupMode}.${timestamp}"
		  else # Just use group name specified by user
		      tempGroup="${logGroup}.${timestamp}"
		  fi
		  
		  logCmd="${logCmd} -g ${tempGroup}"

	      elif [ $autoGroup -eq 1 ] # Generate group name w/ scaling info
		  then
		  input=$( basename $file )
		  tempGroup="${input/.*/}_${groupDev}_${numNodes}n_${groupMode}.${timestamp}"
		  logCmd="${logCmd} -g ${tempGroup}"
	      fi

	      echo ${logCmd}
	      	      
	      # Execute logXML command
	      if [ $logDisabled -eq 0 ]
		  then
		  if [ $debug -ne 1 ]
		      then
		      eval $logCmd
		      if [ $? -ne 0 ]
			  then
		      
			  if [ $? -eq 2 ]
			      then
			      logDisabled=1
			  fi

			  echo "ERROR (E): logXML.sh FAILED for ${TEST_ARRAY[$test]}"
			  hput $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${numNodes}_m${mode}_p${numProcs}" 'FAILED'
		      else
			  hput $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${numNodes}_m${mode}_p${numProcs}" 'Logged'
		      fi
		  else # debug mode
		      hput $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${numNodes}_m${mode}_p${numProcs}" 'N/A'  
		  fi
	      else # Logging disabled
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${numNodes}_m${mode}_p${numProcs}" 'Skipped' 
	      fi

	    done # end mode loop
	  done # end node loop
      else # Scaling not a factor ...Didn't attempt to execute this test

	  # Determine NP value for saving XML status
	  if [ $forceNP -eq 0 ]
	      then
	      numProcs=$(( ${nodeArray[0]} * ${modeArray[0]} ))
	  else
	      numProcs=$forceNP
	  fi

	  # Log compile status
	  if [[ "${hashStatus}" == 'Compile FAILED' ]] || [[ $compile -eq 1 && $forceWeb -eq 1 ]]
	      then
	      
	      # Determine compile driver level from binary in build dir
#	      comp_drv=$( strings "${BUILD_DIR_ARRAY[$test]}/${TEST_ARRAY[$test]}" | grep "SST_COMPILE_DRIVER" )
#	      comp_drv=${comp_drv##*=}

	      np=0
	      ppn=0
	      threads=0  
		 
	      hget $exeHash "${TEST_ARRAY[$test]}:$test:logCompile" compileLog

	      if [ "${hashStatus}" == 'Compile FAILED' ]
		  then
		  signal=$( tail $compileLog | grep [Ee]rror | awk '{print $NF}')

		  if [ $? -ne 0 ] || [ "${signal}" == "" ] || [[ ! "${signal}" =~ ^[0-9] ]] 
		      then # This compile didn't exit normally
		      signal=10 # SIGUSR1
		  fi
	      else
		  signal=0
	      fi

	      logSummary="${hashStatus}"

	      logCmd="${logXml} -e ${testName} -n ${np} -p ${ppn} -t ${threads} -r ${signal} -u ${USER} -m ${serverID} -o ${compileLog}"

	      if [ "${comp_drv}" != "" ]
		  then
		  logCmd="${logCmd} -c ${comp_drv}"
	      fi

	      if [ "${logSummary}" != "" ]
		  then
		  logCmd="${logCmd} -s \"${logSummary}\""
	      fi
		  
	      # Format group name 
	      groupMode="${modeArray[0]}m" # Default

	      # Get text version of BGP mode
	      if [ "${platform}" == 'BGP' ]
		  then
		  bgp_mode_NumtoText ${modeArray[0]} groupMode
		  if [ $? -ne 0 ]
		      then
		      echo "ERROR (E): bgp_mode_NumtoText subroutine FAILED!!"
		      echo "ERROR (E): Using numeric mode value in group name."
		  fi
	      fi

	      # Group name was specified
	      if [ "${logGroup}" != "" ]
		  then
		  if [ $groupScale -eq 1 ] # Add scaling info to given group name
		      then
		      tempGroup="${logGroup}_${nodeArray[0]}n_${groupMode}.${timestamp}"
		  else # Just use group name specified by user
		      tempGroup="${logGroup}.${timestamp}"
		  fi

		  logCmd="${logCmd} -g ${tempGroup}"

	      elif [ $autoGroup -eq 1 ] # Generate group name w/ scaling info
		  then
		  input=$( basename $file )
		  tempGroup="${input/.*/}_${groupDev}_${nodeArray[0]}n_${groupMode}.${timestamp}"
		  logCmd="${logCmd} -g ${tempGroup}"
	      fi
	      
	      echo ${logCmd}
	      
	      # Execute logXML command
	      if [ $logDisabled -eq 0 ]
		  then
		  if [ $debug -ne 1 ]
		      then
		      eval $logCmd
		      if [ $? -ne 0 ]
			  then
			  
			  if [ $? -eq 2 ]
			      then
			      logDisabled=1
			  fi

			  echo "ERROR (E): logXML.sh FAILED for ${TEST_ARRAY[$test]}"
			  hput $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${nodeArray[0]}_m${modeArray[0]}_p${numProcs}" 'FAILED'
		      else 
			  hput $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${nodeArray[0]}_m${modeArray[0]}_p${numProcs}" 'Logged'
		      fi
		  else # debug mode
		      hput $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${nodeArray[0]}_m${modeArray[0]}_p${numProcs}" 'N/A'  
		  fi
	      else # Logging disabled
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${nodeArray[0]}_m${modeArray[0]}_p${numProcs}" 'Skipped' 
	      fi
	  
	  else # Overall status for this test = Compiled, Copied, Skipped, DNE, Rerun 
	      hput $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${nodeArray[0]}_m${modeArray[0]}_p${numProcs}" 'N/A'
	  fi # End compile fail or not compile fail
      fi # End exe or not exe      
    done # end test loop
fi

# =========================================================  
# Summarize results into a file
# =========================================================

echo "Creating summary file ..."

{
    echo -e "\nTEST\t\t\t\t\t\t\tSYSTEM\tNP\tMODE\tOVRRIDE\tSTATUS\t\tRUNTIME\tXML\tLOG"
    echo -e "======================================================\t=======\t=======\t=======\t=======\t===============\t=======\t=======\t=====================================================================================" 
} >> $summaryFile


for ((test=0; test < ${#TEST_ARRAY[@]}; test++))
  do

  # Create test name to be displayed in DB
  testName=""

  # Grab rel and abs exe paths
  hget $exeHash "${TEST_ARRAY[$test]}:$test:stub" relPath
  hget $exeHash "${TEST_ARRAY[$test]}:$test:exeDir" exeDir

  if [ "${testNameLength}" == 'short' ]
      then
      testName="${TEST_ARRAY[$test]}"

      # Print testname to summary file
      if [ ${#testName} -lt 8 ]
          then
	  echo -e -n "${testName}\t\t\t\t\t" >> $summaryFile
      elif [ ${#testName} -lt 16 ]
          then
	  echo -e -n "${testName}\t\t\t\t" >> $summaryFile
      elif [ ${#testName} -lt 24 ]
          then
	  echo -e -n "${testName}\t\t\t" >> $summaryFile
      elif [ ${#testName} -lt 32 ]
          then
	  echo -e -n "${testName}\t\t" >> $summaryFile
      else # < 40
	  echo -e -n "${testName}\t" >> $summaryFile
      fi

  else
      testName="${relPath}/${TEST_ARRAY[$test]}"

      # Print testname to summary file
      if [ ${#testName} -lt 8 ]
          then
	  echo -e -n "${testName}\t\t\t\t\t\t\t" >> $summaryFile
      elif [ ${#testName} -lt 16 ]
          then
	  echo -e -n "${testName}\t\t\t\t\t\t" >> $summaryFile
      elif [ ${#testName} -lt 24 ]
          then
	  echo -e -n "${testName}\t\t\t\t\t" >> $summaryFile
      elif [ ${#testName} -lt 32 ]
          then
	  echo -e -n "${testName}\t\t\t\t" >> $summaryFile
      elif [ ${#testName} -lt 40 ]
          then
	  echo -e -n "${testName}\t\t\t" >> $summaryFile
      elif [ ${#testName} -lt 48 ]
          then
	  echo -e -n "${testName}\t\t" >> $summaryFile
      else # < 56
          echo -e -n "${testName}\t" >> $summaryFile
      fi
  fi

  # Determine system test was run on:
  if [ "${run_type}" == 'runMambo' ] || [ "${run_type}" == 'runFpga' ]
      then
      system=${run_type#run}
  else
      system='HW'
  fi    
  
  # Since FPGA is an acronymn lets capitalize it
  if [ "${run_type}" == 'runFpga' ]
      then 
      system=$( echo $system | awk '{print toupper($0)}' )
  fi

  hget $exeHash "${TEST_ARRAY[$test]}:$test:status" hashStatus
  hget $exeHash "${TEST_ARRAY[$test]}:$test:runTotal" runTotal

  # Overall status indicates this test reached the execution stage
  if [[ "${hashStatus}" =~ ^[0-9] ]]    
      then

      # No scaling
      if [ ${runTotal} -eq 1 ]
	  then

	  # Print System, NP, mode and status to summary file
	  if [ $forceNP -eq 0 ]
	      then
	      numProcs=$(( ${nodeArray[0]} * ${modeArray[0]} ))
	  else
	      numProcs=$forceNP
	  fi

          # Get values straight from the horse's mouth
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:override_n${nodeArray[0]}_m${modeArray[0]}_p${numProcs}" override
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:log_n${nodeArray[0]}_m${modeArray[0]}_p${numProcs}" runLog

          # Determine NP
	  np=$( grep "^NP =" ${runLog} | awk '{print $3}')

          # Determine procs/node
	  if [ "${platform}" == 'BGP' ]
	      then
	      ppn=$( grep "^MODE =" ${runLog} | awk '{print $3}')
	  else
	      ppn=$( grep "^BG_PROCESSESPERNODE =" ${runLog} | awk '{print $3}')
	  fi

	  # FPGA status could be Cancelled, Passed or FAILED
	  if [ "${run_type}" == 'runFpga' ]
	      then
 
	      hget $exeHash "${TEST_ARRAY[$test]}:$test:status_n${nodeArray[0]}_m${modeArray[0]}_p${numProcs}" fpgaStatus
	      echo -e -n "${system}\t${np}\t${ppn}\t${override}\tExe $fpgaStatus" >> $summaryFile

	  else # MMCS_Lite and Mambo runs are only Pass/FAIL
	      
	      if [[ "${hashStatus}" =~ ^0 ]]
		  then

		  echo -e -n "${system}\t${np}\t${ppn}\t${override}\tExe FAILED" >> $summaryFile
	      else
		  echo -e -n "${system}\t${np}\t${ppn}\t${override}\tExe Passed" >> $summaryFile
	      fi
	  fi
	  
	  # Print run time, XML status and log file to summary file
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:runtime_n${nodeArray[0]}_m${modeArray[0]}_p${numProcs}" runtime
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${nodeArray[0]}_m${modeArray[0]}_p${numProcs}" xmlStatus

	  if [ ${#hashStatus} -lt 8 ]
	      then
	      echo -e "\t\t${runtime:1:7}\t${xmlStatus}\t${runLog}" >> $summaryFile
	  else
	      echo -e "\t${runtime:1:7}\t${xmlStatus}\t${runLog}" >> $summaryFile 
	  fi

      # Scaling  
      else
	  # Print system and overall status to summary file
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:runPass" runPass
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:runTotal" runTotal
	  echo -e "${system}\t\t\t\t${runPass}/${runTotal} Passed" >> $summaryFile
	  
	  # Print individual stati to summary file
	  for numNodes in ${nodeArray[@]}
	    do
	    for mode in ${modeArray[@]}
	      do

	      if [ $forceNP -eq 0 ]
		  then
		  numProcs=$(( $numNodes * $mode ))
	      else
		  numProcs=$forceNP
	      fi

	      hget $exeHash "${TEST_ARRAY[$test]}:$test:status_n${numNodes}_m${mode}_p${numProcs}" hashStatus
      	      hget $exeHash "${TEST_ARRAY[$test]}:$test:override_n${numNodes}_m${mode}_p${numProcs}" override
	      hget $exeHash "${TEST_ARRAY[$test]}:$test:runtime_n${numNodes}_m${mode}_p${numProcs}" runtime
	      hget $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${numNodes}_m${mode}_p${numProcs}" xmlStatus
	      hget $exeHash "${TEST_ARRAY[$test]}:$test:log_n${numNodes}_m${mode}_p${numProcs}" runLog

	      if [ -e "${runLog}" ]
		  then
		  # Determine NP
		  np=$( grep "^NP =" ${runLog} | awk '{print $3}')

                  # Determine procs/node
		  if [ "${platform}" == 'BGP' ]
		      then
		      ppn=$( grep "^MODE =" ${runLog} | awk '{print $3}')
		  else
		      ppn=$( grep "^BG_PROCESSESPERNODE =" ${runLog} | awk '{print $3}')
		  fi
	      else
		  np=$numProcs
		  ppn=$mode
		  	
                  # Get text version of BGP mode
		  if [ "${platform}" == 'BGP' ]
		      then
		      bgp_mode_NumtoText ${mode} ppn
		      if [ $? -ne 1 ]
			  then
			  echo "ERROR (E): bgp_mode_NumtoText subroutine FAILED!!"
			  echo "ERROR (E): Using numeric mode value in summary."
		      fi
		  fi
	      fi
	      
              # Print NP, mode, status to summary file
	      if [ "${testNameLength}" == 'short' ]
                  then
		  echo -e -n "\t\t\t\t\t\t${np}\t${ppn}\t${override}\t${hashStatus}" >> $summaryFile
	      else
		  echo -e -n "\t\t\t\t\t\t\t\t${np}\t${ppn}\t${override}\t${hashStatus}" >> $summaryFile
	      fi

	      # Print run time & XML status to summary file
	      if [ ${#hashStatus} -lt 8 ]
		  then
		  echo -e -n "\t\t${runtime:1:7}\t${xmlStatus}" >> $summaryFile
	      else
		  echo -e -n "\t${runtime:1:7}\t${xmlStatus}" >> $summaryFile 
	      fi

	      # Print log file to summary file
	      if [[ ! "${hashStatus}" =~ 'Skipped' ]]
		  then
		  echo -e "\t${runLog}" >> $summaryFile
	      else
		  # Erase this log (only needed for test scenario values)
		  if [ -e ${runLog} ]
		      then
		      rm -fr ${runLog}
		  fi

		  echo -e "\n" >> $summaryFile 
	      fi
	    done # mode loop
	  done # np loop
      fi
  else  # Overall status is Skipped, Compiled, Copy, DNE, Rerun

      # Don't worry about the system if we didn't do more than compile
      if [[ "${hashStatus}" =~ 'Compile' ]]
	      then 
	  echo -e -n "\t\t\t\t${hashStatus}" >> $summaryFile
      else
	  echo -e -n "${system}\t\t\t\t${hashStatus}" >> $summaryFile
      fi

      # If a compile was performed during this run, get the log path
      if [ $compile -eq 1 ] 
      then 
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:logCompile" compileLog

	  if [ ${#hashStatus} -lt 8 ]
	      then
	      echo -e "\t\t\t\t${compileLog}" >> $summaryFile
	  else
	      echo -e "\t\t\t${compileLog}" >> $summaryFile
	  fi
      else
	  echo -e "\n" >> $summaryFile
      fi
  fi
done # test loop

# =========================================================  
# Email summary
# =========================================================

if [ "${emailAddr}" != "" ]
    then
    echo "Emailing summary file ..."

    # Create email file
    echo -e "Subject: pami_verif Summary\n" >> $summaryEmail

    # Add user comment
    if [ "${comment}" != "" ]
	then
	echo -e "${comment}\n" >> $summaryEmail
    fi

    # Add summary file
    cat $summaryFile >> $summaryEmail

    # Add closing
    echo -e "\nThis notification email was automatically generated by pami_verif.sh\n" >> $summaryEmail
    echo "To unsubscribe to this email talk to the sender or don't use the -email option next time." >> $summaryEmail

    # Send email and update summary file
    /usr/sbin/sendmail <$summaryEmail $emailAddr
    if [ $? -ne 0 ]
	then
	echo -e "\nAttempt to send summary email (${summaryEmail}) to ${emailAddr} FAILED with rc = $?!!" >> $summaryFile
    else
	echo -e "\nSuccesfully emailed summary to ${emailAddr}" >> $summaryFile
    fi

    # Delete the email file
    rm $summaryEmail
    if [ $? -ne 0 ]
	then
	echo "ERROR (E): Attempt to delete email file: ${summaryEmail} FAILED with rc = $?!!"
    fi
fi

# =========================================================  
# Print summary to screen
# =========================================================
cat $summaryFile 

# Delete the summary file
rm $summaryFile
if [ $? -ne 0 ]
    then
    echo "ERROR (E): Attempt to delete summary file: ${summaryFile} FAILED with rc = $?!!"
fi

cleanExit 0

# issue 2 beeps indicating test complete
if [[ A1 = A$ding ]]; then echo -en "\007"; sleep 1; echo -en "\007"; fi

