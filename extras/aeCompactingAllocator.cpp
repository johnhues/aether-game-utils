//------------------------------------------------------------------------------
// aeCompactingAllocator.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2025 John Hughes
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
#include "ae/aeCompactingAllocator.h"

//------------------------------------------------------------------------------
// aeCompactingAllocator member functions
//------------------------------------------------------------------------------
aeCompactingAllocator::aeCompactingAllocator( uint32_t bytes )
{
  if( bytes == 0 )
  {
    return;
  }

  m_size = bytes;
  m_data = (uint8_t*)ae::Allocate( AE_ALLOC_TAG_FIXME, bytes, kAlignment );
}

void aeCompactingAllocator::Expand( uint32_t totalBytes )
{
  if( totalBytes <= m_size )
  {
    return;
  }

  if( !m_data )
  {
    m_size = totalBytes;
    m_data = (uint8_t*)ae::Allocate( AE_ALLOC_TAG_FIXME, totalBytes, kAlignment );
    return;
  }

  m_Compact();

  // @TODO: Actually do move
  AE_FAIL_MSG( "Not implemented" );
}

aeCompactingAllocator::~aeCompactingAllocator()
{
  AE_ASSERT_MSG( !m_tail, "Cannot be safely destroyed with existing allocations" );

  if( m_data )
  {
    ae::Free( m_data );
  }
}

void aeCompactingAllocator::m_Compact()
{
  AE_ASSERT( m_data );

  if( m_isCompact )
  {
    return;
  }

  //AE_LOG( "compact" );

  // Get first
  Header* first = m_tail;
  AE_ASSERT( first->check == 0xABABABAB );
  while( first->prev )
  {
    AE_ASSERT( first->prev->check == 0xABABABAB );
    first = first->prev;
  }

  m_tail = nullptr;

  uint8_t* open = m_data;
  for( Header* current = first; current; current = current->next )
  {
    // Only compact current allocations. Freed headers will be squashed in Compact()
    if( !current->external )
    {
      Header* prev = current->prev;
      Header* next = current->next;
      if( prev )
      {
        prev->next = next;
      }
      if( next )
      {
        next->prev = prev;
      }
      continue;
    }

    // Check if already compact
    if( open != (uint8_t*)current )
    {
      // Move entry to front of open
      AE_ASSERT( open >= m_data );
      AE_ASSERT( open < m_data + m_size );
      AE_ASSERT( open + current->size < m_data + m_size );
      memmove( open, current, sizeof( Header ) + current->size );
      current = (Header*)open;
      
      // Update linked list
      current->prev = m_tail;
      if( current->prev )
      {
        current->prev->next = current;
      }
      current->next = nullptr;

      // Update external pointer value
      *( current->external ) = open + sizeof( Header );
    }

    // Increment open offset based on current link in list
    open += sizeof( Header ) + current->size;
    AE_ASSERT( open == (uint8_t*)current + sizeof( Header ) + current->size );

    // Tail is always latest allocation
    m_tail = current;
  }
  
  m_isCompact = true;

  if( !m_tail )
  {
    return;
  }

  m_Verify();
}

aeCompactingAllocator::Header* aeCompactingAllocator::m_GetHeader( void* p )
{
  Header* header = (Header*)( (uint8_t*)p - sizeof( Header ) );
  AE_ASSERT( header->check == 0xABABABAB );
  AE_ASSERT( *header->external == p );
  return header;
}

void aeCompactingAllocator::m_Verify()
{
  AE_ASSERT( m_tail->check == 0xABABABAB );

  Header* first = m_tail;
  while( first->prev )
  {
    AE_ASSERT( first->prev->check == 0xABABABAB );
    first = first->prev;
  }

  for( Header* current = first; current; current = current->next )
  {
    AE_ASSERT( current->check == 0xABABABAB );
  }
}
