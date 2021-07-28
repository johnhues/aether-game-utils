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
#include "ae/aether.h"
#include <map>
#include <vector>

//------------------------------------------------------------------------------
// External macros to force module linking
//------------------------------------------------------------------------------
#define AE_META_CLASS_FORCE_LINK(x) \
extern int force_link_##x; \
struct ForceLink_##x { ForceLink_##x() { force_link_##x = 1; } }; \
ForceLink_##x forceLink_##x;

class aeObject;

//------------------------------------------------------------------------------
// External meta types
//------------------------------------------------------------------------------
namespace aeMeta
{
  class Type;
  
  //------------------------------------------------------------------------------
  // Meta constants
  //------------------------------------------------------------------------------
  typedef uint32_t aeMetaTypeId;
  const aeMetaTypeId kAeInvalidMetaTypeId = 0;
  const uint32_t kMaxMetaTypes = 64;
  const uint32_t kMaxMetaProps = 8;

  //------------------------------------------------------------------------------
  // Internal meta state
  //------------------------------------------------------------------------------
  std::map< ae::Str32, class Type* >& m_GetTypeNameMap();
  std::map< aeMetaTypeId, class Type* >& m_GetTypeIdMap();
  std::vector< class Type* >& m_GetTypes();

  //------------------------------------------------------------------------------
  // Internal function wrappers
  //------------------------------------------------------------------------------
  template< typename T >
  aeObject* PlacementNewInternal( aeObject* d ) { return new( d ) T(); }

  //------------------------------------------------------------------------------
  // Internal meta forward declarations
  //------------------------------------------------------------------------------
  aeMetaTypeId aeMetaGetObjectTypeId( const aeObject* obj );
  aeMetaTypeId aeMetaGetTypeIdFromName( const char* name );

//------------------------------------------------------------------------------
// External meta functions
//------------------------------------------------------------------------------
  uint32_t GetTypeCount();
  const Type* GetTypeByIndex( uint32_t i );
  const Type* GetTypeById( aeMetaTypeId id );
  const Type* GetTypeByName( const char* typeName );
  const Type* GetTypeFromObject( const aeObject& obj );
  const Type* GetTypeFromObject( const aeObject* obj );
  template < typename T > const Type* GetType();
  const class Enum* GetEnum( const char* enumName );
  
  //------------------------------------------------------------------------------
  // Enum class
  //------------------------------------------------------------------------------
  class Enum
  {
  public:
    const char* GetName() const { return m_name.c_str(); }
    uint32_t TypeSize() const { return m_size; }
    bool TypeIsSigned() const { return m_isSigned; }
    
    template < typename T > std::string GetNameByValue( T value ) const;
    template < typename T > bool GetValueFromString( const char* str, T* valueOut ) const;
    template < typename T > bool HasValue( T value ) const;
    
    int32_t GetValueByIndex( int32_t index ) const;
    std::string GetNameByIndex( int32_t index ) const;
    uint32_t Length() const;
    
    template < typename T > static std::string GetNameFromValue( T value );
    template < typename T > static T GetValueFromString( const char* str, T defaultValue );
  
  private:
    ae::Str32 m_name;
    uint32_t m_size;
    bool m_isSigned;
    ae::Map< int32_t, std::string > m_enumValueToName = AE_ALLOC_TAG_META;
    ae::Map< std::string, int32_t > m_enumNameToValue = AE_ALLOC_TAG_META;
  public: // Internal
    Enum( const char* name, uint32_t size, bool isSigned );
    void m_AddValue( const char* name, int32_t value );
    static Enum* s_Get( const char* enumName, bool create, uint32_t size, bool isSigned );
  };
  
