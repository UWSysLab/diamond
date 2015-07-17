d := $(dir $(lastword $(MAKEFILE_LIST)))

$(d)libdiamond.so: $(patsubst %.o,%-pic.o, $(OBJS-client) $(OBJS-data-types))
LDFLAGS-$(d)libdiamond.so += -shared

BINS += $(d)libdiamond.so
