//------------------------------------------------------------------------------
// aeDict.cpp
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
#include "aeDict.h"

//------------------------------------------------------------------------------
// aeDict members
//------------------------------------------------------------------------------
aeDict::aeDict()
{
  m_entryCount = 0;
}

void aeDict::SetString( const char* key, const char* value )
{
  m_GetValue( key )->value = value;
}

void aeDict::SetInt( const char* key, int32_t value )
{
  char buf[ 128 ];
  sprintf( buf, "%d", value );
  SetString( key, buf );
}

void aeDict::SetFloat( const char* key, float value )
{
  char buf[ 128 ];
  sprintf( buf, "%f", value );
  SetString( key, buf );
}

void aeDict::SetBool( const char* key, bool value )
{
  SetString( key, value ? "true" : "false" );
}

void aeDict::SetFloat2( const char* key, aeFloat2 value )
{
  char buf[ 128 ];
  sprintf( buf, "%.2f %.2f", value.x, value.y );
  SetString( key, buf );
}

void aeDict::SetFloat3( const char* key, aeFloat3 value )
{
  char buf[ 128 ];
  sprintf( buf, "%.2f %.2f %.2f", value.x, value.y, value.z );
  SetString( key, buf );
}

void aeDict::SetFloat4( const char* key, aeFloat4 value )
{
  char buf[ 128 ];
  sprintf( buf, "%.2f %.2f %.2f %.2f", value.x, value.y, value.z, value.w );
  SetString( key, buf );
}

void aeDict::SetInt2( const char* key, aeInt2 value )
{
  char buf[ 128 ];
  sprintf( buf, "%d %d", value.x, value.y );
  SetString( key, buf );
}

void aeDict::Clear()
{
  m_entryCount = 0;
}

const char* aeDict::GetString( const char* key, const char* defaultValue ) const
{
  const KeyValue* kv = m_GetValue( key );
  if ( kv )
  {
    return kv->value.c_str();
  }
  return defaultValue;
}

int32_t aeDict::GetInt( const char* key, int32_t defaultValue ) const
{
  const KeyValue* kv = m_GetValue( key );
  if ( kv )
  {
    return atoi( kv->value.c_str() );
  }
  return defaultValue;
}

float aeDict::GetFloat( const char* key, float defaultValue ) const
{
  const KeyValue* kv = m_GetValue( key );
  if ( kv )
  {
    return (float)atof( kv->value.c_str() );
  }
  return defaultValue;
}

bool aeDict::GetBool( const char* key, bool defaultValue ) const
{
  const KeyValue* kv = m_GetValue( key );
  if ( kv )
  {
    if ( kv->value == "true" )
    {
      return true;
    }
    else if ( kv->value == "false" )
    {
      return false;
    }
  }
  return defaultValue;
}

aeFloat2 aeDict::GetFloat2( const char* key, aeFloat2 defaultValue ) const
{
  const KeyValue* kv = m_GetValue( key );
  if ( kv )
  {
    aeFloat2 result( 0.0f );
    sscanf( kv->value.c_str(), "%f %f", &result.x, &result.y );
    return result;
  }
  return defaultValue;
}

aeFloat3 aeDict::GetFloat3( const char* key, aeFloat3 defaultValue ) const
{
  const KeyValue* kv = m_GetValue( key );
  if ( kv )
  {
    aeFloat3 result( 0.0f );
    sscanf( kv->value.c_str(), "%f %f %f", &result.x, &result.y, &result.z );
    return result;
  }
  return defaultValue;
}

aeFloat4 aeDict::GetFloat4( const char* key, aeFloat4 defaultValue ) const
{
  const KeyValue* kv = m_GetValue( key );
  if ( kv )
  {
    aeFloat4 result( 0.0f );
    sscanf( kv->value.c_str(), "%f %f %f %f", &result.x, &result.y, &result.z, &result.w );
    return result;
  }
  return defaultValue;
}

aeInt2 aeDict::GetInt2( const char* key, aeInt2 defaultValue ) const
{
  const KeyValue* kv = m_GetValue( key );
  if ( kv )
  {
    aeInt2 result( 0 );
    sscanf( kv->value.c_str(), "%d %d", &result.x, &result.y );
    return result;
  }
  return defaultValue;
}

aeColor aeDict::GetColor( const char* key, aeColor defaultValue ) const
{
  // uint8_t c[ 4 ];
  // const KeyValue* kv = m_GetValue( key );
  // if ( kv && kv->value.Length() == 9 && sscanf( kv->value.c_str(), "#%2hhx%2hhx%2hhx%2hhx", &c[ 0 ], &c[ 1 ], &c[ 2 ], &c[ 3 ] ) == 4 )
  // {
  //   return (Color)((c[ 0 ] << 24) | (c[ 1 ] << 16) | (c[ 2 ] << 8) | c[ 3 ]);
  // }
  return defaultValue;
}

bool aeDict::Has( const char* key ) const
{
  return m_GetValue( key ) != nullptr;
}

const char* aeDict::GetKey( uint32_t idx ) const
{
  AE_ASSERT( idx < m_entryCount );
  return m_entries[ idx ].key.c_str();
}

const char* aeDict::GetValue( uint32_t idx ) const
{
  AE_ASSERT( idx < m_entryCount );
  return m_entries[ idx ].value.c_str();
}

aeDict::KeyValue* aeDict::m_GetValue( const char* key )
{
  for ( uint32_t i = 0; i < m_entryCount; i++ )
  {
    if ( m_entries[ i ].key == key )
    {
      return &m_entries[ i ];
    }
  }

  AE_ASSERT_MSG( key[ 0 ], "Invalid aeDict entry key" );
  AE_ASSERT_MSG( m_entryCount < kAeDictMaxKeyValues, "Could not allocate aeDict entry" );
  KeyValue* kv = &m_entries[ m_entryCount ];
  kv->key = key;
  m_entryCount++;
  return kv;
}

const aeDict::KeyValue* aeDict::m_GetValue( const char* key ) const
{
  for ( uint32_t i = 0; i < m_entryCount; i++ )
  {
    if ( m_entries[ i ].key == key )
    {
      return &m_entries[ i ];
    }
  }

  return nullptr;
}

std::ostream& operator<<( std::ostream& os, const aeDict& dict )
{
  os << "[";
  for ( uint32_t i = 0; i < dict.Length(); i++ )
  {
    if ( i )
    {
      os << ",";
    }
    os << "<'" << dict.GetKey( i ) << "','" << dict.GetValue( i ) << "'>";
  }
  return os << "]";
}
