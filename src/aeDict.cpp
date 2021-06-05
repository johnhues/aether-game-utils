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
void aeDict::SetString( const char* key, const char* value )
{
  m_entries.Set( key, value );
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

void aeDict::SetVec2( const char* key, ae::Vec2 value )
{
  char buf[ 128 ];
  sprintf( buf, "%.2f %.2f", value.x, value.y );
  SetString( key, buf );
}

void aeDict::SetVec3( const char* key, ae::Vec3 value )
{
  char buf[ 128 ];
  sprintf( buf, "%.2f %.2f %.2f", value.x, value.y, value.z );
  SetString( key, buf );
}

void aeDict::SetVec4( const char* key, ae::Vec4 value )
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
  m_entries.Clear();
}

const char* aeDict::GetString( const char* key, const char* defaultValue ) const
{
  if ( const aeStr128* value = m_entries.TryGet( key ) )
  {
    return value->c_str();
  }
  return defaultValue;
}

int32_t aeDict::GetInt( const char* key, int32_t defaultValue ) const
{
  if ( const aeStr128* value = m_entries.TryGet( key ) )
  {
    return atoi( value->c_str() );
  }
  return defaultValue;
}

float aeDict::GetFloat( const char* key, float defaultValue ) const
{
  if ( const aeStr128* value = m_entries.TryGet( key ) )
  {
    return (float)atof( value->c_str() );
  }
  return defaultValue;
}

bool aeDict::GetBool( const char* key, bool defaultValue ) const
{
  if ( const aeStr128* value = m_entries.TryGet( key ) )
  {
    if ( *value == "true" )
    {
      return true;
    }
    else if ( *value == "false" )
    {
      return false;
    }
  }
  return defaultValue;
}

ae::Vec2 aeDict::GetVec2( const char* key, ae::Vec2 defaultValue ) const
{
  if ( const aeStr128* value = m_entries.TryGet( key ) )
  {
    ae::Vec2 result( 0.0f );
    sscanf( value->c_str(), "%f %f", &result.x, &result.y );
    return result;
  }
  return defaultValue;
}

ae::Vec3 aeDict::GetVec3( const char* key, ae::Vec3 defaultValue ) const
{
  if ( const aeStr128* value = m_entries.TryGet( key ) )
  {
    ae::Vec3 result( 0.0f );
    sscanf( value->c_str(), "%f %f %f", &result.x, &result.y, &result.z );
    return result;
  }
  return defaultValue;
}

ae::Vec4 aeDict::GetVec4( const char* key, ae::Vec4 defaultValue ) const
{
  if ( const aeStr128* value = m_entries.TryGet( key ) )
  {
    ae::Vec4 result( 0.0f );
    sscanf( value->c_str(), "%f %f %f %f", &result.x, &result.y, &result.z, &result.w );
    return result;
  }
  return defaultValue;
}

aeInt2 aeDict::GetInt2( const char* key, aeInt2 defaultValue ) const
{
  if ( const aeStr128* value = m_entries.TryGet( key ) )
  {
    aeInt2 result( 0 );
    sscanf( value->c_str(), "%d %d", &result.x, &result.y );
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
  return m_entries.TryGet( key ) != nullptr;
}

const char* aeDict::GetKey( uint32_t idx ) const
{
  return m_entries.GetKey( idx ).c_str();
}

const char* aeDict::GetValue( uint32_t idx ) const
{
  return m_entries.GetValue( idx ).c_str();
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
