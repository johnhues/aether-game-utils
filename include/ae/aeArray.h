//------------------------------------------------------------------------------
// aeArray.h
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
#ifndef AE_ARRAY_H
#define AE_ARRAY_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aeAlloc.h"
namespace AE_NAMESPACE {

//------------------------------------------------------------------------------
// Array class
//------------------------------------------------------------------------------
template < typename T >
class Array
{
public:
  Array();
  Array( uint32_t size ); // Reserve size (with length of 0)
  Array( uint32_t length, const T& val ); // Reserves 'length' and appends 'length' number of 'val's
  Array( const Array< T >& other );
  ~Array();
  void operator =( const Array< T >& other );
  
  T& Append( const T& value );
  void Append( const T* values, uint32_t count );

  T& Insert( uint32_t index, const T& value );

  template < typename U > int32_t Find( const U& value ) const; // Returns -1 when not found
  template < typename Fn > int32_t FindFn( Fn testFn ) const; // Returns -1 when not found
  void Remove( uint32_t index );

  template < typename U > uint32_t RemoveAll( const U& value );
  template < typename Fn > uint32_t RemoveAllFn( Fn testFn );

  void Reserve( uint32_t total );
  void Clear();
  
  // @NOTE: Performs bounds checking in debug mode. Use 'begin()' to get raw array.
  const T& operator[]( int32_t index ) const;
  T& operator[]( int32_t index );

  uint32_t Length() const;
  uint32_t Size() const;

  // @NOTE: These functions can return null when array length is zero
  T* Begin() { return m_array; }
  T* End() { return m_array + m_length; }
  const T* Begin() const { return m_array; }
  const T* End() const { return m_array + m_length; }
  
private:
  uint32_t m_GetNextSize() const;
  
