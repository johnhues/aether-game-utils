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
// @TODO: Support registering classes in namespaces
//AE_META_CLASS( ae::Object );
int force_link_aeObject = 0;
template <> const char* ae::_TypeName< ae::Object >::Get() { return "ae::Object"; }
template <> void ae::_DefineType< ae::Object >( ae::Type *type, uint32_t index ) { type->Init< ae::Object >( "ae::Object", index ); }
static ae::_TypeCreator< ae::Object > ae_type_creator_aeObject( "ae::Object" );

uint32_t ae::GetTypeCount()
{
  return (uint32_t)_GetTypes().size();
}

const ae::Type* ae::GetTypeByIndex( uint32_t i )
{
  return _GetTypes()[ i ];
}

const ae::Type* ae::GetTypeById( ae::TypeId id )
{
  return _GetTypeIdMap()[ id ];
}

const ae::Type* ae::GetTypeByName( const char* typeName )
{
  auto it = _GetTypeNameMap().find( typeName );
  if ( it != _GetTypeNameMap().end() ) { return it->second; }
  else { return nullptr; }
}

const ae::Type* ae::GetTypeFromObject( const ae::Object& obj )
{
  return GetTypeFromObject( &obj );
}

const ae::Enum* ae::GetEnum( const char* enumName )
{
  return Enum::s_Get( enumName, false, 0 , false );
}

const ae::Type* ae::GetTypeFromObject( const ae::Object* obj )
{
  if ( !obj )
  {
    return nullptr;
  }
  
  ae::TypeId id = GetObjectTypeId( obj );
  auto it = _GetTypeIdMap().find( id );
  if ( it != _GetTypeIdMap().end() )
  {
    return it->second;
  }
  else
  {
    AE_ASSERT_MSG( false, "No meta info for object '#' type id: #", obj, (uint32_t)id );
    return nullptr;
  }
}

const char* ae::Var::GetName() const { return m_name.c_str(); }
ae::Var::Type ae::Var::GetType() const { return m_type; }
const char* ae::Var::GetTypeName() const { return m_typeName.c_str(); }
uint32_t ae::Var::GetOffset() const { return m_offset; }
uint32_t ae::Var::GetSize() const { return m_size; }

bool ae::Var::SetObjectValueFromString( ae::Object* obj, const char* value, std::function< bool( const ae::Type*, const char*, ae::Object** ) > getObjectPointerFromString ) const
{
  if ( !obj )
  {
    return false;
  }
  
  // Safety check to make sure 'this' Var belongs to 'obj' ae::Type
  const ae::Type* objType = ae::GetTypeFromObject( obj );
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
      const ae::Type* refType = GetRefType();
      AE_ASSERT_MSG( getObjectPointerFromString, "Must provide mapping function for reference types when calling SetObjectValueFromString" );
      
      class ae::Object* obj = nullptr;
      if ( getObjectPointerFromString( refType, value, &obj ) )
      {
        if ( obj )
        {
          const ae::Type* objType = ae::GetTypeFromObject( obj );
          AE_ASSERT( objType );
          AE_ASSERT_MSG( objType->IsType( refType ), "SetObjectValueFromString for var '#::#' returned object with wrong type '#'", m_owner->GetName(), GetName(), objType->GetName() );
        }
        class ae::Object** varPtr = reinterpret_cast< class ae::Object** >( varData );
        *varPtr = obj;
        return true;
      }
      return false;
    }
  }
  
  return false;
}

bool ae::Var::SetObjectValue( ae::Object* obj, const ae::Object* value ) const
{
  AE_ASSERT( m_type == Ref );
  
  if ( !obj )
  {
    return false;
  }
  
  const ae::Type* objType = ae::GetTypeFromObject( obj );
  AE_ASSERT( objType );
  AE_ASSERT_MSG( objType->IsType( m_owner ), "Attempting to set var on '#' with unrelated type '#'", objType->GetName(), m_owner->GetName() );
  
  if ( !value )
  {
    memset( (uint8_t*)obj + m_offset, 0, m_size );
    return true;
  }
  
  const ae::Type* refType = GetRefType();
  const ae::Type* valueType = ae::GetTypeFromObject( value );
  AE_ASSERT( valueType );
  AE_ASSERT_MSG( valueType->IsType( refType ), "Attempting to set ref type '#' with unrelated type '#'", refType->GetName(), valueType->GetName() );
  
  uint8_t* target = (uint8_t*)obj + m_offset;
  const ae::Object*& varData = *reinterpret_cast< const ae::Object** >( target );
  varData = value;
  
  return true;
}

const ae::Var* ae::Type::GetVarByName( const char* name ) const
{
  int32_t i = m_vars.FindFn( [name]( auto&& v )
  {
    return v.m_name == name;
  } );
  return ( i >= 0 ) ? &m_vars[ i ] : nullptr;
}

const ae::Type* ae::Type::GetBaseType() const
{
  return GetTypeByName( m_parent.c_str() );
}

bool ae::Type::IsType( const Type* otherType ) const
{
  AE_ASSERT( otherType );
  for ( const ae::Type* baseType = this; baseType; baseType = baseType->GetBaseType() )
  {
    if ( baseType == otherType )
    {
      return true;
    }
  }
  return false;
}

