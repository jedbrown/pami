i~/cvs/bgq/work/gnu-linux/bin/powerpc64-bgq-linux-g++ -g -O3 localreduce.cc -o localreduce -I${HOME}/cvs/bgq/work -lpthread -DNTHREADS=8 -DNITER=10 -DBUFCNT="(32*1024)" -DTEST_TYPE="PULL"

CXX = /bgsys/drivers/ppcfloor/gnu-linux/bin/powerpc64-bgq-linux-g++
CXXFLAGS = -g -O3 -I/bgsys/drivers/ppcfloor

all: localreduce.PUSHPULL.8.10.32k	\
	localreduce.PULL.8.10.32k	\
	localreduce.PUSHPULL.16.10.32k	\
	localreduce.PULL.16.10.32k	\
	localbcast.PUSH.8.10.32k	\
	localbcast.PULL.8.10.32k	\
	localbcast.PUSHPULL.8.10.32k	\
	localbcast.PUSH.16.10.32k	\
	localbcast.PULL.16.10.32k	\
	localbcast.PUSHPULL.16.10.32k

%.PULL.8.10.32k: %.cc
	$(CXX) $? -o $@ $(CXXFLAGS) -lpthread \
		-DNTHREADS=8 -DNITER=10 -DBUFCNT="(32*1024)" -DTEST_TYPE="PULL"

%.PUSH.8.10.32k: %.cc
	$(CXX) $? -o $@ $(CXXFLAGS) -lpthread \
		-DNTHREADS=8 -DNITER=10 -DBUFCNT="(32*1024)" -DTEST_TYPE="PUSH"

%.PUSHPULL.8.10.32k: %.cc
	$(CXX) $? -o $@ $(CXXFLAGS) -lpthread \
		-DNTHREADS=8 -DNITER=10 -DBUFCNT="(32*1024)" -DTEST_TYPE="PUSHPULL"

%.PULL.16.10.32k: %.cc
	$(CXX) $? -o $@ $(CXXFLAGS) -lpthread \
		-DNTHREADS=16 -DNITER=10 -DBUFCNT="(32*1024)" -DTEST_TYPE="PULL"

%.PUSH.16.10.32k: %.cc
	$(CXX) $? -o $@ $(CXXFLAGS) -lpthread \
		-DNTHREADS=16 -DNITER=10 -DBUFCNT="(32*1024)" -DTEST_TYPE="PUSH"

%.PUSHPULL.16.10.32k: %.cc
	$(CXX) $? -o $@ $(CXXFLAGS) -lpthread \
		-DNTHREADS=16 -DNITER=10 -DBUFCNT="(32*1024)" -DTEST_TYPE="PUSHPULL"
