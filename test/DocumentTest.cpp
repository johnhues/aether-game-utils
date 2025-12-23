//------------------------------------------------------------------------------
// Document.cpp
// Copyright (c) John Hughes on 6/15/24. All rights reserved.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aether.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE( "Any DefaultConstructor", "[ae::Any]" )
{
	ae::Any< 16, 8 > any;

	// Default constructed Any should not contain any value
	REQUIRE( any.TryGet< int >() == nullptr );
	REQUIRE( any.TryGet< float >() == nullptr );
	REQUIRE( any.TryGet< double >() == nullptr );
	REQUIRE( any.TryGet< bool >() == nullptr );
}

TEST_CASE( "Any ValueConstructor", "[ae::Any]" )
{
	// Test with int
	ae::Any< 16, 8 > anyInt( 42 );
	const int* intPtr = anyInt.TryGet< int >();
	REQUIRE( intPtr != nullptr );
	REQUIRE( *intPtr == 42 );

	// Test with float
	ae::Any< 16, 8 > anyFloat( 3.14f );
	const float* floatPtr = anyFloat.TryGet< float >();
	REQUIRE( floatPtr != nullptr );
	REQUIRE( *floatPtr == 3.14f );

	// Test with double
	ae::Any< 16, 8 > anyDouble( 2.718 );
	const double* doublePtr = anyDouble.TryGet< double >();
	REQUIRE( doublePtr != nullptr );
	REQUIRE( *doublePtr == 2.718 );

	// Test with bool
	ae::Any< 16, 8 > anyBool( true );
	const bool* boolPtr = anyBool.TryGet< bool >();
	REQUIRE( boolPtr != nullptr );
	REQUIRE( *boolPtr == true );
}

TEST_CASE( "Any AssignmentOperator", "[ae::Any]" )
{
	ae::Any< 16, 8 > any;

	// Initially empty
	REQUIRE( any.TryGet< int >() == nullptr );

	// Assign int
	any = 42;
	const int* intPtr = any.TryGet< int >();
	REQUIRE( intPtr != nullptr );
	REQUIRE( *intPtr == 42 );

	// Reassign different int value
	any = 100;
	intPtr = any.TryGet< int >();
	REQUIRE( intPtr != nullptr );
	REQUIRE( *intPtr == 100 );

	// Assign different type (float)
	any = 3.14f;
	const float* floatPtr = any.TryGet< float >();
	REQUIRE( floatPtr != nullptr );
	REQUIRE( *floatPtr == 3.14f );
	// Previous int type should no longer be retrievable
	REQUIRE( any.TryGet< int >() == nullptr );
}

TEST_CASE( "Any TryGet", "[ae::Any]" )
{
	ae::Any< 16, 8 > any( 42 );

	// Correct type should return valid pointer
	const int* intPtr = any.TryGet< int >();
	REQUIRE( intPtr != nullptr );
	REQUIRE( *intPtr == 42 );

	// Wrong type should return nullptr
	REQUIRE( any.TryGet< float >() == nullptr );
	REQUIRE( any.TryGet< double >() == nullptr );
	REQUIRE( any.TryGet< bool >() == nullptr );
	REQUIRE( any.TryGet< uint32_t >() == nullptr );
	REQUIRE( any.TryGet< int64_t >() == nullptr );
}

TEST_CASE( "Any Get", "[ae::Any]" )
{
	ae::Any< 16, 8 > any( 42 );

	// Correct type should return stored value
	REQUIRE( any.Get( 0 ) == 42 );
	REQUIRE( any.Get( 999 ) == 42 );

	// Wrong type should return default value
	REQUIRE( any.Get( 3.14f ) == 3.14f );
	REQUIRE( any.Get( 2.718 ) == 2.718 );
	REQUIRE( any.Get( true ) == true );

	// Test with different stored type
	ae::Any< 16, 8 > anyFloat( 1.5f );
	REQUIRE( anyFloat.Get( 0.0f ) == 1.5f );
	REQUIRE( anyFloat.Get( 999 ) == 999 ); // Wrong type returns default
}

TEST_CASE( "Any MultipleTypes", "[ae::Any]" )
{
	ae::Any< 16, 8 > any;

	// Store and retrieve int
	any = 123;
	REQUIRE( any.Get( 0 ) == 123 );
	REQUIRE( any.TryGet< float >() == nullptr );

	// Store and retrieve float
	any = 4.56f;
	REQUIRE( any.Get( 0.0f ) == 4.56f );
	REQUIRE( any.TryGet< int >() == nullptr );

	// Store and retrieve double
	any = 7.89;
	REQUIRE( any.Get( 0.0 ) == 7.89 );
	REQUIRE( any.TryGet< int >() == nullptr );
	REQUIRE( any.TryGet< float >() == nullptr );

	// Store and retrieve bool
	any = false;
	REQUIRE( any.Get( true ) == false );
	REQUIRE( any.TryGet< int >() == nullptr );
	REQUIRE( any.TryGet< double >() == nullptr );
}

TEST_CASE( "Any DifferentSizes", "[ae::Any]" )
{
	// Small Any that can hold int and float
	ae::Any< 4, 4 > smallAny;
	smallAny = 42;
	REQUIRE( smallAny.Get( 0 ) == 42 );
	smallAny = 3.14f;
	REQUIRE( smallAny.Get( 0.0f ) == 3.14f );

	// Medium Any that can hold double
	ae::Any< 8, 8 > mediumAny;
	mediumAny = 2.718;
	REQUIRE( mediumAny.Get( 0.0 ) == 2.718 );

	// Large Any
	ae::Any< 32, 8 > largeAny;
	largeAny = 123456789;
	REQUIRE( largeAny.Get( 0 ) == 123456789 );
}

TEST_CASE( "Any StructTypes", "[ae::Any]" )
{
	struct SimpleStruct
	{
		int x;
		float y;
	};

	ae::Any< 16, 8 > any;

	SimpleStruct value{ 10, 2.5f };
	any = value;

	const SimpleStruct* ptr = any.TryGet< SimpleStruct >();
	REQUIRE( ptr != nullptr );
	REQUIRE( ptr->x == 10 );
	REQUIRE( ptr->y == 2.5f );

	SimpleStruct retrieved = any.Get( SimpleStruct{ 0, 0.0f } );
	REQUIRE( retrieved.x == 10 );
	REQUIRE( retrieved.y == 2.5f );
}

TEST_CASE( "Any Vec3Types", "[ae::Any]" )
{
	ae::Any< sizeof( ae::Vec3 ), alignof( ae::Vec3 ) > any;

	ae::Vec3 vec( 1.0f, 2.0f, 3.0f );
	any = vec;

	const ae::Vec3* ptr = any.TryGet< ae::Vec3 >();
	REQUIRE( ptr != nullptr );
	REQUIRE( ptr->x == 1.0f );
	REQUIRE( ptr->y == 2.0f );
	REQUIRE( ptr->z == 3.0f );

	ae::Vec3 retrieved = any.Get( ae::Vec3( 0.0f ) );
	REQUIRE( retrieved.x == 1.0f );
	REQUIRE( retrieved.y == 2.0f );
	REQUIRE( retrieved.z == 3.0f );

	// Test with different value
	any = ae::Vec3( 4.5f, 5.5f, 6.5f );
	retrieved = any.Get( ae::Vec3( 0.0f ) );
	REQUIRE( retrieved.x == 4.5f );
	REQUIRE( retrieved.y == 5.5f );
	REQUIRE( retrieved.z == 6.5f );
}

TEST_CASE( "Any IntegerTypes", "[ae::Any]" )
{
	ae::Any< 16, 8 > any;

	// int8_t
	any = int8_t( 127 );
	REQUIRE( any.Get( int8_t( 0 ) ) == 127 );
	REQUIRE( any.TryGet< int8_t >() != nullptr );
	REQUIRE( any.TryGet< uint8_t >() == nullptr );

	// uint8_t
	any = uint8_t( 255 );
	REQUIRE( any.Get( uint8_t( 0 ) ) == 255 );
	REQUIRE( any.TryGet< uint8_t >() != nullptr );
	REQUIRE( any.TryGet< int8_t >() == nullptr );

	// int16_t
	any = int16_t( 32767 );
	REQUIRE( any.Get( int16_t( 0 ) ) == 32767 );
	REQUIRE( any.TryGet< int16_t >() != nullptr );

	// uint16_t
	any = uint16_t( 65535 );
	REQUIRE( any.Get( uint16_t( 0 ) ) == 65535 );
	REQUIRE( any.TryGet< uint16_t >() != nullptr );

	// int32_t
	any = int32_t( 2147483647 );
	REQUIRE( any.Get( int32_t( 0 ) ) == 2147483647 );
	REQUIRE( any.TryGet< int32_t >() != nullptr );

	// uint32_t
	any = uint32_t( 4294967295 );
	REQUIRE( any.Get( uint32_t( 0 ) ) == 4294967295 );
	REQUIRE( any.TryGet< uint32_t >() != nullptr );

	// int64_t
	any = int64_t( 9223372036854775807LL );
	REQUIRE( any.Get( int64_t( 0 ) ) == 9223372036854775807LL );
	REQUIRE( any.TryGet< int64_t >() != nullptr );

	// uint64_t
	any = uint64_t( 18446744073709551615ULL );
	REQUIRE( any.Get( uint64_t( 0 ) ) == 18446744073709551615ULL );
	REQUIRE( any.TryGet< uint64_t >() != nullptr );
}

