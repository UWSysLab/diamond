d := $(dir $(lastword $(MAKEFILE_LIST)))

$(d)libdiamond.so: $(patsubst %.o,%-pic.o, $(OBJS-client) $(OBJ-data-types))
LDFLAGS-$(d)libdiamond.so += -shared

BINS += $(d)libdiamond.so
