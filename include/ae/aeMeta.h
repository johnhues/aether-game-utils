//------------------------------------------------------------------------------
// Meta.h
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
#ifndef AEMETA_H
#define AEMETA_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aeMath.h"
#include "aeString.h"
#include <map>
#include <vector>

//------------------------------------------------------------------------------
// Meta constants
//------------------------------------------------------------------------------
class aeObject;
typedef uint32_t aeMetaTypeId;
const aeMetaTypeId kAeInvalidMetaTypeId = 0;
const uint32_t kMaxMetaTypes = 64;
const uint32_t kMaxMetaProps = 8;
const uint32_t kMaxMetaVars = 16;

//------------------------------------------------------------------------------
// Internal function wrappers
//------------------------------------------------------------------------------
template< typename T >
static aeObject* PlacementNewInternal( aeObject* d ) { return new( d ) T(); }

//------------------------------------------------------------------------------
// External macros to force module linking
//------------------------------------------------------------------------------
#define AE_META_CLASS_FORCE_LINK(x) \
extern int force_link_##x; \
struct ForceLink_##x { ForceLink_##x() { force_link_##x = 1; } }; \
ForceLink_##x forceLink_##x;

//------------------------------------------------------------------------------
// Internal meta forward declarations
//------------------------------------------------------------------------------
static aeMetaTypeId aeMetaGetObjectTypeId( const aeObject* obj );
static aeMetaTypeId aeMetaGetTypeIdFromName( const char* name );

//------------------------------------------------------------------------------
// External meta types
//------------------------------------------------------------------------------
class aeMeta
{
public:
  class Type;
  
  class Enum
  {
  public:
    const char* GetName() const { return m_name.c_str(); }
    
    uint32_t TypeSize() const { return m_size; }
    bool TypeIsSigned() const { return m_isSigned; }
    
    template < typename T >
    std::string GetNameByValue( T value ) const { return m_enumValueToName.Get( (int32_t)value, "" ); }
      
    template < typename T >
    bool GetValueFromString( const char* str, T* valueOut ) const
    {
      int32_t value = 0;
      if ( m_enumNameToValue.TryGet( str, &value ) ) // Set object var with named enum value
      {
        *valueOut = (T)value;
        return true;
      }
      else if ( isdigit( str[ 0 ] ) || str[ 0 ] == '-' ) // Set object var with a numerical enum value
      {
        value = atoi( str );
        if ( HasValue( value ) )
        {
          *valueOut = (T)value;
          return true;
        }
      }
      
      return false;
    }
    
    template < typename T >
    bool HasValue( T value ) const { return m_enumValueToName.TryGet( value ); }
    
    int32_t GetValueByIndex( int32_t index ) const { return m_enumValueToName.GetKey( index ); }
    std::string GetNameByIndex( int32_t index ) const { return m_enumValueToName.GetValue( index ); }
    uint32_t Length() const { return m_enumValueToName.Length(); }
    
    template < typename T >
    static std::string GetNameFromValue( T value )
    {
      const Enum* enumType = GetEnum< T >();
      AE_ASSERT( enumType );
      return enumType->m_enumValueToName.Get( (int32_t)value, "" );
    }
    
    template < typename T >
    static T GetValueFromString( const char* str, T defaultValue )
    {
      const Enum* enumType = GetEnum< T >();
      AE_ASSERT_MSG( enumType, "Value '#' has no Enum #", str, typeid(T).name() ); // TODO: Pretty print
      enumType->GetValueFromString( str, &defaultValue );
      return defaultValue;
    }
  
  private:
    aeStr32 m_name;
    uint32_t m_size;
    bool m_isSigned;
    ae::Map< int32_t, std::string > m_enumValueToName = AE_ALLOC_TAG_META;
    ae::Map< std::string, int32_t > m_enumNameToValue = AE_ALLOC_TAG_META;
    
  public: // Internal
    Enum( const char* name, uint32_t size, bool isSigned ) :
      m_name( name ),
      m_size( size ),
      m_isSigned( isSigned )
    {}
    
