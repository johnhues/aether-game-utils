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
#include "aeLog.h"
#include "aeMath.h"
#include "aePlatform.h"

struct aeAllocInfo
{
  // Stats
  uint32_t GetAllocationsTotal() const;
  uint32_t GetDeallocationsTotal() const;
  uint32_t GetAllocationBytesTotal() const;
  uint32_t GetDeallocationBytesTotal() const;

  uint32_t GetAllocationsLength() const;
  const char* GetAllocationName( uint32_t index ) const;
  uint32_t GetAllocationCount( uint32_t index ) const;
  uint32_t GetAllocationBytes( uint32_t index ) const;

  uint32_t GetDeallocationsLength() const;
  const char* GetDeallocationName( uint32_t index ) const;
  uint32_t GetDeallocationCount( uint32_t index ) const;
  uint32_t GetDeallocationBytes( uint32_t index ) const;

  // Internal
  void Alloc( const char* typeName, uint32_t bytes );
  void Dealloc( const char* typeName, uint32_t bytes );
};
aeAllocInfo& GetAllocInfo();

#define _AE_ALLOC_DISABLE 0

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
    Header()
    {
      AE_STATIC_ASSERT( sizeof( *this ) <= kHeaderSize );
      AE_STATIC_ASSERT( kHeaderSize % kDefaultAlignment == 0 );
    }

    uint32_t check;
    uint32_t count;
    uint32_t size;
    uint32_t typeSize;
  };
};

#if _AE_ALLOC_DISABLE
void PushArrayAlloc( void* );
bool PopArrayAlloc( void* );
#endif

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
#if _AE_ALLOC_DISABLE
  T* result = new T[ count ];
  PushArrayAlloc( result );
  return result;

#else
  AE_STATIC_ASSERT( alignof( T ) <= kDefaultAlignment );
  AE_STATIC_ASSERT( sizeof( T ) % alignof( T ) == 0 ); // All elements in array should have correct alignment

  uint32_t size = kHeaderSize + sizeof( T ) * count;
  uint8_t* base = (uint8_t*)aeAlignedAlloc( size, kDefaultAlignment );
  AE_ASSERT( (intptr_t)base % kDefaultAlignment == 0 );
#if _AE_DEBUG_
  memset( (void*)base, 0xCD, size );
#endif

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

#if _AE_DEBUG_
  GetAllocInfo().Alloc( aeGetTypeName< T >(), size );
#endif

  return result;
#endif
}

template < typename T, typename ... Args >
static T* aeAlloc::Allocate( Args ... args )
{
#if _AE_ALLOC_DISABLE
  return new T( args ... );
#else
  AE_STATIC_ASSERT( alignof( T ) <= kDefaultAlignment );

  uint32_t size = kHeaderSize + sizeof( T );
  uint8_t* base = (uint8_t*)aeAlignedAlloc( size, kDefaultAlignment );
  AE_ASSERT( (intptr_t)base % kDefaultAlignment == 0 );
#if _AE_DEBUG_
  memset( (void*)base, 0xCD, size );
#endif

  Header* header = (Header*)base;
  header->check = 0xABCD;
  header->count = 1;
  header->size = size;
  header->typeSize = sizeof( T );

#if _AE_DEBUG_
  GetAllocInfo().Alloc( aeGetTypeName< T >(), size );
#endif

  return new( (T*)( base + kHeaderSize ) ) T( args ... );
#endif
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
  uint8_t* base = (uint8_t*)aeAlignedAlloc( size, kDefaultAlignment );
  AE_ASSERT( (intptr_t)base % kDefaultAlignment == 0 );
#if _AE_DEBUG_
  memset( (void*)base, 0xCD, size );
#endif

  Header* header = (Header*)base;
  header->check = 0xABCD;
  header->count = count;
  header->size = size;
  header->typeSize = typeSize;

#if _AE_DEBUG_
  GetAllocInfo().Alloc( "raw", size );
#endif

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

#if _AE_ALLOC_DISABLE
  if ( PopArrayAlloc( obj ) )
  {
    delete[] obj;
  }
  else
  {
    delete obj;
  }
#else
  AE_ASSERT( (intptr_t)obj % kDefaultAlignment == 0 );
  uint8_t* base = (uint8_t*)obj - kHeaderSize;

  Header* header = (Header*)( base );
  AE_ASSERT( header->check == 0xABCD );

  uint32_t count = header->count;
  AE_ASSERT_MSG( sizeof( T ) <= header->typeSize, "Released type T '#' does not match allocated type of size #", aeGetTypeName< T >(), header->typeSize );
  for ( uint32_t i = 0; i < count; i++ )
  {
    T* o = (T*)( (uint8_t*)obj + header->typeSize * i );
    o->~T();
  }

#if _AE_DEBUG_
  memset( (void*)base, 0xDD, header->size );
  GetAllocInfo().Dealloc( aeGetTypeName< T >(), header->size );
#endif

  aeAlignedFree( base );
#endif
}

#endif
