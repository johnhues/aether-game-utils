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
#include "aePlatform.h"
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
#define META_FORCE_LINK_APP(x) { extern int force_link_##x; force_link_##x = 1; }
#define META_FORCE_LINK_LIB( x ) int force_link_##x = 0;

//------------------------------------------------------------------------------
// Internal meta forward declarations
//------------------------------------------------------------------------------
static aeMetaTypeId aeMetaGetObjectTypeId( const aeObject* obj );

//------------------------------------------------------------------------------
// External meta types
//------------------------------------------------------------------------------
class aeMeta
{
public:
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
      Ref
    };

    const char* GetName() const { return m_name.c_str(); }
    Var::Type GetType() const { return m_type; }
    const char* GetTypeName() const { return m_typeName.c_str(); }
    uint32_t GetOffset() const { return m_offset; }
    uint32_t GetSize() const { return m_size; }
    
    // @TODO: Replace return type with an dynamic aeStr
    std::string GetObjectValueAsString( const aeObject* obj ) const
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
          return aeStr32::Format( "#", *reinterpret_cast< const uint8_t* >( varData ) ).c_str();
        case Var::UInt16:
          return aeStr32::Format( "#", *reinterpret_cast< const uint16_t* >( varData ) ).c_str();
        case Var::UInt32:
          return aeStr32::Format( "#", *reinterpret_cast< const uint32_t* >( varData ) ).c_str();
        case Var::Int8:
          return aeStr32::Format( "#", *reinterpret_cast< const int8_t* >( varData ) ).c_str();
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
        case Var::Ref:
          AE_FAIL(); // @TODO
          return "";
      }
      
      return "";
    }
    
    bool SetObjectValueFromString( aeObject* obj, const char* value ) const
    {
      if ( !obj )
      {
        return false;
      }
      
      // @TODO: Add debug safety check to make sure 'this' Var belongs to 'obj' aeMeta::Type
      
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
        case Var::Ref:
        {
          void* v = varData;
          AE_FAIL(); // @TODO
          return false;
        }
      }
      
      return false;
    }
    
    template < typename T >
    bool SetObjectValue( aeObject* obj, const T& value ) const
    {
      if ( !obj )
      {
        return false;
      }
      
      // @TODO: Add debug safety check to make sure 'this' Var belongs to 'obj' aeMeta::Type
      
      Var::Type typeCheck = aeMeta::VarType< T >::GetType();
      AE_ASSERT( typeCheck == m_type );
      AE_ASSERT( m_size == sizeof( T ) );
      
      T* varData = reinterpret_cast< T* >( (uint8_t*)obj + m_offset );
      *varData = value;
      
      return true;
    }

    aeStr32 m_name;
    Var::Type m_type;
    aeStr32 m_typeName;
    uint32_t m_offset;
    uint32_t m_size;
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
    Type() { memset( this, 0, sizeof( *this ) ); }
    
    template < typename T = aeObject >
    T* New( void* obj ) const
    {
      AE_ASSERT( obj );
      AE_ASSERT_MSG( !m_isAbstract, "Placement new function not available for abstract type: #", m_name.c_str() );
      AE_ASSERT( m_placementNew );
      AE_ASSERT( IsType< T >() );
      AE_ASSERT( (uint64_t)obj % GetAlignment() == 0 );
      
      return (T*)m_placementNew( (T*)obj );
    }

    aeMetaTypeId GetId() const { return m_id; }
    
    bool HasProp( const char* prop ) const
    {
      auto* result = std::find( m_props, m_props + m_propCount, prop );
      return result < ( m_props + m_propCount );
    }
    
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

    uint32_t GetSize() const { return m_size; }
    uint32_t GetAlignment() const { return m_align; }
    const char* GetName() const { return m_name.c_str(); }
    bool IsAbstract() const { return m_isAbstract; }
    bool IsPolymorphic() const { return m_isPolymorphic; }

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
    typename std::enable_if< !std::is_abstract<T>::value, void>::type Init( const char* name, uint32_t index )
    {
      m_placementNew = &( PlacementNewInternal< T > );
      m_name = name;
      m_id = aeHash().HashString( name ).Get();
      m_size = sizeof( T );
      m_align = alignof( T );
      m_parent = T::GetBaseTypeName();
      m_isAbstract = false;
      m_isPolymorphic = std::is_polymorphic< T >::value;
    }
    template < typename T >
    typename std::enable_if< std::is_abstract<T>::value, void>::type Init( const char* name, uint32_t index )
    {
      m_placementNew = nullptr;
      m_name = name;
      m_id = aeHash().HashString( name ).Get();
      m_size = sizeof( T );
      m_align = 0;
      m_parent = T::GetBaseTypeName();
      m_isAbstract = true;
      m_isPolymorphic = std::is_polymorphic< T >::value;
    }
    
    void AddProp( const char* prop )
    {
      AE_ASSERT( m_propCount < kMaxMetaProps );
      m_props[ m_propCount++ ] = prop;
      std::sort( &m_props[ 0 ], &m_props[ m_propCount ], []( const auto& a, const auto& b )
      {
        return a < b;
      });
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
    aeObject* ( *m_placementNew )( aeObject* );
    aeStr32 m_name;
    aeMetaTypeId m_id;
    uint32_t m_size;
    uint32_t m_align;
    aeStr32 m_props[ kMaxMetaProps ];
    uint32_t m_propCount;
    Var m_vars[ kMaxMetaVars ];
    uint32_t m_varCount;
    aeStr32 m_parent;
    bool m_isAbstract;
    bool m_isPolymorphic;
  };

  //------------------------------------------------------------------------------
  // External meta functions
  //------------------------------------------------------------------------------
  static uint32_t GetTypeCount() { return m_GetTypes().size(); }
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
    const char* typeName = aeGetTypeName< T >();
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
      
      Var var;
      var.m_name = varName;
      var.m_type = aeMeta::VarType< V >::GetType();
      var.m_typeName = aeMeta::VarType< V >::GetName();
