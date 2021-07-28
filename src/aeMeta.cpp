//------------------------------------------------------------------------------
// aeMeta.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2020 John Hughes
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
#include "aeMeta.h"

//------------------------------------------------------------------------------
// Meta register base object
//------------------------------------------------------------------------------
AE_META_CLASS( aeObject );

uint32_t aeMeta::GetTypeCount()
{
  return (uint32_t)m_GetTypes().size();
}

const aeMeta::Type* aeMeta::GetTypeByIndex( uint32_t i )
{
  return m_GetTypes()[ i ];
}

const aeMeta::Type* aeMeta::GetTypeById( aeMetaTypeId id )
{
  return m_GetTypeIdMap()[ id ];
}

const aeMeta::Type* aeMeta::GetTypeByName( const char* typeName )
{
  auto it = m_GetTypeNameMap().find( typeName );
  if ( it != m_GetTypeNameMap().end() ) { return it->second; }
  else { return nullptr; }
}

const aeMeta::Type* aeMeta::GetTypeFromObject( const aeObject& obj )
{
  return GetTypeFromObject( &obj );
}

const aeMeta::Enum* aeMeta::GetEnum( const char* enumName )
{
  return Enum::s_Get( enumName, false, 0 , false );
}

const aeMeta::Type* aeMeta::GetTypeFromObject( const aeObject* obj )
{
  if ( !obj )
  {
    return nullptr;
  }
  
  aeMetaTypeId id = aeMetaGetObjectTypeId( obj );
  auto it = m_GetTypeIdMap().find( id );
  if ( it != m_GetTypeIdMap().end() )
  {
    return it->second;
  }
  else
  {
    AE_ASSERT_MSG( false, "No meta info for object '#' type id: #", obj, (uint32_t)id );
    return nullptr;
  }
}

