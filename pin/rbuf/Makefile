CC=cc
CXX=g++

ifeq ($(PIN_HOME),)
error:
	@echo "PIN_HOME variable need to be set, before building"
endif

TOOLS_DIR	= tools

CXXFLAGS	+= -Werror -Wall -c -fomit-frame-pointer -std=c++0x -O3	\
		  -fno-strict-aliasing -fno-stack-protector	\
		  -DBIGARRAY_MULTIPLIER=1 -DUSING_XED		\
		  -DTARGET_IA32 -DHOST_IA32 -DTARGET_LINUX

CXXFLAGS_SO	+= -Wl,--hash-style=sysv -Wl,-Bsymbolic	-shared \
		   -Wl,--version-script=$(PIN_HOME)/source/include/pintool.ver

LIBS		+= -lpin  -lxed -ldwarf -lelf -ldl # -liberty

H_INCLUDE	+= -I. -I$(PIN_HOME)/source/include		\
		   -I$(PIN_HOME)/source/include/gen			\
		   -I$(PIN_HOME)/extras/xed2-ia32/include	\
		   -I$(PIN_HOME)/extras/components/include	\
		   -I$(PIN_HOME)/source/tools/InstLib 		\
		   -I../lib -I ../pri_runtime/src

L_INCLUDE	+= -L$(PIN_HOME)/extras/xed2-ia32/lib		\
		-L$(PIN_HOME)/ia32/lib -L$(PIN_HOME)/ia32/lib-ext

PROFILER_HEADERS=tagprop.h  profiler.h libtaint.h  memprof.h

TOOLS_OBJS=rbuf0.so rbuf1.so empty.so

all: $(TOOLS_OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(H_INCLUDE) -o $(@) $(@:.o=.cpp)

empty.so: empty.o
	$(CXX) $(CXXFLAGS_SO) $(L_INCLUDE) -o $(TOOLS_DIR)/$(@) \
		$<

rbuf0.so: rbuf0.o
	$(CXX) $(CXXFLAGS_SO) $(L_INCLUDE) -o $(TOOLS_DIR)/$(@) \
		$<

rbuf1.so: rbuf1.o
	$(CXX) $(CXXFLAGS_SO) $(L_INCLUDE) -o $(TOOLS_DIR)/$(@) \
		$<

clean:
	rm -f $(TOOLS_DIR)/*.so *.o
