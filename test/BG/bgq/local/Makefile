# use "make verify COPTS=-DVERBOSE=N" to turn on verbosity in verification mode,
# showing the first "N" errors (maximum).

CXX = /bgsys/drivers/ppcfloor/gnu-linux/bin/powerpc64-bgq-linux-g++
CXXFLAGS = -g -O3 -I/bgsys/drivers/ppcfloor

COPTS =

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

verify: localreduce.PUSHPULL.VERIFY	\
	localreduce.PULL.VERIFY		\
	localbcast.PUSH.VERIFY		\
	localbcast.PULL.VERIFY		\
	localbcast.PUSHPULL.VERIFY

%.PULL.8.10.32k: %.cc
	$(CXX) $? -o $@ $(CXXFLAGS) $(COPTS) -lpthread \
		-DNTHREADS=8 -DNITER=10 -DBUFCNT="(32*1024)" -DTEST_TYPE="PULL"

%.PUSH.8.10.32k: %.cc
	$(CXX) $? -o $@ $(CXXFLAGS) $(COPTS) -lpthread \
		-DNTHREADS=8 -DNITER=10 -DBUFCNT="(32*1024)" -DTEST_TYPE="PUSH"

%.PUSHPULL.8.10.32k: %.cc
	$(CXX) $? -o $@ $(CXXFLAGS) $(COPTS) -lpthread \
		-DNTHREADS=8 -DNITER=10 -DBUFCNT="(32*1024)" -DTEST_TYPE="PUSHPULL"

%.PULL.16.10.32k: %.cc
	$(CXX) $? -o $@ $(CXXFLAGS) $(COPTS) -lpthread \
		-DNTHREADS=16 -DNITER=10 -DBUFCNT="(32*1024)" -DTEST_TYPE="PULL"

%.PUSH.16.10.32k: %.cc
	$(CXX) $? -o $@ $(CXXFLAGS) $(COPTS) -lpthread \
		-DNTHREADS=16 -DNITER=10 -DBUFCNT="(32*1024)" -DTEST_TYPE="PUSH"

%.PUSHPULL.16.10.32k: %.cc
	$(CXX) $? -o $@ $(CXXFLAGS) $(COPTS) -lpthread \
		-DNTHREADS=16 -DNITER=10 -DBUFCNT="(32*1024)" -DTEST_TYPE="PUSHPULL"

%.PUSH.VERIFY: %.cc
	$(CXX) $? -o $@ $(CXXFLAGS) $(COPTS) -lpthread -DVERIFY \
		-DNTHREADS=8 -DNITER=10 -DBUFCNT="(32*1024)" -DTEST_TYPE="PUSH"

%.PULL.VERIFY: %.cc
	$(CXX) $? -o $@ $(CXXFLAGS) $(COPTS) -lpthread -DVERIFY \
		-DNTHREADS=8 -DNITER=10 -DBUFCNT="(32*1024)" -DTEST_TYPE="PULL"

%.PUSHPULL.VERIFY: %.cc
	$(CXX) $? -o $@ $(CXXFLAGS) $(COPTS) -lpthread -DVERIFY \
		-DNTHREADS=8 -DNITER=10 -DBUFCNT="(32*1024)" -DTEST_TYPE="PUSHPULL"
