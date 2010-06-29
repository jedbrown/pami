# use "make verify COPTS=-DVERBOSE=N" to turn on verbosity in verification mode,
# showing the first "N" errors (maximum).

CXX = /bgsys/drivers/ppcfloor/gnu-linux/bin/powerpc64-bgq-linux-g++
CXXFLAGS = -g -O3 -I/bgsys/drivers/ppcfloor -I/bgsys/drivers/ppcfloor/spi/include/kernel/cnk

COPTS =

all:	localreduce.PUSHPULL.8.10.32k	\
	localreduce.PULL.8.10.32k	\
	localreduce.PUSHPULL.16.10.32k	\
	localreduce.PULL.16.10.32k	\
	localbcast.PUSH.8.10.32k	\
	localbcast.PULL.8.10.32k	\
	localbcast.PUSHPULL.8.10.32k	\
	localbcast.PUSH.16.10.32k	\
	localbcast.PULL.16.10.32k	\
	localbcast.PUSHPULL.16.10.32k

sow:	localbcast.PULL.16.10.256	\
	localbcast.PULL.64.10.256	\
	localbcast.PULL.16.10.1k	\
	localbcast.PULL.64.10.1k	\
	localbcast.PULL.16.10.8k	\
	localbcast.PULL.16.10.64k	\
	localbcast.PUSH.16.10.256	\
	localbcast.PUSH.64.10.256	\
	localbcast.PUSH.16.10.1k	\
	localbcast.PUSH.64.10.1k	\
	localbcast.PUSH.16.10.8k	\
	localbcast.PUSH.16.10.64k	\
	localbcast.PUSHPULL.16.10.256	\
	localbcast.PUSHPULL.64.10.256	\
	localbcast.PUSHPULL.16.10.1k	\
	localbcast.PUSHPULL.64.10.1k	\
	localbcast.PUSHPULL.16.10.8k	\
	localbcast.PUSHPULL.16.10.64k	\
	localreduce.PULL.16.10.256	\
	localreduce.PULL.64.10.256	\
	localreduce.PULL.16.10.64k	\
	localreduce.PUSHPULL.16.10.256	\
	localreduce.PUSHPULL.64.10.256	\
	localreduce.PUSHPULL.16.10.64k

verify: localreduce.VERIFY.0.PUSHPULL.16.1k	\
	localreduce.VERIFY.0.PULL.16.1k		\
	localbcast.VERIFY.0.PUSH.16.1k		\
	localbcast.VERIFY.0.PULL.16.1k		\
	localbcast.VERIFY.0.PUSHPULL.16.1k
	localreduce.VERIFY.0.PUSHPULL.64.1k	\
	localbcast.VERIFY.0.PUSHPULL.64.1k

################################################################################

# <program>.{PUSH|PULL|PUSHPULL}.<NTHREADS>.<NITER>.<BUFCNT>
localbcast.P%: localbcast.cc
	$(CXX) $? -o $@ $(CXXFLAGS) $(COPTS) -lpthread \
		-DTEST_TYPE=$(wordlist 2,2,$(subst ., ,$@))	\
		-DNTHREADS=$(wordlist 3,3,$(subst ., ,$@))	\
		-DNITER=$(wordlist 4,4,$(subst ., ,$@))	\
		-DBUFCNT="($(subst k,*1024,$(wordlist 5,5,$(subst ., ,$@))))"

localreduce.P%: localreduce.cc
	$(CXX) $? -o $@ $(CXXFLAGS) $(COPTS) -lpthread \
		-DTEST_TYPE=$(wordlist 2,2,$(subst ., ,$@))	\
		-DNTHREADS=$(wordlist 3,3,$(subst ., ,$@))	\
		-DNITER=$(wordlist 4,4,$(subst ., ,$@))	\
		-DBUFCNT="($(subst k,*1024,$(wordlist 5,5,$(subst ., ,$@))))"

# <program>.VERIFY.<VERBOSE>.{PUSH|PULL|PUSHPULL}.<NTHREADS>.<BUFCNT>
localbcast.V%: localbcast.cc
	$(CXX) $? -o $@ $(CXXFLAGS) $(COPTS) -lpthread -DVERIFY \
		-DVERBOSE=$(wordlist 3,3,$(subst ., ,$@))	\
		-DTEST_TYPE=$(wordlist 4,4,$(subst ., ,$@))	\
		-DNTHREADS=$(wordlist 5,5,$(subst ., ,$@))	\
		-DBUFCNT="($(subst k,*1024,$(wordlist 6,6,$(subst ., ,$@))))"

localreduce.V%: localreduce.cc
	$(CXX) $? -o $@ $(CXXFLAGS) $(COPTS) -lpthread -DVERIFY \
		-DVERBOSE=$(wordlist 3,3,$(subst ., ,$@))	\
		-DTEST_TYPE=$(wordlist 4,4,$(subst ., ,$@))	\
		-DNTHREADS=$(wordlist 5,5,$(subst ., ,$@))	\
		-DBUFCNT="($(subst k,*1024,$(wordlist 6,6,$(subst ., ,$@))))"