bool aeMeta::Var::SetObjectValueFromString( aeObject* obj, const char* value, std::function< bool( const aeMeta::Type*, const char*, aeObject** ) > getObjectPointerFromString ) const
{
  if ( !obj )
  {
    return false;
  }
  
  // Safety check to make sure 'this' Var belongs to 'obj' aeMeta::Type
  const aeMeta::Type* objType = aeMeta::GetTypeFromObject( obj );
  AE_ASSERT( objType );
  AE_ASSERT_MSG( objType == m_owner, "Attempting to modify object '#' with var '#::#'", objType->GetName(), m_owner->GetName(), GetName() );
  
  void* varData = (uint8_t*)obj + m_offset;
  
  switch ( m_type )
  {
    case Var::String:
    {
      switch ( m_size )
      {
        case 16:
        {
          *(ae::Str16*)varData = value;
          return true;
        }
        case 32:
        {
          *(ae::Str32*)varData = value;
          return true;
        }
        case 64:
        {
          ae::Str64* str = (ae::Str64*)varData;
          *str = value;
          return true;
        }
        case 128:
        {
          ae::Str128* str = (ae::Str128*)varData;
          *str = value;
          return true;
        }
        case 256:
        {
          ae::Str256* str = (ae::Str256*)varData;
          *str = value;
          return true;
        }
        case 512:
        {
          ae::Str512* str = (ae::Str512*)varData;
          *str = value;
          return true;
        }
        default:
        {
          AE_ASSERT_MSG( false, "Invalid string size '#'", m_size );
          return false;
        }
      }
    }
    case Var::UInt8:
    {
      AE_ASSERT(m_size == sizeof(uint8_t) );
      uint8_t* u8 = (uint8_t*)varData;
      sscanf( value, "%hhu", u8 );
      return true;
    }
    case Var::UInt16:
    {
      AE_ASSERT(m_size == sizeof(uint16_t) );
      uint16_t* u16 = (uint16_t*)varData;
      sscanf( value, "%hu", u16 );
      return true;
    }
    case Var::UInt32:
    {
      AE_ASSERT(m_size == sizeof(uint32_t) );
      uint32_t* u32 = (uint32_t*)varData;
      sscanf( value, "%u", u32 );
      return true;
    }
    case Var::Int8:
    {
      AE_ASSERT(m_size == sizeof(int8_t) );
      int8_t* i8 = (int8_t*)varData;
      sscanf( value, "%hhd", i8 );
      return true;
    }
    case Var::Int16:
    {
      AE_ASSERT(m_size == sizeof(int16_t) );
      int16_t* i16 = (int16_t*)varData;
      sscanf( value, "%hd", i16 );
      return true;
    }
    case Var::Int32:
    {
      AE_ASSERT(m_size == sizeof(int32_t) );
      int32_t* i32 = (int32_t*)varData;
      sscanf( value, "%d", i32 );
      return true;
    }
    case Var::Bool:
    {
      // @TODO: Clean this up. Should check for both `true` and `false`, and return false if neither match
      const char* trueStr = "true";
      bool b = value[ 0 ];
      if ( b )
      {
        for ( uint32_t i = 0; ( value[ i ] && trueStr[ i ] ); i++ )
        {
          if ( trueStr[ i ] != tolower( value[ i ] ) )
          {
            b = false;
          }
        }
      }
      *(bool*)varData = b;
      return true;
    }
    case Var::Float:
    {
      AE_ASSERT( m_size == sizeof(float) );
      float* f = (float*)varData;
      sscanf( value, "%f", f );
      return true;
    }
      // case Var::V2f:
      // {
      //   AE_ASSERT( var->m_size == sizeof(v2f) );
      //   v2f* v = (v2f*)varData;
      //   sscanf( value, "%f %f", &(v->X), &(v->Y) );
      //   return true;
      // }
      // case Var::V2i:
      // {
      //   AE_ASSERT( var->m_size == sizeof(v2i) );
      //   v2i* v = (v2i*)varData;
      //   sscanf( value, "%d %d", &(v->X), &(v->Y) );
      //   return true;
      // }
    case Var::Matrix4:
    {
      AE_ASSERT( m_size == sizeof(ae::Matrix4) );
      ae::Matrix4* v = (ae::Matrix4*)varData;
      // @TODO: Should match GetObjectValueAsString() which uses ae::Str::Format
      sscanf( value, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
             v->d, v->d + 1, v->d + 2, v->d + 3,
             v->d + 4, v->d + 5, v->d + 6, v->d + 7,
             v->d + 8, v->d + 9, v->d + 10, v->d + 11,
             v->d + 12, v->d + 13, v->d + 14, v->d + 15 );
      return true;
    }
    case Var::Enum:
    {
      if ( !value[ 0 ] )
      {
        return false;
      }
      
      const class Enum* enumType = GetEnum();
      
      if ( enumType->TypeIsSigned() )
      {
        switch ( enumType->TypeSize() )
        {
        case 1:
          return enumType->GetValueFromString( value, reinterpret_cast< int8_t* >( varData ) );
        case 2:
          return enumType->GetValueFromString( value, reinterpret_cast< int16_t* >( varData ) );
        case 4:
          return enumType->GetValueFromString( value, reinterpret_cast< int32_t* >( varData ) );
        case 8:
          return enumType->GetValueFromString( value, reinterpret_cast< int64_t* >( varData ) );
        default:
          AE_FAIL();
          return false;
        }
      }
      else
      {
        switch ( enumType->TypeSize() )
        {
        case 1:
          return enumType->GetValueFromString( value, reinterpret_cast< uint8_t* >( varData ) );
        case 2:
          return enumType->GetValueFromString( value, reinterpret_cast< uint16_t* >( varData ) );
        case 4:
          return enumType->GetValueFromString( value, reinterpret_cast< uint32_t* >( varData ) );
        case 8:
          return enumType->GetValueFromString( value, reinterpret_cast< uint64_t* >( varData ) );
        default:
          AE_FAIL();
          return false;
        }
      }
      return false;
    }
    case Var::Ref:
    {
      const aeMeta::Type* refType = GetRefType();
      AE_ASSERT_MSG( getObjectPointerFromString, "Must provide mapping function for reference types when calling SetObjectValueFromString" );
      
      class aeObject* obj = nullptr;
      if ( getObjectPointerFromString( refType, value, &obj ) )
      {
        if ( obj )
        {
          const aeMeta::Type* objType = aeMeta::GetTypeFromObject( obj );
          AE_ASSERT( objType );
          AE_ASSERT_MSG( objType->IsType( refType ), "SetObjectValueFromString for var '#::#' returned object with wrong type '#'", m_owner->GetName(), GetName(), objType->GetName() );
        }
        class aeObject** varPtr = reinterpret_cast< class aeObject** >( varData );
        *varPtr = obj;
        return true;
      }
      return false;
    }
  }
  
  return false;
}

