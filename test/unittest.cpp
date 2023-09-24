/**
 * @file unittest.cpp
 */

#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestHarness.h"

/**
 * @defgroup unittest Unit Tests
 * @{
 */

/**
 * Run unit tests.
 * @param argc Length of argv.
 * @param argv Arguments for cpputest.
 * @return 0 if all tests passed, non-0 otherwise.
 */
int main(int argc, char* argv[]){
	return CommandLineTestRunner::RunAllTests(argc, argv);
}

/** @} */ // group unittest
