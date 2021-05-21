//------------------------------------------------------------------------------
// aeDict.h
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
#ifndef AEDICT_H
#define AEDICT_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aeMath.h"
#include "aeRender.h" // @TODO: Create aeColor.h
#include "aeString.h"

//------------------------------------------------------------------------------
// aeDict class
//------------------------------------------------------------------------------
class aeDict
{
public:
  void SetString( const char* key, const char* value );
  void SetInt( const char* key, int32_t value );
  void SetFloat( const char* key, float value );
  void SetBool( const char* key, bool value );
  void SetFloat2( const char* key, aeFloat2 value );
  void SetFloat3( const char* key, aeFloat3 value );
  void SetFloat4( const char* key, aeFloat4 value );
  void SetInt2( const char* key, aeInt2 value );
  void Clear();

  const char* GetString( const char* key, const char* defaultValue ) const;
  int32_t GetInt( const char* key, int32_t defaultValue ) const;
  float GetFloat( const char* key, float defaultValue ) const;
  bool GetBool( const char* key, bool defaultValue ) const;
  aeFloat2 GetFloat2( const char* key, aeFloat2 defaultValue ) const;
  aeFloat3 GetFloat3( const char* key, aeFloat3 defaultValue ) const;
  aeFloat4 GetFloat4( const char* key, aeFloat4 defaultValue ) const;
  aeInt2 GetInt2( const char* key, aeInt2 defaultValue ) const;
  aeColor GetColor( const char* key, aeColor defaultValue ) const;
  bool Has( const char* key ) const;

  const char* GetKey( uint32_t idx ) const;
  const char* GetValue( uint32_t idx ) const;
  uint32_t Length() const { return m_entries.Length(); }
  
  // Supported automatic conversions which would otherwise be deleted below
  void SetString( const char* key, char* value ) { SetString( key, (const char*)value ); }
  void SetInt( const char* key, uint32_t value ) { SetInt( key, (int32_t)value ); }
  void SetFloat( const char* key, double value ) { SetFloat( key, (float)value ); }

private:
  // Prevent the above functions from being called accidentally through automatic conversions
  template < typename T > void SetString( const char*, T ) = delete;
  template < typename T > void SetInt( const char*, T ) = delete;
  template < typename T > void SetFloat( const char*, T ) = delete;
  template < typename T > void SetBool( const char*, T ) = delete;
  template < typename T > void SetFloat2( const char*, T ) = delete;
  template < typename T > void SetFloat3( const char*, T ) = delete;
  template < typename T > void SetFloat4( const char*, T ) = delete;
  template < typename T > void SetInt2( const char*, T ) = delete;
  
  aeMap< aeStr128, aeStr128 > m_entries = AE_ALLOC_TAG_FIXME; // @TODO: aeDict should be templated
};

std::ostream& operator<<( std::ostream& os, const aeDict& dict );

#endif
