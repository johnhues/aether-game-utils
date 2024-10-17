//------------------------------------------------------------------------------
// main.cpp
// Copyright (c) John Hughes on 12/2/19. All rights reserved.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aether.h"
#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>

//------------------------------------------------------------------------------
// Unit test main
//------------------------------------------------------------------------------
int main( int argc, char* argv[] )
{
	AE_STATIC_ASSERT_MSG( AE_MEMORY_CHECKS, "Unit tests are intended to run with AE_MEMORY_CHECKS enabled" );
	const int result = Catch::Session().run( argc, argv );
	return result;
}