const class ae::Enum* ae::Var::GetEnum() const
{
  if ( !m_enum )
  {
    if ( m_type != Var::Enum )
    {
      return nullptr;
    }
    m_enum = ae::GetEnum( m_typeName.c_str() );
  }
  return m_enum;
}

const ae::Type* ae::Var::GetRefType() const
{
  if ( m_refTypeId == kAeInvalidMetaTypeId )
  {
    return nullptr;
  }
  const ae::Type* type = GetTypeById( m_refTypeId );
  AE_ASSERT( type );
  return type;
}

//------------------------------------------------------------------------------
// Internal ae::Object functions
//------------------------------------------------------------------------------
ae::TypeId ae::GetObjectTypeId( const ae::Object* obj )
{
  return obj ? obj->_metaTypeId : ae::kAeInvalidMetaTypeId;
}

ae::TypeId ae::GetTypeIdFromName( const char* name )
{
  // @TODO: Look into https://en.cppreference.com/w/cpp/types/type_info/hash_code
  return name[ 0 ] ? ae::Hash().HashString( name ).Get() : ae::kAeInvalidMetaTypeId;
}

std::map< ae::Str32, ae::Type* >& ae::_GetTypeNameMap()
{
  static std::map< ae::Str32, Type* > s_map;
  return s_map;
}

std::map< ae::TypeId, ae::Type* >& ae::_GetTypeIdMap()
{
  static std::map< ae::TypeId, Type* > s_map;
  return s_map;
}

std::vector< ae::Type* >& ae::_GetTypes()
{
  static std::vector< ae::Type* > s_vec;
  return s_vec;
}

int32_t ae::Enum::GetValueByIndex( int32_t index ) const { return m_enumValueToName.GetKey( index ); }
std::string ae::Enum::GetNameByIndex( int32_t index ) const { return m_enumValueToName.GetValue( index ); }
uint32_t ae::Enum::Length() const { return m_enumValueToName.Length(); }

ae::Enum::Enum( const char* name, uint32_t size, bool isSigned ) :
  m_name( name ),
  m_size( size ),
  m_isSigned( isSigned )
{}

void ae::Enum::m_AddValue( const char* name, int32_t value )
{
  m_enumValueToName.Set( value, name );
  m_enumNameToValue.Set( name, value );
}

ae::Enum* ae::Enum::s_Get( const char* enumName, bool create, uint32_t size, bool isSigned )
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
std::string ae::Var::GetObjectValueAsString( const ae::Object* obj, std::function< std::string( const ae::Object* ) > getStringFromObjectPointer ) const
{
  if ( !obj )
  {
    return "";
  }
  
  // @TODO: Add debug safety check to make sure 'this' Var belongs to 'obj' ae::Type
  
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
      const ae::Object* obj = *reinterpret_cast< const ae::Object* const * >( varData );
      return getStringFromObjectPointer( obj ).c_str();
    }
  }
  
  return "";
}

ae::TypeId ae::Type::GetId() const { return m_id; }
bool ae::Type::HasProperty( const char* prop ) const { return m_props.TryGet( prop ) != nullptr; }
uint32_t ae::Type::GetPropertyCount() const { return m_props.Length(); }
const char* ae::Type::GetPropertyName( uint32_t propIndex ) const { return m_props.GetKey( propIndex ).c_str(); }
uint32_t ae::Type::GetPropertyValueCount( uint32_t propIndex ) const { return m_props.GetValue( propIndex ).Length(); }
uint32_t ae::Type::GetPropertyValueCount( const char* propName ) const { auto* props = m_props.TryGet( propName ); return props ? props->Length() : 0; }
const char* ae::Type::GetPropertyValue( uint32_t propIndex, uint32_t valueIndex ) const { return m_props.GetValue( propIndex )[ valueIndex ].c_str(); }
const char* ae::Type::GetPropertyValue( const char* propName, uint32_t valueIndex ) const { return m_props.Get( propName )[ valueIndex ].c_str(); }
uint32_t ae::Type::GetVarCount() const { return m_vars.Length(); }
const ae::Var* ae::Type::GetVarByIndex( uint32_t i ) const { return &m_vars[ i ]; }
uint32_t ae::Type::GetSize() const { return m_size; }
uint32_t ae::Type::GetAlignment() const { return m_align; }
const char* ae::Type::GetName() const { return m_name.c_str(); }
bool ae::Type::HasNew() const { return m_placementNew; }
bool ae::Type::IsAbstract() const { return m_isAbstract; }
bool ae::Type::IsPolymorphic() const { return m_isPolymorphic; }
bool ae::Type::IsDefaultConstructible() const { return m_isDefaultConstructible; }
const char* ae::Type::GetBaseTypeName() const { return m_parent.c_str(); }

void ae::Type::m_AddProp( const char* prop, const char* value )
{
  auto* props = m_props.TryGet( prop );
  if ( !props )
  {
    props = &m_props.Set( prop, AE_ALLOC_TAG_META );
  }
  if ( value && value[ 0 ] ) // 'm_props' will have an empty array for properties with no value specified
  {
    props->Append( value );
  }
}

void ae::Type::m_AddVar( const Var& var )
{
  m_vars.Append( var );
  std::sort( m_vars.Begin(), m_vars.End(), []( const auto& a, const auto& b )
  {
    return a.GetOffset() < b.GetOffset();
  } );
}
