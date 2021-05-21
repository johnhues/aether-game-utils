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
#include "aeMath.h"
namespace AE_NAMESPACE {

//------------------------------------------------------------------------------
// Array class
//------------------------------------------------------------------------------
template < typename T, uint32_t N = 0 >
class Array
{
public:
  // Static array (N > 0)
  Array();
  Array( uint32_t length, const T& val ); // Appends 'length' number of 'val's
  // Dynamic array (N == 0)
  Array( ae::Tag tag );
  Array( ae::Tag tag, uint32_t size ); // Reserve size (with length of 0)
  Array( ae::Tag tag, uint32_t length, const T& val ); // Reserves 'length' and appends 'length' number of 'val's
  void Reserve( uint32_t total );
  // Static and dynamic arrays
  Array( const Array< T, N >& other ); // Move operators fallback to regular operators if ae::Tags don't match
  Array( Array< T, N >&& other ) noexcept;
  void operator =( const Array< T, N >& other );
  void operator =( Array< T, N >&& other ) noexcept;
  ~Array();
  
  // Add elements
  T& Append( const T& value );
  void Append( const T* values, uint32_t count );
  T& Insert( uint32_t index, const T& value );

  // Find elements
  template < typename U > int32_t Find( const U& value ) const; // Returns -1 when not found
  template < typename Fn > int32_t FindFn( Fn testFn ) const; // Returns -1 when not found

  // Remove elements
  template < typename U > uint32_t RemoveAll( const U& value );
  template < typename Fn > uint32_t RemoveAllFn( Fn testFn );
  void Remove( uint32_t index );
  void Clear();

  // Access elements
  const T& operator[]( int32_t index ) const; // Performs bounds checking in debug mode. Use 'Begin()' to get raw array.
  T& operator[]( int32_t index );
  T* Begin() { return m_array; } // These functions can return null when array length is zero
  T* End() { return m_array + m_length; }
  const T* Begin() const { return m_array; }
  const T* End() const { return m_array + m_length; }

