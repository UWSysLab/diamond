d := $(dir $(lastword $(MAKEFILE_LIST)))

#
# gtest-based tests
#
GTEST_SRCS += $(addprefix $(d), \
		backend-test.cc)

$(d)backend-test: $(LIB-message) $(OBJS-client) $(OBJS-data-types) $(o)backend-test.o $(GTEST_MAIN)

TEST_BINS += $(d)backend-test
