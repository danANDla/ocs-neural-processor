SYSTEMC_HOME    = /usr/include/systemc
TARGET_ARCH     = linux64

SYSTEMC_INC_DIR = $(SYSTEMC_HOME)/include
SYSTEMC_LIB_DIR = $(SYSTEMC_HOME)/lib-$(TARGET_ARCH)

FLAGS           = -g -Wall -pedantic -Wno-long-long \
                 -DSC_INCLUDE_DYNAMIC_PROCESSES -fpermissive
LDFLAGS         = -lsystemc -lm

SRCS = src/testbenches.cpp src/computing_core.cpp src/netreader.cpp src/main.cpp src/data_reader.cpp
OBJS = $(SRCS:.cpp=.o)
	
main:
	g++ -o model $(LDFLAGS) $(FLAGS) $(SRCS)

