//------------------------------------------------------------------------------
// Document.cpp
// Copyright (c) John Hughes on 6/15/24. All rights reserved.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aether.h"
#include <catch2/catch_test_macros.hpp>

namespace ae
{

//------------------------------------------------------------------------------
// ae::DocumentValueType
//------------------------------------------------------------------------------
enum class DocumentValueType
{
	None,
	Value,
	Array,
	Map
};

//------------------------------------------------------------------------------
// ae::DocumentValue
//------------------------------------------------------------------------------
class DocumentValue
{
public:
	DocumentValue( class Document* document, const ae::Tag& tag );
	virtual ~DocumentValue() {}

	//--------------------------------------------------------------------------
	// Type functions
	//--------------------------------------------------------------------------
	DocumentValue& Initialize( DocumentValueType type );
	DocumentValueType GetType() const { return m_type; }
	bool IsValue() const;
	bool IsArray() const;
	bool IsMap() const;

	//--------------------------------------------------------------------------
	// Basic value
	//--------------------------------------------------------------------------
	//! Sets the value to a basic string type. Note that repeated calls to
	//! ValueSet will be coalesced into a single undo operation. This
	//! behavior can be circumvented by ending the current undo group between
	//! calls.
	//! \param value The string value to set. Can be nullptr or empty string.
	void ValueSet( const char* value );
	//! Returns the basic string value. Must only be called when IsValue() returns true.
	//! \return The string value as a null-terminated C string.
	const char* ValueGet() const;

	//--------------------------------------------------------------------------
	// Array manipulation
	//--------------------------------------------------------------------------
	//! Converts this value to an array type, clearing any existing data.
	//! \param reserveLength Optional hint for initial capacity to avoid reallocations.
	void ArrayInitialize( uint32_t reserveLength = 0 );
	//! Inserts a new DocumentValue at the specified index in the array.
	//! \param index The position to insert at. Must be <= ArrayLength().
	//! \return Reference to the newly created DocumentValue.
	DocumentValue& ArrayInsert( uint32_t index );
	//! Appends a new DocumentValue to the end of the array.
	//! \return Reference to the newly created DocumentValue.
	DocumentValue& ArrayAppend();
	//! Removes the DocumentValue at the specified index from the array.
	//! \param index The index to remove. Must be < ArrayLength().
	void ArrayRemove( uint32_t index );
	//--------------------------------------------------------------------------
	// Array iteration
	//--------------------------------------------------------------------------
	//! Returns the number of elements in the array.
	//! \return The array length. Only valid when IsArray() returns true.
	uint32_t ArrayLength() const;
	//! Gets a mutable reference to the DocumentValue at the specified index.
	//! \param index The index to access. Must be < ArrayLength().
	//! \return Mutable reference to the DocumentValue at the specified index.
	DocumentValue& ArrayGet( uint32_t index );
	//! Gets a const reference to the DocumentValue at the specified index.
	//! \param index The index to access. Must be < ArrayLength().
	//! \return Const reference to the DocumentValue at the specified index.
	const DocumentValue& ArrayGet( uint32_t index ) const;