  //------------------------------------------------------------------------------
  // Var class
  //------------------------------------------------------------------------------
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
      Matrix4,
      Enum,
      Ref
    };

    const char* GetName() const { return m_name.c_str(); }
    Var::Type GetType() const { return m_type; }
    const char* GetTypeName() const { return m_typeName.c_str(); }
    uint32_t GetOffset() const { return m_offset; }
    uint32_t GetSize() const { return m_size; }
    
    std::string GetObjectValueAsString( const aeObject* obj, std::function< std::string( const aeObject* ) > getStringFromObjectPointer = nullptr ) const;
    bool SetObjectValueFromString( aeObject* obj, const char* value, std::function< bool( const aeMeta::Type*, const char*, aeObject** ) > getObjectPointerFromString = nullptr ) const;
    bool SetObjectValue( aeObject* obj, const aeObject* value ) const;
    template < typename T > bool SetObjectValue( aeObject* obj, const T& value ) const;
    
    // Types
    const class Enum* GetEnum() const;
    const aeMeta::Type* GetRefType() const;

    // Members
    const aeMeta::Type* m_owner = nullptr;
    ae::Str32 m_name = "";
    Var::Type m_type;
    ae::Str32 m_typeName = "";
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
    uint32_t GetPropertyValueCount( const char* propName ) const { auto* props = m_props.TryGet( propName ); return props ? props->Length() : 0; }
    const char* GetPropertyValue( uint32_t propIndex, uint32_t valueIndex ) const { return m_props.GetValue( propIndex )[ valueIndex ].c_str(); }
    const char* GetPropertyValue( const char* propName, uint32_t valueIndex ) const { return m_props.Get( propName )[ valueIndex ].c_str(); }
    
    // Vars
    uint32_t GetVarCount() const { return m_vars.Length(); }
    const Var* GetVarByIndex( uint32_t i ) const { return &m_vars[ i ]; }
    const Var* GetVarByName( const char* name ) const
    {
      int32_t i = m_vars.FindFn( [name]( auto&& v ) { return v.m_name == name; } );
      return ( i >= 0 ) ? &m_vars[ i ] : nullptr;
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
    const Type* GetBaseType() const;
    bool IsType( const Type* otherType ) const;
    template < typename T > bool IsType() const;
    
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
      m_vars.Append( var );
      std::sort( m_vars.Begin(), m_vars.End(), []( const auto& a, const auto& b )
      {
        return a.GetOffset() < b.GetOffset();
      });
    }

    //------------------------------------------------------------------------------
    // Internal meta type state
    //------------------------------------------------------------------------------
  private:
    aeObject* ( *m_placementNew )( aeObject* ) = nullptr;
    ae::Str32 m_name;
    aeMetaTypeId m_id = kAeInvalidMetaTypeId;
    uint32_t m_size = 0;
    uint32_t m_align = 0;
    ae::Map< ae::Str32, ae::Array< ae::Str32 >, kMaxMetaProps > m_props;
    ae::Array< Var > m_vars = AE_ALLOC_TAG_META;
    ae::Str32 m_parent;
    bool m_isAbstract = false;
    bool m_isPolymorphic = false;
    bool m_isDefaultConstructible = false;
  };
  
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
  void DefineType( Type* type, uint32_t index );

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
  void DefineVar( Var* var );

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
  const Enum* GetEnum();
  
  template < typename E, typename T = typename std::underlying_type< E >::type >
  struct EnumCreator
  {
    EnumCreator( const char* typeName, std::string strMap )
    {
      aeMeta::Enum* enumType = aeMeta::Enum::s_Get( typeName, true, sizeof( T ), std::is_signed< T >::value );
      
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
      uint32_t prefixLen = (uint32_t)strlen( prefix );
      AE_ASSERT( prefixLen < strlen( valueName ) );
      AE_ASSERT( memcmp( prefix, valueName, prefixLen ) == 0 );
      
      aeMeta::Enum* enumType = const_cast< aeMeta::Enum* >( aeMeta::GetEnum< T >() );
      AE_ASSERT_MSG( enumType, "Could not register enum value '#'. No registered Enum.", valueName );
      enumType->m_AddValue( valueName + prefixLen, (int32_t)value );
    }
  };
}

//------------------------------------------------------------------------------
// External inheritor meta object
//------------------------------------------------------------------------------
template < typename Parent, typename Child >
class aeInheritor : public Parent
{
public:
  aeInheritor();

  typedef Parent aeBaseType;

  static const char* GetBaseTypeName() { return aeMeta::TypeName< Parent >::Get(); }
  static const aeMeta::Type* GetBaseType() { return aeMeta::GetType( aeMeta::TypeName< Parent >::Get() ); }
};

//------------------------------------------------------------------------------
// External base meta object
//------------------------------------------------------------------------------
class aeObject
{
public:
  virtual ~aeObject() {}
  static const char* GetBaseTypeName() { return ""; }
  static const aeMeta::Type* GetBaseType() { return nullptr; }
  aeMeta::aeMetaTypeId GetTypeId() const { return _metaTypeId; }

  aeMeta::aeMetaTypeId _metaTypeId;
  ae::Str32 _typeName;
};

template < typename Parent, typename Child >
aeInheritor< Parent, Child >::aeInheritor()
{
  const aeMeta::Type* t = aeMeta::GetTypeByName( aeMeta::TypeName< Child >::Get() );
  AE_ASSERT_MSG( t, "No inheritor type" );
  aeObject::_metaTypeId = t->GetId();
  aeObject::_typeName = aeMeta::TypeName< Child >::Get();
}

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
DefineMetaVarType( ae::Matrix4, Matrix4 );

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
static aeMeta::PropCreator< c > ae_prop_creator_##c##_##p( #c, #p );

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

// @TODO Organize
template < typename T >
bool aeMeta::Var::SetObjectValue( aeObject* obj, const T& value ) const
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

template < typename T >
bool aeMeta::Type::IsType() const
{
  const Type* type = GetType< T >();
  AE_ASSERT( type );
  return IsType( type );
}

template < typename T >
const aeMeta::Type* aeMeta::GetType()
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

template < typename T >
std::string aeMeta::Enum::GetNameFromValue( T value )
{
  const Enum* enumType = GetEnum< T >();
  AE_ASSERT( enumType );
  return enumType->m_enumValueToName.Get( (int32_t)value, "" );
}

template < typename T >
T aeMeta::Enum::GetValueFromString( const char* str, T defaultValue )
{
  const Enum* enumType = GetEnum< T >();
  AE_ASSERT_MSG( enumType, "Value '#' has no Enum #", str, typeid(T).name() ); // TODO: Pretty print
  enumType->GetValueFromString( str, &defaultValue );
  return defaultValue;
}

template < typename T >
std::string aeMeta::Enum::GetNameByValue( T value ) const
{
  return m_enumValueToName.Get( (int32_t)value, "" );
}

template < typename T >
bool aeMeta::Enum::GetValueFromString( const char* str, T* valueOut ) const
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
bool aeMeta::Enum::HasValue( T value ) const
{
  return m_enumValueToName.TryGet( value );
}

#endif
