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
	Basic,
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

	// Type queries
	bool IsBasicValue() const;
	bool IsArray() const;
	bool IsMap() const;
	DocumentValueType GetType() const { return m_type; }

	//--------------------------------------------------------------------------
	// Basic value
	//--------------------------------------------------------------------------
	//! Sets the value to a basic string type. Note that repeated calls to
	//! SetBasicValue will be coalesced into a single undo operation. This
	//! behavior can be circumvented by ending the current undo group between
	//! calls.
	//! \param value The string value to set. Can be nullptr or empty string.
	void SetBasicValue( const char* value );
	//! Returns the basic string value. Must only be called when IsBasicValue() returns true.
	//! \return The string value as a null-terminated C string.
	const char* GetBasicValue() const;

	//--------------------------------------------------------------------------
	// Array manipulation
	//--------------------------------------------------------------------------
	//! Converts this value to an array type, clearing any existing data.
	//! \param reserveLength Optional hint for initial capacity to avoid reallocations.
	void SetArray( uint32_t reserveLength = 0 );
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
	void SetMap( uint32_t reserveLength = 0 );
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
		SetBasicValue,
		InsertArrayValue,
		RemoveArrayValue,
		SetMapValue,
		RemoveMapValue
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
	void m_SetType( DocumentValueType type );
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
	uint32_t GetUndoStackSize() const { return m_undoStack.Length(); }
	uint32_t GetRedoStackSize() const { return m_redoStack.Length(); }

private:
	friend class DocumentValue;
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
bool DocumentValue::IsBasicValue() const { return ( m_type == DocumentValueType::Basic ); }
bool DocumentValue::IsArray() const { return ( m_type == DocumentValueType::Array ); }
bool DocumentValue::IsMap() const { return ( m_type == DocumentValueType::Map ); }

// Basic
void DocumentValue::SetBasicValue( const char* value )
{
	m_SetType( DocumentValueType::Basic );
	if( m_basic != value )
	{
		// Try to combine with previous SetBasicValue operation in current group
		bool combined = false;
		if( m_document->m_currentGroup.Length() > 0 )
		{
			UndoOp& lastOp = m_document->m_currentGroup[ m_document->m_currentGroup.Length() - 1 ];
			if( lastOp.type == UndoOpType::SetBasicValue && lastOp.target == this )
			{
				// Keep the original old value, just update current value
				combined = true;
			}
		}
		if( !combined )
		{
			UndoOp op;
			op.type = UndoOpType::SetBasicValue;
			op.target = this;
			op.oldValue = m_basic;
			m_document->m_PushOp( op );
		}
		m_basic = value;
	}
}
const char* DocumentValue::GetBasicValue() const
{
	AE_ASSERT( IsBasicValue() );
	return m_basic.c_str();
}