  // Array info
  uint32_t Length() const;
  uint32_t Size() const;
  
private:
  uint32_t m_GetNextSize() const;
  uint32_t m_length;
  uint32_t m_size;
  T* m_array;
  typename std::aligned_storage< sizeof(T), alignof(T) >::type m_static[ N ];
  ae::Tag m_tag;
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
template < typename T, uint32_t N >
inline std::ostream& operator<<( std::ostream& os, const Array< T, N >& array )
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
template < typename T, uint32_t N >
Array< T, N >::Array()
{
  AE_STATIC_ASSERT_MSG( N != 0, "Must provide allocator for non-static arrays" );
  
  m_length = 0;
  m_size = N;
  m_array = (T*)m_static;
}

template < typename T, uint32_t N >
Array< T, N >::Array( uint32_t length, const T& value )
{
  AE_STATIC_ASSERT_MSG( N != 0, "Must provide allocator for non-static arrays" );
  
  m_length = length;
  m_size = N;
  m_array = (T*)m_static;
  for ( uint32_t i = 0; i < length; i++ )
  {
    new ( &m_array[ i ] ) T ( value );
  }
}

template < typename T, uint32_t N >
Array< T, N >::Array( ae::Tag tag )
{
  AE_STATIC_ASSERT_MSG( N == 0, "Do not provide allocator for static arrays" );
  
  m_length = 0;
  m_size = 0;
  m_array = nullptr;
  m_tag = tag;
}

template < typename T, uint32_t N >
Array< T, N >::Array( ae::Tag tag, uint32_t size )
{
  AE_STATIC_ASSERT_MSG( N == 0, "Do not provide allocator for static arrays" );
  
  m_length = 0;
  m_size = 0;
  m_array = nullptr;
  m_tag = tag;

  Reserve( size );
}

template < typename T, uint32_t N >
Array< T, N >::Array( ae::Tag tag, uint32_t length, const T& value )
{
  AE_STATIC_ASSERT_MSG( N == 0, "Do not provide allocator for static arrays" );
  
  m_length = 0;
  m_size = 0;
  m_array = nullptr;
  m_tag = tag;

  Reserve( length );

  m_length = length;
  for ( uint32_t i = 0; i < length; i++ )
  {
    new ( &m_array[ i ] ) T ( value );
  }
}

template < typename T, uint32_t N >
Array< T, N >::Array( const Array< T, N >& other )
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

template < typename T, uint32_t N >
Array< T, N >::Array( Array< T, N >&& other ) noexcept
{
  if ( N || m_tag != other.m_tag )
  {
    m_length = 0;
    m_size = 0;
    m_array = nullptr;
    *this = other; // Regular assignment (without std::move)
  }
  else
  {
    m_length = other.m_length;
    m_size = other.m_size;
    m_array = other.m_array;
    
    other.m_length = 0;
    other.m_size = 0;
    other.m_array = nullptr;
  }
}

template < typename T, uint32_t N >
Array< T, N >::~Array()
{
  Clear();
  
  if ( N == 0 )
  {
    ae::GetGlobalAllocator()->Free( m_array );
  }
  m_size = 0;
  m_array = nullptr;
}

template < typename T, uint32_t N >
void Array< T, N >::operator =( const Array< T, N >& other )
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

template < typename T, uint32_t N >
void Array< T, N >::operator =( Array< T, N >&& other ) noexcept
{
  if ( N || m_tag != other.m_tag )
  {
    *this = other; // Regular assignment (without std::move)
  }
  else
  {
    if ( m_array )
    {
      Clear();
      ae::GetGlobalAllocator()->Free( m_array );
    }
    
    m_length = other.m_length;
    m_size = other.m_size;
    m_array = other.m_array;
    
    other.m_length = 0;
    other.m_size = 0;
    other.m_array = nullptr;
  }
}

template < typename T, uint32_t N >
T& Array< T, N >::Append( const T& value )
{
  if ( m_length == m_size )
  {
    Reserve( m_GetNextSize() );
  }

  new ( &m_array[ m_length ] ) T ( value );
  m_length++;

  return m_array[ m_length - 1 ];
}

template < typename T, uint32_t N >
void Array< T, N >::Append( const T* values, uint32_t count )
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

template < typename T, uint32_t N >
T& Array< T, N >::Insert( uint32_t index, const T& value )
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

template < typename T, uint32_t N >
void Array< T, N >::Remove( uint32_t index )
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

template < typename T, uint32_t N >
template < typename U >
uint32_t Array< T, N >::RemoveAll( const U& value )
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

template < typename T, uint32_t N >
template < typename Fn >
uint32_t Array< T, N >::RemoveAllFn( Fn testFn )
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

template < typename T, uint32_t N >
template < typename U >
int32_t Array< T, N >::Find( const U& value ) const
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

template < typename T, uint32_t N >
template < typename Fn >
int32_t Array< T, N >::FindFn( Fn testFn ) const
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

template < typename T, uint32_t N >
void Array< T, N >::Reserve( uint32_t size )
{
  if ( N > 0 )
  {
    AE_ASSERT( N >= size );
    return;
  }
  else if ( size <= m_size )
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
  
  AE_ASSERT( m_tag != ae::Tag() );
  T* arr = (T*)ae::GetGlobalAllocator()->Allocate( m_size * sizeof(T), alignof(T), m_tag );
  for ( uint32_t i = 0; i < m_length; i++ )
  {
    new ( &arr[ i ] ) T ( std::move( m_array[ i ] ) );
    m_array[ i ].~T();
  }
  
  ae::GetGlobalAllocator()->Free( m_array );
  m_array = arr;
}

template < typename T, uint32_t N >
void Array< T, N >::Clear()
{
  for ( uint32_t i = 0; i < m_length; i++ )
  {
    m_array[ i ].~T();
  }
  m_length = 0;
}

template < typename T, uint32_t N >
const T& Array< T, N >::operator[]( int32_t index ) const
{
#if _AE_DEBUG_
  AE_ASSERT( index >= 0 );
  AE_ASSERT( index < (int32_t)m_length );
#endif
  return m_array[ index ];
}

template < typename T, uint32_t N >
T& Array< T, N >::operator[]( int32_t index )
{
#if _AE_DEBUG_
  AE_ASSERT( index >= 0 );
  AE_ASSERT_MSG( index < (int32_t)m_length, "index: # length: #", index, m_length );
#endif
  return m_array[ index ];
}

template < typename T, uint32_t N >
uint32_t Array< T, N >::Length() const
{
  return m_length;
}

template < typename T, uint32_t N >
uint32_t Array< T, N >::Size() const
{
  return m_size;
}

template < typename T, uint32_t N >
uint32_t Array< T, N >::m_GetNextSize() const
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

} // AE_NAMESPACE end

#endif
