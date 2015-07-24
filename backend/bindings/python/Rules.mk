d := $(dir $(lastword $(MAKEFILE_LIST)))

SRCS += $(addprefix $(d), boost.cc)

OBJS-python := $(LIB-message) $(OBJS-data-types) $(o)boost.o 

$(d)libdiamond.so: $(patsubst %.o,%-pic.o, $(LIB-message) $(OBJS-client) $(OBJS-data-types) $(OBJS-python))
LDFLAGS-$(d)libdiamond.so += -shared -lredox -lhiredis -lpython2.7 -lboost_python

UNAME_S := $(shell uname -s)
ifneq ($(UNAME_S),Darwin)
    LDFLAGS-$(d)libdiamond.so += -Wl,-rpath=redox/release
endif

BINS += $(d)libdiamond.so
