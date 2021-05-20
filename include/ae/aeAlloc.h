//------------------------------------------------------------------------------
// aeAlloc.h
// Utilities for allocating objects. Provides functionality to track current and
// past allocations.
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
#ifndef AEALLOC_H
#define AEALLOC_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aether.h"

//------------------------------------------------------------------------------
// aeAlloc
//------------------------------------------------------------------------------
namespace aeAlloc
{
  template < typename T >
  static T* Allocate();

  template < typename T >
  static T* AllocateArray( uint32_t count );

  template < typename T, typename ... Args >
  static T* Allocate( Args ... args );

  static uint8_t* AllocateRaw( uint32_t typeSize, uint32_t typeAlignment, uint32_t count );

  template < typename T >
  static void Release( T* obj );

  template < typename T >
  class Scratch
  {
  public:
    Scratch( uint32_t count )
    {
      m_count = count;
      m_data = AllocateArray< T >( count );
    }

    ~Scratch()
    {
      Release( m_data );
    }

    T* Data() { return m_data; }
    uint32_t Length() const { return m_count; }

    T& operator[] ( int32_t index ) { return m_data[ index ]; }
    const T& operator[] ( int32_t index ) const { return m_data[ index ]; }
    T& GetSafe( int32_t index ) { AE_ASSERT( index < (int32_t)m_count ); return m_data[ index ]; }
    const T& GetSafe( int32_t index ) const { AE_ASSERT( index < (int32_t)m_count ); return m_data[ index ]; }

  private:
    T* m_data;
    uint32_t m_count;
  };

  static const uint32_t kDefaultAlignment = 16;
  static const uint32_t kHeaderSize = 16;
  struct Header
  {
    uint32_t check;
    uint32_t count;
    uint32_t size;
    uint32_t typeSize;
  };
};

//------------------------------------------------------------------------------
// aeAlloc member functions
//------------------------------------------------------------------------------
template < typename T >
T* aeAlloc::Allocate()
{
  return AllocateArray< T >( 1 );
}

template < typename T >
T* aeAlloc::AllocateArray( uint32_t count )
{
  AE_STATIC_ASSERT( alignof( T ) <= kDefaultAlignment );
  AE_STATIC_ASSERT( sizeof( T ) % alignof( T ) == 0 ); // All elements in array should have correct alignment

  uint32_t size = kHeaderSize + sizeof( T ) * count;
  uint8_t* base = (uint8_t*)ae::AlignedAlloc( size, kDefaultAlignment );
  AE_ASSERT( (intptr_t)base % kDefaultAlignment == 0 );
#if _AE_DEBUG_
  memset( (void*)base, 0xCD, size );
#endif

  AE_STATIC_ASSERT( sizeof( Header ) <= kHeaderSize );
  AE_STATIC_ASSERT( kHeaderSize % kDefaultAlignment == 0 );

  Header* header = (Header*)base;
  header->check = 0xABCD;
  header->count = count;
  header->size = size;
  header->typeSize = sizeof( T );

  T* result = (T*)( base + kHeaderSize );
  for ( uint32_t i = 0; i < count; i++ )
  {
    new( &result[ i ] ) T();
  }

  return result;
}

template < typename T, typename ... Args >
T* aeAlloc::Allocate( Args ... args )
{
  AE_STATIC_ASSERT( alignof( T ) <= kDefaultAlignment );

  uint32_t size = kHeaderSize + sizeof( T );
  uint8_t* base = (uint8_t*)ae::AlignedAlloc( size, kDefaultAlignment );
  AE_ASSERT( (intptr_t)base % kDefaultAlignment == 0 );
#if _AE_DEBUG_
  memset( (void*)base, 0xCD, size );
#endif

  Header* header = (Header*)base;
  header->check = 0xABCD;
  header->count = 1;
  header->size = size;
  header->typeSize = sizeof( T );

  return new( (T*)( base + kHeaderSize ) ) T( args ... );
}

uint8_t* aeAlloc::AllocateRaw( uint32_t typeSize, uint32_t typeAlignment, uint32_t count )
{
#if _AE_ALLOC_DISABLE
  return AllocateArray< uint8_t >( count );
#else
  AE_ASSERT( typeAlignment );
  AE_ASSERT( typeAlignment <= kDefaultAlignment );
  AE_ASSERT( typeSize % typeAlignment == 0 ); // All elements in array should have correct alignment

  uint32_t size = kHeaderSize + typeSize * count;
  uint8_t* base = (uint8_t*)ae::AlignedAlloc( size, kDefaultAlignment );
  AE_ASSERT( (intptr_t)base % kDefaultAlignment == 0 );
#if _AE_DEBUG_
  memset( (void*)base, 0xCD, size );
#endif

  Header* header = (Header*)base;
  header->check = 0xABCD;
  header->count = count;
  header->size = size;
  header->typeSize = typeSize;

  return base + kHeaderSize;
#endif
}

template < typename T >
void aeAlloc::Release( T* obj )
{
  if ( !obj )
  {
    return;
  }

  AE_ASSERT( (intptr_t)obj % kDefaultAlignment == 0 );
  uint8_t* base = (uint8_t*)obj - kHeaderSize;

  Header* header = (Header*)( base );
  AE_ASSERT( header->check == 0xABCD );

  uint32_t count = header->count;
  AE_ASSERT_MSG( sizeof( T ) <= header->typeSize, "Released type T '#' does not match allocated type of size #", ae::GetTypeName< T >(), header->typeSize );
  for ( uint32_t i = 0; i < count; i++ )
  {
    T* o = (T*)( (uint8_t*)obj + header->typeSize * i );
    o->~T();
  }

#if _AE_DEBUG_
  memset( (void*)base, 0xDD, header->size );
#endif

  ae::AlignedFree( base );
}

#endif
