#!/bin/bash

trap "cleanExit" EXIT

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

    putHashFile=$1
    putKey=$2
    putValue=$3

    putLockFile="${putHashFile}.lock"
    curValue=''
 
    # Lock the hash for parallel processes
    if [ $hashLock -eq 1 ]
    then
	while ( ! ( set -o noclobber; (echo -e $mypid > "${putLockFile}")  >/dev/null 2>&1 ) )
	do
	    usleep 100000
	done
    fi

    # If file exists, see if key already exists
    if [ -e "${putHashFile}" ] 
    then 
#	hvalue=\"$( grep -a --max-count=1 "^${2} " $1 | awk '{ print substr($0, (index($0," ")+1)) }' )\"
	hget "${putHashFile}" "${putKey}" curValue
	curValue=$( echo $curValue | sed 's/"//g' )

        # Append or replace key/value pair
	if [ "${curValue}" == '' ] # key doesn't exist 
	then # append
	    echo "${putKey}" "${putValue}" >> "${putHashFile}"
	else 
	    # replace old value with new one
	    # Use "," as the delimiter so we don't have to escape all "/"
	    sed -i "s,${putKey} ${curValue},${putKey} ${putValue}," $putHashFile
	fi

    else # create new hash file
	echo "${putKey}" "${putValue}" > "${putHashFile}"
    fi

    if [ $hashLock -eq 1 ]
    then
	rm -f "${putLockFile}"
    fi

    return 0
}

# HASH GET VALUE 
hget () 
{
    getHashFile=$1
    getKey=$2
    getValueVar=$3

    getLockFile="${getHashFile}.lock"
    nestedRead=0
    rc=0

    # Lock the hash for parallel processes
    if [ $hashLock -eq 1 ]
    then

	grepAttempted=0

	while ( ! ( set -o noclobber; (echo -e $mypid > "${getLockFile}")  >/dev/null 2>&1 ) )
	do
	    if [ $grepAttempted -eq 0 ]
	    then
	        # If my process ID = process ID in the lock file, then this is a nested read
		$( grep -a -q $mypid $getLockFile )

		if [ $? -eq 0 ]
		then
		    nestedRead=1
		    break
		fi
	    else
		usleep 100000
	    fi
	done
    fi

    eval $getValueVar=\"$( grep -a --max-count=1 "^${getKey} " $getHashFile | awk '{ print substr($0, (index($0, " ")+1)) }' )\"

    if [ $? -ne 0 ]
    then
	echo "ERROR (E)::hget:  ${getKey} key DNE in ${getHashFile}"
	rc=-1
    fi
    
    if [ $hashLock -eq 1 ] && [ $nestedRead -eq 0 ]
    then
	rm -f "${getLockFile}"
    fi

    return $rc
}

hdelete ()
{
    delHashFile=$1
    delKey=$2

    delLockFile="${delHashFile}.lock"

    # Lock the hash file for parallel processes
    if [ $hashLock -eq 1 ]
    then
	while ( ! ( set -o noclobber; (echo -e $mypid > "${delLockFile}")  >/dev/null 2>&1 ) )
	do
	    usleep 100000
	done
    fi

    sed -i "/${delKey}/d" $delHashFile
	   
    if [ $hashLock -eq 1 ]
    then
	rm -f $delLockFile
    fi
}

hdestroy ()
{
    destHashFile=$1

    destLockFile="${destHashFile}.lock"
    rc=0

    # Lock the hash file
    if [ $hashLock -eq 1 ]
    then
	while ( ! ( set -o noclobber; (echo -e $mypid > "${destLockFile}")  >/dev/null 2>&1 ) )
	do
	    usleep 100000
	done
    fi

    if [ -e $destHashFile ]
    then
	rm -f $destHashFile
    else
	echo "ERROR (E)::hdestroy: ${destHashFile} DNE !!"
	rc=1
    fi
	
    if [ $hashLock -eq 1 ]
    then
	rm -f $destLockFile
    fi

    return $rc
}

# Kill any existing child processes
childKill ()
{
    # send SIGTERM (15) signal to all child processes
    childProcesses="/tmp/$ppid.children"
    pgrep -P $ppid > $childProcesses
    children=$(wc -l < $childProcesses)
    
    if [ $children -gt 0 ]
    then
	echo -e "\nSending SIGTERM signal to ${children} child process(es) ..."
	pkill -SIGTERM -P $ppid #>/dev/null 2>&1

        # wait for all sub processes to exit
	echo "Waiting for ${children} child processes to terminate ..."

	while [ $children -gt 0 ]
	do
	    prevChildren=$children

	    sleep 10

	    pgrep -P $ppid > $childProcesses
	    children=$(wc -l < $childProcesses)

	    if [ $children -ne $prevChildren ] && [ $children -gt 0 ]
	    then
		echo "Waiting for ${children} child process(es) to terminate ..."
	    fi
	done
    fi

    if [ -e $childProcesses ]
    then
	rm -f $childProcesses
    fi
}

# Cleanup all the hash files in /tmp dir
cleanExit ()
{

    # Cancel any FPGA jobs
    if [ "${run_type}" == 'runFpga' ]
    then
	llJobs 'cancel'
    fi

    # Remove compile info hash and lock file
    if [ -e $compileHash ]
    then
	rm -f $compileHash
    fi

    if [ -e "${compileHash}.lock" ]
    then
	rm -f "${compileHash}.lock"
    fi

    # Remove copy info hash and lock file
    if [ -e $copyHash ]
    then
	rm -f $copyHash
    fi

    if [ -e "${copyHash}.lock" ]
    then
	rm -f "${copyHash}.lock"
    fi

    # Remove exe info hash and lock file
    if [ -e $exeHash ]
    then
	rm -f $exeHash
    fi

    if [ -e "${exeHash}.lock" ]
    then
	rm -f "${exeHash}.lock"
    fi

    # Remove block info hash and lock file
    if [ -e $blockHash ]
    then
	rm -f $blockHash
    fi

    if [ -e "${blockHash}.lock" ]
    then
	rm -f "${blockHash}.lock"
    fi

    childKill
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
# runEval input parms
# $1 - Test is/not standalone
# $2 - The overall RC of the run subroutine (used to verify system/file)
# $3 - The *run* signal (used to verify the run ...boot, bin, exit)
# $4 - The log file (used to verify the binary itself)
# $5 - The variable to store the logXML signal for this test scenario
# $6 - The variable to store the logXML summary for this test scenario
#-------------------------------------------------------------------------------
runEval ()
{

evalSA=$1
runSubRC=$2
runSig=$3
testLog=$4
outLogSigVar=$5
outLogSumVar=$6

evalSignal=999
evalSummary=""
 
# Verify standalone test
if [ $evalSA -eq 1 ]
then
    if [ $runSig -eq 0 ] # signal from binary = 0
    then
	evalSignal=$runSubRC 
    else
	evalSignal=$runSig
    fi
# Verify MPICH test
elif [ $mpich -eq 1 ]
then
    if [ -e ${testLog} ]
    then
	$( grep -a -q -m 1 'No Errors' ${testLog} )
    
	evalSignal=$? # 0 if it found it

	if [ $evalSignal -eq 0 ] # MPICH binary passed
	then	
	    if [ $runSig -eq 0 ] # *run* passed 
	    then 
		evalSignal=$runSubRC
	    else
		evalSignal=$runSig
	    fi
	else # Find source of MPICH binary FAIL
	    if [ "${run_type}" != 'hw' ]
		then
		evalSignal=$( grep -a -m 1 'Software Test FAIL' ${testLog} | awk '{print $NF}' )
		if [ $? -ne 0 ] || [ "${evalSignal}" == "" ] || [[ ! "${evalSignal}" =~ ^[0-9] ]]
		then # This test didn't exit normally, check for timeout or boot fail
		    if [ $( tail ${testLog} | grep -a -c "^Timeout reached" ) -eq 1 ]
		    then 
			evalSignal=9 # SIGKILL
			evalSummary="MMCS-Lite Timeout"
		    elif [ $( tail ${testLog} | grep -a -c "boot failed" ) -gt 0 ]
		    then 
			evalSignal=9 # SIGKILL
			evalSummary="Boot FAILED for ${block}" 
		    else # I give up
			evalSignal=10 # SIGUSR1
		    fi
		fi
	    fi
	fi
    else # Log file DNE
	if [ $runSig -eq 0 ] # *run* passed 
	then 
	    evalSignal=$runSubRC
	else
	    evalSignal=$runSig
	fi	
    fi
	
# Verify BGP test    
elif [ "${platform}" == 'bgp' ]
then
    if [ $runSig -eq 0 ] # *run* passed 
    then 
	evalSignal=$runSubRC
    else
	evalSignal=$runSig
    fi
# Verify BGQ test
else
    # Verify binary status from HW run
    if [ "${run_type}" == 'hw' ]
    then
	if [ -e $testLog ]
	then
	    evalSignal=$( grep -a -m 1 'exited with status' ${testLog} | awk '{print $NF}' )
	    if [ $? -ne 0 ] || [ "${evalSignal}" == "" ] || [[ ! "${evalSignal}" =~ ^[0-9] ]]
	    then # check for termination

		evalSignal=$( grep -a -m 1 'terminated by signal' ${testLog} | awk '{print $NF}' )
		if [ $? -ne 0 ] || [ "${evalSignal}" == "" ] || [[ ! "${evalSignal}" =~ ^[0-9] ]]
		then # This test didn't exit normally, check for timeout or boot fail

		    if [ $( tail ${testLog} | grep -a -i -c "timed out" ) -eq 1 ]
		    then 
			evalSignal=9 # SIGKILL
			evalSummary="Timeout"
		    elif [ $( tail ${testLog} | grep -a -i -c "block is busy" ) -gt 0 ]
		    then 
			evalSignal=9 # SIGKILL
			evalSummary="Block is busy"
		    else # I give up
			evalSignal=10 # SIGUSR1
		    fi
		fi
	    fi

	    if [ $evalSignal -eq 0 ] # BGQ binary passed
 	    then  # Verify run status
		if [ $runSig -eq 0 ] # *run* passed 
		then 
		    evalSignal=$runSubRC
		else
		    evalSignal=$runSig
		fi
	    fi
	else # Log file DNE
	    if [ $runSig -eq 0 ] # *run* passed 
	    then 
		evalSignal=$runSubRC
	    else
		evalSignal=$runSig
	    fi
	fi
    else # Verify binary status from Mambo, FPGA or MMCSLite run
	if [ -e $testLog ]
	then
	    # Get parms needed to determine passage
	    nodes=$( grep -a "^NODES =" ${testLog} | awk '{print $3}')
	    passCount=$( grep -a -c 'Software Test PASS' ${testLog} )

            # Use -ge since FPGA runs return more "passes" than nodes
	    if [ $passCount -ge $nodes ]  # BGQ binary passed
	    then 
		if [ $runSig -eq 0 ] # *run* passed 
		then 
		    evalSignal=$runSubRC
		else
		    evalSignal=$runSig
		fi  
	    else # Find source of binary FAIL
		evalSignal=$( grep -a -m 1 'Software Test FAIL' ${testLog} | awk '{print $NF}' )
		if [ $? -ne 0 ] || [ "${evalSignal}" == "" ] || [[ ! "${evalSignal}" =~ ^[0-9] ]]
		then # This test didn't exit normally, check for timeout or boot fail
		    if [ $( tail ${testLog} | grep -a -c "^Timeout reached" ) -eq 1 ]
		    then 
			evalSignal=9 # SIGKILL
			evalSummary="MMCS-Lite Timeout"
		    elif [ $( tail ${testLog} | grep -a -c "boot failed" ) -gt 0 ]
		    then 
			evalSignal=9 # SIGKILL
			evalSummary="Boot FAILED for ${block}"
		    else # I give up
			evalSignal=10 # SIGUSR1
		    fi
		fi
	    fi
	else # Log file DNE
	    if [ $runSig -eq 0 ] # *run* passed 
	    then 
		evalSignal=$runSubRC
	    else
		evalSignal=$runSig
	    fi  
	fi
    fi # end verify non-hw BGQ test
fi
 
eval $outLogSigVar=$evalSignal
eval $outLogSumVar=\"$evalSummary\"

return 0

}

#-------------------------------------------------------------------------------
# runSA input parms
# $1 - The working directory for the test
# $2 - The executable/binary file to run
# $3 - The log file
# $4.1 - Variable name to store the binary return code for this test
# $4.2 - Variable name to store the elapsed time for this test
#-------------------------------------------------------------------------------
runSA ()
{
    # Get the arguments
    cwd=$1
    exe=$2
    logFile=$3
    SASignalVar=$( echo $4 | awk '{print $1}' )
    elapsed_time=$( echo $4 | awk '{print $2}' )

    runSA_rc=0

    # Save off the current directory
    curDir=`pwd`

    echo "SERVER ID = ${serverID}" >> $logFile
    echo ""  >> $logFile

    # Change to the working directory and run the test
    changeDir $cwd

    if [ $? -ne 0 ]
    then
	echo "ERROR (E)::runSA: cd to exe dir FAILED!! Skipping to the next test ..."
	return 1
    fi  
    
    runCmd="./${exe}"

    # Start time in Epoch time
    before=$(date +%s)

    if [ $quietly -eq 1 ]
    then
	echo $runCmd >> $logFile 2>&1
	if [ $debug -eq 0 ]
	then
	    eval $runCmd >> $logFile 2>&1
	    runStatus=($( echo ${PIPESTATUS[*]} ))
	fi
    else
	echo $runCmd 2>&1 | tee -a $logFile 
	if [ $debug -eq 0 ]
	then
	    eval $runCmd 2>&1 | tee -a $logFile
	    runStatus=($( echo ${PIPESTATUS[*]} ))
	fi
    fi

    # End time in Epoch time
    after=$(date +%s)

    # Run time in readable format
    elapsed_seconds="$((${after} - ${before}))" # Needs to be a string
    eval $elapsed_time=$(date -d "1970-01-01 ${elapsed_seconds} seconds" +%T)

    if [ $debug -eq 1 ]
    then
	declare -a runStatus; runStatus[0]=0; runStatus[1]=0
    fi

    eval $SASignalVar=${runStatus[0]}

    # Check status of stand alone test
    if [ ${runStatus[0]} -ne 0 ]
    then
	echo "ERROR (E)::runSA: Execution of ${exe} FAILED!!"
	runSA_rc=${runStatus[0]}
    else # runCmd passed, let's check the logFile
	if [[ $quietly -eq 0 && ${runStatus[1]} -ne 0 ]] || [ ! -e $logFile ]
	then
	    echo "ERROR (E)::runSA: FAILED to redirect output into ${logFile}!!"
	    echo "ERROR (E)::runSA: No way to verify test results!!"
	    runSA_rc=${runStatus[1]}
	fi
    fi

    # Switch back to the previous directory
    changeDir $curDir

    if [ $? -ne 0 ]
    then
	echo "ERROR (E)::runSA: cd back to original dir FAILED!! Exiting."
	exit 1
    fi

    # Return
    return $runSA_rc
}

#-------------------------------------------------------------------------------
# runHW input parms
# $1 - The working directory for the test
# $2 - The executable/binary file to run
# $3 - Test scenario:  nodes, mode, NP
# $4 - The options to pass to mpirun/runjob
# $5 - block
# $6 - The arguments to pass to the executable
# $7 - The log file for this run
# $8.1 - Variable name to store the runfctest return code for this test
# $8.2 - Variable name to store the elapsed time for this test
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
    HWBlock=$5
    args=$6
    logFile=$7
    HWSignalVar=$( echo $8 | awk '{print $1}' )
    elapsedTimeVar=$( echo $8 | awk '{print $2}' )

    runjob="${run_floor}/hlcs/bin/runjob"
    runfctest="${run_floor}/scripts/runfctest.sh"
    type='MMCS'

    runjobArgs=$( echo "${args}" | sed 's/ -/ --args -/g' | sed 's/"//g' ) # use "${args}" for when 1st arg is -n

    if [[ "${exe}" =~ 'threadTest_omp' ]]
    then
	threads=16
    else
	threads=1
    fi

    runHW_rc=0

    envs=""
    runENVHash="/tmp/${USER}_${exe%%.*}_runenvhash.$(date +%Y%m%d-%H%M%S)"       # temp hash of ENV vars to set

    # Put default ENV vars in a hash so they can be overwritten by testfile options (except for mode)
    if [ "${platform}" == 'bgq' ]
	then
	
	if [ "${PAMI_DEVICE}" != "" ]
	then
	    hput $runENVHash 'PAMI_DEVICE' $PAMI_DEVICE
	fi

	hput $runENVHash 'BG_MEMSIZE' $BG_MEMSIZE
	hput $runENVHash 'BG_SHAREDMEMSIZE' $BG_SHAREDMEMSIZE
    fi

    # Parse mpirun/runjob options
    # Determine which ENV vars need to be saved, updated and documented
    set -- $(echo "${opts}")

    while [ "${1}" != "" ]; do
	case $1 in
	    -env | --envs )           parm=$1
                                      shift                  
		                      while [[ "${1}" =~ '=' ]];do
					  env_name=$( echo $1 | cut -d '=' -f1 | sed 's/"//g' )
					  env_val=$( echo $1 | cut -d '=' -f2 | sed 's/"//g' )
					  
					  if [ "${env_name}" != 'BG_PROCESSESPERNODE' ] && [ "${env_name}" != 'MPIRUN_MODE' ]
					  then

					      # Update environment hash
					      hput $runENVHash $env_name $env_val

                                              # Look for OMP threads
					      if [ "${env_name}" == 'OMP_NUM_THREADS' ]
					      then
						  threads=$env_val
					      fi                                      
					      shift
					  fi
				      done

                                      # Remove -envs/--envs from $opts (we'll make our own)
				      
				      if [ "${parm}" == '-envs' ] # BGP
					  then

					  # Save off everything after "-envs"
					  temp_opts=${opts##*-envs}

				          # Set opts = everything before "-envs"
					  opts=${opts%%-envs*}

				          # Final opts string with -envs val removed
					  if [[ "${temp_opts}" =~ '-' ]]
					  then					 
					      opts="${opts}-${temp_opts#*-}"
					  fi
				      else # BGQ
					  # Save off everything after "--envs"
					  temp_opts=${opts##*--envs}

				          # Set opts = everything before "--envs"
					  opts=${opts%%--envs*}

				          # Final opts string with --envs val removed
					  if [[ "${temp_opts}" =~ '--' ]]
					  then					 
					      opts="${opts}--${temp_opts#*--}"
					  fi
				      fi
				      ;;
	    * )                       # Goto the next value
		                      shift
	esac
#	shift
    done

    # Look for number of threads in exe args
    set -- $(echo "${args}" | sed 's/"//g') # use "${args}" for when 1st arg is -n

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

    # Separate out individual block pieces
    # BGP sub block
    HWLocation=$( echo $HWBlock | sed "s|:| |" | awk '{print $2}' )    

    # BGQ Shape
    if [ "${shape}" == '' ]
    then
	HWShape=$( echo $HWBlock | sed "s|:| |g" | awk '{print $3}' )
    else
	HWShape=$shape
    fi

    # BGQ Corner
    if [ "${corner}" == '' ]
    then
	HWCorner=$( echo $HWBlock | sed "s|:| |g" | awk '{print $2}' ) 
    else
	HWCorner=$corner
    fi

    # Block
    HWBlock=$( echo $HWBlock | sed "s|:.*||g" )

    # Check for mpirun (BGP) parms that we don't want to fill the input file with: 
    if [ "${platform}" == 'bgp' ]
    then
	# location
	if [[ ! "${opts}" =~ '--location' ]] && [ "${HWLocation}" != "" ]
	then
	    HWBlock="${HWBlock} --location ${HWLocation}"
	fi

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
    if [ "${platform}" == 'bgq' ]
    then    

	# corner
	if [[ ! "${opts}" =~ '--corner' ]] && [ "${HWCorner}" != "" ]
	then
	    HWBlock="${HWBlock} --corner ${HWCorner}"
	fi

	# shape
	if [[ ! "${opts}" =~ '--shape' ]] && [ "${HWShape}" != "" ]
	then
	    HWBlock="${HWBlock} --shape ${HWShape}"
	fi

        # label
	if [[ ! "${opts}" =~ '--label' ]]
	then
	    opts="${opts} --label"
	fi

        # verbose
	if [[ ! "${opts}" =~ '--verbose' ]] 
	then
	    opts="${opts} --verbose INFO"
	fi
    fi

    # Save off the current directory
    curDir=`pwd`

    # Echo useful info and ENV VARs to logfile
    {
	echo "NP = ${HWProcs}"
	echo "THREADS = ${threads}"
	echo "NODES = ${HWNodes}"
	if [ "${platform}" == 'bgp' ]
	then
	    echo "MODE = ${HWMode}"
	else
	    echo "BG_PROCESSESPERNODE = ${HWMode}"
	fi
    } >> $logFile

    # Print ENV vars to log and create -envs/--envs parm 
    while read xename xeval
    do
	echo "${xename} = ${xeval}" >> $logFile 
	envs="$envs ${xename}=${xeval}"
    done < $runENVHash

    echo "SERVER ID = ${serverID}" >> $logFile

    echo ""  >> $logFile

    # Change to the working directory and run the test
    changeDir $cwd

    if [ $? -ne 0 ]
    then
	echo "ERROR (E)::runHW: cd to exe dir FAILED!! Skipping to the next test ..."
	return 1
    fi

    # Create platform-specific run command
    if [ "${platform}" == 'bgp' ]
    then # Create BGP run command
	runCmd="mpirun -cwd ${cwd} -exe ${exe} -np ${HWProcs} -mode ${HWMode} -partition ${HWBlock}"
	
	# Add ENV vars
	if [ "${envs}" != "" ]
	then
	    runCmd="${runCmd} -envs \"${envs}\""
	fi

	# Add opts
	if [ "${opts}" != "" ]
	then
	    runCmd="${runCmd} ${opts}"
	fi

	# Add args
	if [ "${args}" != "" ]
	then
	    runCmd="${runCmd} -args \"${args}\""
	fi

    else # Create BGQ run command
        # Remove "'s from args string if they exist
#         args=$( echo $args | sed 's/"//g' ) 
#	runCmd="${runfctest} --script ${type} ${opts} --location ${block} --program ${exe} -- ${args}"
#	runCmd="${runjob} --cwd ${cwd} --exe ${exe} --np ${HWProcs} --ranks-per-node ${HWMode} --block ${HWBlock}"
	# Can't run with "--np".  It is unsupported and blocks stdout.
	runCmd="${runjob} --cwd ${cwd} --exe ${exe} --ranks-per-node ${HWMode} --block ${HWBlock}"
  
	# Add ENV vars
	if [ "${envs}" != "" ]
	then
	    runCmd="${runCmd} --envs ${envs}"
	fi
	
	# Add runjob opts
	if [ "${opts}" != "" ]
	then
	    runCmd="${runCmd} ${opts}"
	fi

	# Add args
	if [ "${runjobArgs}" != "" ]
	then
	    runCmd="${runCmd} --args ${runjobArgs}"
	fi
    fi

    # Start time in Epoch time
    before=$(date +%s)

    # Make sure we don't lose any error return codes due to piping
#    set -o pipefail

    if [ $quietly -eq 1 ]
    then
	echo $runCmd >> $logFile 2>&1
	if [ $debug -eq 0 ]
	then
	    eval $runCmd >> $logFile 2>&1
	    runStatus=($( echo ${PIPESTATUS[*]} ))
	fi
    else
	echo $runCmd 2>&1 | tee -a $logFile
	if [ $debug -eq 0 ]
	then
	    eval $runCmd 2>&1 | tee -a $logFile
	    runStatus=($( echo ${PIPESTATUS[*]} ))
	fi
    fi

    # End time in Epoch time
    after=$(date +%s)

    # Run time in readable format
    elapsed_seconds="$((${after} - ${before}))" # Needs to be a string
    eval $elapsedTimeVar=$(date -d "1970-01-01 ${elapsed_seconds} seconds" +%T)

    if [ $debug -eq 1 ]
    then
	if [ "${platform}" == 'bgq' ]
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
	if [[ $quietly -eq 0 && ${runStatus[1]} -ne 0 ]] || [ ! -e $logFile ]
	then
	    echo "ERROR (E)::runHW: FAILED to redirect output into ${logFile}!!"
	    echo "ERROR (E)::runHW: No way to verify test results!!"
	    runHW_rc=${runStatus[1]}
	fi	
    fi

    # Switch back to the previous directory
    changeDir $curDir

    if [ $? -ne 0 ]
    then
	echo "ERROR (E)::runHW: cd back to original dir FAILED!! Exiting."
	exit 1
    fi

    # Return
    return $runHW_rc
}

