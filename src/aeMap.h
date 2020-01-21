//------------------------------------------------------------------------------
// aeMap.h
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
#ifndef AEMAP_H
#define AEMAP_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aeArray.h"
#include "aeLog.h"
#include "aePlatform.h"

//------------------------------------------------------------------------------
// aeMap class
//------------------------------------------------------------------------------
template < typename K, typename V >
class aeMap
{
public:
  V& Set( const K& key, const V& value );
  const V& Get( const K& key, const V& defaultValue ) const;
  
  V* TryGet( const K& key );
  const V* TryGet( const K& key ) const;

  bool TryGet( const K& key, V* valueOut );
  bool TryGet( const K& key, V* valueOut ) const;
  
  bool Remove( const K& key );
  void Clear();

  K& GetKey( uint32_t index );
  V& GetValue( uint32_t index );
  const K& GetKey( uint32_t index ) const;
  const V& GetValue( uint32_t index ) const;
  uint32_t Length() const;

private:
  template < typename K2, typename V2 >
  friend std::ostream& operator<<( std::ostream&, const aeMap< K2, V2 >& );

  struct Entry
  {
    Entry() = default;
    Entry( const K& k, const V& v );

    K key;
    V value;
  };

  int32_t m_FindIndex( const K& key ) const;

  aeArray< Entry > m_entries;
};

//------------------------------------------------------------------------------
// aeMap helper functions
//------------------------------------------------------------------------------
template < typename K >
bool aeMap_IsEqual( const K& k0, const K& k1 );

template <>
inline bool aeMap_IsEqual( const char* const & k0, const char* const & k1 )
{
  return strcmp( k0, k1 ) == 0;
}

template < typename K >
bool aeMap_IsEqual( const K& k0, const K& k1 )
{
  return k0 == k1;
}

//------------------------------------------------------------------------------
// aeMap member functions
//------------------------------------------------------------------------------
template < typename K, typename V >
aeMap< K, V >::Entry::Entry( const K& k, const V& v ) :
  key( k ),
  value( v )
{}

template < typename K, typename V >
int32_t aeMap< K, V >::m_FindIndex( const K& key ) const
{
  for ( uint32_t i = 0; i < m_entries.Length(); i++ )
  {
    if ( aeMap_IsEqual( m_entries[ i ].key, key ) )
    {
      return i;
    }
  }

  return -1;
}

template < typename K, typename V >
V& aeMap< K, V >::Set( const K& key, const V& value )
{
  int32_t index = m_FindIndex( key );
  Entry* entry = ( index >= 0 ) ? &m_entries[ index ] : nullptr;
  if ( entry )
  {
    return entry->value;
  }
  else
  {
    return m_entries.Append( Entry( key, value ) ).value;
  }
}

template < typename K, typename V >
const V& aeMap< K, V >::Get( const K& key, const V& defaultValue ) const
{
  int32_t index = m_FindIndex( key );
  return ( index >= 0 ) ? m_entries[ index ].value : defaultValue;
}

template < typename K, typename V >
V* aeMap< K, V >::TryGet( const K& key )
{
  return const_cast< V* >( const_cast< const aeMap< K, V >* >( this )->TryGet( key ) );
}

template < typename K, typename V >
const V* aeMap< K, V >::TryGet( const K& key ) const
{
  int32_t index = m_FindIndex( key );
  if ( index >= 0 )
  {
    return &m_entries[ index ].value;
  }
  else
  {
    return nullptr;
  }
}

template < typename K, typename V >
bool aeMap< K, V >::TryGet( const K& key, V* valueOut )
{
  return const_cast< const aeMap< K, V >* >( this )->TryGet( key, valueOut );
}

template < typename K, typename V >
bool aeMap< K, V >::TryGet( const K& key, V* valueOut ) const
{
  const V* val = TryGet( key );
  if ( val )
  {
    *valueOut = *val;
    return true;
  }
  return false;
}

template < typename K, typename V >
bool aeMap< K, V >::Remove( const K& key )
{
  int32_t index = m_FindIndex( key );
  if ( index >= 0 )
  {
    m_entries.Remove( index );
    return true;
  }
  else
  {
    return false;
  }
}

template < typename K, typename V >
void aeMap< K, V >::Clear()
{
  m_entries.Clear();
}

template < typename K, typename V >
K& aeMap< K, V >::GetKey( uint32_t index )
{
  return m_entries[ index ].key;
}

template < typename K, typename V >
V& aeMap< K, V >::GetValue( uint32_t index )
{
  return m_entries[ index ].value;
}

template < typename K, typename V >
const K& aeMap< K, V >::GetKey( uint32_t index ) const
{
  return m_entries[ index ].key;
}

template < typename K, typename V >
const V& aeMap< K, V >::GetValue( uint32_t index ) const
{
  return m_entries[ index ].value;
}

template < typename K, typename V >
uint32_t aeMap< K, V >::Length() const
{
  return m_entries.Length();
}

template < typename K, typename V >
std::ostream& operator<<( std::ostream& os, const aeMap< K, V >& map )
{
  os << "{";
  for ( uint32_t i = 0; i < map.m_entries.Length(); i++ )
  {
    os << "(" << map.m_entries[ i ].key << ", " << map.m_entries[ i ].value << ")";
    if ( i != map.m_entries.Length() - 1 )
    {
      os << ", ";
    }
  }
  return os << "}";
}

#endif