#if !_AE_WINDOWS_
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Winvalid-offsetof"
#endif
      var.m_offset = Offset;
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
    PropCreator( const char* typeName, const char* propName )
    {
      aeMeta::Type* type = m_GetTypeNameMap().find( typeName )->second;
      type->AddProp( propName );
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
struct aeMeta::VarType< aeStr<N> >
{
  static aeMeta::Var::Type GetType() { return aeMeta::Var::String; }
  static const char* GetName() { return "String"; }
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
};

//------------------------------------------------------------------------------
// External meta class registerer
//------------------------------------------------------------------------------
#define AE_META_CLASS( x ) \
META_FORCE_LINK_LIB( x ) \
template <> const char* aeMeta::TypeName< x >::Get() { return #x; } \
template <> void aeMeta::DefineType< x >( Type *type, uint32_t index ) { type->Init< x >( #x, index ); } \
static aeMeta::TypeCreator< x > _type_creator_##x( #x );

//------------------------------------------------------------------------------
// External meta var registerer
//------------------------------------------------------------------------------
#define AE_META_VAR( c, v ) \
static aeMeta::VarCreator< c, decltype(c::v), offsetof( c, v ) > _var_creator_##c##_##v( #c, #v );
//------------------------------------------------------------------------------
// External meta property registerer
//------------------------------------------------------------------------------
#define AE_META_PROPERTY( c, p ) \
static aeMeta::PropCreator< c > _prop_creator_##c##_##p( #c, #p );

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
  return dynamic_cast< const T* >( obj );
}

template< typename T, typename C >
T* aeCast( C* obj )
{
  // Cast down to base
  return dynamic_cast< T* >( obj );
}

template< typename T >
T* aeCast( aeObject* obj )
{
  if ( !obj )
  {
    return nullptr;
  }

  const aeMeta::Type* objType = aeMeta::GetTypeFromObject( obj );
  const aeMeta::Type* otherType = aeMeta::GetType< T >();
  AE_ASSERT( objType );
  AE_ASSERT( otherType );

  if ( objType->IsType( otherType ) )
  {
    // Cast up to derived
    return static_cast< T* >( obj );
  }
  else
  {
    return nullptr;
  }
}

//------------------------------------------------------------------------------
// Internal aeObject functions
//------------------------------------------------------------------------------
static aeMetaTypeId aeMetaGetObjectTypeId( const aeObject* obj )
{
  return obj->_metaTypeId;
}

#endif
