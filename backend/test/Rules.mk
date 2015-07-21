d := $(dir $(lastword $(MAKEFILE_LIST)))

#
# gtest-based tests
#
GTEST_SRCS += $(addprefix $(d), \
		backend-test.cc)

$(d)backend-test: $(o)backend-test.o $(LIB-message) $(OBJS-client) $(OBJS-data-types) $(GTEST_MAIN)

TEST_BINS += $(d)backend-test