// Array manipulation
void DocumentValue::SetArray( uint32_t reserveLength )
{
	m_SetType( DocumentValueType::Array );
	m_array.Reserve( reserveLength );
}
DocumentValue& DocumentValue::ArrayInsert( uint32_t index )
{
	AE_ASSERT( IsArray() );
	DocumentValue* newValue = m_document->m_values.New( m_document, m_document->m_tag );

	UndoOp op;
	op.type = UndoOpType::RemoveArrayValue; // Reverse operation
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
	op.type = UndoOpType::InsertArrayValue; // Reverse operation
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
void DocumentValue::SetMap( uint32_t reserveLength )
{
	m_SetType( DocumentValueType::Map );
	m_map.Reserve( reserveLength );
}
DocumentValue& DocumentValue::MapGet( const char* key )
{
	AE_ASSERT( IsMap() );
	DocumentValue* existing = m_map.Get( key, nullptr );
	if( !existing )
	{
		UndoOp op;
		op.type = UndoOpType::RemoveMapValue; // Reverse operation
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
	op.type = UndoOpType::SetMapValue; // Reverse operation
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
void DocumentValue::m_SetType( DocumentValueType type )
{
	if( m_type != type )
	{
		UndoOp op;
		op.type = UndoOpType::SetType;
		op.target = this;
		op.oldType = m_type;
		op.oldValue = m_basic;
		m_document->m_PushOp( op );

		m_type = type;
		// Clear previous data on type change
		m_basic.clear();
		m_array.Clear();
		m_map.Clear();
	}
}

//------------------------------------------------------------------------------
// ae::Document member functions
//------------------------------------------------------------------------------
Document::Document( const ae::Tag& tag ) : DocumentValue( this, tag ), m_tag( tag ), m_values( tag ), m_undoStack( tag ), m_redoStack( tag ), m_currentGroup( tag ) {}
Document::~Document() { m_values.DeleteAll(); }

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
	if( m_undoStack.Length() == 0 ) return false;

	ae::Array< UndoOp > group = std::move( m_undoStack[ m_undoStack.Length() - 1 ] );
	m_undoStack.Remove( m_undoStack.Length() - 1 );

	ae::Array< UndoOp > redoGroup( m_tag );

	// Execute operations in reverse order
	for( int32_t i = group.Length() - 1; i >= 0; i-- )
	{
		const UndoOp& op = group[ i ];
		UndoOp redoOp;
		redoOp.target = op.target;

		switch( op.type )
		{
			case UndoOpType::SetType:
				redoOp.type = UndoOpType::SetType;
				redoOp.oldType = op.target->m_type;
				redoOp.oldValue = op.target->m_basic;
				op.target->m_type = op.oldType;
				op.target->m_basic = op.oldValue; // Shouldn't this be cleared?
				op.target->m_array.Clear();
				op.target->m_map.Clear();
				break;
			case UndoOpType::SetBasicValue:
				redoOp.type = UndoOpType::SetBasicValue;
				redoOp.oldValue = op.target->m_basic;
				op.target->m_basic = op.oldValue;
				break;
			case UndoOpType::InsertArrayValue:
				redoOp.type = UndoOpType::RemoveArrayValue;
				redoOp.index = op.index;
				op.target->m_array.Insert( op.index, op.oldChild );
				AE_ASSERT( op.oldChild->m_refCount == 1 );
				op.oldChild->m_refCount = 0; // Object back in document tree, reset to document ownership
				break;
			case UndoOpType::RemoveArrayValue:
				redoOp.type = UndoOpType::InsertArrayValue;
				redoOp.index = op.index;
				redoOp.oldChild = op.target->m_array[ op.index ];
				op.target->m_array.Remove( op.index );
				m_AddRef( redoOp.oldChild ); // Object removed from document, add redo reference
				break;
			case UndoOpType::SetMapValue:
				redoOp.type = UndoOpType::RemoveMapValue;
				redoOp.key = op.key;
				op.target->m_map.Set( op.key, op.oldChild );
				AE_ASSERT( op.oldChild->m_refCount == 1 );
				op.oldChild->m_refCount = 0; // Object back in document tree, reset to document ownership
				break;
			case UndoOpType::RemoveMapValue:
				redoOp.type = UndoOpType::SetMapValue;
				redoOp.key = op.key;
				if( DocumentValue* existing = op.target->m_map.Get( op.key, nullptr ) )
				{
					redoOp.oldChild = existing;
					m_AddRef( existing ); // Object removed from document, add redo reference
				}
				op.target->m_map.Remove( op.key, nullptr );
				break;
		}
		redoGroup.Append( redoOp );
	}

	m_redoStack.Append( std::move( redoGroup ) );

	// Validate final state consistency
	m_ValidateState( group );

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
		AE_ASSERT( value->m_refCount >= 0 );
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
		AE_ASSERT( target->m_basic.length() == 0 || target->m_type == DocumentValueType::Basic );
		AE_ASSERT( target->m_array.Length() == 0 || target->m_type == DocumentValueType::Array );
		AE_ASSERT( target->m_map.Length() == 0 || target->m_type == DocumentValueType::Map );

		// Validate reference count invariants
		AE_ASSERT( target->m_refCount >= 0 ); // Reference count should never be negative
		// Note: Objects in document tree should have refCount==0, objects only in stacks should have refCount>0
	}
}

bool Document::Redo()
{
	if( m_redoStack.Length() == 0 ) return false;

	ae::Array< UndoOp > group = std::move( m_redoStack[ m_redoStack.Length() - 1 ] );
	m_redoStack.Remove( m_redoStack.Length() - 1 );

	ae::Array< UndoOp > undoGroup( m_tag );

	for( int32_t i = group.Length() - 1; i >= 0; i-- )
	{
		const UndoOp& op = group[ i ];
		UndoOp undoOp;
		undoOp.target = op.target;

		switch( op.type )
		{
			case UndoOpType::SetType:
				undoOp.type = UndoOpType::SetType;
				undoOp.oldType = op.target->m_type;
				undoOp.oldValue = op.target->m_basic;
				op.target->m_type = op.oldType;
				op.target->m_basic = op.oldValue; // Shouldn't this be cleared?
				op.target->m_array.Clear();
				op.target->m_map.Clear();
				break;
			case UndoOpType::SetBasicValue:
				undoOp.type = UndoOpType::SetBasicValue;
				undoOp.oldValue = op.target->m_basic;
				op.target->m_basic = op.oldValue;
				break;
			case UndoOpType::InsertArrayValue:
				undoOp.type = UndoOpType::RemoveArrayValue;
				undoOp.index = op.index;
				op.target->m_array.Insert( op.index, op.oldChild );
				AE_ASSERT( op.oldChild->m_refCount == 1 );
				op.oldChild->m_refCount = 0; // Object back in document tree, reset to document ownership
				break;
			case UndoOpType::RemoveArrayValue:
				undoOp.type = UndoOpType::InsertArrayValue;
				undoOp.index = op.index;
				undoOp.oldChild = op.target->m_array[ op.index ];
				op.target->m_array.Remove( op.index );
				m_AddRef( undoOp.oldChild ); // Object removed from document, add undo reference
				break;
			case UndoOpType::SetMapValue:
				undoOp.type = UndoOpType::RemoveMapValue;
				undoOp.key = op.key;
				op.target->m_map.Set( op.key, op.oldChild );
				AE_ASSERT( op.oldChild->m_refCount == 1 );
				op.oldChild->m_refCount = 0; // Object back in document tree, reset to document ownership
				break;
			case UndoOpType::RemoveMapValue:
				undoOp.type = UndoOpType::SetMapValue;
				undoOp.key = op.key;
				if( DocumentValue* existing = op.target->m_map.Get( op.key, nullptr ) )
				{
					undoOp.oldChild = existing;
					m_AddRef( existing ); // Object removed from document, add undo reference
				}
				op.target->m_map.Remove( op.key, nullptr );
				break;
		}
		undoGroup.Append( undoOp );
	}

	m_undoStack.Append( std::move( undoGroup ) );

	// Validate final state consistency
	m_ValidateState( group );

	return true;
}

} // namespace ae

