d := $(dir $(lastword $(MAKEFILE_LIST)))

SRCS += $(addprefix $(d), long.cc counter.cc string.cc boost.cc)

OBJS-data-types := $(LIB-message) $(o)long.o $(o)counter.o $(o)string.o $(o)boost.o 