    void m_AddValue( const char* name, int32_t value )
    {
      m_enumValueToName.Set( value, name );
      m_enumNameToValue.Set( name, value );
    }
    
    static Enum* s_Get( const char* enumName, bool create, uint32_t size, bool isSigned )
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
  };
  
  class Var
  {
  public:
    enum Type
    {
      String,
      UInt8,
      UInt16,
      UInt32,
      Int8,
      Int16,
      Int32,
      Bool,
      Float,
      // V2f,
      // V2i,
      Float4x4,
      Enum,
      Ref
    };

    const char* GetName() const { return m_name.c_str(); }
    Var::Type GetType() const { return m_type; }
    const char* GetTypeName() const { return m_typeName.c_str(); }
    uint32_t GetOffset() const { return m_offset; }
    uint32_t GetSize() const { return m_size; }
    
    // @TODO: Replace return type with an dynamic aeStr
    std::string GetObjectValueAsString( const aeObject* obj, std::function< std::string( const aeObject* ) > getStringFromObjectPointer = nullptr ) const
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
              return reinterpret_cast< const aeStr16* >( varData )->c_str();
            case 32:
              return reinterpret_cast< const aeStr32* >( varData )->c_str();
            case 64:
              return reinterpret_cast< const aeStr64* >( varData )->c_str();
            case 128:
              return reinterpret_cast< const aeStr128* >( varData )->c_str();
            case 256:
              return reinterpret_cast< const aeStr256* >( varData )->c_str();
            case 512:
              return reinterpret_cast< const aeStr512* >( varData )->c_str();
            default:
              AE_FAIL_MSG( "Invalid string size '#'", m_size );
              return "";
          }
        case Var::UInt8:
          // Prevent char formatting
          return aeStr32::Format( "#", (uint32_t)*reinterpret_cast< const uint8_t* >( varData ) ).c_str();
        case Var::UInt16:
          return aeStr32::Format( "#", *reinterpret_cast< const uint16_t* >( varData ) ).c_str();
        case Var::UInt32:
          return aeStr32::Format( "#", *reinterpret_cast< const uint32_t* >( varData ) ).c_str();
        case Var::Int8:
          // Prevent char formatting
          return aeStr32::Format( "#", (int32_t)*reinterpret_cast< const int8_t* >( varData ) ).c_str();
        case Var::Int16:
          return aeStr32::Format( "#", *reinterpret_cast< const int16_t* >( varData ) ).c_str();
        case Var::Int32:
          return aeStr32::Format( "#", *reinterpret_cast< const int32_t* >( varData ) ).c_str();
        case Var::Bool:
          return aeStr32::Format( "#", *reinterpret_cast< const bool* >( varData ) ).c_str();
        case Var::Float:
          return aeStr32::Format( "#", *reinterpret_cast< const float* >( varData ) ).c_str();
        case Var::Float4x4:
          return aeStr256::Format( "#", *reinterpret_cast< const aeFloat4x4* >( varData ) ).c_str();
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
    
