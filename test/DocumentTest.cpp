//------------------------------------------------------------------------------
// Document.cpp
// Copyright (c) John Hughes on 6/15/24. All rights reserved.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aether.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

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

	// Set initial string
	doc.StringSet( "initial" );
	doc.EndUndoGroup();
	REQUIRE( doc.StringGet() == std::string( "initial" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Change string
	doc.StringSet( "changed" );
	doc.EndUndoGroup();
	REQUIRE( doc.StringGet() == std::string( "changed" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should restore previous string
	REQUIRE( doc.Undo() );
	REQUIRE( doc.StringGet() == std::string( "initial" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Redo should restore changed string
	REQUIRE( doc.Redo() );
	REQUIRE( doc.StringGet() == std::string( "changed" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Multiple undos
	REQUIRE( doc.Undo() );
	REQUIRE( doc.StringGet() == std::string( "initial" ) );
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

	// Start as basic string
	doc.StringSet( "test" );
	doc.EndUndoGroup();
	REQUIRE( doc.IsString() );
	REQUIRE( doc.StringGet() == std::string( "test" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Change to array
	doc.ArrayInitialize( 4 );
	doc.EndUndoGroup();
	REQUIRE( doc.IsArray() );
	REQUIRE( doc.ArrayLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should restore type and string
	REQUIRE( doc.Undo() );
	REQUIRE( doc.IsString() );
	REQUIRE( doc.StringGet() == std::string( "test" ) );
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
	auto& elem0 = doc.ArrayAppend();
	elem0.StringSet( "first" );
	auto& elem1 = doc.ArrayAppend();
	elem1.StringSet( "second" );
	auto& elem2 = doc.ArrayAppend();
	elem2.StringSet( "third" );
	doc.EndUndoGroup();

	REQUIRE( doc.ArrayLength() == 3 );
	REQUIRE( doc.ArrayGet( 0 ).StringGet() == std::string( "first" ) );
	REQUIRE( doc.ArrayGet( 1 ).StringGet() == std::string( "second" ) );
	REQUIRE( doc.ArrayGet( 2 ).StringGet() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Insert in middle
	auto& elemInserted = doc.ArrayInsert( 1 );
	elemInserted.StringSet( "inserted" );
	doc.EndUndoGroup();
	REQUIRE( doc.ArrayLength() == 4 );
	REQUIRE( doc.ArrayGet( 0 ).StringGet() == std::string( "first" ) );
	REQUIRE( doc.ArrayGet( 1 ).StringGet() == std::string( "inserted" ) );
	REQUIRE( doc.ArrayGet( 2 ).StringGet() == std::string( "second" ) );
	REQUIRE( doc.ArrayGet( 3 ).StringGet() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );
	// Undo insert
	REQUIRE( doc.Undo() );
	REQUIRE( doc.ArrayLength() == 3 );
	REQUIRE( doc.ArrayGet( 0 ).StringGet() == std::string( "first" ) );
	REQUIRE( doc.ArrayGet( 1 ).StringGet() == std::string( "second" ) );
	REQUIRE( doc.ArrayGet( 2 ).StringGet() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Remove element
	doc.ArrayRemove( 1 );
	doc.EndUndoGroup();

	REQUIRE( doc.ArrayLength() == 2 );
	REQUIRE( doc.ArrayGet( 0 ).StringGet() == std::string( "first" ) );
	REQUIRE( doc.ArrayGet( 1 ).StringGet() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 3 ); // Redo stack was cleared, new operation added
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo remove should restore element at correct position
	REQUIRE( doc.Undo() );
	REQUIRE( doc.ArrayLength() == 3 );
	REQUIRE( doc.ArrayGet( 0 ).StringGet() == std::string( "first" ) );
	REQUIRE( doc.ArrayGet( 1 ).StringGet() == std::string( "second" ) );
	REQUIRE( doc.ArrayGet( 2 ).StringGet() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 1 );
}

TEST_CASE( "DocumentUndo ObjectOperations", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	doc.ObjectInitialize( 4 );
	doc.EndUndoGroup();
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Add map strings
	auto& val1 = doc.ObjectSet( "key1" );
	val1.StringSet( "string1" );
	auto& val2 = doc.ObjectSet( "key2" );
	val2.StringSet( "string2" );
	doc.EndUndoGroup();

	REQUIRE( doc.ObjectLength() == 2 );
	REQUIRE( doc.ObjectTryGet( "key1" )->StringGet() == std::string( "string1" ) );
	REQUIRE( doc.ObjectTryGet( "key2" )->StringGet() == std::string( "string2" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Remove a key
	REQUIRE( doc.ObjectRemove( "key1" ) ); // Should return true for successful removal
	doc.EndUndoGroup();

	REQUIRE( doc.ObjectLength() == 1 );
	REQUIRE( doc.ObjectTryGet( "key1" ) == nullptr );
	REQUIRE( doc.ObjectTryGet( "key2" )->StringGet() == std::string( "string2" ) );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo remove should restore key and value
	REQUIRE( doc.Undo() );
	REQUIRE( doc.ObjectLength() == 2 );
	REQUIRE( doc.ObjectGetValue( 0 ).StringGet() == std::string( "string1" ) );
	REQUIRE( doc.ObjectGetValue( 1 ).StringGet() == std::string( "string2" ) );
	REQUIRE( doc.ObjectTryGet( "key1" )->StringGet() == std::string( "string1" ) );
	REQUIRE( doc.ObjectTryGet( "key2" )->StringGet() == std::string( "string2" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Redo remove
	REQUIRE( doc.Redo() );
	REQUIRE( doc.ObjectLength() == 1 );
	REQUIRE( doc.ObjectTryGet( "key1" ) == nullptr );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );
}

TEST_CASE( "DocumentUndo UndoGroups", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	doc.ArrayInitialize( 4 );

	// Multiple operations in one group
	doc.ArrayAppend().StringSet( "first" );
	doc.ArrayAppend().StringSet( "second" );
	doc.ArrayAppend().StringSet( "third" );
	doc.EndUndoGroup(); // All three operations in one group

	REQUIRE( doc.ArrayLength() == 3 );
	REQUIRE( doc.GetUndoStackSize() == 1 ); // Single group with multiple operations
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Single undo should reverse entire group
	REQUIRE( doc.Undo() );
	REQUIRE( doc.GetType() == ae::DocumentValueType::Null ); // Back to empty document
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Single redo should restore entire group
	REQUIRE( doc.Redo() );
	REQUIRE( doc.ArrayLength() == 3 );
	REQUIRE( doc.ArrayGet( 0 ).StringGet() == std::string( "first" ) );
	REQUIRE( doc.ArrayGet( 1 ).StringGet() == std::string( "second" ) );
	REQUIRE( doc.ArrayGet( 2 ).StringGet() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );
}

TEST_CASE( "DocumentUndo ClearUndo", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	doc.StringSet( "test" );
	doc.EndUndoGroup();
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	doc.StringSet( "changed" );
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

	// Multiple StringSet calls in same group should coalesce
	doc.StringSet( "first" );
	REQUIRE( doc.StringGet() == std::string( "first" ) );
	doc.StringSet( "second" );
	REQUIRE( doc.StringGet() == std::string( "second" ) );
	doc.StringSet( "third" );
	REQUIRE( doc.StringGet() == std::string( "third" ) );
	doc.EndUndoGroup();

	// Should only have one undo operation despite 3 StringSet calls
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Single undo should revert to initial state (empty)
	REQUIRE( doc.Undo() );
	REQUIRE( doc.GetType() == ae::DocumentValueType::Null );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Single redo should restore final string
	REQUIRE( doc.Redo() );
	REQUIRE( doc.StringGet() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );
}

TEST_CASE( "DocumentUndo ValueSeparateGroups", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// First string with separate undo group
	doc.StringSet( "first" );
	doc.EndUndoGroup();
	REQUIRE( doc.StringGet() == std::string( "first" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Second string with separate undo group (no coalescing)
	doc.StringSet( "second" );
	doc.EndUndoGroup();
	REQUIRE( doc.StringGet() == std::string( "second" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Third string with separate undo group (no coalescing)
	doc.StringSet( "third" );
	doc.EndUndoGroup();
	REQUIRE( doc.StringGet() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Multiple undos should step through each string
	REQUIRE( doc.Undo() );
	REQUIRE( doc.StringGet() == std::string( "second" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	REQUIRE( doc.Undo() );
	REQUIRE( doc.StringGet() == std::string( "first" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 2 );

	REQUIRE( doc.Undo() );
	REQUIRE( doc.GetType() == ae::DocumentValueType::Null );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 3 );
}

TEST_CASE( "DocumentUndo ComplexNesting", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	doc.ArrayInitialize( 4 );

	// Create nested structure
	auto& mapElem = doc.ArrayAppend();
	mapElem.ObjectInitialize( 2 );
	auto& nestedValue = doc.ArrayGet( 0 ).ObjectSet( "nested" );
	nestedValue.StringSet( "deep" );
	doc.EndUndoGroup();
	REQUIRE( doc.ArrayLength() == 1 );
	REQUIRE( doc.ArrayGet( 0 ).IsObject() );
	REQUIRE( doc.ArrayGet( 0 ).ObjectSet( "nested" ).StringGet() == std::string( "deep" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Modify nested string
	doc.ArrayGet( 0 ).ObjectSet( "nested" ).StringSet( "modified" );
	doc.EndUndoGroup();
	REQUIRE( doc.ArrayGet( 0 ).ObjectSet( "nested" ).StringGet() == std::string( "modified" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );
	REQUIRE( doc.Undo() ); // Undo should restore deep nested string
	REQUIRE( doc.ArrayGet( 0 ).ObjectSet( "nested" ).StringGet() == std::string( "deep" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Remove entire nested structure
	doc.ArrayRemove( 0 );
	doc.EndUndoGroup();
	REQUIRE( doc.ArrayLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 2 ); // Redo stack was cleared
	REQUIRE( doc.GetRedoStackSize() == 0 );
	REQUIRE( doc.Undo() ); // Undo should restore entire nested structure
	REQUIRE( doc.ArrayLength() == 1 );
	REQUIRE( doc.ArrayGet( 0 ).IsObject() );
	REQUIRE( doc.ArrayGet( 0 ).ObjectSet( "nested" ).StringGet() == std::string( "deep" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 1 );
}

TEST_CASE( "DocumentUndo MultiElementArrayWithDescendants", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	doc.ArrayInitialize( 8 );

	// Create array with multiple elements, each with deeply nested descendants
	// Tree structure:
	//   Array[0] (elem0) -> Map["nested"] (elem0_nested) -> Array[0] (elem0_deep) -> String
	//   Array[1] (elem1) -> Map["nested"] (elem1_nested) -> Map["value"] (elem1_deep) -> Number
	//   Array[2] (elem2) -> Array[0] (elem2_nested) -> Map["flag"] (elem2_deep) -> Bool

	// Element 0: Map -> Array -> String
	auto& elem0 = doc.ArrayAppend();
	elem0.ObjectInitialize( 2 );
	auto& elem0_nested = elem0.ObjectSet( "nested" );
	elem0_nested.ArrayInitialize( 2 );
	auto& elem0_deep = elem0_nested.ArrayAppend();
	elem0_deep.StringSet( "deep0" );

	// Element 1: Map -> Map -> Number
	auto& elem1 = doc.ArrayAppend();
	elem1.ObjectInitialize( 2 );
	auto& elem1_nested = elem1.ObjectSet( "nested" );
	elem1_nested.ObjectInitialize( 2 );
	auto& elem1_deep = elem1_nested.ObjectSet( "value" );
	elem1_deep.NumberSet( 42.0 );

	// Element 2: Array -> Map -> Bool
	auto& elem2 = doc.ArrayAppend();
	elem2.ArrayInitialize( 2 );
	auto& elem2_nested = elem2.ArrayAppend();
	elem2_nested.ObjectInitialize( 2 );
	auto& elem2_deep = elem2_nested.ObjectSet( "flag" );
	elem2_deep.BoolSet( true );

	doc.EndUndoGroup();
	REQUIRE( doc.ArrayLength() == 3 );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Remove all elements at once
	doc.ArrayRemove( 0 );
	doc.ArrayRemove( 0 );
	doc.ArrayRemove( 0 );
	doc.EndUndoGroup();
	REQUIRE( doc.ArrayLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should restore all elements and ALL their descendants
	REQUIRE( doc.Undo() );
	REQUIRE( doc.ArrayLength() == 3 );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Verify Create callbacks for all descendants (not just children)

	// Verify each element and its descendants

	// Verify restored structure
	REQUIRE( doc.ArrayGet( 0 ).ObjectSet( "nested" ).ArrayGet( 0 ).StringGet() == std::string( "deep0" ) );
	REQUIRE( doc.ArrayGet( 1 ).ObjectSet( "nested" ).ObjectSet( "value" ).NumberGet< double >() == 42.0 );
	REQUIRE( doc.ArrayGet( 2 ).ArrayGet( 0 ).ObjectSet( "flag" ).BoolGet() == true );

	// Redo should destroy all elements and ALL their descendants
	REQUIRE( doc.Redo() );
	REQUIRE( doc.ArrayLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Verify Destroy callbacks for all descendants

}

TEST_CASE( "DocumentUndo MultiElementMapWithDescendants", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	doc.ObjectInitialize( 8 );

	// Create map with multiple entries, each with deeply nested descendants
	// Tree structure:
	//   Map["a"] (entry_a) -> Array[0] (entry_a_nested) -> Map["text"] (entry_a_deep) -> String
	//   Map["b"] (entry_b) -> Map["list"] (entry_b_nested) -> Array[0] (entry_b_deep) -> Number
	//   Map["c"] (entry_c) -> Map["level2"] (entry_c_nested) -> Map["level3"] (entry_c_level3) -> Map["flag"] (entry_c_deep) -> Bool

	// Entry "a": Array -> Map -> String
	auto& entry_a = doc.ObjectSet( "a" );
	entry_a.ArrayInitialize( 2 );
	auto& entry_a_nested = entry_a.ArrayAppend();
	entry_a_nested.ObjectInitialize( 2 );
	auto& entry_a_deep = entry_a_nested.ObjectSet( "text" );
	entry_a_deep.StringSet( "deepA" );

	// Entry "b": Map -> Array -> Number
	auto& entry_b = doc.ObjectSet( "b" );
	entry_b.ObjectInitialize( 2 );
	auto& entry_b_nested = entry_b.ObjectSet( "list" );
	entry_b_nested.ArrayInitialize( 2 );
	auto& entry_b_deep = entry_b_nested.ArrayAppend();
	entry_b_deep.NumberSet( 99.5 );

	// Entry "c": Map -> Map -> Map -> Bool
	auto& entry_c = doc.ObjectSet( "c" );
	entry_c.ObjectInitialize( 2 );
	auto& entry_c_nested = entry_c.ObjectSet( "level2" );
	entry_c_nested.ObjectInitialize( 2 );
	auto& entry_c_level3 = entry_c_nested.ObjectSet( "level3" );
	entry_c_level3.ObjectInitialize( 2 );
	auto& entry_c_deep = entry_c_level3.ObjectSet( "flag" );
	entry_c_deep.BoolSet( false );

	doc.EndUndoGroup();
	REQUIRE( doc.ObjectLength() == 3 );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Remove all map entries
	doc.ObjectRemove( "a" );
	doc.ObjectRemove( "b" );
	doc.ObjectRemove( "c" );
	doc.EndUndoGroup();
	REQUIRE( doc.ObjectLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should restore all entries and ALL their descendants
	REQUIRE( doc.Undo() );
	REQUIRE( doc.ObjectLength() == 3 );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Verify Create callbacks for all descendants (not just children)

	// Verify each entry and its descendants

	// Verify restored structure
	REQUIRE( doc.ObjectSet( "a" ).ArrayGet( 0 ).ObjectSet( "text" ).StringGet() == std::string( "deepA" ) );
	REQUIRE( doc.ObjectSet( "b" ).ObjectSet( "list" ).ArrayGet( 0 ).NumberGet< double >() == 99.5 );
	REQUIRE( doc.ObjectSet( "c" ).ObjectSet( "level2" ).ObjectSet( "level3" ).ObjectSet( "flag" ).BoolGet() == false );

	// Redo should destroy all entries and ALL their descendants
	REQUIRE( doc.Redo() );
	REQUIRE( doc.ObjectLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Verify Destroy callbacks for all descendants

}

TEST_CASE( "DocumentUndo MixedArrayMapDescendants", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	doc.ArrayInitialize( 4 );

	// Create a complex structure with very deep nesting
	// Tree structure:
	//   Array[0] (level1) -> Map["key1"] (level2) -> Array[0] (level3) -> Map["key2"] (level4) -> Array[0] (level5) -> String
	//   Array[1] (level1b) -> Map["keyB"] (level2b) -> Array[0] (level3b) -> Number

	// Array -> Map -> Array -> Map -> Array -> String
	auto& level1 = doc.ArrayAppend();
	level1.ObjectInitialize( 2 );
	auto& level2 = level1.ObjectSet( "key1" );
	level2.ArrayInitialize( 2 );
	auto& level3 = level2.ArrayAppend();
	level3.ObjectInitialize( 2 );
	auto& level4 = level3.ObjectSet( "key2" );
	level4.ArrayInitialize( 2 );
	auto& level5 = level4.ArrayAppend();
	level5.StringSet( "veryDeep" );

	// Add another top-level element with similar structure
	auto& level1b = doc.ArrayAppend();
	level1b.ObjectInitialize( 2 );
	auto& level2b = level1b.ObjectSet( "keyB" );
	level2b.ArrayInitialize( 2 );
	auto& level3b = level2b.ArrayAppend();
	level3b.NumberSet( 123.0 );

	doc.EndUndoGroup();
	REQUIRE( doc.ArrayLength() == 2 );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Clear the entire array
	doc.ArrayRemove( 0 );
	doc.ArrayRemove( 0 );
	doc.EndUndoGroup();
	REQUIRE( doc.ArrayLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should restore entire hierarchy with all descendants
	REQUIRE( doc.Undo() );
	REQUIRE( doc.ArrayLength() == 2 );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Verify restored data
	REQUIRE( doc.ArrayGet( 0 ).ObjectSet( "key1" ).ArrayGet( 0 ).ObjectSet( "key2" ).ArrayGet( 0 ).StringGet() == std::string( "veryDeep" ) );
	REQUIRE( doc.ArrayGet( 1 ).ObjectSet( "keyB" ).ArrayGet( 0 ).NumberGet< double >() == 123.0 );

	// Redo should destroy all descendants
	REQUIRE( doc.Redo() );
	REQUIRE( doc.ArrayLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo again to verify callbacks work on second undo
	REQUIRE( doc.Undo() );
	REQUIRE( doc.ArrayLength() == 2 );
}

TEST_CASE( "DocumentUndo MapIteration", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	doc.ObjectInitialize( 4 );
	doc.EndUndoGroup();
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Add multiple map strings with different keys
	doc.ObjectSet( "apple" ).StringSet( "fruit1" );
	doc.ObjectSet( "banana" ).StringSet( "fruit2" );
	doc.ObjectSet( "cherry" ).StringSet( "fruit3" );
	doc.EndUndoGroup();

	REQUIRE( doc.ObjectLength() == 3 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test ObjectGetKey and MapGetValue iteration
	bool foundApple = false, foundBanana = false, foundCherry = false;
	for( uint32_t i = 0; i < doc.ObjectLength(); i++ )
	{
		const char* key = doc.ObjectGetKey( i );
		const ae::DocumentValue& value = doc.ObjectGetValue( i );

		if( std::string( key ) == "apple" )
		{
			REQUIRE( value.StringGet() == std::string( "fruit1" ) );
			foundApple = true;
		}
		else if( std::string( key ) == "banana" )
		{
			REQUIRE( value.StringGet() == std::string( "fruit2" ) );
			foundBanana = true;
		}
		else if( std::string( key ) == "cherry" )
		{
			REQUIRE( value.StringGet() == std::string( "fruit3" ) );
			foundCherry = true;
		}
	}
	REQUIRE( foundApple );
	REQUIRE( foundBanana );
	REQUIRE( foundCherry );

	// Test non-const MapGetValue for modification
	for( uint32_t i = 0; i < doc.ObjectLength(); i++ )
	{
		const char* key = doc.ObjectGetKey( i );
		ae::DocumentValue& value = doc.ObjectGetValue( i );

		if( std::string( key ) == "banana" )
		{
			value.StringSet( "yellow_fruit" );
			break;
		}
	}
	doc.EndUndoGroup();

	// Verify the modification
	REQUIRE( doc.ObjectTryGet( "banana" )->StringGet() == std::string( "yellow_fruit" ) );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Remove one key and verify iteration still works
	REQUIRE( doc.ObjectRemove( "apple" ) ); // Should return true for successful removal
	doc.EndUndoGroup();

	REQUIRE( doc.ObjectLength() == 2 );
	REQUIRE( doc.GetUndoStackSize() == 4 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Verify remaining keys through iteration
	foundBanana = false;
	foundCherry = false;
	foundApple = false;
	for( uint32_t i = 0; i < doc.ObjectLength(); i++ )
	{
		const char* key = doc.ObjectGetKey( i );
		const ae::DocumentValue& value = doc.ObjectGetValue( i );
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
	REQUIRE( doc.ObjectLength() == 3 );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Verify all keys are back through iteration
	foundApple = false;
	foundBanana = false;
	foundCherry = false;
	for( uint32_t i = 0; i < doc.ObjectLength(); i++ )
	{
		const char* key = doc.ObjectGetKey( i );

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
	doc.ObjectInitialize( 4 );
	doc.EndUndoGroup();
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Add some map strings
	doc.ObjectSet( "existing1" ).StringSet( "string1" );
	doc.ObjectSet( "existing2" ).StringSet( "string2" );
	doc.EndUndoGroup();

	REQUIRE( doc.ObjectLength() == 2 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test successful removal - should return true
	REQUIRE( doc.ObjectRemove( "existing1" ) == true );
	doc.EndUndoGroup();

	REQUIRE( doc.ObjectLength() == 1 );
	REQUIRE( doc.ObjectTryGet( "existing1" ) == nullptr );
	REQUIRE( doc.ObjectTryGet( "existing2" ) != nullptr );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test unsuccessful removal - should return false for non-existent key
	REQUIRE( doc.ObjectRemove( "nonexistent" ) == false );
	doc.EndUndoGroup();

	// Map should be unchanged after failed removal
	REQUIRE( doc.ObjectLength() == 1 );
	REQUIRE( doc.ObjectTryGet( "existing2" ) != nullptr );
	// Undo stack should be unchanged since no operation was performed
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test removing the same key again - should return false
	REQUIRE( doc.ObjectRemove( "existing1" ) == false ); // Already removed
	doc.EndUndoGroup();

	// Map should still be unchanged
	REQUIRE( doc.ObjectLength() == 1 );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test successful removal of remaining key
	REQUIRE( doc.ObjectRemove( "existing2" ) == true );
	doc.EndUndoGroup();

	REQUIRE( doc.ObjectLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 4 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test removal from empty map - should return false
	REQUIRE( doc.ObjectRemove( "anything" ) == false );
	doc.EndUndoGroup();

	REQUIRE( doc.ObjectLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 4 ); // No change since no operation performed
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo and test that removed keys can be successfully removed again
	REQUIRE( doc.Undo() ); // Restore "existing2"
	REQUIRE( doc.ObjectLength() == 1 );
	REQUIRE( doc.ObjectTryGet( "existing2" ) != nullptr );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Should be able to remove it again
	REQUIRE( doc.ObjectRemove( "existing2" ) == true );
	doc.EndUndoGroup();

	REQUIRE( doc.ObjectLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 4 ); // Redo stack cleared, new operation added
	REQUIRE( doc.GetRedoStackSize() == 0 );
}

TEST_CASE( "DocumentUndo MapGetVsTryGet", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	const ae::DocumentValue& constDoc = doc;
	doc.ObjectInitialize( 4 );
	doc.EndUndoGroup();
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Add some map strings
	doc.ObjectSet( "existing" ).StringSet( "string" );
	doc.EndUndoGroup();

	REQUIRE( doc.ObjectLength() == 1 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test const ObjectTryGet with existing key - should return valid pointer
	const ae::DocumentValue* constExisting = constDoc.ObjectTryGet( "existing" );
	REQUIRE( constExisting != nullptr );
	REQUIRE( constExisting->StringGet() == std::string( "string" ) );

	// Test const ObjectTryGet with non-existing key - should return nullptr
	const ae::DocumentValue* constNonExisting = constDoc.ObjectTryGet( "nonexistent" );
	REQUIRE( constNonExisting == nullptr );

	// Test non-const ObjectTryGet with existing key - should return valid pointer
	ae::DocumentValue* nonConstExisting = doc.ObjectTryGet( "existing" );
	REQUIRE( nonConstExisting != nullptr );
	REQUIRE( nonConstExisting->StringGet() == std::string( "string" ) );

	// Test non-const ObjectTryGet with non-existing key - should return nullptr
	ae::DocumentValue* nonConstNonExisting = doc.ObjectTryGet( "nonexistent" );
	REQUIRE( nonConstNonExisting == nullptr );

	// Test modification through non-const ObjectTryGet pointer
	ae::DocumentValue* modifyPtr = doc.ObjectTryGet( "existing" );
	REQUIRE( modifyPtr != nullptr );
	modifyPtr->StringSet( "modified_via_tryget" );
	doc.EndUndoGroup();

	// Verify modification worked
	REQUIRE( doc.ObjectTryGet( "existing" )->StringGet() == std::string( "modified_via_tryget" ) );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	ae::DocumentValue& nonConstRef = doc.ObjectSet( "existing" );
	REQUIRE( nonConstRef.StringGet() == std::string( "modified_via_tryget" ) );

	// Modify through MapGet reference
	nonConstRef.StringSet( "modified_string" );
	doc.EndUndoGroup();

	// Verify modification
	REQUIRE( doc.ObjectTryGet( "existing" )->StringGet() == std::string( "modified_string" ) );
	REQUIRE( doc.GetUndoStackSize() == 4 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Note: MapGet with non-existent key would assert in debug builds
	// so we don't test that scenario
}

TEST_CASE( "DocumentUndo EdgeCasesAndInitialState", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );

	// Test initial empty document state
	REQUIRE( doc.GetType() == ae::DocumentValueType::Null );
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

	// Test setting empty basic string
	doc.StringSet( "" );
	doc.EndUndoGroup();
	REQUIRE( doc.IsString() );
	REQUIRE( doc.StringGet() == std::string( "" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test setting same string (should not create new undo operation)
	doc.StringSet( "" );
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
	doc.ObjectInitialize( 0 );
	doc.EndUndoGroup();
	REQUIRE( doc.IsObject() );
	REQUIRE( doc.ObjectLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test MapRemove on empty map
	REQUIRE( doc.ObjectRemove( "anything" ) == false );
	doc.EndUndoGroup();
	REQUIRE( doc.ObjectLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 3 ); // Should not change
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test ObjectTryGet on empty map
	REQUIRE( doc.ObjectTryGet( "anything" ) == nullptr );

	// Test multiple type changes in same group - should coalesce SetType operations
	doc.StringSet( "test" );
	doc.ArrayInitialize( 2 );
	doc.ObjectInitialize( 2 );
	doc.EndUndoGroup(); // Should coalesce into single type change operation

	REQUIRE( doc.IsObject() );
	REQUIRE( doc.GetUndoStackSize() == 4 ); // Previous 3 + 1 coalesced group
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should revert to previous state (empty map from before the coalesced changes)
	REQUIRE( doc.Undo() );
	REQUIRE( doc.IsObject() );
	REQUIRE( doc.ObjectLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 1 );
}

TEST_CASE( "DocumentUndo Initialize", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test Initialize with same type (None -> None) - should be no-op
	ae::DocumentValue& result = doc.Initialize( ae::DocumentValueType::Null );
	REQUIRE( &result == &doc ); // Should return reference to self
	REQUIRE( doc.GetType() == ae::DocumentValueType::Null );
	REQUIRE( doc.GetUndoStackSize() == 0 ); // No undo operation created
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test Initialize from None to Basic
	doc.Initialize( ae::DocumentValueType::String );
	doc.EndUndoGroup();
	REQUIRE( doc.GetType() == ae::DocumentValueType::String );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Set a basic string to test clearing
	doc.StringSet( "test_string" );
	doc.EndUndoGroup();
	REQUIRE( doc.StringGet() == std::string( "test_string" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test Initialize from Basic to Array (should clear basic string)
	doc.Initialize( ae::DocumentValueType::Array );
	doc.EndUndoGroup();
	REQUIRE( doc.GetType() == ae::DocumentValueType::Array );
	REQUIRE( doc.ArrayLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Add array elements to test clearing
	doc.ArrayAppend().StringSet( "array1" );
	doc.ArrayAppend().StringSet( "array2" );
	doc.ArrayAppend().StringSet( "array3" );
	doc.EndUndoGroup();
	REQUIRE( doc.ArrayLength() == 3 );
	REQUIRE( doc.ArrayGet( 0 ).StringGet() == std::string( "array1" ) );
	REQUIRE( doc.ArrayGet( 1 ).StringGet() == std::string( "array2" ) );
	REQUIRE( doc.ArrayGet( 2 ).StringGet() == std::string( "array3" ) );
	REQUIRE( doc.GetUndoStackSize() == 4 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test Initialize from Array to object (should clear all array elements)
	doc.Initialize( ae::DocumentValueType::Object );
	doc.EndUndoGroup();
	REQUIRE( doc.GetType() == ae::DocumentValueType::Object );
	REQUIRE( doc.ObjectLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 5 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Add map elements to test clearing
	doc.ObjectSet( "key1" ).StringSet( "string1" );
	doc.ObjectSet( "key2" ).StringSet( "string2" );
	doc.ObjectSet( "key3" ).StringSet( "string3" );
	doc.EndUndoGroup();
	REQUIRE( doc.ObjectLength() == 3 );
	REQUIRE( doc.ObjectTryGet( "key1" )->StringGet() == std::string( "string1" ) );
	REQUIRE( doc.ObjectTryGet( "key2" )->StringGet() == std::string( "string2" ) );
	REQUIRE( doc.ObjectTryGet( "key3" )->StringGet() == std::string( "string3" ) );
	REQUIRE( doc.GetUndoStackSize() == 6 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test Initialize from Map to Basic (should clear all map elements)
	doc.Initialize( ae::DocumentValueType::String );
	doc.EndUndoGroup();
	REQUIRE( doc.GetType() == ae::DocumentValueType::String );
	REQUIRE( doc.GetUndoStackSize() == 7 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test undo sequence - should restore each state in reverse order
	REQUIRE( doc.Undo() ); // Back to object with 3 elements
	REQUIRE( doc.GetType() == ae::DocumentValueType::Object );
	REQUIRE( doc.ObjectLength() == 3 );
	REQUIRE( doc.ObjectTryGet( "key1" )->StringGet() == std::string( "string1" ) );
	REQUIRE( doc.ObjectTryGet( "key2" )->StringGet() == std::string( "string2" ) );
	REQUIRE( doc.ObjectTryGet( "key3" )->StringGet() == std::string( "string3" ) );
	REQUIRE( doc.GetUndoStackSize() == 6 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	REQUIRE( doc.Undo() ); // Back to empty Map
	REQUIRE( doc.GetType() == ae::DocumentValueType::Object );
	REQUIRE( doc.ObjectLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 5 );
	REQUIRE( doc.GetRedoStackSize() == 2 );

	REQUIRE( doc.Undo() ); // Back to Array with 3 elements
	REQUIRE( doc.GetType() == ae::DocumentValueType::Array );
	REQUIRE( doc.ArrayLength() == 3 );
	REQUIRE( doc.ArrayGet( 0 ).StringGet() == std::string( "array1" ) );
	REQUIRE( doc.ArrayGet( 1 ).StringGet() == std::string( "array2" ) );
	REQUIRE( doc.ArrayGet( 2 ).StringGet() == std::string( "array3" ) );
	REQUIRE( doc.GetUndoStackSize() == 4 );
	REQUIRE( doc.GetRedoStackSize() == 3 );

	REQUIRE( doc.Undo() ); // Back to empty Array
	REQUIRE( doc.GetType() == ae::DocumentValueType::Array );
	REQUIRE( doc.ArrayLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 4 );

	REQUIRE( doc.Undo() ); // Back to Basic with string
	REQUIRE( doc.GetType() == ae::DocumentValueType::String );
	REQUIRE( doc.StringGet() == std::string( "test_string" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 5 );

	REQUIRE( doc.Undo() ); // Back to empty Basic
	REQUIRE( doc.GetType() == ae::DocumentValueType::String );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 6 );

	REQUIRE( doc.Undo() ); // Back to None
	REQUIRE( doc.GetType() == ae::DocumentValueType::Null );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 7 );

	// Test redo sequence - should restore forward
	REQUIRE( doc.Redo() ); // Forward to Basic
	REQUIRE( doc.GetType() == ae::DocumentValueType::String );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 6 );

	REQUIRE( doc.Redo() ); // Forward to Basic with string
	REQUIRE( doc.GetType() == ae::DocumentValueType::String );
	REQUIRE( doc.StringGet() == std::string( "test_string" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 5 );

	// Test Initialize with same type when data exists - should clear data
	doc.Initialize( ae::DocumentValueType::String );
	doc.EndUndoGroup();
	REQUIRE( doc.GetType() == ae::DocumentValueType::String );
	REQUIRE( doc.GetUndoStackSize() == 3 ); // Clearing the basic string + type change
	REQUIRE( doc.GetRedoStackSize() == 0 ); // Redo stack cleared

	// Undo should restore the basic string
	REQUIRE( doc.Undo() );
	REQUIRE( doc.GetType() == ae::DocumentValueType::String );
	REQUIRE( doc.StringGet() == std::string( "test_string" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 1 );
}

TEST_CASE( "DocumentUndo InitializeChaining", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );

	// Test method chaining with Initialize
	doc.Initialize( ae::DocumentValueType::Array ).ArrayAppend().StringSet( "chained1" );
	doc.ArrayAppend().StringSet( "chained2" );
	doc.EndUndoGroup();

	REQUIRE( doc.GetType() == ae::DocumentValueType::Array );
	REQUIRE( doc.ArrayLength() == 2 );
	REQUIRE( doc.ArrayGet( 0 ).StringGet() == std::string( "chained1" ) );
	REQUIRE( doc.ArrayGet( 1 ).StringGet() == std::string( "chained2" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test chaining with type change
	doc.Initialize( ae::DocumentValueType::Object ).ObjectSet( "chained_key" ).StringSet( "chained_string" );
	doc.EndUndoGroup();

	REQUIRE( doc.GetType() == ae::DocumentValueType::Object );
	REQUIRE( doc.ObjectLength() == 1 );
	REQUIRE( doc.ObjectTryGet( "chained_key" )->StringGet() == std::string( "chained_string" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should restore array
	REQUIRE( doc.Undo() );
	REQUIRE( doc.GetType() == ae::DocumentValueType::Array );
	REQUIRE( doc.ArrayLength() == 2 );
	REQUIRE( doc.ArrayGet( 0 ).StringGet() == std::string( "chained1" ) );
	REQUIRE( doc.ArrayGet( 1 ).StringGet() == std::string( "chained2" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 1 );
}

TEST_CASE( "DocumentUndo InitializeComplexClear", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );

	// Create complex nested array structure
	doc.ArrayInitialize( 3 );
	doc.ArrayAppend().ArrayInitialize( 2 );
	doc.ArrayGet( 0 ).ArrayAppend().StringSet( "nested1" );
	doc.ArrayGet( 0 ).ArrayAppend().ObjectInitialize( 2 );
	doc.ArrayGet( 0 ).ArrayGet( 1 ).ObjectSet( "nested_key" ).StringSet( "nested_string" );
	doc.ArrayAppend().StringSet( "second" );
	doc.EndUndoGroup();

	REQUIRE( doc.IsArray() );
	REQUIRE( doc.ArrayLength() == 2 );
	REQUIRE( doc.ArrayGet( 0 ).IsArray() );
	REQUIRE( doc.ArrayGet( 0 ).ArrayLength() == 2 );
	REQUIRE( doc.ArrayGet( 0 ).ArrayGet( 0 ).StringGet() == std::string( "nested1" ) );
	REQUIRE( doc.ArrayGet( 0 ).ArrayGet( 1 ).IsObject() );
	REQUIRE( doc.ArrayGet( 0 ).ArrayGet( 1 ).ObjectTryGet( "nested_key" )->StringGet() == std::string( "nested_string" ) );
	REQUIRE( doc.ArrayGet( 1 ).StringGet() == std::string( "second" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Initialize to object should clear all nested structures
	doc.Initialize( ae::DocumentValueType::Object );
	doc.EndUndoGroup();
	REQUIRE( doc.IsObject() );
	REQUIRE( doc.ObjectLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should restore entire complex structure
	REQUIRE( doc.Undo() );
	REQUIRE( doc.IsArray() );
	REQUIRE( doc.ArrayLength() == 2 );
	REQUIRE( doc.ArrayGet( 0 ).IsArray() );
	REQUIRE( doc.ArrayGet( 0 ).ArrayLength() == 2 );
	REQUIRE( doc.ArrayGet( 0 ).ArrayGet( 0 ).StringGet() == std::string( "nested1" ) );
	REQUIRE( doc.ArrayGet( 0 ).ArrayGet( 1 ).IsObject() );
	REQUIRE( doc.ArrayGet( 0 ).ArrayGet( 1 ).ObjectTryGet( "nested_key" )->StringGet() == std::string( "nested_string" ) );
	REQUIRE( doc.ArrayGet( 1 ).StringGet() == std::string( "second" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Redo should clear back to empty map
	REQUIRE( doc.Redo() );
	REQUIRE( doc.IsObject() );
	REQUIRE( doc.ObjectLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );
}

TEST_CASE( "DocumentUndo InitializeCoalescing", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );

	// Set initial state
	doc.StringSet( "initial" );
	doc.EndUndoGroup();
	REQUIRE( doc.IsString() );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Multiple Initialize calls in same group should coalesce SetType operations
	doc.Initialize( ae::DocumentValueType::Array );
	doc.Initialize( ae::DocumentValueType::Object );
	doc.Initialize( ae::DocumentValueType::String );
	doc.Initialize( ae::DocumentValueType::Array );
	doc.EndUndoGroup(); // All SetType operations should coalesce into one

	REQUIRE( doc.IsArray() );
	REQUIRE( doc.ArrayLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 2 ); // Initial + coalesced group
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Single undo should revert to initial Basic state
	REQUIRE( doc.Undo() );
	REQUIRE( doc.IsString() );
	REQUIRE( doc.StringGet() == std::string( "initial" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Redo should restore final Array state
	REQUIRE( doc.Redo() );
	REQUIRE( doc.IsArray() );
	REQUIRE( doc.ArrayLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test coalescing with data operations in between
	doc.ArrayAppend().StringSet( "element1" );
	doc.Initialize( ae::DocumentValueType::Object ); // Should clear array and change type
	doc.ObjectSet( "key1" ).StringSet( "string1" );
	doc.Initialize( ae::DocumentValueType::String ); // Should clear map and change type
	doc.StringSet( "final_basic" );
	doc.EndUndoGroup();

	REQUIRE( doc.IsString() );
	REQUIRE( doc.StringGet() == std::string( "final_basic" ) );
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
	doc.ArrayAppend().StringSet( "first" );
	doc.ArrayAppend().StringSet( "second" );
	doc.ArrayAppend().StringSet( "third" );
	doc.EndUndoGroup();

	REQUIRE( doc.IsArray() );
	REQUIRE( doc.ArrayLength() == 3 );
	REQUIRE( doc.ArrayGet( 0 ).StringGet() == std::string( "first" ) );
	REQUIRE( doc.ArrayGet( 1 ).StringGet() == std::string( "second" ) );
	REQUIRE( doc.ArrayGet( 2 ).StringGet() == std::string( "third" ) );
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
	doc.ArrayAppend().StringSet( "new1" );
	doc.ArrayAppend().StringSet( "new2" );
	doc.EndUndoGroup();

	REQUIRE( doc.ArrayLength() == 2 );
	REQUIRE( doc.ArrayGet( 0 ).StringGet() == std::string( "new1" ) );
	REQUIRE( doc.ArrayGet( 1 ).StringGet() == std::string( "new2" ) );
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
	REQUIRE( doc.ArrayGet( 0 ).StringGet() == std::string( "first" ) );
	REQUIRE( doc.ArrayGet( 1 ).StringGet() == std::string( "second" ) );
	REQUIRE( doc.ArrayGet( 2 ).StringGet() == std::string( "third" ) );
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
	REQUIRE( doc.ArrayGet( 0 ).StringGet() == std::string( "new1" ) );
	REQUIRE( doc.ArrayGet( 1 ).StringGet() == std::string( "new2" ) );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test ArrayInitialize on different type (basic string)
	doc.StringSet( "convert_to_array" );
	doc.EndUndoGroup();
	REQUIRE( doc.IsString() );
	REQUIRE( doc.StringGet() == std::string( "convert_to_array" ) );
	REQUIRE( doc.GetUndoStackSize() == 4 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// ArrayInitialize should convert from basic to array
	doc.ArrayInitialize( 1 );
	doc.EndUndoGroup();
	REQUIRE( doc.IsArray() );
	REQUIRE( doc.ArrayLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 5 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should restore basic string
	REQUIRE( doc.Undo() );
	REQUIRE( doc.IsString() );
	REQUIRE( doc.StringGet() == std::string( "convert_to_array" ) );
	REQUIRE( doc.GetUndoStackSize() == 4 );
	REQUIRE( doc.GetRedoStackSize() == 1 );
}

TEST_CASE( "DocumentUndo ObjectInitialize", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Start with map and add some key-value pairs
	doc.ObjectInitialize( 4 );
	doc.ObjectSet( "key1" ).StringSet( "string1" );
	doc.ObjectSet( "key2" ).StringSet( "string2" );
	doc.ObjectSet( "key3" ).StringSet( "string3" );
	doc.EndUndoGroup();

	REQUIRE( doc.IsObject() );
	REQUIRE( doc.ObjectLength() == 3 );
	REQUIRE( doc.ObjectTryGet( "key1" )->StringGet() == std::string( "string1" ) );
	REQUIRE( doc.ObjectTryGet( "key2" )->StringGet() == std::string( "string2" ) );
	REQUIRE( doc.ObjectTryGet( "key3" )->StringGet() == std::string( "string3" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// ObjectInitialize should clear existing map contents
	doc.ObjectInitialize( 2 );
	doc.EndUndoGroup();

	REQUIRE( doc.IsObject() );
	REQUIRE( doc.ObjectLength() == 0 ); // Should be cleared
	REQUIRE( doc.ObjectTryGet( "key1" ) == nullptr );
	REQUIRE( doc.ObjectTryGet( "key2" ) == nullptr );
	REQUIRE( doc.ObjectTryGet( "key3" ) == nullptr );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Add new key-value pairs to verify it works after clearing
	doc.ObjectSet( "newkey1" ).StringSet( "newstring1" );
	doc.ObjectSet( "newkey2" ).StringSet( "newstring2" );
	doc.EndUndoGroup();

	REQUIRE( doc.ObjectLength() == 2 );
	REQUIRE( doc.ObjectTryGet( "newkey1" )->StringGet() == std::string( "newstring1" ) );
	REQUIRE( doc.ObjectTryGet( "newkey2" )->StringGet() == std::string( "newstring2" ) );
	REQUIRE( doc.ObjectTryGet( "key1" ) == nullptr ); // Old keys should still be gone
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should restore empty map
	REQUIRE( doc.Undo() );
	REQUIRE( doc.IsObject() );
	REQUIRE( doc.ObjectLength() == 0 );
	REQUIRE( doc.ObjectTryGet( "newkey1" ) == nullptr );
	REQUIRE( doc.ObjectTryGet( "newkey2" ) == nullptr );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Undo should restore original map with all key-value pairs
	REQUIRE( doc.Undo() );
	REQUIRE( doc.IsObject() );
	REQUIRE( doc.ObjectLength() == 3 );
	REQUIRE( doc.ObjectTryGet( "key1" )->StringGet() == std::string( "string1" ) );
	REQUIRE( doc.ObjectTryGet( "key2" )->StringGet() == std::string( "string2" ) );
	REQUIRE( doc.ObjectTryGet( "key3" )->StringGet() == std::string( "string3" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 2 );

	// Redo should restore cleared map
	REQUIRE( doc.Redo() );
	REQUIRE( doc.IsObject() );
	REQUIRE( doc.ObjectLength() == 0 );
	REQUIRE( doc.ObjectTryGet( "key1" ) == nullptr );
	REQUIRE( doc.ObjectTryGet( "key2" ) == nullptr );
	REQUIRE( doc.ObjectTryGet( "key3" ) == nullptr );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Redo should restore new key-value pairs
	REQUIRE( doc.Redo() );
	REQUIRE( doc.IsObject() );
	REQUIRE( doc.ObjectLength() == 2 );
	REQUIRE( doc.ObjectTryGet( "newkey1" )->StringGet() == std::string( "newstring1" ) );
	REQUIRE( doc.ObjectTryGet( "newkey2" )->StringGet() == std::string( "newstring2" ) );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test ObjectInitialize on different type (basic string)
	doc.StringSet( "convert_to_object" );
	doc.EndUndoGroup();
	REQUIRE( doc.IsString() );
	REQUIRE( doc.StringGet() == std::string( "convert_to_object" ) );
	REQUIRE( doc.GetUndoStackSize() == 4 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// ObjectInitialize should convert from basic to object
	doc.ObjectInitialize( 1 );
	doc.EndUndoGroup();
	REQUIRE( doc.IsObject() );
	REQUIRE( doc.ObjectLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 5 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should restore basic string
	REQUIRE( doc.Undo() );
	REQUIRE( doc.IsString() );
	REQUIRE( doc.StringGet() == std::string( "convert_to_object" ) );
	REQUIRE( doc.GetUndoStackSize() == 4 );
	REQUIRE( doc.GetRedoStackSize() == 1 );
}

TEST_CASE( "DocumentUndo ObjectOrderStability", "[ae::Document][undo][object]" )
{
	ae::Document doc( "test" );
	doc.ObjectInitialize( 10 );

	// Add elements in specific order
	doc.ObjectSet( "alpha" ).StringSet( "first" );
	doc.ObjectSet( "beta" ).StringSet( "second" );
	doc.ObjectSet( "gamma" ).StringSet( "third" );
	doc.ObjectSet( "delta" ).StringSet( "fourth" );
	doc.ObjectSet( "epsilon" ).StringSet( "fifth" );
	doc.EndUndoGroup();

	// Verify initial order through iteration
	REQUIRE( doc.ObjectLength() == 5 );
	REQUIRE( doc.ObjectGetKey( 0 ) == std::string( "alpha" ) );
	REQUIRE( doc.ObjectGetKey( 1 ) == std::string( "beta" ) );
	REQUIRE( doc.ObjectGetKey( 2 ) == std::string( "gamma" ) );
	REQUIRE( doc.ObjectGetKey( 3 ) == std::string( "delta" ) );
	REQUIRE( doc.ObjectGetKey( 4 ) == std::string( "epsilon" ) );

	// Modify existing strings (should not affect order)
	doc.ObjectSet( "gamma" ).StringSet( "modified_third" );
	doc.ObjectSet( "alpha" ).StringSet( "modified_first" );
	doc.EndUndoGroup();

	// Verify order is preserved after modifications
	REQUIRE( doc.ObjectLength() == 5 );
	REQUIRE( doc.ObjectGetKey( 0 ) == std::string( "alpha" ) );
	REQUIRE( doc.ObjectGetKey( 1 ) == std::string( "beta" ) );
	REQUIRE( doc.ObjectGetKey( 2 ) == std::string( "gamma" ) );
	REQUIRE( doc.ObjectGetKey( 3 ) == std::string( "delta" ) );
	REQUIRE( doc.ObjectGetKey( 4 ) == std::string( "epsilon" ) );
	REQUIRE( doc.ObjectGetValue( 0 ).StringGet() == std::string( "modified_first" ) );
	REQUIRE( doc.ObjectGetValue( 2 ).StringGet() == std::string( "modified_third" ) );

	// Add new elements (should append to end)
	doc.ObjectSet( "zeta" ).StringSet( "sixth" );
	doc.ObjectSet( "eta" ).StringSet( "seventh" );
	doc.EndUndoGroup();

	// Verify new elements maintain insertion order
	REQUIRE( doc.ObjectLength() == 7 );
	REQUIRE( doc.ObjectGetKey( 5 ) == std::string( "zeta" ) );
	REQUIRE( doc.ObjectGetKey( 6 ) == std::string( "eta" ) );

	// Remove middle element
	doc.ObjectRemove( "gamma" );
	doc.EndUndoGroup();

	// Verify order is preserved after removal (remaining elements shift)
	REQUIRE( doc.ObjectLength() == 6 );
	REQUIRE( doc.ObjectGetKey( 0 ) == std::string( "alpha" ) );
	REQUIRE( doc.ObjectGetKey( 1 ) == std::string( "beta" ) );
	REQUIRE( doc.ObjectGetKey( 2 ) == std::string( "delta" ) ); // gamma removed, delta shifts up
	REQUIRE( doc.ObjectGetKey( 3 ) == std::string( "epsilon" ) );
	REQUIRE( doc.ObjectGetKey( 4 ) == std::string( "zeta" ) );
	REQUIRE( doc.ObjectGetKey( 5 ) == std::string( "eta" ) );

	// Undo removal - element should return to same position
	REQUIRE( doc.Undo() );
	REQUIRE( doc.ObjectLength() == 7 );
	REQUIRE( doc.ObjectGetKey( 0 ) == std::string( "alpha" ) );
	REQUIRE( doc.ObjectGetKey( 1 ) == std::string( "beta" ) );
	REQUIRE( doc.ObjectGetKey( 2 ) == std::string( "gamma" ) ); // gamma restored to original position
	REQUIRE( doc.ObjectGetKey( 3 ) == std::string( "delta" ) );
	REQUIRE( doc.ObjectGetKey( 4 ) == std::string( "epsilon" ) );
	REQUIRE( doc.ObjectGetKey( 5 ) == std::string( "zeta" ) );
	REQUIRE( doc.ObjectGetKey( 6 ) == std::string( "eta" ) );
	REQUIRE( doc.ObjectGetValue( 2 ).StringGet() == std::string( "modified_third" ) );

	// Undo additions
	REQUIRE( doc.Undo() );
	REQUIRE( doc.ObjectLength() == 5 );
	REQUIRE( doc.ObjectGetKey( 4 ) == std::string( "epsilon" ) ); // zeta and eta should be gone
	REQUIRE( doc.ObjectTryGet( "zeta" ) == nullptr );
	REQUIRE( doc.ObjectTryGet( "eta" ) == nullptr );

	// Undo modifications - strings should revert but order preserved
	REQUIRE( doc.Undo() );
	REQUIRE( doc.ObjectLength() == 5 );
	REQUIRE( doc.ObjectGetKey( 0 ) == std::string( "alpha" ) );
	REQUIRE( doc.ObjectGetKey( 2 ) == std::string( "gamma" ) );
	REQUIRE( doc.ObjectGetValue( 0 ).StringGet() == std::string( "first" ) ); // alpha reverted
	REQUIRE( doc.ObjectGetValue( 2 ).StringGet() == std::string( "third" ) ); // gamma reverted

	// Test complete redo cycle maintains order
	REQUIRE( doc.Redo() ); // redo modifications
	REQUIRE( doc.Redo() ); // redo additions
	REQUIRE( doc.Redo() ); // redo removal

	// Final state should match removal state
	REQUIRE( doc.ObjectLength() == 6 );
	REQUIRE( doc.ObjectGetKey( 0 ) == std::string( "alpha" ) );
	REQUIRE( doc.ObjectGetKey( 1 ) == std::string( "beta" ) );
	REQUIRE( doc.ObjectGetKey( 2 ) == std::string( "delta" ) ); // gamma removed
	REQUIRE( doc.ObjectGetKey( 3 ) == std::string( "epsilon" ) );
	REQUIRE( doc.ObjectGetKey( 4 ) == std::string( "zeta" ) );
	REQUIRE( doc.ObjectGetKey( 5 ) == std::string( "eta" ) );
	REQUIRE( doc.ObjectTryGet( "gamma" ) == nullptr );

	// Test that re-adding a removed key appends to end (new insertion)
	doc.ObjectSet( "gamma" ).StringSet( "re_added" );
	doc.EndUndoGroup();

	REQUIRE( doc.ObjectLength() == 7 );
	REQUIRE( doc.ObjectGetKey( 6 ) == std::string( "gamma" ) ); // Should be at end, not original position
	REQUIRE( doc.ObjectGetValue( 6 ).StringGet() == std::string( "re_added" ) );

	// Verify all other elements maintain their positions
	REQUIRE( doc.ObjectGetKey( 0 ) == std::string( "alpha" ) );
	REQUIRE( doc.ObjectGetKey( 1 ) == std::string( "beta" ) );
	REQUIRE( doc.ObjectGetKey( 2 ) == std::string( "delta" ) );
	REQUIRE( doc.ObjectGetKey( 3 ) == std::string( "epsilon" ) );
	REQUIRE( doc.ObjectGetKey( 4 ) == std::string( "zeta" ) );
	REQUIRE( doc.ObjectGetKey( 5 ) == std::string( "eta" ) );
}

TEST_CASE( "DocumentUndo MapOrderPerfectRestoration", "[ae::Document][undo][object][order]" )
{
	ae::Document doc( "test" );
	doc.ObjectInitialize( 10 );

	// Build a complex map with specific ordering
	doc.ObjectSet( "z_last" ).StringSet( "should_be_first" );
	doc.ObjectSet( "a_first" ).StringSet( "should_be_second" );
	doc.ObjectSet( "m_middle" ).StringSet( "should_be_third" );
	doc.EndUndoGroup();

	// Capture the original order
	std::vector< std::string > originalOrder;
	for( uint32_t i = 0; i < doc.ObjectLength(); i++ )
	{
		originalOrder.push_back( doc.ObjectGetKey( i ) );
	}

	REQUIRE( originalOrder.size() == 3 );
	REQUIRE( originalOrder[ 0 ] == "z_last" );
	REQUIRE( originalOrder[ 1 ] == "a_first" );
	REQUIRE( originalOrder[ 2 ] == "m_middle" );

	// Remove the middle element
	REQUIRE( doc.ObjectRemove( "a_first" ) );
	doc.EndUndoGroup();

	REQUIRE( doc.ObjectLength() == 2 );
	REQUIRE( doc.ObjectGetKey( 0 ) == std::string( "z_last" ) );
	REQUIRE( doc.ObjectGetKey( 1 ) == std::string( "m_middle" ) );
	REQUIRE( doc.ObjectTryGet( "a_first" ) == nullptr );

	// Add some more elements
	doc.ObjectSet( "new_element" ).StringSet( "new_string" );
	doc.ObjectSet( "another_new" ).StringSet( "another_string" );
	doc.EndUndoGroup();

	REQUIRE( doc.ObjectLength() == 4 );
	REQUIRE( doc.ObjectGetKey( 2 ) == std::string( "new_element" ) );
	REQUIRE( doc.ObjectGetKey( 3 ) == std::string( "another_new" ) );

	// Undo the additions
	REQUIRE( doc.Undo() );
	REQUIRE( doc.ObjectLength() == 2 );
	REQUIRE( doc.ObjectGetKey( 0 ) == std::string( "z_last" ) );
	REQUIRE( doc.ObjectGetKey( 1 ) == std::string( "m_middle" ) );
	REQUIRE( doc.ObjectTryGet( "new_element" ) == nullptr );
	REQUIRE( doc.ObjectTryGet( "another_new" ) == nullptr );

	// Undo the removal - should restore exact original position
	REQUIRE( doc.Undo() );
	REQUIRE( doc.ObjectLength() == 3 );

	// Verify the exact original order is restored
	for( uint32_t i = 0; i < doc.ObjectLength(); i++ )
	{
		REQUIRE( doc.ObjectGetKey( i ) == originalOrder[ i ] );
	}

	// Verify strings are correct too
	REQUIRE( doc.ObjectSet( "z_last" ).StringGet() == std::string( "should_be_first" ) );
	REQUIRE( doc.ObjectSet( "a_first" ).StringGet() == std::string( "should_be_second" ) );
	REQUIRE( doc.ObjectSet( "m_middle" ).StringGet() == std::string( "should_be_third" ) );

	// Test that redo preserves order through complex operations
	REQUIRE( doc.Redo() ); // redo removal
	REQUIRE( doc.Redo() ); // redo additions

	REQUIRE( doc.ObjectLength() == 4 );
	REQUIRE( doc.ObjectGetKey( 0 ) == std::string( "z_last" ) );
	REQUIRE( doc.ObjectGetKey( 1 ) == std::string( "m_middle" ) ); // a_first removed
	REQUIRE( doc.ObjectGetKey( 2 ) == std::string( "new_element" ) );
	REQUIRE( doc.ObjectGetKey( 3 ) == std::string( "another_new" ) );

	// Now test removing and re-adding the same key
	REQUIRE( doc.ObjectRemove( "z_last" ) );
	doc.EndUndoGroup();

	// Re-add the same key (should append to end, not go back to original position)
	doc.ObjectSet( "z_last" ).StringSet( "reinserted_string" );
	doc.EndUndoGroup();

	REQUIRE( doc.ObjectLength() == 4 );
	REQUIRE( doc.ObjectGetKey( 0 ) == std::string( "m_middle" ) );
	REQUIRE( doc.ObjectGetKey( 1 ) == std::string( "new_element" ) );
	REQUIRE( doc.ObjectGetKey( 2 ) == std::string( "another_new" ) );
	REQUIRE( doc.ObjectGetKey( 3 ) == std::string( "z_last" ) ); // Should be at end now
	REQUIRE( doc.ObjectSet( "z_last" ).StringGet() == std::string( "reinserted_string" ) );
}

TEST_CASE( "DocumentValue NumberSet NumberGet", "[ae::Document][number]" )
{
	ae::Document doc( "test" );

	// Test int
	doc.NumberSet( 42 );
	REQUIRE( doc.IsNumber() );
	REQUIRE( doc.GetType() == ae::DocumentValueType::Number );
	REQUIRE( doc.NumberGet< int >() == 42 );
	REQUIRE( doc.NumberGet< int32_t >() == 42 );
	REQUIRE( doc.NumberGet< int64_t >() == 42 );
	REQUIRE( doc.NumberGet< float >() == 42.0f );
	REQUIRE( doc.NumberGet< double >() == 42.0 );

	// Test float
	doc.NumberSet( 3.14f );
	REQUIRE( doc.IsNumber() );
	REQUIRE( doc.NumberGet< float >() == 3.14f );
	REQUIRE( doc.NumberGet< double >() == Catch::Approx( 3.14 ).epsilon( 0.01 ) );
	REQUIRE( doc.NumberGet< int >() == 3 );

	// Test double
	doc.NumberSet( 2.718281828 );
	REQUIRE( doc.IsNumber() );
	REQUIRE( doc.NumberGet< double >() == 2.718281828 );
	REQUIRE( doc.NumberGet< float >() == Catch::Approx( 2.718281828f ) );

	// Test uint32_t
	doc.NumberSet( uint32_t( 4294967295 ) );
	REQUIRE( doc.IsNumber() );
	REQUIRE( doc.NumberGet< uint32_t >() == 4294967295 );
	REQUIRE( doc.NumberGet< uint64_t >() == 4294967295 );

	// Test int64_t
	doc.NumberSet( int64_t( 9223372036854775807LL ) );
	REQUIRE( doc.IsNumber() );
	REQUIRE( doc.NumberGet< int64_t >() == 9223372036854775807LL );

	// Test negative numbers
	doc.NumberSet( -42 );
	REQUIRE( doc.IsNumber() );
	REQUIRE( doc.NumberGet< int >() == -42 );
	REQUIRE( doc.NumberGet< int64_t >() == -42 );
	REQUIRE( doc.NumberGet< float >() == -42.0f );

	// Test zero
	doc.NumberSet( 0 );
	REQUIRE( doc.IsNumber() );
	REQUIRE( doc.NumberGet< int >() == 0 );
	REQUIRE( doc.NumberGet< double >() == 0.0 );
}

TEST_CASE( "DocumentValue BoolSet BoolGet", "[ae::Document][bool]" )
{
	ae::Document doc( "test" );

	// Test true
	doc.BoolSet( true );
	REQUIRE( doc.IsBool() );
	REQUIRE( doc.GetType() == ae::DocumentValueType::Bool );
	REQUIRE( doc.BoolGet() == true );

	// Test false
	doc.BoolSet( false );
	REQUIRE( doc.IsBool() );
	REQUIRE( doc.BoolGet() == false );

	// Test switching back to true
	doc.BoolSet( true );
	REQUIRE( doc.IsBool() );
	REQUIRE( doc.BoolGet() == true );
}

TEST_CASE( "DocumentValue Number Bool TypeTransitions", "[ae::Document]" )
{
	ae::Document doc( "test" );

	// Start with a string
	doc.StringSet( "hello" );
	REQUIRE( doc.IsString() );

	// Convert to number
	doc.NumberSet( 42 );
	REQUIRE( !doc.IsString() );
	REQUIRE( doc.IsNumber() );
	REQUIRE( doc.NumberGet< int >() == 42 );

	// Convert to bool
	doc.BoolSet( true );
	REQUIRE( !doc.IsNumber() );
	REQUIRE( doc.IsBool() );
	REQUIRE( doc.BoolGet() == true );

	// Convert to opaque
	doc.OpaqueSet( ae::Vec3( 1, 2, 3 ) );
	REQUIRE( !doc.IsBool() );
	REQUIRE( doc.IsOpaque() );

	// Back to number
	doc.NumberSet( 3.14f );
	REQUIRE( !doc.IsOpaque() );
	REQUIRE( doc.IsNumber() );
	REQUIRE( doc.NumberGet< float >() == 3.14f );
}

TEST_CASE( "DocumentUndo NumberSet Coalesce", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );

	// Set initial number
	doc.NumberSet( 10 );
	doc.EndUndoGroup();
	REQUIRE( doc.NumberGet< int >() == 10 );
	REQUIRE( doc.GetUndoStackSize() == 1 );

	// Multiple NumberSet calls should coalesce
	doc.NumberSet( 20 );
	doc.NumberSet( 30 );
	doc.NumberSet( 40 );
	doc.EndUndoGroup();
	REQUIRE( doc.NumberGet< int >() == 40 );
	REQUIRE( doc.GetUndoStackSize() == 2 ); // Only one undo group for all three sets

	// Undo should restore original value
	REQUIRE( doc.Undo() );
	REQUIRE( doc.NumberGet< int >() == 10 );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Redo should restore final value
	REQUIRE( doc.Redo() );
	REQUIRE( doc.NumberGet< int >() == 40 );
}

TEST_CASE( "DocumentUndo BoolSet Coalesce", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );

	// Set initial bool
	doc.BoolSet( false );
	doc.EndUndoGroup();
	REQUIRE( doc.BoolGet() == false );
	REQUIRE( doc.GetUndoStackSize() == 1 );

	// Multiple BoolSet calls should coalesce
	doc.BoolSet( true );
	doc.BoolSet( false );
	doc.BoolSet( true );
	doc.EndUndoGroup();
	REQUIRE( doc.BoolGet() == true );
	REQUIRE( doc.GetUndoStackSize() == 2 );

	// Undo should restore original value
	REQUIRE( doc.Undo() );
	REQUIRE( doc.BoolGet() == false );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Redo
	REQUIRE( doc.Redo() );
	REQUIRE( doc.BoolGet() == true );
}

TEST_CASE( "DocumentValue OpaqueSet OpaqueGet BasicTypes", "[ae::Document][opaque]" )
{
	ae::Document doc( "test" );

	// Test int
	doc.OpaqueSet( 42 );
	REQUIRE( doc.IsOpaque() );
	REQUIRE( doc.GetType() == ae::DocumentValueType::Opaque );
	REQUIRE( doc.OpaqueGet( 0 ) == 42 );
	REQUIRE( doc.OpaqueGet( 999 ) == 42 );

	// Test float
	doc.OpaqueSet( 3.14f );
	REQUIRE( doc.IsOpaque() );
	REQUIRE( doc.OpaqueGet( 0.0f ) == 3.14f );
	REQUIRE( doc.OpaqueGet( 999.0f ) == 3.14f );

	// Test double
	doc.OpaqueSet( 2.718 );
	REQUIRE( doc.IsOpaque() );
	REQUIRE( doc.OpaqueGet( 0.0 ) == 2.718 );
	REQUIRE( doc.OpaqueGet( 999.0 ) == 2.718 );

	// Test bool
	doc.OpaqueSet( true );
	REQUIRE( doc.IsOpaque() );
	REQUIRE( doc.OpaqueGet( false ) == true );

	// Test uint32_t
	doc.OpaqueSet( uint32_t( 4294967295 ) );
	REQUIRE( doc.IsOpaque() );
	REQUIRE( doc.OpaqueGet( uint32_t( 0 ) ) == 4294967295 );

	// Test int64_t
	doc.OpaqueSet( int64_t( 9223372036854775807LL ) );
	REQUIRE( doc.IsOpaque() );
	REQUIRE( doc.OpaqueGet( int64_t( 0 ) ) == 9223372036854775807LL );
}

TEST_CASE( "DocumentValue OpaqueSet OpaqueGet Structures", "[ae::Document][opaque]" )
{
	ae::Document doc( "test" );

	// Test ae::Vec3
	ae::Vec3 vec( 1.0f, 2.0f, 3.0f );
	doc.OpaqueSet( vec );
	REQUIRE( doc.IsOpaque() );
	ae::Vec3 retrieved = doc.OpaqueGet( ae::Vec3( 0.0f ) );
	REQUIRE( retrieved.x == 1.0f );
	REQUIRE( retrieved.y == 2.0f );
	REQUIRE( retrieved.z == 3.0f );

	// Test ae::Color
	ae::Color color( 1.0f, 0.5f, 0.25f, 0.75f );
	doc.OpaqueSet( color );
	REQUIRE( doc.IsOpaque() );
	ae::Color retrievedColor = doc.OpaqueGet( ae::Color( 0.0f ) );
	REQUIRE( retrievedColor.r == 1.0f );
	REQUIRE( retrievedColor.g == 0.5f );
	REQUIRE( retrievedColor.b == 0.25f );
	REQUIRE( retrievedColor.a == 0.75f );

	// Test custom struct
	struct CustomStruct
	{
		int x;
		float y;
		bool z;
	};

	CustomStruct custom{ 100, 3.5f, true };
	doc.OpaqueSet( custom );
	REQUIRE( doc.IsOpaque() );
	CustomStruct retrievedCustom = doc.OpaqueGet( CustomStruct{ 0, 0.0f, false } );
	REQUIRE( retrievedCustom.x == 100 );
	REQUIRE( retrievedCustom.y == 3.5f );
	REQUIRE( retrievedCustom.z == true );
}

TEST_CASE( "DocumentValue OpaqueSet TypeConversion", "[ae::Document][opaque]" )
{
	ae::Document doc( "test" );

	// Start as None type
	REQUIRE( doc.GetType() == ae::DocumentValueType::Null );

	// OpaqueSet should automatically convert to Opaque type
	doc.OpaqueSet( 42 );
	REQUIRE( doc.IsOpaque() );
	REQUIRE( doc.GetType() == ae::DocumentValueType::Opaque );

	// OpaqueSet should work on already Opaque type
	doc.OpaqueSet( 100 );
	REQUIRE( doc.IsOpaque() );
	REQUIRE( doc.OpaqueGet( 0 ) == 100 );

	// Change from String to Opaque
	doc.StringSet( "test_string" );
	REQUIRE( doc.IsString() );
	REQUIRE( !doc.IsOpaque() );

	doc.OpaqueSet( 3.14f );
	REQUIRE( doc.IsOpaque() );
	REQUIRE( !doc.IsString() );
	REQUIRE( doc.OpaqueGet( 0.0f ) == 3.14f );

	// Change from Array to Opaque
	doc.ArrayInitialize( 4 );
	REQUIRE( doc.IsArray() );
	REQUIRE( !doc.IsOpaque() );

	doc.OpaqueSet( true );
	REQUIRE( doc.IsOpaque() );
	REQUIRE( !doc.IsArray() );
	REQUIRE( doc.OpaqueGet( false ) == true );

	// Change from Map to Opaque
	doc.ObjectInitialize( 4 );
	REQUIRE( doc.IsObject() );
	REQUIRE( !doc.IsOpaque() );

	doc.OpaqueSet( 2.718 );
	REQUIRE( doc.IsOpaque() );
	REQUIRE( !doc.IsObject() );
	REQUIRE( doc.OpaqueGet( 0.0 ) == 2.718 );
}

TEST_CASE( "DocumentValue OpaqueSet OpaqueGet DefaultValues", "[ae::Document][opaque]" )
{
	ae::Document doc( "test" );

	// Store int, retrieve with wrong type should return default
	doc.OpaqueSet( 42 );
	REQUIRE( doc.OpaqueGet( 3.14f ) == 3.14f ); // Wrong type returns default
	REQUIRE( doc.OpaqueGet( 2.718 ) == 2.718 ); // Wrong type returns default
	REQUIRE( doc.OpaqueGet( true ) == true ); // Wrong type returns default

	// Store float, retrieve with wrong type should return default
	doc.OpaqueSet( 1.5f );
	REQUIRE( doc.OpaqueGet( 999 ) == 999 ); // Wrong type returns default
	REQUIRE( doc.OpaqueGet( 2.718 ) == 2.718 ); // Wrong type returns default
	REQUIRE( doc.OpaqueGet( false ) == false ); // Wrong type returns default

	// Store zero value
	doc.OpaqueSet( 0 );
	REQUIRE( doc.OpaqueGet( 999 ) == 0 ); // Correct type returns stored value even if zero

	// Store false
	doc.OpaqueSet( false );
	REQUIRE( doc.OpaqueGet( true ) == false ); // Correct type returns stored value

	// Store custom struct, retrieve with wrong type
	struct Point
	{
		float x;
		float y;
	};

	doc.OpaqueSet( Point{ 5.0f, 10.0f } );
	Point defaultPoint{ 99.0f, 88.0f };
	Point result = doc.OpaqueGet( defaultPoint );
	REQUIRE( result.x == 5.0f );
	REQUIRE( result.y == 10.0f );

	// Retrieve with int should return default int
	REQUIRE( doc.OpaqueGet( 123 ) == 123 );
}

TEST_CASE( "DocumentUndo ValueOperations", "[ae::Document][undo][opaque]" )
{
	ae::Document doc( "test" );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Set initial value
	doc.OpaqueSet( 42 );
	doc.EndUndoGroup();
	REQUIRE( doc.IsOpaque() );
	REQUIRE( doc.OpaqueGet( 0 ) == 42 );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Change value (same type)
	doc.OpaqueSet( 100 );
	doc.EndUndoGroup();
	REQUIRE( doc.OpaqueGet( 0 ) == 100 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should restore previous value
	REQUIRE( doc.Undo() );
	REQUIRE( doc.IsOpaque() );
	REQUIRE( doc.OpaqueGet( 0 ) == 42 );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Redo should restore changed value
	REQUIRE( doc.Redo() );
	REQUIRE( doc.OpaqueGet( 0 ) == 100 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Change to different type
	doc.OpaqueSet( 3.14f );
	doc.EndUndoGroup();
	REQUIRE( doc.OpaqueGet( 0.0f ) == 3.14f );
	REQUIRE( doc.GetUndoStackSize() == 3 );

	// Undo should restore int value
	REQUIRE( doc.Undo() );
	REQUIRE( doc.OpaqueGet( 0 ) == 100 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 1 );
}

TEST_CASE( "DocumentUndo ValueCoalescing", "[ae::Document][undo][opaque]" )
{
	ae::Document doc( "test" );

	// Multiple OpaqueSet calls in same group should coalesce
	doc.OpaqueSet( 1 );
	doc.OpaqueSet( 2 );
	doc.OpaqueSet( 3 );
	doc.EndUndoGroup();

	// Should only have one undo operation
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.OpaqueGet( 0 ) == 3 );

	// Undo should go back to None type (original state)
	REQUIRE( doc.Undo() );
	REQUIRE( doc.GetType() == ae::DocumentValueType::Null );
	REQUIRE( doc.GetUndoStackSize() == 0 );

	// Test coalescing with type changes in same group
	doc.OpaqueSet( 100 );
	doc.OpaqueSet( 200 );
	doc.OpaqueSet( 300 );
	doc.EndUndoGroup();

	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.OpaqueGet( 0 ) == 300 );

	// Now change type without ending group
	doc.OpaqueSet( 3.14f );
	REQUIRE( doc.OpaqueGet( 0.0f ) == 3.14f );
	doc.EndUndoGroup();

	// Should have 2 operations total
	REQUIRE( doc.GetUndoStackSize() == 2 );

	// Undo should restore int value
	REQUIRE( doc.Undo() );
	REQUIRE( doc.OpaqueGet( 0 ) == 300 );

	// Undo again should restore None type
	REQUIRE( doc.Undo() );
	REQUIRE( doc.GetType() == ae::DocumentValueType::Null );
}

TEST_CASE( "DocumentUndo ValueTypeChange", "[ae::Document][undo][opaque]" )
{
	ae::Document doc( "test" );

	// Start as value
	doc.OpaqueSet( 42 );
	doc.EndUndoGroup();
	REQUIRE( doc.IsOpaque() );
	REQUIRE( doc.OpaqueGet( 0 ) == 42 );

	// Change to string
	doc.StringSet( "test_string" );
	doc.EndUndoGroup();
	REQUIRE( doc.IsString() );
	REQUIRE( !doc.IsOpaque() );
	REQUIRE( doc.StringGet() == std::string( "test_string" ) );

	// Undo should restore Opaque type and int value
	REQUIRE( doc.Undo() );
	REQUIRE( doc.IsOpaque() );
	REQUIRE( !doc.IsString() );
	REQUIRE( doc.OpaqueGet( 0 ) == 42 );

	// Change to array
	doc.ArrayInitialize( 4 );
	doc.EndUndoGroup();
	REQUIRE( doc.IsArray() );
	REQUIRE( !doc.IsOpaque() );

	// Undo should restore Opaque type
	REQUIRE( doc.Undo() );
	REQUIRE( doc.IsOpaque() );
	REQUIRE( !doc.IsArray() );
	REQUIRE( doc.OpaqueGet( 0 ) == 42 );

	// Change to object
	doc.ObjectInitialize( 4 );
	doc.EndUndoGroup();
	REQUIRE( doc.IsObject() );
	REQUIRE( !doc.IsOpaque() );

	// Undo should restore Opaque type
	REQUIRE( doc.Undo() );
	REQUIRE( doc.IsOpaque() );
	REQUIRE( !doc.IsObject() );
	REQUIRE( doc.OpaqueGet( 0 ) == 42 );
}

TEST_CASE( "DocumentValue OpaqueSet InArrays", "[ae::Document][opaque][array]" )
{
	ae::Document doc( "test" );
	doc.ArrayInitialize( 4 );

	// Add various Opaque types to array
	doc.ArrayAppend().OpaqueSet( 42 );
	doc.ArrayAppend().OpaqueSet( 3.14f );
	doc.ArrayAppend().OpaqueSet( true );
	doc.ArrayAppend().OpaqueSet( ae::Vec3( 1.0f, 2.0f, 3.0f ) );

	REQUIRE( doc.ArrayLength() == 4 );
	REQUIRE( doc.ArrayGet( 0 ).IsOpaque() );
	REQUIRE( doc.ArrayGet( 0 ).OpaqueGet( 0 ) == 42 );
	REQUIRE( doc.ArrayGet( 1 ).IsOpaque() );
	REQUIRE( doc.ArrayGet( 1 ).OpaqueGet( 0.0f ) == 3.14f );
	REQUIRE( doc.ArrayGet( 2 ).IsOpaque() );
	REQUIRE( doc.ArrayGet( 2 ).OpaqueGet( false ) == true );
	REQUIRE( doc.ArrayGet( 3 ).IsOpaque() );

	ae::Vec3 vec = doc.ArrayGet( 3 ).OpaqueGet( ae::Vec3( 0.0f ) );
	REQUIRE( vec.x == 1.0f );
	REQUIRE( vec.y == 2.0f );
	REQUIRE( vec.z == 3.0f );
}

TEST_CASE( "DocumentValue OpaqueSet InMaps", "[ae::Document][opaque][object]" )
{
	ae::Document doc( "test" );
	doc.ObjectInitialize( 4 );

	// Add various Opaque types to object
	doc.ObjectSet( "int_value" ).OpaqueSet( 42 );
	doc.ObjectSet( "float_value" ).OpaqueSet( 3.14f );
	doc.ObjectSet( "bool_value" ).OpaqueSet( true );
	doc.ObjectSet( "vec_value" ).OpaqueSet( ae::Vec3( 1.0f, 2.0f, 3.0f ) );

	REQUIRE( doc.ObjectLength() == 4 );

	// Verify int value
	REQUIRE( doc.ObjectTryGet( "int_value" ) != nullptr );
	REQUIRE( doc.ObjectTryGet( "int_value" )->IsOpaque() );
	REQUIRE( doc.ObjectTryGet( "int_value" )->OpaqueGet( 0 ) == 42 );

	// Verify float value
	REQUIRE( doc.ObjectTryGet( "float_value" ) != nullptr );
	REQUIRE( doc.ObjectTryGet( "float_value" )->IsOpaque() );
	REQUIRE( doc.ObjectTryGet( "float_value" )->OpaqueGet( 0.0f ) == 3.14f );

	// Verify bool value
	REQUIRE( doc.ObjectTryGet( "bool_value" ) != nullptr );
	REQUIRE( doc.ObjectTryGet( "bool_value" )->IsOpaque() );
	REQUIRE( doc.ObjectTryGet( "bool_value" )->OpaqueGet( false ) == true );

	// Verify Vec3 value
	REQUIRE( doc.ObjectTryGet( "vec_value" ) != nullptr );
	REQUIRE( doc.ObjectTryGet( "vec_value" )->IsOpaque() );
	ae::Vec3 vec = doc.ObjectTryGet( "vec_value" )->OpaqueGet( ae::Vec3( 0.0f ) );
	REQUIRE( vec.x == 1.0f );
	REQUIRE( vec.y == 2.0f );
	REQUIRE( vec.z == 3.0f );
}

TEST_CASE( "DocumentValue OpaqueSet NegativeAndZeroValues", "[ae::Document][opaque]" )
{
	ae::Document doc( "test" );

	// Test negative int
	doc.OpaqueSet( -42 );
	REQUIRE( doc.OpaqueGet( 0 ) == -42 );

	// Test negative float
	doc.OpaqueSet( -3.14f );
	REQUIRE( doc.OpaqueGet( 0.0f ) == -3.14f );

	// Test negative double
	doc.OpaqueSet( -2.718 );
	REQUIRE( doc.OpaqueGet( 0.0 ) == -2.718 );

	// Test zero int
	doc.OpaqueSet( 0 );
	REQUIRE( doc.OpaqueGet( 999 ) == 0 );

	// Test zero float
	doc.OpaqueSet( 0.0f );
	REQUIRE( doc.OpaqueGet( 999.0f ) == 0.0f );

	// Test false
	doc.OpaqueSet( false );
	REQUIRE( doc.OpaqueGet( true ) == false );
}

TEST_CASE( "DocumentValue OpaqueSet Overwrite", "[ae::Document][opaque]" )
{
	ae::Document doc( "test" );

	// Set initial value
	doc.OpaqueSet( 42 );
	REQUIRE( doc.OpaqueGet( 0 ) == 42 );

	// Overwrite with same type
	doc.OpaqueSet( 100 );
	REQUIRE( doc.OpaqueGet( 0 ) == 100 );

	// Overwrite with different type
	doc.OpaqueSet( 3.14f );
	REQUIRE( doc.OpaqueGet( 0.0f ) == 3.14f );
	// Previous int value should not be retrievable
	REQUIRE( doc.OpaqueGet( 999 ) == 999 ); // Returns default

	// Overwrite with another type
	doc.OpaqueSet( true );
	REQUIRE( doc.OpaqueGet( false ) == true );
	// Previous float value should not be retrievable
	REQUIRE( doc.OpaqueGet( 999.0f ) == 999.0f ); // Returns default

	// Overwrite with struct
	struct Point { float x, y; };
	doc.OpaqueSet( Point{ 5.0f, 10.0f } );
	Point p = doc.OpaqueGet( Point{ 0.0f, 0.0f } );
	REQUIRE( p.x == 5.0f );
	REQUIRE( p.y == 10.0f );
	// Previous bool value should not be retrievable
	REQUIRE( doc.OpaqueGet( false ) == false ); // Returns default
}

TEST_CASE( "DocumentValue StringGet TypeAssert", "[ae::Document][throws]" )
{
	ae::Document doc( "test" );

	// StringGet on None type should assert
	REQUIRE_THROWS( doc.StringGet() );

	// StringGet on Opaque type should assert
	doc.OpaqueSet( 42 );
	REQUIRE_THROWS( doc.StringGet() );

	// StringGet on Array type should assert
	doc.ArrayInitialize( 4 );
	REQUIRE_THROWS( doc.StringGet() );

	// StringGet on Map type should assert
	doc.ObjectInitialize( 4 );
	REQUIRE_THROWS( doc.StringGet() );

	// StringGet should work on String type
	doc.StringSet( "test" );
	REQUIRE( doc.StringGet() == std::string( "test" ) );
}

TEST_CASE( "DocumentValue OpaqueGet TypeAssert", "[ae::Document][throws]" )
{
	ae::Document doc( "test" );

	// OpaqueGet on None type should assert
	REQUIRE_THROWS( doc.OpaqueGet( 0 ) );

	// OpaqueGet on String type should assert
	doc.StringSet( "test" );
	REQUIRE_THROWS( doc.OpaqueGet( 0 ) );

	// OpaqueGet on Array type should assert
	doc.ArrayInitialize( 4 );
	REQUIRE_THROWS( doc.OpaqueGet( 0 ) );

	// OpaqueGet on Map type should assert
	doc.ObjectInitialize( 4 );
	REQUIRE_THROWS( doc.OpaqueGet( 0 ) );

	// OpaqueGet should work on Opaque type
	doc.OpaqueSet( 42 );
	REQUIRE( doc.OpaqueGet( 0 ) == 42 );
}

TEST_CASE( "DocumentValue ArrayOperations TypeAssert", "[ae::Document][throws]" )
{
	ae::Document doc( "test" );

	// Array operations on None type should assert
	REQUIRE_THROWS( doc.ArrayLength() );
	REQUIRE_THROWS( doc.ArrayGet( 0 ) );
	REQUIRE_THROWS( doc.ArrayAppend() );
	REQUIRE_THROWS( doc.ArrayInsert( 0 ) );
	REQUIRE_THROWS( doc.ArrayRemove( 0 ) );

	// Array operations on String type should assert
	doc.StringSet( "test" );
	REQUIRE_THROWS( doc.ArrayLength() );
	REQUIRE_THROWS( doc.ArrayGet( 0 ) );
	REQUIRE_THROWS( doc.ArrayAppend() );
	REQUIRE_THROWS( doc.ArrayInsert( 0 ) );
	REQUIRE_THROWS( doc.ArrayRemove( 0 ) );

	// Array operations on Opaque type should assert
	doc.OpaqueSet( 42 );
	REQUIRE_THROWS( doc.ArrayLength() );
	REQUIRE_THROWS( doc.ArrayGet( 0 ) );
	REQUIRE_THROWS( doc.ArrayAppend() );
	REQUIRE_THROWS( doc.ArrayInsert( 0 ) );
	REQUIRE_THROWS( doc.ArrayRemove( 0 ) );

	// Array operations on Map type should assert
	doc.ObjectInitialize( 4 );
	REQUIRE_THROWS( doc.ArrayLength() );
	REQUIRE_THROWS( doc.ArrayGet( 0 ) );
	REQUIRE_THROWS( doc.ArrayAppend() );
	REQUIRE_THROWS( doc.ArrayInsert( 0 ) );
	REQUIRE_THROWS( doc.ArrayRemove( 0 ) );

	// Array operations should work on Array type
	doc.ArrayInitialize( 4 );
	REQUIRE( doc.ArrayLength() == 0 );
	doc.ArrayAppend().OpaqueSet( 1 );
	REQUIRE( doc.ArrayLength() == 1 );
	REQUIRE( doc.ArrayGet( 0 ).OpaqueGet( 0 ) == 1 );
}

TEST_CASE( "DocumentValue ArrayRemove BoundsAssert", "[ae::Document][throws]" )
{
	ae::Document doc( "test" );
	doc.ArrayInitialize( 4 );

	// Remove on empty array should assert
	REQUIRE_THROWS( doc.ArrayRemove( 0 ) );

	// Add element
	doc.ArrayAppend().OpaqueSet( 1 );
	REQUIRE( doc.ArrayLength() == 1 );

	// Remove out of bounds should assert
	REQUIRE_THROWS( doc.ArrayRemove( 1 ) );
	REQUIRE_THROWS( doc.ArrayRemove( 100 ) );

	// Remove valid index should work
	doc.ArrayRemove( 0 );
	REQUIRE( doc.ArrayLength() == 0 );
}

TEST_CASE( "DocumentValue ObjectOperations TypeAssert", "[ae::Document][throws]" )
{
	ae::Document doc( "test" );

	// Object operations on None type should assert
	REQUIRE_THROWS( doc.ObjectLength() );
	REQUIRE_THROWS( doc.ObjectGetKey( 0 ) );
	REQUIRE_THROWS( doc.ObjectGetValue( 0 ) );
	REQUIRE_THROWS( doc.ObjectTryGet( "key" ) );
	REQUIRE_THROWS( doc.ObjectRemove( "key" ) );

	// Object operations on String type should assert
	doc.StringSet( "test" );
	REQUIRE_THROWS( doc.ObjectLength() );
	REQUIRE_THROWS( doc.ObjectGetKey( 0 ) );
	REQUIRE_THROWS( doc.ObjectGetValue( 0 ) );
	REQUIRE_THROWS( doc.ObjectTryGet( "key" ) );
	REQUIRE_THROWS( doc.ObjectRemove( "key" ) );

	// Object operations on Opaque type should assert
	doc.OpaqueSet( 42 );
	REQUIRE_THROWS( doc.ObjectLength() );
	REQUIRE_THROWS( doc.ObjectGetKey( 0 ) );
	REQUIRE_THROWS( doc.ObjectGetValue( 0 ) );
	REQUIRE_THROWS( doc.ObjectTryGet( "key" ) );
	REQUIRE_THROWS( doc.ObjectRemove( "key" ) );

	// Object operations on Array type should assert
	doc.ArrayInitialize( 4 );
	REQUIRE_THROWS( doc.ObjectLength() );
	REQUIRE_THROWS( doc.ObjectGetKey( 0 ) );
	REQUIRE_THROWS( doc.ObjectGetValue( 0 ) );
	REQUIRE_THROWS( doc.ObjectTryGet( "key" ) );
	REQUIRE_THROWS( doc.ObjectRemove( "key" ) );

	// Object operations should work on Object type
	doc.ObjectInitialize( 4 );
	REQUIRE( doc.ObjectLength() == 0 );
	doc.ObjectSet( "test" ).OpaqueSet( 1 );
	REQUIRE( doc.ObjectLength() == 1 );
	REQUIRE( doc.ObjectTryGet( "test" ) != nullptr );
	REQUIRE( doc.ObjectTryGet( "test" )->OpaqueGet( 0 ) == 1 );
}

TEST_CASE( "DocumentValue ConstOperations TypeAssert", "[ae::Document][throws]" )
{
	ae::Document doc( "test" );
	doc.ArrayInitialize( 4 );
	doc.ArrayAppend().OpaqueSet( 42 );

	const ae::DocumentValue& constDoc = doc;

	// Const ArrayGet should work
	REQUIRE( constDoc.ArrayLength() == 1 );
	REQUIRE( constDoc.ArrayGet( 0 ).OpaqueGet( 0 ) == 42 );

	// Change to object
	doc.ObjectInitialize( 4 );
	doc.ObjectSet( "key" ).OpaqueSet( 100 );

	// Const ObjectTryGet should work
	REQUIRE( constDoc.ObjectLength() == 1 );
	const ae::DocumentValue* value = constDoc.ObjectTryGet( "key" );
	REQUIRE( value != nullptr );
	REQUIRE( value->OpaqueGet( 0 ) == 100 );

	// Const ObjectGetKey and MapGetValue should work
	REQUIRE( constDoc.ObjectGetKey( 0 ) == std::string( "key" ) );
	REQUIRE( constDoc.ObjectGetValue( 0 ).OpaqueGet( 0 ) == 100 );

	// Change to String
	doc.StringSet( "test_string" );
	REQUIRE( constDoc.StringGet() == std::string( "test_string" ) );

	// Change to Opaque
	doc.OpaqueSet( 3.14f );
	REQUIRE( constDoc.OpaqueGet( 0.0f ) == 3.14f );
}

TEST_CASE( "DocumentValue EdgeCases", "[ae::Document]" )
{
	ae::Document doc( "test" );

	// Test empty string
	doc.StringSet( "" );
	REQUIRE( doc.IsString() );
	REQUIRE( doc.StringGet() == std::string( "" ) );

	// Test MapSet with empty key
	doc.ObjectInitialize( 4 );
	doc.ObjectSet( "" ).OpaqueSet( 42 );
	REQUIRE( doc.ObjectLength() == 1 );
	REQUIRE( doc.ObjectTryGet( "" ) != nullptr );
	REQUIRE( doc.ObjectTryGet( "" )->OpaqueGet( 0 ) == 42 );

	// Test MapRemove with empty string
	REQUIRE( doc.ObjectRemove( "" ) == true );
	REQUIRE( doc.ObjectLength() == 0 );
}

TEST_CASE( "DocumentValue StringSet LongStrings", "[ae::Document]" )
{
	ae::Document doc( "test" );

	// Test long string (1000 characters)
	std::string longStr( 1000, 'x' );
	doc.StringSet( longStr.c_str() );
	REQUIRE( doc.IsString() );
	REQUIRE( doc.StringGet() == longStr );

	// Test very long string (10000 characters)
	std::string veryLongStr( 10000, 'y' );
	doc.StringSet( veryLongStr.c_str() );
	REQUIRE( doc.IsString() );
	REQUIRE( doc.StringGet() == veryLongStr );

	// Test string with special characters
	doc.StringSet( "Hello\nWorld\t!\r\n\"Quotes\" and 'apostrophes'" );
	REQUIRE( doc.StringGet() == std::string( "Hello\nWorld\t!\r\n\"Quotes\" and 'apostrophes'" ) );

	// Test unicode string (if supported)
	doc.StringSet( "Unicode: 你好世界 🌍 Ñoño" );
	REQUIRE( doc.StringGet() == std::string( "Unicode: 你好世界 🌍 Ñoño" ) );
}

TEST_CASE( "DocumentValue MapSet LongKeys", "[ae::Document]" )
{
	ae::Document doc( "test" );
	doc.ObjectInitialize( 4 );

	// Test long key (1000 characters)
	std::string longKey( 1000, 'k' );
	doc.ObjectSet( longKey.c_str() ).OpaqueSet( 42 );
	REQUIRE( doc.ObjectLength() == 1 );
	REQUIRE( doc.ObjectTryGet( longKey.c_str() ) != nullptr );
	REQUIRE( doc.ObjectTryGet( longKey.c_str() )->OpaqueGet( 0 ) == 42 );

	// Test key with special characters
	doc.ObjectSet( "key\nwith\ttabs\rand\nnewlines" ).OpaqueSet( 100 );
	REQUIRE( doc.ObjectTryGet( "key\nwith\ttabs\rand\nnewlines" ) != nullptr );
	REQUIRE( doc.ObjectTryGet( "key\nwith\ttabs\rand\nnewlines" )->OpaqueGet( 0 ) == 100 );
}

//------------------------------------------------------------------------------
// ae::Document::AddUndoGroupAction tests
//------------------------------------------------------------------------------
TEST_CASE( "Document AddUndoGroupAction basic", "[ae::Document][AddUndoGroupAction]" )
{
	ae::Document doc( "test" );
	int externalState = 0;
	
	// Add document change with custom action
	doc.StringSet( "first" );
	doc.AddUndoGroupAction( "increment external state",
		[&externalState]() { externalState--; }, // undo
		[&externalState]() { externalState++; }  // redo
	);
	doc.EndUndoGroup();
	
	REQUIRE( doc.StringGet() == std::string( "first" ) );
	REQUIRE( externalState == 0 );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );
	
	// Undo should call the undo callback
	REQUIRE( doc.Undo() );
	REQUIRE( externalState == -1 );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 1 );
	
	// Redo should call the redo callback
	REQUIRE( doc.Redo() );
	REQUIRE( externalState == 0 );
	REQUIRE( doc.StringGet() == std::string( "first" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );
}

TEST_CASE( "Document AddUndoGroupAction execute redo immediately", "[ae::Document][AddUndoGroupAction]" )
{
	ae::Document doc( "test" );
	int counter = 0;
	
	// The returned redo callback can be executed immediately
	doc.StringSet( "value" );
	const ae::DocumentCallback& redo = doc.AddUndoGroupAction( "increment",
		[&counter]() { counter--; },
		[&counter]() { counter++; }
	);
	redo(); // Execute redo immediately
	doc.EndUndoGroup();
	
	REQUIRE( counter == 1 );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	
	// Undo
	REQUIRE( doc.Undo() );
	REQUIRE( counter == 0 );
	
	// Redo
	REQUIRE( doc.Redo() );
	REQUIRE( counter == 1 );
}

TEST_CASE( "Document AddUndoGroupAction syncing external graphics resource", "[ae::Document][AddUndoGroupAction]" )
{
	ae::Document doc( "test" );
	
	// Simulate external graphics resource (e.g., texture handle)
	struct GraphicsResource
	{
		int textureId = -1;
		ae::Vec3 color = ae::Vec3( 0.0f );
		bool needsUpdate = false;
	};
	GraphicsResource resource;
	
	// Create a color value in the document
	doc.ObjectInitialize( 4 );
	auto* colorR = &doc.ObjectSet( "r" );
	auto* colorG = &doc.ObjectSet( "g" );
	auto* colorB = &doc.ObjectSet( "b" );
	colorR->NumberSet( 1.0 );
	colorG->NumberSet( 0.0 );
	colorB->NumberSet( 0.0 );
	
	// Add action to sync graphics resource with document
	doc.AddUndoGroupAction( "create red texture",
		[&resource]() {
			// Undo: destroy the texture
			resource.textureId = -1;
			resource.color = ae::Vec3( 0.0f );
			resource.needsUpdate = false;
		},
		[&resource, colorR, colorG, colorB]() {
			// Redo: create/update texture with current color
			resource.textureId = 42;
			resource.color = ae::Vec3(
				(float)colorR->NumberGet< double >(),
				(float)colorG->NumberGet< double >(),
				(float)colorB->NumberGet< double >()
			);
			resource.needsUpdate = true;
		}
	)(); // Execute redo immediately
	
	doc.EndUndoGroup();
	
	REQUIRE( resource.textureId == 42 );
	REQUIRE( resource.color == ae::Vec3( 1.0f, 0.0f, 0.0f ) );
	REQUIRE( resource.needsUpdate == true );
	
	// Update color to green
	resource.needsUpdate = false;
	colorR->NumberSet( 0.0 );
	colorG->NumberSet( 1.0 );
	doc.AddUndoGroupAction( "update to green",
		[&resource, oldR = 1.0, oldG = 0.0, oldB = 0.0]() {
			resource.color = ae::Vec3( (float)oldR, (float)oldG, (float)oldB );
			resource.needsUpdate = true;
		},
		[&resource, colorR, colorG, colorB]() {
			resource.color = ae::Vec3(
				(float)colorR->NumberGet< double >(),
				(float)colorG->NumberGet< double >(),
				(float)colorB->NumberGet< double >()
			);
			resource.needsUpdate = true;
		}
	)();
	doc.EndUndoGroup();
	
	REQUIRE( resource.color == ae::Vec3( 0.0f, 1.0f, 0.0f ) );
	REQUIRE( resource.needsUpdate == true );
	
	// Undo should restore red color
	REQUIRE( doc.Undo() );
	REQUIRE( resource.color == ae::Vec3( 1.0f, 0.0f, 0.0f ) );
	REQUIRE( resource.needsUpdate == true );
	
	// Redo should restore green color
	resource.needsUpdate = false;
	REQUIRE( doc.Redo() );
	REQUIRE( resource.color == ae::Vec3( 0.0f, 1.0f, 0.0f ) );
	REQUIRE( resource.needsUpdate == true );
	
	// Undo back to beginning
	REQUIRE( doc.Undo() );
	REQUIRE( doc.Undo() );
	REQUIRE( resource.textureId == -1 );
	REQUIRE( resource.color == ae::Vec3( 0.0f, 0.0f, 0.0f ) );
}

TEST_CASE( "Document AddUndoGroupAction multiple actions in group", "[ae::Document][AddUndoGroupAction]" )
{
	ae::Document doc( "test" );
	int stateA = 0;
	int stateB = 0;
	int stateC = 0;
	
	// Multiple custom actions in a single undo group
	doc.StringSet( "test" );
	doc.AddUndoGroupAction( "action A",
		[&stateA]() { stateA--; },
		[&stateA]() { stateA++; }
	)();
	doc.AddUndoGroupAction( "action B",
		[&stateB]() { stateB -= 10; },
		[&stateB]() { stateB += 10; }
	)();
	doc.AddUndoGroupAction( "action C",
		[&stateC]() { stateC--; },
		[&stateC]() { stateC++; }
	)();
	doc.EndUndoGroup();
	
	REQUIRE( stateA == 1 );
	REQUIRE( stateB == 10 );
	REQUIRE( stateC == 1 );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	
	// Single undo should reverse all actions in reverse order
	REQUIRE( doc.Undo() );
	REQUIRE( stateA == 0 );
	REQUIRE( stateB == 0 );
	REQUIRE( stateC == 0 );
	
	// Single redo should replay all actions
	REQUIRE( doc.Redo() );
	REQUIRE( stateA == 1 );
	REQUIRE( stateB == 10 );
	REQUIRE( stateC == 1 );
}

TEST_CASE( "Document AddUndoGroupAction interleaved with document ops", "[ae::Document][AddUndoGroupAction]" )
{
	ae::Document doc( "test" );
	ae::Array< int, 8 > externalArray;
	
	// Interleave document operations with custom actions
	doc.ArrayInitialize( 4 );
	doc.ArrayAppend().OpaqueSet( 100 );
	
	// Sync external array with document
	doc.AddUndoGroupAction( "sync external",
		[&externalArray]() {
			if ( externalArray.Length() > 0 )
			{
				externalArray.Remove( externalArray.Length() - 1 );
			}
		},
		[&externalArray]() {
			externalArray.Append( 100 );
		}
	)();
	
	doc.ArrayAppend().OpaqueSet( 200 );
	doc.AddUndoGroupAction( "sync external 2",
		[&externalArray]() {
			if ( externalArray.Length() > 0 )
			{
				externalArray.Remove( externalArray.Length() - 1 );
			}
		},
		[&externalArray]() {
			externalArray.Append( 200 );
		}
	)();
	
	doc.EndUndoGroup();
	
	REQUIRE( doc.ArrayLength() == 2 );
	REQUIRE( doc.ArrayGet( 0 ).OpaqueGet( 0 ) == 100 );
	REQUIRE( doc.ArrayGet( 1 ).OpaqueGet( 0 ) == 200 );
	REQUIRE( externalArray.Length() == 2 );
	REQUIRE( externalArray[ 0 ] == 100 );
	REQUIRE( externalArray[ 1 ] == 200 );
	
	// Undo should restore both document and external state
	REQUIRE( doc.Undo() );
	REQUIRE( doc.IsNull() ); // Back to initial null state
	REQUIRE( externalArray.Length() == 0 );
	
	// Redo should restore both
	REQUIRE( doc.Redo() );
	REQUIRE( doc.IsArray() );
	REQUIRE( doc.ArrayLength() == 2 );
	REQUIRE( externalArray.Length() == 2 );
	REQUIRE( externalArray[ 0 ] == 100 );
	REQUIRE( externalArray[ 1 ] == 200 );
}

TEST_CASE( "Document AddUndoGroupAction capturing DocumentValue pointers", "[ae::Document][AddUndoGroupAction]" )
{
	ae::Document doc( "test" );
	
	// Create object with nested values
	doc.ObjectInitialize( 4 );
	auto* posX = &doc.ObjectSet( "x" );
	auto* posY = &doc.ObjectSet( "y" );
	posX->NumberSet( 10.0 );
	posY->NumberSet( 20.0 );
	
	// External state that mirrors document
	struct Position { double x, y; };
	Position externalPos = { 0.0, 0.0 };
	
	// Safe to capture DocumentValue pointers (documented behavior)
	doc.AddUndoGroupAction( "sync position",
		[&externalPos]() {
			externalPos = { 0.0, 0.0 };
		},
		[&externalPos, posX, posY]() {
			// Safe because action and queue lifetimes match
			externalPos.x = posX->NumberGet< double >();
			externalPos.y = posY->NumberGet< double >();
		}
	)();
	doc.EndUndoGroup();
	
	REQUIRE( externalPos.x == 10.0 );
	REQUIRE( externalPos.y == 20.0 );
	
	// Modify position
	posX->NumberSet( 30.0 );
	posY->NumberSet( 40.0 );
	doc.AddUndoGroupAction( "sync position update",
		[&externalPos]() {
			externalPos = { 10.0, 20.0 };
		},
		[&externalPos, posX, posY]() {
			externalPos.x = posX->NumberGet< double >();
			externalPos.y = posY->NumberGet< double >();
		}
	)();
	doc.EndUndoGroup();
	
	REQUIRE( externalPos.x == 30.0 );
	REQUIRE( externalPos.y == 40.0 );
	
	// Undo restores previous values
	REQUIRE( doc.Undo() );
	REQUIRE( posX->NumberGet< double >() == 10.0 );
	REQUIRE( posY->NumberGet< double >() == 20.0 );
	REQUIRE( externalPos.x == 10.0 );
	REQUIRE( externalPos.y == 20.0 );
	
	// Redo applies new values
	REQUIRE( doc.Redo() );
	REQUIRE( posX->NumberGet< double >() == 30.0 );
	REQUIRE( posY->NumberGet< double >() == 40.0 );
	REQUIRE( externalPos.x == 30.0 );
	REQUIRE( externalPos.y == 40.0 );
}

TEST_CASE( "Document AddUndoGroupAction collision mesh sync", "[ae::Document][AddUndoGroupAction]" )
{
	ae::Document doc( "test" );
	
	// Simulate collision mesh external to document
	struct CollisionMesh
	{
		ae::Array< ae::Vec3, 16 > vertices;
		bool isValid = false;
		
		void build( const ae::Array< ae::Vec3, 16 >& verts )
		{
			vertices = verts;
			isValid = true;
		}
		
		void destroy()
		{
			vertices.Clear();
			isValid = false;
		}
	};
	CollisionMesh collision;
	
	// Create vertices in document
	doc.ArrayInitialize( 4 );
	auto* v0 = &doc.ArrayAppend();
	auto* v1 = &doc.ArrayAppend();
	auto* v2 = &doc.ArrayAppend();
	
	v0->ArrayInitialize( 4 );
	v0->ArrayAppend().NumberSet( 0.0 );
	v0->ArrayAppend().NumberSet( 0.0 );
	v0->ArrayAppend().NumberSet( 0.0 );
	
	v1->ArrayInitialize( 4 );
	v1->ArrayAppend().NumberSet( 1.0 );
	v1->ArrayAppend().NumberSet( 0.0 );
	v1->ArrayAppend().NumberSet( 0.0 );
	
	v2->ArrayInitialize( 4 );
	v2->ArrayAppend().NumberSet( 0.0 );
	v2->ArrayAppend().NumberSet( 1.0 );
	v2->ArrayAppend().NumberSet( 0.0 );
	
	// Build collision mesh from document data
	doc.AddUndoGroupAction( "build collision",
		[&collision]() {
			collision.destroy();
		},
		[&collision, v0, v1, v2]() {
			ae::Array< ae::Vec3, 16 > verts;
			verts.Append( ae::Vec3(
				(float)v0->ArrayGet( 0 ).NumberGet< double >(),
				(float)v0->ArrayGet( 1 ).NumberGet< double >(),
				(float)v0->ArrayGet( 2 ).NumberGet< double >()
			) );
			verts.Append( ae::Vec3(
				(float)v1->ArrayGet( 0 ).NumberGet< double >(),
				(float)v1->ArrayGet( 1 ).NumberGet< double >(),
				(float)v1->ArrayGet( 2 ).NumberGet< double >()
			) );
			verts.Append( ae::Vec3(
				(float)v2->ArrayGet( 0 ).NumberGet< double >(),
				(float)v2->ArrayGet( 1 ).NumberGet< double >(),
				(float)v2->ArrayGet( 2 ).NumberGet< double >()
			) );
			collision.build( verts );
		}
	)();
	doc.EndUndoGroup();
	
	REQUIRE( collision.isValid == true );
	REQUIRE( collision.vertices.Length() == 3 );
	REQUIRE( collision.vertices[ 0 ] == ae::Vec3( 0.0f, 0.0f, 0.0f ) );
	REQUIRE( collision.vertices[ 1 ] == ae::Vec3( 1.0f, 0.0f, 0.0f ) );
	REQUIRE( collision.vertices[ 2 ] == ae::Vec3( 0.0f, 1.0f, 0.0f ) );
	
	// Undo should destroy collision mesh
	REQUIRE( doc.Undo() );
	REQUIRE( collision.isValid == false );
	REQUIRE( collision.vertices.Length() == 0 );
	
	// Redo should rebuild it
	REQUIRE( doc.Redo() );
	REQUIRE( collision.isValid == true );
	REQUIRE( collision.vertices.Length() == 3 );
}

TEST_CASE( "Document AddUndoGroupAction with ClearUndo", "[ae::Document][AddUndoGroupAction]" )
{
	ae::Document doc( "test" );
	int externalCounter = 0;
	
	// Create some actions
	doc.StringSet( "first" );
	doc.AddUndoGroupAction( "action 1",
		[&externalCounter]() { externalCounter--; },
		[&externalCounter]() { externalCounter++; }
	)();
	doc.EndUndoGroup();
	
	doc.StringSet( "second" );
	doc.AddUndoGroupAction( "action 2",
		[&externalCounter]() { externalCounter--; },
		[&externalCounter]() { externalCounter++; }
	)();
	doc.EndUndoGroup();
	
	REQUIRE( externalCounter == 2 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	
	// Clear undo - should not affect current state
	doc.ClearUndo();
	REQUIRE( doc.StringGet() == std::string( "second" ) );
	REQUIRE( externalCounter == 2 );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 0 );
	
	// Cannot undo after clear
	REQUIRE( doc.Undo() == false );
	REQUIRE( externalCounter == 2 );
}

TEST_CASE( "Document AddUndoGroupAction separate groups", "[ae::Document][AddUndoGroupAction]" )
{
	ae::Document doc( "test" );
	int stateA = 0;
	int stateB = 0;
	
	// First group
	doc.StringSet( "group1" );
	doc.AddUndoGroupAction( "group1 action",
		[&stateA]() { stateA--; },
		[&stateA]() { stateA++; }
	)();
	doc.EndUndoGroup();
	
	// Second group
	doc.StringSet( "group2" );
	doc.AddUndoGroupAction( "group2 action",
		[&stateB]() { stateB--; },
		[&stateB]() { stateB++; }
	)();
	doc.EndUndoGroup();
	
	REQUIRE( stateA == 1 );
	REQUIRE( stateB == 1 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	
	// Undo second group only
	REQUIRE( doc.Undo() );
	REQUIRE( doc.StringGet() == std::string( "group1" ) );
	REQUIRE( stateA == 1 );
	REQUIRE( stateB == 0 );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 1 );
	
	// Undo first group
	REQUIRE( doc.Undo() );
	REQUIRE( stateA == 0 );
	REQUIRE( stateB == 0 );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 2 );
	
	// Redo first group
	REQUIRE( doc.Redo() );
	REQUIRE( stateA == 1 );
	REQUIRE( stateB == 0 );
	
	// Redo second group
	REQUIRE( doc.Redo() );
	REQUIRE( doc.StringGet() == std::string( "group2" ) );
	REQUIRE( stateA == 1 );
	REQUIRE( stateB == 1 );
}

TEST_CASE( "Document AddUndoGroupAction state consistency", "[ae::Document][AddUndoGroupAction]" )
{
	ae::Document doc( "test" );
	
	// Test that custom actions are called in sequence with document operations
	int externalCounter = 0;
	
	// First operation
	doc.StringSet( "first" );
	doc.AddUndoGroupAction( "set to 10",
		[&externalCounter]() { externalCounter = 0; },
		[&externalCounter]() { externalCounter = 10; }
	)();
	doc.EndUndoGroup();
	REQUIRE( doc.StringGet() == std::string( "first" ) );
	REQUIRE( externalCounter == 10 );
	
	// Second operation
	doc.StringSet( "second" );
	doc.AddUndoGroupAction( "set to 20",
		[&externalCounter]() { externalCounter = 10; },
		[&externalCounter]() { externalCounter = 20; }
	)();
	doc.EndUndoGroup();
	REQUIRE( doc.StringGet() == std::string( "second" ) );
	REQUIRE( externalCounter == 20 );
	
	// Undo second - both document and external state should revert
	REQUIRE( doc.Undo() );
	REQUIRE( doc.StringGet() == std::string( "first" ) );
	REQUIRE( externalCounter == 10 );
	
	// Redo second - both should advance
	REQUIRE( doc.Redo() );
	REQUIRE( doc.StringGet() == std::string( "second" ) );
	REQUIRE( externalCounter == 20 );
}

TEST_CASE( "Document AddUndoGroupAction complex resource management", "[ae::Document][AddUndoGroupAction]" )
{
	ae::Document doc( "test" );
	
	// Simulate complex resource with dependencies
	struct ComplexResource
	{
		struct SubResource
		{
			int id = -1;
			ae::Str64 name;
		};
		
		ae::Map< ae::Str64, SubResource, 8 > resources;
		ae::Array< int, 8 > activeIds;
		
		void add( const char* name, int id )
		{
			SubResource sub;
			sub.id = id;
			sub.name = name;
			resources.Set( name, sub );
			activeIds.Append( id );
		}
		
		void remove( const char* name )
		{
			if ( const SubResource* sub = resources.TryGet( name ) )
			{
				for ( uint32_t i = 0; i < activeIds.Length(); i++ )
				{
					if ( activeIds[ i ] == sub->id )
					{
						activeIds.Remove( i );
						break;
					}
				}
				resources.Remove( name );
			}
		}
		
		void clear()
		{
			resources.Clear();
			activeIds.Clear();
		}
	};
	ComplexResource resource;
	
	// Build document structure
	doc.ObjectInitialize( 4 );
	auto* objects = &doc.ObjectSet( "objects" );
	objects->ArrayInitialize( 4 );
	doc.EndUndoGroup(); // Establish baseline state
	
	// Add first object
	auto* obj1 = &objects->ArrayAppend();
	obj1->ObjectInitialize( 4 );
	obj1->ObjectSet( "name" ).StringSet( "player" );
	obj1->ObjectSet( "id" ).OpaqueSet( 100 );
	
	doc.AddUndoGroupAction( "add player resource",
		[&resource]() {
			resource.remove( "player" );
		},
		[&resource, obj1]() {
			const char* name = obj1->ObjectTryGet( "name" )->StringGet();
			int id = obj1->ObjectTryGet( "id" )->OpaqueGet( 0 );
			resource.add( name, id );
		}
	)();
	doc.EndUndoGroup();
	
	REQUIRE( resource.resources.Length() == 1 );
	REQUIRE( resource.activeIds.Length() == 1 );
	REQUIRE( resource.activeIds[ 0 ] == 100 );
	
	// Add second object
	auto* obj2 = &objects->ArrayAppend();
	obj2->ObjectInitialize( 4 );
	obj2->ObjectSet( "name" ).StringSet( "enemy" );
	obj2->ObjectSet( "id" ).OpaqueSet( 200 );
	
	doc.AddUndoGroupAction( "add enemy resource",
		[&resource]() {
			resource.remove( "enemy" );
		},
		[&resource, obj2]() {
			const char* name = obj2->ObjectTryGet( "name" )->StringGet();
			int id = obj2->ObjectTryGet( "id" )->OpaqueGet( 0 );
			resource.add( name, id );
		}
	)();
	doc.EndUndoGroup();
	
	REQUIRE( resource.resources.Length() == 2 );
	REQUIRE( resource.activeIds.Length() == 2 );
	REQUIRE( resource.resources.Get( "player" ).id == 100 );
	REQUIRE( resource.resources.Get( "enemy" ).id == 200 );
	
	// Undo second object
	REQUIRE( doc.Undo() );
	REQUIRE( objects->ArrayLength() == 1 );
	REQUIRE( resource.resources.Length() == 1 );
	REQUIRE( resource.activeIds.Length() == 1 );
	REQUIRE( resource.resources.Get( "player" ).id == 100 );
	
	// Undo first object
	REQUIRE( doc.Undo() );
	REQUIRE( objects->ArrayLength() == 0 );
	REQUIRE( resource.resources.Length() == 0 );
	REQUIRE( resource.activeIds.Length() == 0 );
	
	// Redo both
	REQUIRE( doc.Redo() );
	REQUIRE( doc.Redo() );
	REQUIRE( objects->ArrayLength() == 2 );
	REQUIRE( resource.resources.Length() == 2 );
	REQUIRE( resource.activeIds.Length() == 2 );
}