	//--------------------------------------------------------------------------
	// Map manipulation
	//--------------------------------------------------------------------------
	//! Converts this value to a map type, clearing any existing data.
	//! \param reserveLength Optional hint for initial capacity to avoid reallocations.
	void MapInitialize( uint32_t reserveLength = 0 );
	//! Gets a mutable reference to the DocumentValue for the given key,
	//! creating it if it doesn't exist.
	//! \param key The string key to look up. Must not be nullptr.
	//! \return Mutable reference to the DocumentValue associated with the key.
	DocumentValue& MapGet( const char* key );
	//! Removes the key-value pair from the map if it exists.
	//! \param key The string key to remove. Must not be nullptr.
	//! \return True if the key was found and removed, false if the key didn't exist.
	bool MapRemove( const char* key );
	//--------------------------------------------------------------------------
	// Map queries
	//--------------------------------------------------------------------------
	//! Attempts to get a mutable pointer to the DocumentValue for the given key.
	//! \param key The string key to look up. Must not be nullptr.
	//! \return Pointer to the DocumentValue if found, nullptr if the key doesn't exist.
	DocumentValue* MapTryGet( const char* key );
	//! Attempts to get a const pointer to the DocumentValue for the given key.
	//! \param key The string key to look up. Must not be nullptr.
	//! \return Const pointer to the DocumentValue if found, nullptr if the key doesn't exist.
	const DocumentValue* MapTryGet( const char* key ) const;
	//--------------------------------------------------------------------------
	// Map iteration
	//--------------------------------------------------------------------------
	//! Returns the number of key-value pairs in the map.
	//! \return The map size. Only valid when IsMap() returns true.
	uint32_t MapLength() const;
	//! Gets the key string at the specified iteration index.
	//! \param index The iteration index. Must be < MapLength().
	//! \return The key as a null-terminated C string.
	const char* MapGetKey( uint32_t index ) const;
	//! Gets a mutable reference to the DocumentValue at the specified iteration index.
	//! \param index The iteration index. Must be < MapLength().
	//! \return Mutable reference to the DocumentValue at the specified index.
	DocumentValue& MapGetValue( uint32_t index );
	//! Gets a const reference to the DocumentValue at the specified iteration index.
	//! \param index The iteration index. Must be < MapLength().
	//! \return Const reference to the DocumentValue at the specified index.
	const DocumentValue& MapGetValue( uint32_t index ) const;

protected:
	friend class Document;
	enum class UndoOpType
	{
		SetType,
		ValueSet,
		ArrayInsert,
		ArrayRemove,
		MapSet,
		MapRemove
	};
	struct UndoOp
	{
		UndoOpType type;
		class DocumentValue* target;
		uint32_t index = 0;
		std::string key;
		std::string oldValue;
		DocumentValueType oldType = DocumentValueType::None;
		class DocumentValue* oldChild = nullptr;
	};
	DocumentValue() = delete;
	DocumentValue( const DocumentValue& ) = delete;
	DocumentValue& operator=( const DocumentValue& ) = delete;
	Document* m_document = nullptr;
	DocumentValueType m_type = DocumentValueType::None;
	int32_t m_refCount = 0; // References from undo stack only
	std::string m_basic;
	ae::Array< DocumentValue* > m_array;
	ae::Map< std::string, DocumentValue* > m_map;
};

//------------------------------------------------------------------------------
// ae::Document
//------------------------------------------------------------------------------
class Document : public DocumentValue
{
public:
	Document( const ae::Tag& tag );
	~Document();

