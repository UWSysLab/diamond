d := $(dir $(lastword $(MAKEFILE_LIST)))

SRCS += $(addprefix $(d), long.cc counter.cc)

OBJS-data-types := $(LIB-message) $(o)long.o $(o)counter.o 
