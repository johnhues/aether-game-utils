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
	DocumentValueType GetValueType() const { return m_type; }

	// Basic
	bool IsBasicValue() const;
	//! Note that repeated calls to SetBasicValue will be coalesced into a
	//! single undo operation. This behavior can be circumvented by ending the
	//! current undo group between calls.
	void SetBasicValue( const char* value );
	const char* GetBasicValue() const;

	// Array
	bool IsArray() const;
	uint32_t GetArrayLength() const;
	void SetArray( uint32_t reserveLength = 0 );
	DocumentValue& InsertArrayValue( uint32_t index );
	DocumentValue& SetArrayValue( uint32_t index );
	DocumentValue& AppendArrayValue();
	void RemoveArrayValue( uint32_t index );
	DocumentValue* GetArrayValue( uint32_t index );
	const DocumentValue* GetArrayValue( uint32_t index ) const;

	// Map
	bool IsMap() const;
	uint32_t GetMapLength() const;
	void SetMap( uint32_t reserveLength = 0 );
	DocumentValue& SetMapValue( const char* key );
	void RemoveMapValue( const char* key );
	DocumentValue* GetMapValue( const char* key );
	DocumentValue* GetMapValue( uint32_t index );
	const DocumentValue* GetMapValue( const char* key ) const;
	const DocumentValue* GetMapValue( uint32_t index ) const;

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
bool DocumentValue::IsBasicValue() const { return ( m_type == DocumentValueType::Basic ); }
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
bool DocumentValue::IsArray() const { return ( m_type == DocumentValueType::Array ); }
uint32_t DocumentValue::GetArrayLength() const
{
	AE_ASSERT( IsArray() );
	return m_array.Length();
}
void DocumentValue::SetArray( uint32_t reserveLength )
{
	m_SetType( DocumentValueType::Array );
	m_array.Reserve( reserveLength );
}
DocumentValue& DocumentValue::InsertArrayValue( uint32_t index )
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
DocumentValue& DocumentValue::SetArrayValue( uint32_t index )
{
	AE_ASSERT( IsArray() );
	return *m_array[ index ];
}
DocumentValue& DocumentValue::AppendArrayValue()
{
	AE_ASSERT( IsArray() );
	return InsertArrayValue( GetArrayLength() );
}
void DocumentValue::RemoveArrayValue( uint32_t index )
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
DocumentValue* DocumentValue::GetArrayValue( uint32_t index ) { return AE_CALL_CONST_MEMBER_FUNCTION( GetArrayValue( index ) ); }
const DocumentValue* DocumentValue::GetArrayValue( uint32_t index ) const
{
	AE_ASSERT( IsArray() );
	return ( index < GetArrayLength() ) ? m_array[ index ] : nullptr;
}
bool DocumentValue::IsMap() const { return ( m_type == DocumentValueType::Map ); }
uint32_t DocumentValue::GetMapLength() const
{
	AE_ASSERT( IsMap() );
	return m_map.Length();
}
void DocumentValue::SetMap( uint32_t reserveLength )
{
	m_SetType( DocumentValueType::Map );
	m_map.Reserve( reserveLength );
}
DocumentValue& DocumentValue::SetMapValue( const char* key )
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
void DocumentValue::RemoveMapValue( const char* key )
{
	AE_ASSERT( IsMap() );
	DocumentValue* value = nullptr;
	const bool found = m_map.Remove( key, &value );
	AE_DEBUG_ASSERT( found );

	UndoOp op;
	op.type = UndoOpType::SetMapValue; // Reverse operation
	op.target = this;
	op.key = key;
	op.oldChild = value;
	m_document->m_PushOp( op );

	// Increment reference count - now only kept alive by undo stack
	m_document->m_AddRef( value );
}
DocumentValue* DocumentValue::GetMapValue( const char* key ) { return AE_CALL_CONST_MEMBER_FUNCTION( GetMapValue( key ) ); }
DocumentValue* DocumentValue::GetMapValue( uint32_t index ) { return AE_CALL_CONST_MEMBER_FUNCTION( GetMapValue( index ) ); }
const DocumentValue* DocumentValue::GetMapValue( const char* key ) const
{
	AE_ASSERT( IsMap() );
	return m_map.Get( key, nullptr );
}
const DocumentValue* DocumentValue::GetMapValue( uint32_t index ) const
{
	AE_ASSERT( IsMap() );
	return m_map.GetValue( index );
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
	REQUIRE( doc.GetArrayLength() == 0 );
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
	doc.AppendArrayValue().SetBasicValue( "first" );
	doc.AppendArrayValue().SetBasicValue( "second" );
	doc.AppendArrayValue().SetBasicValue( "third" );
	doc.EndUndoGroup();

	REQUIRE( doc.GetArrayLength() == 3 );
	REQUIRE( doc.GetArrayValue( 0 )->GetBasicValue() == std::string( "first" ) );
	REQUIRE( doc.GetArrayValue( 1 )->GetBasicValue() == std::string( "second" ) );
	REQUIRE( doc.GetArrayValue( 2 )->GetBasicValue() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Insert in middle
	doc.InsertArrayValue( 1 ).SetBasicValue( "inserted" );
	doc.EndUndoGroup();

	REQUIRE( doc.GetArrayLength() == 4 );
	REQUIRE( doc.GetArrayValue( 0 )->GetBasicValue() == std::string( "first" ) );
	REQUIRE( doc.GetArrayValue( 1 )->GetBasicValue() == std::string( "inserted" ) );
	REQUIRE( doc.GetArrayValue( 2 )->GetBasicValue() == std::string( "second" ) );
	REQUIRE( doc.GetArrayValue( 3 )->GetBasicValue() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo insert
	REQUIRE( doc.Undo() );
	REQUIRE( doc.GetArrayLength() == 3 );
	REQUIRE( doc.GetArrayValue( 0 )->GetBasicValue() == std::string( "first" ) );
	REQUIRE( doc.GetArrayValue( 1 )->GetBasicValue() == std::string( "second" ) );
	REQUIRE( doc.GetArrayValue( 2 )->GetBasicValue() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Remove element
	doc.RemoveArrayValue( 1 );
	doc.EndUndoGroup();

	REQUIRE( doc.GetArrayLength() == 2 );
	REQUIRE( doc.GetArrayValue( 0 )->GetBasicValue() == std::string( "first" ) );
	REQUIRE( doc.GetArrayValue( 1 )->GetBasicValue() == std::string( "third" ) );
	REQUIRE( doc.GetUndoStackSize() == 3 ); // Redo stack was cleared, new operation added
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo remove should restore element at correct position
	REQUIRE( doc.Undo() );
	REQUIRE( doc.GetArrayLength() == 3 );
	REQUIRE( doc.GetArrayValue( 0 )->GetBasicValue() == std::string( "first" ) );
	REQUIRE( doc.GetArrayValue( 1 )->GetBasicValue() == std::string( "second" ) );
	REQUIRE( doc.GetArrayValue( 2 )->GetBasicValue() == std::string( "third" ) );
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
	doc.SetMapValue( "key1" ).SetBasicValue( "value1" );
	doc.SetMapValue( "key2" ).SetBasicValue( "value2" );
	doc.EndUndoGroup();

	REQUIRE( doc.GetMapLength() == 2 );
	REQUIRE( doc.GetMapValue( "key1" )->GetBasicValue() == std::string( "value1" ) );
	REQUIRE( doc.GetMapValue( "key2" )->GetBasicValue() == std::string( "value2" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Remove a key
	doc.RemoveMapValue( "key1" );
	doc.EndUndoGroup();

	REQUIRE( doc.GetMapLength() == 1 );
	REQUIRE( doc.GetMapValue( "key1" ) == nullptr );
	REQUIRE( doc.GetMapValue( "key2" )->GetBasicValue() == std::string( "value2" ) );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo remove should restore key and value
	REQUIRE( doc.Undo() );
	REQUIRE( doc.GetMapLength() == 2 );
	REQUIRE( doc.GetMapValue( "key1" )->GetBasicValue() == std::string( "value1" ) );
	REQUIRE( doc.GetMapValue( "key2" )->GetBasicValue() == std::string( "value2" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Redo remove
	REQUIRE( doc.Redo() );
	REQUIRE( doc.GetMapLength() == 1 );
	REQUIRE( doc.GetMapValue( "key1" ) == nullptr );
	REQUIRE( doc.GetUndoStackSize() == 3 );
	REQUIRE( doc.GetRedoStackSize() == 0 );
}

TEST_CASE( "DocumentUndo UndoGroups", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	doc.SetArray( 4 );

	// Multiple operations in one group
	doc.AppendArrayValue().SetBasicValue( "first" );
	doc.AppendArrayValue().SetBasicValue( "second" );
	doc.AppendArrayValue().SetBasicValue( "third" );
	doc.EndUndoGroup(); // All three operations in one group

	REQUIRE( doc.GetArrayLength() == 3 );
	REQUIRE( doc.GetUndoStackSize() == 1 ); // Single group with multiple operations
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Single undo should reverse entire group
	REQUIRE( doc.Undo() );
	REQUIRE( doc.GetValueType() == ae::DocumentValueType::None ); // Back to empty document
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Single redo should restore entire group
	REQUIRE( doc.Redo() );
	REQUIRE( doc.GetArrayLength() == 3 );
	REQUIRE( doc.GetArrayValue( 0 )->GetBasicValue() == std::string( "first" ) );
	REQUIRE( doc.GetArrayValue( 1 )->GetBasicValue() == std::string( "second" ) );
	REQUIRE( doc.GetArrayValue( 2 )->GetBasicValue() == std::string( "third" ) );
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
	REQUIRE( doc.GetValueType() == ae::DocumentValueType::None );
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
	REQUIRE( doc.GetValueType() == ae::DocumentValueType::None );
	REQUIRE( doc.GetUndoStackSize() == 0 );
	REQUIRE( doc.GetRedoStackSize() == 3 );
}

TEST_CASE( "DocumentUndo ComplexNesting", "[ae::Document][undo]" )
{
	ae::Document doc( "test" );
	doc.SetArray( 4 );

	// Create nested structure
	doc.AppendArrayValue().SetMap( 2 );
	doc.GetArrayValue( 0 )->SetMapValue( "nested" ).SetBasicValue( "deep" );
	doc.EndUndoGroup();

	REQUIRE( doc.GetArrayLength() == 1 );
	REQUIRE( doc.GetArrayValue( 0 )->IsMap() );
	REQUIRE( doc.GetArrayValue( 0 )->GetMapValue( "nested" )->GetBasicValue() == std::string( "deep" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Modify nested value
	doc.GetArrayValue( 0 )->GetMapValue( "nested" )->SetBasicValue( "modified" );
	doc.EndUndoGroup();

	REQUIRE( doc.GetArrayValue( 0 )->GetMapValue( "nested" )->GetBasicValue() == std::string( "modified" ) );
	REQUIRE( doc.GetUndoStackSize() == 2 );
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should restore deep nested value
	REQUIRE( doc.Undo() );
	REQUIRE( doc.GetArrayValue( 0 )->GetMapValue( "nested" )->GetBasicValue() == std::string( "deep" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 1 );

	// Remove entire nested structure
	doc.RemoveArrayValue( 0 );
	doc.EndUndoGroup();

	REQUIRE( doc.GetArrayLength() == 0 );
	REQUIRE( doc.GetUndoStackSize() == 2 ); // Redo stack was cleared
	REQUIRE( doc.GetRedoStackSize() == 0 );

	// Undo should restore entire nested structure
	REQUIRE( doc.Undo() );
	REQUIRE( doc.GetArrayLength() == 1 );
	REQUIRE( doc.GetArrayValue( 0 )->IsMap() );
	REQUIRE( doc.GetArrayValue( 0 )->GetMapValue( "nested" )->GetBasicValue() == std::string( "deep" ) );
	REQUIRE( doc.GetUndoStackSize() == 1 );
	REQUIRE( doc.GetRedoStackSize() == 1 );
}
