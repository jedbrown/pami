#!/usr/bin/python
import os

for i in xrange(2):
    rc = os.fork()
    if (rc > 0):
        os.environ['XMI_SOCK_TASK'] = str(i)
        os.environ['XMI_SOCK_SIZE'] = str(2)
        os.environ['XMI_UDP_CONFIG'] = '/gsa/rchgsa/projects/o/opencl/jec/play/test.cfg'
#        os.execlp('/gsa/rchgsa/projects/o/opencl/jec/play/fe.py')
        os.execlp('/gsa/rchgsa/projects/o/opencl/jec/xmi/mbuild/xmi/test/p2p/send_latency.')
    elif (rc == 0):
        print "Started rank " + str(i)
    else:
        print "Failed to start rank " + str(i)