TEST_CASE( "Any TypeSafety", "[ae::Any]" )
{
	ae::Any< 16, 8 > any;

	// Store int
	any = 42;
	REQUIRE( any.TryGet< int >() != nullptr );

	// uint32_t should not be retrievable even though it has same size on most platforms
	REQUIRE( any.TryGet< uint32_t >() == nullptr );

	// Store float
	any = 3.14f;
	REQUIRE( any.TryGet< float >() != nullptr );

	// int should not be retrievable even though float is 4 bytes on most platforms
	REQUIRE( any.TryGet< int >() == nullptr );
}

TEST_CASE( "Any Overwrite", "[ae::Any]" )
{
	ae::Any< 16, 8 > any( 42 );

	// Verify initial value
	REQUIRE( any.Get( 0 ) == 42 );

	// Overwrite with same type
	any = 100;
	REQUIRE( any.Get( 0 ) == 100 );

	// Overwrite with different type
	any = 3.14f;
	REQUIRE( any.Get( 0.0f ) == 3.14f );
	REQUIRE( any.TryGet< int >() == nullptr );

	// Overwrite with another different type
	any = true;
	REQUIRE( any.Get( false ) == true );
	REQUIRE( any.TryGet< float >() == nullptr );
	REQUIRE( any.TryGet< int >() == nullptr );
}

TEST_CASE( "Any ZeroValues", "[ae::Any]" )
{
	ae::Any< 16, 8 > any;

	// Store zero int
	any = 0;
	REQUIRE( any.TryGet< int >() != nullptr );
	REQUIRE( any.Get( 999 ) == 0 );

	// Store zero float
	any = 0.0f;
	REQUIRE( any.TryGet< float >() != nullptr );
	REQUIRE( any.Get( 999.0f ) == 0.0f );

	// Store false
	any = false;
	REQUIRE( any.TryGet< bool >() != nullptr );
	REQUIRE( any.Get( true ) == false );
}

TEST_CASE( "Any NegativeValues", "[ae::Any]" )
{
	ae::Any< 16, 8 > any;

	// Negative int
	any = -42;
	REQUIRE( any.Get( 0 ) == -42 );

	// Negative float
	any = -3.14f;
	REQUIRE( any.Get( 0.0f ) == -3.14f );

	// Negative double
	any = -2.718;
	REQUIRE( any.Get( 0.0 ) == -2.718 );
}

TEST_CASE( "Any ColorTypes", "[ae::Any]" )
{
	ae::Any< 16, 8 > any;

	// Color
	ae::Color color( 1.0f, 0.5f, 0.25f, 0.75f );
	any = color;

	const ae::Color* ptr = any.TryGet< ae::Color >();
	REQUIRE( ptr != nullptr );
	REQUIRE( ptr->r == 1.0f );
	REQUIRE( ptr->g == 0.5f );
	REQUIRE( ptr->b == 0.25f );
	REQUIRE( ptr->a == 0.75f );

	ae::Color retrieved = any.Get( ae::Color( 0.0f ) );
	REQUIRE( retrieved.r == 1.0f );
	REQUIRE( retrieved.g == 0.5f );
	REQUIRE( retrieved.b == 0.25f );
	REQUIRE( retrieved.a == 0.75f );
}

TEST_CASE( "Any NonConstTryGet", "[ae::Any]" )
{
	ae::Any< 16, alignof( ae::Vec3 ) > any;

	// Store int
	any = 42;

	// Non-const TryGet should return modifiable pointer
	int* intPtr = any.TryGet< int >();
	REQUIRE( intPtr != nullptr );
	REQUIRE( *intPtr == 42 );

	// Modify through pointer
	*intPtr = 100;
	REQUIRE( *intPtr == 100 );
	REQUIRE( any.Get( 0 ) == 100 );

	// Store struct
	struct Point
	{
		float x;
		float y;
	};

	any = Point{ 1.0f, 2.0f };
	Point* pointPtr = any.TryGet< Point >();
	REQUIRE( pointPtr != nullptr );
	REQUIRE( pointPtr->x == 1.0f );
	REQUIRE( pointPtr->y == 2.0f );

	// Modify struct through pointer
	pointPtr->x = 5.0f;
	pointPtr->y = 10.0f;
	REQUIRE( pointPtr->x == 5.0f );
	REQUIRE( pointPtr->y == 10.0f );

	Point retrieved = any.Get( Point{ 0.0f, 0.0f } );
	REQUIRE( retrieved.x == 5.0f );
	REQUIRE( retrieved.y == 10.0f );

	// Wrong type should still return nullptr
	REQUIRE( any.TryGet< int >() == nullptr );
	REQUIRE( any.TryGet< float >() == nullptr );

	// Test with Vec3
	any = ae::Vec3( 1.0f, 2.0f, 3.0f );
	ae::Vec3* vecPtr = any.TryGet< ae::Vec3 >();
	REQUIRE( vecPtr != nullptr );
	REQUIRE( vecPtr->x == 1.0f );
	REQUIRE( vecPtr->y == 2.0f );
	REQUIRE( vecPtr->z == 3.0f );

	// Modify Vec3
	vecPtr->x = 10.0f;
	vecPtr->y = 20.0f;
	vecPtr->z = 30.0f;

	ae::Vec3 vecRetrieved = any.Get( ae::Vec3( 0.0f ) );
	REQUIRE( vecRetrieved.x == 10.0f );
	REQUIRE( vecRetrieved.y == 20.0f );
	REQUIRE( vecRetrieved.z == 30.0f );
}

TEST_CASE( "Any ConstPointer", "[ae::Any]" )
{
	int value = 42;
	const int* constPtr = &value;

	// Store const pointer
	ae::Any< 16, 8 > any( constPtr );

	// Retrieve with TryGet - should maintain const
	const int* const* retrieved = any.TryGet< const int* >();
	REQUIRE( retrieved != nullptr );
	REQUIRE( *retrieved == constPtr );
	REQUIRE( **retrieved == 42 );

	// Retrieve with Get - should maintain const
	const int* gotPtr = any.Get< const int* >( nullptr );
	REQUIRE( gotPtr == constPtr );
	REQUIRE( *gotPtr == 42 );

	// Test with different const pointer
	int otherValue = 100;
	const int* otherConstPtr = &otherValue;
	any = otherConstPtr;

	gotPtr = any.Get< const int* >( nullptr );
	REQUIRE( gotPtr == otherConstPtr );
	REQUIRE( *gotPtr == 100 );

	// Verify non-const pointer type is different and returns nullptr
	REQUIRE( any.TryGet< int* >() == nullptr );

	// Test Get with default value when wrong type
	int* result = any.Get< int* >( nullptr );
	REQUIRE( result == nullptr ); // Should return default
}

