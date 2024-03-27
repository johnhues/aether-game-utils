//------------------------------------------------------------------------------
// aeCompactingAllocator.h
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
#ifndef AECOMPACTINGALLOCATOR_H
#define AECOMPACTINGALLOCATOR_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aether.h"

//------------------------------------------------------------------------------
// aeCompactingAllocator class
// @NOTE: 'p's must remain valid and writable until Free() is called.
//        'p's will be rewritten to maintain zero internal fragmentation.
//------------------------------------------------------------------------------
class aeCompactingAllocator
{
public:
  aeCompactingAllocator() = default;
  aeCompactingAllocator( uint32_t bytes );
  void Expand( uint32_t totalBytes );
  ~aeCompactingAllocator();

  template < typename T >
  void Allocate( T** p, uint32_t size );
  
  //template < typename T >
  //void Reallocate( T** p, uint32_t size );
  
  template < typename T >
  void Free( T** p );
  
private:
  const static intptr_t kAlignment = 16; // @TODO: Should be configurable

  struct Header
  {
    uint32_t check;
    uint16_t size;
    Header* next;
    Header* prev;
    void** external;
    uint32_t dbgTypeId;

    uint8_t pad[ 24 ];
  };

  void m_Compact();
  Header* m_GetHeader( void* p );
  void m_Verify();

  bool m_isCompact = true;
  uint8_t* m_data = nullptr;
  Header* m_tail = nullptr;
  uint32_t m_size = 0;
};

//------------------------------------------------------------------------------
// aeCompactingAllocator templated member functions
//------------------------------------------------------------------------------
template < typename T >
void aeCompactingAllocator::Allocate( T** _p, uint32_t size )
{
  AE_ASSERT( m_data );
  AE_STATIC_ASSERT( std::is_pod< T >::value );
  AE_STATIC_ASSERT( alignof( T ) <= kAlignment );
  void** p = (void**)_p;

  // Move all allocations to front of m_data
  m_Compact();

  // Always append newest allocation to end
  if ( m_tail )
  {
    Header* next = (Header*)( (uint8_t*)m_tail + sizeof( Header ) + m_tail->size );
    AE_ASSERT( (uint8_t*)next - m_data + size + sizeof( Header ) < m_size ); // @TODO: Should return null
    next->size = (uint16_t)size;
    next->next = nullptr;
    next->external = p;
    next->dbgTypeId = ae::Hash().HashString( ae::GetTypeName< T >() ).Get();
    next->check = 0xABABABAB;
    *p = (uint8_t*)next + sizeof( Header );
    next->prev = m_tail;
    m_tail->next = next;
    m_tail = next;
  }
  else
  {
    m_tail = (Header*)m_data;
    m_tail->size = (uint16_t)size;
    m_tail->next = nullptr;
    m_tail->prev = nullptr;
    m_tail->external = p;
    m_tail->dbgTypeId = ae::Hash().HashString( ae::GetTypeName< T >() ).Get();
    m_tail->check = 0xABABABAB;
    *p = (uint8_t*)m_tail + sizeof( Header );
  }

  //AE_LOG( "sizeof(Header):#", sizeof( Header ) );
  //AE_ASSERT_MSG( (intptr_t)*p % kAlignment == 0, "Allocation alignment: #", (intptr_t)*p % kAlignment );

  m_Verify();

  //AE_LOG( "Allocate #", *p );
}

//void aeCompactingAllocator::Reallocate( void** p, uint32_t size )
//{
//  AE_ASSERT( m_data );
//
//  m_Compact();
//
//  Header* oldHeader = m_GetHeader( p );
//  if ( oldHeader->size > size )
//  {
//    oldHeader->size = size;
//  }
//  else if ( oldHeader->size < size )
//  {
//    Header* newHeader = (Header*)( (uint8_t*)m_tail + sizeof( Header ) + m_tail->size );
//    AE_ASSERT( (uint8_t*)newHeader - m_data + size + sizeof( Header ) < m_size );
//    memmove( newHeader, oldHeader, sizeof( Header ) + oldHeader->size );
//    *( newHeader->external ) = (uint8_t*)newHeader + sizeof( Header );
//    newHeader->size = size;
//    newHeader->next = nullptr;
//    newHeader->prev = m_tail;
//    m_tail->next = newHeader;
//    m_tail = newHeader;
//  }
//}

template < typename T >
void aeCompactingAllocator::Free( T** p )
{
  AE_ASSERT( p );
  if ( *p == nullptr )
  {
    return;
  }

  //AE_LOG( "Free #", *p );

  AE_ASSERT( m_data );
  AE_ASSERT( m_data < (uint8_t*)*p );
  AE_ASSERT( (uint8_t*)*p < m_data + m_size );

  Header* header = m_GetHeader( (void*)*p );
  uint32_t typeHash = ae::Hash().HashString( ae::GetTypeName< T >() ).Get();
  AE_ASSERT_MSG( header->dbgTypeId == typeHash, "Type mismatch between allocation and free" );
  *(header->external) = nullptr;
  header->external = nullptr;

  // Always flag for compaction, even if only to reclaim empty tail headers
  m_isCompact = false;

  m_Verify();
}

#endif
