#!/usr/bin/python
import os
import sys

if ( len( sys.argv ) < 3 ):
   print "forkexec.py config_file test_to_run"
   sys.exit(2)

config = sys.argv[1]
test = sys.argv[2]
tmprev = test.split("/")
testname = tmprev[len(tmprev)-1]

for i in xrange(2):
    rc = os.fork()
    if (rc > 0):
        os.environ['XMI_SOCK_TASK'] = str(i)
        os.environ['XMI_SOCK_SIZE'] = str(2)
#        os.environ['XMI_UDP_CONFIG'] = '/gsa/rchgsa/home/j/e/jecarey/play/test.cfg'
        os.environ['XMI_UDP_CONFIG'] = config
#        os.execlp('/gsa/rchgsa/projects/o/opencl/jec/play/fe.py')
#        os.execlp('/gsa/rchgsa/home/j/e/jecarey/xmi/mbuild/xmi/test/p2p/send_latency.elf','send_latency')
        os.execlp(test,testname)
    elif (rc == 0):
        print "Started rank " + str(i)
    else:
        print "Failed to start rank " + str(i)