TEST_CASE( "DocumentUndo BasicValue", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Set initial value
	doc.SetBasicValue( "initial" );
	doc.EndUndoGroup();
	REQUIRE( doc.GetBasicValue() == std::string( "initial" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Change value
	doc.SetBasicValue( "changed" );
	doc.EndUndoGroup();
	REQUIRE( doc.GetBasicValue() == std::string( "changed" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should restore previous value
	REQUIRE( doc.Undo() );
	REQUIRE( doc.GetBasicValue() == std::string( "initial" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Redo should restore changed value
	REQUIRE( doc.Redo() );
	REQUIRE( doc.GetBasicValue() == std::string( "changed" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Multiple undos
	REQUIRE( doc.Undo() );
	REQUIRE( doc.GetBasicValue() == std::string( "initial" ) );
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
	doc.SetBasicValue( "test" );
	doc.EndUndoGroup();
	REQUIRE( doc.IsBasicValue() );
	REQUIRE( doc.GetBasicValue() == std::string( "test" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Change to array
	doc.SetArray( 4 );
	doc.EndUndoGroup();
	REQUIRE( doc.IsArray() );
	REQUIRE( doc.ArrayLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should restore type and value
	REQUIRE( doc.Undo() );
	REQUIRE( doc.IsBasicValue() );
	REQUIRE( doc.GetBasicValue() == std::string( "test" ) );
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
	doc.SetArray( 4 );
	doc.EndUndoGroup();
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Add array elements
	doc.ArrayAppend().SetBasicValue( "first" );
	doc.ArrayAppend().SetBasicValue( "second" );
	doc.ArrayAppend().SetBasicValue( "third" );
	doc.EndUndoGroup();

	REQUIRE( doc.ArrayLength() == 3 );
	REQUIRE( doc.ArrayGet( 0 ).GetBasicValue() == std::string( "first" ) );
	REQUIRE( doc.ArrayGet( 1 ).GetBasicValue() == std::string( "second" ) );
	REQUIRE( doc.ArrayGet( 2 ).GetBasicValue() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Insert in middle
	doc.ArrayInsert( 1 ).SetBasicValue( "inserted" );
	doc.EndUndoGroup();

	REQUIRE( doc.ArrayLength() == 4 );
	REQUIRE( doc.ArrayGet( 0 ).GetBasicValue() == std::string( "first" ) );
	REQUIRE( doc.ArrayGet( 1 ).GetBasicValue() == std::string( "inserted" ) );
	REQUIRE( doc.ArrayGet( 2 ).GetBasicValue() == std::string( "second" ) );
	REQUIRE( doc.ArrayGet( 3 ).GetBasicValue() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo insert
	REQUIRE( doc.Undo() );
	REQUIRE( doc.ArrayLength() == 3 );
	REQUIRE( doc.ArrayGet( 0 ).GetBasicValue() == std::string( "first" ) );
	REQUIRE( doc.ArrayGet( 1 ).GetBasicValue() == std::string( "second" ) );
	REQUIRE( doc.ArrayGet( 2 ).GetBasicValue() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Remove element
	doc.ArrayRemove( 1 );
	doc.EndUndoGroup();

	REQUIRE( doc.ArrayLength() == 2 );
	REQUIRE( doc.ArrayGet( 0 ).GetBasicValue() == std::string( "first" ) );
	REQUIRE( doc.ArrayGet( 1 ).GetBasicValue() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 3 ); // Redo stack was cleared, new operation added
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo remove should restore element at correct position
	REQUIRE( doc.Undo() );
	REQUIRE( doc.ArrayLength() == 3 );
	REQUIRE( doc.ArrayGet( 0 ).GetBasicValue() == std::string( "first" ) );
	REQUIRE( doc.ArrayGet( 1 ).GetBasicValue() == std::string( "second" ) );
	REQUIRE( doc.ArrayGet( 2 ).GetBasicValue() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 1 );
}

TEST_CASE( "DocumentUndo MapOperations", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	doc.SetMap( 4 );
	doc.EndUndoGroup();
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Add map values
	doc.MapGet( "key1" ).SetBasicValue( "value1" );
	doc.MapGet( "key2" ).SetBasicValue( "value2" );
	doc.EndUndoGroup();

	REQUIRE( doc.MapLength() == 2 );
	REQUIRE( doc.MapTryGet( "key1" )->GetBasicValue() == std::string( "value1" ) );
	REQUIRE( doc.MapTryGet( "key2" )->GetBasicValue() == std::string( "value2" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Remove a key
	REQUIRE( doc.MapRemove( "key1" ) ); // Should return true for successful removal
	doc.EndUndoGroup();

	REQUIRE( doc.MapLength() == 1 );
	REQUIRE( doc.MapTryGet( "key1" ) == nullptr );
	REQUIRE( doc.MapTryGet( "key2" )->GetBasicValue() == std::string( "value2" ) );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo remove should restore key and value
	REQUIRE( doc.Undo() );
	REQUIRE( doc.MapLength() == 2 );
	REQUIRE( doc.MapTryGet( "key1" )->GetBasicValue() == std::string( "value1" ) );
	REQUIRE( doc.MapTryGet( "key2" )->GetBasicValue() == std::string( "value2" ) );
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
	doc.SetArray( 4 );

	// Multiple operations in one group
	doc.ArrayAppend().SetBasicValue( "first" );
	doc.ArrayAppend().SetBasicValue( "second" );
	doc.ArrayAppend().SetBasicValue( "third" );
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
	REQUIRE( doc.ArrayGet( 0 ).GetBasicValue() == std::string( "first" ) );
	REQUIRE( doc.ArrayGet( 1 ).GetBasicValue() == std::string( "second" ) );
	REQUIRE( doc.ArrayGet( 2 ).GetBasicValue() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );
}

TEST_CASE( "DocumentUndo ClearUndo", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	doc.SetBasicValue( "test" );
	doc.EndUndoGroup();
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	doc.SetBasicValue( "changed" );
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

TEST_CASE( "DocumentUndo BasicValueCoalescing", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Multiple SetBasicValue calls in same group should coalesce
	doc.SetBasicValue( "first" );
	REQUIRE( doc.GetBasicValue() == std::string( "first" ) );
	doc.SetBasicValue( "second" );
	REQUIRE( doc.GetBasicValue() == std::string( "second" ) );
	doc.SetBasicValue( "third" );
	REQUIRE( doc.GetBasicValue() == std::string( "third" ) );
	doc.EndUndoGroup();

	// Should only have one undo operation despite 3 SetBasicValue calls
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Single undo should revert to initial state (empty)
	REQUIRE( doc.Undo() );
	REQUIRE( doc.GetType() == ae::DocumentValueType::None );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Single redo should restore final value
	REQUIRE( doc.Redo() );
	REQUIRE( doc.GetBasicValue() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );
}

TEST_CASE( "DocumentUndo BasicValueSeparateGroups", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// First value with separate undo group
	doc.SetBasicValue( "first" );
	doc.EndUndoGroup();
	REQUIRE( doc.GetBasicValue() == std::string( "first" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Second value with separate undo group (no coalescing)
	doc.SetBasicValue( "second" );
	doc.EndUndoGroup();
	REQUIRE( doc.GetBasicValue() == std::string( "second" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Third value with separate undo group (no coalescing)
	doc.SetBasicValue( "third" );
	doc.EndUndoGroup();
	REQUIRE( doc.GetBasicValue() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Multiple undos should step through each value
	REQUIRE( doc.Undo() );
	REQUIRE( doc.GetBasicValue() == std::string( "second" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	REQUIRE( doc.Undo() );
	REQUIRE( doc.GetBasicValue() == std::string( "first" ) );
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
	doc.SetArray( 4 );

	// Create nested structure
	doc.ArrayAppend().SetMap( 2 );
	doc.ArrayGet( 0 ).MapGet( "nested" ).SetBasicValue( "deep" );
	doc.EndUndoGroup();

	REQUIRE( doc.ArrayLength() == 1 );
	REQUIRE( doc.ArrayGet( 0 ).IsMap() );
	REQUIRE( doc.ArrayGet( 0 ).MapGet( "nested" ).GetBasicValue() == std::string( "deep" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Modify nested value
	doc.ArrayGet( 0 ).MapGet( "nested" ).SetBasicValue( "modified" );
	doc.EndUndoGroup();

	REQUIRE( doc.ArrayGet( 0 ).MapGet( "nested" ).GetBasicValue() == std::string( "modified" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should restore deep nested value
	REQUIRE( doc.Undo() );
	REQUIRE( doc.ArrayGet( 0 ).MapGet( "nested" ).GetBasicValue() == std::string( "deep" ) );
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
	REQUIRE( doc.ArrayGet( 0 ).MapGet( "nested" ).GetBasicValue() == std::string( "deep" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 1 );
}

TEST_CASE( "DocumentUndo MapIteration", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	doc.SetMap( 4 );
	doc.EndUndoGroup();
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Add multiple map values with different keys
	doc.MapGet( "apple" ).SetBasicValue( "fruit1" );
	doc.MapGet( "banana" ).SetBasicValue( "fruit2" );
	doc.MapGet( "cherry" ).SetBasicValue( "fruit3" );
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
			REQUIRE( value.GetBasicValue() == std::string( "fruit1" ) );
			foundApple = true;
		}
		else if( std::string( key ) == "banana" )
		{
			REQUIRE( value.GetBasicValue() == std::string( "fruit2" ) );
			foundBanana = true;
		}
		else if( std::string( key ) == "cherry" )
		{
			REQUIRE( value.GetBasicValue() == std::string( "fruit3" ) );
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
			value.SetBasicValue( "yellow_fruit" );
			break;
		}
	}
	doc.EndUndoGroup();

	// Verify the modification
	REQUIRE( doc.MapTryGet( "banana" )->GetBasicValue() == std::string( "yellow_fruit" ) );
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
	doc.SetMap( 4 );
	doc.EndUndoGroup();
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Add some map values
	doc.MapGet( "existing1" ).SetBasicValue( "value1" );
	doc.MapGet( "existing2" ).SetBasicValue( "value2" );
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
	doc.SetMap( 4 );
	doc.EndUndoGroup();
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Add some map values
	doc.MapGet( "existing" ).SetBasicValue( "value" );
	doc.EndUndoGroup();

	REQUIRE( doc.MapLength() == 1 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test const MapTryGet with existing key - should return valid pointer
	const ae::DocumentValue* constExisting = constDoc.MapTryGet( "existing" );
	REQUIRE( constExisting != nullptr );
	REQUIRE( constExisting->GetBasicValue() == std::string( "value" ) );

	// Test const MapTryGet with non-existing key - should return nullptr
	const ae::DocumentValue* constNonExisting = constDoc.MapTryGet( "nonexistent" );
	REQUIRE( constNonExisting == nullptr );

	// Test non-const MapTryGet with existing key - should return valid pointer
	ae::DocumentValue* nonConstExisting = doc.MapTryGet( "existing" );
	REQUIRE( nonConstExisting != nullptr );
	REQUIRE( nonConstExisting->GetBasicValue() == std::string( "value" ) );

	// Test non-const MapTryGet with non-existing key - should return nullptr
	ae::DocumentValue* nonConstNonExisting = doc.MapTryGet( "nonexistent" );
	REQUIRE( nonConstNonExisting == nullptr );

	// Test modification through non-const MapTryGet pointer
	ae::DocumentValue* modifyPtr = doc.MapTryGet( "existing" );
	REQUIRE( modifyPtr != nullptr );
	modifyPtr->SetBasicValue( "modified_via_tryget" );
	doc.EndUndoGroup();

	// Verify modification worked
	REQUIRE( doc.MapTryGet( "existing" )->GetBasicValue() == std::string( "modified_via_tryget" ) );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	ae::DocumentValue& nonConstRef = doc.MapGet( "existing" );
	REQUIRE( nonConstRef.GetBasicValue() == std::string( "modified_via_tryget" ) );

	// Modify through MapGet reference
	nonConstRef.SetBasicValue( "modified_value" );
	doc.EndUndoGroup();

	// Verify modification
	REQUIRE( doc.MapTryGet( "existing" )->GetBasicValue() == std::string( "modified_value" ) );
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
	doc.SetBasicValue( "" );
	doc.EndUndoGroup();
	REQUIRE( doc.IsBasicValue() );
	REQUIRE( doc.GetBasicValue() == std::string( "" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test setting same value (should not create new undo operation)
	doc.SetBasicValue( "" );
	doc.EndUndoGroup();
	REQUIRE( doc.GetUndoStackSize() == 1 ); // Should still be 1
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test array with zero reserve
	doc.SetArray( 0 );
	doc.EndUndoGroup();
	REQUIRE( doc.IsArray() );
	REQUIRE( doc.ArrayLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Test map with zero reserve
	doc.SetMap( 0 );
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

	// Test multiple type changes in same group
	doc.SetBasicValue( "test" );
	doc.SetArray( 2 );
	doc.SetMap( 2 );
	doc.EndUndoGroup(); // Should coalesce into single type change operation

	REQUIRE( doc.IsMap() );
	REQUIRE( doc.GetUndoStackSize() == 4 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should revert to previous state (empty map)
	REQUIRE( doc.Undo() );
	REQUIRE( doc.IsMap() );
	REQUIRE( doc.MapLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 1 );
}