#-------------------------------------------------------------------------------
# runHWRoy input parms
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
	
	exit 1
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

    # Assemble a runjob command 
    if [ A != A$sub_block ]; then sbloc="--location ${block_name}"; fi

    #  

    rjcmd="runjob --block ${block} ${sbloc} --verbose ibm.runjob=INFO $opts --cwd `pwd -P` --exe ${exe} --args ${args}"

    # define a name that will help identify this run, use exe name, input args, modes etc.
    run_name=$exe	

    if [ $debug -eq 1 ]
	then
	echo "runjob_wrapper ${exe} ${rjcmd}"
	jobid="dummy"
	ExitStatus=0
    else
	runjob_wrapper ${run_name} $rjcmd
    fi

    runHW_rc=$ExitStatus

    # Verify output from runjob_wrapper exists
    runjob_out="run.${run_name}.${jobid}.out"

    if [ $debug -eq 1 ]
	then
	echo "dummy runjob of ${exe} PASSED" > $runjob_out
    fi

    if [ -e "${runjob_out}" ]
	then
	cp "${runjob_out}" "${logFile}" 

	if [ $? -ne 0 ] || [ ! -e "${logFile}" ]
	    then
	    echo "ERROR (E)::runHW: Copy of ${runjob_out} to ${logFile} FAILED!!"
	    runHW_rc=1
	fi
    else
	echo "ERROR (E)::runHW: ${runjob_out} DNE!!"
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
	exit 1
    fi

    # Return code
    return $runHW_rc
}