bool aeMeta::Var::SetObjectValue( aeObject* obj, const aeObject* value ) const
{
  AE_ASSERT( m_type == Ref );
  
  if ( !obj )
  {
    return false;
  }
  
  const aeMeta::Type* objType = aeMeta::GetTypeFromObject( obj );
  AE_ASSERT( objType );
  AE_ASSERT_MSG( objType->IsType( m_owner ), "Attempting to set var on '#' with unrelated type '#'", objType->GetName(), m_owner->GetName() );
  
  if ( !value )
  {
    memset( (uint8_t*)obj + m_offset, 0, m_size );
    return true;
  }
  
  const aeMeta::Type* refType = GetRefType();
  const aeMeta::Type* valueType = aeMeta::GetTypeFromObject( value );
  AE_ASSERT( valueType );
  AE_ASSERT_MSG( valueType->IsType( refType ), "Attempting to set ref type '#' with unrelated type '#'", refType->GetName(), valueType->GetName() );
  
  uint8_t* target = (uint8_t*)obj + m_offset;
  const aeObject*& varData = *reinterpret_cast< const aeObject** >( target );
  varData = value;
  
  return true;
}

const aeMeta::Type* aeMeta::Type::GetBaseType() const
{
  return GetTypeByName( m_parent.c_str() );
}

bool aeMeta::Type::IsType( const Type* otherType ) const
{
  AE_ASSERT( otherType );
  for ( const aeMeta::Type* baseType = this; baseType; baseType = baseType->GetBaseType() )
  {
    if ( baseType == otherType )
    {
      return true;
    }
  }
  return false;
}

const class aeMeta::Enum* aeMeta::Var::GetEnum() const
{
  if ( !m_enum )
  {
    if ( m_type != Var::Enum )
    {
      return nullptr;
    }
    m_enum = aeMeta::GetEnum( m_typeName.c_str() );
  }
  return m_enum;
}

const aeMeta::Type* aeMeta::Var::GetRefType() const
{
  if ( m_refTypeId == kAeInvalidMetaTypeId )
  {
    return nullptr;
  }
  const aeMeta::Type* type = GetTypeById( m_refTypeId );
  AE_ASSERT( type );
  return type;
}

//------------------------------------------------------------------------------
// Internal aeObject functions
//------------------------------------------------------------------------------
aeMeta::aeMetaTypeId aeMeta::aeMetaGetObjectTypeId( const aeObject* obj )
{
  return obj ? obj->_metaTypeId : aeMeta::kAeInvalidMetaTypeId;
}

aeMeta::aeMetaTypeId aeMeta::aeMetaGetTypeIdFromName( const char* name )
{
  // @TODO: Look into https://en.cppreference.com/w/cpp/types/type_info/hash_code
  return name[ 0 ] ? ae::Hash().HashString( name ).Get() : aeMeta::kAeInvalidMetaTypeId;
}

std::map< ae::Str32, aeMeta::Type* >& aeMeta::m_GetTypeNameMap()
{
  static std::map< ae::Str32, Type* > s_map;
  return s_map;
}

std::map< aeMeta::aeMetaTypeId, aeMeta::Type* >& aeMeta::m_GetTypeIdMap()
{
  static std::map< aeMetaTypeId, Type* > s_map;
  return s_map;
}

std::vector< aeMeta::Type* >& aeMeta::m_GetTypes()
{
  static std::vector< aeMeta::Type* > s_vec;
  return s_vec;
}

int32_t aeMeta::Enum::GetValueByIndex( int32_t index ) const { return m_enumValueToName.GetKey( index ); }
std::string aeMeta::Enum::GetNameByIndex( int32_t index ) const { return m_enumValueToName.GetValue( index ); }
uint32_t aeMeta::Enum::Length() const { return m_enumValueToName.Length(); }

aeMeta::Enum::Enum( const char* name, uint32_t size, bool isSigned ) :
  m_name( name ),
  m_size( size ),
  m_isSigned( isSigned )
{}

