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
// TestAllocator
//------------------------------------------------------------------------------
class TestAllocator : public ae::_DefaultAllocator
{
public:
	void* Allocate( ae::Tag tag, uint32_t bytes, uint32_t alignment ) override
	{
		void* result = _DefaultAllocator::Allocate( tag, bytes, alignment );
		allocs.Set( result, tag );
		return result;
	}

	void* Reallocate( void* data, uint32_t bytes, uint32_t alignment ) override
	{
		if( data )
		{
			void* result = _DefaultAllocator::Reallocate( data, bytes, alignment );
			allocs.Set( result, allocs.Get( data ) );
			allocs.Remove( data );
			return result;
		}
		return nullptr;
	}

	void Free( void* data ) override
	{
		_DefaultAllocator::Free( data );
		allocs.Remove( data );
	}

	bool IsThreadSafe() const override
	{
		return false;
	}

	ae::Map< void*, ae::Tag, 512 > allocs;
};

//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
int main( int argc, char* argv[] )
{
	TestAllocator allocator;
	ae::SetGlobalAllocator( &allocator );
	const int result = Catch::Session().run( argc, argv );
	if( allocator.allocs.Length() )
	{
		for( auto& alloc : allocator.allocs )
		{
			AE_ERR( "An allocation with tag '#' leaked", alloc.value );
		}
		AE_ERR( "Memory leak detected!" );
		return result -1;
	}
	return result;
}