TEST_CASE( "DocumentUndo Value", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Set initial value
	doc.ValueSet( "initial" );
	doc.EndUndoGroup();
	REQUIRE( doc.ValueGet() == std::string( "initial" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Change value
	doc.ValueSet( "changed" );
	doc.EndUndoGroup();
	REQUIRE( doc.ValueGet() == std::string( "changed" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should restore previous value
	REQUIRE( doc.Undo() );
	REQUIRE( doc.ValueGet() == std::string( "initial" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Redo should restore changed value
	REQUIRE( doc.Redo() );
	REQUIRE( doc.ValueGet() == std::string( "changed" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Multiple undos
	REQUIRE( doc.Undo() );
	REQUIRE( doc.ValueGet() == std::string( "initial" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 1 );
	REQUIRE( doc.Undo() );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 2 );
	REQUIRE( doc.Undo() == false ); // No more undo available
}

TEST_CASE( "DocumentUndo TypeChange", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Start as basic value
	doc.ValueSet( "test" );
	doc.EndUndoGroup();
	REQUIRE( doc.IsValue() );
	REQUIRE( doc.ValueGet() == std::string( "test" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Change to array
	doc.ArrayInitialize( 4 );
	doc.EndUndoGroup();
	REQUIRE( doc.IsArray() );
	REQUIRE( doc.ArrayLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should restore type and value
	REQUIRE( doc.Undo() );
	REQUIRE( doc.IsValue() );
	REQUIRE( doc.ValueGet() == std::string( "test" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Redo should restore array type
	REQUIRE( doc.Redo() );
	REQUIRE( doc.IsArray() );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );
}

TEST_CASE( "DocumentUndo ArrayOperations", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	doc.ArrayInitialize( 4 );
	doc.EndUndoGroup();
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Add array elements
	doc.ArrayAppend().ValueSet( "first" );
	doc.ArrayAppend().ValueSet( "second" );
	doc.ArrayAppend().ValueSet( "third" );
	doc.EndUndoGroup();

	REQUIRE( doc.ArrayLength() == 3 );
	REQUIRE( doc.ArrayGet( 0 ).ValueGet() == std::string( "first" ) );
	REQUIRE( doc.ArrayGet( 1 ).ValueGet() == std::string( "second" ) );
	REQUIRE( doc.ArrayGet( 2 ).ValueGet() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Insert in middle
	doc.ArrayInsert( 1 ).ValueSet( "inserted" );
	doc.EndUndoGroup();

	REQUIRE( doc.ArrayLength() == 4 );
	REQUIRE( doc.ArrayGet( 0 ).ValueGet() == std::string( "first" ) );
	REQUIRE( doc.ArrayGet( 1 ).ValueGet() == std::string( "inserted" ) );
	REQUIRE( doc.ArrayGet( 2 ).ValueGet() == std::string( "second" ) );
	REQUIRE( doc.ArrayGet( 3 ).ValueGet() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo insert
	REQUIRE( doc.Undo() );
	REQUIRE( doc.ArrayLength() == 3 );
	REQUIRE( doc.ArrayGet( 0 ).ValueGet() == std::string( "first" ) );
	REQUIRE( doc.ArrayGet( 1 ).ValueGet() == std::string( "second" ) );
	REQUIRE( doc.ArrayGet( 2 ).ValueGet() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Remove element
	doc.ArrayRemove( 1 );
	doc.EndUndoGroup();

	REQUIRE( doc.ArrayLength() == 2 );
	REQUIRE( doc.ArrayGet( 0 ).ValueGet() == std::string( "first" ) );
	REQUIRE( doc.ArrayGet( 1 ).ValueGet() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 3 ); // Redo stack was cleared, new operation added
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo remove should restore element at correct position
	REQUIRE( doc.Undo() );
	REQUIRE( doc.ArrayLength() == 3 );
	REQUIRE( doc.ArrayGet( 0 ).ValueGet() == std::string( "first" ) );
	REQUIRE( doc.ArrayGet( 1 ).ValueGet() == std::string( "second" ) );
	REQUIRE( doc.ArrayGet( 2 ).ValueGet() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 1 );
}

TEST_CASE( "DocumentUndo MapOperations", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	doc.MapInitialize( 4 );
	doc.EndUndoGroup();
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Add map values
	doc.MapSet( "key1" ).ValueSet( "value1" );
	doc.MapSet( "key2" ).ValueSet( "value2" );
	doc.EndUndoGroup();

	REQUIRE( doc.MapLength() == 2 );
	REQUIRE( doc.MapTryGet( "key1" )->ValueGet() == std::string( "value1" ) );
	REQUIRE( doc.MapTryGet( "key2" )->ValueGet() == std::string( "value2" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Remove a key
	REQUIRE( doc.MapRemove( "key1" ) ); // Should return true for successful removal
	doc.EndUndoGroup();

	REQUIRE( doc.MapLength() == 1 );
	REQUIRE( doc.MapTryGet( "key1" ) == nullptr );
	REQUIRE( doc.MapTryGet( "key2" )->ValueGet() == std::string( "value2" ) );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo remove should restore key and value
	REQUIRE( doc.Undo() );
	REQUIRE( doc.MapLength() == 2 );
	REQUIRE( doc.MapGetValue( 0 ).ValueGet() == std::string( "value1" ) );
	REQUIRE( doc.MapGetValue( 1 ).ValueGet() == std::string( "value2" ) );
	REQUIRE( doc.MapTryGet( "key1" )->ValueGet() == std::string( "value1" ) );
	REQUIRE( doc.MapTryGet( "key2" )->ValueGet() == std::string( "value2" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Redo remove
	REQUIRE( doc.Redo() );
	REQUIRE( doc.MapLength() == 1 );
	REQUIRE( doc.MapTryGet( "key1" ) == nullptr );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );
}

TEST_CASE( "DocumentUndo UndoGroups", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	doc.ArrayInitialize( 4 );

	// Multiple operations in one group
	doc.ArrayAppend().ValueSet( "first" );
	doc.ArrayAppend().ValueSet( "second" );
	doc.ArrayAppend().ValueSet( "third" );
	doc.EndUndoGroup(); // All three operations in one group

	REQUIRE( doc.ArrayLength() == 3 );
	REQUIRE( doc.GetUndoStackSize() == 1 ); // Single group with multiple operations
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Single undo should reverse entire group
	REQUIRE( doc.Undo() );
	REQUIRE( doc.GetType() == ae::DocumentValueType::None ); // Back to empty document
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Single redo should restore entire group
	REQUIRE( doc.Redo() );
	REQUIRE( doc.ArrayLength() == 3 );
	REQUIRE( doc.ArrayGet( 0 ).ValueGet() == std::string( "first" ) );
	REQUIRE( doc.ArrayGet( 1 ).ValueGet() == std::string( "second" ) );
	REQUIRE( doc.ArrayGet( 2 ).ValueGet() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );
}

TEST_CASE( "DocumentUndo ClearUndo", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	doc.ValueSet( "test" );
	doc.EndUndoGroup();
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	doc.ValueSet( "changed" );
	doc.EndUndoGroup();
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	REQUIRE( doc.Undo() ); // Should work before clear
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	doc.ClearUndo();
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	REQUIRE( doc.Undo() == false ); // Should fail after clear
	REQUIRE( doc.Redo() == false ); // Should fail after clear
}

TEST_CASE( "DocumentUndo ValueCoalescing", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Multiple ValueSet calls in same group should coalesce
	doc.ValueSet( "first" );
	REQUIRE( doc.ValueGet() == std::string( "first" ) );
	doc.ValueSet( "second" );
	REQUIRE( doc.ValueGet() == std::string( "second" ) );
	doc.ValueSet( "third" );
	REQUIRE( doc.ValueGet() == std::string( "third" ) );
	doc.EndUndoGroup();

	// Should only have one undo operation despite 3 ValueSet calls
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Single undo should revert to initial state (empty)
	REQUIRE( doc.Undo() );
	REQUIRE( doc.GetType() == ae::DocumentValueType::None );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Single redo should restore final value
	REQUIRE( doc.Redo() );
	REQUIRE( doc.ValueGet() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );
}

TEST_CASE( "DocumentUndo ValueSeparateGroups", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// First value with separate undo group
	doc.ValueSet( "first" );
	doc.EndUndoGroup();
	REQUIRE( doc.ValueGet() == std::string( "first" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Second value with separate undo group (no coalescing)
	doc.ValueSet( "second" );
	doc.EndUndoGroup();
	REQUIRE( doc.ValueGet() == std::string( "second" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Third value with separate undo group (no coalescing)
	doc.ValueSet( "third" );
	doc.EndUndoGroup();
	REQUIRE( doc.ValueGet() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Multiple undos should step through each value
	REQUIRE( doc.Undo() );
	REQUIRE( doc.ValueGet() == std::string( "second" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	REQUIRE( doc.Undo() );
	REQUIRE( doc.ValueGet() == std::string( "first" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 2 );

	REQUIRE( doc.Undo() );
	REQUIRE( doc.GetType() == ae::DocumentValueType::None );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 3 );
}

TEST_CASE( "DocumentUndo ComplexNesting", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	doc.ArrayInitialize( 4 );

	// Create nested structure
	doc.ArrayAppend().MapInitialize( 2 );
	doc.ArrayGet( 0 ).MapSet( "nested" ).ValueSet( "deep" );
	doc.EndUndoGroup();

	REQUIRE( doc.ArrayLength() == 1 );
	REQUIRE( doc.ArrayGet( 0 ).IsMap() );
	REQUIRE( doc.ArrayGet( 0 ).MapSet( "nested" ).ValueGet() == std::string( "deep" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Modify nested value
	doc.ArrayGet( 0 ).MapSet( "nested" ).ValueSet( "modified" );
	doc.EndUndoGroup();

	REQUIRE( doc.ArrayGet( 0 ).MapSet( "nested" ).ValueGet() == std::string( "modified" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should restore deep nested value
	REQUIRE( doc.Undo() );
	REQUIRE( doc.ArrayGet( 0 ).MapSet( "nested" ).ValueGet() == std::string( "deep" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Remove entire nested structure
	doc.ArrayRemove( 0 );
	doc.EndUndoGroup();

	REQUIRE( doc.ArrayLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 2 ); // Redo stack was cleared
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should restore entire nested structure
	REQUIRE( doc.Undo() );
	REQUIRE( doc.ArrayLength() == 1 );
	REQUIRE( doc.ArrayGet( 0 ).IsMap() );
	REQUIRE( doc.ArrayGet( 0 ).MapSet( "nested" ).ValueGet() == std::string( "deep" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 1 );
}

TEST_CASE( "DocumentUndo MapIteration", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	doc.MapInitialize( 4 );
	doc.EndUndoGroup();
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Add multiple map values with different keys
	doc.MapSet( "apple" ).ValueSet( "fruit1" );
	doc.MapSet( "banana" ).ValueSet( "fruit2" );
	doc.MapSet( "cherry" ).ValueSet( "fruit3" );
	doc.EndUndoGroup();

	REQUIRE( doc.MapLength() == 3 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test MapGetKey and MapGetValue iteration
	bool foundApple = false, foundBanana = false, foundCherry = false;
	for( uint32_t i = 0; i < doc.MapLength(); i++ )
	{
		const char* key = doc.MapGetKey( i );
		const ae::DocumentValue& value = doc.MapGetValue( i );

		if( std::string( key ) == "apple" )
		{
			REQUIRE( value.ValueGet() == std::string( "fruit1" ) );
			foundApple = true;
		}
		else if( std::string( key ) == "banana" )
		{
			REQUIRE( value.ValueGet() == std::string( "fruit2" ) );
			foundBanana = true;
		}
		else if( std::string( key ) == "cherry" )
		{
			REQUIRE( value.ValueGet() == std::string( "fruit3" ) );
			foundCherry = true;
		}
	}
	REQUIRE( foundApple );
	REQUIRE( foundBanana );
	REQUIRE( foundCherry );

	// Test non-const MapGetValue for modification
	for( uint32_t i = 0; i < doc.MapLength(); i++ )
	{
		const char* key = doc.MapGetKey( i );
		ae::DocumentValue& value = doc.MapGetValue( i );

		if( std::string( key ) == "banana" )
		{
			value.ValueSet( "yellow_fruit" );
			break;
		}
	}
	doc.EndUndoGroup();

	// Verify the modification
	REQUIRE( doc.MapTryGet( "banana" )->ValueGet() == std::string( "yellow_fruit" ) );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Remove one key and verify iteration still works
	REQUIRE( doc.MapRemove( "apple" ) ); // Should return true for successful removal
	doc.EndUndoGroup();

	REQUIRE( doc.MapLength() == 2 );
	REQUIRE( doc.GetUndoStackSize() == 4 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Verify remaining keys through iteration
	foundBanana = false;
	foundCherry = false;
	foundApple = false;
	for( uint32_t i = 0; i < doc.MapLength(); i++ )
	{
		const char* key = doc.MapGetKey( i );
		const ae::DocumentValue& value = doc.MapGetValue( i );
		if( std::string( key ) == "apple" )
		{
			foundApple = true;
		}
		else if( std::string( key ) == "banana" )
		{
			foundBanana = true;
		}
		else if( std::string( key ) == "cherry" )
		{
			foundCherry = true;
		}
	}
	REQUIRE( !foundApple ); // Should not be found after removal
	REQUIRE( foundBanana );
	REQUIRE( foundCherry );

	// Undo remove and verify iteration works with restored key
	REQUIRE( doc.Undo() );
	REQUIRE( doc.MapLength() == 3 );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Verify all keys are back through iteration
	foundApple = false;
	foundBanana = false;
	foundCherry = false;
	for( uint32_t i = 0; i < doc.MapLength(); i++ )
	{
		const char* key = doc.MapGetKey( i );

		if( std::string( key ) == "apple" )
		{
			foundApple = true;
		}
		else if( std::string( key ) == "banana" )
		{
			foundBanana = true;
		}
		else if( std::string( key ) == "cherry" )
		{
			foundCherry = true;
		}
	}
	REQUIRE( foundApple ); // Should be found after undo
	REQUIRE( foundBanana );
	REQUIRE( foundCherry );
}

TEST_CASE( "DocumentUndo MapRemoveReturnValue", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	doc.MapInitialize( 4 );
	doc.EndUndoGroup();
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Add some map values
	doc.MapSet( "existing1" ).ValueSet( "value1" );
	doc.MapSet( "existing2" ).ValueSet( "value2" );
	doc.EndUndoGroup();

	REQUIRE( doc.MapLength() == 2 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test successful removal - should return true
	REQUIRE( doc.MapRemove( "existing1" ) == true );
	doc.EndUndoGroup();

	REQUIRE( doc.MapLength() == 1 );
	REQUIRE( doc.MapTryGet( "existing1" ) == nullptr );
	REQUIRE( doc.MapTryGet( "existing2" ) != nullptr );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test unsuccessful removal - should return false for non-existent key
	REQUIRE( doc.MapRemove( "nonexistent" ) == false );
	doc.EndUndoGroup();

	// Map should be unchanged after failed removal
	REQUIRE( doc.MapLength() == 1 );
	REQUIRE( doc.MapTryGet( "existing2" ) != nullptr );
	// Undo stack should be unchanged since no operation was performed
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test removing the same key again - should return false
	REQUIRE( doc.MapRemove( "existing1" ) == false ); // Already removed
	doc.EndUndoGroup();

	// Map should still be unchanged
	REQUIRE( doc.MapLength() == 1 );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test successful removal of remaining key
	REQUIRE( doc.MapRemove( "existing2" ) == true );
	doc.EndUndoGroup();

	REQUIRE( doc.MapLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 4 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test removal from empty map - should return false
	REQUIRE( doc.MapRemove( "anything" ) == false );
	doc.EndUndoGroup();

	REQUIRE( doc.MapLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 4 ); // No change since no operation performed
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo and test that removed keys can be successfully removed again
	REQUIRE( doc.Undo() ); // Restore "existing2"
	REQUIRE( doc.MapLength() == 1 );
	REQUIRE( doc.MapTryGet( "existing2" ) != nullptr );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Should be able to remove it again
	REQUIRE( doc.MapRemove( "existing2" ) == true );
	doc.EndUndoGroup();

	REQUIRE( doc.MapLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 4 ); // Redo stack cleared, new operation added
	REQUIRE( doc.GetRedoStackSize() == 0 );
}

TEST_CASE( "DocumentUndo MapGetVsTryGet", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	const ae::DocumentValue& constDoc = doc;
	doc.MapInitialize( 4 );
	doc.EndUndoGroup();
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Add some map values
	doc.MapSet( "existing" ).ValueSet( "value" );
	doc.EndUndoGroup();

	REQUIRE( doc.MapLength() == 1 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test const MapTryGet with existing key - should return valid pointer
	const ae::DocumentValue* constExisting = constDoc.MapTryGet( "existing" );
	REQUIRE( constExisting != nullptr );
	REQUIRE( constExisting->ValueGet() == std::string( "value" ) );

	// Test const MapTryGet with non-existing key - should return nullptr
	const ae::DocumentValue* constNonExisting = constDoc.MapTryGet( "nonexistent" );
	REQUIRE( constNonExisting == nullptr );

	// Test non-const MapTryGet with existing key - should return valid pointer
	ae::DocumentValue* nonConstExisting = doc.MapTryGet( "existing" );
	REQUIRE( nonConstExisting != nullptr );
	REQUIRE( nonConstExisting->ValueGet() == std::string( "value" ) );

	// Test non-const MapTryGet with non-existing key - should return nullptr
	ae::DocumentValue* nonConstNonExisting = doc.MapTryGet( "nonexistent" );
	REQUIRE( nonConstNonExisting == nullptr );

	// Test modification through non-const MapTryGet pointer
	ae::DocumentValue* modifyPtr = doc.MapTryGet( "existing" );
	REQUIRE( modifyPtr != nullptr );
	modifyPtr->ValueSet( "modified_via_tryget" );
	doc.EndUndoGroup();

	// Verify modification worked
	REQUIRE( doc.MapTryGet( "existing" )->ValueGet() == std::string( "modified_via_tryget" ) );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	ae::DocumentValue& nonConstRef = doc.MapSet( "existing" );
	REQUIRE( nonConstRef.ValueGet() == std::string( "modified_via_tryget" ) );

	// Modify through MapGet reference
	nonConstRef.ValueSet( "modified_value" );
	doc.EndUndoGroup();

	// Verify modification
	REQUIRE( doc.MapTryGet( "existing" )->ValueGet() == std::string( "modified_value" ) );
	REQUIRE( doc.GetUndoStackSize() == 4 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Note: MapGet with non-existent key would assert in debug builds
	// so we don't test that scenario
}

TEST_CASE( "DocumentUndo EdgeCasesAndInitialState", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );

	// Test initial empty document state
	REQUIRE( doc.GetType() == ae::DocumentValueType::None );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test that operations on empty document work
	REQUIRE( doc.Undo() == false ); // Nothing to undo
	REQUIRE( doc.Redo() == false ); // Nothing to redo

	// Test EndUndoGroup on empty document
	doc.EndUndoGroup(); // Should not crash or create undo entry
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test ClearUndo on empty document
	doc.ClearUndo(); // Should not crash
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test setting empty basic value
	doc.ValueSet( "" );
	doc.EndUndoGroup();
	REQUIRE( doc.IsValue() );
	REQUIRE( doc.ValueGet() == std::string( "" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test setting same value (should not create new undo operation)
	doc.ValueSet( "" );
	doc.EndUndoGroup();
	REQUIRE( doc.GetUndoStackSize() == 1 ); // Should still be 1
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test array with zero reserve
	doc.ArrayInitialize( 0 );
	doc.EndUndoGroup();
	REQUIRE( doc.IsArray() );
	REQUIRE( doc.ArrayLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test map with zero reserve
	doc.MapInitialize( 0 );
	doc.EndUndoGroup();
	REQUIRE( doc.IsMap() );
	REQUIRE( doc.MapLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test MapRemove on empty map
	REQUIRE( doc.MapRemove( "anything" ) == false );
	doc.EndUndoGroup();
	REQUIRE( doc.MapLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 3 ); // Should not change
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test MapTryGet on empty map
	REQUIRE( doc.MapTryGet( "anything" ) == nullptr );

	// Test multiple type changes in same group - should coalesce SetType operations
	doc.ValueSet( "test" );
	doc.ArrayInitialize( 2 );
	doc.MapInitialize( 2 );
	doc.EndUndoGroup(); // Should coalesce into single type change operation

	REQUIRE( doc.IsMap() );
	REQUIRE( doc.GetUndoStackSize() == 4 ); // Previous 3 + 1 coalesced group
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should revert to previous state (empty map from before the coalesced changes)
	REQUIRE( doc.Undo() );
	REQUIRE( doc.IsMap() );
	REQUIRE( doc.MapLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 1 );
}

TEST_CASE( "DocumentUndo Initialize", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test Initialize with same type (None -> None) - should be no-op
	ae::DocumentValue& result = doc.Initialize( ae::DocumentValueType::None );
	REQUIRE( &result == &doc ); // Should return reference to self
	REQUIRE( doc.GetType() == ae::DocumentValueType::None );
	REQUIRE( doc.GetUndoStackSize() == 0 ); // No undo operation created
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test Initialize from None to Basic
	doc.Initialize( ae::DocumentValueType::Value );
	doc.EndUndoGroup();
	REQUIRE( doc.GetType() == ae::DocumentValueType::Value );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Set a basic value to test clearing
	doc.ValueSet( "test_value" );
	doc.EndUndoGroup();
	REQUIRE( doc.ValueGet() == std::string( "test_value" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test Initialize from Basic to Array (should clear basic value)
	doc.Initialize( ae::DocumentValueType::Array );
	doc.EndUndoGroup();
	REQUIRE( doc.GetType() == ae::DocumentValueType::Array );
	REQUIRE( doc.ArrayLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Add array elements to test clearing
	doc.ArrayAppend().ValueSet( "array1" );
	doc.ArrayAppend().ValueSet( "array2" );
	doc.ArrayAppend().ValueSet( "array3" );
	doc.EndUndoGroup();
	REQUIRE( doc.ArrayLength() == 3 );
	REQUIRE( doc.ArrayGet( 0 ).ValueGet() == std::string( "array1" ) );
	REQUIRE( doc.ArrayGet( 1 ).ValueGet() == std::string( "array2" ) );
	REQUIRE( doc.ArrayGet( 2 ).ValueGet() == std::string( "array3" ) );
	REQUIRE( doc.GetUndoStackSize() == 4 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test Initialize from Array to Map (should clear all array elements)
	doc.Initialize( ae::DocumentValueType::Map );
	doc.EndUndoGroup();
	REQUIRE( doc.GetType() == ae::DocumentValueType::Map );
	REQUIRE( doc.MapLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 5 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Add map elements to test clearing
	doc.MapSet( "key1" ).ValueSet( "value1" );
	doc.MapSet( "key2" ).ValueSet( "value2" );
	doc.MapSet( "key3" ).ValueSet( "value3" );
	doc.EndUndoGroup();
	REQUIRE( doc.MapLength() == 3 );
	REQUIRE( doc.MapTryGet( "key1" )->ValueGet() == std::string( "value1" ) );
	REQUIRE( doc.MapTryGet( "key2" )->ValueGet() == std::string( "value2" ) );
	REQUIRE( doc.MapTryGet( "key3" )->ValueGet() == std::string( "value3" ) );
	REQUIRE( doc.GetUndoStackSize() == 6 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test Initialize from Map to Basic (should clear all map elements)
	doc.Initialize( ae::DocumentValueType::Value );
	doc.EndUndoGroup();
	REQUIRE( doc.GetType() == ae::DocumentValueType::Value );
	REQUIRE( doc.GetUndoStackSize() == 7 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test undo sequence - should restore each state in reverse order
	REQUIRE( doc.Undo() ); // Back to Map with 3 elements
	REQUIRE( doc.GetType() == ae::DocumentValueType::Map );
	REQUIRE( doc.MapLength() == 3 );
	REQUIRE( doc.MapTryGet( "key1" )->ValueGet() == std::string( "value1" ) );
	REQUIRE( doc.MapTryGet( "key2" )->ValueGet() == std::string( "value2" ) );
	REQUIRE( doc.MapTryGet( "key3" )->ValueGet() == std::string( "value3" ) );
	REQUIRE( doc.GetUndoStackSize() == 6 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	REQUIRE( doc.Undo() ); // Back to empty Map
	REQUIRE( doc.GetType() == ae::DocumentValueType::Map );
	REQUIRE( doc.MapLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 5 );
	REQUIRE( doc.GetRedoStackSize() == 2 );

	REQUIRE( doc.Undo() ); // Back to Array with 3 elements
	REQUIRE( doc.GetType() == ae::DocumentValueType::Array );
	REQUIRE( doc.ArrayLength() == 3 );
	REQUIRE( doc.ArrayGet( 0 ).ValueGet() == std::string( "array1" ) );
	REQUIRE( doc.ArrayGet( 1 ).ValueGet() == std::string( "array2" ) );
	REQUIRE( doc.ArrayGet( 2 ).ValueGet() == std::string( "array3" ) );
	REQUIRE( doc.GetUndoStackSize() == 4 );
	REQUIRE( doc.GetRedoStackSize() == 3 );

	REQUIRE( doc.Undo() ); // Back to empty Array
	REQUIRE( doc.GetType() == ae::DocumentValueType::Array );
	REQUIRE( doc.ArrayLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 4 );

	REQUIRE( doc.Undo() ); // Back to Basic with value
	REQUIRE( doc.GetType() == ae::DocumentValueType::Value );
	REQUIRE( doc.ValueGet() == std::string( "test_value" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 5 );

	REQUIRE( doc.Undo() ); // Back to empty Basic
	REQUIRE( doc.GetType() == ae::DocumentValueType::Value );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 6 );

	REQUIRE( doc.Undo() ); // Back to None
	REQUIRE( doc.GetType() == ae::DocumentValueType::None );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 7 );

	// Test redo sequence - should restore forward
	REQUIRE( doc.Redo() ); // Forward to Basic
	REQUIRE( doc.GetType() == ae::DocumentValueType::Value );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 6 );

	REQUIRE( doc.Redo() ); // Forward to Basic with value
	REQUIRE( doc.GetType() == ae::DocumentValueType::Value );
	REQUIRE( doc.ValueGet() == std::string( "test_value" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 5 );

	// Test Initialize with same type when data exists - should clear data
	doc.Initialize( ae::DocumentValueType::Value );
	doc.EndUndoGroup();
	REQUIRE( doc.GetType() == ae::DocumentValueType::Value );
	REQUIRE( doc.GetUndoStackSize() == 3 ); // Clearing the basic value + type change
	REQUIRE( doc.GetRedoStackSize() == 0 ); // Redo stack cleared

	// Undo should restore the basic value
	REQUIRE( doc.Undo() );
	REQUIRE( doc.GetType() == ae::DocumentValueType::Value );
	REQUIRE( doc.ValueGet() == std::string( "test_value" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 1 );
}

TEST_CASE( "DocumentUndo InitializeChaining", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );

	// Test method chaining with Initialize
	doc.Initialize( ae::DocumentValueType::Array ).ArrayAppend().ValueSet( "chained1" );
	doc.ArrayAppend().ValueSet( "chained2" );
	doc.EndUndoGroup();

	REQUIRE( doc.GetType() == ae::DocumentValueType::Array );
	REQUIRE( doc.ArrayLength() == 2 );
	REQUIRE( doc.ArrayGet( 0 ).ValueGet() == std::string( "chained1" ) );
	REQUIRE( doc.ArrayGet( 1 ).ValueGet() == std::string( "chained2" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test chaining with type change
	doc.Initialize( ae::DocumentValueType::Map ).MapSet( "chained_key" ).ValueSet( "chained_value" );
	doc.EndUndoGroup();

	REQUIRE( doc.GetType() == ae::DocumentValueType::Map );
	REQUIRE( doc.MapLength() == 1 );
	REQUIRE( doc.MapTryGet( "chained_key" )->ValueGet() == std::string( "chained_value" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should restore array
	REQUIRE( doc.Undo() );
	REQUIRE( doc.GetType() == ae::DocumentValueType::Array );
	REQUIRE( doc.ArrayLength() == 2 );
	REQUIRE( doc.ArrayGet( 0 ).ValueGet() == std::string( "chained1" ) );
	REQUIRE( doc.ArrayGet( 1 ).ValueGet() == std::string( "chained2" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 1 );
}

TEST_CASE( "DocumentUndo InitializeComplexClear", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );

	// Create complex nested array structure
	doc.ArrayInitialize( 3 );
	doc.ArrayAppend().ArrayInitialize( 2 );
	doc.ArrayGet( 0 ).ArrayAppend().ValueSet( "nested1" );
	doc.ArrayGet( 0 ).ArrayAppend().MapInitialize( 2 );
	doc.ArrayGet( 0 ).ArrayGet( 1 ).MapSet( "nested_key" ).ValueSet( "nested_value" );
	doc.ArrayAppend().ValueSet( "second" );
	doc.EndUndoGroup();

	REQUIRE( doc.IsArray() );
	REQUIRE( doc.ArrayLength() == 2 );
	REQUIRE( doc.ArrayGet( 0 ).IsArray() );
	REQUIRE( doc.ArrayGet( 0 ).ArrayLength() == 2 );
	REQUIRE( doc.ArrayGet( 0 ).ArrayGet( 0 ).ValueGet() == std::string( "nested1" ) );
	REQUIRE( doc.ArrayGet( 0 ).ArrayGet( 1 ).IsMap() );
	REQUIRE( doc.ArrayGet( 0 ).ArrayGet( 1 ).MapTryGet( "nested_key" )->ValueGet() == std::string( "nested_value" ) );
	REQUIRE( doc.ArrayGet( 1 ).ValueGet() == std::string( "second" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Initialize to Map should clear all nested structures
	doc.Initialize( ae::DocumentValueType::Map );
	doc.EndUndoGroup();
	REQUIRE( doc.IsMap() );
	REQUIRE( doc.MapLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should restore entire complex structure
	REQUIRE( doc.Undo() );
	REQUIRE( doc.IsArray() );
	REQUIRE( doc.ArrayLength() == 2 );
	REQUIRE( doc.ArrayGet( 0 ).IsArray() );
	REQUIRE( doc.ArrayGet( 0 ).ArrayLength() == 2 );
	REQUIRE( doc.ArrayGet( 0 ).ArrayGet( 0 ).ValueGet() == std::string( "nested1" ) );
	REQUIRE( doc.ArrayGet( 0 ).ArrayGet( 1 ).IsMap() );
	REQUIRE( doc.ArrayGet( 0 ).ArrayGet( 1 ).MapTryGet( "nested_key" )->ValueGet() == std::string( "nested_value" ) );
	REQUIRE( doc.ArrayGet( 1 ).ValueGet() == std::string( "second" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Redo should clear back to empty map
	REQUIRE( doc.Redo() );
	REQUIRE( doc.IsMap() );
	REQUIRE( doc.MapLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );
}

TEST_CASE( "DocumentUndo InitializeCoalescing", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );

	// Set initial state
	doc.ValueSet( "initial" );
	doc.EndUndoGroup();
	REQUIRE( doc.IsValue() );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Multiple Initialize calls in same group should coalesce SetType operations
	doc.Initialize( ae::DocumentValueType::Array );
	doc.Initialize( ae::DocumentValueType::Map );
	doc.Initialize( ae::DocumentValueType::Value );
	doc.Initialize( ae::DocumentValueType::Array );
	doc.EndUndoGroup(); // All SetType operations should coalesce into one

	REQUIRE( doc.IsArray() );
	REQUIRE( doc.ArrayLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 2 ); // Initial + coalesced group
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Single undo should revert to initial Basic state
	REQUIRE( doc.Undo() );
	REQUIRE( doc.IsValue() );
	REQUIRE( doc.ValueGet() == std::string( "initial" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Redo should restore final Array state
	REQUIRE( doc.Redo() );
	REQUIRE( doc.IsArray() );
	REQUIRE( doc.ArrayLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test coalescing with data operations in between
	doc.ArrayAppend().ValueSet( "element1" );
	doc.Initialize( ae::DocumentValueType::Map ); // Should clear array and change type
	doc.MapSet( "key1" ).ValueSet( "value1" );
	doc.Initialize( ae::DocumentValueType::Value ); // Should clear map and change type
	doc.ValueSet( "final_basic" );
	doc.EndUndoGroup();

	REQUIRE( doc.IsValue() );
	REQUIRE( doc.ValueGet() == std::string( "final_basic" ) );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should restore previous Array state
	REQUIRE( doc.Undo() );
	REQUIRE( doc.IsArray() );
	REQUIRE( doc.ArrayLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 1 );
}

TEST_CASE( "DocumentUndo ArrayInitialize", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Start with array and add some elements
	doc.ArrayInitialize( 4 );
	doc.ArrayAppend().ValueSet( "first" );
	doc.ArrayAppend().ValueSet( "second" );
	doc.ArrayAppend().ValueSet( "third" );
	doc.EndUndoGroup();

	REQUIRE( doc.IsArray() );
	REQUIRE( doc.ArrayLength() == 3 );
	REQUIRE( doc.ArrayGet( 0 ).ValueGet() == std::string( "first" ) );
	REQUIRE( doc.ArrayGet( 1 ).ValueGet() == std::string( "second" ) );
	REQUIRE( doc.ArrayGet( 2 ).ValueGet() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// ArrayInitialize should clear existing array contents
	doc.ArrayInitialize( 2 );
	doc.EndUndoGroup();

	REQUIRE( doc.IsArray() );
	REQUIRE( doc.ArrayLength() == 0 ); // Should be cleared
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Add new elements to verify it works after clearing
	doc.ArrayAppend().ValueSet( "new1" );
	doc.ArrayAppend().ValueSet( "new2" );
	doc.EndUndoGroup();

	REQUIRE( doc.ArrayLength() == 2 );
	REQUIRE( doc.ArrayGet( 0 ).ValueGet() == std::string( "new1" ) );
	REQUIRE( doc.ArrayGet( 1 ).ValueGet() == std::string( "new2" ) );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should restore empty array
	REQUIRE( doc.Undo() );
	REQUIRE( doc.IsArray() );
	REQUIRE( doc.ArrayLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Undo should restore original array with all elements
	REQUIRE( doc.Undo() );
	REQUIRE( doc.IsArray() );
	REQUIRE( doc.ArrayLength() == 3 );
	REQUIRE( doc.ArrayGet( 0 ).ValueGet() == std::string( "first" ) );
	REQUIRE( doc.ArrayGet( 1 ).ValueGet() == std::string( "second" ) );
	REQUIRE( doc.ArrayGet( 2 ).ValueGet() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 2 );

	// Redo should restore cleared array
	REQUIRE( doc.Redo() );
	REQUIRE( doc.IsArray() );
	REQUIRE( doc.ArrayLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Redo should restore new elements
	REQUIRE( doc.Redo() );
	REQUIRE( doc.IsArray() );
	REQUIRE( doc.ArrayLength() == 2 );
	REQUIRE( doc.ArrayGet( 0 ).ValueGet() == std::string( "new1" ) );
	REQUIRE( doc.ArrayGet( 1 ).ValueGet() == std::string( "new2" ) );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test ArrayInitialize on different type (basic value)
	doc.ValueSet( "convert_to_array" );
	doc.EndUndoGroup();
	REQUIRE( doc.IsValue() );
	REQUIRE( doc.ValueGet() == std::string( "convert_to_array" ) );
	REQUIRE( doc.GetUndoStackSize() == 4 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// ArrayInitialize should convert from basic to array
	doc.ArrayInitialize( 1 );
	doc.EndUndoGroup();
	REQUIRE( doc.IsArray() );
	REQUIRE( doc.ArrayLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 5 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should restore basic value
	REQUIRE( doc.Undo() );
	REQUIRE( doc.IsValue() );
	REQUIRE( doc.ValueGet() == std::string( "convert_to_array" ) );
	REQUIRE( doc.GetUndoStackSize() == 4 );
	REQUIRE( doc.GetRedoStackSize() == 1 );
}

TEST_CASE( "DocumentUndo MapInitialize", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Start with map and add some key-value pairs
	doc.MapInitialize( 4 );
	doc.MapSet( "key1" ).ValueSet( "value1" );
	doc.MapSet( "key2" ).ValueSet( "value2" );
	doc.MapSet( "key3" ).ValueSet( "value3" );
	doc.EndUndoGroup();

	REQUIRE( doc.IsMap() );
	REQUIRE( doc.MapLength() == 3 );
	REQUIRE( doc.MapTryGet( "key1" )->ValueGet() == std::string( "value1" ) );
	REQUIRE( doc.MapTryGet( "key2" )->ValueGet() == std::string( "value2" ) );
	REQUIRE( doc.MapTryGet( "key3" )->ValueGet() == std::string( "value3" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// MapInitialize should clear existing map contents
	doc.MapInitialize( 2 );
	doc.EndUndoGroup();

	REQUIRE( doc.IsMap() );
	REQUIRE( doc.MapLength() == 0 ); // Should be cleared
	REQUIRE( doc.MapTryGet( "key1" ) == nullptr );
	REQUIRE( doc.MapTryGet( "key2" ) == nullptr );
	REQUIRE( doc.MapTryGet( "key3" ) == nullptr );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Add new key-value pairs to verify it works after clearing
	doc.MapSet( "newkey1" ).ValueSet( "newvalue1" );
	doc.MapSet( "newkey2" ).ValueSet( "newvalue2" );
	doc.EndUndoGroup();

	REQUIRE( doc.MapLength() == 2 );
	REQUIRE( doc.MapTryGet( "newkey1" )->ValueGet() == std::string( "newvalue1" ) );
	REQUIRE( doc.MapTryGet( "newkey2" )->ValueGet() == std::string( "newvalue2" ) );
	REQUIRE( doc.MapTryGet( "key1" ) == nullptr ); // Old keys should still be gone
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should restore empty map
	REQUIRE( doc.Undo() );
	REQUIRE( doc.IsMap() );
	REQUIRE( doc.MapLength() == 0 );
	REQUIRE( doc.MapTryGet( "newkey1" ) == nullptr );
	REQUIRE( doc.MapTryGet( "newkey2" ) == nullptr );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Undo should restore original map with all key-value pairs
	REQUIRE( doc.Undo() );
	REQUIRE( doc.IsMap() );
	REQUIRE( doc.MapLength() == 3 );
	REQUIRE( doc.MapTryGet( "key1" )->ValueGet() == std::string( "value1" ) );
	REQUIRE( doc.MapTryGet( "key2" )->ValueGet() == std::string( "value2" ) );
	REQUIRE( doc.MapTryGet( "key3" )->ValueGet() == std::string( "value3" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 2 );

	// Redo should restore cleared map
	REQUIRE( doc.Redo() );
	REQUIRE( doc.IsMap() );
	REQUIRE( doc.MapLength() == 0 );
	REQUIRE( doc.MapTryGet( "key1" ) == nullptr );
	REQUIRE( doc.MapTryGet( "key2" ) == nullptr );
	REQUIRE( doc.MapTryGet( "key3" ) == nullptr );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Redo should restore new key-value pairs
	REQUIRE( doc.Redo() );
	REQUIRE( doc.IsMap() );
	REQUIRE( doc.MapLength() == 2 );
	REQUIRE( doc.MapTryGet( "newkey1" )->ValueGet() == std::string( "newvalue1" ) );
	REQUIRE( doc.MapTryGet( "newkey2" )->ValueGet() == std::string( "newvalue2" ) );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test MapInitialize on different type (basic value)
	doc.ValueSet( "convert_to_map" );
	doc.EndUndoGroup();
	REQUIRE( doc.IsValue() );
	REQUIRE( doc.ValueGet() == std::string( "convert_to_map" ) );
	REQUIRE( doc.GetUndoStackSize() == 4 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// MapInitialize should convert from basic to map
	doc.MapInitialize( 1 );
	doc.EndUndoGroup();
	REQUIRE( doc.IsMap() );
	REQUIRE( doc.MapLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 5 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should restore basic value
	REQUIRE( doc.Undo() );
	REQUIRE( doc.IsValue() );
	REQUIRE( doc.ValueGet() == std::string( "convert_to_map" ) );
	REQUIRE( doc.GetUndoStackSize() == 4 );
	REQUIRE( doc.GetRedoStackSize() == 1 );
}

TEST_CASE( "DocumentUndo MapOrderStability", "[ae::Document][undo][map]" )
{
	ae::Document doc( "test" );
	doc.MapInitialize( 10 );

	// Add elements in specific order
	doc.MapSet( "alpha" ).ValueSet( "first" );
	doc.MapSet( "beta" ).ValueSet( "second" );
	doc.MapSet( "gamma" ).ValueSet( "third" );
	doc.MapSet( "delta" ).ValueSet( "fourth" );
	doc.MapSet( "epsilon" ).ValueSet( "fifth" );
	doc.EndUndoGroup();

	// Verify initial order through iteration
	REQUIRE( doc.MapLength() == 5 );
	REQUIRE( doc.MapGetKey( 0 ) == std::string( "alpha" ) );
	REQUIRE( doc.MapGetKey( 1 ) == std::string( "beta" ) );
	REQUIRE( doc.MapGetKey( 2 ) == std::string( "gamma" ) );
	REQUIRE( doc.MapGetKey( 3 ) == std::string( "delta" ) );
	REQUIRE( doc.MapGetKey( 4 ) == std::string( "epsilon" ) );

	// Modify existing values (should not affect order)
	doc.MapSet( "gamma" ).ValueSet( "modified_third" );
	doc.MapSet( "alpha" ).ValueSet( "modified_first" );
	doc.EndUndoGroup();

	// Verify order is preserved after modifications
	REQUIRE( doc.MapLength() == 5 );
	REQUIRE( doc.MapGetKey( 0 ) == std::string( "alpha" ) );
	REQUIRE( doc.MapGetKey( 1 ) == std::string( "beta" ) );
	REQUIRE( doc.MapGetKey( 2 ) == std::string( "gamma" ) );
	REQUIRE( doc.MapGetKey( 3 ) == std::string( "delta" ) );
	REQUIRE( doc.MapGetKey( 4 ) == std::string( "epsilon" ) );
	REQUIRE( doc.MapGetValue( 0 ).ValueGet() == std::string( "modified_first" ) );
	REQUIRE( doc.MapGetValue( 2 ).ValueGet() == std::string( "modified_third" ) );

	// Add new elements (should append to end)
	doc.MapSet( "zeta" ).ValueSet( "sixth" );
	doc.MapSet( "eta" ).ValueSet( "seventh" );
	doc.EndUndoGroup();

	// Verify new elements maintain insertion order
	REQUIRE( doc.MapLength() == 7 );
	REQUIRE( doc.MapGetKey( 5 ) == std::string( "zeta" ) );
	REQUIRE( doc.MapGetKey( 6 ) == std::string( "eta" ) );

	// Remove middle element
	doc.MapRemove( "gamma" );
	doc.EndUndoGroup();

	// Verify order is preserved after removal (remaining elements shift)
	REQUIRE( doc.MapLength() == 6 );
	REQUIRE( doc.MapGetKey( 0 ) == std::string( "alpha" ) );
	REQUIRE( doc.MapGetKey( 1 ) == std::string( "beta" ) );
	REQUIRE( doc.MapGetKey( 2 ) == std::string( "delta" ) ); // gamma removed, delta shifts up
	REQUIRE( doc.MapGetKey( 3 ) == std::string( "epsilon" ) );
	REQUIRE( doc.MapGetKey( 4 ) == std::string( "zeta" ) );
	REQUIRE( doc.MapGetKey( 5 ) == std::string( "eta" ) );

	// Undo removal - element should return to same position
	REQUIRE( doc.Undo() );
	REQUIRE( doc.MapLength() == 7 );
	REQUIRE( doc.MapGetKey( 0 ) == std::string( "alpha" ) );
	REQUIRE( doc.MapGetKey( 1 ) == std::string( "beta" ) );
	REQUIRE( doc.MapGetKey( 2 ) == std::string( "gamma" ) ); // gamma restored to original position
	REQUIRE( doc.MapGetKey( 3 ) == std::string( "delta" ) );
	REQUIRE( doc.MapGetKey( 4 ) == std::string( "epsilon" ) );
	REQUIRE( doc.MapGetKey( 5 ) == std::string( "zeta" ) );
	REQUIRE( doc.MapGetKey( 6 ) == std::string( "eta" ) );
	REQUIRE( doc.MapGetValue( 2 ).ValueGet() == std::string( "modified_third" ) );

	// Undo additions
	REQUIRE( doc.Undo() );
	REQUIRE( doc.MapLength() == 5 );
	REQUIRE( doc.MapGetKey( 4 ) == std::string( "epsilon" ) ); // zeta and eta should be gone
	REQUIRE( doc.MapTryGet( "zeta" ) == nullptr );
	REQUIRE( doc.MapTryGet( "eta" ) == nullptr );

	// Undo modifications - values should revert but order preserved
	REQUIRE( doc.Undo() );
	REQUIRE( doc.MapLength() == 5 );
	REQUIRE( doc.MapGetKey( 0 ) == std::string( "alpha" ) );
	REQUIRE( doc.MapGetKey( 2 ) == std::string( "gamma" ) );
	REQUIRE( doc.MapGetValue( 0 ).ValueGet() == std::string( "first" ) ); // alpha reverted
	REQUIRE( doc.MapGetValue( 2 ).ValueGet() == std::string( "third" ) ); // gamma reverted

	// Test complete redo cycle maintains order
	REQUIRE( doc.Redo() ); // redo modifications
	REQUIRE( doc.Redo() ); // redo additions
	REQUIRE( doc.Redo() ); // redo removal

	// Final state should match removal state
	REQUIRE( doc.MapLength() == 6 );
	REQUIRE( doc.MapGetKey( 0 ) == std::string( "alpha" ) );
	REQUIRE( doc.MapGetKey( 1 ) == std::string( "beta" ) );
	REQUIRE( doc.MapGetKey( 2 ) == std::string( "delta" ) ); // gamma removed
	REQUIRE( doc.MapGetKey( 3 ) == std::string( "epsilon" ) );
	REQUIRE( doc.MapGetKey( 4 ) == std::string( "zeta" ) );
	REQUIRE( doc.MapGetKey( 5 ) == std::string( "eta" ) );
	REQUIRE( doc.MapTryGet( "gamma" ) == nullptr );

	// Test that re-adding a removed key appends to end (new insertion)
	doc.MapSet( "gamma" ).ValueSet( "re_added" );
	doc.EndUndoGroup();

	REQUIRE( doc.MapLength() == 7 );
	REQUIRE( doc.MapGetKey( 6 ) == std::string( "gamma" ) ); // Should be at end, not original position
	REQUIRE( doc.MapGetValue( 6 ).ValueGet() == std::string( "re_added" ) );

	// Verify all other elements maintain their positions
	REQUIRE( doc.MapGetKey( 0 ) == std::string( "alpha" ) );
	REQUIRE( doc.MapGetKey( 1 ) == std::string( "beta" ) );
	REQUIRE( doc.MapGetKey( 2 ) == std::string( "delta" ) );
	REQUIRE( doc.MapGetKey( 3 ) == std::string( "epsilon" ) );
	REQUIRE( doc.MapGetKey( 4 ) == std::string( "zeta" ) );
	REQUIRE( doc.MapGetKey( 5 ) == std::string( "eta" ) );
}

TEST_CASE( "DocumentUndo MapOrderPerfectRestoration", "[ae::Document][undo][map][order]" )
{
	ae::Document doc( "test" );
	doc.MapInitialize( 10 );

	// Build a complex map with specific ordering
	doc.MapSet( "z_last" ).ValueSet( "should_be_first" );
	doc.MapSet( "a_first" ).ValueSet( "should_be_second" );
	doc.MapSet( "m_middle" ).ValueSet( "should_be_third" );
	doc.EndUndoGroup();

	// Capture the original order
	std::vector< std::string > originalOrder;
	for( uint32_t i = 0; i < doc.MapLength(); i++ )
	{
		originalOrder.push_back( doc.MapGetKey( i ) );
	}

	REQUIRE( originalOrder.size() == 3 );
	REQUIRE( originalOrder[ 0 ] == "z_last" );
	REQUIRE( originalOrder[ 1 ] == "a_first" );
	REQUIRE( originalOrder[ 2 ] == "m_middle" );

	// Remove the middle element
	REQUIRE( doc.MapRemove( "a_first" ) );
	doc.EndUndoGroup();

	REQUIRE( doc.MapLength() == 2 );
	REQUIRE( doc.MapGetKey( 0 ) == std::string( "z_last" ) );
	REQUIRE( doc.MapGetKey( 1 ) == std::string( "m_middle" ) );
	REQUIRE( doc.MapTryGet( "a_first" ) == nullptr );

	// Add some more elements
	doc.MapSet( "new_element" ).ValueSet( "new_value" );
	doc.MapSet( "another_new" ).ValueSet( "another_value" );
	doc.EndUndoGroup();

	REQUIRE( doc.MapLength() == 4 );
	REQUIRE( doc.MapGetKey( 2 ) == std::string( "new_element" ) );
	REQUIRE( doc.MapGetKey( 3 ) == std::string( "another_new" ) );

	// Undo the additions
	REQUIRE( doc.Undo() );
	REQUIRE( doc.MapLength() == 2 );
	REQUIRE( doc.MapGetKey( 0 ) == std::string( "z_last" ) );
	REQUIRE( doc.MapGetKey( 1 ) == std::string( "m_middle" ) );
	REQUIRE( doc.MapTryGet( "new_element" ) == nullptr );
	REQUIRE( doc.MapTryGet( "another_new" ) == nullptr );

	// Undo the removal - should restore exact original position
	REQUIRE( doc.Undo() );
	REQUIRE( doc.MapLength() == 3 );

	// Verify the exact original order is restored
	for( uint32_t i = 0; i < doc.MapLength(); i++ )
	{
		REQUIRE( doc.MapGetKey( i ) == originalOrder[ i ] );
	}

	// Verify values are correct too
	REQUIRE( doc.MapSet( "z_last" ).ValueGet() == std::string( "should_be_first" ) );
	REQUIRE( doc.MapSet( "a_first" ).ValueGet() == std::string( "should_be_second" ) );
	REQUIRE( doc.MapSet( "m_middle" ).ValueGet() == std::string( "should_be_third" ) );

	// Test that redo preserves order through complex operations
	REQUIRE( doc.Redo() ); // redo removal
	REQUIRE( doc.Redo() ); // redo additions

	REQUIRE( doc.MapLength() == 4 );
	REQUIRE( doc.MapGetKey( 0 ) == std::string( "z_last" ) );
	REQUIRE( doc.MapGetKey( 1 ) == std::string( "m_middle" ) ); // a_first removed
	REQUIRE( doc.MapGetKey( 2 ) == std::string( "new_element" ) );
	REQUIRE( doc.MapGetKey( 3 ) == std::string( "another_new" ) );

	// Now test removing and re-adding the same key
	REQUIRE( doc.MapRemove( "z_last" ) );
	doc.EndUndoGroup();

	// Re-add the same key (should append to end, not go back to original position)
	doc.MapSet( "z_last" ).ValueSet( "reinserted_value" );
	doc.EndUndoGroup();

	REQUIRE( doc.MapLength() == 4 );
	REQUIRE( doc.MapGetKey( 0 ) == std::string( "m_middle" ) );
	REQUIRE( doc.MapGetKey( 1 ) == std::string( "new_element" ) );
	REQUIRE( doc.MapGetKey( 2 ) == std::string( "another_new" ) );
	REQUIRE( doc.MapGetKey( 3 ) == std::string( "z_last" ) ); // Should be at end now
	REQUIRE( doc.MapSet( "z_last" ).ValueGet() == std::string( "reinserted_value" ) );
}
