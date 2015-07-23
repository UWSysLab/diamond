d := $(dir $(lastword $(MAKEFILE_LIST)))

SRCS += $(addprefix $(d), boost.cc)

OBJS-python := $(LIB-message) $(OBJS-data-types) $(o)boost.o 