	void EndUndoGroup();
	void ClearUndo();
	bool Undo();
	bool Redo();
	uint32_t GetUndoStackSize() const { return m_undoStack.Length() + ( m_currentGroup.Length() ? 1 : 0 ); }
	uint32_t GetRedoStackSize() const { return m_redoStack.Length(); }

private:
	friend class DocumentValue;
	ae::Array< UndoOp > m_ReverseOps( const ae::Array< UndoOp >& ops );
	void m_PushOp( const UndoOp& op );
	void m_AddRef( DocumentValue* value );
	void m_RemoveRef( DocumentValue* value );
	void m_ValidateState( const ae::Array< UndoOp >& operations );
	const ae::Tag m_tag;
	ae::ObjectPool< DocumentValue, 64, true > m_values;
	ae::Array< ae::Array< UndoOp > > m_undoStack;
	ae::Array< ae::Array< UndoOp > > m_redoStack;
	ae::Array< UndoOp > m_currentGroup;
};

//------------------------------------------------------------------------------
// ae::DocumentValue member functions
//------------------------------------------------------------------------------
DocumentValue::DocumentValue( class Document* document, const ae::Tag& tag ) : m_document( document ), m_array( tag ), m_map( tag ) {}

// Type queries
bool DocumentValue::IsValue() const { return ( m_type == DocumentValueType::Value ); }
bool DocumentValue::IsArray() const { return ( m_type == DocumentValueType::Array ); }
bool DocumentValue::IsMap() const { return ( m_type == DocumentValueType::Map ); }

// Basic
void DocumentValue::ValueSet( const char* value )
{
	if( !IsValue() )
	{
		Initialize( DocumentValueType::Value );
	}
	if( m_basic != value )
	{
		// Try to combine with previous ValueSet operation in current group
		bool combined = false;
		if( m_document->m_currentGroup.Length() > 0 )
		{
			UndoOp& lastOp = m_document->m_currentGroup[ m_document->m_currentGroup.Length() - 1 ];
			if( lastOp.type == UndoOpType::ValueSet && lastOp.target == this )
			{
				// Keep the original old value, just update current value
				combined = true;
			}
		}
		if( !combined )
		{
			UndoOp op;
			op.type = UndoOpType::ValueSet;
			op.target = this;
			op.oldValue = m_basic;
			m_document->m_PushOp( op );
		}
		m_basic = value;
	}
}
const char* DocumentValue::ValueGet() const
{
	AE_ASSERT( IsValue() );
	return m_basic.c_str();
}

// Array manipulation
void DocumentValue::ArrayInitialize( uint32_t reserveLength )
{
	Initialize( DocumentValueType::Array );
	m_array.Reserve( reserveLength );
}
DocumentValue& DocumentValue::ArrayInsert( uint32_t index )
{
	AE_ASSERT( IsArray() );
	DocumentValue* newValue = m_document->m_values.New( m_document, m_document->m_tag );

	UndoOp op;
	op.type = UndoOpType::ArrayRemove; // Reverse operation
	op.target = this;
	op.index = index;
	m_document->m_PushOp( op );

	return *m_array.Insert( index, newValue );
}
DocumentValue& DocumentValue::ArrayAppend()
{
	AE_ASSERT( IsArray() );
	return ArrayInsert( ArrayLength() );
}
void DocumentValue::ArrayRemove( uint32_t index )
{
	AE_ASSERT( IsArray() );
	AE_ASSERT( index < m_array.Length() );
	DocumentValue* removed = m_array[ index ];

	UndoOp op;
	op.type = UndoOpType::ArrayInsert; // Reverse operation
	op.target = this;
	op.index = index;
	op.oldChild = removed;
	m_document->m_PushOp( op );

	m_array.Remove( index );
	// Increment reference count - now only kept alive by undo stack
	m_document->m_AddRef( removed );
}

// Array iteration
uint32_t DocumentValue::ArrayLength() const
{
	AE_ASSERT( IsArray() );
	return m_array.Length();
}
DocumentValue& DocumentValue::ArrayGet( uint32_t index )
{
	AE_ASSERT( IsArray() );
	return *m_array[ index ];
}
const DocumentValue& DocumentValue::ArrayGet( uint32_t index ) const
{
	AE_ASSERT( IsArray() );
	return *m_array[ index ];
}

// Map manipulation
void DocumentValue::MapInitialize( uint32_t reserveLength )
{
	Initialize( DocumentValueType::Map );
	m_map.Reserve( reserveLength );
}
DocumentValue& DocumentValue::MapGet( const char* key )
{
	AE_ASSERT( IsMap() );
	DocumentValue* existing = m_map.Get( key, nullptr );
	if( !existing )
	{
		UndoOp op;
		op.type = UndoOpType::MapRemove; // Reverse operation
		op.target = this;
		op.key = key;
		m_document->m_PushOp( op );
		return *m_map.Set( key, m_document->m_values.New( m_document, m_document->m_tag ) );
	}
	return *existing;
}
bool DocumentValue::MapRemove( const char* key )
{
	AE_ASSERT( IsMap() );
	DocumentValue* existing = m_map.Get( key, nullptr );
	if( !existing )
	{
		return false; // Key doesn't exist
	}

	UndoOp op;
	op.type = UndoOpType::MapSet; // Reverse operation
	op.target = this;
	op.key = key;
	op.oldChild = existing;
	m_document->m_PushOp( op );

	m_map.Remove( key, nullptr );
	// Increment reference count - now only kept alive by undo stack
	m_document->m_AddRef( existing );
	return true;
}

// Map queries
DocumentValue* DocumentValue::MapTryGet( const char* key )
{
	AE_ASSERT( IsMap() );
	return m_map.Get( key, nullptr );
}
const DocumentValue* DocumentValue::MapTryGet( const char* key ) const
{
	AE_ASSERT( IsMap() );
	return m_map.Get( key, nullptr );
}

// Map iteration
uint32_t DocumentValue::MapLength() const
{
	AE_ASSERT( IsMap() );
	return m_map.Length();
}
const char* DocumentValue::MapGetKey( uint32_t index ) const
{
	AE_ASSERT( IsMap() );
	return m_map.GetKey( index ).c_str();
}
DocumentValue& DocumentValue::MapGetValue( uint32_t index )
{
	AE_ASSERT( IsMap() );
	return *m_map.GetValue( index );
}
const DocumentValue& DocumentValue::MapGetValue( uint32_t index ) const
{
	AE_ASSERT( IsMap() );
	return *m_map.GetValue( index );
}

// Protected methods
DocumentValue& DocumentValue::Initialize( DocumentValueType type )
{
	// First, clear existing data using proper undo operations
	switch( m_type )
	{
		case DocumentValueType::Array:
			while( m_array.Length() > 0 )
			{
				ArrayRemove( m_array.Length() - 1 );
			}
			break;
		case DocumentValueType::Map:
			while( m_map.Length() > 0 )
			{
				MapRemove( m_map.GetKey( m_map.Length() - 1 ).c_str() );
			}
			break;
		case DocumentValueType::Value:
			// For basic values, create a ValueSet undo operation
			if( !m_basic.empty() )
			{
				UndoOp op;
				op.type = UndoOpType::ValueSet;
				op.target = this;
				op.oldValue = m_basic;
				m_document->m_PushOp( op );
				m_basic.clear();
			}
			break;
		case DocumentValueType::None: break;
	}

	if( m_type != type )
	{
		// Create the type change operation with coalescing logic
		bool combined = false;
		if( m_document->m_currentGroup.Length() > 0 )
		{
			UndoOp& lastOp = m_document->m_currentGroup[ m_document->m_currentGroup.Length() - 1 ];
			if( lastOp.type == UndoOpType::SetType && lastOp.target == this )
			{
				// Keep the original old type, don't add new operation
				combined = true;
			}
		}
		if( !combined )
		{
			UndoOp op;
			op.type = UndoOpType::SetType;
			op.target = this;
			op.oldType = m_type;
			m_document->m_PushOp( op );
		}
		m_type = type;
	}
	// Data should already be cleared by the switch above
	AE_ASSERT( m_basic.empty() );
	AE_ASSERT( m_array.Length() == 0 );
	AE_ASSERT( m_map.Length() == 0 );
	return *this;
}

//------------------------------------------------------------------------------
// ae::Document member functions
//------------------------------------------------------------------------------
Document::Document( const ae::Tag& tag ) : DocumentValue( this, tag ), m_tag( tag ), m_values( tag ), m_undoStack( tag ), m_redoStack( tag ), m_currentGroup( tag ) {}
Document::~Document() { m_values.DeleteAll(); }

ae::Array< Document::UndoOp > Document::m_ReverseOps( const ae::Array< UndoOp >& ops )
{
	ae::Array< UndoOp > reverseGroup( m_tag );
	for( int32_t i = ops.Length() - 1; i >= 0; i-- )
	{
		const UndoOp& op = ops[ i ];
		UndoOp reverseOp;
		reverseOp.target = op.target;

		switch( op.type )
		{
			case UndoOpType::SetType:
				reverseOp.type = UndoOpType::SetType;
				reverseOp.oldType = op.target->m_type;
				op.target->m_type = op.oldType;
				// Data should always be empty when the type changes since
				// Initialize() removes all elements first
				AE_DEBUG_ASSERT( op.target->m_basic.empty() );
				AE_DEBUG_ASSERT( op.target->m_array.Length() == 0 );
				AE_DEBUG_ASSERT( op.target->m_map.Length() == 0 );
				break;
			case UndoOpType::ValueSet:
				reverseOp.type = UndoOpType::ValueSet;
				reverseOp.oldValue = op.target->m_basic;
				op.target->m_basic = op.oldValue;
				break;
			case UndoOpType::ArrayInsert:
				reverseOp.type = UndoOpType::ArrayRemove;
				reverseOp.index = op.index;
				op.target->m_array.Insert( op.index, op.oldChild );
				AE_DEBUG_ASSERT( op.oldChild->m_refCount == 1 );
				op.oldChild->m_refCount = 0; // Object back in document tree, reset to document ownership
				break;
			case UndoOpType::ArrayRemove:
				reverseOp.type = UndoOpType::ArrayInsert;
				reverseOp.index = op.index;
				reverseOp.oldChild = op.target->m_array[ op.index ];
				op.target->m_array.Remove( op.index );
				m_AddRef( reverseOp.oldChild ); // Object removed from document, add reference
				break;
			case UndoOpType::MapSet:
				reverseOp.type = UndoOpType::MapRemove;
				reverseOp.key = op.key;
				op.target->m_map.Set( op.key, op.oldChild );
				AE_DEBUG_ASSERT( op.oldChild->m_refCount == 1 );
				op.oldChild->m_refCount = 0; // Object back in document tree, reset to document ownership
				break;
			case UndoOpType::MapRemove:
				reverseOp.type = UndoOpType::MapSet;
				reverseOp.key = op.key;
				if( DocumentValue* existing = op.target->m_map.Get( op.key, nullptr ) )
				{
					reverseOp.oldChild = existing;
					m_AddRef( existing ); // Object removed from document, add reference
				}
				op.target->m_map.Remove( op.key, nullptr );
				break;
		}
		reverseGroup.Append( reverseOp );
	}
	return reverseGroup;
}

void Document::m_PushOp( const UndoOp& op )
{
	m_currentGroup.Append( op );
	m_redoStack.Clear(); // Clear redo stack on new operation
}

void Document::EndUndoGroup()
{
	if( m_currentGroup.Length() > 0 )
	{
		m_undoStack.Append( std::move( m_currentGroup ) );
		m_currentGroup.Clear();
	}
}

void Document::ClearUndo()
{
#define AE_REMOVE_OP_GROUP_REFS( group ) \
	for( const UndoOp& op : group ) \
	{ \
		if( op.oldChild ) \
		{ \
			m_RemoveRef( op.oldChild ); \
		} \
	}
	for( const auto& group : m_undoStack )
	{
		AE_REMOVE_OP_GROUP_REFS( group );
	}
	for( const auto& group : m_redoStack )
	{
		AE_REMOVE_OP_GROUP_REFS( group );
	}
	AE_REMOVE_OP_GROUP_REFS( m_currentGroup );
#undef AE_REMOVE_OP_GROUP_REFS
	m_undoStack.Clear();
	m_redoStack.Clear();
	m_currentGroup.Clear();
}

bool Document::Undo()
{
	EndUndoGroup(); // Finalize current group
	if( m_undoStack.Length() == 0 )
	{
		return false;
	}
	ae::Array< UndoOp > group = std::move( m_undoStack[ m_undoStack.Length() - 1 ] );
	m_undoStack.Remove( m_undoStack.Length() - 1 );
	m_redoStack.Append( m_ReverseOps( group ) );
	m_ValidateState( m_redoStack[ m_redoStack.Length() - 1 ] );
	return true;
}

void Document::m_AddRef( DocumentValue* value )
{
	if( value )
	{
		value->m_refCount++;
	}
}

void Document::m_RemoveRef( DocumentValue* value )
{
	if( value )
	{
		value->m_refCount--;
		AE_DEBUG_ASSERT( value->m_refCount >= 0 );
		if( value->m_refCount == 0 )
		{
			m_values.Delete( value );
		}
	}
}

void Document::m_ValidateState( const ae::Array< UndoOp >& operations )
{
	for( const UndoOp& op : operations )
	{
		DocumentValue* target = op.target;
		// Validate type consistency with data contents
		AE_DEBUG_ASSERT( target->m_basic.length() == 0 || target->m_type == DocumentValueType::Value );
		AE_DEBUG_ASSERT( target->m_array.Length() == 0 || target->m_type == DocumentValueType::Array );
		AE_DEBUG_ASSERT( target->m_map.Length() == 0 || target->m_type == DocumentValueType::Map );
		AE_DEBUG_ASSERT( target->m_refCount >= 0 );
	}
}

bool Document::Redo()
{
	EndUndoGroup(); // Finalize current group
	if( m_redoStack.Length() == 0 )
	{
		return false;
	}
	ae::Array< UndoOp > group = std::move( m_redoStack[ m_redoStack.Length() - 1 ] );
	m_redoStack.Remove( m_redoStack.Length() - 1 );
	m_undoStack.Append( m_ReverseOps( group ) );
	m_ValidateState( m_undoStack[ m_undoStack.Length() - 1 ] );
	return true;
}

} // namespace ae

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
	doc.MapGet( "key1" ).ValueSet( "value1" );
	doc.MapGet( "key2" ).ValueSet( "value2" );
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
	doc.ArrayGet( 0 ).MapGet( "nested" ).ValueSet( "deep" );
	doc.EndUndoGroup();

	REQUIRE( doc.ArrayLength() == 1 );
	REQUIRE( doc.ArrayGet( 0 ).IsMap() );
	REQUIRE( doc.ArrayGet( 0 ).MapGet( "nested" ).ValueGet() == std::string( "deep" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Modify nested value
	doc.ArrayGet( 0 ).MapGet( "nested" ).ValueSet( "modified" );
	doc.EndUndoGroup();

	REQUIRE( doc.ArrayGet( 0 ).MapGet( "nested" ).ValueGet() == std::string( "modified" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should restore deep nested value
	REQUIRE( doc.Undo() );
	REQUIRE( doc.ArrayGet( 0 ).MapGet( "nested" ).ValueGet() == std::string( "deep" ) );
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
	REQUIRE( doc.ArrayGet( 0 ).MapGet( "nested" ).ValueGet() == std::string( "deep" ) );
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
	doc.MapGet( "apple" ).ValueSet( "fruit1" );
	doc.MapGet( "banana" ).ValueSet( "fruit2" );
	doc.MapGet( "cherry" ).ValueSet( "fruit3" );
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
	doc.MapGet( "existing1" ).ValueSet( "value1" );
	doc.MapGet( "existing2" ).ValueSet( "value2" );
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
	doc.MapGet( "existing" ).ValueSet( "value" );
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

	ae::DocumentValue& nonConstRef = doc.MapGet( "existing" );
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
	doc.MapGet( "key1" ).ValueSet( "value1" );
	doc.MapGet( "key2" ).ValueSet( "value2" );
	doc.MapGet( "key3" ).ValueSet( "value3" );
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
	doc.Initialize( ae::DocumentValueType::Map ).MapGet( "chained_key" ).ValueSet( "chained_value" );
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
	doc.ArrayGet( 0 ).ArrayGet( 1 ).MapGet( "nested_key" ).ValueSet( "nested_value" );
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
	doc.MapGet( "key1" ).ValueSet( "value1" );
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
	doc.MapGet( "key1" ).ValueSet( "value1" );
	doc.MapGet( "key2" ).ValueSet( "value2" );
	doc.MapGet( "key3" ).ValueSet( "value3" );
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
	doc.MapGet( "newkey1" ).ValueSet( "newvalue1" );
	doc.MapGet( "newkey2" ).ValueSet( "newvalue2" );
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
