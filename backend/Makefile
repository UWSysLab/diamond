# Top-level make file for Diamond library 

CC = gcc
CXX = g++
LD = g++
EXPAND = lib/tmpl/expand

CFLAGS := -g -Wall -Wno-uninitialized
CXXFLAGS := -std=c++0x
LDFLAGS := -Lredox/release

# Make sure all is the default
.DEFAULT_GOAL := all

# Eliminate default suffix rules
.SUFFIXES:

# Delete target files if there is an error (or make is interrupted)
.DELETE_ON_ERROR:

# make it so that no intermediate .o files are ever deleted
.PRECIOUS: %.o

##################################################################
# Tracing
#

ifeq ($(V),1)
trace = $(3)
Q =
else
trace = @printf "+ %-6s " $(1) ; echo $(2) ; $(3)
Q = @
endif

##################################################################
# Sub-directories
#

# The directory of the current make fragment.  Each file should
# redefine this at the very top with
#  d := $(dir $(lastword $(MAKEFILE_LIST)))
d :=

# The object directory corresponding to the $(d)
o = .obj/$(d)

# SRCS is the list of all non-test-related source files.
SRCS :=

# BINS is a list of target names for non-test binaries.  These targets
# should depend on the appropriate object files, but should not
# contain any commands.
BINS :=

# add-CFLAGS is a utility macro that takes a space-separated list of
# sources and a set of CFLAGS.  It sets the CFLAGS for each provided
# source.  This should be used like
#
#  $(call add-CFLAGS,$(d)a.c $(d)b.c,$(PG_CFLAGS))
define add-CFLAGS
$(foreach src,$(1),$(eval CFLAGS-$(src) += $(2)))
endef

# Like add-CFLAGS, but for LDFLAGS.  This should be given a list of
# binaries.
define add-LDFLAGS
$(foreach bin,$(1),$(eval LDFLAGS-$(bin) += $(2)))
endef

include lib/Rules.mk
include client/Rules.mk
include data_types/Rules.mk
include libdiamond/Rules.mk

##################################################################
# General rules
#

#
# Compilation
#

# -MD Enable dependency generation and compilation and output to the
# .obj directory.  -MP Add phony targets so make doesn't complain if
# a header file is removed.  -MT Explicitly set the target in the
# generated rule to the object file we're generating.
DEPFLAGS = -M -MF ${@:.o=.d} -MP -MT $@ -MG

# $(call add-CFLAGS,$(TEST_SRCS),$(CHECK_CFLAGS))
OBJS := $(SRCS:%.cc=.obj/%.o)

define compile
	@mkdir -p $(dir $@)
	$(call trace,$(1),$<,\
	  $(CC) -iquote. $(CFLAGS) $(CFLAGS-$<) $(2) $(DEPFLAGS) -E $<)
	$(Q)$(CC) -iquote. $(CFLAGS) $(CFLAGS-$<) $(2) -E -o .obj/$*.t $<
	$(Q)$(EXPAND) $(EXPANDARGS) -o .obj/$*.i .obj/$*.t
	$(Q)$(CC) $(CFLAGS) $(CFLAGS-$<) $(2) -c -o $@ .obj/$*.i
endef

define compilecxx
	@mkdir -p $(dir $@)
	$(call trace,$(1),$<,\
	  $(CXX) -iquote. $(CFLAGS) $(CXXFLAGS) $(CFLAGS-$<) $(2) $(DEPFLAGS) -E $<)
	$(Q)$(CXX) -iquote. $(CFLAGS) $(CXXFLAGS) $(CFLAGS-$<) $(2) -c -o $@ $<
endef

# All object files come in two flavors: regular and
# position-independent.  PIC objects end in -pic.o instead of just .o.
# Link targets that build shared objects must depend on the -pic.o
# versions.
# Slightly different rules for protobuf object files
# because their source files have different locations.

$(OBJS): .obj/%.o: %.cc
	$(call compilecxx,CC,)

$(OBJS:%.o=%-pic.o): .obj/%-pic.o: %.cc
	$(call compilecxx,CCPIC,-fPIC)

#
# Linking
#

$(call add-LDFLAGS,$(TEST_BINS),$(CHECK_LDFLAGS))

$(BINS) $(TEST_BINS): %:
	$(call trace,LD,$@,$(LD) -o $@ $^ $(LDFLAGS) $(LDFLAGS-$@))

#
# Automatic dependencies
#

DEPS := $(OBJS:.o=.d) $(OBJS:.o=-pic.d)

-include $(DEPS)

#
# Cleaning
#

.PHONY: clean
clean:
	$(call trace,RM,binaries,rm -f $(BINS))
	$(call trace,RM,objects,rm -rf .obj)


##################################################################
# Targets
#

.PHONY: all
all: $(BINS)

.PHONY: TAGS
TAGS:
	$(Q)rm -f $@
	$(call trace,ETAGS,sources,\
	  etags $(SRCS) $(TEST_SRCS))
	$(call trace,ETAGS,headers,\
	  etags -a $(foreach dir,$(sort $(dir $(SRCS) $(TEST_SRCS))),\
		     $(wildcard $(dir)*.h)))
