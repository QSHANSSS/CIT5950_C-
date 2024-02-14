# Copyright ©2024 Travis McGaha.  All rights reserved.  Permission is
# hereby granted to students registered for University of Pennsylvania
# CIT 5950 for use solely during Spring Semester 2024 for purposes of
# the course.  No other use, copying, distribution, or modification
# is permitted without prior written consent. Copyrights for
# third-party components of this work must be honored.  Instructors
# interested in reusing these course materials should contact the
# author.

.PHONY = clean all tidy-check format

# define the commands we will use for compilation and library building
CC = gcc-12
CXX = g++-12

# define useful flags to cc/ld/etc.
CFLAGS += -g -Wall -Wpedantic -I. -I.. -std=c2x -O0
CXXFLAGS += -g -Wall -Wpedantic -I. -I.. -std=c++23 -O0

# define common dependencies
OBJS = SimpleFileReader.o BufferedFileReader.o
HEADERS = SimpleFileReader.hpp BufferedFileReader.hpp BufferChecker.hpp
TESTOBJS = test_simplefilereader.o test_bufferedfilereader.o test_performance.o test_suite.o catch.o

CPP_SOURCE_FILES = SimpleFileReader.cpp BufferedFileReader.cpp
HPP_SOURCE_FILES = SimpleFileReader.hpp BufferedFileReader.hpp BufferChecker.hpp

# compile everything; this is the default rule that fires if a user
# just types "make" in the same directory as this Makefile
all: test_suite tidy-check

test_suite: $(TESTOBJS)  $(OBJS)
	$(CXX) $(CFLAGS) -o test_suite $(TESTOBJS) \
	$(CPPUNITFLAGS) $(OBJS) -lpthread $(LDFLAGS)

catch.o: catch.cpp catch.hpp
	$(CXX) $(CXXFLAGS) -c $<


%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $<

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $<

clean:
	/bin/rm -f *.o test_suite

# Phony Targets

# Checks under C++20 since C++23 is still experimental
# Explanantion of args:
#   -extra-arg=-std=c++20 : specifies to check with C++ 23
#   -warnings-as-error= : marks all warnings as errors
#   -checks= : specifies which checks to look for
#     modernize-* turns on all modernize checks to make sure we are using
#                 modern C++ features. C++ and C are different languages
#     cert-* turns on all errors related to CERT security checks.
#     readability-* turns on all errors related to making code more readable
#     cppcoreguidelines-* turns on errors related to the cppcoreguidelines
#                         https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines
#     bugprone-* turns on errors that will help identify frequent beginner bugs
#     -modernize-use-trailing-return-type disables the check that insists that a function
#                                         like `int main()` be written as `auto main() -> int`
#     -modernize-use-auto disables the check that says we should use auto during
#                         variable declaration.
#     -cppcoreguidelines-owning-memory disables the check that requires use of an external library
tidy-check: 
	clang-tidy \
        --extra-arg=--std=c++20 \
        -warnings-as-errors=* \
        -checks=modernize-*,cert-*,readability-*,cppcoreguidelines-*,bugprone-*,-modernize-use-trailing-return-type,-modernize-use-auto,-cppcoreguidelines-owning-memory,-bugprone-easily-swappable-parameters,-modernize-use-nodiscard,-cppcoreguidelines-pro-type-vararg,-modernize-pass-by-value,-modernize-use-default-member-init\
        -header-filter=.* \
        $(CPP_SOURCE_FILES) $(HPP_SOURCE_FILES)

format:
	clang-format -i --verbose --style=Chromium $(CPP_SOURCE_FILES) $(HPP_SOURCE_FILES)