    bool SetObjectValueFromString( aeObject* obj, const char* value, std::function< bool( const aeMeta::Type*, const char*, aeObject** ) > getObjectPointerFromString = nullptr ) const
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
              *(aeStr16*)varData = value;
              return true;
            }
            case 32:
            {
              *(aeStr32*)varData = value;
              return true;
            }
            case 64:
            {
              aeStr64* str = (aeStr64*)varData;
              *str = value;
              return true;
            }
            case 128:
            {
              aeStr128* str = (aeStr128*)varData;
              *str = value;
              return true;
            }
            case 256:
            {
              aeStr256* str = (aeStr256*)varData;
              *str = value;
              return true;
            }
            case 512:
            {
              aeStr512* str = (aeStr512*)varData;
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
        case Var::Float4x4:
        {
          AE_ASSERT( m_size == sizeof(aeFloat4x4) );
          aeFloat4x4* v = (aeFloat4x4*)varData;
          // @TODO: Should match GetObjectValueAsString() which uses aeStr::Format
          sscanf( value, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
                 v->data, v->data + 1, v->data + 2, v->data + 3,
                 v->data + 4, v->data + 5, v->data + 6, v->data + 7,
                 v->data + 8, v->data + 9, v->data + 10, v->data + 11,
                 v->data + 12, v->data + 13, v->data + 14, v->data + 15 );
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
    
    bool SetObjectValue( aeObject* obj, const aeObject* value ) const
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
    
    template < typename T >
    bool SetObjectValue( aeObject* obj, const T& value ) const
    {
      AE_ASSERT( m_type != Ref );
      
      if ( !obj )
      {
        return false;
      }
      
      const aeMeta::Type* objType = aeMeta::GetTypeFromObject( obj );
      AE_ASSERT( objType );
      AE_ASSERT_MSG( objType->IsType( m_owner ), "Attempting to set var on '#' with unrelated type '#'", objType->GetName(), m_owner->GetName() );
      
      Var::Type typeCheck = aeMeta::VarType< T >::GetType();
      AE_ASSERT( typeCheck == m_type );
      AE_ASSERT( m_size == sizeof( T ) );
      
      T* varData = reinterpret_cast< T* >( (uint8_t*)obj + m_offset );
      *varData = value;
      
      return true;
    }
    
    // Enum
    const class Enum* GetEnum() const
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
    
    // Ref
    const aeMeta::Type* GetRefType() const
    {
      if ( m_refTypeId == kAeInvalidMetaTypeId )
      {
        return nullptr;
      }
      const aeMeta::Type* type = GetTypeById( m_refTypeId );
      AE_ASSERT( type );
      return type;
    }

    // Members
    const aeMeta::Type* m_owner = nullptr;
    aeStr32 m_name = "";
    Var::Type m_type;
    aeStr32 m_typeName = "";
    uint32_t m_offset = 0;
    uint32_t m_size = 0;
    aeMetaTypeId m_refTypeId = kAeInvalidMetaTypeId; // @TODO: Need to use an id here in case type has not been registered yet
    mutable const class Enum* m_enum = nullptr;
  };
  
  template < typename T >
  struct VarType
  {
    static Var::Type GetType();
    static const char* GetName();
  };

  //------------------------------------------------------------------------------
  // External MetaType class
  //------------------------------------------------------------------------------
  class Type
  {
  public:
    aeMetaTypeId GetId() const { return m_id; }
    
    // Properties
    bool HasProperty( const char* prop ) const { return m_props.TryGet( prop ) != nullptr; }
    uint32_t GetPropertyCount() const { return m_props.Length(); }
    const char* GetPropertyName( uint32_t propIndex ) const { return m_props.GetKey( propIndex ).c_str(); }
    uint32_t GetPropertyValueCount( uint32_t propIndex ) const { return m_props.GetValue( propIndex ).Length(); }
    uint32_t GetPropertyValueCount( const char* propName ) const { return m_props.Get( propName ).Length(); }
    const char* GetPropertyValue( uint32_t propIndex, uint32_t valueIndex ) const
    {
      return m_props.GetValue( propIndex )[ valueIndex ].c_str();
    }
    const char* GetPropertyValue( const char* propName, uint32_t valueIndex ) const
    {
      return m_props.Get( propName )[ valueIndex ].c_str();
    }
    
    // Vars
    uint32_t GetVarCount() const { return m_varCount; }
    const Var* GetVarByIndex( uint32_t i ) const { return &m_vars[ i ]; }
    const Var* GetVarByName( const char* name ) const
    {
      auto* result = std::find_if( m_vars, m_vars + m_varCount, [name]( const auto& v )
      {
        return v.m_name == name;
      });
      if ( result < ( m_vars + m_varCount ) )
      {
        return result;
      }
      return nullptr;
    }

    // C++ type info
    template < typename T = aeObject >
    T* New( void* obj ) const
    {
      AE_ASSERT( obj );
      AE_ASSERT_MSG( !m_isAbstract, "Placement new not available for abstract type: #", m_name.c_str() );
      AE_ASSERT_MSG( m_isDefaultConstructible, "Placement new not available for type without default constructor: #", m_name.c_str() );
      AE_ASSERT( m_placementNew );
      AE_ASSERT( IsType< T >() );
      AE_ASSERT( (uint64_t)obj % GetAlignment() == 0 );
      return (T*)m_placementNew( (T*)obj );
    }
    uint32_t GetSize() const { return m_size; }
    uint32_t GetAlignment() const { return m_align; }
    const char* GetName() const { return m_name.c_str(); }
    bool HasNew() const { return m_placementNew; }
    bool IsAbstract() const { return m_isAbstract; }
    bool IsPolymorphic() const { return m_isPolymorphic; }
    bool IsDefaultConstructible() const { return m_isDefaultConstructible; }

    // Inheritance info
    const char* GetBaseTypeName() const { return m_parent.c_str(); }
    const Type* GetBaseType() const { return GetTypeByName( m_parent.c_str() ); }
    bool IsType( const Type* otherType ) const
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
    template < typename T >
    bool IsType() const
    {
      const Type* type = GetType< T >();
      AE_ASSERT( type );
      return IsType( type );
    }
    
    //------------------------------------------------------------------------------
    // Internal meta type initialization functions
    //------------------------------------------------------------------------------
    template < typename T >
    typename std::enable_if< !std::is_abstract< T >::value && std::is_default_constructible< T >::value, void >::type
    Init( const char* name, uint32_t index )
    {
      m_placementNew = &( PlacementNewInternal< T > );
      m_name = name;
      m_id = aeMetaGetTypeIdFromName( name );
      m_size = sizeof( T );
      m_align = alignof( T );
      m_parent = T::GetBaseTypeName();
      m_isAbstract = false;
      m_isPolymorphic = std::is_polymorphic< T >::value;
      m_isDefaultConstructible = true;
    }
    template < typename T >
    typename std::enable_if< std::is_abstract< T >::value || !std::is_default_constructible< T >::value, void >::type
    Init( const char* name, uint32_t index )
    {
      m_placementNew = nullptr;
      m_name = name;
      m_id = aeMetaGetTypeIdFromName( name );
      m_size = sizeof( T );
      m_align = 0;
      m_parent = T::GetBaseTypeName();
      m_isAbstract = std::is_abstract< T >::value;
      m_isPolymorphic = std::is_polymorphic< T >::value;
      m_isDefaultConstructible = std::is_default_constructible< T >::value;
    }
    
    void AddProp( const char* prop, const char* value )
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

    void AddVar( const Var& var )
    {
      AE_ASSERT( m_varCount < kMaxMetaVars );
      m_vars[ m_varCount++ ] = var;
      std::sort( &m_vars[ 0 ], &m_vars[ m_varCount ], []( const auto& a, const auto& b )
      {
        return a.GetOffset() < b.GetOffset();
      });
    }

    //------------------------------------------------------------------------------
    // Internal meta type state
    //------------------------------------------------------------------------------
  private:
    friend class aeMeta;
    aeObject* ( *m_placementNew )( aeObject* ) = nullptr;
    aeStr32 m_name;
    aeMetaTypeId m_id = kAeInvalidMetaTypeId;
    uint32_t m_size = 0;
    uint32_t m_align = 0;
    ae::Map< ae::Str32, ae::Array< ae::Str32 >, kMaxMetaProps > m_props;
    Var m_vars[ kMaxMetaVars ];
    uint32_t m_varCount = 0;
    aeStr32 m_parent;
    bool m_isAbstract = false;
    bool m_isPolymorphic = false;
    bool m_isDefaultConstructible = false;
  };

  //------------------------------------------------------------------------------
  // External meta functions
  //------------------------------------------------------------------------------
  static uint32_t GetTypeCount() { return (uint32_t)m_GetTypes().size(); }
  static const Type* GetTypeByIndex( uint32_t i )
  {
    return m_GetTypes()[ i ];
  }

  static const Type* GetTypeById( aeMetaTypeId id )
  {
    return m_GetTypeIdMap()[ id ];
  }

  static const Type* GetTypeByName( const char* typeName )
  {
    auto it = m_GetTypeNameMap().find( typeName );
    if ( it != m_GetTypeNameMap().end() ) { return it->second; }
    else { return nullptr; }
  }

  static const Type* GetTypeFromObject( const aeObject* obj )
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

  static const Type* GetTypeFromObject( const aeObject& obj )
  {
    return GetTypeFromObject( &obj );
  }

  template < typename T >
  static const Type* GetType()
  {
    const char* typeName = ae::GetTypeName< T >();
    auto it = m_GetTypeNameMap().find( typeName );
    if ( it != m_GetTypeNameMap().end() )
    {
      return it->second;
    }
    else
    {
      AE_ASSERT_MSG( false, "No meta info for type name: #", typeName );
      return nullptr;
    }
  }
  
  static const Enum* GetEnum( const char* enumName )
  {
    return Enum::s_Get( enumName, false, 0 , false );
  }
  
  //------------------------------------------------------------------------------
  // External meta initialization helpers
  //------------------------------------------------------------------------------
  template < typename T >
  struct TypeName
  {
    static const char* Get();
  };

  //------------------------------------------------------------------------------
  // Internal meta initialization functions
  //------------------------------------------------------------------------------
  template< typename T >
  static void DefineType( Type* type, uint32_t index );

  template < typename T >
  struct TypeCreator
  {
    TypeCreator( const char *typeName )
    {
      static Type type;
      // aeMetaTypeId id = m_GetNextTypeId();
      DefineType< T >( &type, 0 );
      m_GetTypeNameMap()[ typeName ] = &type;
      m_GetTypeIdMap()[ type.GetId() ] = &type; // @TODO: Should check for hash collision
      m_GetTypes().push_back( &type );
    }
  };

  template< typename C, uint32_t N >
  static void DefineVar( Var* var );

  template< typename C, typename V, uint32_t Offset >
  struct VarCreator
  {
    VarCreator( const char* typeName, const char* varName )
    {
      aeMeta::Type* type = m_GetTypeNameMap().find( typeName )->second;
      AE_ASSERT( type );
      
      Var var;
      var.m_owner = type;
      var.m_name = varName;
      var.m_type = aeMeta::VarType< V >::GetType();
      var.m_typeName = aeMeta::VarType< V >::GetName();
      var.m_refTypeId = aeMetaGetTypeIdFromName( aeMeta::VarType< V >::GetRefTypeName() );
#if !_AE_WINDOWS_
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Winvalid-offsetof"
#endif
      var.m_offset = Offset; // @TODO: Verify var is not member of base class
#if !_AE_WINDOWS_
    #pragma clang diagnostic pop
#endif
      var.m_size = sizeof(V);

      type->AddVar( var );
    }
  };
  
  template< typename C >
  struct PropCreator
  {
    PropCreator( const char* typeName, const char* propName, const char* propValue )
    {
      aeMeta::Type* type = m_GetTypeNameMap().find( typeName )->second;
      type->AddProp( propName, propValue );
    }
  };
  
  // @NOTE: Non-specialized GetEnum() has no implementation so templated GetEnum() calls (defined
  // with AE_ENUM, AE_META_ENUM, and AE_META_ENUM_PREFIX) will call the specialized function.
  template < typename T >
  static const Enum* GetEnum();
  
  template < typename E, typename T = typename std::underlying_type< E >::type >
  struct EnumCreator
  {
    EnumCreator( const char* typeName, std::string strMap )
    {
      class aeMeta::Enum* enumType = aeMeta::Enum::s_Get( typeName, true, sizeof( T ), std::is_signed< T >::value );
      
      strMap.erase( std::remove( strMap.begin(), strMap.end(), ' ' ), strMap.end() );
      strMap.erase( std::remove( strMap.begin(), strMap.end(), '(' ), strMap.end() );
      std::vector< std::string > enumTokens( m_SplitString( strMap, ',' ) );

      T currentValue = 0;
      for ( auto iter = enumTokens.begin(); iter != enumTokens.end(); ++iter )
      {
        std::string enumName;
        if ( iter->find( '=' ) == std::string::npos )
        {
          enumName = *iter;
        }
        else
        {
          std::vector<std::string> enumNameValue( m_SplitString( *iter, '=' ) );
          enumName = enumNameValue[ 0 ];
          if ( std::is_unsigned< T >::value )
          {
            currentValue = static_cast< T >( std::stoull( enumNameValue[ 1 ], 0, 0 ) );
          }
          else
          {
            currentValue = static_cast< T >( std::stoll( enumNameValue[ 1 ], 0, 0 ) );
          }
        }
        
        enumType->m_AddValue( enumName.c_str(), currentValue );
        currentValue++;
      }
    }
    
  private:
    static std::vector< std::string > m_SplitString( std::string str, char separator )
    {
      std::vector< std::string > result;

      std::string item;
      std::stringstream stringStream( str );
      while ( std::getline( stringStream, item, separator ) )
      {
        result.push_back( item );
      }

      return result;
    }
  };
  
  template < typename T >
  class EnumCreator2
  {
  public:
    EnumCreator2( const char* typeName )
    {
      aeMeta::Enum::s_Get( typeName, true, sizeof( T ), std::is_signed< T >::value );
    }
    
    EnumCreator2( const char* valueName, T value )
    {
      const char* prefix = aeMeta::VarType< T >::GetPrefix();
      uint32_t prefixLen = strlen( prefix );
      AE_ASSERT( prefixLen < strlen( valueName ) );
      AE_ASSERT( memcmp( prefix, valueName, prefixLen ) == 0 );
      
      aeMeta::Enum* enumType = const_cast< aeMeta::Enum* >( aeMeta::GetEnum< T >() );
      AE_ASSERT_MSG( enumType, "Could not register enum value '#'. No registered Enum.", valueName );
      enumType->m_AddValue( valueName + prefixLen, (int32_t)value );
    }
  };

  //------------------------------------------------------------------------------
  // Internal meta state
  //------------------------------------------------------------------------------
private:
  // @TODO: Add actual meta type index
  // static aeMetaTypeIndex m_GetNextTypeIndex()
  // {
  //   static aeMetaTypeIndex s_nextTypeId = 0;
  //   s_nextTypeId++;
  //   return s_nextTypeId;
  // }

  static std::map< aeStr32, Type* >& m_GetTypeNameMap()
  {
    static std::map< aeStr32, Type* > s_map;
    return s_map;
  }

  static std::map< aeMetaTypeId, Type* >& m_GetTypeIdMap()
  {
    static std::map< aeMetaTypeId, Type* > s_map;
    return s_map;
  }

  static std::vector< Type* >& m_GetTypes()
  {
    static std::vector< Type* > s_vec;
    return s_vec;
  }
};

