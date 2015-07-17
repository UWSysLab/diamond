d := $(dir $(lastword $(MAKEFILE_LIST)))

SRCS += $(addprefix $(d), \
	lookup3.cc message.cc)

LIB-hash := $(o)lookup3.o

LIB-message := $(o)message.o $(LIB-hash)
