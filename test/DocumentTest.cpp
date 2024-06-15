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
// ae::DocumentValue
//------------------------------------------------------------------------------
class DocumentValue
{
public:
	DocumentValue( class Document* document );
	virtual ~DocumentValue() {}

	// Basic value
	bool IsBasicValue() const;
	void SetBasicValue( const char* value );
	const char* GetBasicValue() const;

	// Array
	bool IsArray() const;
	uint32_t GetArrayLength() const;
	void SetArray( uint32_t reserveLength );
	void InsertArrayValue( uint32_t index, DocumentValue* value );
	void SetArrayValue( uint32_t index, DocumentValue* value );
	void RemoveArrayValue( uint32_t index );
	DocumentValue* GetArrayValue( uint32_t index );
	const DocumentValue* GetArrayValue( uint32_t index ) const;

	// Map
	bool IsMap() const;
	uint32_t GetMapLength() const;
	void SetMap( uint32_t reserveLength );
	void SetMapValue( const char* key, DocumentValue* value );
	void RemoveMapValue( const char* key );
	DocumentValue* GetMapValue( const char* key );
	DocumentValue* GetMapValue( uint32_t index );
	const DocumentValue* GetMapValue( const char* key ) const;
	const DocumentValue* GetMapValue( uint32_t index ) const;

protected:
	friend class Document;
	Document* m_document = nullptr;
};

//------------------------------------------------------------------------------
// ae::Document
//------------------------------------------------------------------------------
class Document : public DocumentValue
{
public:
	Document( const ae::Tag& tag );

	void EndUndoGroup();
	void ClearUndo();
	bool Undo();
	bool Redo();

private:
	friend class DocumentValue;
	const ae::Tag m_tag;
	ae::ObjectPool< DocumentValue, 64, true > m_values;
};

//------------------------------------------------------------------------------
// ae::DocumentValue member functions
//------------------------------------------------------------------------------
DocumentValue::DocumentValue( Document* document ) : m_document( document ) {}

bool DocumentValue::IsBasicValue() const
{
	// TODO: Implement IsBasicValue
	return false;
}

void DocumentValue::SetBasicValue( const char* value )
{
	// TODO: Implement SetBasicValue
}

const char* DocumentValue::GetBasicValue() const
{
	// TODO: Implement GetBasicValue
	return nullptr;
}

bool DocumentValue::IsArray() const
{
	// TODO: Implement IsArray
	return false;
}

uint32_t DocumentValue::GetArrayLength() const
{
	// TODO: Implement GetArrayLength
	return 0;
}

void DocumentValue::SetArray( uint32_t reserveLength )
{
	// TODO: Implement SetArray
}

void DocumentValue::InsertArrayValue( uint32_t index, DocumentValue* value )
{
	// TODO: Implement InsertArrayValue
}

void DocumentValue::SetArrayValue( uint32_t index, DocumentValue* value )
{
	// TODO: Implement SetArrayValue
}

void DocumentValue::RemoveArrayValue( uint32_t index )
{
	// TODO: Implement RemoveArrayValue
}

DocumentValue* DocumentValue::GetArrayValue( uint32_t index )
{
	// TODO: Implement GetArrayValue
	return nullptr;
}

const DocumentValue* DocumentValue::GetArrayValue( uint32_t index ) const
{
	// TODO: Implement GetArrayValue
	return nullptr;
}

bool DocumentValue::IsMap() const
{
	// TODO: Implement IsMap
	return false;
}

uint32_t DocumentValue::GetMapLength() const
{
	// TODO: Implement GetMapLength
	return 0;
}

void DocumentValue::SetMap( uint32_t reserveLength )
{
	// TODO: Implement SetMap
}

void DocumentValue::SetMapValue( const char* key, DocumentValue* value )
{
	// TODO: Implement SetMapValue
}

void DocumentValue::RemoveMapValue( const char* key )
{
	// TODO: Implement RemoveMapValue
}

DocumentValue* DocumentValue::GetMapValue( const char* key )
{
	// TODO: Implement GetMapValue
	return nullptr;
}

DocumentValue* DocumentValue::GetMapValue( uint32_t index )
{
	// TODO: Implement GetMapValue
	return nullptr;
}

const DocumentValue* DocumentValue::GetMapValue( const char* key ) const
{
	// TODO: Implement GetMapValue
	return nullptr;
}

const DocumentValue* DocumentValue::GetMapValue( uint32_t index ) const
{
	// TODO: Implement GetMapValue
	return nullptr;
}

//------------------------------------------------------------------------------
// ae::Document member functions
//------------------------------------------------------------------------------
Document::Document( const ae::Tag& tag ) : DocumentValue( this ), m_tag( tag ), m_values( tag ) {}

void Document::EndUndoGroup()
{
	// TODO: Implement EndUndoGroup
}

void Document::ClearUndo()
{
	// TODO: Implement ClearUndo
}

bool Document::Undo()
{
	// TODO: Implement Undo
	return false;
}

bool Document::Redo()
{
	// TODO: Implement Redo
	return false;
}

} // namespace ae

TEST_CASE( "DocumentValue", "[ae::DocumentValue]" )
{
	ae::Document doc( "test" );
	ae::DocumentValue* value = doc.GetArrayValue( 0 );
	REQUIRE( value == nullptr );
}
