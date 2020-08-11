//------------------------------------------------------------------------------
// aeCompactingAllocator.cpp
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
#include "aeCompactingAllocator.h"

//------------------------------------------------------------------------------
// aeCompactingAllocator member functions
//------------------------------------------------------------------------------
aeCompactingAllocator::aeCompactingAllocator( uint32_t bytes )
{
  if ( bytes == 0 )
  {
    return;
  }

  m_size = bytes;
  m_data = (uint8_t*)aeAlignedAlloc( bytes, kAlignment );
}

void aeCompactingAllocator::Expand( uint32_t totalBytes )
{
  if ( totalBytes <= m_size )
  {
    return;
  }

  if ( !m_data )
  {
    m_size = totalBytes;
    m_data = (uint8_t*)aeAlignedAlloc( totalBytes, kAlignment );
    return;
  }

  m_Compact();

  // @TODO: Actually do move
  AE_FAIL_MSG( "Not implemented" );
}

aeCompactingAllocator::~aeCompactingAllocator()
{
  AE_ASSERT_MSG( !m_tail, "Cannot be safely destroyed with existing allocations" );

  if ( m_data )
  {
    aeAlignedFree( m_data );
  }
}

void aeCompactingAllocator::m_Compact()
{
  AE_ASSERT( m_data );

  if ( m_isCompact )
  {
    return;
  }

  if ( m_tail == nullptr )
  {
    m_isCompact = true;
    return;
  }

  Header* current = m_tail;
  AE_ASSERT( current->check == 0xABABABAB );
  while ( current->prev )
  {
    AE_ASSERT( current->prev->check == 0xABABABAB );
    current = current->prev;
  }

  uint8_t* open = m_data;
  while ( current )
  {
    if ( current->external )
    {
      if ( open != (uint8_t*)current )
      {
        current->prev = m_tail;
        memmove( open, current, sizeof( Header ) + current->size );
        Header* currentHeader = (Header*)open;
        *( currentHeader->external ) = open + sizeof( Header );
        current = currentHeader;
        m_tail->next = current;
      }
      open += sizeof( Header ) + current->size;
      m_tail = current;
    }
    current = current->next;
  }
  if ( open == m_data )
  {
    m_tail = nullptr;
  }
  if ( m_tail )
  {
    m_tail->next = nullptr;
  }

  if ( m_tail )
  {
    uint8_t* end = (uint8_t*)m_tail + sizeof( Header ) + m_tail->size;
  }

  m_isCompact = true;

  current = m_tail;
  if ( !current )
  {
    return;
  }
  AE_ASSERT( current->check == 0xABABABAB );
  while ( current->prev )
  {
    AE_ASSERT( current->prev->check == 0xABABABAB );
    current = current->prev;
  }
  while ( current )
  {
    AE_ASSERT( current->check == 0xABABABAB );
    current = current->next;
  }
}

//void aeCompactingAllocator::Print()
//{
//  if ( m_tail == nullptr )
//  {
//    return;
//  }
//
//  Header* current = m_tail;
//  while ( current->prev )
//  {
//    current = current->prev;
//  }
//
//  while ( current )
//  {
//    AE_LOG( "s:# a:#", current->size, current->external != nullptr );
//    current = current->next;
//  }
//}

aeCompactingAllocator::Header* aeCompactingAllocator::m_GetHeader( void** p )
{
  Header* header = (Header*)( (uint8_t*)p - sizeof( Header ) );
  AE_ASSERT( *header->external == p );
  AE_ASSERT( header->check == 0xABABABAB );
  return header;
}
