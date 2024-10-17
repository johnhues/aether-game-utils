//------------------------------------------------------------------------------
// TestConfig.h
// Copyright (c) John Hughes on 3/2/24. All rights reserved.
//------------------------------------------------------------------------------
#ifndef TESTCONFIG_H
#define TESTCONFIG_H

#define AE_ASSERT_IMPL( msgStr ) throw "assert" // Throw exceptions so unit tests can test asserts
#define AE_MEMORY_CHECKS 1 // Enable strict memory checks for unit tests

#endif // TESTCONFIG_H
