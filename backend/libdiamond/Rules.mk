d := $(dir $(lastword $(MAKEFILE_LIST)))

$(d)libdiamond.so: $(patsubst %.o,%-pic.o, $(OBJS-client) $(OBJS-data-types))
LDFLAGS-$(d)libdiamond.so += -shared -lredox -lhiredis -lpython2.7 -lboost_python -Wl,-rpath=redox/release 

BINS += $(d)libdiamond.so