  uint32_t m_length;
  uint32_t m_size;
  T* m_array;
  
public:
  // @NOTE: Ranged-based loop. Lowercase to match c++ standard ('-.-)
  T* begin() { return m_array; }
  T* end() { return m_array + m_length; }
  const T* begin() const { return m_array; }
  const T* end() const { return m_array + m_length; }
};

//------------------------------------------------------------------------------
// Array ostream operator
//------------------------------------------------------------------------------
template < typename T >
inline std::ostream& operator<<( std::ostream& os, const Array< T >& array )
{
  os << "<";
  for ( uint32_t i = 0; i < array.Length(); i++ )
  {
    os << array[ i ];
    if ( i != array.Length() - 1 )
    {
      os << ", ";
    }
  }
  return os << ">";
}

//------------------------------------------------------------------------------
// Array member functions
//------------------------------------------------------------------------------
template < typename T >
Array< T >::Array()
{
  m_length = 0;
  m_size = 0;
  m_array = nullptr;
}

template < typename T >
Array< T >::Array( uint32_t size )
{
  m_length = 0;
  m_size = 0;
  m_array = nullptr;

  Reserve( size );
}

template < typename T >
Array< T >::Array( uint32_t length, const T& value )
{
  m_length = 0;
  m_size = 0;
  m_array = nullptr;

  Reserve( length );

  m_length = length;
  for ( uint32_t i = 0; i < length; i++ )
  {
    new ( &m_array[ i ] ) T ( value );
  }
}

template < typename T >
Array< T >::Array( const Array< T >& other )
{
  m_length = 0;
  m_size = 0;
  m_array = nullptr;
  
  // Array must be initialized above before calling Reserve
  Reserve( other.m_length );

  m_length = other.m_length;
  for ( uint32_t i = 0; i < m_length; i++ )
  {
    new ( &m_array[ i ] ) T ( other.m_array[ i ] );
  }
}

template < typename T >
Array< T >::~Array()
{
  Clear();
  
  aeAlloc::Release( (typename std::aligned_storage< sizeof(T), alignof(T) >::type*)m_array );
  m_size = 0;
  m_array = nullptr;
}

template < typename T >
void Array< T >::operator =( const Array< T >& other )
{
  if ( m_array == other.m_array )
  {
    return;
  }
  
  Clear();
  
  if ( m_size < other.m_length )
  {
    Reserve( other.m_length );
  }

  m_length = other.m_length;
  for ( uint32_t i = 0; i < m_length; i++ )
  {
    new ( &m_array[ i ] ) T ( other.m_array[ i ] );
  }
}

template < typename T >
T& Array< T >::Append( const T& value )
{
  if ( m_length == m_size )
  {
    Reserve( m_GetNextSize() );
  }

  new ( &m_array[ m_length ] ) T ( value );
  m_length++;

  return m_array[ m_length - 1 ];
}

template < typename T >
void Array< T >::Append( const T* values, uint32_t count )
{
  Reserve( m_length + count );

#if _AE_DEBUG_
  AE_ASSERT( m_size >= m_length + count );
#endif
  for ( uint32_t i = 0; i < count; i++ )
  {
    new ( &m_array[ m_length ] ) T ( values[ i ] );
    m_length++;
  }
}

template < typename T >
T& Array< T >::Insert( uint32_t index, const T& value )
{
#if _AE_DEBUG_
  AE_ASSERT( index <= m_length );
#endif

  if ( m_length == m_size )
  {
    Reserve( m_GetNextSize() );
  }

  if ( index == m_length )
  {
    new ( &m_array[ index ] ) T ( value );
  }
  else
  {
    new ( &m_array[ m_length ] ) T ( std::move( m_array[ m_length - 1 ] ) );
    for ( int32_t i = m_length - 1; i > index; i-- )
    {
      m_array[ i ] = std::move( m_array[ i - 1 ] );
    }
    m_array[ index ] = value;
  }
  
  m_length++;

  return m_array[ index ];
}

template < typename T >
void Array< T >::Remove( uint32_t index )
{
#if _AE_DEBUG_
  AE_ASSERT( index < m_length );
#endif

  m_length--;
  for ( uint32_t i = index; i < m_length; i++ )
  {
    m_array[ i ] = std::move( m_array[ i + 1 ] );
  }
  m_array[ m_length ].~T();
}

template < typename T >
template < typename U >
uint32_t Array< T >::RemoveAll( const U& value )
{
  uint32_t count = 0;
  int32_t index = 0;
  while ( ( index = Find( value ) ) >= 0 )
  {
    // @TODO: Update this to be single loop, so array is only compacted once
    Remove( index );
    count++;
  }
  return count;
}

template < typename T >
template < typename Fn >
uint32_t Array< T >::RemoveAllFn( Fn testFn )
{
  uint32_t count = 0;
  int32_t index = 0;
  while ( ( index = FindFn( testFn ) ) >= 0 )
  {
    // @TODO: Update this to be single loop, so array is only compacted once
    Remove( index );
    count++;
  }
  return count;
}

template < typename T >
template < typename U >
int32_t Array< T >::Find( const U& value ) const
{
  for ( uint32_t i = 0; i < m_length; i++ )
  {
    if ( m_array[ i ] == value )
    {
      return i;
    }
  }
  return -1;
}

template < typename T >
template < typename Fn >
int32_t Array< T >::FindFn( Fn testFn ) const
{
  for ( uint32_t i = 0; i < m_length; i++ )
  {
    if ( testFn( m_array[ i ] ) )
    {
      return i;
    }
  }
  return -1;
}

template < typename T >
void Array< T >::Reserve( uint32_t size )
{
  if ( size <= m_size )
  {
    return;
  }

  // Next power of two
  size--;
  size |= size >> 1;
  size |= size >> 2;
  size |= size >> 4;
  size |= size >> 8;
  size |= size >> 16;
  size++;

#if _AE_DEBUG_
  AE_ASSERT( size );
#endif
  m_size = size;

  T* arr = (T*)aeAlloc::AllocateArray< typename std::aligned_storage< sizeof(T), alignof(T) >::type >( m_size );
  for ( uint32_t i = 0; i < m_length; i++ )
  {
    new ( &arr[ i ] ) T ( std::move( m_array[ i ] ) );
    m_array[ i ].~T();
  }

  aeAlloc::Release( (typename std::aligned_storage< sizeof(T), alignof(T) >::type*)m_array );
  m_array = arr;
}

template < typename T >
void Array< T >::Clear()
{
  for ( uint32_t i = 0; i < m_length; i++ )
  {
    m_array[ i ].~T();
  }
  m_length = 0;
}

template < typename T >
const T& Array< T >::operator[]( int32_t index ) const
{
#if _AE_DEBUG_
  AE_ASSERT( index >= 0 );
  AE_ASSERT( index < (int32_t)m_length );
#endif
  return m_array[ index ];
}

template < typename T >
T& Array< T >::operator[]( int32_t index )
{
#if _AE_DEBUG_
  AE_ASSERT( index >= 0 );
  AE_ASSERT_MSG( index < (int32_t)m_length, "index: # length: #", index, m_length );
#endif
  return m_array[ index ];
}

template < typename T >
uint32_t Array< T >::Length() const
{
  return m_length;
}

template < typename T >
uint32_t Array< T >::Size() const
{
  return m_size;
}

template < typename T >
uint32_t Array< T >::m_GetNextSize() const
{
  if ( m_size == 0 )
  {
    return aeMath::Max( 1, 32 / sizeof(T) ); // @NOTE: Initially allocate 32 bytes (rounded down) of type
  }
  else
  {
    return m_size * 2;
  }
}

} // ae namespace end

// @TODO: Remove
#define aeArray ae::Array

#endif
