d := $(dir $(lastword $(MAKEFILE_LIST)))

$(d)libdiamond.so: $(patsubst %.o,%-pic.o, $(LIB-message) $(OBJS-client) $(OBJS-data-types))
LDFLAGS-$(d)libdiamond.so += -shared -lredox -lhiredis -lpython2.7 -lboost_python

UNAME_S := $(shell uname -s)
ifneq ($(UNAME_S),Darwin)
    LDFLAGS-$(d)libdiamond.so += -Wl,-rpath=redox/release
endif

BINS += $(d)libdiamond.so
