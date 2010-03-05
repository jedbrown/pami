# use my local mambo floor
# Build mambo for bgq with commfabric support

#set MY_BGQ_SANDBOX $env(BGQHOME)

# use mambo floor 
set my_mambo_dir /bgsys/bgq/drivers/x86_64.multi-mambo/
#set my_app_floor 
set my_app       NativeInterface.cnk
set my_pers      NativeInterface.cnk.pers
set my_ttype     cnk
set my_num_cores 2
set my_firmware  firmware.elf
# Amith's bgq_firmware.elf
#set my_firmware  /u/brunhe1/shared/bgq_firmware.elf
set my_tls	 off
set my_trace     off
set my_cyclemode off

# path where tcl script is located
# configure BGQ chip
set my_mambo_tcls_dir ${my_mambo_dir}/run/bgq/tcls

source ${my_mambo_tcls_dir}/bgq_cores.tcl

if {${my_tls}=="on"} {
  source ${my_mambo_tcls_dir}/debug_tools.tcl
}

set my_num_of_cores $my_num_cores
set my_test ""
# default 'my_trace' is off
# set my_trace ${my_trace_on}

define dup bgq my_conf


if {${my_cyclemode}=="on"} {
# These settings are required to support TLS
# Disable dataless mode in the caches

# Disable L1 Cache dataless mode
my_conf config cache l1_dataless off

# Disable L2 Cache dataless mode
my_conf config cache l2_dataless off

# L3 Cache size = 0
my_conf config cache l3 size  0

# Using only the dummy MU model
my_conf config mu dummy on
my_conf config mu dummy_rd_delay 200
my_conf config mu dummy_wr_delay 200
}

# Configures BGQ with default configuration
bgq_default_machine my_conf $my_num_of_cores

# Force it to run with 1 thread only
my_conf config processor/number_of_threads 4


if {${my_tls}=="on"} {
  
  # These settings are required to support TLS
  # Disable dataless mode in the caches

  # Disable L1 Cache dataless mode
  my_conf config cache l1_dataless off

  # Disable L2 Cache dataless mode
  my_conf config cache l2_dataless off

  # L3 Cache size = 0
  my_conf config cache l3 size  0

  # Using only the dummy MU model
  my_conf config mu dummy on
  my_conf config mu dummy_rd_delay 200
  my_conf config mu dummy_wr_delay 200
}

# Create a new instance of simulator

define machine my_conf my_sim


#
# load booter (relative path to executable)
#


#
# Load the Firmware, CNK, and app personality
#
#set my_booter  $my_app_floor/firmware/bin/bgq_firmware.elf

set my_kernel   /bgsys/bgq/drivers/x86_64.floor/cnk/bin/bgq_kernel.elf

################################################################################
## Procedure to load the booter and the application images on one core        ##
################################################################################
proc bgq_load_images { my_sim cpu_num num_of_cpus my_app my_firmware my_ttype my_pers my_kernel } {

$my_sim cpu $cpu_num load elf ${my_app}

# capture program info from registers
# Mambo sets the pc and toc registers from the elf file and points
# gpr1 to the top of physical memory (for use as program stack).
#
set my_app_iar [my_sim cpu $cpu_num display spr pc]
set my_mem_top [my_sim cpu $cpu_num display gpr  1]
set my_app_toc [my_sim cpu $cpu_num display gpr  2]

#
# load booter into cpu cpu_num
#
if {${my_ttype}=="cnk"} {
$my_sim cpu $cpu_num load elf $my_firmware
$my_sim cpu $cpu_num load elf $my_pers
$my_sim cpu $cpu_num load elf $my_kernel
}
if {${my_ttype}=="fwddr"} {
$my_sim cpu $cpu_num load elf $my_firmware
}

#set my_firmware_iar [my_sim cpu $cpu_num display spr pc]
# reset entry: top of 32-bit physical address
set my_firmware_reset_entry 0x0fffffffc

#
# set program counter and pass information to firmware via parameter registers
#
$my_sim cpu $cpu_num thread 0 set spr pc $my_firmware_reset_entry
$my_sim cpu $cpu_num thread 0 set gpr  3 $my_mem_top
$my_sim cpu $cpu_num thread 0 set gpr  4 $my_app_iar
$my_sim cpu $cpu_num thread 0 set gpr  5 $my_app_toc
$my_sim cpu $cpu_num thread 0 set gpr  6 $num_of_cpus

# set Processor Identification Register - PIR
set pir_value [expr {$cpu_num*4} ]

$my_sim cpu $cpu_num set spr pir $pir_value
}