#-------------------------------------------------------------------------------
# exe_preProcessing input parms
# $1.1 - Num nodes based on commandline values
# $1.2 - Mode based on commandline values
# $1.3 - NP based on commandline values
# $1.4 - Threads based on binary name
# $2 - The binary name
# $3 - The options to pass to runjob
# $4 - The args to pass to runjob (so that we can determine final # of threads)
# $5.1 - Variable name to hold final node value to run with
# $5.2 - Variable name to hold final mode valued to run with
# $5.3 - Variable name to hold final NP value to run with
# $5.4 - Variable name to hold final Threads value to be run with (in case we skip this test)
# $5.5 - Variable to hold final opts string
# $5.6 - Variable name to hold override status
# $5.7 - Variable name to hold max NP value for this test
#-------------------------------------------------------------------------------
exe_preProcessing ()
{
    orgNodes=$( echo $1 | awk '{print $1}' )
    orgMode=$( echo $1 | awk '{print $2}' )
    orgNP=$( echo $1 | awk '{print $3}' )
    orgThreads=$( echo $1 | awk '{print $4}' )
    bin=$2
    opts=$3
    args=$4
    finalNodesVar=$( echo $5 | awk '{print $1}' )
    finalModeVar=$( echo $5 | awk '{print $2}' )
    finalNPVar=$( echo $5 | awk '{print $3}' )
    finalThreadsVar=$( echo $5 | awk '{print $4}' )
    finalOptsVar=$( echo $5 | awk '{print $5}' )
    overrideVar=$( echo $5 | awk '{print $6}' )
    maxNPVar=$( echo $5 | awk '{print $7}' )

    eppNodes=$orgNodes
    eppMode=$orgMode
    eppNP=$orgNP
    eppThreads=$orgThreads  

    OpenMP=0
    eppMaxNP=0
    npOverride=0
    eppOverride='N'

    runjob="${run_floor}/hlcs/bin/runjob"
    runfctest="${run_floor}/scripts/runfctest.sh"

    runScript='mpirun'

    if [ "${platform}" == 'bgq' ]
    then
	if [ "${run_type}" == 'hw' ]
	then
	    runScript=$runjob

	    # If "--timelimit" option exists, change it to "--timeout"
	    opts=$( echo $opts | sed -e 's/--timelimit/--timeout/g' )
	else
	    runScript=$runfctest

	    # If "--timeout" option exists, change it to "--timelimit"
	    opts=$( echo $opts | sed -e 's/--timeout/--timelimit/g' )
	fi
    fi

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
    set -- $(echo "${opts}")
#if [ "${opts}" != "" ]
#then
    while [ "${1}" != "" ]; do
	case $1 in
	    -env | --envs )           shift                  
		                      while [[ "${1}" =~ '=' ]]; do
					  env_name=$( echo $1 | cut -d '=' -f1 | sed 's/"//g' )
					  env_val=$( echo $1 | cut -d '=' -f2 | sed 's/"//g' )
					  
					  # Update final mode value
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

                                          # Update final threads value
					  if [ "${env_name}" == 'OMP_NUM_THREADS' ]
					  then
					      eppThreads=$env_val
					      OpenMP=1
					      # D/C about override parm since we can't set this from command line
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
	    --maxnp    )              shift
	                              eppMaxNP=$1

                                      # Remove --maxnp from $opts
				      # Save off everything after "--maxnp"
				      temp_opts=${opts##*--maxnp}

				      # Set opts = everything before "--maxnp"
				      opts=${opts%%--maxnp*}

				      # Final opts string with --maxnp val removed
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
		                      if [[ "${1}" =~ '-' ]] # option
				      then
	
                                          # Check against *run* help text
					  if [ "${platform}" == 'bgp' ]
					  then
					      found=$($runScript -h | grep -a -c -e "${1}")
					  else # BGQ
						  found=$($runScript --help | grep -a -c -e "${1}")
					  fi

					  if [ $found -eq 0 ]
					  then 
					      echo "ERROR (E)::exe_preProcessing: Unsupported option ${1} for ${runScript}!!"
					      echo "ERROR (E)::exe_preProcessing: Type ${runScript} --help to view supported options"
					      return 1
					  fi
 
					  shift
				      else # value
					  shift
				      fi
	esac
#	shift
    done
#fi

    # Parse exe args looking for number of threads
    set -- $(echo "${args}" | sed -e 's/"//g') # use "${args}" for when 1st arg is -n

    while [ "${1}" != "" ]; do
	case $1 in
	    --numPthreads )                  shift 
		                             if [ $OpenMP -eq 1 ]
						 then
						 eppThreads=$(( $eppThreads + $1))
					     else
						 eppThreads=$((1 + $1)) # total num threads = main thread + $1 Pthreads = 1 + $1
					     fi
					     ;;
	    -n )                             shift
		                             if [[ "${bin}" =~ 'threadTest' ]]
					     then
						 eppThreads=$1 # for threadTests, total num threads = main thread + ($1 - 1) Pthreads = $1   
					     fi
					     ;;
        * ) #continue                    
	esac
	shift
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
	if [ "${platform}" == 'bgp' ]
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
    eval $finalThreadsVar=$eppThreads
    eval $overrideVar=$eppOverride
    eval $finalOptsVar=\"$opts\"
    eval $maxNPVar=$eppMaxNP

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
# $8.1 - Variable name to store the runfctest return code for this test
# $8.2 - Variable name to store the elapsed time for this test
# $9 - block (last since not needed for FPGA)
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
    simSignalVar=$( echo $8 | awk '{print $1}' )
    elapsedTimeVar=$( echo $8 | awk '{print $2}' )
    simBlock=$9

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
    # Replace command-line positional parameters with $opts values
    set -- $(echo "${opts}")

    while [ "${1}" != "" ]; do
	case $1 in
	    --envs )                  shift                  
		                      while [[ "${1}" =~ '=' ]];do
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
    set -- $(echo "${args}" | sed -e 's/"//g') # use "${args} for when 1st arg is -n

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

    # svchost (for MMCS-Lite and Mambo)
    if [ "${type}" != "runFpga" ] && [[ ! "${opts}" =~ "--svchost" ]] 
    then
	opts="${opts} --svchost ${abs_script_dir}/svchost.pers"
    fi

    # Check for MMCS-Lite specific parms that we don't want to fill the input file with:
    if [ "${type}" == 'runMmcsLite' ]
    then

        # numnodes
	if [[ ! "${opts}" =~ "--numnodes" ]]
	then
	    opts="${opts} --numnodes ${simNodes}"
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
	echo "NODES = ${simNodes}"
	echo "BG_PROCESSESPERNODE = ${BG_PROCESSESPERNODE}"
	echo "THREADS = ${threads}"
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

    echo "" >> $logFile

    # Change to the working directory and run the test
    changeDir $cwd

    if [ $? -ne 0 ]
    then
	echo "ERROR (E)::runSim: cd to exe dir FAILED!! Skipping to the next test ..."
	return 1
    fi  

    if [ "${type}" == 'runMmcsLite' ]
    then
	    
	sub_block=$( echo $simBlock | sed "s|:| |" | cut -d " " -s -f2 )
	simBlock=$( echo $simBlock | sed "s|:.*||g" )

	runCmd="${runfctest} --script ${type} ${opts} --location ${simBlock} --program ${exe} -- ${args}"
    else # runMambo or runFpga
	runCmd="${runfctest} --script ${type} ${opts} --program ${exe} -- ${args}"
    fi

    # Start time in Epoch time
    before=$(date +%s)

    if [ $quietly -eq 1 ]
    then
	echo $runCmd >> $logFile 2>&1
	if [ $debug -eq 0 ]
	then
	    eval $runCmd >> $logFile 2>&1
	    runStatus=($( echo ${PIPESTATUS[*]} ))
	fi
    else
	echo $runCmd 2>&1 | tee -a $logFile
	if [ $debug -eq 0 ]
	then
	    eval $runCmd 2>&1 | tee -a $logFile
	    runStatus=($( echo ${PIPESTATUS[*]} ))
	fi
    fi

    # End time in Epoch time
    after=$(date +%s)

    # Run time in readable format
    elapsed_seconds="$((${after} - ${before}))" # Needs to be a string
    eval $elapsedTimeVar=$(date -d "1970-01-01 ${elapsed_seconds} seconds" +%T)

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
		exit 1
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
	echo "ERROR (E)::runSim: Execution of ${exe} FAILED!!"
	runSim_rc=${runStatus[0]}
    else # runCmd passed, let's check the logFile	
	if [[ $quietly -eq 0 && ${runStatus[1]} -ne 0 ]] || [ ! -e $logFile ]
	then
	    echo "ERROR (E)::runSim: FAILED to redirect output into ${logFile}!!"
	    echo "ERROR (E)::runSim: No way to verify test results!!"
	    runSim_rc=${runStatus[1]}
	fi
    fi

    # Verify success/failure of binary itself for runMambo and runMmcsLite runs
#    if [ "${runSim_rc}" -eq 0 ] && [ "${type}" != 'runFpga' ]
#    then
#	$(tail -10 $logFile | grep -a -q 'Software Test PASS')
#       
#	runSim_rc=$?
#    fi

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
	exit 1
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
	echo -e "\nERROR (E):llJobs:  Parameter ${action} is undefined.  Defined values are: query and cancel.\n"
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
    jobidArray=($(llq | grep -a $USER | awk '{print $1}'))

    # Read all job stati into an array
    jobSTArray=($(llq | grep -a $USER | awk '{print $5}'))

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
		    if [ "${jobSTArray[$jobid]}" == "NR" ] || [ "${jobSTArray[$jobid]}" == "X" ]
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
		    echo -e "\nCancelling ${TEST_ARRAY[$test]} (jobid ${jobidArray[$jobid]})"
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
	echo -e -n "\n ${fpga_queue} job(s) remaining in the queue." 
    fi

    return $llJobs_rc
}