//------------------------------------------------------------------------------
// Internal meta var registration
//------------------------------------------------------------------------------
#define DefineMetaVarType( t, e ) \
template <> \
struct aeMeta::VarType< t > { \
static aeMeta::Var::Type GetType() { return aeMeta::Var::e; } \
static const char* GetName() { return #t; } \
static const char* GetRefTypeName() { return ""; } \
};

DefineMetaVarType( uint8_t, UInt8 );
DefineMetaVarType( uint16_t, UInt16 );
DefineMetaVarType( uint32_t, UInt32 );
DefineMetaVarType( int8_t, Int8 );
DefineMetaVarType( int16_t, Int16 );
DefineMetaVarType( int32_t, Int32 );
DefineMetaVarType( bool, Bool );
DefineMetaVarType( float, Float );
// DefineMetaVarType( v2f, V2f );
// DefineMetaVarType( v2i, V2i );
DefineMetaVarType( aeFloat4x4, Float4x4 );

template < uint32_t N >
struct aeMeta::VarType< ae::Str<N> >
{
  static aeMeta::Var::Type GetType() { return aeMeta::Var::String; }
  static const char* GetName() { return "String"; }
  static const char* GetRefTypeName() { return ""; }
};

template < typename T >
struct aeMeta::VarType< T* >
{
  static aeMeta::Var::Type GetType()
  {
    static_assert( std::is_base_of< aeObject, T >::value, "AE_META_VAR refs must have base type aeObject" );
    return aeMeta::Var::Ref;
  }
  static const char* GetName() { return "Ref"; }
  static const char* GetRefTypeName() { return ae::GetTypeName< T >(); }
};

//------------------------------------------------------------------------------
// External meta class registerer
//------------------------------------------------------------------------------
#define AE_META_CLASS( x ) \
int force_link_##x = 0; \
template <> const char* aeMeta::TypeName< x >::Get() { return #x; } \
template <> void aeMeta::DefineType< x >( Type *type, uint32_t index ) { type->Init< x >( #x, index ); } \
static aeMeta::TypeCreator< x > ae_type_creator_##x( #x );

//------------------------------------------------------------------------------
// External meta var registerer
//------------------------------------------------------------------------------
#define AE_META_VAR( c, v ) \
static aeMeta::VarCreator< c, decltype(c::v), offsetof( c, v ) > ae_var_creator_##c##_##v( #c, #v );
//------------------------------------------------------------------------------
// External meta property registerer
//------------------------------------------------------------------------------
#define AE_META_PROPERTY( c, p ) \
static aeMeta::PropCreator< c > ae_prop_creator_##c##_##p( #c, #p, "" );

#define AE_META_PROPERTY_VALUE( c, p, v ) \
static aeMeta::PropCreator< c > ae_prop_creator_##c##_##p_##v( #c, #p, #v );

//------------------------------------------------------------------------------
// External enum definer and registerer
//------------------------------------------------------------------------------
// Define a new enum (must register with AE_ENUM_REGISTER)
#define AE_ENUM( E, T, ... ) \
  enum class E : T { \
    __VA_ARGS__ \
  }; \
  template <> \
  struct aeMeta::VarType< E > { \
    static aeMeta::Var::Type GetType() { return aeMeta::Var::Enum; } \
    static const char* GetName() { return #E; } \
    static const char* GetRefTypeName() { return ""; } \
  }; \
  struct AE_ENUM_##E { AE_ENUM_##E( const char* name = #E, const char* def = #__VA_ARGS__ ); };\
  static std::ostream &operator << ( std::ostream &os, E e ) { \
    os << aeMeta::GetEnum( #E )->GetNameByValue( (int32_t)e ); \
    return os; \
  }

// Register an enum defined with AE_ENUM
#define AE_ENUM_REGISTER( E ) \
  AE_ENUM_##E::AE_ENUM_##E( const char* name, const char* def ) { aeMeta::EnumCreator< E > ec( name, def ); } \
  AE_ENUM_##E ae_enum_creator_##E; \
  template <> const aeMeta::Enum* aeMeta::GetEnum< E >() { static const aeMeta::Enum* e = GetEnum( #E ); return e; }

//------------------------------------------------------------------------------
// External c-style enum registerer
//------------------------------------------------------------------------------
// Register an already defined c-style enum type
#define AE_META_ENUM( E ) \
  template <> \
  struct aeMeta::VarType< E > { \
    static aeMeta::Var::Type GetType() { return aeMeta::Var::Enum; } \
    static const char* GetName() { return #E; } \
    static const char* GetRefTypeName() { return ""; } \
    static const char* GetPrefix() { return ""; } \
  }; \
  aeMeta::EnumCreator2< E > ae_enum_creator_##E( #E ); \
  template <> const aeMeta::Enum* aeMeta::GetEnum< E >() { static const aeMeta::Enum* e = GetEnum( #E ); return e; }

// Register an already defined c-style enum type where each value has a prefix
#define AE_META_ENUM_PREFIX( E, PREFIX ) \
  template <> \
  struct aeMeta::VarType< E > { \
    static aeMeta::Var::Type GetType() { return aeMeta::Var::Enum; } \
    static const char* GetName() { return #E; } \
    static const char* GetRefTypeName() { return ""; } \
    static const char* GetPrefix() { return #PREFIX; } \
  }; \
  aeMeta::EnumCreator2< E > ae_enum_creator_##E( #E ); \
  template <> const aeMeta::Enum* aeMeta::GetEnum< E >() { static const aeMeta::Enum* e = GetEnum( #E ); return e; }

// Register c-style enum value
#define AE_META_ENUM_VALUE( E, V ) \
  aeMeta::EnumCreator2< E > ae_enum_creator_##E##_##V( #V, V );

// Register c-style enum value with a manually specified name
#define AE_META_ENUM_VALUE_NAME( E, V, N ) \
aeMeta::EnumCreator2< E > ae_enum_creator_##E##_##V( #N, V );

//------------------------------------------------------------------------------
// External enum class registerer
//------------------------------------------------------------------------------
// Register an already defined enum class type
#define AE_META_ENUM_CLASS( E ) \
  template <> \
  struct aeMeta::VarType< E > { \
    static aeMeta::Var::Type GetType() { return aeMeta::Var::Enum; } \
    static const char* GetName() { return #E; } \
    static const char* GetRefTypeName() { return ""; } \
    static const char* GetPrefix() { return ""; } \
  }; \
  namespace aeEnums::_##E { aeMeta::EnumCreator2< E > ae_enum_creator( #E ); } \
  template <> const aeMeta::Enum* aeMeta::GetEnum< E >() { static const aeMeta::Enum* e = GetEnum( #E ); return e; }
  // @NOTE: Nested namespace declaration requires C++17

// Register enum class value
#define AE_META_ENUM_CLASS_VALUE( E, V ) \
  namespace aeEnums::_##E { aeMeta::EnumCreator2< E > ae_enum_creator_##V( #V, E::V ); }

//------------------------------------------------------------------------------
// External base meta object
//------------------------------------------------------------------------------
class aeObject
{
public:
  virtual ~aeObject() {}
  static const char* GetBaseTypeName() { return ""; }
  static const aeMeta::Type* GetBaseType() { return nullptr; }
  aeMetaTypeId GetTypeId() const { return _metaTypeId; }

  aeMetaTypeId _metaTypeId;
  aeStr32 _typeName;
};

//------------------------------------------------------------------------------
// External inheritor meta object
//------------------------------------------------------------------------------
template < typename Parent, typename Child >
class aeInheritor : public Parent
{
public:
  aeInheritor()
  {
    const aeMeta::Type* t = aeMeta::GetTypeByName( aeMeta::TypeName< Child >::Get() );
    AE_ASSERT_MSG( t, "No inheritor type" );
    aeObject::_metaTypeId = t->GetId();
    aeObject::_typeName = aeMeta::TypeName< Child >::Get();
  }

  typedef Parent aeBaseType;

  static const char* GetBaseTypeName() { return aeMeta::TypeName< Parent >::Get(); }
  static const aeMeta::Type* GetBaseType() { return aeMeta::GetType( aeMeta::TypeName< Parent >::Get() ); }
};

//------------------------------------------------------------------------------
// aeCast
//------------------------------------------------------------------------------
template< typename T, typename C >
const T* aeCast( const C* obj )
{
  // Cast down to base
  static_assert( std::is_base_of< C, T >::value || std::is_base_of< T, C >::value, "Unrelated types" );
  return dynamic_cast< const T* >( obj );
}

template< typename T, typename C >
T* aeCast( C* obj )
{
  // Cast down to base
  static_assert( std::is_base_of< C, T >::value || std::is_base_of< T, C >::value, "Unrelated types" );
  return dynamic_cast< T* >( obj );
}

//------------------------------------------------------------------------------
// Internal aeObject functions
//------------------------------------------------------------------------------
static aeMetaTypeId aeMetaGetObjectTypeId( const aeObject* obj )
{
  return obj ? obj->_metaTypeId : kAeInvalidMetaTypeId;
}

static aeMetaTypeId aeMetaGetTypeIdFromName( const char* name )
{
  // @TODO: Look into https://en.cppreference.com/w/cpp/types/type_info/hash_code
  return name[ 0 ] ? ae::Hash().HashString( name ).Get() : kAeInvalidMetaTypeId;
}

#endif