#################################################################################
# Loop over all cores
for {set i 0} {$i < $my_num_of_cores} {incr i} {

        puts "About to load elf images (app, kernel, firmware)"

        bgq_load_images my_sim $i $my_num_of_cores $my_app $my_firmware $my_ttype $my_pers $my_kernel

        puts "About to reset shadow tlb"

        # Resets Shadow TLB cpu i
        #bgq_configure_reset_shadow_tlb my_sim $i

        bgq_configure_boot_edram_shadow_tlb my_sim $i
}

# Enable debug info:
#simdebug set "INSTRUCTION" 1
#simdebug set "INSTRUCTION_W_REGS" 1
#simdebug set "EXCEPTIONS" 1
#simdebug set "XLATE" 1
#simdebug set "QPX" 1
#simdebug set "MEM_REFS" 1;

#my_sim mode warmup
#my_sim mode cycle

puts "About to initialize THREAD_ACTIVE registers"

# Initializes THREAD_ACTIVE registers

set_thread_active_registers my_sim $my_num_of_cores



#if {${my_ttype}=="cnk"} {
#set my_kernel  $my_app_floor/cnk/bin/bgq_kernel.elf
#set my_test $my_app
#my_sim load elf $my_pers
#my_sim load elf $my_kernel
#my_sim load elf $my_firmware
#}
#if {${my_ttype}=="fwddr"} {
#set my_test $my_app
#my_sim load elf $my_firmware
#}
#if {${my_ttype}=="fwext"} {
#set my_test $my_app
#}


# Enable debug info:
#simdebug set "INSTRUCTION" 1
#simdebug set "INSTRUCTION_W_REGS" 1
#simdebug set "EXCEPTIONS" 1
#simdebug set "XLATE" 1
#simdebug set "QPX" 1


#
# launch instruction tracer, if desired
#
if ($my_trace=="yes") {
   if [catch { exec ${my_mambo_dir}/bin/emitter/reader [pid] & }] {
      puts "Can't start tracer"
      exit -1
      }

   ereader expect 1
   simemit set instructions 1
   simemit start
   }



#
# launch L1 Cache emitter reader
# if(1) enables reader
#
if (0) {
   if [catch { exec  ${my_mambo_dir}/bin/emitter/l1cache_reader [pid] ${my_app}_l1trace & }] {
      puts "Cannot start tracer"
      exit -1
   }

   ereader expect 1

   # Lists all instructions with cycle mode
   #simemit set "Instructions" 1

   # These work with warmup mode
   #simemit set "Memory_Write" 1
   #simemit set "Memory_Read" 1

   # These work with cycle mode
   simemit set "L1_DCache_Miss" 1
   #simemit set "L1_ICache_Miss" 1

   simemit start
}


#simdebug set "INSTRUCTION" 1;
#simdebug set "INSTRUCTION_W_REGS" 1;
#simdebug set "MEM_REFS" 1;

#simdebug set "BGQ_TORUS" 1

#
# enable PERCS cache model
#

#my_sim mode warmup

if {${my_cyclemode}=="on"} {
  my_sim mode cycle
}

#simdebug set "BGQ_MU_MMIO" 1


#
# launch simulation
#  
#
my_sim go
quit