# =========================================================  
# Log to Web
# =========================================================
logToWeb ()
{
    ltwAbsPath=$( echo $1 | awk '{print $1}' ) # build or exe abs path
    ltwRelPath=$( echo $1 | awk '{print $2}' ) 
    ltwTest=$( echo $1 | awk '{print $3}' )
    ltwNP=$( echo $2 | awk '{print $1}' )
    ltwMode=$( echo $2 | awk '{print $2}' )
    ltwThreads=$( echo $2 | awk '{print $3}' )
    scaleNodes=$( echo $3 | awk '{print $1}' ) # Used for auto group name creation
    scaleMode=$( echo $3 | awk '{print $2}' )  # Used for auto group name creation
    ltwOverallStatus=$4 
    ltwTestStatus=$5                           
    ltwTestSignal=$6
    ltwTestSummary=$7                          # individual summary of exe'd tests, else = overall status
    ltwTestLog=$8                              # make or exe log 
    xmlStatusVar=$9

    echo -e "\nLogging results to ${upperFamily} Test History webpage ..."

    logSummary=""
    xmlStatus=""

    # Create test name to be displayed in DB
    testName=""

    if [ "${testNameLength}" == 'short' ]
    then # use name only
	testName=$ltwTest
    else # Use relative path and name
	testName="${ltwRelPath}/${ltwTest}"
    fi

    # Determine compile driver level from binary if it exists
    if [ "${codeFamily}" == 'sst' ] && [ $mpich -eq 0 ] && [ -e "${ltwAbsPath}/${ltwTest}" ]
    then
	comp_drv=$( strings "${ltwAbsPath}/${ltwTest}" | grep -a -m 1 "SST_COMPILE_DRIVER" )
	comp_drv=${comp_drv##*=}
    fi

    # Overall status indicates this test reached the execution stage
    if [[ "${ltwOverallStatus}" =~ ^[0-9] ]] 
    then
	# Determine run driver level (2nd to last dir in floor)
	run_drv=${run_floor%/*} # Get rid of last dir
	run_drv=${run_drv##*/} # Only keep newest last dir (driver)

    fi

    # Create logXML command
    logCmd="${logXml} -e ${testName} -n ${ltwNP} -p ${ltwMode} -t ${ltwThreads} -r ${ltwTestSignal} -u ${USER} -m ${serverID}"

    if [ "${ltwTestLog}" != "" ]
    then
	    logCmd="${logCmd} -o ${ltwTestLog}"
    fi

    if [ "${comp_drv}" != "" ]
    then
	    logCmd="${logCmd} -c ${comp_drv}"
    fi
    
    if [ "${run_drv}" != "" ]
    then
	logCmd="${logCmd} -d ${run_drv}"
    fi

    if [ "${ltwTestSummary}" != "" ]
    then
	logCmd="${logCmd} -s \"${ltwTestSummary}\""
    fi

    # Format group name 
    groupMode="${scaleMode}m" # Default

    # Get text version of BGP mode
    if [ "${platform}" == 'bgp' ]
    then
	bgp_mode_NumtoText $scaleMode groupMode
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
	    tempGroup="${logGroup}_${scaleNodes}n_${groupMode}.${timestamp}"
	else # Just use group name specified by user
	    tempGroup="${logGroup}.${timestamp}"
	fi
	
	logCmd="${logCmd} -g ${tempGroup}"

    elif [ $autoGroup -eq 1 ] # Generate group name w/ scaling info
    then
	input=$( basename $file )
	tempGroup="${input/.*/}_${groupDev}_${scaleNodes}n_${groupMode}.${timestamp}"
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

		echo "ERROR (E): logXML.sh FAILED for ${ltwTest}"
		xmlStatus='FAILED'
	    else
		xmlStatus='Logged'
	    fi
	else # debug mode
	    xmlStatus='N/A'  
	fi
    else # Logging disabled
	xmlStatus='Skipped' 
    fi

    eval $xmlStatusVar=$xmlStatus
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
	    echo "ERROR (E)::changeDir: cd to ${dir} FAILED!!"
	    cDir_rc=1
	fi
    else
	echo "ERROR (E)::changeDir: ${dir} DNE!!"
	cDir_rc=1
    fi

    return $cDir_rc
}

reConfig ()
{
    configDir=$1
    level=$2
    tar=$3
    type=$4
    configFamily=$5
    configCmd=
    new_floor=
    config_rc=0

    # Save current dir
    curDir=`pwd`

    # cd to bgq dir
    changeDir $configDir
    if [ $? -ne 0 ]
    then
	echo "ERROR (E)::reConfig: changeDir FAILED!!"
	config_rc=1
    fi

    echo "INFO (I)::reConfig: Reconfiguring ${configDir} tree to: ${level}"

    # Create the configure command
    if [ $mpich -eq 1 ] || [ "${configFamily}" == 'sst' ]
    then
	configCmd="${abs_bgq_dir}/scripts/configure --with-floor=${level}"
    else # pami
	configCmd="${abs_pami_buildtools_dir}/configure --with-target=${tar} --with-bgfloor=${level}"

	if [ "${type}" == 'runMambo' ]
	then
	    configCmd="${configCmd} --with-mambo-workarounds"
	fi
    fi

    # Run configure 
    echo $configCmd
    eval $configCmd

    # Verify tree floor changed successfully
    if [ $? -ne 0 ]
    then
	echo "ERROR (E)::reConfig: configure script FAILED!!"
	config_rc=1
    else # command passed, verify floor (sst/mpich only, unimplemented in pami)
	if [ $mpich -eq 1 ] || [ "${configFamily}" == 'sst' ]
	    then
	    new_floor=$(grep -a 'BGQ_FLOOR_DIR=' "${configDir}/Make.rules" | cut -d '=' -f2 )
	    new_floor=$(readlink -e $new_floor)

	    if [ "${new_floor}" != "${level}" ]
	    then
		echo "ERROR (E)::reConfig: Floor mismatch:"
		echo "ERROR (E)::reConfig: BGQ_FLOOR_DIR => ${new_floor}"
		echo "ERROR (E)::reConfig: Desired       => ${level}"
		config_rc=1
	    else
		echo "BGQ_FLOOR_DIR is now set to: ${level}"
	    fi
	else # pami
	    echo "BGQ_FLOOR_DIR is now set to: ${level}"
	fi
    fi

    # Restore original dir
    changeDir $curDir

    return $config_rc
}

usage ()
{
    echo "sst_verif.sh compiles, copies and executes binaries it reads in from a file."
    echo "By default, binaries will be copied to a bgusr exe dir and run on hw." 
    echo "Separate make log files and runtime log files are made for each binary."
    echo "sst_verif.sh can be called from any dir, but must reside in ../bgq/system_tests/sst/scripts"
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
    echo "Format:  sst_verif.sh [options]"
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
    echo "                             To run multiple jobs in parallel, surround space separated block list with \"\":"
    echo "                             ex:  -b \"<block 1> <block 2> ..<block n>\""
    echo ""
    echo "                             To run subblocks in BGQ:"
    echo "                             --block <<block>:<corner>:<shape>>"
    echo "                             ex:  -b \"R00-M0-N01:R00-M0-N01-J02:1x1x1x1x1 R00-M0-N01:R00-M0-N01-J03:1x1x1x1x1"
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
    echo "-distclean | --distclean     Run make distclean before building binaries. MUST be combined with -c and valid test suite."
    echo "                             NOTE:  Will automatically be run if reconfigure and valid test suite are both selected."
    echo "                             NOTE:  Valid test suites are:  MPICH."
    echo "                                      ex:  -c --distclean -mpich"
    echo "                                      ex:  -rc -mpich"
    echo ""
    echo "-dn | --display-name <length>Specify how much of the test name to display on the Test History web page."
    echo "                             Valid values: short or rel."
    echo "                             default: rel              (displays /exercisers/bgqmem/bgqmem.elf)"
    echo "                             ex:  --display-name short (displays bgqmem.elf)"
    echo ""
    echo "-e | --exebase <path>        Specify trunk for binaries to be copied to and run from."
    echo "                              hw default: /bgusr/<USER>/<sandbox>/bgq/exe/sst"
    echo "                                      ex: /bgusr/alderman/bgq_svn/bgq/exe/sst" 
    echo "                             sim default: /gsa/rchgsa/home/<USER0:1>/<USER1:1>/<USER>/<sandbox>/bgq/exe/sst"                                 
    echo "                                      ex: /gsa/rchgsa/home/a/l/alderman/bgq_svn/bgq/exe/sst"
    echo ""
    echo "-email | --email <addr>      Email results summary to recipient(s). Can be combined with -comment option."
    echo "                                      ex: -email foo@us.ibm.com"
    echo "                                      ex: --email \"foo@us.ibm.com manchu@us.ibm.com\""
    echo ""
    echo "-f | --fpga                  Directs copies to gsa exe dir and runs from gsa exe dir on fpga simulator."
    echo ""
    echo "-fs | --force-scaling        Forces run to use command line values for parms that exist in both the command line and input file.  Otherwise, input file values trump command line values."
    echo ""
    echo "-fw | --force-web            Force compile results to the online DB. Default is to only document compile fails."
    echo ""
    echo "-g | --group <name>          Provide group name for logXml.sh. Time stamp will atomatically be appended by this script."
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
    echo "                              hw default: ../bgq/system_tests/sst/scripts/bvt.in"
    echo "                             sim default: ../bgq/system_tests/sst/scripts/bvt.in.sim"
    echo ""
    echo "                             NOTE:  to specify a binary as \"compile only\" type --compile-only on the same line as the binary (after the binary name, but before --args (if present))" 
    echo "" 
    echo "-l | --lite                  Directs copies to bgusr exe dir and runs from bgusr exe dir on hw using MMCS-Lite."
    echo ""
    echo "-m | --mambo                 Directs copies to gsa exe dir and runs from gsa exe dir on mambo simulator."
    echo ""
    echo "-mode <arg>                  Specify BGP mode to run in. Can be given as a space separated range for scaling purposes.  Command line value(s) will be trumped by -mode or -env MPIRUN_MODE in the input file unless -fs|--force-scaling is also used on the command line."
    echo "                             Valid values: SMP, DUAL & VN."
    echo "                             default: 1 rank per node"
    echo "                             ex:  -mode DUAL"
    echo "                             ex:  -mode \"SMP VN\" (scaling)"
    echo ""
    echo "-mpich | --mpich )           Skips non-power-of-2 values for numnodes/mode and \"io\" tests."
    echo ""
    echo "-noweb | --noweb             Skips updating Test History web page using logXML.sh."
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
    echo "-rpn | --ranks-per-node <arg> Specify \"mode\" to run in. Can be given as a space separated range for scaling purposes.  Command line value(s) will be trumped by --ranks-per-node in the input file unless -fs|--force-scaling is also used on the command line."
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
    echo "                             default: everything before /bgq/system_tests/sst/scripts"
    echo "                             ex: /bghome/alderman/test"
    echo ""
    echo "-v | --verbose               Enable extra info to be printed to output."
    echo ""
    echo "-h | --help                  This help text."
    echo ""
    echo " ******************************************************************************"
    echo ""
    echo " testlist features (inline options that can be set in the testlist file between each testname and its --args parm (if it exists)):"
    echo""
    echo " --compile-only              Tells script that this binary is to only be compiled.  This test will be skipped during copy and run phases"
    echo "                             ex:  functional/se/microBenchmarks/hello_se.elf --compile-only"
    echo ""
    echo " --maxnp <NP limit>          Tells script that if NP of current test scenario > this value, skip this test"
    echo "                             ex:  api/context/post-multithreaded-perf.cnk --maxnp 63 --timelimit 180 --args 1"
    echo ""
    echo " --standalone                Tells script that this binary is to be run natively without real or simulated hadware"
    echo "                             ex:  scripts/hello.sh --standalone"

}

# --- Global variables ---
ppid=$$
mypid=$$
codeFamily='sst'                             # sst or pami
hashLock=0

# common dirs
abs_script_dir=$(dirname $(readlink -f $0))  # my dir ("scripts" dir for sst, "test" dir for pami)
abs_source_dir=${abs_script_dir%/*}          # "sst" dir for sst/"pami" dir for pami: one back from dir this script is in
abs_rules_dir=""
comp_base=""

# sst dirs
abs_systest_dir=${abs_source_dir%/*}
abs_bgq_dir=${abs_systest_dir%/*}
abs_mpich_src_dir="${abs_bgq_dir}/comm"
abs_mpich_dir="${abs_mpich_src_dir}/build/mpich2/pami-gcc/test/mpi"

# pami dirs
abs_pami_sb_dir=${abs_source_dir%/*}         # pami sandbox dir: one back from source dir ("pami" dir)
abs_pami_buildtools_dir="${abs_pami_sb_dir}/buildtools"
abs_pami_build_dir="${abs_pami_sb_dir}/build"
file=""
verbose=0
quietly=0
ding=${DING:-0};        # 1 = issue audible "ding" for error or completion messages.
debug=0
reconfigure=0
cur_floor=
timestamp="$(date +%Y%m%d-%H%M%S)"
exeHash="/tmp/${USER}_hashmap.exe.${timestamp}"         # store data for each test

# Compile vars
make_log='make.log'
compile=2          # 0 = off, 1 = on, 2 = unset
make_distclean=0   # for suites like MPICH and GA/ARMCI after reconfiguring
make_clean=0
ignoreMakeFails=0  # for mipch/pami recursive makes
compile_floor=""
copy=2             # 0 = off, 1 = on, 2 = unset
compileHash="/tmp/${USER}_hashmap.compile.${timestamp}" # avoid recompiles
copyHash="/tmp/${USER}_hashmap.copy.${timestamp}"       # avoid recopies

# Exe vars
gsa_base="/gsa/rchgsa/home/${USER:0:1}/${USER:1:1}/${USER}"
bgusr_base="/bgusr/${USER}"
sandbox=""
common_exe_dir=""
exe_base=""
user_outdir=0
out_dir=""
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
#block_name="missing"
block=""
corner=""
shape=""
sub_block=""
fpga_queue=0
temp_jobid=""      # used to build common jobid between llsubmit and llq
jobid=""           # used by FPGA and HW runs
mpich=0
ignoreSysFails=0
blockHash="/tmp/${USER}_hashmap.blocks.${timestamp}"       # block status

# Logging/Documenting vars
summaryFile="sst_verif_summary.${timestamp}"
web_update=1
forceWeb=0
logXml="${abs_script_dir}/logXML.sh"
db_host=""
platform='bgq'
nameLength=
testNameLength='rel'
autoGroup=0
groupDev='MU'
logGroup=""
groupScale=0
logDisabled=0           # set to 1 when logXML has rc = 2 (can't create xml file)
emailAddr=""
summaryEmail="sst_verif_summary_email.${timestamp}"
comment=""

# =========================================================
# PARSE INPUT PARMS
# =========================================================
while [ "${1}" != "" ]; do
    case $1 in
        -ag | --autogroup )     autoGroup=1
                                ;;
        -b | --block )          shift
	                        declare -a blockNameArray=($( echo $1 | sed 's/"//g' )) 
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
        -cb | --compile-base )  shift
	                        comp_base=$1
                                ;;
        -cp | --copy )          copy=1
	                        if [ $compile -eq 2 ]; then compile=0; fi
	                        if [ $run -eq 2 ]; then run=0; fi
                                ;;
        -corner | --corner )    shift
	                        corner=$1
                                ;;
        -d | --debug )          debug=1
                                ;;
        -db | --dbhost )        shift
	                        db_host=$1
                                ;;
        -distclean | --distclean )    make_distclean=1
                                ;;
        -dn | --display-name )  shift
	                        testNameLength=$1
				if [ "${testNameLength}" != 'short' ] &&  [ "${testNameLength}" != 'rel' ]
				then
				    echo "ERROR (E):  Unknown display-name arg:  ${1}"
				    usage
				    exit 1
				fi
                                ;;
        -e | --exebase )        shift
	                        exe_base=$1
                                ;;
        -email | --email )      shift
	                        emailAddr=$1
                                ;;
        -f | --fpga )           run_type='runFpga'
                                ;;
        -fam | --family )       shift
	                        codeFamily=$(echo $1 | tr '[:upper:]' '[:lower:]' )
                                ;;
        -fs | --force-scaling ) forceScaling=1
                                ;;        
	-fw | --force-web )     forceWeb=1
                                ;;
        -g | --group )          shift
	                        logGroup=$1
                                ;;
        -gs | --group-scale )   groupScale=1
                                ;;
        -i | --input )          shift
	                        file=$1
                                ;;
	-ise | --ignore-syserr )ignoreSysFails=1
	                        ;;
	-l | --lite )           run_type='runMmcsLite'
	                        ;;
        -m | --mambo )          run_type='runMambo'
                                ;;
        -mpich | --mpich )      mpich=1
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
	                        platform=$(echo $1 | tr '[:upper:]' '[:lower:]' )
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
	                        abs_bgq_dir="${1}/bgq"
				abs_systest_dir="${abs_bgq_dir}/system_tests"
				abs_source_dir="${abs_systest_dir}/sst"
				abs_mpich_dir="${abs_bgq_dir}/comm/build/mpich2/pami-gcc/test/mpi"
                                ;;
        -shape | --shape )      shift
	                        shape=$1
                                ;;
        -v | --verbose )        verbose=1
                                ;;
        -h | --help )           usage
                                exit 0
                                ;;
        * )                     echo "ERROR (E):  Unknown option ${1}"
                                usage
                                exit 1
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

# Verify Code Family
upperFamily=$( echo $codeFamily | awk '{print toupper($0)}' )

case $codeFamily in
    pami )	
	echo "Verifying $upperFamily Family Tree ..."
	if [ "${db_host}" == "" ]
	then
	    db_host='msg'
	fi	
	;;
    sst )	
	echo "Verifying $upperFamily Family Tree ..."
	if [ "${db_host}" == "" ]
	then
	    db_host='sst'
	fi
	;;
    * )                     
	echo "ERROR (E):  Unrecognized code family: ${codeFamily}"
	echo "ERROR (E):  Valid values are:  pami & sst"
	exit 1
esac 

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
	    exit 1
	fi
    fi
fi

# Set default compile base dir
if [ "${comp_base}" == "" ]
then
    if [ $mpich -eq 1 ]
    then
	comp_base=$abs_mpich_dir       # ../pami-gcc/test/mpi dir
    elif [ "${codeFamily}" == 'sst' ]
    then
	comp_base=$abs_source_dir      # ../bgq/system_tets/sst dir
    else # pami
	comp_base=$abs_pami_build_dir  # ../build dir
    fi
fi

# Verify compile base (default or user specified)
# Don't check now for MPICH, because comm may not have been built yet
if [ $mpich -eq 0 ] && [ ! -d "${comp_base}" ]
then
    echo "ERROR (E): Compile dir: ${comp_base} DNE!! Exiting."
    exit 1
fi

# Create absolute rules dir (holds Make.rules)
if [ $mpich -eq 1 ] || [ "${codeFamily}" == 'sst' ]
then
    abs_rules_dir=$abs_bgq_dir
else # pami
    abs_rules_dir=$comp_base
fi

# Verify platform
# Turn on case-insensitive matching (-s set nocasematch)
shopt -s nocasematch

case $platform in
    bgp | bgq | linux | mpi | percs )
	upperPlatform=$( echo $platform | awk '{print toupper($0)}' )
	logXml="${logXml} --platform ${upperPlatform}"
	;;
    * )                     
	echo "ERROR (E):  Unrecognized platform: ${platform}"
	echo "ERROR (E):  Valid values are:  bgp, bgq, linux, mpi & percs"
	exit 1
esac

# Turn off case-insensitive matching (-u unset nocasematch)
shopt -u nocasematch

# Compare user platform with existing Make.rules target
if [ "${codeFamily}" == "pami" ] && [ $mpich -eq 0 ]
then
    if [ -e "${abs_rules_dir}/Make.rules" ]
    then 
	target=$(grep -a "^TARGET" "${abs_rules_dir}/Make.rules" | awk '{print $3}')
	
	if [ $? -ne 0 ] || [ "${target}" == "" ]
	then
	    echo "ERROR (E): FAILED to determine current Make.rules target. Exiting."
	    exit 1
	else
	    if [ "${platform}" != "${target}" ]
	    then
		echo "ERROR (E): platform ${platform} != target ${target} in ${abs_rules_dir}/Make.rules!!"
		echo "ERROR (E): Run ../buildtools/configure manually from: $abs_rules_dir"
		echo "ERROR (E):    or"
		echo "ERROR (E): Rerun sst_pami_verif.sh and append the -rc <floor> option."
		echo "ERROR (E): Exiting."
		exit 1
	    fi
	fi
    elif [ $reconfigure -eq 0 ]
    then
	echo "ERROR (E): ${abs_rules_dir}/Make.rules DNE!!"
	echo "ERROR (E): Run ../buildtools/configure manually from: $abs_pami_build_dir"
	echo "ERROR (E):    or"
	echo "ERROR (E): Rerun sst_pami_verif.sh and append the -rc <floor> option."
	echo "ERROR (E): Exiting."
	exit 1
    fi
fi

# Set file to default based on input parms
if [ "${file}" == "" ]
then
    if [ $mpich -eq 1 ]
    then
	file="${abs_script_dir}/mpich2.testlist.mode"
    elif [ "${codeFamily}" == 'sst' ]
    then
	if [ "${run_type}" == 'runMambo' ] || [ "${run_type}" == 'runFpga' ]
	then
	    file="${abs_script_dir}/master.in.sim"
	else
	    file="${abs_script_dir}/master.in"
	fi
    else # pami
	file="${abs_script_dir}/testfile.in.single-node"
    fi
fi
    
# Ensure input file exists
if [ ! -e "${file}" ]
then
    echo "ERROR (E): Input file: ${file} DNE!! Exiting."

    #ding ding ding
    if [[ A1 = A$ding ]]; then echo -en "\007"; sleep 1; echo -en "\007"; sleep 1; echo -en "\007"; fi

    exit 1
else
    file=$(readlink -e "${file}" )
fi
 
# Default to current host if server ID wasn't provided
if [ "${serverID}" == 'unknown' ]
then
    serverID=$HOSTNAME
fi

# Set exe_base to default if not specified
if [ "${exe_base}" == "" ]
then

    # Set common exe dir parm
    if [ $mpich -eq 1 ] || [ "${codeFamily}" == 'sst' ]
    then
	common_exe_dir='bgq/exe/sst'
    else # pami
	common_exe_dir="pami/${platform}/exe/test"
    fi

    # Set sandbox value
    temp_dir=""
    if [ $mpich -eq 1 ] || [ "${codeFamily}" == 'sst' ]
    then
	temp_dir=${abs_bgq_dir%/*} # dir before "bgq" dir
    else # pami
	temp_dir=${abs_source_dir%/*} # dir before source "pami" dir
    fi

    sandbox=${temp_dir#*${USER}/}

    if [ "${run_type}" == 'runMambo' ] || [ "${run_type}" == 'runFpga' ]
    then
	exe_base="${gsa_base}/${sandbox}/${common_exe_dir}"
    else
	exe_base="${bgusr_base}/${sandbox}/${common_exe_dir}"
    fi
fi

# Verify/Create the exe dir (default or user specified) if copying or running
if [ $copy -eq 1 ] || [ $run -eq 1 ]
then
    if [ ! -d "${exe_base}" ]
    then
        # exe_base dir DNE
        # Create if user plans to copy binaries during this run
	if [ $copy -eq 1 ]
	then
	    echo "Creating exe base ..."
	    mkdir -p "${exe_base}"

	    if [ $? -ne 0 ] || [ ! -d "${exe_base}" ]
	    then
		echo "Creation of ${exe_base} FAILED!!"
		exit 1
	    fi
	else # Fail if user only planned to run
	    
	    echo "ERROR (E): Runtime dir: ${exe_base} DNE!! Exiting."
	    exit 1
	fi
    fi
fi

# Verify/Create the results dir specified by the user if running
if [ $user_outdir -eq 1 ] && [ $run -eq 1 ]
then
    if [ ! -d "${out_dir}" ]
    then
	#results dir DNE, let's create it
	echo "Creating results dir ..."
	mkdir -p "${out_dir}"

	if [ $? -ne 0 ] || [ ! -d "${out_dir}" ]
	then
	    echo "Creation of ${out_dir} FAILED!!"
	    exit 1
	fi
    fi
fi

# Set "mode" ENV variable to default value
if [ "${platform}" == 'bgq' ]
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
    if [ "${platform}" == 'bgq' ]
    then
	modeArray[0]=$BG_PROCESSESPERNODE
    else
	modeArray[0]=$MPIRUN_MODE
    fi
else 
    # Convert BGP text mode values to numbers
    if [ "${platform}" == 'bgp' ]
    then  
	for ((pmode=0; pmode < ${#modeArray[@]}; pmode++))
	do
            # Get numerical version of mode
	    bgp_mode_TexttoNum ${modeArray[$pmode]} numMode

	    if [ $? -ne 0 ]
	    then
		echo "ERROR (E): bgp_mode_TexttoNum subroutine FAILED!!"
		exit 1
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

# --- BGQ Runtime defaults ---
if [ $run -eq 1 ] && [ "${platform}" == 'bgq' ]
then
    # Set up memory ENV variables to default values
    if [ "${BG_MEMSIZE}" == "" ] && [ "${run_type}" != 'runFpga' ]
    then 
	echo "Setting BG_MEMSIZE to 4096"
	export BG_MEMSIZE=4096 
    else # When BG_MEMSIZE env var is set
	echo "BG_MEMSIZE set to ${BG_MEMSIZE}"
    fi

    if [ "${BG_SHAREDMEMSIZE}" == "" ]
    then 
	echo "Setting BG_SHAREDMEMSIZE to 32"
	export BG_SHAREDMEMSIZE=32
    else
	echo "BG_SHAREDMEMSIZE set to ${BG_SHAREDMEMSIZE}"
    fi

    # PAMI_DEVICE default
    # When PAMI_DEVICE env var is unset
    if [ "${PAMI_DEVICE}" == "" ]
    then 
	if [ "${run_type}" == 'runFpga' ] || [ "${run_type}" == 'hw' ]
	then
	    echo "Setting PAMI_DEVICE to S"
	    export PAMI_DEVICE='S' # No MU unit in FPGA model
	    groupDev='SHMem'
	else
	    groupDev='Both'
	fi
    else # When PAMI_DEVICE env var is set
	if [[ "${run_type}" == 'runFpga' || "${run_type}" == 'hw' ]] && [ "${PAMI_DEVICE}" != "S" ]
	then
	    reason='MMCS HW'
	    if [ "${run_type}" == 'runFpga' ]
	    then
		reason='FPGA'
	    fi

	    echo "Forcing PAMI_DEVICE to S for ${reason}" # No MU unit in FPGA model
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
		exit 1
	    fi
	fi
    fi
fi

# --- Set and verify floor vars ---
# Set cur_floor
arch=$(uname -p)
cur_base='/bgsys/bgq/drivers'

if [ "${arch}" == 'ppc64' ]
then
    cur_floor=$(readlink -e "${cur_base}/ppcfloor" )
elif [ "$arch" == 'x86_64' ]
then
    cur_floor=$(readlink -e "${cur_base}/x86_64.floor" )
else
    echo "ERROR (E): Unknown arch (${arch})!! Can't determine current floor. Exiting."
    exit 1
fi

# Exit if readlink failed
if [ $? -ne 0 ]
then
    echo "ERROR (E): readlink command FAILED!! Can't determine current floor. Exiting."
    exit 1
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
	    echo "ERROR (E): ${compile_floor} DNE!! Exiting."
	    exit 1
	fi
    fi
 
    reConfig $abs_rules_dir $compile_floor $platform $run_type $codeFamily
    if [ $? -ne 0 ]
    then
	echo "ERROR (E): reConfig FAILED!! Exiting."
	exit 1
    fi
else # Set compile floor to existing floor in Make.rules
    if [ $mpich -eq 1 ] || [ "${codeFamily}" == 'sst' ]
    then
	compile_floor=$(grep -a 'BGQ_FLOOR_DIR=' "${abs_bgq_dir}/Make.rules" | cut -d '=' -f2 )
	compile_floor=$(readlink -e $compile_floor)
    fi
fi

# Set compile_floor to BGQ_FLOOR_DIR in Make.rules (created by configure script)
if [ $mpich -eq 1 ] || [ "${codeFamily}" == 'sst' ]
then
    if [ $compile -eq 1 ] || [ $copy -eq 1 ]
    then
	if [ ! -e "${abs_rules_dir}/Make.rules" ]
	then
	    echo "ERROR (E): ${abs_rules_dir}/Make.rules DNE!! Cannot determine compile floor."
	    echo "ERROR (E): Run ../scripts/configure manually from: ${abs_rules_dir}"
	    echo "ERROR (E):    or"
	    echo "ERROR (E): Rerun sst_pami_verif.sh and append -rc <floor> option."
	    echo "ERROR (E): Exiting."
	    exit 1
	else
	    compile_floor=$(fgrep -a 'BGQ_FLOOR_DIR=' "${abs_rules_dir}/Make.rules" | cut -d '=' -f2)
	    if [ ! -e "${compile_floor}" ] 
	    then
		if [ $compile -eq 1 ]
		    then
		    echo "ERROR (E): BGQ_FLOOR_DIR: ${compile_floor} DNE!! Exiting."
		    exit 1
		else
		    echo "WARNING (W): BGQ_FLOOR_DIR: ${compile_floor} DNE!!"
		fi
	    fi
	fi
    fi
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
	    exit 1
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
  TEST_ARRAY[$element]=$(echo "${xtest##*/}" | tr -d '\n' | tr -d '\r')
  hput $compileHash ${TEST_ARRAY[$element]} 0
  hput $copyHash ${TEST_ARRAY[$element]} 0

  # Store build and exe dirs  
  bin_base=""
  if [ $mpich -eq 1 ] || [ "${codeFamily}" == 'sst' ]
      then
      bin_base=$comp_base
  else # pami
      bin_base="${comp_base}/pami/test"
  fi
  
  if [[ $xtest =~ '/' ]]
  then  
      hput $exeHash "${TEST_ARRAY[$element]}:$element:stub" "${xtest%/*}"
      hput $exeHash "${TEST_ARRAY[$element]}:$element:buildDir" "${bin_base}/${xtest%/*}"
      hput $exeHash "${TEST_ARRAY[$element]}:$element:exeDir" "${exe_base}/${xtest%/*}"
  else
      hput $exeHash "${TEST_ARRAY[$element]}:$element:stub" ""
      hput $exeHash "${TEST_ARRAY[$element]}:$element:buildDir" "${bin_base}"
      hput $exeHash "${TEST_ARRAY[$element]}:$element:exeDir" "${exe_base}"
  fi

  # Enable test
  hput $exeHash "${TEST_ARRAY[$element]}:$element:exe" 1
  hput $exeHash "${TEST_ARRAY[$element]}:$element:runTotal" $(( ${#nodeArray[@]} * ${#modeArray[@]} ))
  hput $exeHash "${TEST_ARRAY[$element]}:$element:runPass" 0

  # Create Overall Status element
  hput $exeHash "${TEST_ARRAY[$element]}:$element:status" 'Skipped'

  # Create Compile log and signal elements
  hput $exeHash "${TEST_ARRAY[$element]}:$element:logCompile" 'N/A'
  hput $exeHash "${TEST_ARRAY[$element]}:$element:compSignal" 0

  # Store "runjob" options
  tempOpts="$(echo ${xopts%%--args*} | tr -d '\n' | tr -d '\r')"

  # Is this test compile only?
  if [[ "${tempOpts}" =~ 'compile-only' ]]
  then
      hput $exeHash "${TEST_ARRAY[$element]}:$element:compileOnly" 1
      
      if [ $compile -eq 0 ]
      then # Disable this test (for copy and/or exe) and tell user why
	  hput $exeHash "${TEST_ARRAY[$element]}:$element:exe" 0
	  hput $exeHash "${TEST_ARRAY[$element]}:$element:status" 'Skipped (Compile Only)'
      fi
  else
      hput $exeHash "${TEST_ARRAY[$element]}:$element:compileOnly" 0
      hput $exeHash "${TEST_ARRAY[$element]}:$element:runOpts" "${tempOpts}"
  fi

  # Is this a standalone/native test?
  if [[ "${tempOpts}" =~ 'standalone' ]]
  then 
      hput $exeHash "${TEST_ARRAY[$element]}:$element:standAlone" 1
  else
      hput $exeHash "${TEST_ARRAY[$element]}:$element:standAlone" 0
      if [ "${run_type}" == 'hw' ] || [ "${run_type}" == 'runMmcsLite' ]
      then
          needBlock=1
      fi
  fi 
      
  # Disable mpich "io" runs for now:
  if [ $mpich -eq 1 ]
  then
      if [[ "${xtest}" =~ ^io/ ]] || [[ "${xtest}" =~ '/io/' ]]
      then
	  hput $exeHash "${TEST_ARRAY[$element]}:$element:exe" 0
	  hput $exeHash "${TEST_ARRAY[$element]}:$element:status" 'Skipped (I/O)'
      fi
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
if [ $run -eq 1 ] && [ $needBlock -eq 1 ] && [[ "${run_type}" == "hw" || "${run_type}" == "runMmcsLite" ]] 
then 
    if [ ${#blockNameArray[@]} -eq 0 ]
    then
	echo -e "ERROR (E): Block parameter(s) missing. Specify with -b <block> or --block <block>.\nExiting." 

            #ding ding ding
	if [[ A1 = A$ding ]]; then echo -en "\007"; sleep 1; echo -en "\007"; sleep 1; echo -en "\007"; fi

	exit 1
    else
	declare -a blockArray

	for ((index=0; index < ${#blockNameArray[@]}; index++))
	do
	    blockArray[${#blockArray[@]}]=${blockNameArray[$index]}
	    hput $blockHash ${blockNameArray[$index]} 'free'
	done
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

    if [ "${cur_floor}" == "${compile_floor}" ]
    then
	echo "Compiling with floor:  ${cur_floor}"
    else
	echo "WARNING (W): Floor mismatch:"
	echo "WARNING (W): Latest floor  => ${cur_floor}"
	echo "WARNING (W): Compile floor => ${compile_floor}"
    fi

    cleanType="clean"

    if [ $make_distclean -eq 1 ]
    then
	cleanType="distclean"
    fi

    # Perform make distclean of source dir for select test suites (MPICH) if we reran configure
    if [ $reconfigure -eq 1 ] && [ $mpich -eq 1 ]
	then
	
	suiteSourceDir=$abs_mpich_src_dir
	make_clean=0               # because the /build dir will be wiped out
	make_distclean=0           # ditto

	changeDir $suiteSourceDir
	if [ $? -ne 0 ]
	then
	    echo "ERROR (E): changeDir FAILED!! Exiting."
	    exit 1
	fi

	echo -e "\nINFO (I):  Performing make distclean in:  ${suiteSourceDir}\n"
	makeCmd='make distclean'
	echo $makeCmd

	if [ $debug -eq 0 ]
	then
	    eval $makeCmd
	fi
    fi # end make distclean of source dir

    # Perform make [dist]clean if requested
    if [ $make_clean -eq 1 ] || [ $make_distclean -eq 1 ]
    then

	# Clean tree
	echo -e "\nINFO (I):  Performing make ${cleanType} in:  ${comp_base}\n"

	changeDir $comp_base
	if [ $? -ne 0 ]
	then
	    echo "ERROR (E): changeDir FAILED!! Exiting."
	    exit 1
	fi

	# Create make clean command
	makeCmd=""
	if [ $mpich -eq 1 ] || [ "${codeFamily}" == 'sst' ]
	then
	    makeCmd="make ${cleanType}"
	else # pami
	    makeCmd="/bglhome/jratt/install-sles10/bin/make ${cleanType}"
	fi

	echo $makeCmd
	
	if [ $debug -eq 0 ]
	then
	    eval $makeCmd
	fi
	
	# Clean performance applications tree if it exists in testfile
	appExists=$( grep -a -m 1 -c "^../applications" $file ) 
	if [ $appExists -gt 0 ] && [ "${codeFamily}" == 'sst' ]
	then 
	    echo -e "\nINFO (I):  Performing make ${cleanType} in:  ${abs_systest_dir}/applications\n"
	    
	    changeDir "${abs_systest_dir}/applications"
	    if [ $? -ne 0 ]
	    then
		echo "ERROR (E): changeDir FAILED!! Exiting."
		exit 1
	    fi

	    echo $makeCmd
	    
	    if [ $debug -eq 0 ]
	    then
		eval $makeCmd
	    fi
	fi
    fi # end make clean

    # === Let's build some binaries ===
    # Split up the make work
    MAKECPUS=$(grep -a -c '^processor' /proc/cpuinfo)

    # --- Only build selected sst binaries ---
    if [ $mpich -eq 0 ] && [ "${codeFamily}" == 'sst' ]
    then
	for ((test=0; test < ${#TEST_ARRAY[@]}; test++))
	do

            # Don't build disabled tests
	    hget $exeHash "${TEST_ARRAY[$test]}:$test:exe" enabled

	    if [ "${enabled}" == '0' ]
	    then
		echo "Skipping compile of ${TEST_ARRAY[$test]} ..."
		continue
	    fi

            # Don't rebuild tests
	    hget $compileHash ${TEST_ARRAY[$test]} alreadyCompiled

	    if [ "${alreadyCompiled}" == "1" ]
	    then
		echo "Already compiled ${TEST_ARRAY[${test}]} ..."

	        # Update exe hash file
		hput $exeHash "${TEST_ARRAY[$test]}:$test:status" 'Compiled'
		hput $exeHash "${TEST_ARRAY[$test]}:$test:logCompile" $alreadyCompiled
		continue
	    fi

            # CD to build dir
	    hget $exeHash "${TEST_ARRAY[$test]}:$test:buildDir" buildDir

	    changeDir $buildDir
	    if [ $? -ne 0 ]
	    then
		hput $exeHash "${TEST_ARRAY[$test]}:$test:status" 'Compile FAILED'
		echo "ERROR (E): changeDir FAILED!! Skipping to the next test ..."
  
      	        # Disable this test for the rest of this run
		hput $exeHash "${TEST_ARRAY[$test]}:$test:exe" 0
		continue
	    fi

            # Generate unique make.log for this compile
	    if [ $debug -eq 1 ]
	    then 
		compileLog="${buildDir}/${TEST_ARRAY[$test]%\.*}.${make_log}.dummy"
	    else
		compileLog="${buildDir}/${TEST_ARRAY[$test]%\.*}.${make_log}"
	    fi

	    # Remove existing log (That's what the DB is for)
	    if [ -e $compileLog ]
	    then
		rm $compileLog
	    fi

	    hput $exeHash "${TEST_ARRAY[$test]}:$test:logCompile" "${compileLog}"

	    echo -e "\nCompiling test ${TEST_ARRAY[$test]} ..."

	    makeCmd="make -j ${MAKECPUS} ${TEST_ARRAY[$test]}"

	    # Execute make
	    if [ $quietly -eq 1 ]
	    then
		echo $makeCmd >> $compileLog 2>&1
		if [ $debug -eq 0 ]
		then
		    eval $makeCmd >> $compileLog 2>&1
		    makeStatus=($( echo ${PIPESTATUS[*]} ))
		fi
	    else
		echo $makeCmd 2>&1 | tee -a $compileLog
		if [ $debug -eq 0 ]
		then
		    eval $makeCmd 2>&1 | tee -a $compileLog
		    makeStatus=($( echo ${PIPESTATUS[*]} ))
		fi
	    fi

	    if [ $debug -eq 1 ]
	    then
		
		# Create a dummy binary if one DNE
		if [ ! -e "${TEST_ARRAY[$test]}" ]
		then
		    echo "dummy exe" > ${TEST_ARRAY[$test]}
		fi

		declare -a makeStatus; makeStatus[0]=0; makeStatus[1]=0
	    fi

	    # Store make signal for future reference
	    hput $exeHash "${TEST_ARRAY[$test]}:$test:compSignal" ${makeStatus[0]}

            # Check status of make
	    overallStatus=""
	    if [ ${makeStatus[0]} -ne 0 ] || [ ! -e "${TEST_ARRAY[$test]}" ]
	    then
		overallStatus='Compile FAILED'
		hput $exeHash "${TEST_ARRAY[$test]}:$test:status" "${overallStatus}"
		echo "ERROR (E): make of ${TEST_ARRAY[$test]} FAILED!!"

      	        # Disable this test for the rest of this run
		hput $exeHash "${TEST_ARRAY[$test]}:$test:exe" 0

	    else # Make passed
		overallStatus='Compiled'
		hput $exeHash "${TEST_ARRAY[$test]}:$test:status" "${overallStatus}"
		hput $compileHash "${TEST_ARRAY[$test]}" "${compileLog}" # Don't build this test again

		# Let's check the log
		if [[ $quietly -eq 0 && ${makeStatus[1]} -ne 0 ]] || [ ! -e "${compileLog}" ]
		then
		    echo "WARNING (W): FAILED to redirect output into ${compileLog}!!"
		fi
	    fi

            # Disable this test if it was compile only
	    hget $exeHash "${TEST_ARRAY[$test]}:$test:compileOnly" compileOnly
	    if [ "${compileOnly}" == '1' ]
	    then
		hput $exeHash "${TEST_ARRAY[$test]}:$test:exe" 0
	    fi

	    # Log to web if compile failed or user chose to force results to DB
	    if [ "${overallStatus}" == 'Compile FAILED' ] || [ $forceWeb -eq 1 ]
	    then

		signal=999

		if [ "${overallStatus}" == 'Compile FAILED' ]
		then
		    signal=$( tail $compileLog | grep -a -m 1 ' Error' | awk '{print $NF}')

		    if [ $? -ne 0 ] || [ "${signal}" == "" ] || [[ ! "${signal}" =~ ^[0-9] ]] 
		    then # This compile didn't exit normally
			signal=10 # SIGUSR1
		    fi
		else
		    signal=0
		fi
		
		hget $exeHash "${TEST_ARRAY[$test]}:$test:stub" stub

		logToWeb "${buildDir} ${stub} ${TEST_ARRAY[$test]}" "0 0 0" "${nodeArray[0]} ${modeArray[0]}" "${overallStatus}" "" $signal "${overallStatus}" "${compileLog}" xmlStat

	      # Determine NP value for saving XML status
		keyNP=0
		if [ $forceNP -eq 0 ]
		then
		    keyNP=$(( ${nodeArray[0]} * ${modeArray[0]} ))
		else
		    keyNP=$forceNP
		fi

		hput $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${nodeArray[0]}_m${modeArray[0]}_p${keyNP}" $xmlStat
	    fi # end log to web
	done # end loop through tests
    else # --- mpich or pami compile ---
    
	# Build source code for select test suites (MPICH)
	if [ $mpich -eq 1 ] 
	    then
	    
	    # Change to the comm source dir
	    changeDir $abs_mpich_src_dir
	    if [ $? -ne 0 ]
	    then
		echo "ERROR (E): changeDir FAILED!! Exiting."
		exit 1
	    fi

	    #Remove existing log (That's what the DB is for)
	    if [ -e $make_log ]
	    then
		rm $make_log
	    fi

	    makeCmd="make XL= "

	    if [ $quietly -eq 1 ]
	    then
		echo $makeCmd >> $make_log 2>&1
		if [ $debug -eq 0 ]
		then
		    eval $makeCmd >> $make_log 2>&1
		    makeStatus=($( echo ${PIPESTATUS[*]} ))
		fi
	    else
		echo $makeCmd 2>&1 | tee -a $make_log
		if [ $debug -eq 0 ]
		then
		    eval $makeCmd 2>&1 | tee -a $make_log
		    makeStatus=($( echo ${PIPESTATUS[*]} ))
		fi
	    fi

            # Check status of make
	    if [ ${makeStatus[0]} -ne 0 ]
	    then
		echo "ERROR (E): make FAILED!!"
		exit 1

	    else # make passed, let's check the log
		if [[ $quietly -eq 0 && ${makeStatus[1]} -ne 0 ]] || [ ! -e "${make_log}" ]
		then
		    echo "WARNING (W): FAILED to redirect output into ${make_log}!!"
		fi
	    fi
	fi # end build source code of select test suites

	# Build tests
	# Change to the compile base dir	   
	changeDir $comp_base
	if [ $? -ne 0 ]
	then
	    echo "ERROR (E): changeDir FAILED!! Exiting."
	    exit 1
	fi

	#Remove existing log (That's what the DB is for)
	if [ -e $make_log ]
	then
	    rm $make_log
	fi

	if [ $mpich -eq 1 ]
	then
	    export COMPILEONLY=1
	    export VERBOSE=1
		
	    # Create dummy stagingdir to avoid MPICH runtests hang ...will erase later
	    stagingDir="${exe_base}/dummy_mpich_bin"

	    if [ ! -d $stagingDir ]
		then
		mkdir -p $stagingDir

		if [ $? -ne 0 ] || [ ! -d $stagingDir ]
		then
		    echo "Creation of ${stagingDir} FAILED!!"
		    exit 1
		fi
	    fi

	    export STAGINGDIR=$stagingDir

	    makeCmd="make testing"
	else # pami
	    makeCmd="/bglhome/jratt/install-sles10/bin/make -j ${MAKECPUS}"
	fi

	# Execute make
	if [ $quietly -eq 1 ]
	then
	    echo $makeCmd >> $make_log 2>&1
	    if [ $debug -eq 0 ]
	    then
		eval $makeCmd >> $make_log 2>&1
		makeStatus=($( echo ${PIPESTATUS[*]} ))
	    fi
	else
	    echo $makeCmd 2>&1 | tee -a $make_log
	    if [ $debug -eq 0 ]
	    then
		eval $makeCmd 2>&1 | tee -a $make_log
		makeStatus=($( echo ${PIPESTATUS[*]} ))
	    fi
	fi	

	# Erase dummy MPICH stagingdir
	if [ $mpich -eq 1 ] && [ -e $stagingDir ]
	    then
	    rm -fr $stagingDir
	    unset STAGINGDIR
	fi

	if [ $debug -eq 1 ]
	then
		
	    # Create dummy binaries if they DNE
	    for ((test=0; test < ${#TEST_ARRAY[@]}; test++))
	    do
		
		hget $exeHash "${TEST_ARRAY[$test]}:$test:buildDir" buildDir

		if [ ! -e "${buildDir}/${TEST_ARRAY[$test]}" ]
		then
		    echo "dummy exe" > "${buildDir}/${TEST_ARRAY[$test]}"
		fi
	    done

	    declare -a makeStatus; makeStatus[0]=0; makeStatus[1]=0
	fi

	# See which tests built. Disable tests that failed or are compile only.
	binaryCount=0

	for ((test=0; test < ${#TEST_ARRAY[@]}; test++))
	do
	   
	    hget $exeHash "${TEST_ARRAY[$test]}:$test:buildDir" buildDir

	    overallStatus=""

	    if [ ! -e "${buildDir}/${TEST_ARRAY[$test]}" ]
	    then
		overallStatus='Compile FAILED'
		hput $exeHash "${TEST_ARRAY[$test]}:$test:status" "${overallStatus}"
		echo "Compile of ${buildDir}/${TEST_ARRAY[$test]} FAILED!!"
	 
      	        # Disable this test for the rest of this run
		hput $exeHash "${TEST_ARRAY[$test]}:$test:exe" 0
	    else # binary exists
		overallStatus='Compiled'
		hput $exeHash "${TEST_ARRAY[$test]}:$test:status" "${overallStatus}"
		binaryCount=$(( $binaryCount + 1 ))
            
                # Disable this test if it was compile only
		hget $exeHash "${TEST_ARRAY[$test]}:$test:compileOnly" compileOnly
		if [ "${compileOnly}" == '1' ]
		then
		    hput $exeHash "${TEST_ARRAY[$test]}:$test:exe" 0
		fi
	    fi

	    # Update compile log for this test
	    hput $exeHash "${TEST_ARRAY[$test]}:$test:logCompile" "${make_log}"

	    # Log to web if compile failed or user chose to force results to DB
	    if [ "${overallStatus}" == 'Compile FAILED' ] || [ $forceWeb -eq 1 ]
	    then

		signal=999

		if [ "${overallStatus}" == 'Compile FAILED' ]
		then
		    signal=$( tail $compileLog | grep -a -m 1 ' Error' | awk '{print $NF}')

		    if [ $? -ne 0 ] || [ "${signal}" == "" ] || [[ ! "${signal}" =~ ^[0-9] ]] 
		    then # This compile didn't exit normally
			signal=10 # SIGUSR1
		    fi
		else
		    signal=0
		fi
		
		hget $exeHash "${TEST_ARRAY[$test]}:$test:stub" stub

		logToWeb "${buildDir} ${stub} ${TEST_ARRAY[$test]}" "0 0 0" "${nodeArray[0]} ${modeArray[0]}" "${overallStatus}" "" $signal "${overallStatus}" "${make_log}" xmlStat

	      # Determine NP value for saving XML status
		keyNP=0
		if [ $forceNP -eq 0 ]
		then
		    keyNP=$(( ${nodeArray[0]} * ${modeArray[0]} ))
		else
		    keyNP=$forceNP
		fi

		hput $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${nodeArray[0]}_m${modeArray[0]}_p${keyNP}" $xmlStat
	    fi

	done

        # Check status of make
	if [ ${makeStatus[0]} -ne 0 ]
	then
	    echo "ERROR (E): make FAILED!!"

	    # If not compile only
	    if [ $copy -eq 1 ] || [ $run -eq 1 ]
	    then

	        # Exit now if user didn't run make clean or didn't choose to ignore make fails
		if [ $make_clean -eq 0 ] && [ $ignoreMakeFails -eq 0 ]
		then
		    exit 1
		elif [ $binaryCount -eq 0 ] # Could continue, but no good binaries
		then
		    exit 1
		fi
	    fi
 
	else # make passed, let's check the log
	    if [[ $quietly -eq 0 &&  ${makeStatus[1]} -ne 0 ]] || [ ! -e "${make_log}" ]
	    then
		echo "WARNING (W): FAILED to redirect output into ${make_log}!!"
	    fi
	fi
    fi # end mpich/pami compile 	
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

    if [ $compile -ne 1 ] # Already know this if we compiled during this run
    then
	# Always do this for sst, only do this for pami if we ran with -rc (can only determine pami compile floor by reconfiguring)
	if [ "${codeFamily}" == 'sst' ] || [[ "${codeFamily}" == 'pami' && $reconfigure -eq 1 ]] 
	then
	    if [ "${cur_floor}" != "${compile_floor}" ]
	    then
		echo 'WARNING (W): Floor mismatch:'
		echo "WARNING (W): Latest floor      => ${cur_floor}"
		echo "WARNING (W): Compiled w/ floor => ${compile_floor}"
	    fi
	fi
    fi

    # --- Copy each binary to the exe dir ---
    echo "Copying binaries to exe dirs relative to: ${exe_base}"

    for ((test=0; test < ${#TEST_ARRAY[@]}; test++))
      do

      # Don't bother copying tests that didn't compile or are compile only
      hget $exeHash "${TEST_ARRAY[$test]}:$test:exe" enabled
      if [ "${enabled}" == '0' ]
	  then
	  echo "Skipping copy of ${TEST_ARRAY[$test]} ..."
	  continue
      fi

      # Don't recopy tests
      hget $copyHash ${TEST_ARRAY[$test]} alreadyCopied
      if [ "${alreadyCopied}" == '1' ]
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
	      exit 1
	  fi
      fi

      cp "${buildDir}/${TEST_ARRAY[$test]}" "${exeDir}"
      signal=$?
      if [ $? -ne 0 ] || [ ! -e "${exeDir}/${TEST_ARRAY[$test]}" ]
	  then
	  overallStatus='Copy FAILED'
	  hput $exeHash "${TEST_ARRAY[$test]}:$test:status" "${overallStatus}"
	  echo "Copy of ${buildDir}/${TEST_ARRAY[$test]} to ${exeDir} FAILED!!"
	  echo "Skipping to next test ..."

      	  # Disable this test for the rest of this run
	  hput $exeHash "${TEST_ARRAY[$test]}:$test:exe" 0

	  # Log to web if user planned to run this test
	  if [ $run -eq 1 ]
	  then
	    
	      hget $exeHash "${TEST_ARRAY[$test]}:$test:stub" stub

	      logToWeb "${exeDir} ${stub} ${TEST_ARRAY[$test]}" "0 0 0" "${nodeArray[0]} ${modeArray[0]}" "${overallStatus}" "" $signal "${overallStatus}" "" xmlStat

	      # Determine NP value for saving XML status
	      keyNP=0
	      if [ $forceNP -eq 0 ]
	      then
		  keyNP=$(( ${nodeArray[0]} * ${modeArray[0]} ))
	      else
		  keyNP=$forceNP
	      fi

	      hput $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${nodeArray[0]}_m${modeArray[0]}_p${keyNP}" $xmlStat	      
	  fi

	  continue # to next test
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
	  enabled=0
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:exe" enabled
	  if [ "${enabled}" == '1' ]
	  then

	      # Ensure file exists in exe dir
	      hget $exeHash "${TEST_ARRAY[$test]}:$test:exeDir" exeDir

	      if [ ! -e "${exeDir}/${TEST_ARRAY[$test]}" ]
	      then
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:status" 'Skipped (DNE)'
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
      enabled=0
      hget $exeHash "${TEST_ARRAY[$test]}:$test:exe" enabled
      if [ "${enabled}" == '1' ]
      then # update overall status
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:runPass" runPass
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:runTotal" runTotal
	  
	  hput $exeHash "${TEST_ARRAY[$test]}:$test:status" "${runPass}/${runTotal} Passed"
	  
      else # This test is disabled
	  # Log to web if this is a skipped test
	  overallStatus=''
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:status" overallStatus

	  if [[ "${overallStatus}" == 'Skipped (Compile Only)' && $compile -eq 0 ]] || [ "${overallStatus}" == 'Skipped (DNE)' ] || [ "${overallStatus}" == 'Skipped (I/O)' ]
	  then

	      hget $exeHash "${TEST_ARRAY[$test]}:$test:stub" stub
	      hget $exeHash "${TEST_ARRAY[$test]}:$test:exeDir" exeDir

	      logToWeb "${exeDir} ${stub} ${TEST_ARRAY[$test]}" "0 0 0" "${nodeArray[0]} ${modeArray[0]}" "${overallStatus}" "" 999 "${overallStatus}" "" xmlStat

	      # Determine NP value for saving XML status
	      keyNP=0
	      if [ $forceNP -eq 0 ]
	      then
		  keyNP=$(( ${nodeArray[0]} * ${modeArray[0]} ))
	      else
		  keyNP=$forceNP
	      fi

	      hput $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${nodeArray[0]}_m${modeArray[0]}_p${keyNP}" $xmlStat
	  fi
      fi
    done

    # --- Run selected binaries ---
    if [ ${#blockArray[@]} -gt 1 ]
    then
	hashLock=1
    fi

    for numNodes in ${nodeArray[@]}
    do
      for mode in ${modeArray[@]}
      do

	lastTest=""

	for ((test=0; test < ${#TEST_ARRAY[@]}; test++))
	do

	  block=""  
	  preProcRC=0
	  exeRC=0

	  hget $exeHash "${TEST_ARRAY[$test]}:$test:exe" enabled
	  if [ "${enabled}" != '1' ]
	  then # test is disabled
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
	  exeThreads=1       # Numeric value of # of threads used in run* subroutine
	  exeRuntime=0       # Elapsed time of run command
	  maxNP=0            # Maximum valid NP value for for this test

	  if [[ "${TEST_ARRAY[$test]}" =~ 'threadTest_omp' ]]
	  then
	      exeThreads=16
	  fi

	  # Get parms for this test
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:exeDir" exeDir
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:stub" stub
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:runOpts" runOpts
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:exeArgs" exeArgs

	  # Determine final values for nodes, mode & NP
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:standAlone" standAlone
	  if [ "${standAlone}" == '1' ]
	  then
	      exeNodes=0
	      eppInputMode=0
	      exeNP=0
	      exeThreads=0
	      exeOverride='N/A'
	  else
 	      # Get text version of BGP mode
	      if [ "${platform}" == 'bgp' ]
	      then
		  bgp_mode_NumtoText $mode eppInputMode
		  if [ $? -ne 0 ]
		  then
		      echo "ERROR (E): bgp_mode_NumtoText subroutine FAILED!!"

		      # Amend overall status
		      overallStatus='Skipped (Bad run* Parm)'
		      hput $exeHash "${TEST_ARRAY[$test]}:$test:status" "${overallStatus}"

	              # Disable test
		      hput $exeHash "${TEST_ARRAY[$test]}:$test:exe" 0

		      #Log to web		      
		      logToWeb "${exeDir} ${stub} ${TEST_ARRAY[$test]}" "0 0 0" "${nodeArray[0]} ${modeArray[0]}" "${overallStatus}" "" 999 "${overallStatus}" "" xmlStat

	              # Determine NP value for saving XML status
		      keyNP=0
		      if [ $forceNP -eq 0 ]
		      then
			  keyNP=$(( ${nodeArray[0]} * ${modeArray[0]} ))
		      else
			  keyNP=$forceNP
		      fi

		      hput $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${nodeArray[0]}_m${modeArray[0]}_p${keyNP}" $xmlStat

		      continue # to next test
		  fi
	      fi

	      exe_preProcessing "$numNodes $eppInputMode $numProcs $exeThreads" "${TEST_ARRAY[$test]##*/}" "${runOpts}" "${exeArgs}" "exeNodes exeMode exeNP exeThreads exeOpts exeOverride maxNP"

	      preProcRC=$?

	      if [ $preProcRC -ne 0 ]
	      then
		  echo -e "\nERROR (E):  exe_preProcessing sub FAILED!! Disabling ${TEST_ARRAY[$test]} test ...\n"
	      
	          # Amend overall status
		  overallStatus='Skipped (Bad run* Parm)'
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:status" "${overallStatus}"

	          # Disable test
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:exe" 0

		  # Log to web	      
		  logToWeb "${exeDir} ${stub} ${TEST_ARRAY[$test]}" "0 0 0" "${nodeArray[0]} ${modeArray[0]}" "${overallStatus}" "" 999 "${overallStatus}" "" xmlStat

	          # Determine NP value for saving XML status
		  keyNP=0
		  if [ $forceNP -eq 0 ]
		  then
		      keyNP=$(( ${nodeArray[0]} * ${modeArray[0]} ))
		  else
		      keyNP=$forceNP
		  fi

		  hput $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${nodeArray[0]}_m${modeArray[0]}_p${keyNP}" $xmlStat

		  continue # to next test
	      fi
	  fi

	  exeInputMode=$exeMode
	  # Get text version of BGP exe mode
	  if [ "${platform}" == 'bgp' ]
	  then
	      bgp_mode_NumtoText $exeMode exeInputMode
	      if [ $? -ne 0 ]
	      then
		  echo "ERROR (E): bgp_mode_NumtoText subroutine FAILED!!"

		  # Amend overall status
		  overallStatus='Skipped (Bad run* Parm)'
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:status" "${overallStatus}"

	          # Disable test
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:exe" 0

		  # Log to web		      
		  logToWeb "${exeDir} ${stub} ${TEST_ARRAY[$test]}" "0 0 0" "${nodeArray[0]} ${modeArray[0]}" "${overallStatus}" "" 999 "${overallStatus}" "" xmlStat

	          # Determine NP value for saving XML status
		  keyNP=0
		  if [ $forceNP -eq 0 ]
		  then
		      keyNP=$(( ${nodeArray[0]} * ${modeArray[0]} ))
		  else
		      keyNP=$forceNP
		  fi

		  hput $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${nodeArray[0]}_m${modeArray[0]}_p${keyNP}" $xmlStat

		  continue # to next test
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
		      exit 1
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

	  # Ensure this logfile is unique
	  while ( ! ( set -o noclobber; (echo -e "${runLog}\n" > "${runLog}")  >/dev/null 2>&1 ) )
	  do
	      if [ $debug -eq 1 ]
	      then
		  runLog="${out_dir}/${TEST_ARRAY[$test]%\.*}_$(date +%Y%m%d-%H%M%S).log.n${exeNodes}_m${exeMode}_p${exeNP}.$run_type.dummy"
	      else
		  runLog="${out_dir}/${TEST_ARRAY[$test]%\.*}_$(date +%Y%m%d-%H%M%S).log.n${exeNodes}_m${exeMode}_p${exeNP}.$run_type"
	      fi	      
	  done  

	  hput $exeHash "${TEST_ARRAY[$test]}:$test:log_n${numNodes}_m${mode}_p${numProcs}" $runLog

          # See if test has been disabled since we last checked
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:exe" enabled
	  if [ "${enabled}" == '0' ]
	  then
      
	      echo "Skipping execution of disabled test: ${TEST_ARRAY[$test]} (nodes = ${exeNodes}, mode = ${exeInputMode}, np = ${exeNP}) ..."

	      # Set individual test status and summary
	      testStatus=testSummary='Skipped (DOA)'
	      hput $exeHash "${TEST_ARRAY[$test]}:$test:status_n${numNodes}_m${mode}_p${numProcs}" $testStatus
	      hput $exeHash "${TEST_ARRAY[$test]}:$test:summary_n${numNodes}_m${mode}_p${numProcs}" $testSummary
 
	      # Ensure we report this right in the summary
	      {
		  echo "NP = ${exeNP}"
		  echo "THREADS = ${exeThreads}"
		  echo "NODES = ${exeNodes}"
		  if [ "${platform}" == 'bgp' ]
		  then
		      echo "MODE = ${exeInputMode}"
		  else
		      echo "BG_PROCESSESPERNODE = ${exeInputMode}"
		  fi
 
	      } >> $runLog


	      # Log to web
	      hget $exeHash "${TEST_ARRAY[$test]}:$test:status" overallStatus

	      logToWeb "${exeDir} ${stub} ${TEST_ARRAY[$test]}" "${exeNP} ${exeInputMode} ${exeThreads}" "${numNodes} ${mode}" "${overallStatus}" "${testStatus}" 999 "${testSummary}" "${runLog}" xmlStat

	      hput $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${numNodes}_m${mode}_p${numProcs}" $xmlStat

	      continue # to next test
	  fi

	  # Skip runs with NP > maxNP and user has not chosen to force scaling
	  if [ $forceScaling -eq 0 ] && [ $maxNP -gt 0 ] && [ $exeNP -gt $maxNP ]
	  then 
	      echo -e "Skipping ${TEST_ARRAY[$test]} (nodes = ${exeNodes}, mode = ${exeInputMode}, NP = ${exeNP}). NP (${exeNP}) > max NP for this test (${maxNP})."
                 
	      # Set individual test status and summary
	      testStatus=testSummary='Skipped (NP > maxNP)'
	      hput $exeHash "${TEST_ARRAY[$test]}:$test:status_n${numNodes}_m${mode}_p${numProcs}" $testStatus
	      hput $exeHash "${TEST_ARRAY[$test]}:$test:summary_n${numNodes}_m${mode}_p${numProcs}" $testSummary

	      # Ensure we report this right in the summary
	      {
		  echo "NP = ${exeNP}"
		  echo "THREADS = ${exeThreads}"
		  echo "NODES = ${exeNodes}"
		  if [ "${platform}" == 'bgp' ]
		  then
		      echo "MODE = ${exeInputMode}"
		  else
		      echo "BG_PROCESSESPERNODE = ${exeInputMode}"
		  fi
	      } >> $runLog

	      # Log to web
	      hget $exeHash "${TEST_ARRAY[$test]}:$test:status" overallStatus

	      logToWeb "${exeDir} ${stub} ${TEST_ARRAY[$test]}" "${exeNP} ${exeInputMode} ${exeThreads}" "${numNodes} ${mode}" "${overallStatus}" "${testStatus}" 999 "${testSummary}" "${runLog}" xmlStat

	      hput $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${numNodes}_m${mode}_p${numProcs}" $xmlStat

	      continue # to next test
	  fi

	  # Skip runs with NP that's not a power-of-2 (unimplemented)
	  npMinus1=$(( $exeNP - 1 ))
	  bitwiseNP=$(( $exeNP & $npMinus1 ? 1 : 0 ))

	  if [ $bitwiseNP != 0 ] # np is not a power-of-2
	  then
	      echo -e "Skipping ${TEST_ARRAY[$test]} (nodes = ${exeNodes}, mode = ${exeInputMode}, NP = ${exeNP}). NP value of ${exeNP} is not a power of 2."
                 
	      # Set individual test status and summary
	      testStatus=testSummary='Skipped (! ^2)'
	      hput $exeHash "${TEST_ARRAY[$test]}:$test:status_n${numNodes}_m${mode}_p${numProcs}" $testStatus
	      hput $exeHash "${TEST_ARRAY[$test]}:$test:summary_n${numNodes}_m${mode}_p${numProcs}" $testSummary

	      # Ensure we report this right in the summary
	      {
		  echo "NP = ${exeNP}"
		  echo "THREADS = ${exeThreads}"
		  echo "NODES = ${exeNodes}"
		  if [ "${platform}" == 'bgp' ]
		  then
		      echo "MODE = ${exeInputMode}"
		  else
		      echo "BG_PROCESSESPERNODE = ${exeInputMode}"
		  fi
	      } >> $runLog

	      # Log to web
	      hget $exeHash "${TEST_ARRAY[$test]}:$test:status" overallStatus

	      logToWeb "${exeDir} ${stub} ${TEST_ARRAY[$test]}" "${exeNP} ${exeInputMode} ${exeThreads}" "${numNodes} ${mode}" "${overallStatus}" "${testStatus}" 999 "${testSummary}" "${runLog}" xmlStat

	      hput $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${numNodes}_m${mode}_p${numProcs}" $xmlStat

	      continue # to next test
	  fi

          # See if this is a rerun
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:combo_n${exeNodes}_m${exeMode}_p${exeNP}" combo

	  if [ "${combo}" == "1" ]
	  then
	      echo -e "Skipping rerun of ${TEST_ARRAY[$test]} (nodes = ${exeNodes}, mode = ${exeInputMode}, NP = ${exeNP})"
                 
	      # Set individual test status and summary
	      testStatus=testSummary='Skipped (Rerun)'
	      hput $exeHash "${TEST_ARRAY[$test]}:$test:status_n${numNodes}_m${mode}_p${numProcs}" $testStatus
	      hput $exeHash "${TEST_ARRAY[$test]}:$test:summary_n${numNodes}_m${mode}_p${numProcs}" $testSummary

	      # Ensure we report this right in the summary
	      {
		  echo "NP = ${exeNP}"
		  echo "THREADS = ${exeThreads}"
		  echo "NODES = ${exeNodes}"
		  if [ "${platform}" == 'bgp' ]
		  then
		      echo "MODE = ${exeInputMode}"
		  else
		      echo "BG_PROCESSESPERNODE = ${exeInputMode}"
		  fi
	      } >> $runLog

	      # Log to web
	      hget $exeHash "${TEST_ARRAY[$test]}:$test:status" overallStatus

	      logToWeb "${exeDir} ${stub} ${TEST_ARRAY[$test]}" "${exeNP} ${exeInputMode} ${exeThreads}" "${numNodes} ${mode}" "${overallStatus}" "${testStatus}" 999 "${testSummary}" "${runLog}" xmlStat

	      hput $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${numNodes}_m${mode}_p${numProcs}" $xmlStat

	      continue # to next test
	  fi

          # Update hash with final node, mode and NP values to compare with next iteration
	  hput $exeHash "${TEST_ARRAY[$test]}:$test:combo_n${exeNodes}_m${exeMode}_p${exeNP}" 1

	  # Wait for an available block if necessary
	  if [ $needBlock -eq 1 ]
	  then
	      if [ ${#blockArray[@]} -gt 1 ]
	      then
		  echo -e "\nWaiting for a free block ..."
	      fi
	
	      block=""

	      while [ "${block}" == "" ]
	      do
		  # Look for a free block in the block hash
		  for ((index=0; index < ${#blockArray[@]}; index++))
		  do
		      hget $blockHash ${blockArray[$index]} blockStatus

		      if [ "${blockStatus}" == 'free' ]
		      then
			  block=${blockArray[$index]}
			  hput $blockHash ${blockArray[$index]} 'in use'
			  break
		      fi
		  done
	      done
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

          # Compare non-mpich binary floor with runtime floor
	  if [ $mpich -eq 0 ] && [ "${codeFamily}" == 'sst' ]
	  then
              bin_floor=$( strings "${exeDir}/${TEST_ARRAY[$test]}" | grep -a -m 1 "SST_COMPILE_DRIVER" )
	      bin_floor=${bin_floor##*=}

              if [ "${bin_floor}" != "${run_floor}" ]
	      then
		  echo "WARNING (W): Floor mismatch:"
		  echo "WARNING (W): Binary floor  => ${bin_floor}"
		  echo "WARNING (W): Runtime floor => ${run_floor}"
              fi
	  fi

	  # runfctest creates exe dirs with "second" granularity
	  # So when running the same test back-to-back, wait for 1 second, to avoid dir collisions
	  if [ "${TEST_ARRAY[$test]}" == "${lastTest}" ] && [ "${run_type}" != 'hw' ]
	  then
	      sleep 1
	  fi

	  # Psuedo-fork by creating new subshell using ()&
	  (
	      mypid=$$

              # Call correct run script
	      if [ $standAlone -eq 1 ]
	      then
		  runSA "${exeDir}" "${TEST_ARRAY[$test]}" "${runLog}" "exeSignal exeRuntime"
	      elif [ "${run_type}" != 'hw' ]
	      then
		  runSim $run_type "${exeDir}" "${TEST_ARRAY[$test]}" "${exeNodes} ${exeInputMode} ${exeNP}" "${exeOpts}" "${exeArgs}" "${runLog}" "exeSignal exeRuntime" $block
	      else 
		  runHW "${exeDir}" "${TEST_ARRAY[$test]}" "${exeNodes} ${exeInputMode} ${exeNP}" "${exeOpts}" $block "${exeArgs}" "${runLog}" "exeSignal exeRuntime"
	      fi

	      exeRC=$?
	      
	      # "Free" block
	      if [ $needBlock -eq 1 ]
	      then
		  hput $blockHash $block 'free'
	      fi

	      # Document signal and elapsed time for this run
	      if [ "${run_type}" != 'runFpga' ]
	      then
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:runtime_n${numNodes}_m${mode}_p${numProcs}" $exeRuntime

	          # Append run time to log file
		  echo "Elapsed time:  ${exeRuntime}" >> $runLog
	      else # FPGA run
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:runtime_n${numNodes}_m${mode}_p${numProcs}" 'Funknown' # Threw an 'F' in there since I cut off the first runtime character when printing summary to preserve tabs

	          # Also document fctest dir for FPGA runs
		  temp_fctest=$(fgrep -a "Test directory:" "${runLog}" | awk '{print $3}')
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:FPGAfctestDir_n${numNodes}_m${mode}_p${numProcs}" $temp_fctest
	      fi

              # Verify that the test "passed"
	      if [ $exeRC -eq 0 ] && [ "${run_type}" == 'runFpga' ]
	      then 
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:status_n${numNodes}_m${mode}_p${numProcs}" 'Submitted'
	      else # run runEval
		  runEval $standAlone $exeRC $exeSignal $runLog finalSignal exeSummary
	  
	          # Store signal & summary
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:signal_n${numNodes}_m${mode}_p${numProcs}" $finalSignal
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:summary_n${numNodes}_m${mode}_p${numProcs}" "${exeSummary}"

		  testStatus=""

		  if [ $finalSignal -eq 0 ]
		  then
		      testStatus='Passed'

		      hget $exeHash "${TEST_ARRAY[$test]}:$test:runPass" runPass
		      hput $exeHash "${TEST_ARRAY[$test]}:$test:runPass" $(( $runPass + 1 ))

		      echo "${TEST_ARRAY[$test]} ${testStatus}."
		  else
		      testStatus='FAILED'
		      
	              # Disable this test if failure is NOT due to "block is not booted"
		      if [[ ! $exeSummary =~ 'Boot' ]] || [[ ! $exeSummary =~ 'Block' ]] 
		      then
			  hput $exeHash "${TEST_ARRAY[$test]}:$test:exe" 0
		      fi

		      echo "${TEST_ARRAY[$test]} ${testStatus}!!"
		  fi

		  # Update individual test status
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:status_n${numNodes}_m${mode}_p${numProcs}" $testStatus

	          # Log to web
		  hget $exeHash "${TEST_ARRAY[$test]}:$test:status" overallStatus

		  logToWeb "${exeDir} ${stub} ${TEST_ARRAY[$test]}" "${exeNP} ${exeInputMode} ${exeThreads}" "${numNodes} ${mode}" "${overallStatus}" "${testStatus}" $finalSignal "${exeSummary}" "${runLog}" xmlStat

		  hput $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${numNodes}_m${mode}_p${numProcs}" $xmlStat

	      fi # end test verification
	  ) & # end psuedo-fork subshell
	
	  lastTest=${TEST_ARRAY[$test]}
      
	done # end test loop
      done # end mode loop
    done # end node loop

    # Wait for all children (subshells) to complete
    wait
    hashLock=0
fi # end EXECUTE section

# --- Monitor/Control FPGA Loadleveler Jobs ---
if [ "${run_type}" == 'runFpga' ] && [ $debug -eq 0 ]
then

    # Add up and gather job ids of submitted jobs
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

	  hget $exeHash "${TEST_ARRAY[$test]}:$test:status_n${numNodes}_m${mode}_p${numProcs}" testStatus
	  if [ "${testStatus}" == 'Submitted' ]
	  then
	      fpga_queue=$(( $fpga_queue + 1 )) # total number of jobs submitted

	      hget $exeHash "${TEST_ARRAY[$test]}:$test:log_n${numNodes}_m${mode}_p${numProcs}" runLog

	      jobLine=$(fgrep -a "llsubmit" "${runLog}" | cut -d '"' -f2)
	      jobID=${jobLine%%.*} # set to everything up to first "."
	      jobID="${jobID}.${jobLine##*.}" # add everything after last "."
	      hput $exeHash "${TEST_ARRAY[$test]}:$test:FPGAjobID_n${numNodes}_m${mode}_p${numProcs}" $jobID
	  fi

	done # end test loop
      done # end mode loop
    done # end node loop

    # Wait for any remaining jobs to complete
    if [ $fpga_queue -gt 0 ]
    then

	echo -e -n "\nWaiting for ${fpga_queue} job(s) to complete "

	while [ $fpga_queue -gt 0 ]
	do
            # See if any jobs are still in the queue
	    llJobs 'query'
	    if [ $? -ne 0 ]
	    then
		echo -e "\nERROR (E):  ll jobs query FAILED!!\n"
		break
	    fi

	    sleep 60
	    echo -n '.'
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
    if [ "${run_type}" == "runFpga" ]
	then

        # Append runtime data and determine results
	for ((test=0; test < ${#TEST_ARRAY[@]}; test++))
	  do

	  # Get parms needed for logging to the web
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:exeDir" exeDir
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:stub" stub
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:status" overallStatus

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
	      
	      hget $exeHash "${TEST_ARRAY[$test]}:$test:status_n${numNodes}_m${mode}_p${numProcs}" testStatus
	      if [ "${testStatus}" == 'Submitted' ] || [ "${testStatus}" == 'Cancelled' ]
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
		  # print error msg and change status if previous status was Submitted, but not if Cancelled
		  if [ "${testStatus}" == 'Submitted' ]
		  then
		      echo "ERROR (E): Runtime output file: ${outfile} DNE!! Skipping to the next test."

		      hput $exeHash "${TEST_ARRAY[$test]}:$test:status_n${numNodes}_m${mode}_p${numProcs}" 'Output DNE'
		  fi

		  continue
	      fi

              # Determine pass or fail of finished run
	      if [ "${testStatus}" == 'Submitted' ]
		  then
 
		  runEval 0 0 0 $runLog finalSignal exeSummary
	      
	          # Store signal & summary
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:signal_n${numNodes}_m${mode}_p${numProcs}" $finalSignal
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:summary_n${numNodes}_m${mode}_p${numProcs}" $exeSummary

		  testStatus=""

		  if [ $finalSignal -eq 0 ]
		  then
		      testStatus='Passed'

		      hget $exeHash "${TEST_ARRAY[$test]}:$test:runPass" runPass
		      hput $exeHash "${TEST_ARRAY[$test]}:$test:runPass" $(( $runPass + 1 ))
		  else
		      testStatus='FAILED'
		  fi

		  # Update individual test status
		  hput $exeHash "${TEST_ARRAY[$test]}:$test:status_n${numNodes}_m${mode}_p${numProcs}" $testStatus

	          # Log to web
	          # Determine np
		  np=$( grep -a "^NP =" ${runLog} | awk '{print $3}')


                  # Determine procs/node
		  if [ "${platform}" == 'bgp' ]
		  then
		      ppn=$( grep -a "^MODE =" ${runLog} | awk '{print $3}')
		      
                      # Get numerical version of mode
		      bgp_mode_TexttoNum $ppn ppn

		      if [ $? -ne 0 ]
		      then
			  echo "ERROR (E): bgp_mode_TexttoNum subroutine FAILED!!"
			  hput $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${numNodes}_m${mode}_p${numProcs}" 'FAILED'
			  continue # to next node/mode combo
		      fi
		  else
		      ppn=$( grep -a "^BG_PROCESSESPERNODE =" ${runLog} | awk '{print $3}')
		  fi

                  # Determine number of threads
		  threads=$( grep -a "^THREADS =" ${runLog} | awk '{print $3}')

		  logToWeb "${exeDir} ${stub} ${TEST_ARRAY[$test]}" "${np} ${ppn} ${threads}" "${numNodes} ${mode}" "${overallStatus}" "${testStatus}" $finalSignal "${exeSummary}" "${runLog}" xmlStat

		  hput $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${numNodes}_m${mode}_p${numProcs}" $xmlStat
		  
	      fi
	    done # mode loop
	  done # node loop
	done # test loop
    fi # end FPGA run eval
   
    # Update overall status of all executed tests
    for ((test=0; test < ${#TEST_ARRAY[@]}; test++))
      do
  
      hget $exeHash "${TEST_ARRAY[$test]}:$test:status" overallStatus
      hget $exeHash "${TEST_ARRAY[$test]}:$test:runTotal" runTotal
      hget $exeHash "${TEST_ARRAY[$test]}:$test:runPass" runPass

      # Overall status indicates this test reached the execution stage
      if [[ "${overallStatus}" =~ ^[0-9] ]]    
	  then
	  hput $exeHash "${TEST_ARRAY[$test]}:$test:status" "${runPass}/${runTotal} Passed"
      fi
    done
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

  hget $exeHash "${TEST_ARRAY[$test]}:$test:standAlone" standAlone
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

	  if [ "${standAlone}" == '1' ]
	      then 
	      np=''
	      ppn=''
	  else
              # Determine NP
	      np=$( grep -a "^NP =" ${runLog} | awk '{print $3}')

              # Determine procs/node
	      if [ "${platform}" == 'bgp' ]
	          then
		  ppn=$( grep -a "^MODE =" ${runLog} | awk '{print $3}')
	      else
		  ppn=$( grep -a "^BG_PROCESSESPERNODE =" ${runLog} | awk '{print $3}')
	      fi
	  fi

	  # Get status of this particular run in case it was skipped or was an FPGA run
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:status_n${nodeArray[0]}_m${modeArray[0]}_p${numProcs}" runStatus

	  # FPGA status could be Cancelled, Passed or FAILED
	  if [ "${run_type}" == 'runFpga' ]
	      then
	      echo -e -n "${system}\t${np}\t${ppn}\t${override}\tExe ${runStatus}" >> $summaryFile

	  else # Standalone, MMCS[_Lite] and Mambo runs are only Pass/FAIL
	      
	      if [[ "${hashStatus}" =~ ^0 ]]
		  then
		  if [[ ! "${runStatus}" =~ 'FAILED' ]]
		      then
		      echo -e -n "${system}\t${np}\t${ppn}\t${override}\t${runStatus}" >> $summaryFile
		  else
		      echo -e -n "${system}\t${np}\t${ppn}\t${override}\tExe FAILED" >> $summaryFile
		  fi
	      else
		  echo -e -n "${system}\t${np}\t${ppn}\t${override}\tExe Passed" >> $summaryFile
	      fi
	  fi
	  
	  # Print run time & XML status to the summary file
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:runtime_n${nodeArray[0]}_m${modeArray[0]}_p${numProcs}" runtime
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${nodeArray[0]}_m${modeArray[0]}_p${numProcs}" xmlStatus

	  if [ ${#hashStatus} -lt 8 ]
	      then
	      echo -e -n "\t\t${runtime:1:7}\t${xmlStatus}" >> $summaryFile
	  else
	      echo -e -n "\t${runtime:1:7}\t${xmlStatus}" >> $summaryFile 
	  fi

	  # Print log file to summary file if appropriate
	  if [[ ! "${runStatus}" =~ 'Skipped' ]]
	      then
	      echo -e "\t${runLog}" >> $summaryFile
	  else
	      echo "" >> $summaryFile
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

	      if [ "${standAlone}" == '1' ]
		  then 
		  np=''
		  ppn=''
	      elif [ -e "${runLog}" ]
		  then
		  # Determine NP
		  np=$( grep -a "^NP =" ${runLog} | awk '{print $3}')

                  # Determine procs/node
		  if [ "${platform}" == 'bgp' ]
		      then
		      ppn=$( grep -a "^MODE =" ${runLog} | awk '{print $3}')
		  else
		      ppn=$( grep -a "^BG_PROCESSESPERNODE =" ${runLog} | awk '{print $3}')
		  fi 
	      else
		  np=$numProcs
		  ppn=$mode

                  # Get text version of BGP mode
		  if [ "${platform}" == 'bgp' ]
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

		  echo "" >> $summaryFile 
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

      # Get and print the logXML status
      hget $exeHash "${TEST_ARRAY[$test]}:$test:xml_n${nodeArray[0]}_m${modeArray[0]}_p${numProcs}" xmlStatus
      if [ ${#hashStatus} -lt 8 ]
      then
	  echo -e -n "\t\t\t${xmlStatus}" >> $summaryFile
      elif [ ${#hashStatus} -lt 16 ]
      then
	  echo -e -n "\t\t${xmlStatus}" >> $summaryFile 
      else
	  echo -e -n "\t${xmlStatus}" >> $summaryFile 
      fi


      # If a compile was performed during this run, get the log path
      if [ $compile -eq 1 ] 
      then 
	  hget $exeHash "${TEST_ARRAY[$test]}:$test:logCompile" compileLog
	  echo -e "\t${compileLog}" >> $summaryFile
      else
	  echo "" >> $summaryFile
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
    echo -e "Subject: sst_verif Summary\n" >> $summaryEmail

    # Add user comment
    if [ "${comment}" != "" ]
	then
	echo -e "${comment}\n" >> $summaryEmail
    fi

    # Add summary file
    cat $summaryFile >> $summaryEmail

    # Add closing
    echo -e "\nThis notification email was automatically generated by sst_verif.sh\n" >> $summaryEmail
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

exit 0

# issue 2 beeps indicating test complete
if [[ A1 = A$ding ]]; then echo -en "\007"; sleep 1; echo -en "\007"; fi

