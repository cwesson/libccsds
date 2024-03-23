
CPP := g++
LD := g++

SRCS := $(shell find src/ -name \*.cpp)
OBJS := $(addsuffix .o,$(addprefix bin/,$(basename $(SRCS))))
DEPS := $(OBJS:%.o=%.d)

UTBIN := bin/unittest
CPPUTESTLIB := test/cpputest/src/CppUTest/libCppUTest.a test/cpputest/src/CppUTestExt/libCppUTestExt.a
UTCPPARGS := -Iinclude -Itest/cpputest/include -Wall -Wextra -Werror -g
UTLDARGS := -pthread
UTSRCS := $(SRCS) $(shell ls test/*.cpp)
UTOBJS := $(addsuffix .o,$(addprefix bin/ut/,$(basename $(UTSRCS))))
UTDEPS := $(UTOBJS:%.o=%.d)
UTARGS := -c -v -ojunit

-include $(DEPS)
-include $(UTDEPS)

.PHONY: all test unittest clean realclean

all: test

test: unittest

unittest: $(UTBIN)

$(UTBIN): $(UTOBJS) $(CPPUTESTLIB)
	$(LD) $(UTLDARGS) -o $@ $^
	$(UTBIN) $(UTARGS)

bin/ut/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CPP) $(UTCPPARGS) -MMD -o $@ -c $<

cpputest: $(CPPUTESTLIB)

$(CPPUTESTLIB):
	cd test/cpputest; cmake .
	make -C test/cpputest

.NOTPARALLEL:

clean:
	rm -rf $(OBJS) $(DEPS) $(UTOBJS) $(UTDEPS)

realclean:
	rm -rf bin/