void aeMeta::Enum::m_AddValue( const char* name, int32_t value )
{
  m_enumValueToName.Set( value, name );
  m_enumNameToValue.Set( name, value );
}

aeMeta::Enum* aeMeta::Enum::s_Get( const char* enumName, bool create, uint32_t size, bool isSigned )
{
  static ae::Map< std::string, Enum* > enums = AE_ALLOC_TAG_META;
  if ( create )
  {
    AE_ASSERT( !enums.TryGet( enumName ) );
    return enums.Set( enumName, ae::New< Enum >( AE_ALLOC_TAG_META, enumName, size, isSigned ) );
  }
  else
  {
    Enum* metaEnum = enums.Get( enumName, nullptr );
    AE_ASSERT_MSG( metaEnum, "Could not find meta registered Enum named '#'", enumName );
    return metaEnum;
  }
}

// @TODO: Replace return type with a dynamic ae::Str
std::string aeMeta::Var::GetObjectValueAsString( const aeObject* obj, std::function< std::string( const aeObject* ) > getStringFromObjectPointer ) const
{
  if ( !obj )
  {
    return "";
  }
  
  // @TODO: Add debug safety check to make sure 'this' Var belongs to 'obj' aeMeta::Type
  
  const void* varData = reinterpret_cast< const uint8_t* >( obj ) + m_offset;
  
  switch ( m_type )
  {
    case Var::String:
      switch ( m_size )
      {
        case 16:
          return reinterpret_cast< const ae::Str16* >( varData )->c_str();
        case 32:
          return reinterpret_cast< const ae::Str32* >( varData )->c_str();
        case 64:
          return reinterpret_cast< const ae::Str64* >( varData )->c_str();
        case 128:
          return reinterpret_cast< const ae::Str128* >( varData )->c_str();
        case 256:
          return reinterpret_cast< const ae::Str256* >( varData )->c_str();
        case 512:
          return reinterpret_cast< const ae::Str512* >( varData )->c_str();
        default:
          AE_FAIL_MSG( "Invalid string size '#'", m_size );
          return "";
      }
    case Var::UInt8:
      // Prevent char formatting
      return ae::Str32::Format( "#", (uint32_t)*reinterpret_cast< const uint8_t* >( varData ) ).c_str();
    case Var::UInt16:
      return ae::Str32::Format( "#", *reinterpret_cast< const uint16_t* >( varData ) ).c_str();
    case Var::UInt32:
      return ae::Str32::Format( "#", *reinterpret_cast< const uint32_t* >( varData ) ).c_str();
    case Var::Int8:
      // Prevent char formatting
      return ae::Str32::Format( "#", (int32_t)*reinterpret_cast< const int8_t* >( varData ) ).c_str();
    case Var::Int16:
      return ae::Str32::Format( "#", *reinterpret_cast< const int16_t* >( varData ) ).c_str();
    case Var::Int32:
      return ae::Str32::Format( "#", *reinterpret_cast< const int32_t* >( varData ) ).c_str();
    case Var::Bool:
      return ae::Str32::Format( "#", *reinterpret_cast< const bool* >( varData ) ).c_str();
    case Var::Float:
      return ae::Str32::Format( "#", *reinterpret_cast< const float* >( varData ) ).c_str();
    case Var::Matrix4:
      return ae::Str256::Format( "#", *reinterpret_cast< const ae::Matrix4* >( varData ) ).c_str();
    case Var::Enum:
    {
      const class Enum* enumType = GetEnum();
      int32_t value = 0;
      switch ( enumType->TypeSize() )
      {
        case 1: value = *reinterpret_cast< const int8_t* >( varData ); break;
        case 2: value = *reinterpret_cast< const int16_t* >( varData ); break;
        case 4: value = *reinterpret_cast< const int32_t* >( varData ); break;
        case 8: value = *reinterpret_cast< const int64_t* >( varData ); break;
        default: AE_FAIL();
      }
      return enumType->GetNameByValue( value );
    }
    case Var::Ref:
    {
      AE_ASSERT_MSG( getStringFromObjectPointer, "Must provide mapping function for reference types when calling GetObjectValueAsString" );
      const aeObject* obj = *reinterpret_cast< const aeObject* const * >( varData );
      return getStringFromObjectPointer( obj ).c_str();
    }
  }
  
  return "";
}
