//------------------------------------------------------------------------------
// ScratchTest.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2025 John Hughes
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aether.h"
#include "TestConfig.h"
#include <catch2/catch_test_macros.hpp>
#include <thread>
#include <vector>
#include <string>

#ifndef __EMSCRIPTEN__
TEST_CASE( "Scratch concurrent allocations are thread local", "[scratch]" )
{
	constexpr int numThreads = 4;
	const int iterations = 250;
	std::vector< std::thread > threads;
	std::array< bool, numThreads > success;
	std::array< std::string, numThreads > errors;
	success.fill( true );

	s_intentionalAssert = true;
	for( int t = 0; t < numThreads; t++ )
	{
		threads.emplace_back( [t, iterations, &success, &errors]()
		{
			try
			{
				for( int i = 0; i < iterations; i++ )
				{
					ae::Scratch< uint64_t > outer( 128 );
					for( int j = 0; j < 128; j++ )
					{
						outer[ j ] = ( (uint64_t)t << 48 ) | ( (uint64_t)i << 16 ) | (uint64_t)j;
					}

					std::this_thread::yield();

					ae::Scratch< uint64_t > inner( 64 );
					for( int j = 0; j < 64; j++ )
					{
						inner[ j ] = 0xABCDEF0000000000ULL | ( (uint64_t)t << 32 ) | ( (uint64_t)i << 8 ) | (uint64_t)j;
					}

					std::this_thread::yield();

					for( int j = 0; j < 64; j++ )
					{
						const uint64_t expected = 0xABCDEF0000000000ULL | ( (uint64_t)t << 32 ) | ( (uint64_t)i << 8 ) | (uint64_t)j;
						if( inner[ j ] != expected )
						{
							success[ t ] = false;
							errors[ t ] = "inner scratch data was overwritten";
							return;
						}
					}

					for( int j = 0; j < 128; j++ )
					{
						const uint64_t expected = ( (uint64_t)t << 48 ) | ( (uint64_t)i << 16 ) | (uint64_t)j;
						if( outer[ j ] != expected )
						{
							success[ t ] = false;
							errors[ t ] = "outer scratch data was overwritten";
							return;
						}
					}
				}
			}
			catch( const char* msg )
			{
				success[ t ] = false;
				errors[ t ] = msg;
			}
		} );
	}

	for( auto& thread : threads )
	{
		thread.join();
	}
	s_intentionalAssert = false;

	for( int t = 0; t < numThreads; t++ )
	{
		INFO( errors[ t ] );
		REQUIRE( success[ t ] );
	}
}
#endif
